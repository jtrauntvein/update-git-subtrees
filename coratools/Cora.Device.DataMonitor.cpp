/* Cora.Device.DataMonitor.cpp

   Copyright (C) 2004, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 03 November 2004
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2010-10-04 09:49:42 -0600 (Mon, 04 Oct 2010) $ 
   Last Changed by: $Author: tmecham $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.DataMonitor.h"
#include "Cora.Broker.RecordDesc.h"
#include <iterator>


namespace Cora
{
   namespace Device
   {
      namespace
      {
         ////////////////////////////////////////////////////////////
         // class event_base
         ////////////////////////////////////////////////////////////
         class event_base: public Csi::Event
         {
         protected:
            ////////////////////////////////////////////////////////////
            // monitor
            ////////////////////////////////////////////////////////////
            DataMonitor *monitor;

            ////////////////////////////////////////////////////////////
            // client
            ////////////////////////////////////////////////////////////
            typedef DataMonitor::client_type client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_base(
               uint4 event_id,
               DataMonitor *monitor_,
               client_type *client_):
               Event(event_id,monitor_),
               monitor(monitor_),
               client(client_)
            { }

         public:
            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify() = 0;

            ////////////////////////////////////////////////////////////
            // do_notify
            ////////////////////////////////////////////////////////////
            void do_notify()
            {
               if(client_type::is_valid_instance(client))
                  notify();
               else
                  monitor->finish();
            }

            friend class Cora::Device::DataMonitor;
         };

         
         ////////////////////////////////////////////////////////////
         // class started_event
         ////////////////////////////////////////////////////////////
         class started_event: public event_base
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // record
            ////////////////////////////////////////////////////////////
            typedef client_type::record_handle record_handle;
            record_handle record;

            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            {
               std::list<record_handle> records;
               records.push_back(record);
               monitor->add_recycled_records(records);
               client->on_started(monitor,record);
            }
            
            ////////////////////////////////////////////////////////////
            // cpost
            //////////////////////////////////////////////////////////// 
            static void cpost(
               DataMonitor *monitor,
               client_type *client,
               record_handle &record)
            {
               try{(new started_event(monitor,client,record))->post(); }
               catch(Csi::Event::BadPost &) { }
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            started_event(
               DataMonitor *monitor,
               client_type *client,
               record_handle &record_):
               event_base(event_id,monitor,client),
               record(record_)
            { }
         };


         uint4 const started_event::event_id =
         Csi::Event::registerType("Cora::Devise::DataMonitor::started_event");


         ////////////////////////////////////////////////////////////
         // class failure_event
         ////////////////////////////////////////////////////////////
         class failure_event: public event_base
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // failure
            ////////////////////////////////////////////////////////////
            typedef client_type::failure_type failure_type;
            failure_type failure;

            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            { client->on_failure(monitor,failure); }
            
            ////////////////////////////////////////////////////////////
            // cpost
            //////////////////////////////////////////////////////////// 
            static void cpost(
               DataMonitor *monitor,
               client_type *client,
               failure_type failure)
            {
               try{(new failure_event(monitor,client,failure))->post(); }
               catch(Csi::Event::BadPost &) { }
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            failure_event(
               DataMonitor *monitor,
               client_type *client,
               failure_type failure_):
               event_base(event_id,monitor,client),
               failure(failure_)
            { }
         };


         uint4 const failure_event::event_id =
         Csi::Event::registerType("Cora::Device::DataMonitor::failure_event");
         

         ////////////////////////////////////////////////////////////
         // class records_event
         ////////////////////////////////////////////////////////////
         class records_event: public event_base
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // record
            ////////////////////////////////////////////////////////////
            typedef client_type::records_type records_type;
            records_type records;

            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            {
               monitor->add_recycled_records(records);
               client->on_records(monitor,records);
            }
            
            ////////////////////////////////////////////////////////////
            // cpost
            //////////////////////////////////////////////////////////// 
            static void cpost(
               DataMonitor *monitor,
               client_type *client,
               records_type const &records)
            {
               try{(new records_event(monitor,client,records))->post(); }
               catch(Csi::Event::BadPost &) { }
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            records_event(
               DataMonitor *monitor,
               client_type *client,
               records_type const &records_):
               event_base(event_id,monitor,client),
               records(records_)
            { }   
         };


         uint4 const records_event::event_id =
         Csi::Event::registerType("Cora::Device::DataMonitor::records_event");
      };


      ////////////////////////////////////////////////////////////
      // class DataMonitor definitions
      ////////////////////////////////////////////////////////////
      DataMonitor::DataMonitor():
         client(0),
         state(state_standby)
      {
         value_factory.bind(new Cora::Broker::ValueFactory);
      } // constructor

      
      DataMonitor::~DataMonitor()
      { finish(); }

      
      void DataMonitor::start(
         client_type *client_,
         router_handle &router)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
            {
               state = state_delegate;
               client = client_;
               DeviceBase::start(router);
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw Csi::MsgExcept("Invalid state for starting");
      } // start

      
      void DataMonitor::start(
         client_type *client_,
         ClientBase *other_component)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
            {
               state = state_delegate;
               client = client_;
               DeviceBase::start(other_component);
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw Csi::MsgExcept("Invalid state for starting");
      } // start

      
      void DataMonitor::finish()
      {
         client = 0;
         state = state_standby;
         recycled_records.clear();
         monitor_tran = 0;
         DeviceBase::finish();
      } // finish


      void DataMonitor::add_recycled_records(recycled_records_type &source)
      {
         std::copy(
            source.begin(),
            source.end(),
            std::back_inserter(recycled_records));
      } // add_recycled_records

      
      void DataMonitor::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(state == state_active)
         {
            switch(msg->getMsgType())
            {
            case Messages::data_monitor_start_ack:
               on_start_ack(msg);
               break;
               
            case Messages::data_monitor_not:
               on_notification(msg);
               break;
               
            case Messages::data_monitor_stopped_not:
               on_stopped_notification(msg);
               break;
               
            default:
               DeviceBase::onNetMessage(rtr,msg);
               break;
            }
         }
         else if(state == state_delegate)
            DeviceBase::onNetMessage(rtr,msg);
      } // onNetMessage

      
      void DataMonitor::on_devicebase_ready()
      {
         Csi::Messaging::Message cmd(
            device_session,
            Messages::data_monitor_start_cmd);
         cmd.addUInt4(monitor_tran = ++last_tran_no);
         cmd.addWStr(table_name);
         cmd.addBool(true);     // do translation
         state = state_active;
         router->sendMessage(&cmd);
      } // on_devicebase_ready

      
      void DataMonitor::on_devicebase_failure(devicebase_failure_type failure)
      {
         client_type::failure_type client_failure;
         switch(failure)
         {
         case devicebase_failure_logon:
            client_failure = client_type::failure_invalid_logon;
            break;
            
         case devicebase_failure_session:
            client_failure = client_type::failure_connection_failed;
            break;
            
         case devicebase_failure_unsupported:
            client_failure = client_type::failure_unsupported;
            break;
            
         case devicebase_failure_security:
            client_failure = client_type::failure_server_security;
            break;

         case devicebase_failure_invalid_device_name:
            client_failure = client_type::failure_invalid_station_name;
            break;
            
         default:
            client_failure = client_type::failure_unknown;
            break;   
         }
         failure_event::cpost(this,client,client_failure);
      } // on_brokerbase_failure

      
      void DataMonitor::on_devicebase_session_failure()
      {
         failure_event::cpost(this,client,client_type::failure_connection_failed);
      } // on_devicebase_session_failure

      
      void DataMonitor::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         event_base *event = dynamic_cast<event_base *>(ev.get_rep());
         if(event)
         {
            client_type *client = this->client;
            if(event->getType() == failure_event::event_id)
               finish();
            if(event->client == client)
               event->do_notify();
         }
      } // receive

      
      void DataMonitor::on_start_ack(Csi::Messaging::Message *message)
      {
         uint4 tran_no;
         uint4 outcome;
         message->readUInt4(tran_no);
         message->readUInt4(outcome);
         if(tran_no == monitor_tran)
         {
            if(outcome == 1)
            {
               description.bind(
                  new Cora::Broker::RecordDesc(
                     get_device_name(),
                     table_name));
               if(description->read(*message,true))
               {
                  record_handle record(
                     new Cora::Broker::Record(
                        description,
                        *value_factory));
                  started_event::cpost(this,client,record);
               }
               else
                  failure_event::cpost(this,client,client_type::failure_unknown);
            }
            else
            {
               client_type::failure_type failure;
               switch(outcome)
               {
               case 3:
                  failure = client_type::failure_invalid_table_name;
                  break;
                  
               default:
                  failure = client_type::failure_unknown;
                  break;
               }
               failure_event::cpost(this,client,failure);
            }
         }
      } // on_start_ack

      
      void DataMonitor::on_notification(Csi::Messaging::Message *message)
      {
         uint4 tran_no;
         message->readUInt4(tran_no);
         if(state == state_active && tran_no == monitor_tran)
         {
            uint4 records_count;
            client_type::records_type records;

            message->readUInt4(records_count);
            for(uint4 i = 0; i < records_count; ++i)
            {
               // we need to obtain a record handle either from the recycle queue or we will have to
               // create a new record
               record_handle record;
               if(!recycled_records.empty())
               {
                  record = recycled_records.front();
                  recycled_records.pop_front();
               }
               else
               {
                  record.bind(
                     new Cora::Broker::Record(
                        description,
                        *value_factory));
               }

               // process the new record
               record->read(*message,false);
               records.push_back(record);
            }

            // notify the client of the new records
            if(!records.empty())
               records_event::cpost(this,client,records);
         }
      } // on_notification

      
      void DataMonitor::on_stopped_notification(Csi::Messaging::Message *message)
      {
         uint4 tran_no;
         uint4 reason;
         message->readUInt4(tran_no);
         message->readUInt4(reason);
         if(tran_no == monitor_tran)
         {
            client_type::failure_type failure;
            switch(reason)
            {
            case 2:
               failure = client_type::failure_table_deleted;
               break;
               
            case 3:
               failure = client_type::failure_station_shut_down;
               break;
               
            default:
               failure = client_type::failure_unknown;
               break;
            }
            failure_event::cpost(this,client,failure);
         }
      } // on_stopped_notification 
   };
};




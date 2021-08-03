/* Cora.Device.TablesEnumerator.cpp

   Copyright (C) 2000, 2021 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 21 July 2000
   Last Change: Friday 09 April 2021
   Last Commit: $Date: 2021-04-09 18:00:59 -0600 (Fri, 09 Apr 2021) $ 
   Committed by: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.TablesEnumerator.h"


namespace Cora
{
   namespace Device
   {
      namespace TablesEnumeratorHelpers
      {
         class event_base: public Csi::Event
         {
         public:
            TablesEnumerator *tran;
            TablesEnumeratorClient *client;

            event_base(uint4 event_id,
                       TablesEnumerator *tran_,
                       TablesEnumeratorClient *client_):
               tran(tran_),
               client(client_),
               Event(event_id,tran_)
            { }

            virtual void notify() = 0;
         };


         class event_start: public event_base
         {
         public:
            static uint4 const event_id;

            static void create_and_post(TablesEnumerator *tran,
                                        TablesEnumeratorClient *client);

            virtual void notify() { client->on_started(tran); }
            
         private:
            event_start(TablesEnumerator *tran,
                        TablesEnumeratorClient *client):
               event_base(event_id,tran,client)
            { }
         };


         uint4 const event_start::event_id =
         Csi::Event::registerType("Cora::Device::TablesEnumerator::event_start");


         void event_start::create_and_post(TablesEnumerator *tran,
                                        TablesEnumeratorClient *client)
         {
            try { (new event_start(tran,client))->post(); }
            catch(Csi::Event::BadPost &) { }
         }


         class event_failure: public event_base
         {
         public:
            static uint4 const event_id;
            typedef TablesEnumeratorClient::failure_type failure_type;
            failure_type failure;

            static void create_and_post(
               TablesEnumerator *tran, TablesEnumeratorClient *client, failure_type failure)
            {
               try { (new event_failure(tran,client,failure))->post(); }
               catch(Csi::Event::BadPost &) { }
            }

            virtual void notify() { client->on_failure(tran,failure); }
            
         private:
            event_failure(TablesEnumerator *tran,
                        TablesEnumeratorClient *client,
                          failure_type failure_):
               event_base(event_id,tran,client),
               failure(failure_)
            { }
         };
         uint4 const event_failure::event_id =
            Csi::Event::registerType("Cora::Device::TablesEnumerator::event_failure");


         class event_table_added: public event_base
         {
         public:
            static uint4 const event_id;
            StrUni table_name;
            bool enabled;
            typedef std::list<StrUni> area_names_type;
            area_names_type area_names;

            static event_table_added *create(
               TablesEnumerator *tran, TablesEnumeratorClient *client)
            { return new event_table_added(tran,client); }
            
            void post()
            {
               try { Event::post(); }
               catch(Csi::Event::BadPost &) { }
            }

            virtual void notify()
            { client->on_table_added(tran,table_name,enabled,area_names); }
            
         private:
            event_table_added(TablesEnumerator *tran,
                        TablesEnumeratorClient *client):
               event_base(event_id,tran,client)
            { }
         };
         uint4 const event_table_added::event_id =
            Csi::Event::registerType("Cora::Device::TablesEnumerator::event_table_added");


         class event_table_deleted: public event_base
         {
         public:
            static uint4 const event_id;
            StrUni table_name;

            static void create_and_post(
               TablesEnumerator *tran, TablesEnumeratorClient *client, StrUni const &table_name)
            {
               try { (new event_table_deleted(tran,client,table_name))->post(); }
               catch(Csi::Event::BadPost &) { }
            }
            
            virtual void notify()
            { client->on_table_deleted(tran,table_name); }
            
         private:
            event_table_deleted(
               TablesEnumerator *tran, TablesEnumeratorClient *client, StrUni const &table_name_):
               event_base(event_id,tran,client),
               table_name(table_name_)
            { }
         };
         uint4 const event_table_deleted::event_id =
            Csi::Event::registerType("Cora::Device::TablesEnumerator::event_table_deleted");


         class event_table_enabled: public event_base
         {
         public:
            static uint4 const event_id;
            StrUni table_name;
            bool enabled;

            static void create_and_post(
               TablesEnumerator *tran,
               TablesEnumeratorClient *client,
               StrUni const &table_name,
               bool enabled)
            {
               try { (new event_table_enabled(tran,client,table_name,enabled))->post(); }
               catch(Csi::Event::BadPost &) { }
            }
            
            virtual void notify()
            { client->on_table_enabled(tran,table_name,enabled); }
            
         private:
            event_table_enabled(
               TablesEnumerator *tran,
               TablesEnumeratorClient *client,
               StrUni const &table_name_,
               bool enabled_):
               event_base(event_id,tran,client),
               table_name(table_name_),
               enabled(enabled_)
            { }
         };
         uint4 const event_table_enabled::event_id =
            Csi::Event::registerType("Cora::Device::TablesEnumerator::event_table_enabled");


         class event_table_areas_changed: public event_base
         {
         public:
            static uint4 const event_id;
            StrUni table_name;
            typedef std::list<StrUni> area_names_type;
            area_names_type area_names;

            static event_table_areas_changed *create(
               TablesEnumerator *tran, TablesEnumeratorClient *client)
            { return new event_table_areas_changed(tran, client); }
            
            void post()
            {
               try { Event::post(); }
               catch(Csi::Event::BadPost &) { }
            }
            
            virtual void notify()
            { client->on_table_areas_changed(tran,table_name,area_names); }
            
         private:
            event_table_areas_changed(TablesEnumerator *tran,
                                      TablesEnumeratorClient *client):
               event_base(event_id,tran,client)
            { }
         };
         uint4 const event_table_areas_changed::event_id =
            Csi::Event::registerType("Cora::Device::TablesEnumerator::event_table_areas_changed");
      };


      void TablesEnumerator::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace TablesEnumeratorHelpers;
         event_base *event = dynamic_cast<event_base *>(ev.get_rep());

         if(ev->getType() == event_failure::event_id)
            finish();
         if(event && TablesEnumeratorClient::is_valid_instance(event->client))
            event->notify();
         else
            finish();
      } // receive

      void TablesEnumerator::format_failure(std::ostream &out, client_type::failure_type failure)
      {
         switch(failure)
         {
         case client_type::failure_connection_failed:
            format_devicebase_failure(out, devicebase_failure_session);
            break;

         case client_type::failure_invalid_logon:
            format_devicebase_failure(out, devicebase_failure_logon);
            break;

         case client_type::failure_server_security_blocked:
            format_devicebase_failure(out, devicebase_failure_security);
            break;
           
         case client_type::failure_device_name_invalid:
            format_devicebase_failure(out, devicebase_failure_invalid_device_name);
            break;
            
         default:
            format_devicebase_failure(out, devicebase_failure_unknown);
            break;
         }
      } // format_failure

      void TablesEnumerator::onNetMessage(
         Csi::Messaging::Router *rtr, Csi::Messaging::Message *msg)
      {
         if(state == state_active)
         {
            switch(msg->getMsgType())
            {
            case Messages::tables_enum_start_ack:
               on_start_ack(msg);
               break;
               
            case Messages::tables_enum_stopped_not:
               on_stopped_not(msg);
               break;
               
            case Messages::tables_enum_table_added_not:
               on_table_added_not(msg);
               break;
               
            case Messages::tables_enum_table_deleted_not:
               on_table_deleted_not(msg);
               break;

            case Messages::tables_enum_table_areas_not:
               on_table_areas_not(msg);
               break;

            case Messages::tables_enum_table_enabled_not:
               on_table_enabled_not(msg);
               break;
               
            default:
               DeviceBase::onNetMessage(rtr,msg);
               break;
            }
         }
         else
            DeviceBase::onNetMessage(rtr,msg);
      } // onNetMessage
      
      void TablesEnumerator::on_devicebase_failure(devicebase_failure_type failure)
      {
         TablesEnumeratorClient::failure_type client_failure;
         
         switch(failure)
         {
         case devicebase_failure_logon:
            client_failure = TablesEnumeratorClient::failure_invalid_logon;
            break;
            
         case devicebase_failure_session:
            client_failure = TablesEnumeratorClient::failure_connection_failed;
            break;
            
         case devicebase_failure_invalid_device_name:
            client_failure = TablesEnumeratorClient::failure_device_name_invalid;
            break;
            
         default:
            client_failure = TablesEnumeratorClient::failure_unknown;
            break;
         }
         TablesEnumeratorHelpers::event_failure::create_and_post(this,client,client_failure);
      } // on_devicebase_failure
      
      void TablesEnumerator::on_devicebase_ready()
      {
         state = state_active;
         Csi::Messaging::Message start_command(
            device_session, Messages::tables_enum_start_cmd);
         start_command.addUInt4(++last_tran_no);
         start_command.addBool(send_temporaries);
         router->sendMessage(&start_command);
      } // on_devicebase_ready

      void TablesEnumerator::on_start_ack(Csi::Messaging::Message *message)
      {
         using namespace TablesEnumeratorHelpers;
         event_start::create_and_post(this,client);
      } // on_start_ack
      
      void TablesEnumerator::on_stopped_not(Csi::Messaging::Message *message)
      {
         using namespace TablesEnumeratorHelpers;
         event_failure::create_and_post(this,client,TablesEnumeratorClient::failure_connection_failed);
      } // on_stopped_not
      
      void TablesEnumerator::on_table_added_not(Csi::Messaging::Message *message)
      {
         using namespace TablesEnumeratorHelpers;
         event_table_added *event = event_table_added::create(this,client);
         uint4 tran_no;
         uint4 count;
         StrUni area_name;

         message->readUInt4(tran_no);
         message->readWStr(event->table_name);
         message->readBool(event->enabled);
         message->readUInt4(count);
         for(uint4 i = 0; i < count; ++i)
         {
            message->readWStr(area_name);
            event->area_names.push_back(area_name);
         }
         event->post();
      } // on_table_added_not
      
      void TablesEnumerator::on_table_deleted_not(Csi::Messaging::Message *message)
      {
         using namespace TablesEnumeratorHelpers;
         uint4 tran_no;
         StrUni table_name;

         message->readUInt4(tran_no);
         message->readWStr(table_name);
         event_table_deleted::create_and_post(this,client,table_name);
      } // on_table_deleted_not
      
      void TablesEnumerator::on_table_enabled_not(Csi::Messaging::Message *message)
      {
         using namespace TablesEnumeratorHelpers;
         uint4 tran_no;
         StrUni table_name;
         bool enabled;

         message->readUInt4(tran_no);
         message->readWStr(table_name);
         message->readBool(enabled);
         event_table_enabled::create_and_post(this,client,table_name,enabled);
      } // on_table_enabled_not
      
      void TablesEnumerator::on_table_areas_not(Csi::Messaging::Message *message)
      {
         using namespace TablesEnumeratorHelpers;
         uint4 tran_no;
         StrUni area_name;
         uint4 count;
         event_table_areas_changed *event = event_table_areas_changed::create(this,client);
         
         message->readUInt4(tran_no);
         message->readWStr(event->table_name);
         message->readUInt4(count);
         for(uint4 i = 0; i < count; ++i)
         {
            message->readWStr(area_name);
            event->area_names.push_back(area_name);
         }
         event->post();
      } // on_table_areas_not 
   };
};

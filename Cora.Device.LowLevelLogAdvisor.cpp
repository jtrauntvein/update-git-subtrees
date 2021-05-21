/* Cora.Device.LowLevelLogAdvisor.cpp

   Copyright (C) 2002, 2017 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 05 July 2002
   Last Change: Tuesday 19 December 2017
   Last Commit: $Date: 2017-12-19 16:29:39 -0600 (Tue, 19 Dec 2017) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.LowLevelLogAdvisor.h"
#include "Cora.Device.Defs.h"
#include <assert.h>
#include <iterator>


namespace Cora
{
   namespace Device
   {
      namespace LowLevelLogAdvisorHelpers
      {
         ////////////////////////////////////////////////////////////
         // class event_base
         ////////////////////////////////////////////////////////////
         class event_base: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // client
            ////////////////////////////////////////////////////////////
            typedef LowLevelLogAdvisorClient client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // advisor
            ////////////////////////////////////////////////////////////
            typedef LowLevelLogAdvisor advisor_type;
            advisor_type *advisor;

            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify() = 0;
      
         protected:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_base(
               uint4 event_id,
               client_type *client_,
               advisor_type *advisor_):
               Event(event_id,advisor_),
               client(client_),
               advisor(advisor_)
            { }
         };


         ////////////////////////////////////////////////////////////
         // class event_started
         ////////////////////////////////////////////////////////////
         class event_started: public event_base
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;
            
            ////////////////////////////////////////////////////////////
            // create_and_post
            ////////////////////////////////////////////////////////////
            static void create_and_post(
               client_type *client,
               advisor_type *advisor)
            {
               try { (new event_started(client,advisor))->post(); }
               catch(BadPost &) { }
            }
            
            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            { client->on_started(advisor); }
            
         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_started(
               client_type *client,
               advisor_type *advisor):
               event_base(event_id,client,advisor)
            { }
         };


         uint4 const event_started::event_id =
         Csi::Event::registerType("Cora::Device::LowLevelLogAdvisor::event_started");


         ////////////////////////////////////////////////////////////
         // class event_failure
         ////////////////////////////////////////////////////////////
         class event_failure: public event_base
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
            // create_and_post
            ////////////////////////////////////////////////////////////
            static void create_and_post(
               client_type *client,
               advisor_type *advisor,
               failure_type failure)
            {
               try { (new event_failure(client,advisor,failure))->post(); }
               catch(BadPost &) { }
            }

            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            { client->on_failure(advisor,failure); }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_failure(
               client_type *client,
               advisor_type *advisor,
               failure_type failure_):
               event_base(event_id,client,advisor),
               failure(failure_)
            { }
         };


         uint4 const event_failure::event_id =
         Csi::Event::registerType("Cora::Device::LowLevelLogAdvisor::event_failure");


         ////////////////////////////////////////////////////////////
         // class event_record
         ////////////////////////////////////////////////////////////
         class event_record: public event_base
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // record
            ////////////////////////////////////////////////////////////
            typedef advisor_type::log_record_handle record_type;
            record_type record;

            ////////////////////////////////////////////////////////////
            // create_and_post
            ////////////////////////////////////////////////////////////
            static void create_and_post(
               client_type *client,
               advisor_type *advisor,
               record_type &record)
            {
               try { (new event_record(client,advisor,record))->post(); }
               catch(BadPost &) { } 
            }

            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            {
               client->on_log_message(
                  advisor,
                  record->stamp,
                  record->is_input,
                  record->data.getContents(),
                  (uint4)record->data.length());
            }
            
         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_record(
               client_type *client,
               advisor_type *advisor,
               record_type &record_):
               event_base(event_id,client,advisor),
               record(record_)
            { } 
         };


         uint4 const event_record::event_id =
         Csi::Event::registerType("Cora::Device::LowLevelLogAdvisor::event_record"); 
      };


      ////////////////////////////////////////////////////////////
      // class LowLevelLogAdvisor Definitions
      ////////////////////////////////////////////////////////////
      LowLevelLogAdvisor::LowLevelLogAdvisor():
         records_per_batch(100),
         back_up_count(1),
         client(0),
         advise_tran(0),
         state(state_standby)
      { }

      
      LowLevelLogAdvisor::~LowLevelLogAdvisor()
      { finish(); }

      
      void LowLevelLogAdvisor::set_records_per_batch(uint4 records_per_batch_)
      {
         if(state == state_standby)
            records_per_batch = records_per_batch_;
         else
            throw exc_invalid_state();
      } // set_records_per_batch

      
      void LowLevelLogAdvisor::set_back_up_count(uint4 back_up_count_)
      {
         if(state == state_standby)
            back_up_count = back_up_count_;
         else
            throw exc_invalid_state();
      } // set_back_up_count

      
      void LowLevelLogAdvisor::start(
         ClientBase *other_component,
         client_type *client_)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
            {
               client = client_;
               state = state_delegate;
               DeviceBase::start(other_component);
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start

      
      void LowLevelLogAdvisor::start(
         router_handle &router,
         client_type *client_)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
            {
               client = client_;
               state = state_delegate;
               DeviceBase::start(router);
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start

      
      void LowLevelLogAdvisor::finish()
      {
         state = state_standby;
         client = 0;
         unread_records.clear();
         cached_records.clear();
         DeviceBase::finish();
      } // finish

      
      void LowLevelLogAdvisor::next_record()
      {
         if(state == state_wait_for_client)
         {
            // we need to get rid of the first unread record
            if(!unread_records.empty())
            {
               cached_records.push_back(unread_records.front());
               unread_records.pop_front();
            }

            // if there are more unread records, we have only to post the next notification
            using namespace LowLevelLogAdvisorHelpers;
            if(!unread_records.empty())
               event_record::create_and_post(client,this,unread_records.front());
            else
               start_get_next_batch();
         }
         else
            throw exc_invalid_state();
      } // next_record

      
      void LowLevelLogAdvisor::next_batch()
      {
         if(state == state_wait_for_client)
         {
            std::copy(
               unread_records.begin(),
               unread_records.end(),
               std::back_inserter(cached_records));
            unread_records.clear();
            start_get_next_batch();
         }
         else
            throw exc_invalid_state();
      } // next_batch

      
      void LowLevelLogAdvisor::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace LowLevelLogAdvisorHelpers;
         event_base *event = static_cast<event_base *>(ev.get_rep());
         client_type *client = this->client;
         
         if(event->getType() == event_failure::event_id)
            finish();
         if(event->client == client && client_type::is_valid_instance(client))
            event->notify();
      } // receive

      
      void LowLevelLogAdvisor::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(state != state_delegate)
         {
            switch(msg->getMsgType())
            {
            case Messages::low_level_log_advise_not:
               on_log_advise_not(msg);
               break;

            default:
               DeviceBase::onNetMessage(rtr,msg);
               break;
            }
         }
         else
            DeviceBase::onNetMessage(rtr,msg);
      } // onNetMessage
      
      
      void LowLevelLogAdvisor::on_devicebase_ready()
      {
         using namespace LowLevelLogAdvisorHelpers;
         Csi::Messaging::Message start_cmd(device_session,Messages::low_level_log_advise_start_cmd);
         start_cmd.addUInt4(advise_tran = ++last_tran_no);
         start_cmd.addUInt4(records_per_batch);
         start_cmd.addUInt4(back_up_count);
         router->sendMessage(&start_cmd);
         event_started::create_and_post(client,this);
         state = state_wait_for_server;
      } // on_devicebase_ready
      
      
      void LowLevelLogAdvisor::on_devicebase_failure(devicebase_failure_type failure)
      {
         using namespace LowLevelLogAdvisorHelpers;
         client_type::failure_type client_failure;
         switch(failure)
         {
         case devicebase_failure_logon:
            client_failure = client_type::failure_invalid_logon;
            break;
            
         case devicebase_failure_session:
            client_failure = client_type::failure_session_failed;
            break;
            
         case devicebase_failure_unsupported:
            client_failure = client_type::failure_unsupported_transaction;
            break;
            
         case devicebase_failure_security:
            client_failure = client_type::failure_server_security_blocked;
            break;

         case devicebase_failure_invalid_device_name:
            client_failure = client_type::failure_invalid_device_name;
            break;
            
         default:
            client_failure = client_type::failure_unknown;
            break;
         }
         event_failure::create_and_post(client,this,client_failure);
      } // on_devicebase_failure
      
      
      void LowLevelLogAdvisor::on_devicebase_session_failure()
      {
         using namespace LowLevelLogAdvisorHelpers;
         event_failure::create_and_post(client,this,client_type::failure_session_failed);
      } // on_devicebase_session_failure
      
      
      void LowLevelLogAdvisor::on_log_advise_not(Csi::Messaging::Message *message)
      {
         uint4 tran_no;
         bool more_to_come;
         uint4 num_recs;
         message->readUInt4(tran_no);
         message->readBool(more_to_come);
         message->readUInt4(num_recs);

         if(tran_no == advise_tran)
         {
            using namespace LowLevelLogAdvisorHelpers;
            if(more_to_come)
            {
               // we will iterate through each of the log records.
               for(uint4 i = 0; i < num_recs; ++i)
               {
                  // we need to create or to pull off of the cache the next record to read.
                  log_record_handle record;
                  if(!cached_records.empty())
                  {
                     record = cached_records.front();
                     cached_records.pop_front();
                  }
                  else
                     record.bind(new log_record_type);

                  // we can now read the information for the log record
                  int8 stamp; 
                  message->readInt8(stamp);
                  message->readBool(record->is_input);
                  message->readBStr(record->data);
                  record->stamp = stamp;
                  unread_records.push_back(record);
               }

               if(state == state_wait_for_server && !unread_records.empty())
               {
                  event_record::create_and_post(client,this,unread_records.front());
                  state = state_wait_for_client;
               }
            }
            else
               event_failure::create_and_post(client,this,client_type::failure_server_cancelled);
         } 
      } // on_log_advise_not
      
      
      void LowLevelLogAdvisor::start_get_next_batch()
      {
         Csi::Messaging::Message cont_cmd(device_session,Messages::low_level_log_advise_cont_cmd);
         cont_cmd.addUInt4(advise_tran);
         router->sendMessage(&cont_cmd);
         state = state_wait_for_server;
      } // start_get_next_batch 
   };
};

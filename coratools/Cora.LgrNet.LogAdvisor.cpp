/* Cora.LgrNet.LogAdvisor.cpp

   Copyright (C) 2002, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 28 February 2002
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2020-02-10 10:53:20 -0600 (Mon, 10 Feb 2020) $ 
   Last Changed by: $Author: amortenson $

*/

#pragma warning(disable:4786) // disable overlong symbols warning
#pragma hdrstop               // stop creation of precompiled header
#include "Cora.LgrNet.LogAdvisor.h"
#include "Cora.LgrNet.Defs.h"
#include <iterator>
#include "coratools.strings.h"


namespace Cora
{
   namespace LgrNet
   {
      namespace LogAdvisorHelpers
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
            typedef LogAdvisorClient client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // advisor
            ////////////////////////////////////////////////////////////
            typedef LogAdvisor advisor_type;
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
         Csi::Event::registerType("Cora::LgrNet::LogAdvisor::event_started");


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
         Csi::Event::registerType("Cora::LgrNet::LogAdvisor::event_failure");


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
            { client->on_log_message(advisor,record->stamp,record->data); }
            
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
         Csi::Event::registerType("Cora::LgrNet::LogAdvisor::event_record");
      };


      ////////////////////////////////////////////////////////////
      // class LogAdvisor definitions
      ////////////////////////////////////////////////////////////
      LogAdvisor::LogAdvisor():
         state(state_standby),
         client(0),
         log_id(log_comms_status),
         records_per_batch(1),
         back_up_count(0)
      { }


      LogAdvisor::~LogAdvisor()
      { finish(); }


      void LogAdvisor::set_log_id(log_id_type log_id_)
      {
         if(state == state_standby)
            log_id = log_id_;
         else
            throw exc_invalid_state();
      } // set_log_id


      void LogAdvisor::set_records_per_batch(uint4 records_per_batch_)
      {
         if(state == state_standby)
            records_per_batch = records_per_batch_;
         else
            throw exc_invalid_state();
      } // set_records_per_batch


      void LogAdvisor::set_back_up_count(uint4 back_up_count_)
      {
         if(state == state_standby)
            back_up_count = back_up_count_;
         else
            throw exc_invalid_state(); 
      } // set_back_up_count


      void LogAdvisor::start(
         client_type *client_,
         ClientBase *other_component)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
            {
               state = state_delegate;
               client = client_;
               ClientBase::start(other_component);
            }
            else
               throw std::invalid_argument("Invalid LogAdvisorClient instance");
         }
         else
            throw exc_invalid_state();
      } // start


      void LogAdvisor::start(
         client_type *client_,
         router_handle &router)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
            {
               state = state_delegate;
               client = client_;
               ClientBase::start(router);
            }
            else
               throw std::invalid_argument("Invalid LogAdvisorClient instance");
         }
         else
            throw exc_invalid_state();
      } // start


      void LogAdvisor::finish()
      {
         client = 0;
         state = state_standby;
         unread_records.clear();
         cached_records.clear();
         ClientBase::finish();
      } // finish

      
      void LogAdvisor::next_record()
      {
         if(state == state_wait_for_client)
         {
            // we will now get rid of the first record
            if(!unread_records.empty())
            {
               cached_records.push_back(unread_records.front());
               unread_records.pop_front();
            }

            // if there are more unread records, we need do nothing more than post the next record
            // event
            using namespace LogAdvisorHelpers;
            if(!unread_records.empty())
               event_record::create_and_post(client,this,unread_records.front());
            else
               start_get_next_batch();
         }
         else
            throw exc_invalid_state();
      } // next_record


      void LogAdvisor::next_batch()
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


      void LogAdvisor::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace LogAdvisorHelpers;
         event_base *event = static_cast<event_base *>(ev.get_rep());
         client_type *client = this->client;
         
         if(event->getType() == event_failure::event_id)
            finish();
         if(event->client == client && client_type::is_valid_instance(client))
            event->notify();
      } // receive


      void LogAdvisor::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(state != state_delegate)
         {
            switch(msg->getMsgType())
            {
            case Messages::log_advise_not:
               on_log_advise_not(msg);
               break;

            default:
               ClientBase::onNetMessage(rtr,msg);
               break;
            }
         }
         else
            ClientBase::onNetMessage(rtr,msg);
      } // onNetMessage


      void LogAdvisor::on_corabase_ready()
      {
         using namespace LogAdvisorHelpers;
         Csi::Messaging::Message start_cmd(
            net_session,
            Messages::log_advise_start_cmd);
         start_cmd.addUInt4(advise_tran = ++last_tran_no);
         start_cmd.addUInt4(log_id);
         start_cmd.addUInt4(records_per_batch);
         start_cmd.addUInt4(back_up_count);
         router->sendMessage(&start_cmd);
         event_started::create_and_post(client,this);
         state = state_wait_for_server;
      } // on_corabase_ready


      void LogAdvisor::on_corabase_failure(corabase_failure_type failure)
      {
         using namespace LogAdvisorHelpers;
         client_type::failure_type client_failure;
         switch(failure)
         {
         case corabase_failure_logon:
            client_failure = client_type::failure_invalid_logon;
            break;
            
         case corabase_failure_session:
            client_failure = client_type::failure_session_failed;
            break;
            
         case corabase_failure_unsupported:
            client_failure = client_type::failure_unsupported_transaction;
            break;
            
         case corabase_failure_security:
            client_failure = client_type::failure_server_security_blocked;
            break;
            
         default:
            client_failure = client_type::failure_unknown;
            break;
         }
         event_failure::create_and_post(client,this,client_failure);
      } // on_corabase_failure

      void LogAdvisor::format_failure(
         std::ostream &out, client_type::failure_type failure)
      {
         using namespace LogAdvisorStrings;
         switch(failure)
         {
            case client_type::failure_invalid_logon:
               describe_failure(out, corabase_failure_logon);
               break;

            case client_type::failure_session_failed:
               describe_failure(out, corabase_failure_session);
               break;

            case client_type::failure_unsupported_transaction:
               describe_failure(out, corabase_failure_unsupported);
               break;

            case client_type::failure_server_security_blocked:
               describe_failure(out, corabase_failure_security);
               break;

            case client_type::failure_server_cancelled:
               out << my_strings[strid_server_cancelled];
               break;

            default:
               describe_failure(out, corabase_failure_unknown);
               break;
         }
      } // format_failure

      void LogAdvisor::on_corabase_session_failure()
      {
         using namespace LogAdvisorHelpers;
         event_failure::create_and_post(client,this,client_type::failure_session_failed);
      } // on_corabase_session_failure


      void LogAdvisor::on_log_advise_not(Csi::Messaging::Message *message)
      {
         uint4 tran_no;
         bool more_to_come;
         uint4 num_recs;

         message->readUInt4(tran_no);
         message->readBool(more_to_come);
         message->readUInt4(num_recs);

         if(tran_no == advise_tran)
         {
            using namespace LogAdvisorHelpers;
            if(more_to_come)
            {
               for(uint4 i = 0; i < num_recs; ++i)
               {
                  // we need to create or obtain a record to read the next one into
                  log_record_handle record;
                  if(!cached_records.empty())
                  {
                     record = cached_records.front();
                     cached_records.pop_front();
                  }
                  else
                     record.bind(new log_record_type);

                  // we can now read the record from the message
                  int8 stamp_nsec;
                  message->readInt8(stamp_nsec);
                  message->readStr(record->data);
                  record->stamp = stamp_nsec;
                  unread_records.push_back(record);
               }

               if(state == state_wait_for_server && !unread_records.empty())
               {
                  event_record::create_and_post(client,this,unread_records.front());
                  state = state_wait_for_client;
               }
            }
            else
               event_failure::create_and_post(
                  client,
                  this,
                  client_type::failure_server_cancelled);
         }
      } // on_log_advise_not


      void LogAdvisor::start_get_next_batch()
      {
         Csi::Messaging::Message cont_cmd(
            net_session,
            Messages::log_advise_proceed_cmd);
         cont_cmd.addUInt4(advise_tran);
         router->sendMessage(&cont_cmd);
         state = state_wait_for_server;
      } // start_get_next_batch
   };
};


      

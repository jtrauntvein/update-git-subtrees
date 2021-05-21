/* Cora.Device.CollectAreaPoller.cpp

   Copyright (C) 2002, 2015 Campbell Scientific, Inc.
   
   Written by: Jon Trauntvein
   Date Begun: Monday 24 June 2002
   Last Change: Monday 18 May 2015
   Last Commit: $Date: 2015-05-18 13:10:48 -0600 (Mon, 18 May 2015) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.CollectAreaPoller.h"
#include "coratools.strings.h"


namespace Cora
{
   namespace Device 
   {
      namespace CollectAreaPollerHelpers
      {
         ////////////////////////////////////////////////////////////
         // class event_complete
         ////////////////////////////////////////////////////////////
         class event_complete: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // client
            ////////////////////////////////////////////////////////////
            typedef CollectAreaPollerClient client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // outcome
            ////////////////////////////////////////////////////////////
            typedef client_type::outcome_type outcome_type;
            outcome_type outcome;

            ////////////////////////////////////////////////////////////
            // create_and_post
            ////////////////////////////////////////////////////////////
            typedef CollectAreaPoller poller_type;
            static void create_and_post(
               poller_type *poller,
               client_type *client,
               outcome_type outcome);

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_complete(
               poller_type *poller,
               client_type *client_,
               outcome_type outcome_):
               Event(event_id,poller),
               client(client_),
               outcome(outcome_)
            { }
         };


         void event_complete::create_and_post(
            poller_type *poller,
            client_type *client,
            outcome_type outcome)
         {
            try { (new event_complete(poller,client,outcome))->post(); }
            catch(Csi::Event::BadPost &) { }
         }


         uint4 const event_complete::event_id =
         Csi::Event::registerType("Cora::Device::CollectAreaPoller::event_complete");


         ////////////////////////////////////////////////////////////
         // class event_status_not
         ////////////////////////////////////////////////////////////
         class event_status_not: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // client
            ////////////////////////////////////////////////////////////
            typedef CollectAreaPollerClient client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // values_expected
            ////////////////////////////////////////////////////////////
            uint4 values_expected;

            ////////////////////////////////////////////////////////////
            // values_stored
            ////////////////////////////////////////////////////////////
            uint4 values_stored;

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_status_not(
               CollectAreaPoller *poller,
               client_type *client_,
               uint4 values_expected_,
               uint4 values_stored_):
               Event(event_id,poller),
               client(client_),
               values_expected(values_expected_),
               values_stored(values_stored_)
            { }

         public:
            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               CollectAreaPoller *poller,
               client_type *client,
               uint4 values_expected,
               uint4 values_stored)
            {
               try
               {
                  (new event_status_not(
                     poller,
                     client,
                     values_expected,
                     values_stored))->post();
               }
               catch(Csi::Event::BadPost &)
               { }
            }
         };


         uint4 const event_status_not::event_id =
         Csi::Event::registerType(
            "Cora::Device::CollectAreaPoller::event_status_not");
      };

      
      ////////////////////////////////////////////////////////////
      // class CollectAreaPoller definitions
      ////////////////////////////////////////////////////////////
      CollectAreaPoller::CollectAreaPoller():
         client(0),
         state(state_standby),
         priority(priority_higher)
      { }

      
      CollectAreaPoller::~CollectAreaPoller()
      { finish(); }

      
      void CollectAreaPoller::set_collect_area_name(StrUni const &collect_area_name_)
      {
         if(state == state_standby)
            collect_area_name = collect_area_name_;
         else
            throw exc_invalid_state();
      } // set_collect_area_name


      void CollectAreaPoller::set_priority(priority_type priority_)
      {
         if(state != state_standby)
            throw exc_invalid_state();
         priority = priority_;
      } // set_priority

      
      void CollectAreaPoller::start(
         client_type *client_,
         router_handle &router)
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
               throw std::invalid_argument("Invalid client pointer specified");
         }
         else
            throw exc_invalid_state();
      } // start

      
      void CollectAreaPoller::start(
         client_type *client_,
         ClientBase *other_client)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
            {
               client = client_;
               state = state_delegate;
               DeviceBase::start(other_client);
            }
            else
               throw std::invalid_argument("Invalid client pointer specified");
         }
         else
            throw exc_invalid_state();
      } // start


      bool CollectAreaPoller::cancel()
      {
         bool rtn(false);
         if(state == state_active)
         {
            Csi::Messaging::Message cmd(device_session, Messages::selective_manual_poll_stop_cmd);
            cmd.addUInt4(poll_tran);
            router->sendMessage(&cmd);
            rtn = true;
         }
         return rtn;
      } // cancel
      
      
      void CollectAreaPoller::finish()
      {
         client = 0;
         state = state_standby;
         DeviceBase::finish();
      } // finish


      void CollectAreaPoller::format_outcome(std::ostream &out, client_type::outcome_type outcome)
      {
         switch(outcome)
         {
         case client_type::outcome_success:
            out << common_strings[common_success];
            break;
            
         case client_type::outcome_invalid_logon:
            format_devicebase_failure(out, devicebase_failure_logon);
            break;
            
         case client_type::outcome_server_session_failed:
            format_devicebase_failure(out, devicebase_failure_session);
            break;
                     
         case client_type::outcome_invalid_device_name:
            format_devicebase_failure(out, devicebase_failure_invalid_device_name);
            break;
            
         case client_type::outcome_unsupported:
            format_devicebase_failure(out, devicebase_failure_unsupported);
            break;
            
         case client_type::outcome_server_security_blocked:
            format_devicebase_failure(out, devicebase_failure_security);
            break;
            
         case client_type::outcome_logger_security_blocked:
            out << common_strings[common_logger_security_blocked];
            break;
            
         case client_type::outcome_comm_failure:
            out << common_strings[common_comm_failed];
            break;
            
         case client_type::outcome_communication_disabled:
            out << common_strings[common_comm_disabled];
            break;
            
         case client_type::outcome_table_defs_invalid:
            out << common_strings[common_table_defs_invalid];
            break;
            
         case client_type::outcome_invalid_collect_area_name:
            out << common_strings[common_invalid_collect_area_name];
            break;
            
         case client_type::outcome_file_io_failure:
            out << common_strings[common_file_io_failed];
            break;

         case client_type::outcome_logger_busy:
            out << common_strings[common_logger_locked];
            break;

         case client_type::outcome_aborted:
            out << common_strings[common_aborted];
            break;
            
         default:
            format_devicebase_failure(out, devicebase_failure_unknown);
            break;
         }
      } // format_outcome

      
      void CollectAreaPoller::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace CollectAreaPollerHelpers;
         if(ev->getType() == event_complete::event_id)
         {
            event_complete *event = static_cast<event_complete *>(ev.get_rep());
            bool client_is_same = (client == event->client);
            
            finish();
            if(client_is_same && client_type::is_valid_instance(event->client))
               event->client->on_complete(this,event->outcome);
         }
         else if(ev->getType() == event_status_not::event_id)
         {
            event_status_not *event = static_cast<event_status_not *>(ev.get_rep());
            if(event->client == client && client_type::is_valid_instance(client))
            {
               client->on_status_not(
                  this,
                  event->values_expected,
                  event->values_stored);
            }
         }
      } // receive

      
      void CollectAreaPoller::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(state == state_active)
         {
            if(msg->getMsgType() == Messages::selective_manual_poll_ack)
            {
               // read the message
               uint4 tran_no;
               uint4 resp_code;
               
               msg->readUInt4(tran_no);
               msg->readUInt4(resp_code);
               values_expected = values_stored = 0;
               if(msg->whatsLeft() >= 8)
               {
                  msg->readUInt4(values_expected);
                  msg->readUInt4(values_stored);
               }

               // dispatch the event
               using namespace CollectAreaPollerHelpers; 
               client_type::outcome_type outcome;
               
               switch(resp_code)
               {
               case 1:
                  outcome = client_type::outcome_success;
                  break;

               case 3:
                  outcome = client_type::outcome_invalid_collect_area_name;
                  break;

               case 5:
                  outcome = client_type::outcome_communication_disabled;
                  break;

               case 6:
               case 7:
                  outcome = client_type::outcome_comm_failure;
                  break;

               case 8:
                  outcome = client_type::outcome_aborted;
                  break;

               case 9:
                  outcome = client_type::outcome_table_defs_invalid;
                  break;

               case 10:
                  outcome = client_type::outcome_logger_busy;
                  break;
                  
               case 11:
                  outcome = client_type::outcome_file_io_failure;
                  break;

               case 12:
                  outcome = client_type::outcome_logger_security_blocked;
                  break;

               case 13:
                  outcome = client_type::outcome_logger_busy;
                  break;
                  
               default:
                  outcome = client_type::outcome_unknown;
                  break;
               }
               event_complete::create_and_post(
                  this,
                  client,
                  outcome);
            }
            else if(msg->getMsgType() == Messages::selective_manual_poll_status_not)
            {
               uint4 tran_no;
               msg->readUInt4(tran_no);
               msg->readUInt4(values_expected);
               msg->readUInt4(values_stored);
               CollectAreaPollerHelpers::event_status_not::cpost(
                  this,client,values_expected,values_stored);
            }
            else
               DeviceBase::onNetMessage(rtr,msg);
         }
         else
            DeviceBase::onNetMessage(rtr,msg);
      } // onNetMessage

      
      void CollectAreaPoller::on_devicebase_ready()
      {
         Csi::Messaging::Message cmd(device_session,Messages::selective_manual_poll_cmd);
         poll_tran = ++last_tran_no;
         cmd.addUInt4(poll_tran);
         cmd.addWStr(collect_area_name);
         cmd.addUInt4(priority);
         cmd.addBool(true);
         state = state_active;
         router->sendMessage(&cmd);
      } // on_devicebase_ready

      
      void CollectAreaPoller::on_devicebase_failure(devicebase_failure_type failure)
      {
         using namespace CollectAreaPollerHelpers;
         client_type::outcome_type outcome;

         switch(failure)
         {
         case devicebase_failure_logon:
            outcome = client_type::outcome_invalid_logon;
            break;
            
         case devicebase_failure_session:
            outcome = client_type::outcome_server_session_failed;
            break;
            
         case devicebase_failure_invalid_device_name:
            outcome = client_type::outcome_invalid_device_name;
            break;
            
         case devicebase_failure_unsupported:
            outcome = client_type::outcome_unsupported;
            break;
            
         case devicebase_failure_security:
            outcome = client_type::outcome_server_security_blocked;
            break;
            
         default:
            outcome = client_type::outcome_unknown;
            break;
         }
         event_complete::create_and_post(this,client,outcome);
      } // on_devicebase_failure

      
      void CollectAreaPoller::on_devicebase_session_failure()
      {
         using namespace CollectAreaPollerHelpers;
         event_complete::create_and_post(this,client,client_type::outcome_server_session_failed);
      } // on_devicebase_session_failure 
   };
};

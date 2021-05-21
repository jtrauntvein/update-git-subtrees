/* Cora.Device.ManualPoller.cpp

   Copyright (C) 2001, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 11 July 2001
   Last Change: Tuesday 29 September 2020
   Last Commit: $Date: 2020-10-02 10:01:41 -0600 (Fri, 02 Oct 2020) $ 
   Committed by: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.ManualPoller.h"
#include "coratools.strings.h"


namespace Cora
{
   namespace Device
   {
      namespace ManualPollerHelpers
      {
         class event_complete: public Csi::Event
         {
         public:
            typedef ManualPollerClient client_type;
            client_type *client;
            typedef client_type::outcome_type outcome_type;
            outcome_type outcome;
            static uint4 const event_id;

            static void create_and_post(
               ManualPoller *poller,
               client_type *client,
               outcome_type outcome)
            {
               try { (new event_complete(poller,client,outcome))->post(); }
               catch(Csi::Event::BadPost &) { }
            }

         private:
            event_complete(
               ManualPoller *poller,
               client_type *client_,
               outcome_type outcome_):
               Event(event_id,poller),
               client(client_),
               outcome(outcome_)
            { }
         };


         uint4 const event_complete::event_id =
         Csi::Event::registerType("Cora::Device::ManualPoller::event_complete");


         class event_status_notification: public Csi::Event
         {
         public:
            typedef ManualPollerClient client_type;
            client_type *client;
            uint4 values_expected;
            uint4 values_stored;
            static uint4 const event_id;

         private:
            event_status_notification(
               ManualPoller *poller,
               client_type *client_,
               uint4 values_expected_,
               uint4 values_stored_):
               Event(event_id,poller),
               client(client_),
               values_expected(values_expected_),
               values_stored(values_stored_)
            { }

         public:
            static void cpost(
               ManualPoller *poller,
               client_type *client,
               uint4 values_expected,
               uint4 values_stored)
            {
               try
               {
                  (new event_status_notification(
                     poller,client,values_expected,values_stored))->post();
               }
               catch(Csi::Event::BadPost &)
               { }
            }
         };


         uint4 const event_status_notification::event_id =
         Csi::Event::registerType(
            "Cora::Device::ManualPoller::event_status_notification");
      };

      
      bool ManualPoller::cancel()
      {
         bool rtn = false;
         if(interface_version >= Csi::VersionNumber("1.3.1"))
         {
            using namespace ManualPollerHelpers;
            if(state == state_active)
            {
               Csi::Messaging::Message stop_command(
                  device_session,
                  Messages::manual_poll_stop_cmd);
               stop_command.addUInt4(poll_transaction);
               router->sendMessage(&stop_command);
            }
            else
               event_complete::create_and_post(this,client,client_type::outcome_aborted);
            rtn = true;
         }
         return rtn;
      } // cancel


      void ManualPoller::format_outcome(
         std::ostream &out, client_type::outcome_type outcome)
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
            
         case client_type::outcome_aborted:
            out << common_strings[common_aborted];
            break;
            
         case client_type::outcome_logger_locked:
            out << common_strings[common_logger_locked];
            break;
            
         case client_type::outcome_file_io_failed:
            out << common_strings[common_file_io_failed];
            break;
            
         case client_type::outcome_no_table_defs:
            out << common_strings[common_no_tables];
            break;
            
         default:
            format_devicebase_failure(out, devicebase_failure_unknown);
            break;
         }
      } // format_outcome


      void ManualPoller::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace ManualPollerHelpers;
         if(ev->getType() == event_complete::event_id)
         {
            event_complete *event = static_cast<event_complete *>(ev.get_rep());
            if(event->client == client && client_type::is_valid_instance(client))
            {
               finish();
               event->client->on_complete(this,event->outcome);
            }
            else
               finish();
         }
         else if(ev->getType() == event_status_notification::event_id)
         {
            event_status_notification *event =
               static_cast<event_status_notification *>(ev.get_rep());
            if(event->client == client && client_type::is_valid_instance(client))
            {
               client->on_status_notification(
                  this,
                  event->values_expected,
                  event->values_stored);
            } 
         }
      } // receive

      
      void ManualPoller::onNetMessage(Csi::Messaging::Router *rtr, Csi::Messaging::Message *msg)
      {
         if(state == state_active)
         {
            if(msg->getMsgType() == Messages::manual_poll_ack)
            {
               using namespace ManualPollerHelpers;
               uint4 tran_no;
               uint4 resp_code;
               client_type::outcome_type outcome;

               msg->readUInt4(tran_no);
               msg->readUInt4(resp_code);
               switch(resp_code)
               {
               case 1:
                  outcome = client_type::outcome_success;
                  break;

               case 3:
                  outcome = client_type::outcome_logger_security_blocked;
                  break;

               case 4:
                  outcome = client_type::outcome_comm_failure;
                  break;

               case 5:
                  outcome = client_type::outcome_communication_disabled;
                  break;

               case 6:
                  outcome = client_type::outcome_table_defs_invalid;
                  break;

               case 7:
                  outcome = client_type::outcome_aborted;
                  break;

               case 8:
                  outcome = client_type::outcome_logger_locked;
                  break;

               case 9:
                  outcome = client_type::outcome_file_io_failed;
                  break;

               case 10:
                  outcome = client_type::outcome_no_table_defs;
                  break;
                  
               default:
                  outcome = client_type::outcome_unknown;
                  break;
               }
               event_complete::create_and_post(this,client,outcome);
            }
            else if(msg->getMsgType() == Messages::manual_poll_status_not)
            {
               using namespace ManualPollerHelpers;
               uint4 tran_no;
               uint4 values_expected;
               uint4 values_stored;

               msg->readUInt4(tran_no);
               msg->readUInt4(values_expected);
               msg->readUInt4(values_stored);
               event_status_notification::cpost(
                  this,
                  client,
                  values_expected,
                  values_stored);
            }
            else
               DeviceBase::onNetMessage(rtr,msg);
         }
         else
            DeviceBase::onNetMessage(rtr,msg);
      } // onNetMessage

      
      void ManualPoller::on_devicebase_ready()
      {
         if(client_type::is_valid_instance(client))
         {
            Csi::Messaging::Message command(
               device_session,
               Messages::manual_poll_cmd);
            poll_transaction = ++last_tran_no;
            command.addUInt4(poll_transaction);
            command.addBool(true); // send status notifications
            command.addBool(always_store_newest);
            router->sendMessage(&command);
            state = state_active;
         }
         else
            finish();
      } // on_devicebase_ready

      
      void ManualPoller::on_devicebase_failure(devicebase_failure_type failure)
      {
         using namespace ManualPollerHelpers;
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
   };
};

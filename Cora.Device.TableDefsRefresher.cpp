/* Cora.Device.TableDefsRefresher.cpp

   Copyright (C) 2001, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 18 January 2001
   Last Change: Friday 18 October 2019
   Last Commit: $Date: 2019-10-18 14:35:34 -0600 (Fri, 18 Oct 2019) $ 
   Committed by: $Author: jon $
   
*/


#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.TableDefsRefresher.h"
#include "coratools.strings.h"


namespace Cora
{
   namespace Device
   {
      namespace TableDefsRefresherHelpers
      {
         class event_complete: public Csi::Event
         {
         public:
            static uint4 const event_id;
            typedef TableDefsRefresherClient client_type;
            typedef client_type::outcome_type outcome_type;
            client_type *client;
            outcome_type outcome;
            
         private:
            event_complete(
               TableDefsRefresher *refresher_,
               client_type *client_,
               outcome_type outcome_):
               Event(event_id,refresher_),
               client(client_),
               outcome(outcome_)
            { }

         public:
            static void create_and_post(
               TableDefsRefresher *refresher,
               client_type *client,
               outcome_type outcome)
            {
               try { (new event_complete(refresher,client,outcome))->post(); }
               catch(BadPost &) { }
            }
         };


         uint4 const event_complete::event_id =
         Csi::Event::registerType("Cora::Device::TableDefsRefresher::event_complete");
      };


      TableDefsRefresher::TableDefsRefresher():
         client(0),
         state(state_standby),
         action(action_merge)
      { }


      TableDefsRefresher::~TableDefsRefresher()
      { finish(); }


      void TableDefsRefresher::set_action(action_type action_)
      {
         if(state == state_standby)
            action = action_;
         else
            throw exc_invalid_state();
      } // set_action

      
      void TableDefsRefresher::start(
         TableDefsRefresherClient *client_,
         router_handle &router)
      {
         if(state == state_standby)
         {
            if(TableDefsRefresherClient::is_valid_instance(client_))
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


      void TableDefsRefresher::start(
         TableDefsRefresherClient *client_,
         ClientBase *other_component)
      {
         if(state == state_standby)
         {
            if(TableDefsRefresherClient::is_valid_instance(client_))
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


      void TableDefsRefresher::finish()
      {
         client = 0;
         state = state_standby;
         DeviceBase::finish();
      } // finish


      void TableDefsRefresher::describe_outcome(
         std::ostream &out, client_type::outcome_type outcome)
      {
         using namespace TableDefsRefresherStrings;
         switch(outcome)
         {
         case client_type::outcome_success:
            out << common_strings[common_success];
            break;
            
         case client_type::outcome_communication_failed:
            out << common_strings[common_comm_failed];
            break;
            
         case client_type::outcome_communication_disabled:
            out <<  common_strings[common_comm_disabled];
            break;
            
         case client_type::outcome_invalid_table_name:
            out << my_strings[strid_outcome_invalid_table_name];
            break;
            
         case client_type::outcome_logger_security_blocked:
            out << common_strings[common_logger_security_blocked];
            break;
            
         case client_type::outcome_server_security_blocked:
            format_devicebase_failure(out, devicebase_failure_security);
            break;
            
         case client_type::outcome_invalid_device_name:
            format_devicebase_failure(out, devicebase_failure_invalid_device_name);
            break;
            
         case client_type::outcome_unsupported:
            format_devicebase_failure(out, devicebase_failure_unsupported);
            break;
            
         case client_type::outcome_invalid_logon:
            format_devicebase_failure(out, devicebase_failure_logon);
            break;
            
         case client_type::outcome_session_failed:
            format_devicebase_failure(out, devicebase_failure_session);
            break;
            
         case client_type::outcome_in_progress:
            out << my_strings[strid_outcome_in_progress];
            break;
            
         case client_type::outcome_network_locked:
            out << my_strings[strid_outcome_network_locked];
            break;
            
         default:
            format_devicebase_failure(out, devicebase_failure_unknown);
            break;
         }
      } // describe_outcome


      void TableDefsRefresher::on_devicebase_ready()
      {
         if(TableDefsRefresherClient::is_valid_instance(client))
         {
            Csi::Messaging::Message command(
               device_session,
               Messages::logger_table_defs_get_cmd);
            command.addUInt4(++last_tran_no);
            if(interface_version >= Csi::VersionNumber("1.3.4.6"))
               command.addUInt4(action);
            router->sendMessage(&command);
            state = state_active;
         }
         else
            finish();
      } // on_devicebase_ready


      void TableDefsRefresher::on_devicebase_failure(devicebase_failure_type failure)
      {
         using namespace TableDefsRefresherHelpers;
         TableDefsRefresherClient::outcome_type outcome;
         switch(failure)
         {
         case devicebase_failure_logon:
            outcome = TableDefsRefresherClient::outcome_invalid_logon;
            break;

         case devicebase_failure_invalid_device_name:
            outcome = TableDefsRefresherClient::outcome_invalid_device_name;
            break;

         case devicebase_failure_unsupported:
            outcome = TableDefsRefresherClient::outcome_unsupported;
            break;

         case devicebase_failure_security:
            outcome = TableDefsRefresherClient::outcome_server_security_blocked;
            break;

         default:
            outcome = TableDefsRefresherClient::outcome_unknown;
            break;
         }
         event_complete::create_and_post(this,client,outcome);
      } // on_devicebase_failure


      void TableDefsRefresher::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(state == state_active)
         {
            if(msg->getMsgType() == Messages::logger_table_defs_get_ack)
            {
               using namespace TableDefsRefresherHelpers;
               uint4 tran_no;
               uint4 resp_code;
               TableDefsRefresherClient::outcome_type outcome;
               
               msg->readUInt4(tran_no);
               msg->readUInt4(resp_code);
               switch(resp_code)
               {
               case 1:
                  outcome = TableDefsRefresherClient::outcome_success;
                  break;

               case 2:
                  outcome = TableDefsRefresherClient::outcome_in_progress;
                  break;

               case 3:
                  outcome = TableDefsRefresherClient::outcome_logger_security_blocked;
                  break;

               case 4:
                  outcome = TableDefsRefresherClient::outcome_communication_failed;
                  break;

               case 5:
                  outcome = TableDefsRefresherClient::outcome_communication_disabled;
                  break;

               case 7:
                  outcome = TableDefsRefresherClient::outcome_network_locked;
                  break;

               default:
                  outcome = TableDefsRefresherClient::outcome_unknown;
                  break;
               }
               event_complete::create_and_post(this,client,outcome);
            }
            else
               DeviceBase::onNetMessage(rtr,msg);
         }
         else
            DeviceBase::onNetMessage(rtr,msg);
      } // onNetMessage


      void TableDefsRefresher::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace TableDefsRefresherHelpers;
         if(ev->getType() == event_complete::event_id)
         {
            event_complete *event = static_cast<event_complete *>(ev.get_rep());
            if(client == event->client)
            {
               finish();
               if(TableDefsRefresherClient::is_valid_instance(event->client))
                  event->client->on_complete(this,event->outcome);
            }
         }
      } // receive
   };
};

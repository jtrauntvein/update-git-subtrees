/* Cora.Device.ClassicASender.cpp

   Copyright (C) 2008, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 25 January 2008
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.ClassicASender.h"
#include "coratools.strings.h"


namespace Cora
{
   namespace Device
   {
      namespace
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
            typedef ClassicASender sender_type;
            typedef sender_type::client_type client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // outcome
            ////////////////////////////////////////////////////////////
            typedef client_type::outcome_type outcome_type;
            outcome_type outcome;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               sender_type *sender, client_type *client, outcome_type outcome)
            {
               event_complete *event = new event_complete(sender, client, outcome);
               event->post();
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_complete(
               sender_type *sender,
               client_type *client_,
               outcome_type outcome_):
               Event(event_id, sender),
               client(client_),
               outcome(outcome_)
            { } 
         };


         uint4 const event_complete::event_id =
            Csi::Event::registerType("Cora::Device::ClassicASender::event_complete");
      };


      ////////////////////////////////////////////////////////////
      // class ClassicASender definitions
      ////////////////////////////////////////////////////////////
      void ClassicASender::start(
         client_type *client_, router_handle &router)
      {
         if(state != state_standby)
            throw exc_invalid_state();
         if(!client_type::is_valid_instance(client_))
            throw std::invalid_argument("Invalid client pointer");
         client = client_;
         state = state_delegate;
         DeviceBase::start(router);
      } // start


      void ClassicASender::start(
         client_type *client_, ClientBase *other_component)
      {
         if(state != state_standby)
            throw exc_invalid_state();
         if(!client_type::is_valid_instance(client_))
            throw std::invalid_argument("Invalid client pointer");
         client = client_;
         state = state_delegate;
         DeviceBase::start(other_component);
      } // start


      void ClassicASender::finish()
      {
         DeviceBase::finish();
         client = 0;
         state = state_standby;
      } // finish


      void ClassicASender::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         if(ev->getType() == event_complete::event_id)
         {
            event_complete *event = static_cast<event_complete *>(ev.get_rep());
            client_type *client = this->client;
            finish();
            if(event->client == client && client_type::is_valid_instance(client))
               client->on_complete(this, event->outcome);
         }
      } // receive


      void ClassicASender::format_outcome(std::ostream &out, client_type::outcome_type outcome)
      {
         using namespace ClassicASenderStrings;
         switch(outcome)
         {
         case client_type::outcome_success:
            out << my_strings[strid_outcome_success]; 
            break;
            
         case client_type::outcome_invalid_logon:
            format_devicebase_failure(out, devicebase_failure_logon);
            break;
            
         case client_type::outcome_session_failure:
            format_devicebase_failure(out, devicebase_failure_session);
            break;
            
         case client_type::outcome_invalid_device_name:
            format_devicebase_failure(out, devicebase_failure_invalid_device_name); 
            break;
            
         case client_type::outcome_blocked_by_server:
            format_devicebase_failure(out, devicebase_failure_security);
            break;
            
         case client_type::outcome_unsupported:
            format_devicebase_failure(out, devicebase_failure_unsupported);
            break;
            
         case client_type::outcome_comm_failed:
            out << my_strings[strid_outcome_comm_failed];
            break;
            
         case client_type::outcome_comm_disabled:
            out << my_strings[strid_outcome_comm_disabled];
            break;

         default:
            format_devicebase_failure(out, devicebase_failure_unknown);
            break;
         }
      } // format_outcome


      void ClassicASender::on_devicebase_ready()
      {
         Csi::Messaging::Message cmd(device_session, Messages::classic_send_a_cmd);
         cmd.addUInt4(++last_tran_no);
         cmd.addBool(do_reset);
         state = state_active;
         router->sendMessage(&cmd);
      } // on_devicebase_ready


      void ClassicASender::on_devicebase_failure(devicebase_failure_type failure)
      {
         client_type::outcome_type outcome;
         switch(failure)
         {
         case devicebase_failure_logon:
            outcome = client_type::outcome_invalid_logon;
            break;
            
         case devicebase_failure_session:
            outcome = client_type::outcome_session_failure;
            break;
            
         case devicebase_failure_invalid_device_name:
            outcome = client_type::outcome_invalid_device_name;
            break;
            
         case devicebase_failure_unsupported:
            outcome = client_type::outcome_unsupported;
            break;
            
         case devicebase_failure_security:
            outcome = client_type::outcome_blocked_by_server;
            break;
            
         default:
            outcome = client_type::outcome_unknown;
            break;   
         }
         event_complete::cpost(this,client,outcome);
      } // on_devicebase_failure


      void ClassicASender::onNetMessage(
         Csi::Messaging::Router *router, Csi::Messaging::Message *message)
      {
         if(state == state_active)
         {
            if(message->getMsgType() == Messages::classic_send_a_ack)
            {
               uint4 tran_no;
               uint4 rcd;
               client_type::outcome_type outcome = client_type::outcome_unknown;
               
               message->readUInt4(tran_no);
               message->readUInt4(rcd);
               switch(rcd)
               {
               case 1:
                  outcome = client_type::outcome_success;
                  break;
                  
               case 3:
                  outcome = client_type::outcome_comm_failed;
                  break;
                  
               case 4:
                  outcome = client_type::outcome_comm_disabled;
                  break;
               }
               event_complete::cpost(this, client, outcome);
            }
            else
               DeviceBase::onNetMessage(router, message);
         }
         else
            DeviceBase::onNetMessage(router, message);
      } // onNetMessage
   };
};


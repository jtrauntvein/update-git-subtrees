/* Cora.Device.LinkCloser.cpp

   Copyright (C) 2014, 2014 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Thursday 10 April 2014
   Last Change: Friday 18 April 2014
   Last Commit: $Date: 2014-04-18 14:12:59 -0600 (Fri, 18 Apr 2014) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.LinkCloser.h"
#include <iostream>


namespace Cora
{
   namespace Device
   {
      namespace
      {
         class complete_event: public Csi::Event
         {
         public:
            static uint4 const event_id;
            LinkCloserClient::outcome_type outcome;

            static void cpost(LinkCloser *closer, LinkCloserClient::outcome_type outcome)
            {
               complete_event *event(new complete_event(closer, outcome));
               event->post();
            }

         private:
            complete_event(LinkCloser *closer, LinkCloserClient::outcome_type outcome_):
               Event(event_id, closer),
               outcome(outcome_)
            { }
         };


         uint4 const complete_event::event_id(
            Csi::Event::registerType("Cora::Device::LinkCloser::complete_event"));
      };


      void LinkCloser::format_outcome(
         std::ostream &out, client_type::outcome_type outcome)
      {
         switch(outcome)
         {
         case client_type::outcome_failure_unknown:
         default:
            format_devicebase_failure(out, devicebase_failure_unknown);
            break;
            
         case client_type::outcome_success:
            out << "success";
            break;
            
         case client_type::outcome_failure_logon:
            format_devicebase_failure(out, devicebase_failure_logon);
            break;
            
         case client_type::outcome_failure_session:
            format_devicebase_failure(out, devicebase_failure_session);
            break;
            
         case client_type::outcome_failure_unsupported:
            format_devicebase_failure(out, devicebase_failure_unsupported);
            break;
            
         case client_type::outcome_failure_security:
            format_devicebase_failure(out, devicebase_failure_security);
            break;
            
         case client_type::outcome_failure_invalid_device_name:
            format_devicebase_failure(out, devicebase_failure_invalid_device_name);
            break;
         }
      } // format_outcome


      void LinkCloser::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         if(ev->getType() == complete_event::event_id)
         {
            complete_event *event(static_cast<complete_event *>(ev.get_rep()));
            client_type *report = client;
            finish();
            if(client_type::is_valid_instance(report))
               report->on_complete(this, event->outcome);
         }
      } // receive


      void LinkCloser::on_devicebase_ready()
      {
         Csi::Messaging::Message cmd(device_session, Messages::force_link_offline_cmd);
         cmd.addUInt4(++last_tran_no);
         cmd.addBool(recursive);
         state = state_active;
         router->sendMessage(&cmd);
      } // on_devicebase_ready


      void LinkCloser::on_devicebase_failure(devicebase_failure_type failure)
      {
         client_type::outcome_type outcome;
         switch(failure)
         {
         case devicebase_failure_logon:
            outcome = client_type::outcome_failure_logon;
            break;

         case devicebase_failure_session:
            outcome = client_type::outcome_failure_session;
            break;

         case devicebase_failure_invalid_device_name:
            outcome = client_type::outcome_failure_invalid_device_name;
            break;

         case devicebase_failure_unsupported:
            outcome = client_type::outcome_failure_unsupported;
            break;

         case devicebase_failure_security:
            outcome = client_type::outcome_failure_security;
            break;

         default:
            outcome = client_type::outcome_failure_unknown;
            break;
         }
         complete_event::cpost(this, outcome);
      } // on_devicebase_failure


      void LinkCloser::onNetMessage(
         Csi::Messaging::Router *router, Csi::Messaging::Message *message)
      {
         if(state == state_active)
         {
            if(message->getMsgType() == Messages::force_link_offline_ack)
            {
               uint4 resp_code;
               message->movePast(4); // skip tran no
               message->readUInt4(resp_code);
               if(resp_code == 1)
                  complete_event::cpost(this, client_type::outcome_success);
               else
                  complete_event::cpost(this, client_type::outcome_failure_unknown);
            }
            else
               DeviceBase::onNetMessage(router, message);
         }
         else
            DeviceBase::onNetMessage(router, message);
      } // onNetMessage
   };
};


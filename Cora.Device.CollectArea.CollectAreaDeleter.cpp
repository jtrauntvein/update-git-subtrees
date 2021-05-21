/* Cora.Device.CollectArea.CollectAreaDeleter.cpp

   Copyright (C) 2016, 2021 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Wednesday 14 September 2016
   Last Change: Tuesday 06 April 2021
   Last Commit: $Date: 2021-04-06 16:11:35 -0600 (Tue, 06 Apr 2021) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.CollectArea.CollectAreaDeleter.h"
#include <iostream>


namespace Cora
{
   namespace Device
   {
      namespace CollectArea
      {
         namespace
         {
            class event_complete: public Csi::Event
            {
            public:
               /**
                * Specifies the unique identifier for this event.
                */
               static uint4 const event_id;

               /**
                * Specifies the outcome.
                */
               CollectAreaDeleterClient::outcome_type const outcome;

               /**
                * Creates an posts this event.
                */
               static void cpost(CollectAreaDeleter *deleter, CollectAreaDeleterClient::outcome_type outcome)
               { return (new event_complete(deleter, outcome))->post(); }

            private:
               event_complete(CollectAreaDeleter *deleter, CollectAreaDeleterClient::outcome_type outcome_):
                  Event(event_id, deleter),
                  outcome(outcome_)
               { }
            };


            uint4 const event_complete::event_id(
               Csi::Event::registerType("Cora::Device::CollectArea::ColletAreaDeleter::event_complete"));
         };


         void CollectAreaDeleter::on_devicebase_ready()
         {
            Csi::Messaging::Message cmd(device_session, Messages::collect_area_delete_cmd);
            cmd.addUInt4(++last_tran_no);
            cmd.addWStr(area_name);
            state = state_active;
            router->sendMessage(&cmd);
         } // on_devicebase_ready


         void CollectAreaDeleter::on_devicebase_failure(devicebase_failure_type failure)
         {
            client_type::outcome_type outcome(client_type::outcome_failure_unknown);
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
            }
            event_complete::cpost(this, outcome);
         } // on_devicebase_failure


         void CollectAreaDeleter::onNetMessage(Csi::Messaging::Router *router, Csi::Messaging::Message *message)
         {
            if(state == state_active && message->getMsgType() == Messages::collect_area_delete_ack)
            {
               uint4 tran_no;
               uint4 response;
               client_type::outcome_type outcome(client_type::outcome_failure_unknown);
               
               message->readUInt4(tran_no);
               message->readUInt4(response);
               switch(response)
               {
               case 1:
                  outcome = client_type::outcome_success;
                  break;

               case 2:
                  outcome = client_type::outcome_failure_invalid_area_name;
                  break;
               }
               event_complete::cpost(this, outcome);
            }
            else
               DeviceBase::onNetMessage(router, message);
         } // onNetMessage


         void CollectAreaDeleter::receive(Csi::SharedPtr<Csi::Event> &ev)
         {
            if(ev->getType() == event_complete::event_id)
            {
               event_complete *event(static_cast<event_complete *>(ev.get_rep()));
               client_type *report(client);
               finish();
               if(client_type::is_valid_instance(report))
                  report->on_complete(this, event->outcome);
            }
         } // receive

         void CollectAreaDeleter::format_outcome(std::ostream &out, client_type::outcome_type outcome)
         {
            switch(outcome)
            {
            case client_type::outcome_success:
               out << "success";
               break;
               
            case client_type::outcome_failure_logon:
               format_devicebase_failure(out, devicebase_failure_logon);
               break;

            case client_type::outcome_failure_session:
               format_devicebase_failure(out, devicebase_failure_session);
               break;

            case client_type::outcome_failure_security:
               format_devicebase_failure(out, devicebase_failure_security);
               break;

            case client_type::outcome_failure_unsupported:
               format_devicebase_failure(out, devicebase_failure_unsupported);
               break;

            case client_type::outcome_failure_invalid_device_name:
               format_devicebase_failure(out, devicebase_failure_invalid_device_name);
               break;

            case client_type::outcome_failure_invalid_area_name:
               out << "invalid collect area name or collect area cannot be deleted";
               break;
               
            default:
               format_devicebase_failure(out, devicebase_failure_unknown);
               break;
            }
         }
      };
   };
};


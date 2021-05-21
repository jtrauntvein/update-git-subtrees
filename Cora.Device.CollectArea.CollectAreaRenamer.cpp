/* Cora.Device.CollectArea.CollectAreaRenamer.cpp

   Copyright (C) 2016, Campbell Scientific, Inc

   Written by: Jon Trauntvein 
   Date Begun: Friday 09 September 2016
   Last Change: Friday 09 September 2016
   Last Commit: $Date: 2016-09-12 11:05:12 -0600 (Mon, 12 Sep 2016) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.CollectArea.CollectAreaRenamer.h"


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
                * Specifies the event ID.
                */
               static uint4 const event_id;

               /**
                * Specifies the outcome.
                */
               CollectAreaRenamer::client_type::outcome_type outcome;

               /**
                * Creates and posts this event to the specified component.
                */
               static void cpost(CollectAreaRenamer *dest, CollectAreaRenamer::client_type::outcome_type outcome)
               {
                  (new event_complete(dest, outcome))->post();
               }

            private:
               /**
                * Constructor
                */
               event_complete(CollectAreaRenamer *dest, CollectAreaRenamer::client_type::outcome_type outcome_):
                  Event(event_id, dest),
                  outcome(outcome_)
               { }
            };


            uint4 const event_complete::event_id(
               Csi::Event::registerType("Cora::Device::CollectArea::CollectAreaRenamer::event_complete"));
         };


         void CollectAreaRenamer::on_devicebase_ready()
         {
            Csi::Messaging::Message command(device_session, Messages::collect_area_rename_cmd);
            command.addUInt4(++last_tran_no);
            command.addWStr(area_name);
            command.addWStr(new_name);
            state = state_active;
            router->sendMessage(&command);
         } // on_devicebase_ready


         void CollectAreaRenamer::on_devicebase_failure(devicebase_failure_type failure)
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


         void CollectAreaRenamer::onNetMessage(Csi::Messaging::Router *router, Csi::Messaging::Message *message)
         {
            if(state == state_active && message->getMsgType() == Messages::collect_area_rename_ack)
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

               case 3:
                  outcome = client_type::outcome_failure_invalid_new_area_name;
                  break;
               }
               event_complete::cpost(this, outcome);
            }
            else
               DeviceBase::onNetMessage(router, message);
         } // onNetMessage


         void CollectAreaRenamer::receive(Csi::SharedPtr<Csi::Event> &ev)
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
      };
   };
};


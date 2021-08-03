/* Cora.Device.AlohaStationAreaCreator.cpp

   Copyright (C) 2021, Campbell Scientific, Inc

   Written by: Jon Trauntvein 
   Date Begun: Tuesday 12 January 2021
   Last Change: Tuesday 12 January 2021
   Last Commit: $Date: 2021-01-12 15:28:09 -0600 (Tue, 12 Jan 2021) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.AlohaStationAreaCreator.h"
#include <iostream>


namespace Cora
{
   namespace Device
   {
      namespace
      {
         class event_complete: public Csi::Event
         {
         public:
            static uint4 const event_id;
            typedef AlohaStationAreaCreatorClient::outcome_type outcome_type;
            outcome_type const outcome;

            static void cpost(AlohaStationAreaCreator *sender, outcome_type outcome)
            { (new event_complete(sender, outcome))->post(); }

         private:
            event_complete(AlohaStationAreaCreator *sender, outcome_type outcome_):
               Event(event_id, sender),
               outcome(outcome_)
            { }
         };
         uint4 const event_complete::event_id(
            Csi::Event::registerType("Cora::Device::AlohaStationAreaCreator::event_complete"));
      };

      void AlohaStationAreaCreator::receive(event_handle &ev)
      {
         if(ev->getType() == event_complete::event_id)
         {
            event_complete *event((event_complete *)ev.get_rep());
            client_type *report(client);
            finish();
            if(client_type::is_valid_instance(report))
               report->on_complete(this, event->outcome);
         }
      } // receive

      void AlohaStationAreaCreator::format_outcome(
         std::ostream &out, client_type::outcome_type outcome)
      {
         switch(outcome)
         {
         case client_type::outcome_success:
            out << "success";
            break;

         case client_type::outcome_failure_session:
            format_devicebase_failure(out, devicebase_failure_session);
            break;

         case client_type::outcome_failure_invalid_device_name:
            format_devicebase_failure(out, devicebase_failure_invalid_device_name);
            break;

         case client_type::outcome_failure_logon:
            format_devicebase_failure(out, devicebase_failure_logon);
            break;

         case client_type::outcome_failure_security:
            format_devicebase_failure(out, devicebase_failure_security);
            break;

         case client_type::outcome_failure_unsupported:
            format_devicebase_failure(out, devicebase_failure_unsupported);
            break;

         case client_type::outcome_failure_invalid_station_id:
            out << "invalid station identifier";
            break;
            
         default:
            format_devicebase_failure(out, devicebase_failure_unknown);
            break;
         }
      } // format_outcome

      void AlohaStationAreaCreator::on_devicebase_ready()
      {
         Csi::Messaging::Message cmd(device_session, Messages::create_aloha_station_area_cmd);
         cmd.addUInt4(++last_tran_no);
         cmd.addUInt4(station_id);
         cmd.addWStr(station_name);
         state = state_active;
         router->sendMessage(&cmd);
      } // on_devicebase_ready

      void AlohaStationAreaCreator::on_devicebase_failure(devicebase_failure_type failure)
      {
         client_type::outcome_type outcome(client_type::outcome_failure_unknown);
         switch(failure)
         {
         case devicebase_failure_invalid_device_name:
            outcome = client_type::outcome_failure_invalid_device_name;
            break;

         case devicebase_failure_session:
            outcome = client_type::outcome_failure_session;
            break;

         case devicebase_failure_logon:
            outcome = client_type::outcome_failure_logon;
            break;

         case devicebase_failure_security:
            outcome = client_type::outcome_failure_security;
            break;

         case devicebase_failure_unsupported:
            outcome = client_type::outcome_failure_unsupported;
            break;
         }
         event_complete::cpost(this, outcome);
      } // on_devicebase_failure

      void AlohaStationAreaCreator::onNetMessage(
         Csi::Messaging::Router *router, Csi::Messaging::Message *message)
      {
         if(state == state_active)
         {
            if(message->getMsgType() == Messages::create_aloha_station_area_ack)
            {
               client_type::outcome_type outcome(client_type::outcome_failure_unknown);
               uint4 tran_no, response;
               message->readUInt4(tran_no);
               message->readUInt4(response);
               switch(response)
               {
               case 1:
                  outcome = client_type::outcome_success;
                  break;

               case 3:
                  outcome = client_type::outcome_failure_invalid_station_id;
                  break;
               }
               event_complete::cpost(this, outcome);
            }
            else
               DeviceBase::onNetMessage(router, message);
         }
         else
            DeviceBase::onNetMessage(router, message);
      } // onNetMessage
   };
};


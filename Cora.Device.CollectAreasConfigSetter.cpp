/* Cora.Device.CollectAreasConfigSetter.cpp

   Copyright (C) 2020, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Tuesday 31 March 2020
   Last Change: Thursday 02 April 2020
   Last Commit: $Date: 2020-04-02 09:29:23 -0600 (Thu, 02 Apr 2020) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.CollectAreasConfigSetter.h"
#include "coratools.strings.h"
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
            typedef CollectAreasConfigSetterClient::outcome_type outcome_type;
            outcome_type const outcome;

            static void cpost(CollectAreasConfigSetter *sender, outcome_type outcome)
            { (new event_complete(sender, outcome))->post(); }

         private:
            event_complete(CollectAreasConfigSetter *sender, outcome_type outcome_):
               Event(event_id, sender),
               outcome(outcome_)
            { }
         };


         uint4 const event_complete::event_id(
            Csi::Event::registerType("Cora::Device::CollectAreasConfigSetter::complete"));
      };


      void CollectAreasConfigSetter::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         if(ev->getType() == event_complete::event_id)
         {
            client_type *report(client);
            event_complete *event((event_complete *)ev.get_rep());
            finish();
            if(client_type::is_valid_instance(report))
               report->on_complete(this, event->outcome);
         }
      } // receive


      void CollectAreasConfigSetter::describe_outcome(std::ostream &out, client_type::outcome_type outcome)
      {
         switch(outcome)
         {
         case client_type::outcome_success:
            out << common_strings[common_success];
            break;
            
         case client_type::outcome_failure_invalid_device_name:
            format_devicebase_failure(out, devicebase_failure_invalid_device_name);
            break;
            
         case client_type::outcome_failure_security:
            format_devicebase_failure(out, devicebase_failure_security);
            break;
            
         case client_type::outcome_failure_unsupported:
            format_devicebase_failure(out, devicebase_failure_unsupported);
            break;
            
         case client_type::outcome_failure_session:
            format_devicebase_failure(out, devicebase_failure_session);
            break;
            
         case client_type::outcome_failure_logon:
            format_devicebase_failure(out, devicebase_failure_logon);
            break;

         case client_type::outcome_failure_network_locked:
            out << common_strings[common_network_locked];
            break;

         case client_type::outcome_failure_invalid_device_type:
            out << "configuration device type does not match";
            break;
            
         default:
            format_devicebase_failure(out, devicebase_failure_unknown);
            break;
         }
      } // describe_outcome


      void CollectAreasConfigSetter::on_devicebase_ready()
      {
         Csi::Messaging::Message command(device_session, Messages::set_collect_areas_config_cmd);
         command.addUInt4(++last_tran_no);
         command.addStr(config);
         state = state_active;
         router->sendMessage(&command);
      } // on_devicebase_ready


      void CollectAreasConfigSetter::on_devicebase_failure(devicebase_failure_type failure)
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


      void CollectAreasConfigSetter::onNetMessage(
         Csi::Messaging::Router *router, Csi::Messaging::Message *message)
      {
         if(state == state_active)
         {
            if(message->getMsgType() == Messages::set_collect_areas_config_ack)
            {
               uint4 tran_no, rcd;
               client_type::outcome_type outcome(client_type::outcome_failure_unknown);
               
               message->readUInt4(tran_no);
               message->readUInt4(rcd);
               switch(rcd)
               {
               case 1:
                  outcome = client_type::outcome_success;
                  break;

               case 3:
                  outcome = client_type::outcome_failure_invalid_config;
                  break;

               case 4:
                  outcome = client_type::outcome_failure_network_locked;
                  break;

               case 5:
                  outcome = client_type::outcome_failure_invalid_device_type;
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



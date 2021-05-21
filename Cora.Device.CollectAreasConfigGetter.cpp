/* Cora.Device.CollectAreasConfigGetter.cpp

   Copyright (C) 2020, Campbell Scientific, Inc

   Written by: Jon Trauntvein 
   Date Begun: Tuesday 31 March 2020
   Last Change: Tuesday 31 March 2020
   Last Commit: $Date: 2020-03-31 15:20:40 -0600 (Tue, 31 Mar 2020) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.CollectAreasConfigGetter.h"
#include "coratools.strings.h"
#include "Csi.BuffStream.h"


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
            typedef CollectAreasConfigGetterClient client_type;
            typedef client_type::outcome_type outcome_type;
            outcome_type const outcome;
            typedef Csi::Xml::Element::value_type config_handle;
            config_handle config;

            static void cpost(
               CollectAreasConfigGetter *sender, outcome_type outcome, config_handle config = 0)
            { (new event_complete(sender, outcome, config))->post(); }

         private:
            event_complete(
               CollectAreasConfigGetter *sender, outcome_type outcome_, config_handle &config_):
               Event(event_id, sender),
               outcome(outcome_),
               config(config_)
            { }
         };


         uint4 const event_complete::event_id(
            Csi::Event::registerType("Cora::Device::CollectAreasConfigGetter::event_complete"));
      };


      void CollectAreasConfigGetter::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         if(ev->getType() == event_complete::event_id)
         {
            client_type *report(client);
            event_complete *event((event_complete *)ev.get_rep());
            
            finish();
            if(client_type::is_valid_instance(report))
               report->on_complete(this, event->outcome, event->config);
         }
      } // receive


      void CollectAreasConfigGetter::describe_outcome(std::ostream &out, client_type::outcome_type outcome)
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
            
         default:
            format_devicebase_failure(out, devicebase_failure_unknown);
            break;
         }
      } // describe_outcome


      void CollectAreasConfigGetter::on_devicebase_ready()
      {
         Csi::Messaging::Message command(device_session, Messages::get_collect_areas_config_cmd);
         command.addUInt4(++last_tran_no);
         state = state_active;
         router->sendMessage(&command);
      } // on_devicebase_ready


      void CollectAreasConfigGetter::on_devicebase_failure(devicebase_failure_type failure)
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


      void CollectAreasConfigGetter::onNetMessage(
         Csi::Messaging::Router *router, Csi::Messaging::Message *message)
      {
         if(state == state_active)
         {
            if(message->getMsgType() == Messages::get_collect_areas_config_ack)
            {
               Csi::Xml::Element::value_type config;
               client_type::outcome_type outcome(client_type::outcome_failure_unknown);
               uint4 tran_no, rcd;
               message->readUInt4(tran_no);
               message->readUInt4(rcd);
               if(rcd == 1)
               {
                  try
                  {
                     StrAsc config_str;
                     message->readStr(config_str);
                     Csi::IBuffStream input(config_str.c_str(), config_str.length());
                     config.bind(new Csi::Xml::Element(L""));
                     config->input(input);
                     outcome = client_type::outcome_success;
                  }
                  catch(std::exception &)
                  { config.clear(); }
               }
               event_complete::cpost(this, outcome, config);
            }
            else
               DeviceBase::onNetMessage(router, message);
         }
         else
            DeviceBase::onNetMessage(router, message);
      } // onNetMessage
   };
};

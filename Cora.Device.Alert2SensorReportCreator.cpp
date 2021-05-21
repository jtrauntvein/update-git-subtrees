/* Cora.Device.Alert2SensorReportCreator.cpp

   Copyright (C) 2016, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Thursday 14 July 2016
   Last Change: Monday 31 October 2016
   Last Commit: $Date: 2016-10-31 13:55:51 -0600 (Mon, 31 Oct 2016) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.Alert2SensorReportCreator.h"
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

            Alert2SensorReportCreatorClient::outcome_type const outcome;

            static void cpost(Alert2SensorReportCreator *sender, Alert2SensorReportCreatorClient::outcome_type outcome)
            {
               (new complete_event(sender, outcome))->post();
            }

         private:
            complete_event(Alert2SensorReportCreator *sender, Alert2SensorReportCreatorClient::outcome_type outcome_):
               Event(event_id, sender),
               outcome(outcome_)
            { }
         };


         uint4 const complete_event::event_id(Csi::Event::registerType("Cora::Device::Alert2SensorReportCreator::event_complete"));
      };


      void Alert2SensorReportCreator::on_devicebase_ready()
      {
         Csi::Messaging::Message command(device_session, Messages::create_alert2_sensor_report_cmd);
         command.addUInt4(++last_tran_no);
         command.addWStr(report_name);
         command.addUInt2(sensor_id);
         state = state_active;
         router->sendMessage(&command);
      } // on_devicebase_ready


      void Alert2SensorReportCreator::on_devicebase_failure(devicebase_failure_type failure)
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
         complete_event::cpost(this, outcome);
      } // on_devicebase_failure


      void Alert2SensorReportCreator::onNetMessage(
         Csi::Messaging::Router *router, Csi::Messaging::Message *message)
      {
         if(state == state_active)
         {
            if(message->getMsgType() == Messages::create_alert2_sensor_report_ack)
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
                  outcome = client_type::outcome_failure_invalid_report_name;
                  break;
               }
               complete_event::cpost(this, outcome);
            }
            else
               DeviceBase::onNetMessage(router, message);
         }
         else
            DeviceBase::onNetMessage(router, message);
      } // onNetMessage


      void Alert2SensorReportCreator::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         if(ev->getType() == complete_event::event_id)
         {
            complete_event *event(static_cast<complete_event *>(ev.get_rep()));
            client_type *report(client);
            finish();
            if(client_type::is_valid_instance(report))
               report->on_complete(this, event->outcome);
         }
      } // receive


      void Alert2SensorReportCreator::format_outcome(
         std::ostream &out, client_type::outcome_type outcome)
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
            
         case client_type::outcome_failure_invalid_report_name:
            out << "invalid report area name";
            break;
            
         default:
            format_devicebase_failure(out, devicebase_failure_unknown);
            break;
         }
      } // format_outcome
   };
};




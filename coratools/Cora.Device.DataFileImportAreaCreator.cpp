/* Cora.Device.DataFileImportAreaCreator.cpp

   Copyright (C) 2019, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Wednesday 03 July 2019
   Last Change: Friday 26 July 2019
   Last Commit: $Date: 2019-07-26 15:46:36 -0600 (Fri, 26 Jul 2019) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.DataFileImportAreaCreator.h"
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

            DataFileImportAreaCreatorClient::outcome_type const outcome;

            static void cpost(DataFileImportAreaCreator *sender, DataFileImportAreaCreatorClient::outcome_type outcome)
            { (new event_complete(sender, outcome))->post(); }

         private:
            event_complete(DataFileImportAreaCreator *sender, DataFileImportAreaCreatorClient::outcome_type outcome_):
               Event(event_id, sender),
               outcome(outcome_)
            { }
         };


         uint4 const event_complete::event_id(
            Csi::Event::registerType("Cora::Device::DataFileImportAreaCreator::event_complete"));
      };

      
      void DataFileImportAreaCreator::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         if(ev->getType() == event_complete::event_id)
         {
            event_complete *event = (event_complete *)ev.get_rep();
            client_type *report(client);
            finish();
            if(client_type::is_valid_instance(report))
               report->on_complete(this, event->outcome);
         }
      } // receive


      void DataFileImportAreaCreator::describe_outcome(
         std::ostream &out, client_type::outcome_type outcome)
      {
         switch(outcome)
         {
         case client_type::outcome_success:
            out << "success";
            break;
            
         case client_type::outcome_failure_invalid_device_name:
            format_devicebase_failure(out, devicebase_failure_invalid_device_name);
            break;
            
         case client_type::outcome_failure_server_blocked:
            format_devicebase_failure(out, devicebase_failure_security);
            break;
            
         case client_type::outcome_failure_unsupported:
            format_devicebase_failure(out, devicebase_failure_unsupported);
            break;
            
         case client_type::outcome_failure_session:
            format_devicebase_failure(out, devicebase_failure_session);
            break;
            
         case client_type::outcome_failure_invalid_area_name:
            out << "invalid collect area name";
            break;
            
         case client_type::outcome_failure_logon:
            format_devicebase_failure(out, devicebase_failure_logon);
            break;
            
         default:
            format_devicebase_failure(out, devicebase_failure_unknown);
            break;
         }
      } // describe_outcome

      
      void DataFileImportAreaCreator::on_devicebase_ready()
      {
         Csi::Messaging::Message command(device_session, Messages::create_import_area_cmd);
         command.addUInt4(++last_tran_no);
         command.addWStr(area_name);
         state = state_active;
         router->sendMessage(&command);
      } // on_devicebase_ready


      void DataFileImportAreaCreator::on_devicebase_failure(devicebase_failure_type failure)
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
            outcome = client_type::outcome_failure_server_blocked;
            break;
         }
         event_complete::cpost(this, outcome);
      } // on_devicebase_failure


      void DataFileImportAreaCreator::onNetMessage(
         Csi::Messaging::Router *router, Csi::Messaging::Message *message)
      {
         if(state == state_active)
         {
            if(message->getMsgType() == Messages::create_import_area_ack)
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

               case 3:
                  outcome = client_type::outcome_failure_invalid_area_name;
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


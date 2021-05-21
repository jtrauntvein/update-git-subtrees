/* Cora.Device.FslGetter.cpp

   Copyright (C) 2020, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Friday 23 October 2020
   Last Change: Saturday 24 October 2020
   Last Commit: $Date: 2020-10-24 08:00:58 -0600 (Sat, 24 Oct 2020) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.FslGetter.h"
#include <ostream>


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
            FslGetterClient::outcome_type const outcome;
            StrAsc const labels;

            static void cpost(
               FslGetter *sender, FslGetterClient::outcome_type outcome, StrAsc const &labels = "")
            { (new event_complete(sender, outcome, labels))->post(); }

         private:
            event_complete(
               FslGetter *sender, FslGetterClient::outcome_type outcome_, StrAsc const &labels_):
               Event(event_id, sender),
               outcome(outcome_),
               labels(labels_)
            { }
         };
         uint4 const event_complete::event_id(
            Csi::Event::registerType("Cora::Device::FslGetter::event_complete"));
      };


      void FslGetter::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         if(ev->getType() == event_complete::event_id)
         {
            event_complete *event((event_complete *)ev.get_rep());
            client_type *receipt(client);
            finish();
            if(client_type::is_valid_instance(receipt))
               receipt->on_complete(this, event->outcome, event->labels);
         }
      } // receive

      void FslGetter::format_outcome(std::ostream &out, client_type::outcome_type outcome)
      {
         switch(outcome)
         {
         case client_type::outcome_failure_unknown:
         default:
            DeviceBase::format_devicebase_failure(out, devicebase_failure_unknown);
            break;

         case client_type::outcome_success:
            out << "success";
            break;

         case client_type::outcome_failure_logon:
            DeviceBase::format_devicebase_failure(out, devicebase_failure_logon);
            break;

         case client_type::outcome_failure_session:
            DeviceBase::format_devicebase_failure(out, devicebase_failure_session);
            break;

         case client_type::outcome_failure_invalid_device_name:
            DeviceBase::format_devicebase_failure(out, devicebase_failure_invalid_device_name);
            break;

         case client_type::outcome_failure_unsupported:
            DeviceBase::format_devicebase_failure(out, devicebase_failure_unsupported);
            break;

         case client_type::outcome_failure_security:
            DeviceBase::format_devicebase_failure(out, devicebase_failure_security);
            break;

         case client_type::outcome_failure_no_labels:
            out << "no labels";
            break;

         case client_type::outcome_failure_invalid_format:
            out << "unsupported output format";
            break;
         }
      } // format_outcome

      void FslGetter::on_devicebase_ready()
      {
         Csi::Messaging::Message cmd(device_session, Messages::get_final_storage_labels_cmd);
         cmd.addUInt4(++last_tran_no);
         cmd.addUInt4(output_format);
         state = state_active;
         router->sendMessage(&cmd);
      } // on_devicebase_ready

      void FslGetter::on_devicebase_failure(devicebase_failure_type failure)
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

      void FslGetter::onNetMessage(
         Csi::Messaging::Router *router, Csi::Messaging::Message *message)
      {
         if(state == state_active)
         {
            if(message->getMsgType() == Messages::get_final_storage_labels_ack)
            {
               uint4 tran_no;
               uint4 response;
               StrAsc labels;
               client_type::outcome_type outcome(client_type::outcome_failure_unknown);
               message->readUInt4(tran_no);
               message->readUInt4(response);
               switch(response)
               {
               case 1:
                  message->readStr(labels);
                  outcome = client_type::outcome_success;
                  break;

               case 3:
                  outcome = client_type::outcome_failure_no_labels;
                  break;

               case 4:
                  outcome = client_type::outcome_failure_invalid_format;
                  break;
               }
               event_complete::cpost(this, outcome, labels);
            }
            else
               DeviceBase::onNetMessage(router, message);
         }
         else
            DeviceBase::onNetMessage(router, message);
      } // onNetMessage
   };
};

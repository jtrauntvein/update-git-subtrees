/* Cora.Device.AlohaMessagesLogQuery.cpp

   Copyright (C) 2021, 2021 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Saturday 06 February 2021
   Last Change: Wednesday 10 February 2021
   Last Commit: $Date: 2021-02-11 09:18:20 -0600 (Thu, 11 Feb 2021) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.AlohaMessagesLogQuery.h"
#include <iostream>


namespace Cora
{
   namespace Device
   {
      namespace
      {
         class event_started: public Csi::Event
         {
         public:
            static uint4 const event_id;

            static void cpost(AlohaMessagesLogQuery *sender)
            { (new event_started(sender))->post(); }

         private:
            event_started(AlohaMessagesLogQuery *sender):
               Event(event_id, sender)
            { }
         };
         uint4 const event_started::event_id(
            Csi::Event::registerType("Cora::Device::AlohaMessagesLogQuery::event_started"));


         class event_messages: public Csi::Event
         {
         public:
            static uint4 const event_id;

            static void cpost(AlohaMessagesLogQuery *sender)
            { (new event_messages(sender))->post(); }

         private:
            event_messages(AlohaMessagesLogQuery *sender):
               Event(event_id, sender)
            { }
         };
         uint4 const event_messages::event_id(
            Csi::Event::registerType("Cora::Device::AlohaMessagesLogQuery::event_messages"));


         class event_complete: public Csi::Event
         {
         public:
            static uint4 const event_id;
            typedef AlohaMessagesLogQueryClient::outcome_type outcome_type;
            outcome_type const outcome;

            static void cpost(AlohaMessagesLogQuery *sender, outcome_type outcome)
            { (new event_complete(sender, outcome))->post(); }

         private:
            event_complete(AlohaMessagesLogQuery *sender, outcome_type outcome_):
               Event(event_id, sender),
               outcome(outcome_)
            { }
         };
         uint4 const event_complete::event_id(
            Csi::Event::registerType("Cora::Device::AlohaMessagesLogQuery::event_complete"));
      };


      void AlohaMessagesLogQuery::describe_outcome(
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

         case client_type::outcome_failure_logon:
            format_devicebase_failure(out, devicebase_failure_logon);
            break;

         case client_type::outcome_failure_invalid_device_name:
            format_devicebase_failure(out, devicebase_failure_invalid_device_name);
            break;

         case client_type::outcome_failure_unsupported:
            format_devicebase_failure(out, devicebase_failure_unsupported);
            break;

         case client_type::outcome_failure_security:
            format_devicebase_failure(out, devicebase_failure_security);
            break;

         case client_type::outcome_failure_logging_disabled:
            out << "logging is disabled";
            break;

         case client_type::outcome_failure_invalid_time_range:
            out << "invalid message time range was specified";
            break;

         case client_type::outcome_failure_invalid_predicate:
            out << "invalid predicate expression was specified";
            break;

         case client_type::outcome_failure_aborted:
            out << "aborted";
            break;
            
         default:
            format_devicebase_failure(out, devicebase_failure_unknown);
            break;
         }
      } // describe_outcome

      void AlohaMessagesLogQuery::receive(event_handle &ev)
      {
         if(ev->getType() == event_started::event_id)
         {
            if(client_type::is_valid_instance(client))
               client->on_started(this);
            else
               finish();
         }
         else if(ev->getType() == event_complete::event_id)
         {
            auto event(static_cast<event_complete *>(ev.get_rep()));
            client_type *report(client);
            finish();
            if(client_type::is_valid_instance(report))
               report->on_complete(this, event->outcome);
         }
         else if(ev->getType() == event_messages::event_id)
         {
            if(client_type::is_valid_instance(client))
            {
               if(client->on_messages(this, messages))
                  send_ack();
            }
            else
               finish();
         }
      } // receive

      void AlohaMessagesLogQuery::send_ack(uint4 sequence_no)
      {
         if(state == state_needs_ack)
         {
            Csi::Messaging::Message ack(device_session, Messages::query_aloha_messages_ack);
            ack.addUInt4(tran_no);
            ack.addBool(false);
            ack.addUInt4(sequence_no == 0xffffffff ? last_sequence_no : sequence_no);
            state = state_active;
            messages.clear();
            router->sendMessage(&ack);
         }
      } // send_ack

      void AlohaMessagesLogQuery::on_devicebase_ready()
      {
         Csi::Messaging::Message start_cmd(device_session, Messages::query_aloha_messages_start_cmd);
         tran_no = ++last_tran_no;
         start_cmd.addUInt4(tran_no);
         start_cmd.addInt8(begin_stamp.get_nanoSec());
         start_cmd.addInt8(end_stamp.get_nanoSec());
         if(predicate.length())
            start_cmd.addStr(predicate);
         state = state_active;
         router->sendMessage(&start_cmd);
      } // on_devicebase_ready
      
      void AlohaMessagesLogQuery::on_devicebase_failure(devicebase_failure_type failure)
      {
         client_type::outcome_type outcome(client_type::outcome_failure_unknown);
         switch(failure)
         {
         case devicebase_failure_session:
            outcome = client_type::outcome_failure_session;
            break;

         case devicebase_failure_logon:
            outcome = client_type::outcome_failure_logon;
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

      void AlohaMessagesLogQuery::onNetMessage(
         Csi::Messaging::Router *router, Csi::Messaging::Message *message)
      {
         if(state >= state_active)
         {
            uint4 const message_type(message->getMsgType());
            if(message_type == Messages::query_aloha_messages_start_ack)
            {
               uint4 tran, rcd;
               message->readUInt4(tran);
               message->readUInt4(rcd);
               if(rcd == 1)
                  event_started::cpost(this);
               else
               {
                  client_type::outcome_type outcome(client_type::outcome_failure_unknown);
                  switch(rcd)
                  {
                  case 3:
                     outcome = client_type::outcome_failure_logging_disabled;
                     break;

                  case 4:
                     outcome = client_type::outcome_failure_invalid_time_range;
                     break;

                  case 5:
                     outcome = client_type::outcome_failure_invalid_predicate;
                     break;
                  }
                  event_complete::cpost(this, outcome);
               }
            }
            else if(message_type == Messages::query_aloha_messages_not)
            {
               uint4 tran, status;
               message->readUInt4(tran);
               message->readUInt4(status);
               if(status == 1)
               {
                  uint4 count;
                  int8 nsec;
                  StrAsc temp;
                  
                  message->readUInt4(last_sequence_no);
                  message->readUInt4(count);
                  for(uint4 i = 0; i < count; ++i)
                  {
                     message->readInt8(nsec);
                     message->readStr(temp);
                     messages.push_back(client_type::message_type(nsec, temp));
                  }
                  if(state == state_active)
                  {
                     state = state_needs_ack;
                     event_messages::cpost(this);
                  }
               }
               else
               {
                  client_type::outcome_type outcome(client_type::outcome_failure_unknown);
                  switch(status)
                  {
                  case 2:
                     outcome = client_type::outcome_success;
                     break;

                  case 3:
                     outcome = client_type::outcome_failure_aborted;
                     break;
                  }
                  event_complete::cpost(this, outcome);
               }
            }
            else
               DeviceBase::onNetMessage(router, message);
         }
         else
            DeviceBase::onNetMessage(router, message);
      } // onNetMessage
   };
};


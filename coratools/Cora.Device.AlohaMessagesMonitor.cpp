/* Cora.Device.AlohaMessagesMonitor.cpp

   Copyright (C) 2021, Campbell Scientific, Inc

   Written by: Jon Trauntvein 
   Date Begun: Tuesday 02 February 2021
   Last Change: Tuesday 02 February 2021
   Last Commit: $Date: 2021-02-03 13:40:59 -0600 (Wed, 03 Feb 2021) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.AlohaMessagesMonitor.h"
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

            static void cpost(AlohaMessagesMonitor *sender)
            { (new event_started(sender))->post(); }

         private:
            event_started(AlohaMessagesMonitor *sender):
               Event(event_id, sender)
            { }
         };
         uint4 const event_started::event_id(
            Csi::Event::registerType("Cora::Device::AlohaMessagesMonitor::event_started"));


         class event_failure: public Csi::Event
         {
         public:
            static uint4 const event_id;
            typedef AlohaMessagesMonitor::client_type::failure_type failure_type;
            failure_type const failure;
            static void cpost(AlohaMessagesMonitor *sender, failure_type failure)
            { (new event_failure(sender, failure))->post(); }

         private:
            event_failure(AlohaMessagesMonitor *sender, failure_type failure_):
               Event(event_id, sender),
               failure(failure_)
            { }
         };
         uint4 const event_failure::event_id(
            Csi::Event::registerType("Cora::Device::AlohaMessagesMonitor::event_failure"));


         class event_messages: public Csi::Event
         {
         public:
            static uint4 const event_id;
            static void cpost(AlohaMessagesMonitor *sender)
            { (new event_messages(sender))->post(); }

         private:
            event_messages(AlohaMessagesMonitor *sender):
               Event(event_id, sender)
            { }
         };
         uint4 const event_messages::event_id(
            Csi::Event::registerType("Cora::Device::AlohaMessagesMonitor::event_messages"));
      };

      
      void AlohaMessagesMonitor::receive(event_handle &ev)
      {
         if(ev->getType() == event_started::event_id)
         {
            if(client_type::is_valid_instance(client))
               client->on_started(this);
            else
               finish();
         }
         else if(ev->getType() == event_failure::event_id)
         {
            event_failure *event((event_failure *)ev.get_rep());
            client_type *alert(client);
            finish();
            if(client_type::is_valid_instance(alert))
               alert->on_failure(this, event->failure);
         }
         else if(ev->getType() == event_messages::event_id)
         {
            if(client_type::is_valid_instance(client))
            {
               if(client->on_messages(this, messages))
                  send_ack();
               else
                  state = state_ack_wait;
            }
            else
               finish();
         }
      } // receive

      void AlohaMessagesMonitor::send_ack()
      {
         if(state == state_ack_wait)
         {
            Csi::Messaging::Message ack(device_session, Messages::monitor_aloha_messages_ack);
            ack.addUInt4(tran_no);
            router->sendMessage(&ack);
            messages.clear();
            state = state_active;
         }
      } // send_ack

      void AlohaMessagesMonitor::describe_failure(
         std::ostream &out, client_type::failure_type failure)
      {
         switch(failure)
         {
         default:
            format_devicebase_failure(out, devicebase_failure_unknown);
            break;

         case client_type::failure_logon:
            format_devicebase_failure(out, devicebase_failure_logon);
            break;

         case client_type::failure_session:
            format_devicebase_failure(out, devicebase_failure_session);
            break;

         case client_type::failure_invalid_device_name:
            format_devicebase_failure(out, devicebase_failure_invalid_device_name);
            break;

         case client_type::failure_unsupported:
            format_devicebase_failure(out, devicebase_failure_unsupported);
            break;

         case client_type::failure_security:
            format_devicebase_failure(out, devicebase_failure_security);
            break;

         case client_type::failure_shut_down:
            out << "receiver is being shut down";
            break;
         }
      } // describe_failure

      void AlohaMessagesMonitor::on_devicebase_ready()
      {
         Csi::Messaging::Message start_cmd(device_session, Messages::monitor_aloha_messages_start_cmd);
         tran_no = ++last_tran_no;
         start_cmd.addUInt4(tran_no);
         state = state_active;
         router->sendMessage(&start_cmd);
      } // on_devicebase_ready

      void AlohaMessagesMonitor::on_devicebase_failure(devicebase_failure_type failure)
      {
         client_type::failure_type report(client_type::failure_unknown);
         switch(failure)
         {
         case devicebase_failure_logon:
            report = client_type::failure_logon;
            break;
            
         case devicebase_failure_session:
            report = client_type::failure_session;
            break;
            
         case devicebase_failure_invalid_device_name:
            report = client_type::failure_invalid_device_name;
            break;
            
         case devicebase_failure_unsupported:
            report = client_type::failure_unsupported;
            break;
            
         case devicebase_failure_security:
            report = client_type::failure_security;
            break;
         }
         event_failure::cpost(this, report);
      } // on_devicebase_failure

      void AlohaMessagesMonitor::onNetMessage(
         Csi::Messaging::Router *router, Csi::Messaging::Message *message)
      {
         if(state >= state_active)
         {
            if(message->getMsgType() == Messages::monitor_aloha_messages_start_ack)
               event_started::cpost(this);
            else if(message->getMsgType() == Messages::monitor_aloha_messages_not)
            {
               uint4 reported_tran, rcd;
               message->readUInt4(reported_tran);
               message->readUInt4(rcd);
               if(rcd == 1)
               {
                  uint4 count;
                  int8 stamp;
                  StrAsc aloha_str;
                  message->readUInt4(count);
                  for(uint4 i = 0; i < count; ++i)
                  {
                     message->readInt8(stamp);
                     message->readStr(aloha_str);
                     messages.push_back(client_type::message_type(stamp, aloha_str));
                  }
                  state = state_ack_wait;
                  event_messages::cpost(this);
               }
               else if(rcd == 2)
                  event_failure::cpost(this, client_type::failure_shut_down);
               else
                  event_failure::cpost(this, client_type::failure_unknown);
            }
            else
               DeviceBase::onNetMessage(router, message);
         }
         else
            DeviceBase::onNetMessage(router, message);
      } // onNetMessage
   };
};


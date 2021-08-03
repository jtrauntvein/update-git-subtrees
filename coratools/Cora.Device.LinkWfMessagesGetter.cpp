/* Cora.Device.LinkWfMessagesGetter.cpp

   Copyright (C) 2005, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 05 January 2005
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.LinkWfMessagesGetter.h"


namespace Cora
{
   namespace Device
   {
      namespace
      {
         ////////////////////////////////////////////////////////////
         // class event_complete
         ////////////////////////////////////////////////////////////
         class event_complete: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // client
            ////////////////////////////////////////////////////////////
            typedef LinkWfMessagesGetter::client_type client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // outcome
            ////////////////////////////////////////////////////////////
            typedef client_type::outcome_type outcome_type;
            outcome_type outcome;

            ////////////////////////////////////////////////////////////
            // messages
            ////////////////////////////////////////////////////////////
            typedef client_type::messages_type messages_type;
            messages_type messages;

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_complete(
               LinkWfMessagesGetter *getter,
               client_type *client_,
               outcome_type outcome_,
               messages_type const &messages_):
               Event(event_id,getter),
               client(client_),
               outcome(outcome_),
               messages(messages_)
            { }
            
         public:
            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               LinkWfMessagesGetter *getter,
               client_type *client,
               outcome_type outcome,
               messages_type const &messages = messages_type())
            {
               event_complete *event = new event_complete(getter,client,outcome,messages);
               try { event->post(); }
               catch(Event::BadPost &) { delete event; } 
            } 
         };


         uint4 const event_complete::event_id =
         Csi::Event::registerType("Cora::Device::LinkWfMessagesGetter::event_complete");
      };


      ////////////////////////////////////////////////////////////
      // class LinkWfMessagesGetter definitions
      ////////////////////////////////////////////////////////////
      void LinkWfMessagesGetter::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         if(ev->getType() == event_complete::event_id)
         {
            event_complete *event = static_cast<event_complete *>(ev.get_rep());
            client_type *client = this->client;
            if(event->client == client)
            {
               finish();
               if(client_type::is_valid_instance(client))
                  client->on_complete(this,event->outcome,event->messages);
            }
         }
      } // receive

      
      void LinkWfMessagesGetter::on_devicebase_ready()
      {
         Csi::Messaging::Message cmd(
            device_session,
            Messages::get_link_wf_messages_cmd);
         cmd.addUInt4(++last_tran_no);
         state = state_active;
         router->sendMessage(&cmd);
      } // on_devicebase_ready

      
      void LinkWfMessagesGetter::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(state == state_active)
         {
            if(msg->getMsgType() == Messages::get_link_wf_messages_ack)
            {
               uint4 tran_no;
               uint4 messages_count;
               client_type::messages_type messages;

               msg->readUInt4(tran_no);
               msg->readUInt4(messages_count);
               for(uint4 i = 0; i < messages_count; ++i)
               {
                  client_type::message_type message;
                  int8 stamp;
                  uint4 severity;
                  
                  msg->readWStr(message.dev_name);
                  msg->readInt8(stamp); message.stamp = stamp;
                  msg->readUInt4(severity); message.severity = static_cast<SwfCode>(severity);
                  msg->readStr(message.text);
                  messages.push_back(message);
               }
               event_complete::cpost(
                  this,
                  client,
                  client_type::outcome_success,
                  messages);
            }
            else
               DeviceBase::onNetMessage(rtr,msg);
         }
         else
            DeviceBase::onNetMessage(rtr,msg);
      } // onNetMessage

      
      void LinkWfMessagesGetter::on_devicebase_failure(devicebase_failure_type failure)
      {
         client_type::outcome_type outcome;
         switch(failure)
         {
         case devicebase_failure_logon:
            outcome = client_type::outcome_invalid_logon;
            break;
            
         case devicebase_failure_session:
            outcome = client_type::outcome_session_failed;
            break;
            
         case devicebase_failure_invalid_device_name:
            outcome = client_type::outcome_invalid_device_name;
            break;
            
         case devicebase_failure_unsupported:
            outcome = client_type::outcome_unsupported;
            break;
            
         case devicebase_failure_security:
            outcome = client_type::outcome_security_blocked;
            break;
            
         default:
            outcome = client_type::outcome_unknown;
            break;
         }
         event_complete::cpost(this,client,outcome);
      } // on_devicebase_failure 
   };
};


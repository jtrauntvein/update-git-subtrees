/*  File Name: $RCSfile: Csi.ToolbarCommunicator.cpp,v $

  Copyright (C) 2002, 2014 Campbell Scientific, Inc.

  Written By: Tyler Mecham
  Date Begun: 4/4/2002 12:53:16 PM

  Last Changed By: $Author: jon $
  Last Commit: $Date: 2014-11-19 11:07:15 -0600 (Wed, 19 Nov 2014) $
*/

#pragma hdrstop

#include "Csi.ToolbarCommunicator.h"
#include "Csi.Messaging.Message.h"
#include "trace.h"

namespace Csi
{
   ToolbarCommunicator::ToolbarCommunicator():
      state(state_standby),
      session_no(0),
      tran_no(0),
      client(0),
      local_port(0),
      allow_remote_connections(false),
      close_resp_pending(false),
      local_server_running(false)
   { }


   ToolbarCommunicator::~ToolbarCommunicator()
   { finish(); }


   void ToolbarCommunicator::start(
      ToolbarCommunicatorClient *client_, 
      router_handle &router_)
   {
      if( state == state_standby )
      {
         if( ToolbarCommunicatorClient::is_valid_instance(client_) )
         {
            client = client_;
            router = router_;
            session_no = router->openSession(this);
                
            //Send client register message
            Csi::Messaging::Message reg_client(session_no,reg_client_cmd);
            reg_client.addStr(client_name);
            router->sendMessage(&reg_client);
            state = state_registering;
         }
         else
            throw std::invalid_argument("ToolbarCommunicatorClient::Invalid client pointer");
      }
      else
         throw exc_invalid_state();
   }


   void ToolbarCommunicator::finish()
   {
      if(state != state_standby)
      {
         if(ToolbarCommunicatorClient::is_valid_instance(router.get_rep()) && session_no)
            router->closeSession(session_no);
         session_no = 0;
         state = state_standby;
      }
   }


   void ToolbarCommunicator::set_client_name(StrAsc const &client_name_)
   {
      if( state == state_standby )
         client_name = client_name_;
      else
         throw exc_invalid_state();
   }


   void ToolbarCommunicator::start_enumerate_clients()
   {
      if( state == state_ready )
      {
      }
      else
         throw exc_invalid_state();
   }


   void ToolbarCommunicator::stop_enumerate_clients()
   {
      if( state == state_ready )
      {
      }
      else
         throw exc_invalid_state();
   }


   void ToolbarCommunicator::forward_message(
      ToolbarCommunicatorClient *client,
      StrAsc const &client_name,
      ForwardedToolbarMessage::ForwardedMessage *msg)
   {
      if( state == state_ready )
      {
         Csi::Messaging::Message the_msg(session_no,forward_msg_cmd);
         the_msg.addStr(client_name);
         the_msg.addUInt4(++tran_no);
         //add custom msg info
         uint4 msg_type = msg->get_msg_type();
         the_msg.addUInt4(msg_type);
         if(msg_type == ForwardedToolbarMessage::rtmc_file_loaded)
         {
            ForwardedToolbarMessage::RTMCFileOpenMessage *rtmc_msg = 
               dynamic_cast<ForwardedToolbarMessage::RTMCFileOpenMessage*>(msg);
            the_msg.addStr(rtmc_msg->get_file_name());
         }
         router->sendMessage(&the_msg);
      }
      else
         throw exc_invalid_state();
   }


   void ToolbarCommunicator::onNetSesBroken(
      Csi::Messaging::Router *router, 
      uint4 session_no, 
      uint4 error_code, 
      char const *error_message)
   {
      //If the client isn't valid, finish
      if( ToolbarCommunicatorClient::is_valid_instance(client) )
      {
         client->on_failure(this,client->failure_connection_failed);
      }
      finish();
   }


   void ToolbarCommunicator::onNetMessage( 
      Csi::Messaging::Router *router, 
      Csi::Messaging::Message *message)
   {
      //If the client isn't valid, finish
      if( !ToolbarCommunicatorClient::is_valid_instance(client) )
         finish();

      uint4 message_type = message->getMsgType();
      switch(message_type)
      {
         case reg_client_ack:
         {
            bool registered;
            message->readBool(registered);
            if( registered )
            {
               StrAsc workDir;
               message->readStr(workDir);
               client->on_received_working_directory(
                  this, 
                  workDir);
               allow_remote_connections = true;
               local_server_running = true;
               local_port = 6789;
               if(message->whatsLeft())
               {
                  StrUni temp;
                  StrAsc temp2;
                  uint4 port;
                  uint4 op_mode;
                  
                  message->readStr(temp2);
                  message->readUInt4(port);
                  local_port = static_cast<uint2>(port);
                  message->readWStr(temp); // skip the logon name
                  message->readWStr(temp); // skip the password
                  message->readUInt4(op_mode);
                  allow_remote_connections = (op_mode == 2 || op_mode == 3);
                  local_server_running = (op_mode == 1 || op_mode == 2);
               }
               state = state_ready;
               client->on_started(this);
            }
            else
            {
               client->on_failure(
                  this,
                  client->failure_registration_failed);
            }
            break;
         }
         case set_window_state_cmd:
         {
            uint4 state;
            message->readUInt4(state);
            bool state_changed = false;
            switch(state)
            {
            case 0: //HIDE
               state_changed = client->on_hide(this);
               break;
            case 1: //RESTORE
               state_changed = client->on_restore(this);
               break;
            case 2: //SHOW
               state_changed = client->on_show(this);
               break;
            }
            Csi::Messaging::Message ack(session_no,set_window_state_ack);
            ack.addBool(state_changed);
            router->sendMessage(&ack);
            break;
         }
         case close_cmd:
         {                
            Csi::Messaging::Message ack(session_no, close_ack);
            router->sendMessage(&ack);

            bool can_close = client->on_close(this);

            if( !ToolbarCommunicatorClient::is_valid_instance(client) )
               finish();

            if(!Csi::InstanceValidator::is_valid_instance(router))
               return;

            if(can_close || client->send_close_resp_now(this))
            {
               Csi::Messaging::Message resp(session_no, close_resp);
               if(can_close)
                  resp.addUInt4(0);
               else
                  resp.addUInt4(1);
               router->sendMessage(&resp);
            }
            else
               close_resp_pending = true;
            break;
         }
         case enum_clients_note:
         {
            break;
         }
         case enum_clients_stop_ack:
         {
            break;
         }
         case forward_msg_ack:
         {
            break;
         }
         case forward_msg_note:
         {
            StrAsc client_name;
            message->readStr(client_name);
            uint4 tran_no;
            message->readUInt4(tran_no);
            uint4 custom_msg_type;
            message->readUInt4(custom_msg_type);
            switch(custom_msg_type)
            {
               case ForwardedToolbarMessage::rtdaq_connect_note:
               {
                  uint4 connection_state = 0;
                  message->readUInt4(connection_state);
                  StrAsc station_name;
                  message->readStr(station_name);
                  ForwardedToolbarMessage::RTDAQConnectMessage *forward_message = 
                     new ForwardedToolbarMessage::RTDAQConnectMessage(connection_state,station_name);
                  client->on_forwarded_msg(this,forward_message);

                  //Sending the client a message can result in a shutdown ... so check here
                  if(!ToolbarCommunicator::is_valid_instance(this))
                     return;

                  //Processed msg, so send ack
                  Csi::Messaging::Message ack(session_no,forward_msg_note_ack);
                  ack.addUInt4(tran_no);
                  router->sendMessage(&ack);
                  break;
               }
               case ForwardedToolbarMessage::rtmc_file_loaded:
               {
                  StrAsc rtmc_file;
                  message->readStr(rtmc_file);
                  ForwardedToolbarMessage::RTMCFileOpenMessage *forward_message = 
                     new ForwardedToolbarMessage::RTMCFileOpenMessage(rtmc_file);
                  client->on_forwarded_msg(this,forward_message);

                  //Sending the client a message can result in a shutdown ... so check here
                  if(!ToolbarCommunicator::is_valid_instance(this))
                     return;

                  //Processed msg, so send ack
                  Csi::Messaging::Message ack(session_no,forward_msg_note_ack);
                  ack.addUInt4(tran_no);
                  router->sendMessage(&ack);
                  break;
               }
               default:
                  break;
            }
            break;
         }
         default:
         {
            break;
         }
      }
   } // onNetMessage


   void ToolbarCommunicator::continue_close()
   {
      if(close_resp_pending)
      {
         Csi::Messaging::Message resp(session_no, close_resp);
         resp.addUInt4(0);
         router->sendMessage(&resp);
         close_resp_pending = false;
      }
   } // continue_close
};

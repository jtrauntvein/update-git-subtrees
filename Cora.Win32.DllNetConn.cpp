/* Cora.Win32.DllNetConn.cpp

   Copyright (C) 2000, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 05 June 2000
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Committed by: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Win32.DllNetConn.h"
#include "Cora.DllNetConnManager.h"
#include "Csi.Messaging.Message.h"
#include "Csi.Messaging.Defs.h"
#include "Csi.Messaging.Stub.h"
#include "Csi.Messaging.Server.h"


namespace Cora
{
   namespace Win32
   {
      ////////////////////////////////////////////////////////////
      // class DllNetConn definitions
      ////////////////////////////////////////////////////////////
      DllNetConn::DllNetConn(HMODULE coralib_module_):
         coralib_module(coralib_module_),
         send_message_func(0),
         is_server(false),
         is_attached(true),
         identifier(0)
      {
         send_message_func = reinterpret_cast<cora_messaging_callback_type>(
            ::GetProcAddress(
               coralib_module,
               "cora_messaging_call_forward"));
         if(send_message_func == 0)
            throw Csi::OsException("Messaging function lookup failed");
      } // constructor


      DllNetConn::DllNetConn(
         HMODULE coralib_module_,
         cora_messaging_callback_type send_message_func_,
         Csi::Messaging::Server *server):
         coralib_module(coralib_module_),
         send_message_func(send_message_func_),
         is_server(true),
         identifier(0),
         is_attached(false)
      {
         identifier = reinterpret_cast<uint4>(this);
         DllNetConnManager::add_connection(this, coralib_module);
         event_server_created::create_and_post(this, server);
      } // server constructor


      DllNetConn::~DllNetConn()
      {
         if(is_attached)
            DllNetConnManager::remove_connection(this);
      } // destructor


      void DllNetConn::attach()
      {
         if(!is_server)
         {
            typedef uint4(__stdcall start_conn_type)(cora_messaging_callback_type);
            start_conn_type *start_conn = reinterpret_cast<start_conn_type *>(
               ::GetProcAddress(
                  coralib_module,
                  "cora_start_connection"));
            if(start_conn == 0)
               throw Csi::OsException("Failed to lookup cora_start_connection");
            identifier = start_conn(DllNetConnManager::receive_event);
            if(identifier != 0)
               DllNetConnManager::add_connection(this, coralib_module);
            else
               throw Csi::MsgExcept("DllNetConn::attach -- cora_start_connection failed");
         }
         is_attached = true;
      } // attach


      void DllNetConn::detach()
      {
         if(is_attached)
         {
            send_message_func(identifier, cora_messaging_cancelled, 0, 0);
            DllNetConnManager::remove_connection(this);
            is_attached = false;
         }
      } // detach


      void DllNetConn::sendMessage(Csi::Messaging::Message *msg)
      {
         if(is_attached)
         {
            // we need to make a deep copy of the message so that we can control its lifetime. If this
            // connection is deployed in a server application, we also need to make a copy of the
            // message so that the client can delete it when finished.
            Csi::Messaging::Message message(*msg, true);
            if(is_server)
               DllNetConnManager::add_message(message);

            // we can now invoke the client's call-back routine.
            int rcd = send_message_func(
               identifier,
               cora_messaging_message,
               message.getLen(),
               message.getMsg());
            if(rcd == cora_messaging_rtn_accepted)
               resetTxWd();
            else
            {
               if(is_server)
                  DllNetConnManager::delete_message(message.getMsg());
               is_attached = false;
               DllNetConnManager::remove_connection(this);
               router->onConnClosed();
            }
         }
      } // sendMessage


      void DllNetConn::receive(Csi::SharedPtr<Csi::Event> &event_)
      {
         if(event_->getType() == event_message_received::event_id)
         {
            event_message_received *event = static_cast<event_message_received *>(event_.get_rep());
            if(event->message.getMsgType() != Csi::Messaging::Messages::type_heart_beat)
               router->rcvMessage(&event->message);
         }
         else if(event_->getType() == event_connection_cancelled::event_id)
         {
            is_attached = false;
            DllNetConnManager::remove_connection(this);
            router->onConnClosed();
         }
         else if(event_->getType() == event_server_created::event_id)
         {
            event_server_created *event = static_cast<event_server_created *>(event_.get_rep());
            Csi::Messaging::Stub *new_stub = new Csi::Messaging::Stub(event->server, this);
         }
      } // receive


      ////////////////////////////////////////////////////////////
      // class DllNetConn::event_message_received definitions
      ////////////////////////////////////////////////////////////
      uint4 const DllNetConn::event_message_received::event_id =
         Csi::Event::registerType("Cora::DllNetConn::event_message_received");


      void DllNetConn::event_message_received::create_and_post(
         DllNetConn *conn,
         void const *buff,
         uint4 buff_len)
      {
         try
         {
            event_message_received *ev = new event_message_received(conn, buff, buff_len);
            ev->post();
         }
         catch(Csi::Event::BadPost &)
         {}
      } // create_and_post


      DllNetConn::event_message_received::event_message_received(
         DllNetConn *conn,
         void const *buff,
         uint4 buff_len):
         Event(event_id, conn),
         message(buff, buff_len, true)
      {}


      ////////////////////////////////////////////////////////////
      // class DllNetConn::event_connection_cancelled definitions
      ////////////////////////////////////////////////////////////
      uint4 const DllNetConn::event_connection_cancelled::event_id =
         Csi::Event::registerType("Cora::DllNetConn::event_connection_cancelled");


      void DllNetConn::event_connection_cancelled::create_and_post(DllNetConn *conn)
      {
         try
         {
            event_connection_cancelled *ev = new event_connection_cancelled(conn);
            ev->post();
         }
         catch(Csi::Event::BadPost &)
         {}
      } // create_and_post


      DllNetConn::event_connection_cancelled::event_connection_cancelled(DllNetConn *conn):
         Event(event_id, conn)
      {}


      ////////////////////////////////////////////////////////////
      // class DllNetConn::event_server_created definitions
      ////////////////////////////////////////////////////////////
      uint4 const DllNetConn::event_server_created::event_id =
         Csi::Event::registerType("Cora::DllNetConn::event_server_created");


      void DllNetConn::event_server_created::create_and_post(
         DllNetConn *conn,
         Csi::Messaging::Server *server)
      {
         try
         {
            event_server_created *ev = new event_server_created(conn, server);
            ev->post();
         }
         catch(Csi::Event::BadPost &)
         {}
      } // create_and_post


      DllNetConn::event_server_created::event_server_created(
         DllNetConn *conn,
         Csi::Messaging::Server *server_):
         Event(event_id, conn),
         server(server_)
      {}
   };
};
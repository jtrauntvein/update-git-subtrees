/* Csi.Messaging.ProxyListener.cpp

   Copyright (C) 2014, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Tuesday 12 August 2014
   Last Change: Friday 03 June 2016
   Last Commit: $Date: 2016-06-08 12:40:35 -0600 (Wed, 08 Jun 2016) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Messaging.ProxyListener.h"
#include "Csi.SocketConnection.h"
#include "Csi.Messaging.Proxy.Defs.h"
#include "Csi.Messaging.Message.h"
#include "Csi.Messaging.Stub.h"
#include "Csi.Utils.h"
#include "Csi.Digest.h"
#include <stdlib.h>



namespace Csi
{
   namespace Messaging
   {
      ProxyListener::ProxyListener(
         StrAsc const &proxy_address_,
         StrAsc const &proxy_account_,
         StrUni const &proxy_password_,
         Server *default_server_,
         EventReceiver *logger_):
         auth_session(0),
         proxy_session(0),
         proxy_address(proxy_address_),
         proxy_account(proxy_account_),
         proxy_password(proxy_password_.to_utf8()),
         logger(logger_),
         default_server(default_server_)
      {
         // we need to connect to the proxy server.  We will first parse the server address
         StrAsc address_portion;
         uint2 port_portion;

         parse_uri_address(address_portion, port_portion, proxy_address);
         if(port_portion == 0)
            port_portion = 6788;
         proxy_router.bind(
            new Router(
               new SocketConnection(address_portion.c_str(), port_portion)));
         auth_session = proxy_router->openSession(this);
         proxy_session = proxy_router->openSession(this);

         // we will send the logon request message to the proxy
         Message logon_cmd(auth_session, Proxy::Messages::server_logon_cmd);
         logon_cmd.addUInt4(1); // tran no
         logon_cmd.addStr(proxy_account);
         logon_cmd.addUInt4(proxy_session);
         proxy_router->sendMessage(&logon_cmd);
      } // constructor


      ProxyListener::~ProxyListener()
      {
         if(proxy_router != 0)
         {
            if(auth_session)
               proxy_router->closeSession(auth_session);
            if(proxy_session)
               proxy_router->closeSession(proxy_session);
            proxy_router.clear();
         }
      } // destructor


      void ProxyListener::onNetSesBroken(
         Router *router,
         uint4 session_no,
         uint4 error_code,
         char const *error)
      {
         if(session_no == proxy_session)
         {
            proxy_session = 0;
            report_failure(error);
         }
      } // onNetSesBroken


      void ProxyListener::onNetMessage(
         Router *router, Message *message)
      {
         switch(message->getMsgType())
         {
         case Proxy::Messages::server_logon_challenge:
            on_logon_challenge(message);
            break;
            
         case Proxy::Messages::server_logon_ack:
            on_logon_ack(message);
            break;
            
         case Proxy::Messages::virtual_conn_not:
            on_virtual_conn_not(message);
            break;

         case Proxy::Messages::virtual_conn_forward_cmd:
            on_forward_cmd(message);
            break;

         case Proxy::Messages::virtual_conn_close_cmd:
            on_virtual_conn_close_cmd(message);
            break;

         case Proxy::Messages::server_register_ack:
            on_register_server_ack(message);
            break;
         }
      } // onNetMessage


      void ProxyListener::on_logon_challenge(Message *message)
      {
         uint4 tran_no;
         byte server_token[4];
         StrBin server_digest;

         if(message->readUInt4(tran_no) &&
            message->readBlock(server_token, sizeof(server_token)) &&
            message->readBStr(server_digest))
         {
            uint4 client_token_val(rand() + (rand() << 16));
            byte client_token[4];
            Md5Digest md5;
            Message response(proxy_session, Proxy::Messages::server_logon_response);
            
            response.addUInt4(tran_no);
            memcpy(client_token, &client_token_val, sizeof(client_token));
            response.addBlock(client_token, sizeof(client_token));
            md5.add(client_token, sizeof(client_token));
            md5.add(server_token, sizeof(server_token));
            md5.add(proxy_password.c_str(), proxy_password.length());
            response.addBytes(md5.final(), Md5Digest::digest_size);
            proxy_router->sendMessage(&response);
         }
         else
            report_failure("invalid logon challenge format");
      } // on_logon_challenge


      void ProxyListener::on_logon_ack(Message *message)
      {
         uint4 tran_no;
         uint4 outcome;
         if(message->readUInt4(tran_no) && message->readUInt4(outcome))
         {
            if(outcome == Proxy::server_logon_outcome_success)
            {
               log_event("registered with the proxy");
            }
            else
            {
               if(outcome == Proxy::server_logon_outcome_invalid_proxy)
               {
                  // the proxy server has indicated that the server account does not exist.  Instead
                  // of reporting a failure, we will attempt to ask the server to create out
                  // account.
                  Message register_cmd(auth_session, Proxy::Messages::server_register_cmd);
                  register_cmd.addUInt4(tran_no + 1);
                  register_cmd.addStr(proxy_account);
                  register_cmd.addWStr(proxy_password);
                  proxy_router->sendMessage(&register_cmd);
               }
               else
               {
                  char const *failures[] =
                     {
                        "unrecognised failure",
                        "success",
                        "proxy account does not exist",
                        "proxy already registered",
                        "proxy logon timed out",
                        "invalid response to the logon challenge"
                     };
                  uint4 failure = outcome;
                  if(outcome > 5)
                     failure = 0;
                  report_failure(failures[failure]);
               }
            }
         }
         else
            report_failure("invalid logon acknowledgement format");
      } // on_logon_ack


      void ProxyListener::on_virtual_conn_not(Message *message)
      {
         uint4 virtual_conn_id;
         StrAsc remote_address;
         if(message->readUInt4(virtual_conn_id) && message->readStr(remote_address))
         {
            ProxyConn *conn(
               new ProxyConn(
                  proxy_password,
                  virtual_conn_id,
                  remote_address,
                  proxy_router,
                  proxy_session));
            Stub *stub(new Stub(default_server, conn));
            connections[virtual_conn_id] = conn;
         }
         else
            report_failure("invalid virtual connection notification");
      } // on_virtual_conn_not


      void ProxyListener::on_forward_cmd(Message *message)
      {
         uint4 virtual_conn_id;
         if(message->readUInt4(virtual_conn_id))
         {
            connections_type::iterator ci(connections.find(virtual_conn_id));
            if(ci != connections.end())
            {
               message->reset();
               ci->second->onNetMessage(proxy_router.get_rep(), message);
            }
         }
      } // on_forward_cmd


      void ProxyListener::on_virtual_conn_close_cmd(Message *message)
      {
         uint4 virtual_conn_id;
         if(message->readUInt4(virtual_conn_id))
         {
            connections_type::iterator ci(connections.find(virtual_conn_id));
            if(ci != connections.end())
            {
               ci->second->getRouter()->onConnClosed();
               connections.erase(ci);
            }
         }
      } // on_virtual_conn_close_cmd


      void ProxyListener::on_register_server_ack(Message *message)
      {
         uint4 tran_no;
         uint4 outcome;
         if(message->readUInt4(tran_no) && message->readUInt4(outcome))
         {
            if(outcome == 1)
            {
               Message logon_cmd(auth_session, Proxy::Messages::server_logon_cmd);
               logon_cmd.addUInt4(tran_no + 1);
               logon_cmd.addStr(proxy_account);
               logon_cmd.addUInt4(proxy_session);
               proxy_router->sendMessage(&logon_cmd);
            }
            else
            {
               char const *outcomes[] =
               {
                  "unrecognised failure",
                  "success",
                  "account already exists but is inaccessable",
                  "invalid proxy account name"
               };
               if(outcome > 3)
                  outcome = 0;
               report_failure(outcomes[outcome]);
            }
         }
      } // on_register_server-_ack


      void ProxyListener::log_event(StrAsc const &message)
      {
         if(logger)
            SocketServiceLogEvent::cpost(logger, message);
      }


      void ProxyListener::report_failure(StrAsc const &message)
      {
         if(logger)
            SocketServiceLogEvent::cpost(logger, message);
         if(proxy_router != 0)
         {
            if(proxy_session != 0)
               proxy_router->closeSession(proxy_session);
            if(auth_session != 0)
               proxy_router->closeSession(auth_session);
            proxy_session = auth_session = 0;
            proxy_router.clear();
         }
         default_server->onListenerFail(this);
      } // report_failure
   };
};



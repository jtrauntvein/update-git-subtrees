/* Cora.ClientBase.cpp

   Copyright (C) 2000, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 25 May 2000
   Last Change: Thursday 17 December 2020
   Last Commit: $Date: 2020-12-17 12:40:37 -0600 (Thu, 17 Dec 2020) $ 
   Committed by: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.ClientBase.h"
#include "Cora.LgrNet.Defs.h"
#include "Cora.Sec2.Defs.h"
#include "coratools.strings.h" 
#include "Csi.Utils.h"
#include "Csi.Digest.h"
#include <assert.h>
#include <stdlib.h>


namespace Cora
{
   ////////////////////////////////////////////////////////////
   // class ClientBase definitions
   ////////////////////////////////////////////////////////////
   ClientBase::ClientBase():
      net_session(0),
      last_tran_no(0),
      state(corabase_state_standby),
      lgrnet_access_level(Sec2::AccessLevels::level_root)
   { }

   
   ClientBase::~ClientBase()
   { finish(); }

   
   void ClientBase::set_logon_name(StrUni const &logon_name_)
   {
      if(state == corabase_state_standby)
         logon_name = logon_name_;
      else
         throw exc_invalid_state();
   } // set_logon_name


   void ClientBase::set_logon_password(StrUni const &logon_password_)
   {
      if(state == corabase_state_standby)
         logon_password = logon_password_;
      else
         throw exc_invalid_state();
   } // set_logon_password

   void ClientBase::set_access_token(StrAsc const &access_token_)
   {
      if(state != corabase_state_standby)
         throw exc_invalid_state();
      access_token = access_token_;
   }


   void ClientBase::set_application_name(StrUni const &application_name_)
   {
      if(state == corabase_state_standby)
         application_name = application_name_;
      else
         throw exc_invalid_state();
   } // set_application_name


   void ClientBase::start(router_handle &router_)
   {
      if(state == corabase_state_standby)
      {
         if(router_ != 0 && router_->numRoutes() == 0)
         {
            // we need to create a new session and execute the logon protocol on it
            router = router_;
            last_tran_no = 0;
            state = corabase_state_logging_on;
            net_session = router->openSession(this);

            // we will now use the new session to get the server version information
            Csi::Messaging::Message query_cmd(
               net_session,
               Csi::Messaging::Messages::type_query_server_cmd);
            router->sendMessage(&query_cmd);
         }
         else
            throw std::invalid_argument("Invalid router");
      }
      else
         throw exc_invalid_state();
   } // start


   void ClientBase::start(ClientBase *other_client, bool use_own_logon)
   {
      if(state == corabase_state_standby)
      {
         if(ClientBase::is_valid_instance(other_client) &&
            other_client->state == corabase_state_ready)
         {
            // we will copy most of the information we need from the other client
            lgrnet_access_level = other_client->lgrnet_access_level;
            last_tran_no = 0;
            router = other_client->router;
            interface_version = other_client->interface_version;
            server_name = other_client->server_name;
            application_name = other_client->application_name;
            access_token = other_client->access_token;
            refresh_token = other_client->refresh_token;
            
            // now we need to create our own session and clone the original
            net_session = router->openSession(this);
            Csi::Messaging::Message clone_cmd(
               other_client->net_session,
               Cora::LgrNet::Messages::clone_session_cmd);
            clone_cmd.addUInt4(net_session);
            router->sendMessage(&clone_cmd);

            if(!use_own_logon ||
               (logon_name == other_client->logon_name && logon_password == other_client->logon_password))
            {
               if(!use_own_logon)
               {
                  logon_name = other_client->logon_name;
                  logon_password = other_client->logon_password;
               }
               state = corabase_state_ready;
               on_corabase_ready();
            }
            else
               start_logon();
         }
         else
            throw std::invalid_argument("Invalid other_client parameter");
      }
      else
         throw exc_invalid_state();
   } // start

   
   void ClientBase::describe_failure(
      std::ostream &out,
      corabase_failure_type failure)
   {
      using namespace ClientBaseStrings;
      switch(failure)
      {
      case corabase_failure_logon:
         out << my_strings[strid_failure_logon];
         break;
         
      case corabase_failure_session:
         out << my_strings[strid_failure_session];
         break;
         
      case corabase_failure_unsupported:
         out << my_strings[strid_failure_unsupported];
         break;
         
      case corabase_failure_security:
         out << my_strings[strid_failure_security];
         break;

      case corabase_failure_invalid_access:
         out << my_strings[strid_failure_invalid_access];
         break;

      case corabase_failure_access_expired:
         out << my_strings[strid_failure_access_expired];
         break;
         
      default:
         out << my_strings[strid_failure_unknown];
         break;
      }
   } // describe_failure

   
   void ClientBase::finish()
   {
      if(state != corabase_state_standby)
      {
         if(router.get_rep() && net_session)
            router->closeSession(net_session);
         net_session = 0;
         state = corabase_state_standby;
         router.clear();
      }
   } // finish

   
   void ClientBase::onNetMessage(
      Csi::Messaging::Router *rtr,
      Csi::Messaging::Message *msg)
   {
      if(msg->getMsgType() == Csi::Messaging::Messages::type_query_server_ack)
      {
         if(state == corabase_state_logging_on)
         {
            // initialise the server interface version
            StrUni interface_version_string;
            
            msg->readWStr(server_name);
            msg->readWStr(interface_version_string);
            interface_version = interface_version_string.c_str();
            start_logon();
         }
      }
      else if(msg->getMsgType() == LgrNet::Messages::logon_ex_challenge)
      {
         uint4 tran_no;
         byte server_token[sizeof(uint4)];
         StrBin server_digest;

         if(msg->readUInt4(tran_no) &&
            msg->readBlock(server_token, sizeof(server_token)) &&
            msg->readBStr(server_digest))
         {
            // we need to form the components of our response to the challenge.
            Csi::Md5Digest md5;
            uint4 client_token(rand() + (rand() << 16));
            byte client_token_bytes[sizeof(client_token)];
            byte client_digest[16];
            
            memcpy(&client_token_bytes, &client_token, sizeof(client_token));
            md5.add(client_token_bytes, sizeof(client_token_bytes));
            md5.add(server_token, sizeof(server_token));
            for(size_t i = 0; i < logon_password.length(); ++i)
            {
               uint2 ch = static_cast<uint2>(logon_password[i]);
               if(!Csi::is_big_endian())
                  Csi::reverse_byte_order(&ch, sizeof(ch));
               md5.add(&ch, sizeof(ch));
            }
            memcpy(client_digest, md5.final(), Csi::Md5Digest::digest_size);

            // send the response
            Csi::Messaging::Message response(tran_no, LgrNet::Messages::logon_ex_response);
            response.addUInt4(tran_no);
            response.addWStr(logon_name);
            response.addBlock(client_token_bytes, sizeof(client_token_bytes));
            response.addBytes(client_digest, sizeof(client_digest));
            router->sendMessage(&response);
         }
         else
         {
            state = corabase_state_standby;
            router->closeSession(net_session);
            net_session = 0;
            on_corabase_failure(corabase_failure_logon);
         }
      }
      else if(msg->getMsgType() == LgrNet::Messages::logon_ack ||
              msg->getMsgType() == LgrNet::Messages::logon_ex_ack)
      {
         if(state == corabase_state_logging_on)
         {
            uint4 tran_no;
            bool status;
            msg->readUInt4(tran_no);
            msg->readBool(status);
            if(status)
            {
               state = corabase_state_ready;
               on_corabase_ready();
            }
            else
            {
               state = corabase_state_standby;
               router->closeSession(net_session);
               net_session = 0;
               on_corabase_failure(corabase_failure_logon);
            }
         }
      }
      else if(msg->getMsgType() == LgrNet::Messages::login_access_token_ack)
      {
         if(state == corabase_state_logging_on)
         {
            uint4 tran_no(0), rcd(0);
            msg->readUInt4(tran_no);
            msg->readUInt4(rcd);
            if(rcd == 1)
            {
               state = corabase_state_ready;
               msg->readStr(access_token);
               msg->readStr(refresh_token);
               on_corabase_ready();
            }
            else
            {
               corabase_failure_type rtn(corabase_failure_unknown);
               switch(rcd)
               {
               case 3:
                  rtn = corabase_failure_invalid_access;
                  break;

               case 4:
                  rtn = corabase_failure_access_expired;
                  break;
               }
               state = corabase_state_standby;
               router->closeSession(net_session);
               net_session = 0;
               on_corabase_failure(rtn);
            }
         }
      }
      else if(msg->getMsgType() == Csi::Messaging::Messages::type_message_rejected_not)
      {
         uint4 reason;
         msg->readUInt4(reason);
         switch(reason)
         {
         case Csi::Messaging::Messages::message_rejected_unsupported:
            on_corabase_failure(corabase_failure_unsupported);
            break;
            
         case Csi::Messaging::Messages::message_rejected_security:
            on_corabase_failure(corabase_failure_security);
            break;

         default:
            on_corabase_failure(corabase_failure_unknown);
            break;
         }
      }
      else if(msg->getMsgType() == Cora::LgrNet::Messages::announce_access_level)
      {
         msg->reset();
         msg->readUInt4(lgrnet_access_level);
      }
      else if(msg->getMsgType() == Cora::LgrNet::Messages::snapshot_restored_not)
         on_snapshot_restored();
   } // onNetMessage


   void ClientBase::start_logon()
   {
      // if the interface version supports it, we need to send the extended logon start command.
      if(interface_version >= Csi::VersionNumber("2.3.1") && access_token.length())
      {
         Csi::Messaging::Message login_cmd(net_session, LgrNet::Messages::login_access_token_cmd);
         login_cmd.addUInt4(++last_tran_no);
         login_cmd.addStr(access_token);
         router->sendMessage(&login_cmd);
      }
      else if(interface_version >= Csi::VersionNumber("1.3.13.1"))
      {
         Csi::Messaging::Message logon_cmd(net_session, LgrNet::Messages::logon_ex_start_cmd);
         logon_cmd.addUInt4(++last_tran_no);
         logon_cmd.addWStr(logon_name);
         logon_cmd.addWStr(application_name);
         router->sendMessage(&logon_cmd);
      }
      else
      {
         // we need to execute the logon protocol
         Csi::Messaging::Message logon_cmd(net_session, LgrNet::Messages::logon_cmd);
         logon_cmd.addUInt4(++last_tran_no);
         logon_cmd.addWStr(logon_name);
         logon_cmd.addWStr(logon_password);
         logon_cmd.addWStr(application_name);
         router->sendMessage(&logon_cmd);
      }
   } // start_logon


   void ClientBase::onNetSesBroken(
      Csi::Messaging::Router *rtr,
      uint4 session_no,
      uint4 reason,
      char const *msg)
   {
      // we should only report a failure if it came while logging on
      if(state == corabase_state_logging_on)
      {
         on_corabase_failure(corabase_failure_session);
         state = corabase_state_standby;
      }
      else if(session_no == net_session)
      {
         on_corabase_session_failure();
         net_session = 0;
      }
   } // onNetSesBroken
};

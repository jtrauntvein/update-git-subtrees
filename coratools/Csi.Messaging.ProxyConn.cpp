/* Csi.Messaging.ProxyConn.cpp

   Copyright (C) 2014, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Monday 11 August 2014
   Last Change: Friday 03 June 2016
   Last Commit: $Date: 2016-06-08 12:40:35 -0600 (Wed, 08 Jun 2016) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Messaging.ProxyConn.h"
#include "Csi.Messaging.Message.h"
#include "Csi.Messaging.Proxy.Defs.h"
#include "Csi.SocketConnection.h"
#include "Csi.Utils.h"
#include "Csi.Digest.h"
#include "mbedtls/aes.h"
#include <stdlib.h>


namespace Csi
{
   namespace Messaging
   {
      ProxyConn::ProxyConn(
         StrAsc const &proxy_address_,
         StrAsc const &proxy_account_,
         StrUni const &proxy_password_):
         proxy_address(proxy_address_),
         proxy_account(proxy_account_),
         proxy_password(proxy_password_.to_utf8()),
         virtual_conn_id(0),
         auth_session(0),
         proxy_session(0),
         for_server(false)
      {
         init_encryption();
      } // constructor


      ProxyConn::ProxyConn(
         StrAsc const &proxy_password_,
         uint4 virtual_conn_id_,
         StrAsc const &remote_address_,
         SharedPtr<Router> &proxy_router_,
         uint4 proxy_session_):
         proxy_password(proxy_password_),
         virtual_conn_id(virtual_conn_id_),
         remote_address(remote_address_),
         proxy_router(proxy_router_),
         proxy_session(proxy_session_),
         for_server(true)
      {
         init_encryption();
      } // constructor


      ProxyConn::~ProxyConn()
      {
         delete[] encrypt_context;
         delete[] decrypt_context;
      } // destructor


      void ProxyConn::attach()
      {
         if(!for_server)
         {
            // we need to parse the proxy server address and port.
            StrAsc address_portion;
            uint2 port;
            Csi::parse_uri_address(address_portion, port, proxy_address);
            if(port == 0)
               port = 6788;
            
            // we need to create the proxy_router and connection that we will use to communicate with the
            // proxy server.
            proxy_router.bind(
               new Router(
                  new SocketConnection(address_portion.c_str(), port)));
            auth_session = proxy_router->openSession(this);
            proxy_session = proxy_router->openSession(this);
            
            // We will need to send a client logon message to the proxy.
            Message logon_cmd(auth_session, Proxy::Messages::client_logon_cmd);
            logon_cmd.addUInt4(1); // we will always use an auth tran number of one
            logon_cmd.addStr(proxy_account);
            logon_cmd.addUInt4(proxy_session);
            proxy_router->sendMessage(&logon_cmd);
         }
      } // attach


      void ProxyConn::detach()
      {
         if(proxy_router != 0)
         {
            if(auth_session != 0)
               proxy_router->closeSession(auth_session);
            if(proxy_session != 0)
               proxy_router->closeSession(proxy_session);
            auth_session = proxy_session = 0;
            virtual_conn_id = 0;
            proxy_router.clear();
         }
      } // detach


      void ProxyConn::sendMessage(Message *message)
      {
         message_queue.push_back(new Message(*message, true));
         do_send_messages();
      } // sendMessage


      void ProxyConn::onNetSesBroken(
         Router *router, uint4 session_no, uint4 error_code, char const *error_message)
      {
         if(session_no == proxy_session)
         {
            proxy_router.clear();
            proxy_session = auth_session = 0;
            virtual_conn_id = 0;
            router->onConnClosed();
         }
      } // onNetSesBroken


      void ProxyConn::onNetMessage(Router *router, Message *message)
      {
         switch(message->getMsgType())
         {
         case Proxy::Messages::client_logon_challenge:
            on_proxy_auth_challenge(message);
            break;
            
         case Proxy::Messages::client_logon_ack:
            on_proxy_auth_ack(message);
            break;

         case Proxy::Messages::virtual_conn_forward_cmd:
            on_conn_forward_cmd(message);
            break;
         }
      } // onNetMessage


      void ProxyConn::init_encryption()
      {
         // we will calculate the check sum for the proxy password to use as the AES key
         byte aes_key[Md5Digest::digest_size];
         Md5Digest md5;

         md5.add(proxy_password.c_str(), proxy_password.length());
         memcpy(aes_key, md5.final(), sizeof(aes_key));

         // we need to construct the encryption and decryption contexts
         byte init_vector[sizeof(aes_key)];
         mbedtls_aes_context *encrypt_struct;
         mbedtls_aes_context *decrypt_struct;
         encrypt_context = new byte[sizeof(mbedtls_aes_context)];
         decrypt_context = new byte[sizeof(mbedtls_aes_context)];
         encrypt_struct = reinterpret_cast<mbedtls_aes_context *>(encrypt_context);
         decrypt_struct = reinterpret_cast<mbedtls_aes_context *>(decrypt_context);
         memset(init_vector, 0, sizeof(init_vector));
         mbedtls_aes_init(encrypt_struct);
         mbedtls_aes_init(decrypt_struct);
         mbedtls_aes_setkey_enc(encrypt_struct, aes_key, 128);
         mbedtls_aes_setkey_dec(decrypt_struct, aes_key, 128);
      } // init_encryption


      void ProxyConn::do_send_messages()
      {
         if(virtual_conn_id != 0)
         {
            for(message_queue_type::iterator mi = message_queue.begin();
                mi != message_queue.end();
                ++mi)
            {
               message_handle &message(*mi);
               Message forward_cmd(proxy_session, Proxy::Messages::virtual_conn_forward_cmd);
               uint2 message_sig(calcSigFor(message->getMsg(), message->getLen()));
               uint4 encrypted_len(message->getLen());
               mbedtls_aes_context *context(reinterpret_cast<mbedtls_aes_context *>(encrypt_context));
               Md5Digest md5;
               byte init_vector[Md5Digest::digest_size];
               
               if(encrypted_len % 16 != 0)
                  encrypted_len += 16 - (encrypted_len % 16);
               forward_cmd.addUInt4(virtual_conn_id);
               forward_cmd.addUInt4(1); // specify an AES-128 cipher
               forward_cmd.addUInt4(message->getLen());
               forward_cmd.addUInt2(message_sig);
               md5.add(forward_cmd.getBody(), forward_cmd.getBodyLen());
               memcpy(init_vector, md5.final(), sizeof(init_vector));
               encrypt_output.clear(encrypted_len);
               encrypt_input.reserve(encrypted_len);
               encrypt_input.setContents(message->getMsg(), message->getLen());
               while(encrypt_input.length() < encrypted_len)
                  encrypt_input.append(0);
               mbedtls_aes_crypt_cbc(
                  reinterpret_cast<mbedtls_aes_context *>(encrypt_context),
                  MBEDTLS_AES_ENCRYPT,
                  encrypted_len,
                  init_vector,
                  (byte const *)encrypt_input.getContents(),
                  encrypt_output.getContents_writable());
               forward_cmd.addBytes(encrypt_output.getContents(), (uint4)encrypt_output.length());
               proxy_router->sendMessage(&forward_cmd);
            }
            message_queue.clear();
         }
         resetTxWd();
      } // do_send_messages


      void ProxyConn::on_proxy_auth_challenge(Message *message)
      {
         uint4 tran_no;
         byte server_token[4];
         StrBin server_digest;

         if(message->readUInt4(tran_no) &&
            message->readBlock(server_token, sizeof(server_token)) &&
            message->readBStr(server_digest))
         {
            Message response(message->getClntSesNo(), Proxy::Messages::client_logon_response);
            uint4 client_token_val(rand() + (rand() << 16));
            byte client_token[4];
            Md5Digest md5;

            memcpy(client_token, &client_token_val, sizeof(client_token));
            response.addUInt4(tran_no);
            response.addBlock(client_token, sizeof(client_token));
            md5.add(client_token, sizeof(client_token));
            md5.add(server_token, sizeof(server_token));
            md5.add(proxy_password.c_str(), proxy_password.length());
            response.addBytes(md5.final(), Md5Digest::digest_size);
            proxy_router->sendMessage(&response);
         }
         else
         {
            detach();
            router->onConnClosed();
         }
      } // on_proxy_auth_challenge


      void ProxyConn::on_proxy_auth_ack(Message *message)
      {
         uint4 tran_no;
         uint4 outcome;
         if(message->readUInt4(tran_no) && message->readUInt4(outcome))
         {
            if(outcome == 1)
            {
               // we'll read the virtual conn ID and will also close the auth session since the
               // proxy session is now functioning.
               message->readUInt4(virtual_conn_id);
               proxy_router->closeSession(auth_session);
               auth_session = 0;
               do_send_messages();
            }
            else
            {
               detach();
               router->onConnClosed();
            }
         }
         else
         {
            detach();
            router->onConnClosed();
         }
      } // on_proxy_auth_ack


      void ProxyConn::on_conn_forward_cmd(Message *message)
      {
         uint4 conn_id;
         uint4 cipher;
         uint4 orig_len;
         uint2 orig_sig;
         if(message->readUInt4(conn_id) &&
            message->readUInt4(cipher) &&
            message->readUInt4(orig_len) &&
            message->readUInt2(orig_sig) &&
            message->readBStr(encrypt_input))
         {
            if(cipher == 1)
            {
               mbedtls_aes_context *context(reinterpret_cast<mbedtls_aes_context *>(decrypt_context));
               Md5Digest md5;
               uint2 calc_sig;
               byte init_vector[Md5Digest::digest_size];
               
               md5.add(message->getBody(), 14);
               memcpy(init_vector, md5.final(), sizeof(init_vector));
               encrypt_output.clear(encrypt_input.length());
               mbedtls_aes_crypt_cbc(
                  context,
                  MBEDTLS_AES_DECRYPT,
                  encrypt_input.length(),
                  init_vector,
                  (byte const *)encrypt_input.getContents(),
                  encrypt_output.getContents_writable());
               calc_sig = calcSigFor(encrypt_output.getContents(), orig_len);
               if(calc_sig == orig_sig)
               {
                  Message decrypted(encrypt_output.getContents(), (uint4)encrypt_output.length(), false);
                  if(decrypted.getMsgType() != Messages::type_heart_beat)
                     router->rcvMessage(&decrypted);
               }
               else
                  onNetSesBroken(proxy_router.get_rep(), proxy_session, 0, "invalid message signature");
            }
            else
               onNetSesBroken(proxy_router.get_rep(), proxy_session, 0, "unsupported cipher");
         }
         else
            onNetSesBroken(proxy_router.get_rep(), proxy_session, 0, "invalid forward command received");
      } // on_conn_forward_cmd
   };
};


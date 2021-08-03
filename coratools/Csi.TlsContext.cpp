/* Csi.TlsContext.cpp

   Copyright (C) 2010, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 27 October 2010
   Last Change: Thursday 22 October 2020
   Last Commit: $Date: 2020-10-22 11:26:39 -0600 (Thu, 22 Oct 2020) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.TlsContext.h"
#include "Csi.Utils.h"
#include "Csi.MaxMin.h"
#include "Csi.StrAscStream.h"
#include "mbedtls/platform.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/certs.h"
#include "mbedtls/x509.h"
#include "mbedtls/ssl.h"
#include "mbedtls/error.h"
#include "mbedtls/debug.h"
#include "mbedtls/net.h"
#include <deque>


namespace Csi
{
   namespace TlsContextHelpers
   {
      namespace
      {
         void my_debug(void *context, int level, char const *file, int line, char const *message)
         {
            trace("%s: %04d: %s", file, line, message);
         }


         int my_read(void *socket, byte *buff, size_t buff_len)
         {
            TlsContext *context(SocketTcpSock::get_tls_context());
            int rtn(MBEDTLS_ERR_NET_INVALID_CONTEXT);
            if(context)
               rtn = context->do_read(static_cast<SocketTcpSock *>(socket), buff, buff_len);
            return rtn;
         } // my_read


         int my_write(void *socket, byte const *buff, size_t buff_len)
         {
            TlsContext *context(SocketTcpSock::get_tls_context());
            int rtn(MBEDTLS_ERR_NET_INVALID_CONTEXT);
            if(context)
               rtn = context->do_write(static_cast<SocketTcpSock *>(socket), buff, buff_len);
            return rtn;
         } // my_write
      };

      
      /**
       * Defines the object that will maintain configuration objects for the global TLS context.
       */
      class TlsContextImpl
      {
      public:
         /**
          * Specifies the context structure for entropy
          */
         mbedtls_entropy_context entropy;

         /**
          * Specifies the context structure for the random number generator.
          */
         mbedtls_ctr_drbg_context ctr;

         /**
          * Specifies the server config structure.
          */
         mbedtls_ssl_config server_config;

         /**
          * Specifies the client config structure.
          */
         mbedtls_ssl_config client_config;

         /**
          * Specifies the context structure for the host certificate.
          */
         mbedtls_x509_crt certificate;

         /**
          * Specifies the context structure for the cacert.
          */
         mbedtls_x509_crt cacerts;

         /**
          * Specifies the context structure for the private key.
          */
         mbedtls_pk_context private_key;

         /**
          * Specifies the types of connections that this implementation can support.
          */
         TlsContext::service_level_type level;

         /**
          * Returns the last error message that was encountered.
          */
         StrAsc message;
         
         /**
          * Constructor
          */
         TlsContextImpl(
            StrAsc const &certificate_name,
            StrAsc const &private_key_name,
            StrAsc const &password):
            level(TlsContext::service_level_none)
         {
            int rcd;
            mbedtls_ssl_config_init(&client_config);
            mbedtls_ssl_config_init(&server_config);
            mbedtls_x509_crt_init(&certificate);
            mbedtls_x509_crt_init(&cacerts);
            mbedtls_pk_init(&private_key);
            mbedtls_entropy_init(&entropy);
            mbedtls_ctr_drbg_init(&ctr);
            mbedtls_debug_set_threshold(1);
            try
            {
               rcd = mbedtls_ctr_drbg_seed(&ctr, mbedtls_entropy_func, &entropy, 0, 0);
               if(rcd != 0)
                  throw TlsException(rcd, "random generator initialisation failed");
               rcd = mbedtls_ssl_config_defaults(
                  &client_config, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT);
               if(rcd != 0)
                  throw TlsException(rcd, "initialise client configuration failed");
               rcd = mbedtls_x509_crt_parse(&cacerts, (byte const *)mbedtls_test_cas_pem, mbedtls_test_cas_pem_len);
               if(rcd != 0)
                  throw TlsException(rcd, "parse cacerts");
               mbedtls_ssl_conf_rng(&client_config, mbedtls_ctr_drbg_random, &ctr);
               // mbedtls_ssl_conf_dbg(&client_config, my_debug, 0);
               mbedtls_ssl_conf_authmode(&client_config, MBEDTLS_SSL_VERIFY_OPTIONAL);
               mbedtls_ssl_conf_ca_chain(&client_config, &cacerts, 0);
               level = TlsContext::service_level_client;
               try
               {
                  if(certificate_name.length() > 0 && private_key_name.length() > 0)
                  {
                     rcd = mbedtls_x509_crt_parse_file(&certificate, certificate_name.c_str());
                     if(rcd != 0)
                        throw TlsException(rcd, "parse certificate file failed");
                     rcd = mbedtls_pk_parse_keyfile(&private_key, private_key_name.c_str(), password.c_str());
                     if(rcd != 0)
                        throw TlsException(rcd, "parse key file failed");
                     rcd = mbedtls_ssl_config_defaults(
                        &server_config, MBEDTLS_SSL_IS_SERVER, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT);
                     if(rcd != 0)
                        throw TlsException(rcd, "initialise server configuration failed");
                     mbedtls_ssl_conf_rng(&server_config, mbedtls_ctr_drbg_random, &ctr);
                     mbedtls_ssl_conf_dbg(&server_config, my_debug, 0);
                     mbedtls_ssl_conf_min_version(&server_config, 1, 2);
                     rcd = mbedtls_ssl_conf_own_cert(&server_config, &certificate, &private_key);
                     if(rcd != 0)
                        throw TlsException(rcd, "set server certificate failed");
                     level = TlsContext::service_level_client_server;
                  }
               }
               catch(std::exception &e)
               {
                  message = e.what();
               }
            }
            catch(std::exception &)
            {
               mbedtls_x509_crt_free(&certificate);
               mbedtls_x509_crt_free(&cacerts);
               mbedtls_pk_free(&private_key);
               mbedtls_ssl_config_free(&client_config);
               mbedtls_ssl_config_free(&server_config);
               mbedtls_ctr_drbg_free(&ctr);
               mbedtls_entropy_free(&entropy);
               throw;
            }
         }

         /**
          * Destructor
          */
         ~TlsContextImpl()
         {
            mbedtls_x509_crt_free(&certificate);
            mbedtls_x509_crt_free(&cacerts);
            mbedtls_pk_free(&private_key);
            mbedtls_ssl_config_free(&client_config);
            mbedtls_ssl_config_free(&server_config);
            mbedtls_ctr_drbg_free(&ctr);
            mbedtls_entropy_free(&entropy);
         }
      };


      /**
       * Defines an object that maintains the state for a single client or server connection.
       */
      class TlsConnection
      {
      public:
         /**
          * Specifies the socket associated with this TLS connection.
          */
         SocketTcpSock *socket;

         /**
          * Specifies the TLS context for this connection.
          */
         mbedtls_ssl_context context;

         /**
          * Set to true if the TLS handshake is complete.
          */
         bool handshake_complete;

         /**
          * Specifies the queue used to transmitting the cipher stream.
          */
         ByteQueue encrypted_tx;

         /**
          * Specifies the queue used to receiving the encrypted stream.
          */
         ByteQueue encrypted_rx;

         /**
          * Set to true if the connection object has been removed.
          */
         bool is_busy;

         /**
          * Set to true if the application has attempting to close the connection.
          */
         bool was_removed;

         /**
          * Set to true if a read event is already pending.
          */
         bool read_pending;
         
      public:
         /**
          * @param socket_ Specifies the buffered socket object associated with this connection.
          *
          * @param config Specifies the tls configuration used to initialise the context.
          *
          * @param transmit_before Specifies data that should be transmitted immediately before the
          * TLS handshake.
          *
          * @param before_len Specifies the number of bytes in the transmit_before buffer.
          */
         TlsConnection(
            SocketTcpSock *socket_,
            mbedtls_ssl_config *config,
            void const *transmit_before,
            size_t before_len):
            socket(socket_),
            handshake_complete(false),
            encrypted_tx(1024, true),
            encrypted_rx(1024, true),
            was_removed(false),
            is_busy(false),
            read_pending(false)
         {
            int rcd;
            mbedtls_ssl_init(&context);
            rcd = mbedtls_ssl_setup(&context, config);
            if(rcd != 0)
               throw TlsException(rcd, "TLS context initialisation failed");
            if(socket->get_domain_name().length() > 0)
            {
               rcd = mbedtls_ssl_set_hostname(&context, socket->get_domain_name().c_str());
               if(rcd != 0)
                  throw TlsException(rcd, "set host name extension");
            }
            mbedtls_ssl_set_bio(&context, socket, my_write, my_read, 0);
            if(transmit_before && before_len != 0)
               encrypted_tx.push(transmit_before, (uint4)before_len);
         }

         /**
          * Destructor
          */
         ~TlsConnection()
         {
            mbedtls_ssl_free(&context);
         }
      };


      TlsException::TlsException(int error_code_, char const *more):
         MsgExcept(more),
         error_code(error_code_)
      {
         if(error_code != 0)
         {
            char error[128];
            mbedtls_strerror(error_code, error, sizeof(error));
            msg += ": ";
            msg += error;
         }
      } // constructor
   };

   
   namespace
   {
      /**
       * Defines an event that will report that data has been received for a low level connection.
       */
      class event_socket_data: public Csi::Event
      {
      public:
         /**
          * Specifies the identifier for this event type.
          */
         static uint4 const event_id;

         /**
          * Specifies the affected socket.
          */
         SocketTcpSock *socket;

         /**
          * creates and posts this event.
          */
         static void cpost(TlsContext *receiver, SocketTcpSock *socket)
         {
            (new event_socket_data(receiver, socket))->post();
         }

      private:
         event_socket_data(TlsContext *receiver, SocketTcpSock *socket_):
            Event(event_id, receiver),
            socket(socket_)
         { }
      };


      uint4 const event_socket_data::event_id(
         Event::registerType("Csi::TlsContext::event_socket_data"));
      
      
      ////////////////////////////////////////////////////////////
      // class event_data_read
      ////////////////////////////////////////////////////////////
      class event_data_read: public Event
      {
      public:
         ////////////////////////////////////////////////////////////
         // event_id
         ////////////////////////////////////////////////////////////
         static uint4 const event_id;

         ////////////////////////////////////////////////////////////
         // socket
         ////////////////////////////////////////////////////////////
         SocketTcpSock *socket;

         ////////////////////////////////////////////////////////////
         // cpost
         ////////////////////////////////////////////////////////////
         static void cpost(TlsContext *context, SocketTcpSock *socket)
         {
            event_data_read *event(new event_data_read(context, socket));
            event->post();
         }

      private:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         event_data_read(TlsContext *context, SocketTcpSock *socket_):
            Event(event_id, context),
            socket(socket_)
         { } 
      };


      uint4 const event_data_read::event_id(Event::registerType("Csi::TlsContext::event_data_read"));


      ////////////////////////////////////////////////////////////
      // class event_context_initialised
      ////////////////////////////////////////////////////////////
      class event_context_initialised: public Csi::Event
      {
      public:
         ////////////////////////////////////////////////////////////
         // event_id
         ////////////////////////////////////////////////////////////
         static uint4 const event_id;

         ////////////////////////////////////////////////////////////
         // success
         ////////////////////////////////////////////////////////////
         TlsContext::service_level_type level;

         ////////////////////////////////////////////////////////////
         // message
         ////////////////////////////////////////////////////////////
         StrAsc const message;

         ////////////////////////////////////////////////////////////
         // cpost
         ////////////////////////////////////////////////////////////
         static void cpost(
            TlsContext *context,
            TlsContext::service_level_type level,
            StrAsc const &message)
         {
            event_context_initialised *event(
               new event_context_initialised(
                  context, level, message));
            event->post();
         }

      private:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         event_context_initialised(
            TlsContext *context, TlsContext::service_level_type level_, StrAsc const &message_):
            Event(event_id, context),
            level(level_),
            message(message_)
         { }
      };


      uint4 const event_context_initialised::event_id(
         Event::registerType("Csi::TlsContext::event_context_initialised"));
   };

   
   ////////////////////////////////////////////////////////////
   // class TlsContext definitions
   ////////////////////////////////////////////////////////////
   TlsContext::TlsContext(
      StrAsc const &certificate_name_,
      StrAsc const &private_key_name_,
      StrAsc const &password_):
      certificate_name(certificate_name_),
      private_key_name(private_key_name_),
      password(password_),
      accepts_client_connections(false)
   {
      try
      {
         implementation.bind(new impl_type(certificate_name, private_key_name, password));
         event_context_initialised::cpost(this, implementation->level, implementation->message);
      }
      catch(std::exception &e)
      {
         event_context_initialised::cpost(this, service_level_none, e.what());
      }
   } // constructor


   TlsContext::~TlsContext()
   {
      Protector<connections_type>::key_type connections(this->connections);
      connections->clear();
      implementation.clear();
   } // destructor


   void TlsContext::add_connection(
      SocketTcpSock *socket,
      bool is_client,
      void const *transmit_before,
      uint4 before_len)
   {
      // we need to ensure that the configuration has been created.
      if(implementation == 0)
      {
         try
         {
            implementation.bind(new impl_type(certificate_name, private_key_name, password));
            event_context_initialised::cpost(this, implementation->level, implementation->message);
         }
         catch(std::exception &e)
         {
            event_context_initialised::cpost(this, service_level_none, e.what());
         }
      }
      
      // now we can attempt to add the connection
      Protector<connections_type>::key_type connections(this->connections);
      connection_handle connection(
         new connection_type(
            socket,
            is_client ? &implementation->client_config : &implementation->server_config,
            transmit_before,
            before_len));
      connections->insert(connections_type::value_type(socket, connection));
      event_socket_data::cpost(this, socket);
   } // add_connection


   void TlsContext::remove_connection(SocketTcpSock *socket)
   {
      Protector<connections_type>::key_type connections(this->connections);
      connections_type::iterator ci(connections->find(socket));
      if(ci != connections->end())
      {
         ci->second->was_removed = true;
         if(!ci->second->is_busy)
            connections->erase(ci);
      }
   } // remove_connection


   void TlsContext::receive(SharedPtr<Event> &ev)
   {
      if(ev->getType() == event_context_initialised::event_id)
      {
         event_context_initialised *event(static_cast<event_context_initialised *>(ev.get_rep()));
         accepts_client_connections = event->level == service_level_client_server;
         on_context_initialised(event->level, event->message);
      }
      else if(ev->getType() == event_socket_data::event_id)
      {
         event_socket_data *event(static_cast<event_socket_data *>(ev.get_rep()));
         Protector<connections_type>::key_type connections(this->connections);
         connections_type::iterator ci(connections->find(event->socket));
         if(ci != connections->end())
         {
            // if we have not yet completed the handshake for the connection, we will need to do so
            // now.
            connection_handle connection(ci->second);
            int rcd(0);
            connection->is_busy = true;
            connection->read_pending = false;
            while(!connection->handshake_complete)
            {
               rcd = mbedtls_ssl_handshake(&connection->context);
               if(rcd != 0)
               {
                  if(rcd != MBEDTLS_ERR_SSL_WANT_READ && rcd != MBEDTLS_ERR_SSL_WANT_WRITE)
                  {
                     connections->erase(ci);
                     event->socket->on_socket_error(0);
                     return;
                  }
                  else
                     break;
               }
               else
               {
                  connection->handshake_complete = true;
                  event->socket->on_tls_client_ready();
               }
            }

            // if the handshake is complete, we can now attempt to read or write data for the connection.
            ByteQueue &socket_tx(event->socket->get_write_buffer());
            while(rcd >= 0 && connection->handshake_complete && !connection->was_removed)
            {
               // we need to attempt to write any available data.
               size_t send_len(csimin<size_t>(sizeof(low_level_buff), socket_tx.size()));
               if(send_len)
               {
                  socket_tx.copy(low_level_buff, (uint4)send_len);
                  rcd = mbedtls_ssl_write(&connection->context, low_level_buff, send_len);
                  if(rcd > 0)
                  {
                     event->socket->on_low_level_write(low_level_buff, rcd);
                     socket_tx.pop(rcd);
                     if(socket_tx.size() > 0)
                        continue;
                  }
                  else if(rcd != MBEDTLS_ERR_SSL_WANT_READ && rcd != MBEDTLS_ERR_SSL_WANT_READ)
                  {
                     connections->erase(ci);
                     event->socket->on_socket_error(0);
                     return;
                  }
               }
               
               // we need to attempt to read any available data
               rcd = mbedtls_ssl_read(&connection->context, low_level_buff, sizeof(low_level_buff));
               if(rcd > 0)
                  event->socket->on_low_level_read(low_level_buff, rcd);
               else if(rcd != MBEDTLS_ERR_SSL_WANT_READ && rcd != MBEDTLS_ERR_SSL_WANT_WRITE)
               {
                  connections->erase(ci);
                  event->socket->on_socket_error(0);
               }
            }
            connection->is_busy = false;
            if(connection->was_removed)
               connections->erase(ci);
         }
      }
   } // receive


   int TlsContext::do_write(SocketTcpSock *connection, void const *buff, size_t buff_len)
   {
      int rtn(MBEDTLS_ERR_NET_INVALID_CONTEXT);
      Protector<connections_type>::key_type connections(this->connections);
      connections_type::iterator ci(connections->find(connection));
      if(ci != connections->end())
      {
         connection_handle &context(ci->second);
         context->encrypted_tx.push(buff, (uint4)buff_len);
         rtn = static_cast<int>(buff_len);
         connection->on_tls_data_ready();
      }
      return rtn;
   } // do_write


   int TlsContext::do_read(SocketTcpSock *connection, void *buff, size_t buff_len)
   {
      int rtn(MBEDTLS_ERR_NET_INVALID_CONTEXT);
      Protector<connections_type>::key_type connections(this->connections);
      connections_type::iterator ci(connections->find(connection));
      if(ci != connections->end())
      {
         connection_handle &context(ci->second);
         if(context->encrypted_rx.size() > 0)
            rtn = static_cast<int>(context->encrypted_rx.pop(buff, (uint4)buff_len));
         else
            rtn = MBEDTLS_ERR_SSL_WANT_READ;
      }
      return rtn;
   } // do_read


   void TlsContext::on_socket_data(SocketTcpSock *socket, void const *buff, size_t buff_len)
   {
      Protector<connections_type>::key_type connections(this->connections);
      connections_type::iterator ci(connections->find(socket));
      if(ci != connections->end())
      {
         connection_handle &connection(ci->second);
         if(buff && buff_len > 0)
            connection->encrypted_rx.push(buff, (uint4)buff_len);
         if(!connection->read_pending)
         {
            connection->read_pending = true;
            event_socket_data::cpost(this, socket);
         }
      }
   } // on_socket_data


   ByteQueue *TlsContext::get_socket_write_buffer(SocketTcpSock *socket)
   {
      ByteQueue *rtn(0);
      Protector<connections_type>::key_type connections(this->connections);
      connections_type::iterator ci(connections->find(socket));
      if(ci != connections->end())
         rtn = &ci->second->encrypted_tx;
      return rtn;
   } // get_socket_write_buffer
};



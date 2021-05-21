/* Csi.Messaging.ProxyConn.h

   Copyright (C) 2014, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Monday 11 August 2014
   Last Change: Friday 03 June 2016
   Last Commit: $Date: 2016-06-08 12:40:35 -0600 (Wed, 08 Jun 2016) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_Messaging_ProxyConn_h
#define Csi_Messaging_ProxyConn_h

#include "Csi.Messaging.Connection.h"
#include "Csi.Messaging.Router.h"
#include "Csi.Messaging.Node.h"
#include "StrUni.h"
#include "StrBin.h"
#include <deque>


namespace Csi
{
   namespace Messaging
   {
      /**
       * Defines a class that acts as a connection through a messaging proxy server on the side of
       * both the messaging server as well as the messaging client.
       */
      class ProxyConn: public Connection, public Node
      {
      public:
         /**
          * Defines a constructor that can be used to connect to the proxy server by a messaging
          * client.
          *
          * @param proxy_address_ Specifies the address and optional port for the proxy server.
          *
          * @param proxy_account_ Specifies the name of the proxy account.
          *
          * @param proxy_password_ Specifies the password for the proxy account.
          */
         ProxyConn(
            StrAsc const &proxy_address_,
            StrAsc const &proxy_account_,
            StrUni const &proxy_password_);

         /**
          * Defines a constructor that can be used by a proxy listener to handle a virtual
          * connection notification.
          *
          * @param proxy_password_  Specifies the proxy password
          *
          * @param virtual_conn_id_ Specifies the virtual connection ID.
          *
          * @param remote_address_ Specifies the remote address.
          *
          * @param proxy_router_  Specifies the router used to communicate with the proxy.
          *
          * @param proxy_session_  Specifies the session number used to communicate with the proxy.
          */
         ProxyConn(
            StrAsc const &proxy_password_,
            uint4 virtual_conn_id_,
            StrAsc const &remote_address_,
            SharedPtr<Router> &proxy_router_,
            uint4 proxy_session_);

         /**
          * Destructor
          */
         virtual ~ProxyConn();

         /**
          * Overloads the base class version to make the connection to the proxy server.
          */
         virtual void attach();

         /**
          * Overloads the base class version to release the connection to the proxy server.
          */
         virtual void detach();

         /**
          * Overloads the base class version to transmit the specified message.
          */
         virtual void sendMessage(Message *message);

         /**
          * @return Overloads the base class version to return the remote address.
          */
         virtual StrAsc get_remote_address()
         { return remote_address; }

         /**
          * Overloads the base class version to handle the notification that a session has been
          * closed.
          */
         virtual void onNetSesBroken(
            Router *router,
            uint4 session_no,
            uint4 error_code,
            char const *error_message);

         /**
          * Overloads the base class version to handle the notification of an incoming message.
          */
         virtual void onNetMessage(Router *router, Message *message);

      private:
         /**
          * Initialises the AES encryption and decryption context structures.
          */
         void init_encryption();
         
         /**
          * Implements the code that sends any messages waiting in the message queue.
          */
         void do_send_messages();

         /**
          * Handles a client proxy challenge message.
          */
         void on_proxy_auth_challenge(Message *message);

         /**
          * Handles the client proxy acknowledgement message.
          */
         void on_proxy_auth_ack(Message *message);

         /**
          * Handles the connection forward command message.
          */
         void on_conn_forward_cmd(Message *message);

      private:
         /**
          * Specifies the router used to communicate with the proxy.
          */
         Csi::SharedPtr<Router> proxy_router;

         /**
          * Specifies the identifier for the virtual connection.
          */
         uint4 virtual_conn_id;

         /**
          * Specifies the session used for authentication with the proxy server.
          */
         uint4 auth_session;

         /**
          * Specifies the session number used for communication with the proxy.
          */
         uint4 proxy_session;

         /**
          * Specifies a list of message objects that are wating to be sent once the virtual
          * connection has been established.
          */
         typedef SharedPtr<Message> message_handle;
         typedef std::deque<message_handle> message_queue_type;
         message_queue_type message_queue;

         /**
          * Specifies the proxy address.
          */
         StrAsc const proxy_address;

         /**
          * Specifies the proxy account name.
          */
         StrAsc const proxy_account;

         /**
          * Specifies the proxy password.
          */
         StrAsc const proxy_password;

         /**
          * Reference to the structure used for encryption of outgoing messages.
          */
         byte *encrypt_context;

         /**
          * Reference to the structure used for descryption of incoming messages.
          */
         byte *decrypt_context;

         /**
          * Used to buffer data going into the encryption algorithm.
          */
         StrBin encrypt_input;

         /**
          * Used to buffer data coming out of the encryption algorithm.
          */
         StrBin encrypt_output;

         /**
          * Specifies the remote address for server side connections.
          */
         StrAsc remote_address;

         /**
          * Set to true if this connection is being used for a messaging server.
          */
         bool const for_server;
      };
   };
};


#endif

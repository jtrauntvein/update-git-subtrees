/* Csi.Messaging.ProxyListener.h

   Copyright (C) 2014, 2014 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Tuesday 12 August 2014
   Last Change: Monday 18 August 2014
   Last Commit: $Date: 2014-08-18 12:36:04 -0600 (Mon, 18 Aug 2014) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_Messaging_ProxyListener_h
#define Csi_Messaging_ProxyListener_h

#include "Csi.Messaging.Router.h"
#include "Csi.Messaging.Server.h"
#include "Csi.Messaging.Node.h"
#include "Csi.Messaging.ProxyConn.h"
#include "Csi.SocketTcpService.h"
#include <map>


namespace Csi
{
   namespace Messaging
   {
      /**
       * Defines a class the connects to CSI messaging proxy service and registers itself as a
       * messaging service.  Once registered, new virtual connections of type ProxyConn will be
       * created and passed to the default server object.
       */
      class ProxyListener: public Node
      {
      public:
         /**
          * Constructor
          *
          * @param proxy_address_  Specifies the address and optional port number for the messaging
          * proxy server.
          *
          * @param proxy_account_ Specifies the account name used to register with the proxy.
          *
          * @param proxy_password_ Specifies the password used to authenticate with the proxy.
          *
          * @param default_server_  Specifies the messaging server that will be notified of new
          * virtual connections when they are detected.
          *
          * @param logger_  Specifies an event receiver that will be notified of status events.
          */
         ProxyListener(
            StrAsc const &proxy_address_,
            StrAsc const &proxy_account_,
            StrUni const &proxy_password_,
            Server *default_server_,
            EventReceiver *logger_);

         /**
          * Destructor
          */
         virtual ~ProxyListener();

         // @group: messaging client methods

         /**
          * Overloads the base class version to handle the case when a session has been reported
          * closed or broken.
          */
         virtual void onNetSesBroken(
            Router *router,
            uint4 session_no,
            uint4 error_code,
            char const *error);

         /**
          * Overloads the base class version to handle the case when a message has been received.
          */
         virtual void onNetMessage(
            Router *router, Message *message);
         
         // @endgroup:

      private:
         /**
          * Handles the server logon challenge
          */
         void on_logon_challenge(Message *message);

         /**
          * Handles the server logon ack.
          */
         void on_logon_ack(Message *message);

         /**
          * Handles the virtual connection notification.
          */
         void on_virtual_conn_not(Message *message);

         /**
          * Handles the forward message command.
          */
         void on_forward_cmd(Message *message);

         /**
          * Handles the virtual connection close command./
          */
         void on_virtual_conn_close_cmd(Message *message);

         /**
          * Handles the register server acknowledgement.
          */
         void on_register_server_ack(Message *message);

         /**
          * Responsible for generating event log messages to the logger object.
          *
          * @param message Specifies the message that will be logged.
          */
         void log_event(StrAsc const &message);

         /**
          * Reports a failure for this listener
          *
          * @param message Specifies the message that will be reported to the logger.
          */
         void report_failure(StrAsc const &message);

      private:
         /**
          * Specifies the messaging router used to communicate with the proxy server.
          */
         SharedPtr<Router> proxy_router;

         /**
          * Specifies the session used to authenticate with the proxy router.
          */
         uint4 auth_session;

         /**
          * Specifies the session used to communicate with the proxy object.
          */
         uint4 proxy_session;

         /**
          * Specifies the address and, optionally, the TCP por of the proxy server.
          */
         StrAsc const proxy_address;

         /**
          * Specifies the proxy account name.
          */
         StrAsc const proxy_account;

         /**
          * Specifies the proxy account password in UTF-8 encoding.
          */
         StrAsc const proxy_password;

         /**
          * Specifies the event reciever that will handle logged events.
          */
         EventReceiver *logger;

         /**
          * Specifies the server object that will receive notifications of new connections.
          */
         Server *default_server;

         /**
          * Specifies the set of connections that were created by this listener keyed by their
          * virtual connection number.
          */
         typedef std::map<uint4, ProxyConn *> connections_type;
         connections_type connections;
      };
   };
};

#endif


/* Cora.Posix.DllNetConn.h

   Copyright (C) 2020, 2020 Campbell Scientific, Inc.

   Written by: Andrew Mortenson
   Date Begun: Tuesday 20 May 2020
   Last Change: Friday 07 August 2020
   Last Commit: $Date: 2020-08-07 11:07:32 -0600 (Fri, 07 Aug 2020) $
   Committed by: $Author: jon $
   
*/

#ifndef Cora_Posix_DllNetConn_h
#define Cora_Posix_DllNetConn_h

#include "Csi.Messaging.Connection.h"
#include "Csi.Messaging.Message.h"
#include "CoraDllMessagingDefs.h"
#include "Csi.Events.h"

//@group class forward declarations
namespace Csi
{
   namespace Messaging { class Server; };
};
//@endgroup

namespace Cora
{
   namespace Posix
   {
      /**
       * Defines a connection derived component that uses the DLL messaging interface to send and
       * receive messages using DLL function calls to the LoggerNet server.  Internally, this
       * component uses a DllNetConnManager to help interact with the DLL interface and to keep
       * track of connection instances on both the client and server sides.
       */
      class DllNetConn:
         public Csi::Messaging::Connection,
         public Csi::EventReceiver
      {
      public:
         /**
          * Defines the class of event that will be posted when a message has been received.
          */
         class event_message_received: public Csi::Event
         {
         public:
            static uint4 const event_id;
            Csi::Messaging::Message message;
            static void create_and_post(DllNetConn *conn, void const *buff, uint4 buff_len);

         private:
            event_message_received(DllNetConn *conn, void const *buff, uint4 buff_len);
         };

         /**
          * Defines the class of event that will be posted when a connection cancelled event is
          * received.
          */
         class event_connection_cancelled: public Csi::Event
         {
         public:
            static uint4 const event_id;
            static void create_and_post(DllNetConn *conn);

         private:
            event_connection_cancelled(DllNetConn *conn);
         };

         /**
          * Defines the class of event that is posted when a server connection is created using the
          * server constructor.
          */
         class event_server_created: public Csi::Event
         {
         public:
            static uint4 const event_id;
            Csi::Messaging::Server *server;
            static void create_and_post(DllNetConn *conn, Csi::Messaging::Server *server);

         private:
            event_server_created(DllNetConn *conn, Csi::Messaging::Server *server_);
         };


      public:
         /**
          * Client (default) constructor
          *
          * @param coralib_module_ Specifies the handle to the DLL module for the coralib server
          * DLL.
          */
         DllNetConn(void* coralib_module_);

         /**
          * Server constructor
          *
          * @param coralib_module_ Specifies the module handle to the server DLL.
          *
          * @param send_message_func_ Specifies a function pointer passed by the client.
          *
          * @param server Specifies the server object.
          */
         DllNetConn(
            void* coralib_module_,
            cora_messaging_callback_type send_message_func_,
            Csi::Messaging::Server *server);

         /**
          * Destructor
          */
         virtual ~DllNetConn();

         /**
          * Overrides the base class version to create the connection.
          */
         virtual void attach() override;

         /**
          * Overrides the base class version to break the connection.
          */
         virtual void detach() override;

         /**
          * Overrides the base class version to transmit the specified message.
          */
         virtual void sendMessage(Csi::Messaging::Message *msg) override;

         /**
          * @return Overrides the base class version to indicate that the peer is not remote.
          */
         virtual bool peer_is_remote() override
         { return false; }

         /**
          * @return Returns an eight byte integer that uniquely identifies this connection.
          */
         uint8 get_identifier() const
         { return identifier; }

         /**
          * @return Returns true if this connection was created using the server constructor.
          */
         bool get_is_server() const { return is_server; }

         /**
          * Overrides the base class version to handle asynchronous events.
          */
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

      private:
         /**
          * A pointer to a function on the peer side that will be invoked when an event must be sent
          * to the other side of the connection.
          */
         cora_messaging_callback_type send_message_func;

         /**
          * A four byte integer that uniquely identifies this connection object.  It is changed when
          * attach() is called to take on the value returned by cora_start_connection().
          */
         uint8 identifier;

         /**
          * Set to true if this object was created by the server constructor.
          */
         bool is_server;

         /**
          * Set to true if there is an active connection in an attached state.
          */
         bool is_attached;

         /**
          * Handle to the coralib DLL module.  Thjis is used to dynamically look up the function
          * names in the cora server DLL.
          */
         void *coralib_module;
      };
   };
};

#endif

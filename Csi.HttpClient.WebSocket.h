/* Csi.HttpClient.WebSocket.h

   Copyright (C) 2015, 2015 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Wednesday 21 October 2015
   Last Change: Saturday 14 November 2015
   Last Commit: $Date: 2017-04-03 16:25:43 -0600 (Mon, 03 Apr 2017) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_HttpClient_WebSocket_h
#define Csi_HttpClient_WebSocket_h

#include "Csi.HttpClient.h"


namespace Csi
{
   namespace HttpClient
   {
      class WebSocket;

      /**
       * Defines the message op codes that can be used.
       */
      enum websock_op_code
      {
         websock_op_continuation = 0,
         websock_op_text = 1,
         websock_op_binary = 2,
         websock_op_close = 8,
         websock_op_ping = 9,
         websock_op_pong = 10
      };

      
      /**
       * Defines the interface that the application must implement in order to
       * use the {@link WebSocket} component.
       */
      class WebSocketClient: public Csi::InstanceValidator
      {
      public:
         /**
          * Called when the websocket connection has been made.  The
          * application must wait for this notification before it attemps to
          * send and messages to the server.
          *
          * @param sender Specifies the web socket that it is sending this
          * notification.
          */
         virtual void on_connected(WebSocket *sender) = 0;

         /**
          * Called when a failure has occurred with the web socket.
          *
          * @param sender Specifies the component reporting this failure.
          *
          * @param failure Specifies the type of failure that has occurred.
          *
          * @param http_response Specifies the HTTP response code.
          */
         enum failure_type
         {
            failure_unknown = -1,
            failure_connect,
            failure_unsupported,
            failure_security,
            failure_server_closed
         };
         virtual void on_failure(
            WebSocket *sender, failure_type failure, int http_response) = 0;

         /**
          * Called when a message or message fragment has been received from
          * the server.
          *
          * @param sender Specifies the component reporting this event.
          *
          * @param content Specifies the start of the message content.
          *
          * @param content_len Specifies the number of bytes for this message.
          *
          * @param op_code Specifies the op code for this message.
          *
          * @param fin Set to true if this is the last fragment of the message.
          */
         virtual void on_message(
            WebSocket *sender,
            void const *content,
            uint4 content_len,
            websock_op_code op_code,
            bool fin) = 0;
      };

      
      /**
       * Defines a component that acts as a web socket client. In order to use
       * this component, the application must provide an object that extends
       * class WebSocketClient.  It should create an instance of this class and
       * then call {@link WebSocket#connect}.
       */
      class WebSocket: public RequestClient, public Upgrade, public EventReceiver, public OneShotClient
      {
      public:
         /**
          * Default Constructor
          *
          * @param timer_ Specifies the one shot timer shared between this component, the
          * connection, and the application.
          *
          * @param ping_interval_ Specifies the interval at which pings will be sent for an inactive connection.
          */
         typedef SharedPtr<OneShot> timer_handle;
         WebSocket(timer_handle timer_ = 0, uint4 ping_interval_ = 60000);

         /**
          * Destructor
          */
         virtual ~WebSocket();

         /**
          * Starts the connection process for this web socket.  Will throw an
          * exception derived from std::exception if this component is in an
          * invalid state or any of the parameters are invalid.
          *
          * @param client_ Specifies the reference to the client object.
          *
          * @param uri_ Specifies the URI for the server to which we will
          * connect.  This value must specify a protocol of "ws" or "wss".
          *
          * @param auth_name Specifies the authorisation user name.
          *
          * @param auth_password Specifies the authorisation password.
          */
         void connect(
            WebSocketClient *client_,
            Uri const &uri_,
            StrAsc const &protocol,
            StrAsc const &auth_name = "",
            StrAsc const &auth_password = "");

         /**
          * Closes any resources owned by this web socket.
          */
         void close();

         /**
          * Overloads the base class version to handle a failed HTTP request.
          */
         virtual bool on_failure(Request *sender);

         /**
          * Overloads the base class version to handle a completed HTTP
          * request.
          */
         virtual bool on_response_complete(Request *sender); 

         /**
          * Called to transmit a message or message fragment to the server.
          *
          * @param content Specifies the content of the message or message fragment.
          *
          * @param content_len Specifies the length of this message or message
          * fragment.
          *
          * @param op_code Specifies the operation code for this message or
          * message fragment.
          *
          * @param fin Set to true if this is the last fragment of the message.
          *
          * @throw std::exception Throws a std::exception if this component is
          * not in a state to send a message.
          */
         void send_message(
            void const *content,
            uint4 content_len,
            websock_op_code op_code = websock_op_text,
            bool fin = true);

         /**
          * Overloads the base class version to handle posted events.
          */
         virtual void receive(SharedPtr<Event> &ev);

         /**
          * Overloads the base class version to handle received data from the connection.
          */
         virtual void on_read(Connection *sender);

         /**
          * Overloads the error received event from the connection.
          */
         virtual void on_error(Connection *sender, int error_code);

         /**
          * @param watcher_ Specifies the application object that will watch the message I/O for
          * this connection.
          */
         typedef SharedPtr<ConnectionWatcher> watcher_handle;
         void set_watcher(watcher_handle watcher_);

         /**
          * Overloads the handler for timer events.
          */
         virtual void onOneShotFired(uint4 id);

      private:
         /**
          * Used to report an error to the client.
          */
         typedef WebSocketClient::failure_type failure_type;
         void do_report_error(failure_type failure, int http_resp_code);

         /**
          * Handles a received message.
          */
         void do_on_message(uint2 op_code, bool finished);
         
      private:
         /**
          * Specifies the state of this web socket.
          */
         enum state_type
         {
            state_standby,
            state_connecting,
            state_before_frame,
            state_read_len,
            state_read_mask,
            state_read_payload
         } state;

         /**
          * Specifies the connection to the server.
          */
         SharedPtr<Connection> connection;

         /**
          * Specifies the request used to negotiate the web socket.
          */
         SharedPtr<Request> request;

         /**
          * Specifies the URI for this web socket.
          */
         Uri uri;

         /**
          * Specifies the client for this component.
          */
         WebSocketClient *client;

         /**
          * Specifies the payload for the current message.
          */
         StrBin payload;

         /**
          * Specifies the expected length of the current payload.
          */
         int8 payload_len;

         /**
          * Specifies the last header that was received.
          */
         uint2 last_header;

         /**
          * Specifies the mask used for the current incoming message.
          */
         byte rx_mask[4];

         /**
          * Buffers the last message sent.
          */
         StrBin tx_payload;

         /**
          * Used to buffer messages that have not been completed.
          */
         StrBin complete_buffer;

         /**
          * Specifies the op code that was received when the first fragment was received.
          */
         websock_op_code fragmented_op_code;

         /**
          * Specifies the low level logger for this component.
          */
         watcher_handle watcher;

         /**
          * Specifies the timer used for the connection and this object's own watch dog.
          */
         timer_handle timer;

         /**
          * Specifies the identifier for the watch dog timer for this web socket.
          */
         uint4 watchdog_id;

         /**
          * Keeps track of the number of pings that have been sent.
          */
         uint4 pings_count;

         /**
          * Specifies the maximum interval for timing pings to test an inactive connection.
          */
         uint4 ping_interval;
      };
   };
};


#endif

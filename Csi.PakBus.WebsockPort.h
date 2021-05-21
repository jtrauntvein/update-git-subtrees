/* Csi.PakBus.WebsockPort.h

   Copyright (C) 2020, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Saturday 29 February 2020
   Last Change: Thursday 05 March 2020
   Last Commit: $Date: 2020-03-05 09:53:24 -0600 (Thu, 05 Mar 2020) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_PakBus_WebsockPort_h
#define Csi_PakBus_WebsockPort_h
#include "Csi.PakBus.PortBase.h"
#include "Csi.HttpClient.WebSocket.h"
#include "OneShot.h"
#include <map>
#include <deque>


namespace Csi
{
   namespace PakBus
   {
      /**
       * Defines an object that can act as a port for PakBuds communication using websocket
       * services.
       */
      class WebsockPort:
         public PortBase,
         public HttpClient::WebSocketClient,
         public OneShotClient
      {
      private:
         /**
          * Specifies the URL used to contact the PakBus/WS server.
          */
         StrAsc const ws_url;

         /**
          * Specifies the network ID that will be used when connecting the the server.
          */
         StrAsc const network_id;

         /**
          * Specifies the websocket that will be used for communication.
          */
         typedef HttpClient::WebSocket server_type;
         typedef SharedPtr<server_type> server_handle;
         server_handle server;

         /**
          * Set to true if the websocket is in a connected state.
          */
         bool server_connected;

         /**
          * Specifies the currently assigned pakBus router.
          */
         router_handle router;

         /**
          * Keeps track of the list of pakbus sessions that are currently active.
          */
         typedef std::map<uint4, uint4> sessions_type;
         sessions_type sessions;

         /**
          * Keeps track of the broadcast messages that are waiting for the port to be opened.
          */
         typedef std::deque<message_handle> pending_messages_type;
         pending_messages_type pending_broadcasts;

         /**
          * Keeps track of all of the non-broadcast messages that are waiting for the port.
          */
         pending_messages_type pending_messages;

         /**
          * Identifies the timer that will drive attempts to retry connections to the PakBus/WS
          * server.
          */
         uint4 retry_id;

         /**
          * Specifies the object used for timing events.
          */
         Csi::SharedPtr<OneShot> timer;

         /**
          * Used to buffer frames before they are transmitted.
          */
         StrBin frame_buffer;

         /**
          * Identifies the timer used to send beacons.
          */
         uint4 beacon_id;

      public:
         /**
          * Constructor
          *
          * @param network_id_ Specifies the network identifier that will be used when connecting to
          * the server.
          *
          * @param ws_url_ Specifies the URL used to connect to the PakBus/WS server.
          */
         WebsockPort(StrAsc const &network_id_, StrAsc const &ws_url = "ws://localhost:8080");

         /**
          * Destructor
          */
         virtual ~WebsockPort();

         /**
          * Overloads the base class version to handle the case where a message is ready to be sent.
          */
         virtual void on_message_ready(uint2 phys_dest, priority_type priority) override;

         /**
          * Overloads the base class version to handle a request to broadcast a message.
          */
         virtual void broadcast_message(message_handle &message) override;

         /**
          * Overloads the base class version to handle a notification that a message has been
          * aborted.
          */
         virtual void on_message_aborted(uint2 phys_dest) override;

         /**
          * Overloads the base class version to return the beacon interval.
          */
         virtual uint2 get_beacon_interval() override;

         /**
          * Overloads the base class version to return the expected worst case response interval.
          */
         virtual uint4 get_worst_case_response() override;

         /**
          * Overloads the base class version to evaluate whether an session between the two
          * addresses is active.
          */
         virtual bool has_session(uint2 source, uint2 dest) override;

         /**
          * Overloads the base class version to return false to indicate that the link is not
          * dialed.
          */
         virtual bool link_is_dialed() override
         { return false; } 

         /**
          * Overloads the base class version to return true if the link is currently active.
          */
         virtual bool link_is_active() override;

         /**
          * Overloads the base class version to return true if the link must be closed.
          */
         virtual bool must_close_link() override;

         /**
          * Overloads the base class version to handle the case where a neighbour has been lost.
          */
         virtual void on_neighbour_lost(uint2 phys_address) override;

         /**
          * Overloads the base class version to reset the session timer between the two addresses.
          */
         virtual void reset_session_timer(uint2 source, uint2 dest) override;

         /**
          * Overloads the base class version to return  the port name.
          */
         virtual StrAsc get_port_name() const override;

         /**
          * Overloads the base class version to set the pakBus router.
          */
         virtual void set_pakbus_router(router_handle router_) override;

         /**
          * Overloads the base class version to return the PakBus router.
          */
         virtual router_handle &get_pakbus_router() override;

         /**
          * Overloads the base class version to handle the notification that the websocket is
          * connected.
          */
         virtual void on_connected(server_type *sender) override;

         /**
          * Overloads the base class version to handle the notification that the websocket has
          * failed.
          */
         virtual void on_failure(server_type *sender, failure_type failure, int http_response) override;

         /**
          * Overloads the base class version to handle an incoming websocket message.
          */
         virtual void on_message(
            server_type *sender,
            void const *content,
            uint4 content_len,
            HttpClient::websock_op_code op_code,
            bool fin) override;

         /**
          * Overloads the base class version to handle timed events.
          */
         virtual void onOneShotFired(uint4 id) override;

      private:
         /**
          * Transmits the specified message through the link.
          */
         void send_message(message_handle &message);

         /**
          * Starts the attempt to connect to the server.
          */
         void start_connect();

         /**
          * Updates the sessions based upon the specified message header.
          */
         void update_expect_more(message_handle &message);

         /**
          * Transmits a beacon if beaconing is enabled for this port.
          */
         void send_beacon();
      };
   };
};


#endif

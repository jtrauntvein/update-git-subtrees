/* Csi.ToolbarCommunicator.h

   Copyright (C) 2002, 2013 Campbell Scientific, Inc.

   Written by: Tyler Mecham
   Date Begun: 4 April 2002
   Last Change: Monday 22 April 2013
   Last Commit: $Date: 2013-04-22 17:42:06 -0600 (Mon, 22 Apr 2013) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Csi_ToolbarCommunicator_h
#define Csi_ToolbarCommunicator_h

#include "Csi.Messaging.Node.h"
#include "Csi.Messaging.Router.h"
#include "Csi.Messaging.Connection.h"
#include "StrAsc.h"
#include "StrUni.h"
#include "Csi.SharedPtr.h"


namespace Csi
{
   class ToolbarCommunicator;

   namespace ForwardedToolbarMessage
   {
      enum msg_types
      {
         rtdaq_connect_note = 3,
         rtmc_file_loaded = 4
      };


      ///////////////////////////////////////////////////////////
      // class ForwardedMessage
      ///////////////////////////////////////////////////////////
      class ForwardedMessage
      {
      public:
         ///////////////////////////////////////////////////////////
         // get_msg_type
         //
         // get the forwarded_msg_type from the derived message
         ///////////////////////////////////////////////////////////
         virtual uint4 get_msg_type() = 0;
      };

      ///////////////////////////////////////////////////////////
      // RTMCFileOpenMessage
      ///////////////////////////////////////////////////////////
      class RTMCFileOpenMessage:
         public ForwardedMessage
      {
      public:
         ///////////////////////////////////////////////////////////
         // Constructor
         ///////////////////////////////////////////////////////////
         RTMCFileOpenMessage(StrAsc const &file_name_):
            file_name(file_name_)
         { }

         ///////////////////////////////////////////////////////////
         // get_msg_type
         ///////////////////////////////////////////////////////////
         virtual uint4 get_msg_type()
         { return rtmc_file_loaded; }

         ///////////////////////////////////////////////////////////
         // get_file_name
         ///////////////////////////////////////////////////////////
         StrAsc const &get_file_name()
         { return file_name; }

      private:
         ///////////////////////////////////////////////////////////
         // file_name
         ///////////////////////////////////////////////////////////
         StrAsc file_name;
      };

      ///////////////////////////////////////////////////////////
      // class RTDAQConnectMessage
      ///////////////////////////////////////////////////////////
      class RTDAQConnectMessage: 
         public ForwardedMessage
      {
      public:
         RTDAQConnectMessage(uint4 state, StrAsc const &station):
            connection_state(state),
            station_name(station)
         { }

         ///////////////////////////////////////////////////////////
         // get_msg_type
         ///////////////////////////////////////////////////////////
         virtual uint4 get_msg_type()
         { return rtdaq_connect_note; }

         ///////////////////////////////////////////////////////////
         // get_connection_state
         ///////////////////////////////////////////////////////////
         uint4 get_connection_state()
         { return connection_state; }

         ///////////////////////////////////////////////////////////
         // const 
         ///////////////////////////////////////////////////////////
         StrAsc const &get_station_name()
         { return station_name; }

      private:
         ///////////////////////////////////////////////////////////
         // connection_state
         //
         // enum{0=Disconnected, 1=Connecting, 2=Connected, 3=Reconnecting}
         ///////////////////////////////////////////////////////////
         uint4 connection_state;

         ///////////////////////////////////////////////////////////
         // station_name
         ///////////////////////////////////////////////////////////
         StrAsc station_name;
      };
   };

   ////////////////////////////////////////////////////////////
   // class ToolbarCommunicatorClient
   //
   // Defines the interface a client must define if they are
   // going to be controlled by the loggernet toolbar.
   ////////////////////////////////////////////////////////////
   class ToolbarCommunicatorClient: public Csi::EventReceiver
   {
   public:
      ////////////////////////////////////////////////////////////
      // on_started
      //
      // We were able to successfully register with the toolbar
      // and can now receive toolbar events
      ////////////////////////////////////////////////////////////
      virtual void on_started(ToolbarCommunicator *tran)
      { }
      
      ////////////////////////////////////////////////////////////
      // on_hide
      //
      // Return true if you can minimize your app
      ////////////////////////////////////////////////////////////
      virtual bool on_hide(ToolbarCommunicator *tran) = 0;
      
      ////////////////////////////////////////////////////////////
      // on_show
      //
      // Return true if you can show your app and bring to front
      ////////////////////////////////////////////////////////////
      virtual bool on_show(ToolbarCommunicator *tran) = 0;
      
      ////////////////////////////////////////////////////////////
      // on_restore
      //
      // Return true if you can restore and bring app to the front
      ////////////////////////////////////////////////////////////
      virtual bool on_restore(ToolbarCommunicator *tran) = 0;
      
      ////////////////////////////////////////////////////////////
      // on_close
      //
      // Return true if your app can close
      ////////////////////////////////////////////////////////////
      virtual bool on_close(ToolbarCommunicator *tran) = 0;

      ////////////////////////////////////////////////////////////
      // send_close_resp_now
      //
      // Called when on_close() has returned false (don't allow the close) but
      // the communicator should delay sending the response until the client
      // invokes the communicator's continue_close() method. 
      ////////////////////////////////////////////////////////////
      virtual bool send_close_resp_now(ToolbarCommunicator *communicator)
      { return true; }
      
      ////////////////////////////////////////////////////////////
      // on_received_working_directory
      //
      // Received in response to registering ourselves with the toolbar
      ////////////////////////////////////////////////////////////
      virtual void on_received_working_directory(
         ToolbarCommunicator *tran, 
         StrAsc const &work_dir)
      { }
      
      ////////////////////////////////////////////////////////////
      // on_forwarded_msg
      //
      // Recieved a forwarded message from another client
      ////////////////////////////////////////////////////////////
      virtual void on_forwarded_msg(
         ToolbarCommunicator *tran,
         ForwardedToolbarMessage::ForwardedMessage *msg)
      { }
      
      ////////////////////////////////////////////////////////////
      // on_failure
      ////////////////////////////////////////////////////////////
      enum failure_type
      {
         failure_unknown = 0,
         failure_connection_failed = 1,
         failure_registration_failed = 2,
         failure_forward_message_failed = 3,
      };
      virtual void on_failure(ToolbarCommunicator *tran, failure_type failure)
      {}

      ////////////////////////////////////////////////////////////
      // receive
      ////////////////////////////////////////////////////////////
      virtual void receive(SharedPtr<Event> &ev)
      { }
   };


   ////////////////////////////////////////////////////////////
   // class exc_invalid_state
   //
   // Defines the class of object that will be thrown (as an exception) when an operation (such
   // as property set) is attempted on an object of class derived from this class when the state
   // is invalid.
   ////////////////////////////////////////////////////////////
   class exc_invalid_state: public std::exception
   {
   public:
      virtual char const *what() const throw ()
      { 
         return "ToolbarCommunicatorClient::Invalid state for attempted operation"; 
      }
   };

   ////////////////////////////////////////////////////////////
   // class ToolbarCommunicator
   ////////////////////////////////////////////////////////////
   class ToolbarCommunicator : public Csi::Messaging::Node  
   {
   public:
      ////////////////////////////////////////////////////////////
      // constructor
      ////////////////////////////////////////////////////////////
      ToolbarCommunicator();
      
      ////////////////////////////////////////////////////////////
      // destructor
      ////////////////////////////////////////////////////////////
      virtual ~ToolbarCommunicator();
      
      ////////////////////////////////////////////////////////////
      // start
      ////////////////////////////////////////////////////////////
      typedef Csi::SharedPtr<Csi::Messaging::Router> router_handle;
      void start(ToolbarCommunicatorClient *client_, router_handle &router_);
      
      ////////////////////////////////////////////////////////////
      // finish
      ////////////////////////////////////////////////////////////
      void finish();
      
      ////////////////////////////////////////////////////////////
      // set_client_name
      //
      // Set the client name that is to be used when registering
      // with the toolbar
      ////////////////////////////////////////////////////////////
      void set_client_name(StrAsc const &client_name_);
      
      ////////////////////////////////////////////////////////////
      // start_enumerate_clients()
      //
      // Begins enumerating clients registered with the toolbar.
      // This is an ongoing transaction, so the client will need
      // to stop this if they don't want to receive notifications.
      ////////////////////////////////////////////////////////////
      void start_enumerate_clients();
      
      ////////////////////////////////////////////////////////////
      // stop_enumerate_clients
      // 
      // Stop enumerating the clients that register with the toolbar
      ////////////////////////////////////////////////////////////
      void stop_enumerate_clients();
      
      ////////////////////////////////////////////////////////////
      // forward_message
      //
      // Forward a message to a client registered with the toolbar
      ////////////////////////////////////////////////////////////
      void forward_message(
         ToolbarCommunicatorClient *client,
         StrAsc const &client_name,
         ForwardedToolbarMessage::ForwardedMessage *msg);
      
      //@group Csi::Messaging::Node Overrides
      ////////////////////////////////////////////////////////////
      // onNetSesBroker
      //
      // Called when a session that had been previously opened is no longer viable
      ////////////////////////////////////////////////////////////
      virtual void onNetSesBroken(
         Csi::Messaging::Router *router,
         uint4 session_no,
         uint4 error_code,
         char const *error_message);

      ////////////////////////////////////////////////////////////
      // onNetMessage()
      // 
      // Called by the router when a message on the session associated with this node has been
      // received
      ////////////////////////////////////////////////////////////
      virtual void onNetMessage(
         Csi::Messaging::Router *router,
         Csi::Messaging::Message *message);
      //@endgroup

      ////////////////////////////////////////////////////////////
      // should_allow_remote_connections
      ////////////////////////////////////////////////////////////
      bool get_allow_remote_connections() const
      { return allow_remote_connections; }

      ////////////////////////////////////////////////////////////
      // get_local_server_running
      ////////////////////////////////////////////////////////////
      bool get_local_server_running() const
      { return local_server_running; }

      ////////////////////////////////////////////////////////////
      // get_local_port
      ////////////////////////////////////////////////////////////
      uint2 get_local_port() const
      { return local_port; }

      ////////////////////////////////////////////////////////////
      // continue_close
      //
      // Called by the application after a delayed close event can now be
      // honoured. 
      ////////////////////////////////////////////////////////////
      void continue_close();

      ////////////////////////////////////////////////////////////
      // get_close_resp_pending
      ////////////////////////////////////////////////////////////
      bool get_close_resp_pending() const
      { return close_resp_pending; }

   private:
      ////////////////////////////////////////////////////////////
      // session_no
      ////////////////////////////////////////////////////////////
      uint4 session_no;

      ///////////////////////////////////////////////////////////
      // tran_no
      ///////////////////////////////////////////////////////////
      uint4 tran_no;
      
      ////////////////////////////////////////////////////////////
      // router
      ////////////////////////////////////////////////////////////
      router_handle router;
      
      ////////////////////////////////////////////////////////////
      // client_name
      ////////////////////////////////////////////////////////////
      StrAsc client_name;
      
      ////////////////////////////////////////////////////////////
      // client
      ////////////////////////////////////////////////////////////
      ToolbarCommunicatorClient *client;
      
      ////////////////////////////////////////////////////////////
      // state
      ////////////////////////////////////////////////////////////
      enum state_type
      {
         state_standby,
         state_registering,
         state_ready,
      } state;

      ////////////////////////////////////////////////////////////
      // local_port
      //
      // Specifies the port of the local server. 
      ////////////////////////////////////////////////////////////
      uint2 local_port;
      
      ////////////////////////////////////////////////////////////
      // allow_remote_connections
      //
      // Set to true if the toolbar will allow us to log onto remote servers. 
      ////////////////////////////////////////////////////////////
      bool allow_remote_connections;

      ////////////////////////////////////////////////////////////
      // local_server_running
      ////////////////////////////////////////////////////////////
      bool local_server_running;

      ////////////////////////////////////////////////////////////
      // close_resp_pending
      ////////////////////////////////////////////////////////////
      bool close_resp_pending;
      
      ////////////////////////////////////////////////////////////
      // message type defs
      ////////////////////////////////////////////////////////////
      enum message_types
      {
         reg_client_cmd = 1501,
         reg_client_ack = 1502,
         set_window_state_cmd = 1503,
         set_window_state_ack = 1504,
         close_cmd = 1505,
         close_resp = 1506,
         enum_clients_start_cmd = 1509,
         enum_clients_note = 1510,
         enum_clients_stop_cmd = 1511,
         enum_clients_stop_ack = 1512,
         forward_msg_cmd = 1515,
         forward_msg_note = 1516,
         forward_msg_ack = 1517,
         forward_msg_note_ack = 1518,
         restore_client_cmd = 1519,
         restore_client_ack = 1520,
         close_ack = 1521
      };
   };
};

#endif // Csi_ToolbarCommunicator_h

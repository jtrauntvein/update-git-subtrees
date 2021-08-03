/* Cora.Device.ConnectionManager.h

   Copyright (C) 2000, 2009 Campbell Scientific, Inc.

   Written by: Carl Zmola, Revised by Jon Trauntvein
   Date Begun: Wednesday 05 July 2000
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Committed by: $Author: jon $
   
*/


#ifndef Cora_Device_ConnectionManager_h
#define Cora_Device_ConnectionManager_h


#include "Cora.Device.DeviceBase.h"
#include "CsiEvents.h"


namespace Cora
{
   namespace Device
   {
      //@group class forward declarations
      class ConnectionManager; 
      //@endgroup

      ////////////////////////////////////////////////////////////
      // class ConnectionManagerClient
      //
      // Defines the methods that should be implemented by a connection manager client
      ////////////////////////////////////////////////////////////
      class ConnectionManagerClient: public Csi:: InstanceValidator
      {
      public:
         ////////// on_connection_manager_started
         // Called once the transaction has been successfully started.
         virtual void on_started(
            ConnectionManager *manager)
         { }

         ////////// on_connection_manager_state_change
         // Called when the device connection state changes. The parameter will be true if the
         // device is on-line by the server's definition. Note that the server defines a device as
         // being on-line if all of its parents are in transparent mode. This does not mean that the
         // device is present or ready to communicate itself.
         virtual void on_state_change(
            ConnectionManager *manager,
            bool is_online)
         { }

         ////////// on_connection_manager_failure
         enum failure_type
         {
            failure_unknown = 0,
            failure_unexpected = 1,
            failure_connection_failed = 2,
            failure_invalid_logon = 3,
            failure_server_security_blocked = 4,
            failure_device_name_invalid = 5,
            failure_server_terminated_transaction = 6,
            failure_device_does_not_support = 7,
            failure_path_does_not_support = 8
         };
         virtual void on_failure(
            ConnectionManager *manager,
            failure_type failure,
            StrUni const &next_candidate) = 0;
      };


      ////////////////////////////////////////////////////////////
      // class ConnectionManager
      //
      // Defines an object that encapsulates the Device Connection Management Transaction. A client
      // object should use this class by creating an instance of it, setting the appropropriate
      // properties, and calling start().
      //
      // As connection management events are recieved, these will be passed to the client object
      // through the ConnectionManagerClient interface.
      //
      // When the transaction should be ended, the client can call the finish() method which will
      // place this object into a standby state where the property set and start() methods can again
      // be called.
      ////////////////////////////////////////////////////////////
      class ConnectionManager: public DeviceBase, public Csi::EvReceiver
      {
      public:
         ////////// enum priority_type
         // Lists the possible values of the priority property
         enum priority_type
         {
            priority_high = 0,
            priority_normal = 1,
            priority_low = 2,
         };
         
      private:
         //@group properties
         ////////// priority
         // Corresponds with the priority parameter of the start command
         priority_type priority;

         ////////// force_open
         // Corresponds with the force_open parameter in the start command
         bool force_open;

         ////////// keep_open
         // Corresponds with the keep_open parameter in the start command
         bool keep_open;
         //@endgroup

      public:
         ////////// constructor
         ConnectionManager();

         ////////// destructor
         virtual ~ConnectionManager();

         //@group property access methods
         ////////// set_priority
         void set_priority(priority_type priority_);

         ////////// set_keep_open
         void set_keep_open(bool keep_open_);

         ////////// set_force_open
         void set_force_open(bool force_open_);
         //@endgroup

         ////////// start
         // Called to begin the connection management transaction using the property value already
         // (hopefully) assigned.
         void start(
            ConnectionManagerClient *client_,
            router_handle &router);
         void start(
            ConnectionManagerClient *client_,
            ClientBase *other_component);
         
         ////////// finish
         // Called to end the connection managerment transaction, release whatever server resources
         // claimed, and to place this object back into a standby (startable) state.
         void finish();
         
      protected:
         ////////// onNetMessage
         virtual void onNetMessage(Csi::Messaging::Router *router, 
                                   Csi::Messaging::Message *message);

         //@group methods overloaded from class DeviceBase
         virtual void on_devicebase_failure(devicebase_failure_type failure);
         virtual void on_devicebase_session_failure();
         virtual void on_devicebase_ready();
         //@endgroup

         ////////// on_start_ack
         void on_start_ack(Csi::Messaging::Message *message);

         ////////// on_status_notification
         void on_status_notification(Csi::Messaging::Message *message);
         
         ////////// receive
         void receive(Csi::SharedPtr<Csi::Event> &ev);

      private:
         ////////// state
         // records the internal state of this object
         enum state_type
         {
            state_standby,      // ready to be started
            state_delegate,     // waiting for logon and attach
            state_local,        // process events locally
         } state;

         ////////// client
         // The object that will recieve event notifications
         ConnectionManagerClient *client;
      }; // ConnectionManager
      

   };
};


#endif

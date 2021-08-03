/* Cora.Device.CommResourceManager.h

   Copyright (C) 2001, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 05 July 2001
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Cora_Device_CommResourceManager_h
#define Cora_Device_CommResourceManager_h

#include "Cora.Device.ConnectionManager.h"
#include "Csi.InstanceValidator.h"
#include "Csi.Events.h"


namespace Cora
{
   namespace Device
   {
      //@group class forward declarations
      class CommResourceManager; 
      //@endgroup


      ////////////////////////////////////////////////////////////
      // class CommResourceManagerClient
      ////////////////////////////////////////////////////////////
      class CommResourceManagerClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_started
         ////////////////////////////////////////////////////////////
         virtual void on_started(
            CommResourceManager *manager)
         { }

         ////////////////////////////////////////////////////////////
         // on_complete
         ////////////////////////////////////////////////////////////
         enum failure_type
         {
            failure_unknown = 0,
            failure_server_session_lost = 1,
            failure_invalid_logon = 2,
            failure_server_security_blocked = 3,
            failure_invalid_device_name = 4,
            failure_device_shut_down_by_server = 5,
            failure_device_communication_disabled = 6,
            failure_unsupported = 7,
            failure_unreachable = 8,
            failure_max_time_online = 9
         };
         virtual void on_failure(
            CommResourceManager *manager,
            failure_type failure) = 0;
      };


      ////////////////////////////////////////////////////////////
      // class CommResourceManager
      //
      // Declares a component that can be used to manage the communication resource associated with
      // a device. This gives the application a chance to specify that communication resources
      // needed by a device should be retained when the server has no othere reason to retain
      // them. This component will work with all device types except passive device types (such as
      // remote modems, etc).
      //
      // An application can use this component by deriving a class from class
      // CommResourceManagerClient so that it can receive event notifications from the
      // component. The application can then create an instance of this component, call the
      // appropriate property set methods including set_device_name(), and invoke the start()
      // method. When the server transaction has been successfully started, the client's
      // on_started() method will be called. If the server transaction ends at any time, the
      // component will invoke the client objects' on_complete() method.
      //
      // If the transaction must be ended, the client can invoke the finish() method (or delete the
      // component) and the server transaction will be released.
      ////////////////////////////////////////////////////////////
      class CommResourceManager:
         public DeviceBase,
         public ConnectionManagerClient,
         public Csi::EventReceiver
      {
      public:
         ////////////////////////////////////////////////////////////
         // enum priority_type
         ////////////////////////////////////////////////////////////
         enum priority_type
         {
            priority_high = 0,
            priority_normal = 1,
            priority_low = 2
         };

      private:
         //@group properties declarations
         ////////////////////////////////////////////////////////////
         // priority
         //
         // Declares the priority that should be used for the server transaction. A high priority
         // transaction will take precedence over scheduled server operations, a normal priority
         // will be the same as scheduled operations, and a low priority can be preempted by
         // scheduled server operations.
         ////////////////////////////////////////////////////////////
         priority_type priority;
         //@endgroup

      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         CommResourceManager();

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~CommResourceManager();

         ////////////////////////////////////////////////////////////
         // set_priority
         ////////////////////////////////////////////////////////////
         void set_priority(priority_type priority);

         ////////////////////////////////////////////////////////////
         // get_priority
         ////////////////////////////////////////////////////////////
         priority_type get_priority() const
         { return priority; }
         
         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         typedef CommResourceManagerClient client_type; 
         void start(
            client_type *client_,
            router_handle &router);
         void start(
            client_type *client_,
            ClientBase *other_component);

         ////////////////////////////////////////////////////////////
         // finish
         ////////////////////////////////////////////////////////////
         void finish();

         ////////////////////////////////////////////////////////////
         // describe_failure
         ////////////////////////////////////////////////////////////
         static void describe_failure(std::ostream &out, client_type::failure_type failure);

      protected:
         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

         ////////////////////////////////////////////////////////////
         // onNetMessage
         ////////////////////////////////////////////////////////////
         virtual void onNetMessage(
            Csi::Messaging::Router *rtr,
            Csi::Messaging::Message *msg);

         ////////////////////////////////////////////////////////////
         // on_devicebase_ready
         ////////////////////////////////////////////////////////////
         virtual void on_devicebase_ready();
         
         ////////////////////////////////////////////////////////////
         // on_devicebase_failure
         ////////////////////////////////////////////////////////////
         virtual void on_devicebase_failure(devicebase_failure_type failure);

         ////////////////////////////////////////////////////////////
         // on_devicebase_session_failure
         ////////////////////////////////////////////////////////////
         virtual void on_devicebase_session_failure();

         //@group ConnectionManagerClient methods
         ////////////////////////////////////////////////////////////
         // on_started
         ////////////////////////////////////////////////////////////
         virtual void on_started(
            ConnectionManager *manager);

         ////////////////////////////////////////////////////////////
         // on_failure
         ////////////////////////////////////////////////////////////
         virtual void on_failure(
            ConnectionManager *manager,
            failure_type failure,
            StrUni const &next_candidate);
         //@endgroup

      private:
         ////////////////////////////////////////////////////////////
         // on_start_ack
         ////////////////////////////////////////////////////////////
         void on_start_ack(Csi::Messaging::Message *message);

         ////////////////////////////////////////////////////////////
         // on_stopped_not
         ////////////////////////////////////////////////////////////
         void on_stopped_not(Csi::Messaging::Message *message);
         
      private:
         ////////////////////////////////////////////////////////////
         // client
         ////////////////////////////////////////////////////////////
         client_type *client;

         ////////////////////////////////////////////////////////////
         // state
         ////////////////////////////////////////////////////////////
         enum state_type
         {
            state_standby,
            state_delegate,
            state_active,
         } state;

         ////////////////////////////////////////////////////////////
         // connection_manager
         //
         // Keeps track of the component that is responsible for connection management for servers
         // that do not support the manage comm resources transaction.
         ////////////////////////////////////////////////////////////
         Csi::SharedPtr<ConnectionManager> connection_manager;
      };
   };
};


#endif

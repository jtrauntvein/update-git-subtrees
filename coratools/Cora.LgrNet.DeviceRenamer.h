/* Cora.LgrNet.DeviceRenamer.h

   Copyright (C) 2000, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 13 June 2001
   Last Change: Tuesday 08 October 2019
   Last Commit: $Date: 2019-10-16 11:44:34 -0600 (Wed, 16 Oct 2019) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Cora_LgrNet_DeviceRenamer_h
#define Cora_LgrNet_DeviceRenamer_h
#include "Cora.ClientBase.h"
#include "CsiEvents.h"
#include "Csi.InstanceValidator.h"


namespace Cora
{
   namespace LgrNet
   {
      class DeviceRenamer;


      /**
       * Defines the interface that an application must implement in order to use the DeviceRenamer
       * component type.
       */
      class DeviceRenamerClient: public Csi::InstanceValidator
      {
      public:
         /**
          * Called by the component when the LoggerNet transaction has been completed.
          *
          * @param sender Specifies the component calling this method.
          *
          * @param outcome Specifies the outcome of the LoggerNet transaction.
          */
         enum outcome_type
         {
            outcome_unknown = 0,
            outcome_success = 1,
            outcome_invalid_logon = 2,
            outcome_session_broken = 3,
            outcome_unsupported = 4,
            outcome_server_security_blocked = 5,
            outcome_invalid_device_name = 6,
            outcome_device_online = 7,
            outcome_network_locked = 8
         };
         virtual void on_complete(DeviceRenamer *sender, outcome_type outcome) = 0;
      };


      /**
       * Defines a component that can be used by an application to rename a device in LoggerNet's
       * network map.  In order to use this component, the application must provide an object that
       * inherits from class DeviceRenamerClient.  It should then create an instance of this object,
       * set its properties including device name and device new name, and then call one of the two
       * versions of start().   When the LoggerNet transaction is complete, the client object's
       * on_complete() method will be called.
       */
      class DeviceRenamer:
         public ClientBase,
         public Csi::EventReceiver
      {
      private:
         /**
          * Specifies the name of the device to be renamed.
          */
         StrUni device_name;

         /**
          * Specifies the new name that should be assigned to the device.
          */
         StrUni new_device_name;

      public:
         /**
          * Constructor
          */
         DeviceRenamer();

         /**
          * Destructor
          */
         virtual ~DeviceRenamer();

         /**
          * @param device_name_ Specifies the name of the device in the network map to be renamed.
          */
         void set_device_name(StrUni const &device_name_);

         /**
          * @param new_device_name_ Specifies the new name that will be assigned to the device in
          * the network map.
          */
         void set_new_device_name(StrUni const &new_device_name_);

         /**
          * Called to start the LoggerNet transaction.
          *
          * @param client_ Specifies the application object that will receive completion
          * notifications.
          *
          * @param router Specifies a newly created messaging router.
          *
          * @param other_component Specifies another component that has a connection that can be
          * shared with this component.
          */
         typedef DeviceRenamerClient client_type;
         void start(client_type *client_, router_handle &router);
         void start(client_type *client_, ClientBase *other_component);

         /**
          * Called to close this component and return it to a standby state.
          */
         virtual void finish();

         /**
          * Writes a description of the specified outcome to the provided stream.
          *
          * @param out Specifies the output stream.
          *
          * @param outcome Specifies the outcome code.
          */
         static void describe_outcome(std::ostream &out, client_type::outcome_type outcome);

         /**
          * Overloads the base class version to handle asynchronous completion messages.
          */
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);
         
      protected:
         /**
          * Overloads the base class to start the server transaction.
          */
         virtual void on_corabase_ready();

         /**
          * Overloads the base class version to handle a failure event.
          */
         virtual void on_corabase_failure(corabase_failure_type failure);

         /**
          * Overloads the base class version to handle a connection failre.
          */
         virtual void on_corabase_session_failure()
         { on_corabase_failure(corabase_failure_session); }

         /**
          * Overloads the base class version to handle a message from the server.
          */
         virtual void onNetMessage(Csi::Messaging::Router *rtr, Csi::Messaging::Message *msg);

      private:
         /**
          * Handles a network map enumeration notification.
          */
         void on_network_map_enum_not(Csi::Messaging::Message *message);

         /**
          * Handles an acknowledgement from the server.
          */
         void on_rename_device_ack(Csi::Messaging::Message *message);

         /**
          * Starts the server transactiob to rename the device.
          */
         void start_rename(uint4 version);
         
      private:
         /**
          * Specifies the client object.
          */
         client_type *client;

         /**
          * Specifies the state of this component.'*/
         enum state_type
         {
            state_standby,
            state_delegate,
            state_get_network_map,
            state_active
         } state;
      };
   };
};

#endif

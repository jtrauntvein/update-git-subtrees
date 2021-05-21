/* Cora.LgrNet.DeviceRelationsDescriber.h

   Copyright (C) 2000, 2019 Campbell Scientific, Inc.

   Written by: Andrew Mortenson
   Date Begun: Friday 25 October 2019
   Last Change: Tuesday 29 October 2019
   Last Commit: $Date: 2019-10-29 12:35:46 -0600 (Tue, 29 Oct 2019) $
   Committed by: $Author: jon $

*/

#ifndef Cora_LgrNet_DeviceRelationsDescriber_h
#define Cora_LgrNet_DeviceRelationsDescriber_h
#include "Cora.ClientBase.h"
#include "Csi.Events.h"
#include "Csi.InstanceValidator.h"
#include <deque>


namespace Cora
{
   namespace LgrNet
   {
      namespace DeviceRelationsDescriberHelpers
      {
         /**
         * Specifies the devices that can be created as immediate children of a device specified in the device_relations_type.
         */
         struct allowed_child_type
         {
            /**
             * Specifies the device type codes for devices that can be created as children.
             */
            std::deque<uint4> allowedTypes;
            
            /**
             * Specifies the maximum number of child devices that can be created in this slot.
             */
            uint4 maxCount;

            /**
             * Specifies the maximum times that a child and parent of the same type can be specifies sequentially in the same communications link.
             */
            uint4 maxDepth;
         };

         
         /**
          * Description of a device that can be created in the network map.
          */
         struct device_relations_type
         {
            /**
             * Specifies the type code for a device type that can be created in the server's network map.
             */
            uint4 deviceType;

            /**
             * Specifies a list of device types that must not be created with this device type in the link.
             */
            std::deque<uint4> disallowedDescendants;
            
            /**
             * Specifies a list of open "slots" or points to which a child can connect to this device.
             */
            std::deque<allowed_child_type> slots; 
         };

      }

      //@group class forward declarations
      class DeviceRelationsDescriber;
      //@endgroup

      
      /**
       * Defines the interface that an application must implement to used the DeviceRelationsDescriber
       * component type.
       */
      class DeviceRelationsDescriberClient: public Csi::InstanceValidator
      {
      public:
         /**
         * Called by the component when the LoggerNet transaction has been completed.
         *
         * @param describer Specifies the component calling this method.
         *
         * @param outcome Specifies the outcome of the LoggerNet transaction.
         *
         * @param deviceRelationsTypes Description of all device types that can be created in the newtwork map.
         */
         enum outcome_type
         {
            outcome_unknown = 0,
            outcome_success = 1,
            outcome_session_failed = 2,
            outcome_invalid_logon = 3,
            outcome_server_security_blocked=4,
            outcome_unsupported = 5
         };
         typedef DeviceRelationsDescriberHelpers::device_relations_type device_relations_type;
         typedef std::deque<device_relations_type> relations_type;
         virtual void on_complete(
            DeviceRelationsDescriber *describer,
            outcome_type outcome,
            relations_type const &relations) = 0;
      };


      /**
       * Defines a component that can be used to get the rules used by the LoggerNet server to
       * govern how devices can be connected in the network map.  In order to use this component,
       * the application must provide an object that extends class DeviceRelationsDescriberClient.
       * It should then create an instance of this class and call one of the two versions of
       * start(). When the server transaction is complete, this object will call the client's
       * on_complete() method.
       */
      class DeviceRelationsDescriber:
         public ClientBase,
         public Csi::EventReceiver
      {
      public:
         /**
          * Constructor
          */
         DeviceRelationsDescriber();

         /**
          * Destructor
          */
         virtual ~DeviceRelationsDescriber();

         /**
          * Called to start the LoggerNet transaction.
          *
          * @param client_ Specifies the application object that will receive completion
          * notifications.
          *
          * @param router Specifies a newly created messaging router;
          *
          * @param other_component Specifies another component that has a connection that can be
          * shared with this component.
          */
         typedef DeviceRelationsDescriberClient client_type;
         void start(
            client_type *client_,
            router_handle &router);
         void start(
            client_type *client_,
            ClientBase *other_component);

         /**
          * Called to close this component and return it to a standby state.
          */
         void finish();

         /**
          * Writes a description of the specified outcome to provided stream.
          *
          * @param out Specifies the output stream.
          *
          * @param outcome Specifies the outcome code.
          */
         static void describe_outcome(
            std::ostream &out, client_type::outcome_type outcome);

      protected:
         /**
          * Overloads the base class version to handle asynchronous completion messages.
          */
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev) override;

         /**
          * Overloads the base class version to handle a message from the server.
          */
         virtual void onNetMessage(Csi::Messaging::Router *rtr, Csi::Messaging::Message *msg) override;

         /**
          * Overloads the base class to start the server transaction.
          */
         virtual void on_corabase_ready() override;

         /**
          * Overloads the base class version to handle a connection failure.
          */
         virtual void on_corabase_failure(corabase_failure_type failure) override;
         
      private:
         /**
          * Interprets the describe_device_relations_ack.
          */
         void on_describe_device_relations_ack(Csi::Messaging::Message *message);
         
      private:
         /**
          * Specifies the client object.
          */
         client_type *client;

         /**
          * Specifies the state of this component.
          */
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

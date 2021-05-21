/* Cora.LgrNet.DeviceAdder.h

   Copyright (C) 2000, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Saturday 11 November 2000
   Last Change: Wednesday 15 January 2020
   Last Commit: $Date: 2020-01-15 13:59:18 -0600 (Wed, 15 Jan 2020) $ 
   Committed by: $author:$
   
*/

#ifndef Cora_LgrNet_DeviceAdder_h
#define Cora_LgrNet_DeviceAdder_h
#include "Cora.ClientBase.h"
#include "Cora.LgrNet.NetworkMapper.h"
#include "Cora.LgrNet.Defs.h"


namespace Cora
{
   namespace LgrNet
   {
      class DeviceAdder;
      
      /**
       * Defines the interface that an application object must implement to use the DeviceAdder
       * component.
       */
      class DeviceAdderClient: public Csi::InstanceValidator
      {
      public:
         /**
          * Called when the server transaction has eithr finished or failed.
          *
          * @param sender Specifies the component calling this method.
          *
          * @param outcome Specifies the outcome of the transaction.
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
            outcome_unattachable = 7,
            outcome_unsupported_device_type = 8,
            outcome_network_locked = 9,
            outcome_too_many_stations = 10
         };
         virtual void on_complete(DeviceAdder *sender, outcome_type outcome) = 0;
      };


      /**
       * Defines an object that can be used to add a device to the LoggerNet network map.   In order
       * to use this component, the application must provide an object that inherits from class
       * DeviceAdder client.  It must then create an instance of this class, set its properties
       * including device_type, anchor_name, anchor_type, and device_name and then  invoke one of
       * the two versions of start().  When the server transaction has completed or failed, the
       * client object's on_complete() method will be called.
       */
      class DeviceAdder:
         public ClientBase,
         protected NetworkMapperClient,
         public Csi::EventReceiver
      {
      private:
         /**
          * Specifies the name of the new device.
          */
         StrUni device_name;

         /**
          * Specifies the type code for the device that will be added.
          */
         DeviceTypes::device_type_code device_type;

         /**
          * Specifies the device to which the device will be anchored.
          */
         StrUni anchor_name;

         /**
          * Specifies the relation between the new device and the anchor device.
          */
         anchor_code_type anchor_code;

         /**
          * Starts the add transaction.
          */
         void start_add(uint4 network_version);
         
         /**
          * Specifies the state of this component.
          */
         enum state_type
         {
            state_standby,
            state_delegate,
            state_active
         } state;

         /**
          * Specifies the application client object reference.
          */
         DeviceAdderClient *client;

         /**
          * Specifies the component that receives the network map.
          */
         Csi::SharedPtr<NetworkMapper> mapper;

      public:
         /**
          * Constructor
          */
         DeviceAdder();

         /**
          * Destructor
          */
         virtual ~DeviceAdder();

         /**
          * @param device_name_ Specifies the name for the new device.
          */
         void set_device_name(StrUni const &device_name_)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            device_name = device_name_;
         }

         /**
          * @return Returns the name for the new device.
          */
         StrUni const &get_device_name() const
         { return device_name; }

         /**
          * @param device_type_ Specifies the type for the new device.
          */
         void set_device_type(DeviceTypes::device_type_code device_type_)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            device_type = device_type_;
         }
         void set_device_type(DevTypeCode device_type_)
         { set_device_type((DeviceTypes::device_type_code)device_type_); }

         /**
          * @return Returns the type code for the new device.
          */
         DeviceTypes::device_type_code get_device_type() const
         { return device_type; }

         /**
          * @param value Specifies the device to which the new device will be anchored.
          */
         void set_anchor_name(StrUni const &value)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            anchor_name = value;
         }

         /**
          * @return Returns the name of the device to which the new device will be anchored.
          */
         StrUni const &get_anchor_name() const
         { return anchor_name; }

         /**
          * @param anchor_code_ Specifies the code that identifies how the new device will be
          * anchored.
          */
         void set_anchor_code(anchor_code_type anchor_code_)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            anchor_code = anchor_code_;
         }

         /**
          * @return Returns the code that identifies how the new device will be anchored.
          */
         anchor_code_type get_anchor_code() const
         { return anchor_code; }

         /**
          * Starts the server transactions to add the new device.
          *
          * @param client_ Specifies the client component that will receive completion notification.
          *
          * @param router Specifies a router that has not been previously connected to the server.
          *
          * @param other_component Specifies a connected component that can share its connection
          * with this component.
          */
         typedef DeviceAdderClient client_type;
         void start(
            client_type *client_,
            router_handle &router);
         void start(
            client_type *client,
            ClientBase *other_component);

         /**
          * Overloads the base class to clean up the resources for this component.
          */
         virtual void finish();

         /**
          * Formats the specified outcome code to the given stream.
          *
          * @param out Specifies the stream to which the description will be written.
          *
          * @param outcome Specifies the outcome to describe.
          */
         static void describe_outcome(std::ostream &out, client_type::outcome_type outcome);

         /**
          * Overloads the base class version to handle asynchronous events.
          */
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);
         
      protected:
         /**
          * Overloads the base class version to handle the case where we are ready to work with the
          * server.
          */
         virtual void on_corabase_ready();

         /**
          * Overloads the base class version to handle a reported failure.
          */
         virtual void on_corabase_failure(corabase_failure_type failure);

         /**
          * Overloads the base class version to handle a connection failure.
          */
         virtual void on_corabase_session_failure()
         { on_corabase_failure(corabase_failure_session); }

         /**
          * Overloads the base class version to handle an incoming message,
          */
         virtual void onNetMessage(Csi::Messaging::Router *rtr, Csi::Messaging::Message *msg);

         /**
          * Overloads the base class version to handle a failure from the mapper.
          */
         virtual void on_failure(NetworkMapper *mapper, failure_type failure);

         /**
          * Overloads the base class version to handle the notification of a change in the network
          * map.
          */
         virtual void on_notify(
            NetworkMapper *mapper,
            uint4 network_map_version,
            uint4 agent_transaction_id,
            bool first_notification,
            uint4 device_count);
      };
   };
};

#endif

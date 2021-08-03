/* Cora.Device.CollectAreasConfigSetter.h

   Copyright (C) 2020, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Tuesday 31 March 2020
   Last Change: Thursday 02 April 2020
   Last Commit: $Date: 2020-04-02 09:29:23 -0600 (Thu, 02 Apr 2020) $
   Last Changed by: $Author: jon $

*/

#ifndef Cora_Device_CollectAreasConfigSetter_h
#define Cora_Device_CollectAreasConfigSetter_h
#include "Cora.Device.DeviceBase.h"
#include "Csi.Events.h"


namespace Cora
{
   namespace Device
   {
      class CollectAreasConfigSetter;


      /**
       * Defines the interface that must be implemented by the application in order to use the CollectAreasConfigSetter
       * component type.
       */
      class CollectAreasConfigSetterClient: public Csi::InstanceValidator
      {
      public:
         /**
          * Called to report the completion of the server transaction.
          *
          * @param sender Specifies the component calling this method.
          *
          * @param outcome Specifies the outcome.
          */
         enum outcome_type
         {
            outcome_failure_unknown = 0,
            outcome_success = 1,
            outcome_failure_invalid_device_name = 2,
            outcome_failure_security = 3,
            outcome_failure_unsupported = 4,
            outcome_failure_session = 5,
            outcome_failure_logon = 6,
            outcome_failure_invalid_config = 7,
            outcome_failure_network_locked = 8,
            outcome_failure_invalid_device_type = 9
         };
         virtual void on_complete(CollectAreasConfigSetter *sender, outcome_type outcome) = 0;
      };


      /**
       * Defines a component object that can be used to set the collect areas configuration for a station.  In order to
       * use this class, the application must provide an object that inherits from class
       * CollectAreasConfigSetterClient.  It should then create an instance of this class, set its properties include
       * device name and config, and call one of the two versions of start().  When the LoggerNet transaction is
       * complete, the component will call the client's on_complete() method.
       */
      class CollectAreasConfigSetter: public DeviceBase, public Csi::EventReceiver
      {
      private:
         /**
          * Specifies the application delegate.
          */
         CollectAreasConfigSetterClient *client;

         /**
          * Specifies the config string to pass to the server.
          */
         StrAsc config;

         /**
          * Specifies the state of this component.
          */
         enum state_type
         {
            state_standby,
            state_delegate,
            state_active
         } state;

      public:
         /**
          * Constructor
          */
         CollectAreasConfigSetter():
            client(0),
            state(state_standby)
         { }

         /**
          * Destructor
          */
         virtual ~CollectAreasConfigSetter()
         { finish(); }

         /**
          * @param config_ Specifies the configuration string.
          */
         void set_config(StrAsc const &config_)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            config = config_;
         }
         
         /**
          * @return Returns the configuration string.
          */
         StrAsc const &get_config() const
         { return config; }

         /**
          * Called to start the transaction.
          *
          * @param client_ Specifies the application delegate pointer.
          *
          * @param router Specifies a router that has been newly created but not yet connected.
          *
          * @param other_client Specifies a component that already has a server connection.
          */
         typedef CollectAreasConfigSetterClient client_type;
         void start(client_type *client_, router_handle router)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("invalid client pointer");
            client = client_;
            state = state_delegate;
            DeviceBase::start(router);
         }
         void start(client_type *client_, ClientBase *other_client)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("invalid client pointer");
            client = client_;
            state = state_delegate;
            DeviceBase::start(other_client);
         }

         /**
          * Overloads the base class to ensure that this component is reset to its standby state.
          */
         virtual void finish() override
         {
            state = state_standby;
            client = 0;
            DeviceBase::finish();
         }

         /**
          * Overrides the base class version to handle asynchrounous events.
          */
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev) override;

         /**
          * Writes a description of the specified outcome code to the given stream.
          */
         static void describe_outcome(std::ostream &out, client_type::outcome_type outcome);

      protected:
         /**
          * Overloads the base class version to handle the report that the device session is ready
          * for comms.
          */
         virtual void on_devicebase_ready() override;

         /**
          * Overloads the base class version to handle the notification of a session failure.
          */
         virtual void on_devicebase_session_failure() override
         { on_devicebase_failure(devicebase_failure_session); }

         /**
          * Overloads the base class version to handle a failure report.
          */
         virtual void on_devicebase_failure(devicebase_failure_type failure) override;

         /**
          * Overloads the base class version to handle an incoming message.
          */
         virtual void onNetMessage(Csi::Messaging::Router *router, Csi::Messaging::Message *message) override;
      };
   };
};


#endif

/* Cora.Device.CollectAreasConfigGetter.h

   Copyright (C) 2020, Campbell Scientific, Inc

   Written by: Jon Trauntvein 
   Date Begun: Tuesday 31 March 2020
   Last Change: Tuesday 31 March 2020
   Last Commit: $Date: 2020-03-31 15:20:40 -0600 (Tue, 31 Mar 2020) $
   Last Changed by: $Author: jon $

*/

#ifndef Cora_Device_CollectAreasConfigGetter_h
#define Cora_Device_CollectAreasConfigGetter_h
#include "Cora.Device.DeviceBase.h"
#include "Csi.Events.h"
#include "Csi.Xml.Element.h"


namespace Cora
{
   namespace Device
   {
      class CollectAreasConfigGetter;

      
      /**
       * Defines the interface that the application must be implemented in order to use tge
       * CollectAreasConfigGetter component type.
       */
      class CollectAreasConfigGetterClient: public Csi::InstanceValidator
      {
      public:
         /**
          * Called by the component when the transaction has been completed.
          *
          * @param sender Specifies the component calling this method.
          *
          * @param outcome Specifies the outcome of the transaction.
          *
          * @param config Specifies the XML configuration structure received from the server.  This
          * will be null if the transaction failed.
          */
         enum outcome_type
         {
            outcome_failure_unknown = 0,
            outcome_success = 1,
            outcome_failure_invalid_device_name = 2,
            outcome_failure_security = 3,
            outcome_failure_unsupported = 4,
            outcome_failure_session = 5,
            outcome_failure_logon = 6
         };
         virtual void on_complete(
            CollectAreasConfigGetter *sender, outcome_type outcome, Csi::Xml::Element::value_type &config) = 0;
      };


      /**
       * Defines a component that can be used to extract an XML structure from a given datalogger
       * device that represents the state of the collect areas for that device.  That state will
       * include the collect areas themselves, settings, collection state structures, and other data
       * that the server will write in its configuration file for those collect areas.  In order to
       * use this component, the application must provide an object that inherits from
       * CollectAreasConfigGetterClient.  It should then create an object of this class, set its
       * properties including device name, and then call one of the two versions of start.  When the
       * LoggerNet transaction is complete, the component will call the client's on_complete method.
       */
      class CollectAreasConfigGetter: public DeviceBase, public Csi::EventReceiver
      {
      private:
         /**
          * Specifies the application delegate.
          */
         CollectAreasConfigGetterClient *client;

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
         CollectAreasConfigGetter():
            state(state_standby),
            client(0)
         { }

         /**
          * Destructor
          */
         virtual ~CollectAreasConfigGetter()
         { finish(); }

         /**
          * Called to start the transaction.
          *
          * @param client_ Specifies the application delegate pointer.
          *
          * @param router Specifies a router that has been newly created but not yet connected.
          *
          * @param other_client Specifies a component that already has a server connection.
          */
         typedef CollectAreasConfigGetterClient client_type;
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

/* Cora.Device.AlohaStationAreaCreator.h

   Copyright (C) 2021, Campbell Scientific, Inc

   Written by: Jon Trauntvein 
   Date Begun: Tuesday 12 January 2021
   Last Change: Tuesday 12 January 2021
   Last Commit: $Date: 2021-01-12 15:28:09 -0600 (Tue, 12 Jan 2021) $
   Last Changed by: $Author: jon $

*/

#ifndef Cora_Device_AlohaStationAreaCreator_h
#define Cora_Device_AlohaStationAreaCreator_h

#include "Cora.Device.DeviceBase.h"
#include "Csi.Events.h"


namespace Cora
{
   namespace Device
   {
      class AlohaStationAreaCreator;


      /**
       * Defines the interface that must be implemented by an application object in  order to use
       * the AlohaStationAreaCreator component type.
       */
      class AlohaStationAreaCreatorClient: public Csi::InstanceValidator
      {
      public:
         /**
          * Called when the server transaction is complete.
          *
          * @param sender Specifies the component calling this method.
          *
          * @param outcome Specifies the outcome of the transaction.
          */
         enum outcome_type
         {
            outcome_failure_unknown = 0,
            outcome_success = 1,
            outcome_failure_session = 2,
            outcome_failure_invalid_device_name = 3,
            outcome_failure_logon = 4,
            outcome_failure_security = 5,
            outcome_failure_unsupported = 6,
            outcome_failure_invalid_station_id = 7
         };
         virtual void on_complete(
            AlohaStationAreaCreator *sender, outcome_type outcome) = 0;
      };


      /**
       * Defines a component that can be used to create an Aloha Station Report collect area for an
       * AlohaReceiver LoggerNet device type.  In order to use this component, the application must
       * provide an object that implements the AlohaStationAreaCreatorClient interface.  It should
       * then create an instance of this class, set properties including device name and station id,
       * and then call one of the two versions of start().  When the server transaction is
       * complete, the component will call the client's on_complete() method.
       */
      class AlohaStationAreaCreator: public DeviceBase, public Csi::EventReceiver
      {
      private:
         /**
          * Specifies the client object.
          */
         AlohaStationAreaCreatorClient *client;

         /**
          * Specifies the station ID.
          */
         uint4 station_id;

         /**
          * Specifies the station name property.
          */
         StrUni station_name;

         /**
          * Specifies the state of this transaction.
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
         AlohaStationAreaCreator():
            client(0),
            state(state_standby),
            station_id(0)
         { }

         /**
          * Destructor
          */
         virtual ~AlohaStationAreaCreator()
         {
            finish();
         }

         /**
          * @return Returns the value of the station ID property.
          */
         uint4 get_station_id() const
         { return station_id; }

         /**
          * @param value Specifies the new value of the station ID property.
          */
         void set_station_id(uint4 value)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            station_id = value;
         }

         /**
          * @return Returns the value of the station_name property.
          */
         StrUni const &get_station_name() const
         { return station_name; }

         /**
          * @param value Specifies the value for station name property.
          */
         void set_station_name(StrUni const &value)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            station_name = value;
         }

         /**
          * Connects to the server if needed and starts the transaction.
          *
          * @param client_ Specifies the application object that will receive the completion
          * notification.
          *
          * @param router Specifies a newly created, not connected messagubg router.
          *
          * @param other_client Specifies a connected component with which this component can share
          * its connection.
          */
         typedef AlohaStationAreaCreatorClient client_type;
         void start(client_type *client_, router_handle &router)
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
          * Overrides the base class version to reset local state.
          */
         virtual void finish() override
         {
            state = state_standby;
            client = 0;
            DeviceBase::finish();
         }

         /**
          * Overrides the base class version to handle async notifications.
          */
         virtual void receive(event_handle &ev) override;

         /**
          * Formats the specified outcome code to the given stream.
          */
         static void format_outcome(std::ostream &out, client_type::outcome_type outcome);

      protected:
         /**
          * Overrides the base class version to handle the notification that the device connection
          * is ready.
          */
         virtual void on_devicebase_ready() override;

         /**
          * Overrides the base class version to handle the notification of a failure.
          */
         virtual void on_devicebase_failure(devicebase_failure_type failure) override;

         /**
          * Overrides the base class version to handle incoming messages.
          */
         virtual void onNetMessage(
            Csi::Messaging::Router *router, Csi::Messaging::Message *message) override;
      };
   };
};


#endif

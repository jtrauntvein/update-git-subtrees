/* Cora.Device.CollectAreasEnumerator.h

  Copyright (c) 2001, 2019 Campbell Scientific, Inc.

  Written by: Jon Trauntvein
  Date Begun: Tuesday 06 November 2001
  Last Change: Friday 30 August 2019
  Last Commit: $Date: 2019-08-30 16:34:28 -0600 (Fri, 30 Aug 2019) $ 
  Committed by: $Author: jon $
  
*/

#ifndef Cora_Device_CollectAreasEnumerator_h
#define Cora_Device_CollectAreasEnumerator_h

#include "Cora.Device.DeviceBase.h"
#include "Csi.InstanceValidator.h"
#include "Csi.Events.h"


namespace Cora
{
   namespace Device
   {
      class CollectAreasEnumerator;

      
      /**
       * Defines the interface that must be implemented by an application object that attempts to
       * use this component.
       */
      class CollectAreasEnumeratorClient: public Csi::InstanceValidator
      {
      public:
         /**
          * Called when a new collect has been added or while first enumerating the list of collect
          * areas.  If this is called before on_started() is called, the collect area is one that
          * already exists.
          *
          * @param lister Specifies the component that is calling tbhis method.
          *
          * @param area_name Specifies the name of the collect area.
          *
          * @param persistence Specifies the persistence code for the collect area.
          *
          * @param type_id Specifies the code that identifies the type for this collect area.
          */
         typedef CollectArea::Persistence::Type persistence_type;
         typedef CollectArea::Types::Type type_id_type;
         virtual void on_area_added(
            CollectAreasEnumerator *lister,
            StrUni const &area_name,
            persistence_type persistence,
            type_id_type type_id)
         { }

         /**
          * Called to report that a collect area has been deleted.
          *
          * @param lister Specifies the component that is calling this method.
          *
          * @param area_name Specifies the name of the collect area that has been deletd.
          */
         virtual void on_area_deleted(
            CollectAreasEnumerator *lister,
            StrUni const &area_name)
         { }

         /**
          * Called after the list of initial collect areas have been reported to the client by
          * on_area_added.
          *
          * @param lister Specifies the component that is calling this method.
          */
         virtual void on_started(
            CollectAreasEnumerator *lister)
         { }

         /**
          * Called to report that a failure has occurred for this component.
          *
          * @param lister Specifies the component that is calling this method.
          *
          * @param failure Specifies a code that identifiers the cause of the failure.
          */
         enum failure_type
         {
            failure_unknown = 0,
            failure_connection = 1,
            failure_invalid_logon = 2,
            failure_server_security_blocked = 3,
            failure_device_name_invalid = 4,
            failure_not_supported = 5
         };
         virtual void on_failure(
            CollectAreasEnumerator *lister,
            failure_type failure) = 0;
      };


      /**
       * Defines a component that can be used to keep track of the list of collect areas owned by a
       * datalogger device in LoggerNet's network map.  In order to use this component, the
       * application must provide an object that is derived from class
       * CollectAreasEnumeratorClient.  It must then create an instance of this class, set the
       * device name, and invoke one of the two versions of start(). As the LoggerNet transaction is
       * started, the component will report all of the existing collect areas by calling the
       * client's on_area_added() method and will then call the client's on_started() method.
       * Thereafter, the client will be informed when an new area is added or an existing area is
       * deleted.  If a failure occurs at any time, the client's on_failure() method will be
       * called.
       */
      class CollectAreasEnumerator:
         public DeviceBase,
         public Csi::EventReceiver
      {
      public:
         /**
          * Constructor
          */
         CollectAreasEnumerator();

         /**
          * Destructor
          */
         virtual ~CollectAreasEnumerator();

         /**
          * Called to start the server transaction.
          *
          * @param client Specifies the application object that will receive notifications.
          *
          * @param router Specifies a router that has been newly created and not yet connected.
          *
          * @param other_component Specifies a component that already is connected to the server so
          * that this component can share that connection.
          */
         typedef CollectAreasEnumeratorClient client_type;
         void start(
            client_type *client_,
            router_handle &router);
         void start(
            client_type *client_,
            ClientBase *other_component);

         /**
          * Called to close the server transaction and release any resources used by this component,
          */
         virtual void finish();

         /**
          * Called by the application to format a failure code.
          *
          * @param out Specifies the stream to which the code will be written.
          *
          * @param failure Specifies the failure code to format.
          */
         static void format_failure(std::ostream &out, client_type::failure_type failure);
         
      protected:
         /**
          * Overloads the base class version to asynchronously deliver events to the application.
          */
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

         /**
          * Overloads the base class version to handle incoming server messages.
          */
         virtual void onNetMessage(Csi::Messaging::Router *rtr, Csi::Messaging::Message *msg);

         /**
          * Overloads the base class to handle failures reported on the device connection level.
          */
         virtual void on_devicebase_failure(devicebase_failure_type failure);

         /**
          * Overloads the base class version to handle a failure to stay connected to the device.
          */
         virtual void on_devicebase_session_failure();

         /**
          * Overloads the base class version to handle the event where we have established the
          * cloned session with the device.
          */
         virtual void on_devicebase_ready();

      private:
         /**
          * Handles the message that reports that a collect area has been added.
          */
         void on_area_added_not(Csi::Messaging::Message *message);

         /**
          * Handles the message that reports that a collect area has been deleted.
          */
         void on_area_deleted_not(Csi::Messaging::Message *message);

         /**
          * Handles the message that reports that the transaction had to be stopped.
          */
         void on_stopped_not(Csi::Messaging::Message *message);

      private:
         /**
          * Specifies the client that will receive notifications for this component,\.
          */
         client_type *client;

         /**
          * Specifies the state of this component.
          */
         enum state_type
         {
            state_standby,
            state_delegate,
            state_before_active,
            state_active
         } state;

         /**
          * Set to true to indicate that we want area types.
          */
         bool wants_area_types;
      };
   };
};


#endif

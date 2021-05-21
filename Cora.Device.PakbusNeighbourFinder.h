/* Cora.Device.PakbusNeighbourFinder.h

   Copyright (C) 2003, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 02 December 2003
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Cora_Device_PakbusNeighbourFinder_h
#define Cora_Device_PakbusNeighbourFinder_h

#include "Cora.Device.DeviceBase.h"


namespace Cora
{
   namespace Device
   {
      //@group class forward declarations
      class PakbusNeighbourFinder;
      //@endgroup


      ////////////////////////////////////////////////////////////
      // class PakbusNeighbourFinderClient
      ////////////////////////////////////////////////////////////
      class PakbusNeighbourFinderClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_started
         ////////////////////////////////////////////////////////////
         virtual void on_started(
            PakbusNeighbourFinder *finder)
         { }

         ////////////////////////////////////////////////////////////
         // on_failure
         ////////////////////////////////////////////////////////////
         enum failure_type
         {
            failure_unknown = 0,
            failure_connection_failed = 1,
            failure_invalid_logon = 2,
            failure_server_security_blocked = 3,
            failure_invalid_device_name = 4,
            failure_communication_disabled = 5,
            failure_link_failed = 6,
            failure_other_transaction = 7,
            failure_unsupported = 8,
         };
         virtual void on_failure(
            PakbusNeighbourFinder *finder,
            failure_type failure) = 0;
      };


      ////////////////////////////////////////////////////////////
      // class PakbusNeighbourFinder
      //
      // Defines a component that sets up the conditions where the server can
      // reliably discover neighbours regardless of link type and past state.
      //
      // In order to use this component, an application must provide a client
      // object that is derived from class PakbusNeighbourFinderClient.  It
      // must then create an instance of this class and set appropriate
      // properties including device_name.  The name of the device should match
      // one of the names of PakBus ports.  It must then invoke the start()
      // method.  When the server transaction has been successfully started,
      // the component will invoke the client object's on_started() method.
      // The transaction will continue until the application calls the
      // component's finish() method or until the an error causes the
      // transaction to fail.  If a failure occurs, the client object's
      // on_failure() will be invoked.
      ////////////////////////////////////////////////////////////
      class PakbusNeighbourFinder:
         public DeviceBase,
         public Csi::EventReceiver
      {
      private:
         ////////////////////////////////////////////////////////////
         // client
         ////////////////////////////////////////////////////////////
         PakbusNeighbourFinderClient *client;

         ////////////////////////////////////////////////////////////
         // state
         ////////////////////////////////////////////////////////////
         enum state_type
         {
            state_standby,
            state_delegate,
            state_active
         } state;
         
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         PakbusNeighbourFinder():
            client(0),
            state(state_standby)
         { }

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~PakbusNeighbourFinder()
         { finish(); }

         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         typedef PakbusNeighbourFinderClient client_type;
         void start(
            client_type *client_,
            router_handle &router);

         ////////////////////////////////////////////////////////////
         // start (use existing connection)
         ////////////////////////////////////////////////////////////
         void start(
            client_type *client_,
            ClientBase *other_component);

         ////////////////////////////////////////////////////////////
         // finish
         ////////////////////////////////////////////////////////////
         virtual void finish()
         {
            state = state_standby;
            client = 0;
            DeviceBase::finish();
         }

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
         // on_deviecbase_ready
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

      private:
         ////////////////////////////////////////////////////////////
         // on_stopped_not
         ////////////////////////////////////////////////////////////
         void on_stopped_not(Csi::Messaging::Message *message);
      };
   };
};


#endif

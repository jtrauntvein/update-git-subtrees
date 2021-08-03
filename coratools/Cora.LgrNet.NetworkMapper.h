/* Cora.LgrNet.NetworkMapper.h

   Copyright (C) 2000, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Saturday 11 November 2000
   Last Change: Tuesday 22 January 2019
   Last Commit: $Date: 2019-01-22 15:04:50 -0600 (Tue, 22 Jan 2019) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Cora_LgrNet_NetworkMapper_h
#define Cora_LgrNet_NetworkMapper_h


#include "Cora.ClientBase.h"
#include "Csi.InstanceValidator.h"
#include "CsiEvents.h"
#include "Cora.Defs.h"


namespace Cora
{
   namespace LgrNet
   {
      //@group class forward definitions
      class NetworkMapper;
      //@endgroup

      /**
       * Defines the interface required to be implemented by the application's client to the
       * NetworkMapper component.
       */
      class NetworkMapperClient: public Csi::InstanceValidator
      {
      public:
         /**
          * Called when the mapper has failed.
          *
          * @param mapper Specifies the component reporting this failure.
          *
          * @param failure Specifies the failure that is being reported.
          */
         enum failure_type
         {
            failure_unknown = 0,
            failure_invalid_logon = 1,
            failure_session_broken = 2,
            failure_unsupported = 3,
            failure_server_security = 4,
         };
         virtual void on_failure(NetworkMapper *mapper, failure_type failure) = 0;

         /**
          * Called when a network map notification has first been received from the server.
          * Following this call, the client will receive zero or more calls to on_device().  One of
          * these notifications will arrive for each device described in the netowrk map and in the
          * order given by the server.
          *
          * @param mapper Specifies the component sending this notification.
          *
          * @param network_map_version Specifies the map version number reported by the server.
          *
          * @param agent_transaction_id Specifies the transaction that is responsible for the change
          * having occurred.
          *
          * @param first_notification Set to true if this is the first notification since the
          * component was started.
          *
          * @param device_count Specifies the number of devices.
          */
         virtual void on_notify(
            NetworkMapper *mapper,
            uint4 network_map_version,
            uint4 agent_transaction_id,
            bool first_notification,
            uint4 device_count)
         { }

         /**
          * Called once for each device described by the server.
          *
          * @param mapper Specifies the component reporting this notification.
          *
          * @param device_type Specifies the device type code.
          *
          * @param device_object_id Specifies the unique identifier for the device.
          *
          * @param name Specifies the name of the device.
          *
          * @param level Specifies the device indentation level.
          *
          * @param last_devicee Set to true if this is the last device notification to be handled.
          */
         virtual void on_device(
            NetworkMapper *mapper,
            DevTypeCode device_type,
            uint4 device_object_id,
            StrUni const &name,
            uint4 level,
            bool last_device)
         { }

         /**
          * Called when the server reports that a snapshot has been restored.
          *
          * @param sender Specifies the component sending this notification.
          */
         virtual void on_snapshot_restored(NetworkMapper *sender)
         { }
      };


      ////////////////////////////////////////////////////////////
      // class NetworkMapper
      //
      // Defines an object that keeps its client up to date regarding changes to the LoggerNet
      // network map. This object will translate network map changed notication messages received
      // into one or more function calls to the client.
      //
      // After creating an object of this class, the application should invoke the appropriate
      // methods to set properties such as logon_name and logon_password. When the properties have
      // been set, the application can invoke the start() method. Once the object has logged onto
      // the server, it will start the network map enumerate transaction with the server. Each time
      // a network map notification has been received, the mapper object will invoke the client's
      // on_notify() once and the client's on_device() methods once for each device described. If,
      // at any time, an error occurs, the mapper will invoke the client's on_failure() method and
      // return to a standby state.
      ////////////////////////////////////////////////////////////
      class NetworkMapper: public ClientBase, public Csi::EvReceiver
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         NetworkMapper();

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~NetworkMapper();

         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         typedef NetworkMapperClient client_type;
         void start(
            NetworkMapperClient *client_,
            router_handle &router);
         void start(
            NetworkMapperClient *client_,
            ClientBase *other_component);

         ////////////////////////////////////////////////////////////
         // finish
         ////////////////////////////////////////////////////////////
         void finish();

         ////////////////////////////////////////////////////////////
         // format_failure
         //
         // Formats text that describes the client failure code.
         ////////////////////////////////////////////////////////////
         static void format_failure(
            std::ostream &out, client_type::failure_type failure);

      protected:
         ////////////////////////////////////////////////////////////
         // on_corabase_ready
         ////////////////////////////////////////////////////////////
         virtual void on_corabase_ready();

         ////////////////////////////////////////////////////////////
         // on_corabase_failure
         ////////////////////////////////////////////////////////////
         virtual void on_corabase_failure(corabase_failure_type failure);

         ////////////////////////////////////////////////////////////
         // on_corabase_session_failure
         ////////////////////////////////////////////////////////////
         virtual void on_corabase_session_failure();

         /**
          * Overloads the base class version to handle the snapshot restored event.
          */
         virtual void on_snapshot_restored();

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

      private:
         ////////////////////////////////////////////////////////////
         // state
         ////////////////////////////////////////////////////////////
         enum state_type
         {
            state_standby,
            state_delegate,
            state_before_active,
            state_active
         } state;

         ////////////////////////////////////////////////////////////
         // client
         ////////////////////////////////////////////////////////////
         client_type *client;
      };
   };
};

#endif

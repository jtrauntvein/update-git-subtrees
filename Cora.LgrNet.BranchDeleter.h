/* Cora.LgrNet.BranchDeleter.h

   Copyright (C) 2000, 2010 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 05 December 2000
   Last Change: Thursday 21 October 2010
   Last Commit: $Date: 2010-10-21 16:46:00 -0600 (Thu, 21 Oct 2010) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Cora_LgrNet_BranchDeleter_h
#define Cora_LgrNet_BranchDeleter_h
#include "Cora.ClientBase.h"
#include "CsiEvents.h"
#include "Csi.InstanceValidator.h"


namespace Cora
{
   namespace LgrNet
   {
      //@group class forward declarations
      class BranchDeleter;
      //@endgroup


      ////////////////////////////////////////////////////////////
      // class BranchDeleterClient
      //
      // Defines the interface that a client to a BranchDeleter object should implement.
      ////////////////////////////////////////////////////////////
      class BranchDeleterClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // method on_complete
         //
         // Called when the BranchDeleter object has finished the server transaction to delete a
         // network branch.
         ////////////////////////////////////////////////////////////
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
            outcome_network_locked = 8,
         };
         virtual void on_complete(
            BranchDeleter *deleter,
            outcome_type outcome) = 0;
      };


      ////////////////////////////////////////////////////////////
      // class BranchDeleter
      //
      // Defines an object that delete a specified device as well as its children from the cora
      // server network map.
      //
      // After creating an object of this type, an application should set the appropriate
      // properties. "device_name" is a required property. Once the properties have been set, the
      // application can call start() which will start the transaction with the cora server. Once
      // the server transaction is complete, the client's on_complete() method will be invoked and
      // the object will return to a standby state where different properties can be specified and
      // start() invoked again.
      //
      // The client can abort the notification by invoking finish(). This will not, however, abort
      // the transaction with the server.
      ////////////////////////////////////////////////////////////
      class BranchDeleter:
         public ClientBase,
         public Csi::EvReceiver
      {
      private:
         //@group properties
         ////////////////////////////////////////////////////////////
         // member device_name
         //
         // The name of the device that should be deleted.
         ////////////////////////////////////////////////////////////;
         StrUni device_name;
         //@endgroup

      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         BranchDeleter();

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~BranchDeleter();

         ////////////////////////////////////////////////////////////
         // method set_device_name
         ////////////////////////////////////////////////////////////
         void set_device_name(StrUni const &device_name_);

         ////////////////////////////////////////////////////////////
         // get_device_name
         ////////////////////////////////////////////////////////////
         StrUni const &get_device_name() const
         { return device_name; }

         ////////////////////////////////////////////////////////////
         // method start
         ////////////////////////////////////////////////////////////
         typedef BranchDeleterClient client_type;
         void start(
            client_type *client_,
            router_handle &router);
         void start(
            client_type *client_,
            ClientBase *other_component);

         ////////////////////////////////////////////////////////////
         // method finish
         ////////////////////////////////////////////////////////////
         void finish();

         ////////////////////////////////////////////////////////////
         // describe_outcome
         ////////////////////////////////////////////////////////////
         static void describe_outcome(
            std::ostream &out, client_type::outcome_type outcome);

      protected:
         ////////////////////////////////////////////////////////////
         // method on_corabase_ready
         ////////////////////////////////////////////////////////////
         virtual void on_corabase_ready();

         ////////////////////////////////////////////////////////////
         // method on_corabase_failure
         ////////////////////////////////////////////////////////////
         virtual void on_corabase_failure(corabase_failure_type failure);

         ////////////////////////////////////////////////////////////
         // method on_corabase_session_failure
         ////////////////////////////////////////////////////////////
         virtual void on_corabase_session_failure();

         ////////////////////////////////////////////////////////////
         // method receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

         ////////////////////////////////////////////////////////////
         // method onNetMessage
         ////////////////////////////////////////////////////////////
         virtual void onNetMessage(
            Csi::Messaging::Router *rtr,
            Csi::Messaging::Message *msg);

      private:
         ////////////////////////////////////////////////////////////
         // on_network_map_enum_not
         ////////////////////////////////////////////////////////////
         void on_network_map_enum_not(Csi::Messaging::Message *message);

         ////////////////////////////////////////////////////////////
         // on_delete_branch_ack
         ////////////////////////////////////////////////////////////
         void on_delete_branch_ack(Csi::Messaging::Message *message);

         ////////////////////////////////////////////////////////////
         // start_delete
         ////////////////////////////////////////////////////////////
         void start_delete(uint4 version);
         
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
            state_get_network_map,
            state_active
         } state;
      };
   };
};

#endif

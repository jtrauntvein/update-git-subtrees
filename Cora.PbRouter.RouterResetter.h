/* Cora.PbRouter.RouterResetter.h

   Copyright (C) 2002, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 06 August 2002
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Cora_PbRouter_RouterResetter_h
#define Cora_PbRouter_RouterResetter_h


#include "Cora.PbRouter.PbRouterBase.h"
#include "Csi.InstanceValidator.h"


namespace Cora
{
   namespace PbRouter
   {
      //@group class forward declarations
      class RouterResetter;
      //@endgroup


      ////////////////////////////////////////////////////////////
      // class RouterResetterClient
      ////////////////////////////////////////////////////////////
      class RouterResetterClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_complete
         ////////////////////////////////////////////////////////////
         enum outcome_type
         {
            outcome_unknown = 0,
            outcome_success = 1,
            outcome_invalid_logon = 2,
            outcome_invalid_router_id = 3,
            outcome_server_permission_denied = 4,
            outcome_server_session_failed = 5,
            outcome_communication_failed = 6,
            outcome_unsupported = 7, 
            outcome_invalid_pakbus_address = 8, 
         };
         virtual void on_complete(
            RouterResetter *resetter,
            outcome_type outcome) = 0;
      };


      ////////////////////////////////////////////////////////////
      // class RouterResetter
      //
      // Defines a component that can be used to reset the routing tables of one or more routers on
      // the network that the router identified by router_id is a part of.
      //
      // In order to use this component, an application must provide an object derived from class
      // RouterRetterClient.  It can create an instance ofRouterResetter and set the appropriate
      // attributes including router_id and pakbus_address (the default pakbus_address will be the
      // broadcast address).  It should then call one of the versions of start() which will start
      // the server transaction.  When the server transaction is complete, the client object's
      // on_complete() method will be called and the component returned to a standby state.
      ////////////////////////////////////////////////////////////
      class RouterResetter:
         public PbRouterBase,
         public Csi::EventReceiver
      {
      private:
         ////////////////////////////////////////////////////////////
         // pakbus_address
         //
         // The address of the node that should have its routing tables reset.  Initialised by
         // default to the broadcast address.
         ////////////////////////////////////////////////////////////
         uint2 pakbus_address;

         ////////////////////////////////////////////////////////////
         // destroy_routing_tables
         //
         // If true the routing table will be destroyed and relearned completely.
         // If false the routing table is just verified.
         // Defaults to true.
         ////////////////////////////////////////////////////////////
         bool destroy_routing_tables;
         
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         RouterResetter();

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~RouterResetter();

         ////////////////////////////////////////////////////////////
         // set_pakbus_address
         ////////////////////////////////////////////////////////////
         void set_pakbus_address(uint2 pakbus_address_);

         ////////////////////////////////////////////////////////////
         // get_pakbus_address
         ////////////////////////////////////////////////////////////
         uint2 get_pakbus_address() const
         { return pakbus_address; }

         ////////////////////////////////////////////////////////////
         // set_destroy_routing_tables
         ////////////////////////////////////////////////////////////
         void set_destroy_routing_tables(bool destroy_routing_tables_);

         ////////////////////////////////////////////////////////////
         // get_destroy_routing_tables
         ////////////////////////////////////////////////////////////
         bool get_destroy_routing_tables()
         { return destroy_routing_tables; }

         ////////////////////////////////////////////////////////////
         // start (new router)
         ////////////////////////////////////////////////////////////
         typedef RouterResetterClient client_type;
         void start(
            client_type *client_,
            router_handle &router);

         ////////////////////////////////////////////////////////////
         // start (existing connection)
         ////////////////////////////////////////////////////////////
         void start(
            client_type *client_,
            ClientBase *other_component);

         ////////////////////////////////////////////////////////////
         // finish
         ////////////////////////////////////////////////////////////
         void finish();

      protected:
         ////////////////////////////////////////////////////////////
         // on_pbrouterbase_ready
         ////////////////////////////////////////////////////////////
         virtual void on_pbrouterbase_ready();

         ////////////////////////////////////////////////////////////
         // on_pbrouterbase_failure
         ////////////////////////////////////////////////////////////
         virtual void on_pbrouterbase_failure(pbrouterbase_failure_type);

         ////////////////////////////////////////////////////////////
         // onNetMessage
         ////////////////////////////////////////////////////////////
         virtual void onNetMessage(
            Csi::Messaging::Router *router,
            Csi::Messaging::Message *message);

         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

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
            state_active
         } state;
      };
   };
};


#endif

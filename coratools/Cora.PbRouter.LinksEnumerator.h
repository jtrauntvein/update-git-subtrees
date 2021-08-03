/* Cora.PbRouter.LinksEnumerator.h

   Copyright (C) 2002, 2009 Campbell Scientific, Inc.

   Written by: Tyler Mecham
   Date Begun: Monday 10 June 2002
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2019-11-22 15:34:57 -0600 (Fri, 22 Nov 2019) $ 
   Last Changed by: $Author: amortenson $

*/

#ifndef Cora_PbRouter_LinksEnumerator_h
#define Cora_PbRouter_LinksEnumerator_h

#include "Cora.PbRouter.PbRouterBase.h"
#include "Csi.InstanceValidator.h"
#include "Csi.Events.h"
#include <list>


namespace Cora
{
   namespace PbRouter
   {
      //@group class forward declarations
      class LinksEnumerator;
      //@endgroup


      ////////////////////////////////////////////////////////////
      // class LinksEnumeratorClient
      ////////////////////////////////////////////////////////////
      class LinksEnumeratorClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_started
         //
         // Called after the transaction has been started and reached a steady state
         ////////////////////////////////////////////////////////////
         virtual void on_started(LinksEnumerator *tran){ }

         ////////////////////////////////////////////////////////////
         // on_failure
         //
         // Called after the transaction has failed
         ////////////////////////////////////////////////////////////
         enum failure_type
         {
            failure_unknown  = 0,
            failure_connection_failed = 1,
            failure_invalid_logon = 2,
            failure_server_security_blocked = 3,
            failure_invalid_router_id = 4,
            failure_server_session_failed = 5,
            failure_unsupported = 6,
         };
         virtual void on_failure(LinksEnumerator *tran, failure_type failure) = 0;

         ////////////////////////////////////////////////////////////
         // on_link_added
         //
         // Called when a link has been added
         ////////////////////////////////////////////////////////////
         virtual void on_link_added(
            LinksEnumerator *tran,
            uint2 node1_address,
            uint4 node1_net_map_id,
            bool node1_is_router,
            uint2 node2_address,
            uint4 node2_net_map_id,
            bool node2_is_router,
            uint4 worst_case_resp_time)
         { }


         ////////////////////////////////////////////////////////////
         // on_link_deleted
         //
         // Called when a link has been deleted
         ////////////////////////////////////////////////////////////
         virtual void on_link_deleted(
            LinksEnumerator *tran,
            uint2 node1_address,
            uint4 node1_net_map_id,
            uint2 node2_address,
            uint4 node2_net_map_id)
         { }


         ////////////////////////////////////////////////////////////
         // on_link_changed
         //
         // Called when a link has been changed
         ////////////////////////////////////////////////////////////
         virtual void on_link_changed(
            LinksEnumerator *tran,
            uint2 node1_address,
            uint4 node1_net_map_id,
            bool node1_is_router,
            uint2 node2_address,
            uint4 node2_net_map_id,
            bool node2_is_router,
            uint4 worst_case_resp_time)
         { }
      };


      ////////////////////////////////////////////////////////////
      // class LinksEnumerator
      //
      // This class defines a component that enumerates the list of links associated with
      // PakBus ports that are known to the router associated with a PakBus port object.
      //
      // An application can use this component by creating an object that is derived from class
      // LinksEnumeratorClient and creating an instance of this class.  The application should then
      // set the appropriate attributes including pakbus_router_id and then invoke the start()
      // method.
      ////////////////////////////////////////////////////////////
      class LinksEnumerator:
         public PbRouterBase,
         public Csi::EventReceiver
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         LinksEnumerator();

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~LinksEnumerator();

         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         typedef LinksEnumeratorClient client_type;
         void start(
            client_type *client,
            router_handle &router);
         void start(
            client_type *client,
            ClientBase *other_component);

         ////////////////////////////////////////////////////////////
         // finish
         ////////////////////////////////////////////////////////////
         void finish();

         ////////////////////////////////////////////////////////////
         // format_failure
         ////////////////////////////////////////////////////////////
         static void format_failure(std::ostream &out, client_type::failure_type failure);

      protected:
         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

         ////////////////////////////////////////////////////////////
         // on_pbrouterbase_ready
         ////////////////////////////////////////////////////////////
         virtual void on_pbrouterbase_ready();

         ////////////////////////////////////////////////////////////
         // on_pbrouterbase_failure
         ////////////////////////////////////////////////////////////
         virtual void on_pbrouterbase_failure(pbrouterbase_failure_type failure);

         ////////////////////////////////////////////////////////////
         // onNetMessage
         ////////////////////////////////////////////////////////////
         virtual void onNetMessage(
            Csi::Messaging::Router *rtr,
            Csi::Messaging::Message *msg);

      private:
         ////////////////////////////////////////////////////////////
         // on_start_ack
         ////////////////////////////////////////////////////////////
         void on_start_ack(Csi::Messaging::Message *message);

         ////////////////////////////////////////////////////////////
         // on_stopped_not
         ////////////////////////////////////////////////////////////
         void on_stopped_not(Csi::Messaging::Message *message);

         ////////////////////////////////////////////////////////////
         // on_change_not
         ////////////////////////////////////////////////////////////
         void on_change_not(Csi::Messaging::Message *message);

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

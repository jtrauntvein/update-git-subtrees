/* Cora.Device.NodesLister.h

   Copyright (C) 2002, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 06 May 2002
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Cora_Device_NodesLister_h
#define Cora_Device_NodesLister_h

#include "Cora.PbRouter.PbRouterBase.h"
#include "Csi.InstanceValidator.h"
#include "Csi.Events.h"
#include <list>


namespace Cora
{
   namespace PbRouter
   {
      //@group class forward declarations
      class NodesLister;
      //@endgroup


      ////////////////////////////////////////////////////////////
      // class NodesListerClient
      ////////////////////////////////////////////////////////////
      class NodesListerClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_complete
         //
         // Called when the server transaction has been completed.  The nodes list will be empty if
         // the outcome does not indicate success.
         ////////////////////////////////////////////////////////////
         enum outcome_type
         {
            outcome_unknown = 0, 
            outcome_success = 1,
            outcome_invalid_logon = 2,
            outcome_invalid_router_id = 3,
            outcome_server_permission_denied = 4,
            outcome_unsupported = 5,
            outcome_server_session_failed = 6,
         };
         struct node_type
         {
            uint2 pakbus_address;
            uint2 router_address;
            uint4 worst_case_response_time;
            StrUni network_map_name;
         };
         typedef std::list<node_type> nodes_list_type;
         virtual void on_complete(
            NodesLister *lister,
            outcome_type outcome,
            nodes_list_type &nodes_list) = 0;
      };


      ////////////////////////////////////////////////////////////
      // class NodesLister
      //
      // This class defines a component that retrieves the list of PakBus ports that are known to
      // the router.
      //
      // An application can use this component by creating an object that is derived from class
      // NodesListerClient and creating an instance of this class.  The application should
      // then set the appropriate attributes including device_name and then invoke the start()
      // method.  When the server transaction is complete, the component will call the client's
      // on_complete() method with parameters that indicate the outcome of the transaction and, if
      // the transaction succeeded, the list of nodes known at the time the transaction was
      // executed. 
      ////////////////////////////////////////////////////////////
      class NodesLister:
         public PbRouterBase,
         public Csi::EventReceiver
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         NodesLister();

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~NodesLister();

         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         typedef NodesListerClient client_type;
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

      protected:
         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

         ////////////////////////////////////////////////////////////
         // on_devicebase_ready
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

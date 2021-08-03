/* Cora.LgrNet.BranchMover.h

   Copyright (C) 2001, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 27 June 2001
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2012-08-08 08:36:43 -0600 (Wed, 08 Aug 2012) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Cora_LgrNet_BranchMover_h
#define Cora_LgrNet_BranchMover_h

#include "Cora.ClientBase.h"
#include "CsiEvents.h"
#include "Csi.InstanceValidator.h"


namespace Cora
{
   namespace LgrNet
   {
      //@group class forward declarations
      class BranchMover;
      //@endgroup


      ////////////////////////////////////////////////////////////
      // class BranchMoverClient
      ////////////////////////////////////////////////////////////
      class BranchMoverClient: 
         public Csi::InstanceValidator
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
            outcome_session_broken = 3,
            outcome_unsupported = 4,
            outcome_server_security_blocked = 5,
            outcome_specified_device_not_found = 6,
            outcome_specified_anchor_not_found = 7,
            outcome_unattachable = 8,
            outcome_pending_transaction = 9,
            outcome_network_locked = 10,
         };
         virtual void on_complete(
            BranchMover *mover,
            outcome_type outcome) = 0;
      };


      ////////////////////////////////////////////////////////////
      // class BranchMover
      //
      // Defines a component that can be used to move a branch of devices from one place in the
      // network map to another place.
      //
      // An application uses this class by deriving a client object from class BranchMoverClient. It
      // can then create an instance of this class and set the appropriate properties including
      // branch_root_name, anchor_name, and anchor_code. It should then call start(). Once the
      // server transaction is complete, the component will invoke the client's on_complete() method
      // to inform it of the outcome.
      //
      // If the application invokes finish() while the server transaction is pending, the server
      // transaction will proceed. The only effect of finish() is to stop the outcome notification
      // from returning to the client.
      ////////////////////////////////////////////////////////////
      class BranchMover:
         public ClientBase,
         public Csi::EventReceiver
      {
      public:
         ////////////////////////////////////////////////////////////
         // enum anchor_code_type
         ////////////////////////////////////////////////////////////
         enum anchor_code_type
         {
            anchor_before = 0,
            anchor_as_child = 1,
            anchor_after = 2
         };

      private:
         //@group component properties
         ////////////////////////////////////////////////////////////
         // branch_root_name
         //
         // The name of the device that is the root of the branch that is to be moved. 
         ////////////////////////////////////////////////////////////
         StrUni branch_root_name;

         ////////////////////////////////////////////////////////////
         // anchor_name
         //
         // The name of the device that the branch root is to be introduced relative to in the
         // network map.
         ////////////////////////////////////////////////////////////
         StrUni anchor_name;

         ////////////////////////////////////////////////////////////
         // anchor_code
         //
         // The code that describes how the branch will be re-inserted into the network map.
         ////////////////////////////////////////////////////////////
         anchor_code_type anchor_code;
         //@endgroup

      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         BranchMover();

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~BranchMover();

         //@group property access methods
         ////////////////////////////////////////////////////////////
         // set_branch_root_name
         ////////////////////////////////////////////////////////////
         void set_branch_root_name(StrUni const &branch_root_name_);

         ////////////////////////////////////////////////////////////
         // set_anchor_name
         ////////////////////////////////////////////////////////////
         void set_anchor_name(StrUni const &anchor_name_);

         ////////////////////////////////////////////////////////////
         // set_anchor_code
         ////////////////////////////////////////////////////////////
         void set_anchor_code(anchor_code_type anchor_code_);

         ////////////////////////////////////////////////////////////
         // get_branch_root_name
         ////////////////////////////////////////////////////////////
         StrUni const &get_branch_root_name() const { return branch_root_name; }

         ////////////////////////////////////////////////////////////
         // get_anchor_name
         ////////////////////////////////////////////////////////////
         StrUni const &get_anchor_name() const { return anchor_name; }

         ////////////////////////////////////////////////////////////
         // get_anchor_code
         ////////////////////////////////////////////////////////////
         anchor_code_type get_anchor_code() const { return anchor_code; }
         //@endgroup

         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         typedef BranchMoverClient client_type;
         void start(
            client_type *client_,
            router_handle &router);
         void start(
            client_type *client_,
            ClientBase *other_component);
         
         ////////////////////////////////////////////////////////////
         // finish
         ////////////////////////////////////////////////////////////
         void finish();

         ////////////////////////////////////////////////////////////
         // describe_outcome
         ////////////////////////////////////////////////////////////
         static void describe_outcome(std::ostream &out, client_type::outcome_type outcome);

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
         // on_network_map_not
         ////////////////////////////////////////////////////////////
         void on_network_map_not(Csi::Messaging::Message *message);

         ////////////////////////////////////////////////////////////
         // on_move_branch_ack
         ////////////////////////////////////////////////////////////
         void on_move_branch_ack(Csi::Messaging::Message *message);

         ////////////////////////////////////////////////////////////
         // start_move
         ////////////////////////////////////////////////////////////
         void start_move(uint4 version);
         
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

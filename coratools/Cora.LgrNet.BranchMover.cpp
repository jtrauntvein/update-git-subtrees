/* Cora.LgrNet.BranchMover.cpp

   Copyright (C) 2001, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 27 June 2001
   Last Change: Tuesday 07 August 2012
   Last Commit: $Date: 2012-08-08 08:36:43 -0600 (Wed, 08 Aug 2012) $ 
   Committed by: $Author: jon $
   
*/

#pragma warning(disable:4786) // disable overlong symbols warning
#pragma hdrstop               // stop creation of precompiled header
#include "Cora.LgrNet.BranchMover.h"
#include "Cora.LgrNet.Defs.h"
#include "coratools.strings.h"


namespace Cora
{
   namespace LgrNet
   {
      namespace BranchMoverHelpers
      {
         ////////////////////////////////////////////////////////////
         // class event_complete
         ////////////////////////////////////////////////////////////
         class event_complete: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // client
            ////////////////////////////////////////////////////////////
            typedef BranchMoverClient client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // outcome
            ////////////////////////////////////////////////////////////
            typedef client_type::outcome_type outcome_type;
            outcome_type outcome;

            ////////////////////////////////////////////////////////////
            // create_and_post
            ////////////////////////////////////////////////////////////
            static void create_and_post(
               BranchMover *mover,
               client_type *client,
               outcome_type outcome)
            {
               try { (new event_complete(mover,client,outcome))->post(); }
               catch(Csi::Event::BadPost &) { }
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_complete(
               BranchMover *mover,
               client_type *client_,
               outcome_type outcome_):
               Event(event_id,mover),
               client(client_),
               outcome(outcome_)
            { }
         };


         uint4 const event_complete::event_id =
         Csi::Event::registerType("Cora::LgrNet::BranchMover::event_complete");
      };

      
      ////////////////////////////////////////////////////////////
      // class BranchMover definitions
      ////////////////////////////////////////////////////////////
      BranchMover::BranchMover():
         client(0),
         state(state_standby),
         anchor_code(anchor_after)
      { }


      BranchMover::~BranchMover()
      { finish(); }


      void BranchMover::set_branch_root_name(StrUni const &branch_root_name_)
      {
         if(state == state_standby)
            branch_root_name = branch_root_name_;
         else
            throw exc_invalid_state();
      } // set_branch_root_name


      void BranchMover::set_anchor_name(StrUni const &anchor_name_)
      {
         if(state == state_standby)
            anchor_name = anchor_name_;
         else
            throw exc_invalid_state(); 
      } // set_anchor_name


      void BranchMover::set_anchor_code(anchor_code_type anchor_code_)
      {
         if(state == state_standby)
            anchor_code = anchor_code_;
         else
            throw exc_invalid_state();
      } // set_anchor_code


      void BranchMover::start(
         client_type *client_,
         router_handle &router)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
            {
               state = state_delegate;
               client = client_;
               ClientBase::start(router);
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start


      void BranchMover::start(
         client_type *client_,
         ClientBase *other_component)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
            {
               state = state_delegate;
               client = client_;
               ClientBase::start(other_component);
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start


      void BranchMover::finish()
      {
         state = state_standby;
         client = 0;
         ClientBase::finish();
      } // finish


      void BranchMover::describe_outcome(
         std::ostream &out, client_type::outcome_type outcome)
      {
         using namespace BranchMoverStrings;
         switch(outcome)
         {
         case client_type::outcome_success:
            out << "success";
            break;
            
         case client_type::outcome_invalid_logon:
            describe_failure(out, corabase_failure_logon);
            break;
            
         case client_type::outcome_session_broken:
            describe_failure(out, corabase_failure_session);
            break;
            
         case client_type::outcome_unsupported:
            describe_failure(out, corabase_failure_unsupported);
            break;
            
         case client_type::outcome_server_security_blocked:
            describe_failure(out, corabase_failure_security);
            break;
            
         case client_type::outcome_specified_device_not_found:
            out << my_strings[strid_device_not_found];
            break;
            
         case client_type::outcome_specified_anchor_not_found:
            out << my_strings[strid_anchor_not_found];
            break;

         case client_type::outcome_unattachable:
            out << my_strings[strid_unattachable];
            break;
            
         case client_type::outcome_pending_transaction:
            out << my_strings[strid_pending_transaction];
            break;
            
         case client_type::outcome_network_locked:
            out << my_strings[strid_network_locked];
            break;

         default:
            describe_failure(out, corabase_failure_unknown);
            break;
         }
      }


      void BranchMover::on_corabase_ready()
      {
         if(interface_version < Csi::VersionNumber("1.3.1.31"))
         {
            // we need to get the network map in order to have the version number
            Csi::Messaging::Message command(
               net_session,
               Messages::network_map_enum_cmd);
            router->sendMessage(&command);
            state = state_get_network_map;
         }
         else
            start_move(0);
      } // on_corabase_ready


      void BranchMover::on_corabase_failure(corabase_failure_type failure)
      {
         using namespace BranchMoverHelpers;
         client_type::outcome_type outcome;
         switch(failure)
         {
         case corabase_failure_logon:
            outcome = client_type::outcome_invalid_logon;
            break;

         case corabase_failure_session:
            outcome = client_type::outcome_session_broken;
            break;

         case corabase_failure_unsupported:
            outcome = client_type::outcome_unsupported;
            break;

         case corabase_failure_security:
            outcome = client_type::outcome_server_security_blocked;
            break;

         default:
            outcome = client_type::outcome_unknown;
            break;
         }
         event_complete::create_and_post(this,client,outcome);
      } // on_corabase_failure


      void BranchMover::on_corabase_session_failure()
      {
         using namespace BranchMoverHelpers;
         event_complete::create_and_post(this,client,client_type::outcome_session_broken);
      } // on_corabase_session_failure


      void BranchMover::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace BranchMoverHelpers;
         if(ev->getType() == event_complete::event_id)
         {
            event_complete *event = static_cast<event_complete *>(ev.get_rep());
            if(event->client == client && client_type::is_valid_instance(client))
            {
               finish();
               event->client->on_complete(this,event->outcome);
            }
            else
               finish();
         }
      } // receive


      void BranchMover::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(state == state_get_network_map || state == state_active)
         {
            switch(msg->getMsgType())
            {
            case Messages::network_map_advise_not:
               on_network_map_not(msg);
               break;

            case Messages::move_branch_ack:
               on_move_branch_ack(msg);
               break;

            default:
               ClientBase::onNetMessage(rtr,msg);
               break;
            }
         }
         else
            ClientBase::onNetMessage(rtr,msg);
      } // onNetMessage


      void BranchMover::on_network_map_not(Csi::Messaging::Message *message)
      {
         if(state == state_get_network_map)
         {
            // extract the version from the message
            uint4 net_map_version;
            message->readUInt4(net_map_version);
            start_move(net_map_version);
         }
      } // on_network_map_not


      void BranchMover::on_move_branch_ack(Csi::Messaging::Message *message)
      {
         // read the acknowledgement
         uint4 tran_no;
         uint4 resp_code;
         message->readUInt4(tran_no);
         message->readUInt4(resp_code);

         // process the outcome
         using namespace BranchMoverHelpers;
         client_type::outcome_type outcome;

         switch(resp_code)
         {
         case 0:
            outcome = client_type::outcome_success;
            break;

         case 102:
            outcome = client_type::outcome_specified_device_not_found;
            break;

         case 103:
            outcome = client_type::outcome_specified_anchor_not_found;
            break;

         case 104:
            outcome = client_type::outcome_pending_transaction;
            break;

         case 108:
            outcome = client_type::outcome_unattachable;
            break;

         case 109:
            outcome = client_type::outcome_network_locked;
            break;

         default:
            outcome = client_type::outcome_unknown;
            break;
         }
         event_complete::create_and_post(this,client,outcome);
      } // on_move_branch_ack


      void BranchMover::start_move(uint4 version)
      {
         Csi::Messaging::Message command(net_session,Messages::move_branch_cmd);
         command.addUInt4(++last_tran_no);
         command.addUInt4(version);
         command.addWStr(branch_root_name);
         command.addWStr(anchor_name);
         command.addUInt4(anchor_code);
         router->sendMessage(&command);
         state = state_active;
      } // start_move
   };
};



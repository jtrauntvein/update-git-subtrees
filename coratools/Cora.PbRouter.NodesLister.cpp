/* Cora.PbRouter.NodesLister.cpp

Copyright (C) 2002, 2009 Campbell Scientific, Inc.

Written by: Jon Trauntvein
Date Begun: Monday 06 May 2002
Last Change: Friday 23 October 2009
Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
Last Changed by: $Author: jon $

*/

#pragma warning(disable:4786) // disable overlong symbols warning
#pragma hdrstop               // stop creation of precompiled header
#include "Cora.PbRouter.NodesLister.h"


namespace Cora
{
   namespace PbRouter
   {
      namespace NodesListerHelpers
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
            typedef NodesLister::client_type client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // outcome
            ////////////////////////////////////////////////////////////
            typedef client_type::outcome_type outcome_type;
            outcome_type outcome;

            ////////////////////////////////////////////////////////////
            // nodes_list
            ////////////////////////////////////////////////////////////
            typedef client_type::nodes_list_type nodes_list_type;
            nodes_list_type nodes_list;

            ////////////////////////////////////////////////////////////
            // create
            ////////////////////////////////////////////////////////////
            static event_complete *create(
               NodesLister *lister,
               client_type *client,
               outcome_type outcome)
            { return new event_complete(lister,client,outcome); }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_complete(
               NodesLister *lister,
               client_type *client_,
               outcome_type outcome_):
               Event(event_id,lister),
               client(client_),
               outcome(outcome_)
            { }
         };


         uint4 const event_complete::event_id =
         Csi::Event::registerType("Cora::PbRouter::NodesLister::event_complete");
      };


      ////////////////////////////////////////////////////////////
      // class NodesLister definitions
      ////////////////////////////////////////////////////////////
      NodesLister::NodesLister():
         client(0),
         state(state_standby)
      { }

      
      NodesLister::~NodesLister()
      { finish(); }

      
      void NodesLister::start(
         client_type *client_,
         router_handle &router)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
            {
               client = client_;
               state = state_delegate;
               PbRouterBase::start(router);
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start

      
      void NodesLister::start(
         client_type *client_,
         ClientBase *other_component)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
            {
               client = client_;
               state = state_delegate;
               PbRouterBase::start(other_component);
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start

      
      void NodesLister::finish()
      {
         client = 0;
         state = state_standby;
         PbRouterBase::finish();
      } // finish

      
      void NodesLister::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace NodesListerHelpers;
         if(ev->getType() == event_complete::event_id)
         {
            event_complete *event = static_cast<event_complete *>(ev.get_rep());
            finish();
            if(client_type::is_valid_instance(event->client))
               event->client->on_complete(this,event->outcome,event->nodes_list);
         }
      } // receive

      
      void NodesLister::on_pbrouterbase_ready()
      {
         Csi::Messaging::Message cmd(pbrouter_session,Messages::list_nodes_cmd);
         cmd.addUInt4(++last_tran_no);
         router->sendMessage(&cmd);
         state = state_active; 
      } // on_pbrouterbase_ready

      
      void NodesLister::on_pbrouterbase_failure(pbrouterbase_failure_type failure)
      {
         using namespace NodesListerHelpers;
         client_type::outcome_type outcome;
         switch(failure)
         {
         case pbrouterbase_failure_logon:
            outcome = client_type::outcome_invalid_logon;
            break;
            
         case pbrouterbase_failure_session:
            outcome = client_type::outcome_server_session_failed;
            break;
            
         case pbrouterbase_failure_invalid_router_id:
            outcome = client_type::outcome_invalid_router_id;
            break;
            
         case pbrouterbase_failure_unsupported:
            outcome = client_type::outcome_unsupported;
            break;
            
         case pbrouterbase_failure_security:
            outcome = client_type::outcome_server_permission_denied;
            break;
            
         default:
            outcome = client_type::outcome_unknown;
            break;
         }
         event_complete *event = event_complete::create(this,client,outcome);
         event->post();
      } // on_pbrouterbase_failure

      
      void NodesLister::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         using namespace NodesListerHelpers;
         if(state == state_active)
         {
            if(msg->getMsgType() == Messages::list_nodes_ack)
            {
               uint4 tran_no;
               uint4 count;
               event_complete *event = event_complete::create(
                  this,client,client_type::outcome_success);
               
               msg->readUInt4(tran_no);
               msg->readUInt4(count);
               for(uint4 i = 0; i < count; ++i)
               {
                  client_type::node_type node;
                  msg->readUInt2(node.pakbus_address);
                  msg->readUInt2(node.router_address);
                  msg->readUInt4(node.worst_case_response_time);
                  msg->readWStr(node.network_map_name);
                  event->nodes_list.push_back(node);
               }
               event->post();
            }
            else
               PbRouterBase::onNetMessage(rtr,msg);
         }
         else
            PbRouterBase::onNetMessage(rtr,msg);
      } // onNetMessage 
   };
};

/* Cora.PbRouter.RouterResetter.cpp

   Copyright (C) 2002, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 06 August 2002
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Last Changed by: $Author: jon $

*/

#pragma warning(disable:4786) // disable overlong symbols warning
#pragma hdrstop               // stop creation of precompiled header

#include "Cora.PbRouter.RouterResetter.h"
#include "Csi.PakBus.Router.h"

namespace Cora
{
   namespace PbRouter
   {
      namespace RouterResetterHelpers
      {
         ////////////////////////////////////////////////////////////
         // class event_complete
         ////////////////////////////////////////////////////////////
         class event_complete: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // client
            ////////////////////////////////////////////////////////////
            typedef RouterResetterClient client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // outcome
            ////////////////////////////////////////////////////////////
            typedef client_type::outcome_type outcome_type;
            outcome_type outcome;

            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;
            
            ////////////////////////////////////////////////////////////
            // create_and_post
            ////////////////////////////////////////////////////////////
            static void create_and_post(
               RouterResetter *resetter,
               client_type *client,
               outcome_type outcome)
            {
               try{ (new event_complete(resetter,client,outcome))->post(); }
               catch(Csi::Event::BadPost &) { }
            }

         private:
            event_complete(
               RouterResetter *resetter,
               client_type *client_,
               outcome_type outcome_):
               Event(event_id,resetter),
               client(client_),
               outcome(outcome_)
            { }
         };


         uint4 const event_complete::event_id =
         Csi::Event::registerType("Cora::PbRouter::RouterResetter::event_complete");
      };

      
      ////////////////////////////////////////////////////////////
      // class RouterResetter definitions
      ////////////////////////////////////////////////////////////
      RouterResetter::RouterResetter():
         state(state_standby),
         client(0),
         pakbus_address(Csi::PakBus::Router::broadcast_address),
         destroy_routing_tables(true)
      { }
      
         
      RouterResetter::~RouterResetter()
      { finish(); }

      
      void RouterResetter::set_pakbus_address(uint2 pakbus_address_)
      {
         if(state == state_standby)
            pakbus_address = pakbus_address_;
         else
            throw exc_invalid_state();
      } // set_pakbus_address


      void RouterResetter::set_destroy_routing_tables(bool destroy_routing_tables_)
      {
         if(state == state_standby)
            destroy_routing_tables = destroy_routing_tables_;
         else
            throw exc_invalid_state();
      } // set_destroy_routing_tables

      
      void RouterResetter::start(
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

      
      void RouterResetter::start(
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

      
      void RouterResetter::finish()
      {
         client = 0;
         state = state_standby;
         PbRouterBase::finish();
      } // finish

      
      void RouterResetter::on_pbrouterbase_ready()
      {
         Csi::Messaging::Message cmd(pbrouter_session,Messages::router_reset_cmd);
         cmd.addUInt4(++last_tran_no);
         cmd.addUInt2(pakbus_address);
         cmd.addBool(destroy_routing_tables);
         state = state_active;
         router->sendMessage(&cmd);
      } // on_pbrouterbase_ready

      
      void RouterResetter::on_pbrouterbase_failure(pbrouterbase_failure_type failure)
      {
         using namespace RouterResetterHelpers;
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
         event_complete::create_and_post(this,client,outcome);
      } // on_pbrouterbase_failure

      
      void RouterResetter::onNetMessage(
         Csi::Messaging::Router *router,
         Csi::Messaging::Message *message)
      {
         if(state == state_active)
         {
            if(message->getMsgType() == Messages::router_reset_ack)
            {
               uint4 tran_no;
               uint4 outcome;
               client_type::outcome_type client_outcome;
               
               message->readUInt4(tran_no);
               message->readUInt4(outcome);
               switch(outcome)
               {
               case 1:
                  client_outcome = client_type::outcome_success;
                  break;

               case 2:
                  client_outcome = client_type::outcome_invalid_pakbus_address;
                  break;

               case 3:
                  client_outcome = client_type::outcome_communication_failed;
                  break;

               default:
                  client_outcome = client_type::outcome_unknown;
                  break;
               }
               RouterResetterHelpers::event_complete::create_and_post(this,client,client_outcome);
            }
            else
               PbRouterBase::onNetMessage(router,message);
         }
         else
            PbRouterBase::onNetMessage(router,message);
      } // onNetMessage

      
      void RouterResetter::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace RouterResetterHelpers;
         if(ev->getType() == event_complete::event_id)
         {
            event_complete *event = static_cast<event_complete *>(ev.get_rep());
            client_type *client = this->client;
            finish();
            if(event->client == client && client_type::is_valid_instance(client))
               client->on_complete(this,event->outcome);
         }
      } // receive 
   };
};

/* Cora.PbRouter.CommResourceManager.cpp

   Copyright (C) 2003, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 19 February 2003
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Last Changed by: $Author: jon $
   CVS $Header: /home/group/cvs2/cora/coratools/Cora.PbRouter.CommResourceManager.cpp,v 1.1.1.1 2004/04/30 13:18:37 jon Exp $

*/

#pragma warning(disable:4786) // disable overlong symbols warning
#pragma hdrstop               // stop creation of precompiled header
#include "Cora.PbRouter.CommResourceManager.h"


namespace Cora
{
   namespace PbRouter
   {
      namespace
      {
         ////////////////////////////////////////////////////////////
         // class event_base
         ////////////////////////////////////////////////////////////
         class event_base: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // manager
            ////////////////////////////////////////////////////////////
            typedef CommResourceManager manager_type;
            manager_type *manager;
            
            ////////////////////////////////////////////////////////////
            // client
            ////////////////////////////////////////////////////////////
            typedef manager_type::client_type client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify() = 0;

         protected:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_base(
               uint4 event_id,
               manager_type *manager_,
               client_type *client_):
               Event(event_id,manager_),
               manager(manager_),
               client(client_)
            { }
         };


         ////////////////////////////////////////////////////////////
         // class event_started
         ////////////////////////////////////////////////////////////
         class event_started: public event_base
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // create_and_post
            ////////////////////////////////////////////////////////////
            static void create_and_post(
               manager_type *manager,
               client_type *client)
            {
               try{(new event_started(manager,client))->post();}
               catch(Csi::Event::BadPost &) { }
            }

            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            {
               if(client_type::is_valid_instance(client))
                  client->on_started(manager);
            }
            
         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_started(
               manager_type *manager,
               client_type *client):
               event_base(event_id,manager,client)
            { }
         };


         uint4 const event_started::event_id =
         Csi::Event::registerType("Cora::PbRouter::CommResourceManager::event_started");


         ////////////////////////////////////////////////////////////
         // class event_failed
         ////////////////////////////////////////////////////////////
         class event_failed: public event_base
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // failure
            ////////////////////////////////////////////////////////////
            typedef client_type::failure_type failure_type;
            failure_type failure;
            
            ////////////////////////////////////////////////////////////
            // create_and_post
            ////////////////////////////////////////////////////////////
            static void create_and_post(
               manager_type *manager,
               client_type *client,
               failure_type failure)
            {
               try{(new event_failed(manager,client,failure))->post(); }
               catch(Csi::Event::BadPost &) { }
            }

            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            {
               if(client_type::is_valid_instance(client))
                  client->on_failure(manager,failure);
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_failed(
               manager_type *manager,
               client_type *client,
               failure_type failure_):
               event_base(event_id,manager,client),
               failure(failure_)
            { }
         };


         uint4 const event_failed::event_id =
         Csi::Event::registerType("Cora::PbRouter::CommResourceManager::event_failed");
      };


      ////////////////////////////////////////////////////////////
      // class CommResourceManager definitions
      ////////////////////////////////////////////////////////////
      CommResourceManager::CommResourceManager():
         client(0),
         state(state_standby),
         priority(priority_normal),
         pakbus_address(0)
      { }

      
      CommResourceManager::~CommResourceManager()
      { finish(); }


      void CommResourceManager::set_priority(priority_type priority_)
      {
         if(state == state_standby)
            priority = priority_;
         else
            throw exc_invalid_state();
      } // set_priority

      
      void CommResourceManager::set_pakbus_address(uint2 pakbus_address_)
      {
         if(state == state_standby)
            pakbus_address = pakbus_address_;
         else
            throw exc_invalid_state();
      } // set_pakbus_address

      
      void CommResourceManager::start(
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

      
      void CommResourceManager::start(
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

      
      void CommResourceManager::finish()
      {
         client = 0;
         state = state_standby;
         PbRouterBase::finish();
      } // finish

      
      void CommResourceManager::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         event_base *event = dynamic_cast<event_base *>(ev.get_rep());
         if(event)
         {
            client_type *client = this->client;
            if(event->getType() == event_failed::event_id)
               finish();
            if(event->client == client && client_type::is_valid_instance(client))
               event->notify();
         }
      } // receive

      
      void CommResourceManager::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(state == state_active)
         {
            if(msg->getMsgType() == Messages::manage_comm_resource_start_ack)
            {
               event_started::create_and_post(this,client);
            }
            else if(msg->getMsgType() == Messages::manage_comm_resource_stopped_not)
            {
               uint4 tran_no;
               uint4 reason;
               client_type::failure_type failure;
               
               msg->readUInt4(tran_no);
               msg->readUInt4(reason);
               switch(reason)
               {
               case 3:
                  failure = client_type::failure_unreachable;
                  break;

               case 4:
                  failure = client_type::failure_router_shut_down;
                  break;

               default:
                  failure = client_type::failure_unknown;
                  break;
               }
               event_failed::create_and_post(this,client,failure);
            }
            else
               PbRouterBase::onNetMessage(rtr,msg);
         }
         else
            PbRouterBase::onNetMessage(rtr,msg);
      } // onNetMessage

      
      void CommResourceManager::on_pbrouterbase_ready()
      {
         if(client_type::is_valid_instance(client))
         {
            Csi::Messaging::Message start_command(
               pbrouter_session,
               Messages::manage_comm_resource_start_cmd);
            start_command.addUInt4(++last_tran_no);
            start_command.addUInt2(pakbus_address);
            start_command.addUInt4(priority);
            state = state_active;
            router->sendMessage(&start_command);
         }
         else
            finish();
      } // on_pbrouterbase_ready

      
      void CommResourceManager::on_pbrouterbase_failure(pbrouterbase_failure_type failure)
      {
         client_type::failure_type client_failure;
         switch(failure)
         {
         case pbrouterbase_failure_logon:
            client_failure = client_type::failure_invalid_logon;
            break;
            
         case pbrouterbase_failure_session:
            client_failure = client_type::failure_server_session_lost;
            break;
            
         case pbrouterbase_failure_invalid_router_id:
            client_failure = client_type::failure_invalid_router_id;
            break;
            
         case pbrouterbase_failure_unsupported:
            client_failure = client_type::failure_unsupported;
            break;
            
         case pbrouterbase_failure_security:
            client_failure = client_type::failure_server_security_blocked;
            break;
            
         default:
            client_failure = client_type::failure_unknown;
            break;
         }
         event_failed::create_and_post(this,client,client_failure);
      } // on_pbrouterbase_failure
   };
};

/* Cora.LgrNet.BatchModeMaintainer.cpp

   Copyright (C) 2001, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 14 December 2001
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Last Changed by: $Author: jon $

*/

#pragma warning(disable:4786) // disable overlong symbols warning
#pragma hdrstop               // stop creation of precompiled header
#include "Cora.LgrNet.BatchModeMaintainer.h"
#include "Cora.LgrNet.Defs.h"


namespace Cora
{
   namespace LgrNet
   {
      namespace BatchModeMaintainerHelpers
      {
         ////////////////////////////////////////////////////////////
         // class event_base
         ////////////////////////////////////////////////////////////
         class event_base: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // client
            ////////////////////////////////////////////////////////////
            typedef BatchModeMaintainerClient client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // maintainer
            ////////////////////////////////////////////////////////////
            typedef BatchModeMaintainer maintainer_type;
            maintainer_type *maintainer;

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
               maintainer_type *maintainer_,
               client_type *client_):
               Event(event_id,maintainer_),
               maintainer(maintainer_),
               client(client_)
            { }
         };

         
         ////////////////////////////////////////////////////////////
         // class event_failure
         ////////////////////////////////////////////////////////////
         class event_failure: public event_base
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
               maintainer_type *maintainer,
               client_type *client,
               failure_type failure)
            {
               try{ (new event_failure(maintainer,client,failure))->post(); }
               catch(Csi::Event::BadPost &) { }
            }

            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            { client->on_failure(maintainer,failure); }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_failure(
               maintainer_type *maintainer,
               client_type *client,
               failure_type failure_):
               event_base(event_id,maintainer,client),
               failure(failure_)
            { }
         };


         uint4 const event_failure::event_id =
         Csi::Event::registerType("Cora::LgrNet::BatchModeMaintainer::event_failure");


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
               maintainer_type *maintainer,
               client_type *client)
            {
               try{ (new event_started(maintainer,client))->post(); }
               catch(Csi::Event::BadPost &) { }
            }

            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            { client->on_started(maintainer); }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_started(
               maintainer_type *maintainer,
               client_type *client):
               event_base(event_id,maintainer,client)
            { }
         };


         uint4 const event_started::event_id =
         Csi::Event::registerType("Cora::LgrNet::BatchModeMaintainer::event_started");
      };

      
      ////////////////////////////////////////////////////////////
      // class BatchModeMaintainer definitions
      ////////////////////////////////////////////////////////////
      BatchModeMaintainer::BatchModeMaintainer():
         state(state_standby),
         client(0)
      { }
      
      
      BatchModeMaintainer::~BatchModeMaintainer()
      { finish(); }

      
      void BatchModeMaintainer::start(
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

      
      void BatchModeMaintainer::start(
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

      
      void BatchModeMaintainer::finish()
      {
         client = 0;
         state = state_standby;
         ClientBase::finish();
      } // finish

      
      void BatchModeMaintainer::on_corabase_ready()
      {
         Csi::Messaging::Message command(
            net_session,
            Messages::batch_mode_start_cmd);
         command.addUInt4(++last_tran_no);
         router->sendMessage(&command);
         state = state_active;
      } // on_corabase_ready

      
      void BatchModeMaintainer::on_corabase_failure(corabase_failure_type failure)
      {
         using namespace BatchModeMaintainerHelpers;
         client_type::failure_type client_failure;
         switch(failure)
         {
         case corabase_failure_logon:
            client_failure = client_type::failure_invalid_logon;
            break;
            
         case corabase_failure_session:
            client_failure = client_type::failure_session_broken;
            break;
            
         case corabase_failure_unsupported:
            client_failure = client_type::failure_unsupported;
            break;
            
         case corabase_failure_security:
            client_failure = client_type::failure_server_security_blocked;
            break;
            
         default:
            client_failure = client_type::failure_unknown;
            break; 
         }
         event_failure::create_and_post(this,client,client_failure);
      } // on_corabase_failure

      
      void BatchModeMaintainer::on_corabase_session_failure()
      {
         using namespace BatchModeMaintainerHelpers;
         event_failure::create_and_post(this,client,client_type::failure_session_broken);
      } // on_corabase_session_failure

      
      void BatchModeMaintainer::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace BatchModeMaintainerHelpers;
         event_base *event = static_cast<event_base *>(ev.get_rep());

         if(event->getType() == event_failure::event_id)
            finish();
         if(client_type::is_valid_instance(event->client))
            event->notify();
      } // receive

      
      void BatchModeMaintainer::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(state == state_active)
         {
            switch(msg->getMsgType())
            {
            case Messages::batch_mode_start_ack:
               on_start_ack(msg);
               break;

            default:
               ClientBase::onNetMessage(rtr,msg);
               break;
            }
         }
         else
            ClientBase::onNetMessage(rtr,msg);
      } // onNetMessage


      void BatchModeMaintainer::on_start_ack(Csi::Messaging::Message *message)
      {
         using namespace BatchModeMaintainerHelpers;
         uint4 tran_no;
         uint4 outcome;
         
         message->readUInt4(tran_no);
         message->readUInt4(outcome);
         if(outcome == 1)
         {
            state = state_active;
            event_started::create_and_post(this,client);
         }
         else
            event_failure::create_and_post(this,client,client_type::failure_duplicate);
      } // on_start_ack
   };
};

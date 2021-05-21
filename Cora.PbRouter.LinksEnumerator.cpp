/* Cora.PbRouter.LinksEnumerator.cpp

   Copyright (C) 2002, 2009 Campbell Scientific, Inc.

   Written by: Tyler Mecham
   Date Begun: Monday 10 June 2002
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2019-11-22 15:34:57 -0600 (Fri, 22 Nov 2019) $ 
   Last Changed by: $Author: amortenson $

*/

#pragma warning(disable:4786) // disable overlong symbols warning
#pragma hdrstop               // stop creation of precompiled header
#include "Cora.PbRouter.LinksEnumerator.h"


namespace Cora
{
   namespace PbRouter
   {
      namespace LinksEnumeratorHelpers
      {
         ////////// class event_base
         class event_base: public Csi::Event
         {
         public:
            LinksEnumerator *tran;
            LinksEnumeratorClient *client;

            ////////// constructor
            event_base(uint4 event_id,
                       LinksEnumerator *tran_,
                       LinksEnumeratorClient *client_):
               tran(tran_),
               client(client_),
               Event(event_id,tran_)
            { }

            ////////// notify
            // Called by the enumerator to send the notification to the client 
            virtual void notify() = 0;
         };


         ////////////////////////////////////////////////////////////
         // class event_started
         ////////////////////////////////////////////////////////////
         class event_start: public event_base
         {
         public:
            static uint4 const event_id;

            static void create_and_post(LinksEnumerator *tran,
                                        LinksEnumeratorClient *client);

            virtual void notify() { client->on_started(tran); }
            
         private:
            event_start(LinksEnumerator *tran,
                        LinksEnumeratorClient *client):
               event_base(event_id,tran,client)
            { }
         };


         uint4 const event_start::event_id =
         Csi::Event::registerType("Cora::PbRouter::LinksEnumerator::event_start");


         void event_start::create_and_post(LinksEnumerator *tran,
                                        LinksEnumeratorClient *client)
         {
            try { (new event_start(tran,client))->post(); }
            catch(Csi::Event::BadPost &) { }
         }


         ////////////////////////////////////////////////////////////
         // class event_failure
         ////////////////////////////////////////////////////////////
         class event_failure: public event_base
         {
         public:
            static uint4 const event_id;
            typedef LinksEnumeratorClient::failure_type failure_type;
            failure_type failure;

            static void create_and_post(LinksEnumerator *tran,
                                        LinksEnumeratorClient *client,
                                        failure_type failure);

            virtual void notify() { client->on_failure(tran,failure); }
            
         private:
            event_failure(LinksEnumerator *tran,
                        LinksEnumeratorClient *client,
                          failure_type failure_):
               event_base(event_id,tran,client),
               failure(failure_)
            { }
         };


         uint4 const event_failure::event_id =
         Csi::Event::registerType("Cora::PbRouter::LinksEnumerator::event_failure");


         void event_failure::create_and_post(LinksEnumerator *tran,
                                             LinksEnumeratorClient *client,
                                             failure_type failure)
         {
            try { (new event_failure(tran,client,failure))->post(); }
            catch(Csi::Event::BadPost &) { }
         }


         ////////////////////////////////////////////////////////////
         // class event_link_not
         ////////////////////////////////////////////////////////////
         class event_link_not: public event_base
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

         private:
            ////////////////////////////////////////////////////////////
            // node1_address
            ////////////////////////////////////////////////////////////
            uint2 node1_address;

            ////////////////////////////////////////////////////////////
            // node1_net_map_id
            ////////////////////////////////////////////////////////////
            uint4 node1_net_map_id;

            ////////////////////////////////////////////////////////////
            // node1_is_router
            ////////////////////////////////////////////////////////////
            bool node1_is_router;

            ////////////////////////////////////////////////////////////
            // node2_address
            ////////////////////////////////////////////////////////////
            uint2 node2_address;

            ////////////////////////////////////////////////////////////
            // node2_net_map_id
            ////////////////////////////////////////////////////////////
            uint4 node2_net_map_id;

            ////////////////////////////////////////////////////////////
            // node2_is_router
            ////////////////////////////////////////////////////////////
            bool node2_is_router;

            ////////////////////////////////////////////////////////////
            // worst_case_response_time
            ////////////////////////////////////////////////////////////
            uint4 worst_case_resp_time;

            ////////////////////////////////////////////////////////////
            // change_code
            ////////////////////////////////////////////////////////////
            uint2 change_code;

            friend class Cora::PbRouter::LinksEnumerator;
            
         public:
            ////////////////////////////////////////////////////////////
            // create
            ////////////////////////////////////////////////////////////
            static event_link_not *create(
               LinksEnumerator *tran,
               LinksEnumeratorClient *client,
               uint2 change_code);
            
            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify();
            
         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_link_not(
               LinksEnumerator *tran,
               LinksEnumeratorClient *client,
               uint2 change_code_):
               event_base(event_id,tran,client),
               change_code(change_code_)
            { }
         };


         uint4 const event_link_not::event_id =
         Csi::Event::registerType("Cora::PbRouter::LinksEnumerator::event_link_not");


         event_link_not *event_link_not::create(
            LinksEnumerator *tran,
            LinksEnumeratorClient *client,
            uint2 change_code)
         { return new event_link_not(tran,client,change_code); }


         void event_link_not::notify()
         {
            switch(change_code)
            {
            case 1:             // added
               client->on_link_added(
                  tran,
                  node1_address,node1_net_map_id,node1_is_router,
                  node2_address,node2_net_map_id,node2_is_router,
                  worst_case_resp_time);
               break;

            case 2:
               client->on_link_deleted(
                  tran,node1_address,node1_net_map_id,node2_address,node2_net_map_id);
               break;

            case 3:
               client->on_link_changed(
                  tran,
                  node1_address,node1_net_map_id,node1_is_router,
                  node2_address,node2_net_map_id,node2_is_router,
                  worst_case_resp_time);
               break;
            }
         } // notify
      };


      ////////////////////////////////////////////////////////////
      // class LinksEnumerator definitions
      ////////////////////////////////////////////////////////////
      LinksEnumerator::LinksEnumerator():
         client(0),
         state(state_standby)
      { }

      
      LinksEnumerator::~LinksEnumerator()
      { finish(); }

      
      void LinksEnumerator::start(
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

      
      void LinksEnumerator::start(
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

      
      void LinksEnumerator::finish()
      {
         client = 0;
         state = state_standby;
         PbRouterBase::finish();
      } // finish

      void LinksEnumerator::format_failure(std::ostream &out, client_type::failure_type failure)
      {
         switch(failure)
         {
            case client_type::failure_connection_failed:
            case client_type::failure_server_session_failed:
               PbRouterBase::format_failure(out, pbrouterbase_failure_session);
               break;

            case client_type::failure_invalid_logon:
               PbRouterBase::format_failure(out, pbrouterbase_failure_logon);
               break;

            case client_type::failure_server_security_blocked:
               PbRouterBase::format_failure(out, pbrouterbase_failure_security);
               break;
              
            case client_type::failure_unsupported:
               PbRouterBase::format_failure(out, pbrouterbase_failure_unsupported);
               break;

            case client_type::failure_invalid_router_id:
               PbRouterBase::format_failure(out, pbrouterbase_failure_invalid_router_id);
               break;
            default:
               PbRouterBase::format_failure(out, pbrouterbase_failure_unknown);
               break;
         }
      }

      
      void LinksEnumerator::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace LinksEnumeratorHelpers;
         event_base *event = dynamic_cast<event_base *>(ev.get_rep());

         if(ev->getType() == event_failure::event_id)
            finish();
         if(event && LinksEnumeratorClient::is_valid_instance(event->client))
            event->notify();
         else
            finish();
      } // receive

      
      void LinksEnumerator::on_pbrouterbase_ready()
      {
         Csi::Messaging::Message cmd(pbrouter_session,Messages::links_enum_start_cmd);
         cmd.addUInt4(++last_tran_no);
         router->sendMessage(&cmd);
         state = state_active; 
      } // on_pbrouterbase_ready

      
      void LinksEnumerator::on_pbrouterbase_failure(pbrouterbase_failure_type failure_)
      {
         using namespace LinksEnumeratorHelpers;
         client_type::failure_type failure;
         switch(failure_)
         {
         case pbrouterbase_failure_logon:
            failure = client_type::failure_invalid_logon;
            break;
            
         case pbrouterbase_failure_session:
            failure = client_type::failure_server_session_failed;
            break;
            
         case pbrouterbase_failure_invalid_router_id:
            failure = client_type::failure_invalid_router_id;
            break;
            
         case pbrouterbase_failure_unsupported:
            failure = client_type::failure_unsupported;
            break;
            
         case pbrouterbase_failure_security:
            failure = client_type::failure_server_security_blocked;
            break;
            
         default:
            failure = client_type::failure_unknown;
            break;
         }
         event_failure::create_and_post(this,client,failure);
      } // on_pbrouterbase_failure

      
      void LinksEnumerator::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         using namespace LinksEnumeratorHelpers;
         if(state == state_active)
         {
            switch(msg->getMsgType())
            {
            case Messages::links_enum_start_ack:
               on_start_ack(msg);
               break;
               
            case Messages::links_enum_stopped_not:
               on_stopped_not(msg);
               break;
               
            case Messages::links_enum_link_not:
               on_change_not(msg);
               break;
               
            default:
               PbRouterBase::onNetMessage(rtr,msg);
               break;
            }
         }
         else
            PbRouterBase::onNetMessage(rtr,msg);
      } // onNetMessage 


      void LinksEnumerator::on_start_ack(Csi::Messaging::Message *message)
      {
         using namespace LinksEnumeratorHelpers;
         uint4 tran_no;
         uint4 links_count;
         message->readUInt4(tran_no);
         message->readUInt4(links_count);
         for( uint4 i = 0; i < links_count; i++ )
         {
            event_link_not *event = event_link_not::create(this,client,1);
            message->readUInt2(event->node1_address);
            message->readUInt4(event->node1_net_map_id);
            message->readBool(event->node1_is_router);
            message->readUInt2(event->node2_address);
            message->readUInt4(event->node2_net_map_id);
            message->readBool(event->node2_is_router);
            message->readUInt4(event->worst_case_resp_time);
            event->post();
         }

         event_start::create_and_post(this,client);
      } // on_start_ack

      
      void LinksEnumerator::on_stopped_not(Csi::Messaging::Message *message)
      {
         using namespace LinksEnumeratorHelpers;
         event_failure::create_and_post(this,client,LinksEnumeratorClient::failure_unknown);
      } // on_stopped_not


      void LinksEnumerator::on_change_not(Csi::Messaging::Message *message)
      {
         using namespace LinksEnumeratorHelpers;

         uint4 tran_no;
         uint2 change_code;
         event_link_not *event;
         
         message->readUInt4(tran_no);
         message->readUInt2(change_code);
         event = event_link_not::create(this,client,change_code);
         message->readUInt2(event->node1_address);
         message->readUInt4(event->node1_net_map_id);
         message->readBool(event->node1_is_router);
         message->readUInt2(event->node2_address);
         message->readUInt4(event->node2_net_map_id);
         message->readBool(event->node2_is_router);
         message->readUInt4(event->worst_case_resp_time);
         event->post();
      } // on_change_not
   };
};

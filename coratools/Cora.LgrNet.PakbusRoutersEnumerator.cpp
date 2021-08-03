/* Cora.LgrNet.PakbusRoutersEnumerator.cpp

   Copyright (C) 2002, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 11 June 2002
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2019-11-21 14:57:26 -0600 (Thu, 21 Nov 2019) $ 
   Last Changed by: $Author: amortenson $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.LgrNet.PakbusRoutersEnumerator.h"
#include "Cora.LgrNet.Defs.h"
#include <sstream>


namespace Cora
{
   namespace LgrNet
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
            // enumerator
            ////////////////////////////////////////////////////////////
            typedef PakbusRoutersEnumerator enumerator_type;
            enumerator_type *enumerator;

            ////////////////////////////////////////////////////////////
            // client
            ////////////////////////////////////////////////////////////
            typedef enumerator_type::client_type client_type;
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
               enumerator_type *enumerator_,
               client_type *client_):
               Event(event_id,enumerator_),
               enumerator(enumerator_),
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
               enumerator_type *enumerator,
               client_type *client)
            {
               try {(new event_started(enumerator,client))->post(); }
               catch(Csi::Event::BadPost &) { }
            }

            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            { client->on_started(enumerator); }

         private:
            ////////////////////////////////////////////////////////////
            // conctructor
            ////////////////////////////////////////////////////////////
            event_started(
               enumerator_type *enumerator,
               client_type *client):
               event_base(event_id,enumerator,client)
            { }
         };


         uint4 const event_started::event_id =
         Csi::Event::registerType("Cora::LgrNet::PakbusRoutersEnumerator::event_started");


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
               enumerator_type *enumerator,
               client_type *client,
               failure_type failure)
            {
               try {(new event_failure(enumerator,client,failure))->post(); }
               catch(Csi::Event::BadPost &) { }
            }

            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            { client->on_failure(enumerator,failure); }
            
         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_failure(
               enumerator_type *enumerator,
               client_type *client,
               failure_type failure_):
               event_base(event_id,enumerator,client),
               failure(failure_)
            { }
         };


         uint4 const event_failure::event_id =
         Csi::Event::registerType("Cora::LgrNet::PakbusRoutersEnumerator::event_failure");


         ////////////////////////////////////////////////////////////
         // class event_change
         ////////////////////////////////////////////////////////////
         class event_change: public event_base
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // change
            ////////////////////////////////////////////////////////////
            enum change_type
            {
               change_added = 1,
               change_deleted = 2,
            } change;

            ////////////////////////////////////////////////////////////
            // pakbus_router_name
            ////////////////////////////////////////////////////////////
            StrUni pakbus_router_name;
            
            ////////////////////////////////////////////////////////////
            // create_and_post
            ////////////////////////////////////////////////////////////
            static void create_and_post(
               enumerator_type *enumerator,
               client_type *client,
               change_type change,
               StrUni const &pakbus_router_name)
            {
               try {(new event_change(enumerator,client,change,pakbus_router_name))->post(); }
               catch(Csi::Event::BadPost &) { }
            }

            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            {
               if(change == change_added)
                  client->on_router_added(enumerator,pakbus_router_name);
               else
                  client->on_router_deleted(enumerator,pakbus_router_name);
            }

         private:
            ////////////////////////////////////////////////////////////
            // conctructor
            ////////////////////////////////////////////////////////////
            event_change(
               enumerator_type *enumerator,
               client_type *client,
               change_type change_,
               StrUni const &pakbus_router_name_):
               event_base(event_id,enumerator,client),
               change(change_),
               pakbus_router_name(pakbus_router_name_)
            { }
         };


         uint4 const event_change::event_id =
         Csi::Event::registerType("Cora::LgrNet::PakbusRoutersEnumerator::event_change");
      };


      ////////////////////////////////////////////////////////////
      // class PakbusRoutersEnumerator definitions
      ////////////////////////////////////////////////////////////
      PakbusRoutersEnumerator::PakbusRoutersEnumerator():
         client(0),
         state(state_standby)
      { }

      
      PakbusRoutersEnumerator::~PakbusRoutersEnumerator()
      { finish(); }

      
      void PakbusRoutersEnumerator::start(client_type *client_, router_handle &router)
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

      
      void PakbusRoutersEnumerator::start(client_type *client_, ClientBase *other_component)
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
      
      
      void PakbusRoutersEnumerator::finish()
      {
         client = 0;
         state = state_standby;
         ClientBase::finish();
      } // finish

      void PakbusRoutersEnumerator::format_failure(std::ostream &out, client_type::failure_type failure)
      {
         switch(failure)
         {
            case client_type::failure_session_failed:
               ClientBase::describe_failure(out, ClientBase::corabase_failure_session);
               break;
            case client_type::failure_invalid_logon:
               ClientBase::describe_failure(out, ClientBase::corabase_failure_logon);
               break;
            case client_type::failure_unsupported:
               ClientBase::describe_failure(out, ClientBase::corabase_failure_unsupported);
               break;
            case client_type::failure_server_security_blocked:
               ClientBase::describe_failure(out, ClientBase::corabase_failure_security);
               break;
            default:
               ClientBase::describe_failure(out, ClientBase::corabase_failure_unknown);
               break;
         }
      }
      
      void PakbusRoutersEnumerator::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(state == state_active)
         {
            switch(msg->getMsgType())
            {
            case Messages::enum_pakbus_routers_start_ack:
               on_old_start_ack(msg);
               break;

            case Messages::enum_pakbus_routers_not:
               on_old_notification(msg);
               break;

            case Messages::enum_pakbus_router_names_start_ack:
               on_start_ack(msg);
               break;

            case Messages::enum_pakbus_router_names_not:
               on_notification(msg);
               break;

            case Messages::enum_pakbus_routers_stopped_not:
            case Messages::enum_pakbus_router_names_stopped_not:
               event_failure::create_and_post(
                  this,
                  client,
                  client_type::failure_session_failed);
               break;

            default:
               ClientBase::onNetMessage(rtr,msg);
               break;
            }
         }
         else
            ClientBase::onNetMessage(rtr,msg);
      } // onNetMessage
      
      
      void PakbusRoutersEnumerator::on_corabase_ready()
      {
         state = state_active;
         if(interface_version <= Csi::VersionNumber("1.3.4.7"))
         {
            Csi::Messaging::Message cmd(net_session,Messages::enum_pakbus_routers_start_cmd);
            cmd.addUInt4(++last_tran_no);
            router->sendMessage(&cmd); 
         }
         else
         {
            Csi::Messaging::Message cmd(
               net_session,
               Messages::enum_pakbus_router_names_start_cmd);
            cmd.addUInt4(++last_tran_no);
            router->sendMessage(&cmd); 
         }
      } // on_corabase_ready

      
      void PakbusRoutersEnumerator::on_corabase_failure(corabase_failure_type failure)
      {
         client_type::failure_type client_failure;
         switch(failure)
         {
         case corabase_failure_logon:
            client_failure = client_type::failure_invalid_logon;
            break;

         case corabase_failure_session:
            client_failure = client_type::failure_session_failed;
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

      
      void PakbusRoutersEnumerator::on_corabase_session_failure()
      {
         event_failure::create_and_post(this,client,client_type::failure_session_failed);
      } // on_corabase_session_failure

      
      void PakbusRoutersEnumerator::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         event_base *event = static_cast<event_base *>(ev.get_rep());
         if(event->client == client && client_type::is_valid_instance(client))
         {
            if(event->getType() == event_failure::event_id)
               finish();
            event->notify();
         }
         else
            finish();
      } // receive


      void PakbusRoutersEnumerator::on_start_ack(Csi::Messaging::Message *message)
      { event_started::create_and_post(this,client); }


      void PakbusRoutersEnumerator::on_notification(Csi::Messaging::Message *message)
      {
         uint4 tran_no;
         uint4 change;
         StrUni name;
         
         message->readUInt4(tran_no);
         message->readUInt4(change);
         message->readWStr(name);
         event_change::create_and_post(
            this,
            client,
            static_cast<event_change::change_type>(change),
            name);
      } // on_notification

      
      void PakbusRoutersEnumerator::on_old_start_ack(Csi::Messaging::Message *message)
      {
         uint4 tran_no;
         uint4 outcome;
         uint4 routers_count;
         uint2 router_pakbus_address; 

         message->readUInt4(tran_no);
         message->readUInt4(outcome);
         message->readUInt4(routers_count);
         if(outcome == 1)
         {
            for(uint4 i = 0; i < routers_count; ++i)
            {
               std::ostringstream router_name;
               
               message->readUInt2(router_pakbus_address);
               router_name << router_pakbus_address;
               event_change::create_and_post(
                  this,client,event_change::change_added,router_name.str().c_str());
            }
            event_started::create_and_post(this,client);
         }
         else
            event_failure::create_and_post(this,client,client_type::failure_unknown);
      } // on_start_ack

      
      void PakbusRoutersEnumerator::on_old_notification(Csi::Messaging::Message *message)
      {
         uint4 tran_no;
         uint4 change;
         uint2 router_pakbus_address;
         std::ostringstream router_name;
         
         message->readUInt4(tran_no);
         message->readUInt4(change);
         message->readUInt2(router_pakbus_address);
         router_name << router_pakbus_address;
         event_change::create_and_post(
            this,
            client,
            static_cast<event_change::change_type>(change),
            router_name.str().c_str());
      } // on_notification
   };
};

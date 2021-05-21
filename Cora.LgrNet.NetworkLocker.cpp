/* Cora.LgrNet.NetworkLocker.cpp

   Copyright (C) 2003, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 29 April 2003
   Last Change: Thursday 12 July 2018
   Last Commit: $Date: 2018-07-12 09:03:38 -0600 (Thu, 12 Jul 2018) $ 
   Last Changed by: $Author: jon $

*/

#pragma warning(disable: 4996)
#pragma hdrstop               // stop creation of precompiled header
#include "Cora.LgrNet.NetworkLocker.h"
#include "Cora.LgrNet.Defs.h"
#include "coratools.strings.h"
#include "boost/format.hpp"


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
            // client
            ////////////////////////////////////////////////////////////
            typedef NetworkLockerClient client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // locker
            ////////////////////////////////////////////////////////////
            typedef NetworkLocker locker_type;
            locker_type *locker;

         protected:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_base(
               uint4 event_id,
               locker_type *locker_,
               client_type *client_):
               Event(event_id,locker_),
               locker(locker_),
               client(client_)
            { }

         public:
            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify() = 0;

            ////////////////////////////////////////////////////////////
            // do_notify
            ////////////////////////////////////////////////////////////
            void do_notify()
            {
               if(client_type::is_valid_instance(client))
                  notify();
            }
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
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            { client->on_started(locker); }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_started(
               locker_type *locker,
               client_type *client):
               event_base(event_id,locker,client)
            { }

         public:
            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               locker_type *locker,
               client_type *client)
            {
               try{(new event_started(locker,client))->post();}
               catch(Csi::Event::BadPost &) { }
            } 
         };


         uint4 const event_started::event_id =
         Csi::Event::registerType("Cora::LgrNet::NetworkLocker::event_started");


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
            // oc_logon_namne
            ////////////////////////////////////////////////////////////
            StrUni oc_logon_name;

            ////////////////////////////////////////////////////////////
            // oc_app_name
            ////////////////////////////////////////////////////////////
            StrUni oc_app_name;

            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            { client->on_failure(locker,failure,oc_logon_name,oc_app_name); }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_failure(
               locker_type *locker,
               client_type *client,
               failure_type failure_,
               StrUni const &oc_logon_name_,
               StrUni const &oc_app_name_):
               event_base(event_id,locker,client),
               failure(failure_),
               oc_logon_name(oc_logon_name_),
               oc_app_name(oc_app_name_)
            { }

         public:
            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               locker_type *locker,
               client_type *client,
               failure_type failure,
               StrUni const &oc_logon_name = L"",
               StrUni const &oc_app_name = L"")
            {
               try{(new event_failure(locker,client,failure,oc_logon_name,oc_app_name))->post();}
               catch(Csi::Event::BadPost &) { }
            } 
         };


         uint4 const event_failure::event_id =
         Csi::Event::registerType("Cora::LgrNet::NetworkLocker::event_failure");
      };

      
      ////////////////////////////////////////////////////////////
      // class NetworkLocker definitions
      ////////////////////////////////////////////////////////////
      void NetworkLocker::start(
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

      
      void NetworkLocker::start(
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


      void NetworkLocker::describe_failure(
         std::ostream &out,
         client_type::failure_type failure,
         StrUni const &other_logon_name,
         StrUni const &other_app_name)
      {
         using namespace NetworkLockerStrings;
         switch(failure)
         {
         case client_type::failure_invalid_logon:
            ClientBase::describe_failure(out, corabase_failure_logon);
            break;
            
         case client_type::failure_session_broken:
            ClientBase::describe_failure(out, corabase_failure_session);
            break;
            
         case client_type::failure_unsupported:
            ClientBase::describe_failure(out, corabase_failure_unsupported);
            break;
            
         case client_type::failure_server_security_blocked:
            ClientBase::describe_failure(out, corabase_failure_security);
            break;
            
         case client_type::failure_already_locked:
            if(other_logon_name.length() > 0)
            {
               out << boost::format(my_strings[strid_failure_already_locked].c_str()) %
                  other_logon_name % other_app_name;
            }
            else
            {
               out << boost::format(my_strings[strid_failure_already_locked_no_user].c_str()) %
                  other_app_name;
            }
            break;
         }
      } // describe_failure
      
      
      void NetworkLocker::on_corabase_ready()
      {
         Csi::Messaging::Message cmd(
            net_session,
            Messages::lock_network_start_cmd);
         cmd.addUInt4(++last_tran_no);
         state = state_active;
         router->sendMessage(&cmd);
      } // on_corabase_ready
      
      
      void NetworkLocker::on_corabase_failure(corabase_failure_type failure)
      {
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
         event_failure::cpost(this,client,client_failure);
      } // on_corabase_failure
      
      
      void NetworkLocker::on_corabase_session_failure()
      {
         event_failure::cpost(this,client,client_type::failure_session_broken);
      } // on_corabase_session_failure
      
      
      void NetworkLocker::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         event_base *event = dynamic_cast<event_base *>(ev.get_rep());
         if(event)
         {
            client_type *client = this->client;
            if(event->getType() == event_failure::event_id)
               finish();
            event->do_notify(); 
         }
      } // receive
      
      
      void NetworkLocker::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(state == state_active)
         {
            if(msg->getMsgType() == Messages::lock_network_start_ack)
            {
               event_started::cpost(this,client);
            }
            else if(msg->getMsgType() == Messages::lock_network_stopped_not)
            {
               uint4 tran_no;
               uint4 reason;
               StrUni oc_logon_name;
               StrUni oc_app_name;
               client_type::failure_type failure;
               
               msg->readUInt4(tran_no);
               msg->readUInt4(reason);
               switch(reason)
               {
               case 4:
                  failure = client_type::failure_already_locked;
                  msg->readWStr(oc_logon_name);
                  msg->readWStr(oc_app_name);
                  break;

               default:
                  failure = client_type::failure_unknown;
                  break;
               }
               event_failure::cpost(this,client,failure,oc_logon_name,oc_app_name);
            }
            else
               ClientBase::onNetMessage(rtr,msg);
         }
         else
            ClientBase::onNetMessage(rtr,msg);
      } // onNetMessage 
   };
};


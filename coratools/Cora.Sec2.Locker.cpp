/* Cora.Sec2.Locker.cpp

   Copyright (C) 2002, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 31 December 2002
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Last Changed by: $Author: jon $

*/

#pragma warning(disable:4786) // disable overlong symbols warning
#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Sec2.Locker.h"


namespace Cora
{
   namespace Sec2
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
            // locker
            ////////////////////////////////////////////////////////////
            typedef Locker locker_type;
            locker_type *locker;

            ////////////////////////////////////////////////////////////
            // client
            ////////////////////////////////////////////////////////////
            typedef locker_type::client_type client_type;
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
               locker_type *locker_,
               client_type *client_):
               Event(event_id,locker_),
               locker(locker_),
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
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            { client->on_started(locker); }

            ////////////////////////////////////////////////////////////
            // create_and_post
            ////////////////////////////////////////////////////////////
            static void create_and_post(
               locker_type *locker,
               client_type *client)
            {
               try{(new event_started(locker,client))->post();}
               catch(Csi::Event::BadPost &) { }
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_started(
               locker_type *locker,
               client_type *client):
               event_base(event_id,locker,client)
            { } 
         };


         uint4 const event_started::event_id =
         Csi::Event::registerType("Cora::Sec2::Locker::event_started");


         ////////////////////////////////////////////////////////////
         // class event_stopped
         ////////////////////////////////////////////////////////////
         class event_stopped: public event_base
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // reason
            ////////////////////////////////////////////////////////////
            typedef client_type::stopped_reason_type reason_type;
            reason_type reason;
            
            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            { client->on_stopped(locker,reason); }

            ////////////////////////////////////////////////////////////
            // create_and_post
            ////////////////////////////////////////////////////////////
            static void create_and_post(
               locker_type *locker,
               client_type *client,
               reason_type reason)
            {
               try{(new event_stopped(locker,client,reason))->post();}
               catch(Csi::Event::BadPost &) { }
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_stopped(
               locker_type *locker,
               client_type *client,
               reason_type reason_):
               event_base(event_id,locker,client),
               reason(reason_)
            { } 
         };


         uint4 const event_stopped::event_id =
         Csi::Event::registerType("Cora::Sec2::Locker::event_stopped");
      };


      ////////////////////////////////////////////////////////////
      // class Locker definitions
      ////////////////////////////////////////////////////////////
      Locker::Locker():
         client(0),
         state(state_standby)
      { }

      
      Locker::~Locker()
      { finish(); }

      
      void Locker::start(
         client_type *client_,
         router_handle &router)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
            {
               client = client_;
               state = state_delegate;
               Sec2Base::start(router);
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start

      
      void Locker::start(
         client_type *client_,
         ClientBase *other_component)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
            {
               client = client_;
               state = state_delegate;
               Sec2Base::start(other_component);
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start

      
      void Locker::finish()
      {
         client = 0;
         state = state_standby;
         Sec2Base::finish();
      } // finish

      
      bool Locker::cancel()
      {
         bool rtn = false;
         if(state == state_active)
         {
            Csi::Messaging::Message command(sec2_session,Messages::lock_stop_cmd);
            command.addUInt4(lock_tran_no);
            state = state_cancel;
            router->sendMessage(&command);
         }
         return rtn;
      } // cancel

      
      void Locker::on_sec2base_ready()
      {
         Csi::Messaging::Message command(sec2_session,Messages::lock_start_cmd);
         command.addUInt4(lock_tran_no = ++last_tran_no);
         state = state_active;
         router->sendMessage(&command);
      } // on_sec2base_ready

      
      void Locker::on_sec2base_failure(sec2base_failure_type failure)
      {
         client_type::stopped_reason_type reason;
         switch(failure)
         {
         case sec2base_failure_logon:
            reason = client_type::stopped_reason_invalid_logon;
            break;
            
         case sec2base_failure_session:
            reason = client_type::stopped_reason_connection_failed;
            break;
            
         case sec2base_failure_unsupported:
            reason = client_type::stopped_reason_unsupported;
            break;
            
         case sec2base_failure_security:
            reason = client_type::stopped_reason_insufficient_access;
            break;
            
         default:
            reason = client_type::stopped_reason_unknown;
            break;
         }
         event_stopped::create_and_post(this,client,reason);
      } // on_sec2base_failure

      
      void Locker::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(state == state_active || state == state_cancel)
         {
            if(msg->getMsgType() == Messages::lock_start_ack)
            {
               uint4 tran_no;
               uint4 outcome;
               msg->readUInt4(tran_no);
               msg->readUInt4(outcome);
               if(tran_no == lock_tran_no)
               {
                  if(outcome == 1)
                     event_started::create_and_post(this,client);
                  else
                  {
                     client_type::stopped_reason_type reason;
                     switch(outcome)
                     {
                     case 3:
                        reason = client_type::stopped_reason_insufficient_access;
                        break;

                     case 4:
                        reason = client_type::stopped_reason_already_locked;
                        break;

                     default:
                        reason = client_type::stopped_reason_unknown;
                        break;
                     }
                     event_stopped::create_and_post(this,client,reason);
                  }
               }
            }
            else if(msg->getMsgType() == Messages::lock_stopped_not)
            {
               uint4 tran_no;
               uint4 server_reason;
               msg->readUInt4(tran_no);
               msg->readUInt4(server_reason);
               if(tran_no == lock_tran_no)
               {
                  client_type::stopped_reason_type client_reason;
                  switch(server_reason)
                  {
                  case 1:
                     client_reason = client_type::stopped_reason_requested;
                     break;
                     
                  case 2:
                     client_reason = client_type::stopped_reason_connection_failed;
                     break;
                     
                  case 3:
                     client_reason = client_type::stopped_reason_insufficient_access;
                     break;

                  default:
                     client_reason = client_type::stopped_reason_unknown;
                     break;
                  }
                  event_stopped::create_and_post(this,client,client_reason);
               } 
            }
            else
               Sec2Base::onNetMessage(rtr,msg);
         }
         else
            Sec2Base::onNetMessage(rtr,msg);
      } // onNetMessage

      
      void Locker::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         event_base *event = dynamic_cast<event_base *>(ev.get_rep());
         if(event)
         {
            client_type *client = this->client;
            if(state != state_standby)
            {
               if(event->getType() == event_stopped::event_id)
                  finish();
               if(event->client == client && client_type::is_valid_instance(client))
                  event->notify();
               else
                  finish();
            }
         }
      } // receive
   };
};

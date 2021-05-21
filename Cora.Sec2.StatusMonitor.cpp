/* Cora.Sec2.StatusMonitor.cpp

   Copyright (C) 2002, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 24 December 2002
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2019-10-29 16:13:19 -0600 (Tue, 29 Oct 2019) $ 
   Last Changed by: $Author: amortenson $

*/

#pragma warning(disable:4786) // disable overlong symbols warning
#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Sec2.StatusMonitor.h"


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
            // monitor
            ////////////////////////////////////////////////////////////
            typedef StatusMonitor monitor_type;
            monitor_type *monitor;

            ////////////////////////////////////////////////////////////
            // client
            ////////////////////////////////////////////////////////////
            typedef monitor_type::client_type client_type;
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
               monitor_type *monitor_,
               client_type *client_):
               Event(event_id,monitor_),
               monitor(monitor_),
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
            // security_enabled
            ////////////////////////////////////////////////////////////
            bool security_enabled;

            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            {
               if(client_type::is_valid_instance(client))
                  client->on_started(monitor,security_enabled);
            }

            ////////////////////////////////////////////////////////////
            // create_and_post
            ////////////////////////////////////////////////////////////
            static void create_and_post(
               monitor_type *monitor,
               client_type *client,
               bool security_enabled)
            {
               try{(new event_started(monitor,client,security_enabled))->post();}
               catch(Csi::Event::BadPost &) { }
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_started(
               monitor_type *monitor,
               client_type *client,
               bool security_enabled_):
               event_base(event_id,monitor,client),
               security_enabled(security_enabled_)
            { }
         };


         uint4 const event_started::event_id =
         Csi::Event::registerType("Cora::Sec2::StatusMonitor::event_started");


         ////////////////////////////////////////////////////////////
         // class event_changed
         ////////////////////////////////////////////////////////////
         class event_changed: public event_base
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // security_enabled
            ////////////////////////////////////////////////////////////
            bool security_enabled;

            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            {
               if(client_type::is_valid_instance(client))
                  client->on_security_changed(monitor,security_enabled);
            }

            ////////////////////////////////////////////////////////////
            // create_and_post
            ////////////////////////////////////////////////////////////
            static void create_and_post(
               monitor_type *monitor,
               client_type *client,
               bool security_enabled)
            {
               try{(new event_changed(monitor,client,security_enabled))->post();}
               catch(Csi::Event::BadPost &) { }
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_changed(
               monitor_type *monitor,
               client_type *client,
               bool security_enabled_):
               event_base(event_id,monitor,client),
               security_enabled(security_enabled_)
            { }
         };


         uint4 const event_changed::event_id =
         Csi::Event::registerType("Cora::Sec2::StatusMonitor::event_changed");


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
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            {
               if(client_type::is_valid_instance(client))
                  client->on_failure(monitor,failure);
            }

            ////////////////////////////////////////////////////////////
            // create_and_post
            ////////////////////////////////////////////////////////////
            static void create_and_post(
               monitor_type *monitor,
               client_type *client,
               failure_type failure)
            {
               try{(new event_failure(monitor,client,failure))->post();}
               catch(Csi::Event::BadPost &) { }
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_failure(
               monitor_type *monitor,
               client_type *client,
               failure_type failure_):
               event_base(event_id,monitor,client),
               failure(failure_)
            { }
         };


         uint4 const event_failure::event_id =
         Csi::Event::registerType("Cora::Sec2::StatusMonitor::event_failure");
      };


      ////////////////////////////////////////////////////////////
      // class StatusMonitor definitions
      ////////////////////////////////////////////////////////////
      StatusMonitor::StatusMonitor():
         client(0),
         state(state_standby)
      { }

      
      StatusMonitor::~StatusMonitor()
      { finish(); }

      
      void StatusMonitor::start(
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

      
      void StatusMonitor::start(
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

      
      void StatusMonitor::finish()
      {
         client = 0;
         state = state_standby;
         Sec2Base::finish();
      } // finish

      void StatusMonitor::format_failure(std::ostream &out, client_type::failure_type failure)
      {
         switch(failure)
         {
            case client_type::failure_connection_failed:
               Sec2Base::format_failure(out, sec2base_failure_session);
               break;
            case client_type::failure_logon:
               Sec2Base::format_failure(out, sec2base_failure_logon);
               break;
            case client_type::failure_insufficient_access:
               Sec2Base::format_failure(out, sec2base_failure_security);
               break;
            case client_type::failure_unsupported:
               Sec2Base::format_failure(out, sec2base_failure_unsupported);
               break;
            default:
               Sec2Base::format_failure(out, sec2base_failure_unknown);
               break;
         }
      }
      
      void StatusMonitor::on_sec2base_ready()
      {
         Csi::Messaging::Message command(sec2_session,Messages::monitor_status_start_cmd);
         command.addUInt4(++last_tran_no);
         state = state_before_active;
         router->sendMessage(&command);
      } // on_sec2base_ready

      
      void StatusMonitor::on_sec2base_failure(sec2base_failure_type failure)
      {
         client_type::failure_type client_failure;
         switch(failure)
         {
         case sec2base_failure_logon:
            client_failure = client_type::failure_logon;
            break;
            
         case sec2base_failure_session:
            client_failure = client_type::failure_connection_failed;
            break;
            
         case sec2base_failure_unsupported:
            client_failure = client_type::failure_unsupported;
            break;
            
         case sec2base_failure_security:
            client_failure = client_type::failure_insufficient_access;
            break;
            
         default:
            client_failure = client_type::failure_unknown;
            break;
         }
         event_failure::create_and_post(this,client,client_failure);
      } // on_sec2base_failure

      
      void StatusMonitor::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(state == state_active || state == state_before_active)
         {
            if(msg->getMsgType() == Messages::monitor_status_not)
            {
               uint4 tran_no;
               bool security_enabled;
               msg->readUInt4(tran_no);
               msg->readBool(security_enabled);
               if(state == state_before_active)
               {
                  state = state_active;
                  event_started::create_and_post(this,client,security_enabled);
               }
               else
                  event_changed::create_and_post(this,client,security_enabled);
            }
            else if(msg->getMsgType() == Messages::monitor_status_stopped_not)
            {
               uint4 tran_no;
               uint4 reason;
               client_type::failure_type failure;
               
               msg->readUInt4(tran_no);
               msg->readUInt4(reason);
               switch(reason)
               {
               case 3:
                  failure = client_type::failure_connection_failed;
                  break;
                  
               case 4:
                  failure = client_type::failure_insufficient_access;
                  break;
                  
               default:
                  failure = client_type::failure_unknown;
                  break;
               }
               event_failure::create_and_post(this,client,failure);
            }
            else
               Sec2Base::onNetMessage(rtr,msg);
         }
         else
            Sec2Base::onNetMessage(rtr,msg);
      } // onNetMessage

      
      void StatusMonitor::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         event_base *event = dynamic_cast<event_base *>(ev.get_rep());
         if(event)
         {
            client_type *client = this->client;
            if(event->getType() == event_failure::event_id)
               finish();
            if(client == event->client)
               event->notify();
         }
      } // receive
   };
};


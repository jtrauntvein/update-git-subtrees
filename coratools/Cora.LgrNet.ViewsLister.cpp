/* Cora.LgrNet.ViewsLister.cpp

   Copyright (C) 2012, Campbell Scientific, Inc

   Written by: Jon Trauntvein 
   Date Begun: Tuesday 21 August 2012
   Last Change: Tuesday 21 August 2012
   Last Commit: $Date: 2012-08-22 09:16:13 -0600 (Wed, 22 Aug 2012) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.LgrNet.ViewsLister.h"
#include "Cora.LgrNet.Defs.h"


namespace Cora
{
   namespace LgrNet
   {
      namespace
      {
         ////////////////////////////////////////////////////////////
         // class event_failure
         ////////////////////////////////////////////////////////////
         class event_failure: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // failure
            ////////////////////////////////////////////////////////////
            typedef ViewsListerClient::failure_type failure_type;
            failure_type failure;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(ViewsLister *lister, failure_type failure)
            {
               event_failure *event(new event_failure(lister, failure));
               event->post();
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_failure(ViewsLister *lister, failure_type failure_):
               Event(event_id, lister),
               failure(failure_)
            { }
         };


         uint4 const event_failure::event_id(
            Csi::Event::registerType("Cora::LgrNet::ViewsLister::event_failure"));


         ////////////////////////////////////////////////////////////
         // class event_not
         ////////////////////////////////////////////////////////////
         class event_not: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // reason
            ////////////////////////////////////////////////////////////
            uint4 reason;

            ////////////////////////////////////////////////////////////
            // view_id
            ////////////////////////////////////////////////////////////
            uint4 view_id;

            ////////////////////////////////////////////////////////////
            // view_name
            ////////////////////////////////////////////////////////////
            StrUni const view_name;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               ViewsLister *lister,
               uint4 reason,
               uint4 id,
               StrUni const &name)
            {
               event_not *event(new event_not(lister, reason, id, name));
               event->post();
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_not(
               ViewsLister *lister,
               uint4 reason_,
               uint4 id,
               StrUni const &name):
               Event(event_id, lister),
               reason(reason_),
               view_id(id),
               view_name(name)
            { }
         };


         uint4 const event_not::event_id(
            Csi::Event::registerType("Cora::LgrNet::ViewsLister::event_not"));


         ////////////////////////////////////////////////////////////
         // class event_started
         ////////////////////////////////////////////////////////////
         class event_started: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(ViewsLister *lister)
            {
               event_started *event(new event_started(lister));
               event->post();
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_started(ViewsLister *lister):
               Event(event_id, lister)
            { }
         };


         uint4 const event_started::event_id(
            Csi::Event::registerType("Cora::LgrNet::ViewsLister::event_started"));
      };


      ////////////////////////////////////////////////////////////
      // class ViewsLister definitions
      ////////////////////////////////////////////////////////////
      void ViewsLister::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         if(ev->getType() == event_failure::event_id)
         {
            event_failure *event(static_cast<event_failure *>(ev.get_rep()));
            client_type *client(this->client);
            finish();
            if(client_type::is_valid_instance(client))
               client->on_failure(this, event->failure);
         }
         else if(ev->getType() == event_not::event_id)
         {
            event_not *event(static_cast<event_not *>(ev.get_rep()));
            if(client_type::is_valid_instance(client))
            {
               switch(event->reason)
               {
               case 1:
                  client->on_view_added(this, event->view_id, event->view_name);
                  break;

               case 2:
                  client->on_view_removed(this, event->view_id, event->view_name);
                  break;

               case 3:
                  client->on_view_changed(this, event->view_id, event->view_name);
                  break;
               }
            }
            else
               finish();
         }
         else if(ev->getType() == event_started::event_id)
         {
            if(client_type::is_valid_instance(client))
               client->on_started(this);
            else
               finish();
         }
      } // receive

      
      void ViewsLister::describe_failure(std::ostream &out, client_type::failure_type failure)
      {
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

         case client_type::failure_server_security:
            ClientBase::describe_failure(out, corabase_failure_security);
            break;
            
         default:
            ClientBase::describe_failure(out, corabase_failure_unknown);
            break;
         }
      } // describe_failure

      
      void ViewsLister::on_corabase_ready()
      {
         Csi::Messaging::Message cmd(net_session, Messages::enum_views_start_cmd);
         cmd.addUInt4(++last_tran_no);
         state = state_before_start;
         router->sendMessage(&cmd);
      } // on_corabase_ready

      
      void ViewsLister::on_corabase_failure(corabase_failure_type failure_)
      {
         client_type::failure_type failure(client_type::failure_unknown);
         switch(failure_)
         {
         case corabase_failure_logon:
            failure = client_type::failure_invalid_logon;
            break;
            
         case corabase_failure_session:
            failure = client_type::failure_session_broken;
            break;
            
         case corabase_failure_unsupported:
            failure = client_type::failure_unsupported;
            break;
            
         case corabase_failure_security:
            failure = client_type::failure_server_security;
            break;
         }
         event_failure::cpost(this, failure);
      } // on_corabase_failure

      
      void ViewsLister::onNetMessage(
         Csi::Messaging::Router *router,
         Csi::Messaging::Message *message)
      {
         if(state == state_before_start || state == state_started)
         {
            if(message->getMsgType() == Messages::enum_views_not)
            {
               uint4 tran_no;
               uint4 trigger;
               uint4 count;
               uint4 view_id;
               StrUni view_name;
               
               message->readUInt4(tran_no);
               message->readUInt4(trigger);
               message->readUInt4(count);
               for(uint4 i = 0; i < count; ++i)
               {
                  message->readUInt4(view_id);
                  message->readWStr(view_name);
                  event_not::cpost(this, trigger, view_id, view_name);
               }
               if(state == state_before_start)
               {
                  state = state_started;
                  event_started::cpost(this);
               }
            }
            else if(message->getMsgType() == Messages::enum_views_stopped_not)
            {
               event_failure::cpost(this, client_type::failure_unknown);
            }
            else
               ClientBase::onNetMessage(router, message);
         }
         else
            ClientBase::onNetMessage(router, message);
      } // onNetMessage
   };
};


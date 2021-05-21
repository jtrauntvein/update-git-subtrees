/* Cora.LgrNet.ViewMonitor.cpp

   Copyright (C) 2012, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Tuesday 21 August 2012
   Last Change: Thursday 21 November 2019
   Last Commit: $Date: 2019-11-21 14:03:18 -0600 (Thu, 21 Nov 2019) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.LgrNet.ViewMonitor.h"
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
            typedef ViewMonitorClient::failure_type failure_type;
            failure_type failure;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(ViewMonitor *lister, failure_type failure)
            {
               event_failure *event(new event_failure(lister, failure));
               event->post();
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_failure(ViewMonitor *lister, failure_type failure_):
               Event(event_id, lister),
               failure(failure_)
            { }
         };


         uint4 const event_failure::event_id(
            Csi::Event::registerType("Cora::LgrNet::ViewMonitor::event_failure"));


         ////////////////////////////////////////////////////////////
         // class event_changed
         ////////////////////////////////////////////////////////////
         class event_changed: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // view_desc
            ////////////////////////////////////////////////////////////
            StrAsc const view_desc;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(ViewMonitor *monitor, StrAsc const &view_desc)
            {
               event_changed *event(new event_changed(monitor, view_desc));
               event->post();
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_changed(ViewMonitor *monitor, StrAsc const &view_desc_):
               Event(event_id, monitor),
               view_desc(view_desc_)
            { }
         };


         uint4 const event_changed::event_id(
            Csi::Event::registerType("Cora::LgrNet::ViewMonitor::event_changed"));


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
            static void cpost(ViewMonitor *lister)
            {
               event_started *event(new event_started(lister));
               event->post();
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_started(ViewMonitor *lister):
               Event(event_id, lister)
            { }
         };


         uint4 const event_started::event_id(
            Csi::Event::registerType("Cora::LgrNet::ViewMonitor::event_started"));
      };


      ////////////////////////////////////////////////////////////
      // class ViewMonitor definitions
      ////////////////////////////////////////////////////////////
      void ViewMonitor::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         if(ev->getType() == event_failure::event_id)
         {
            event_failure *event(static_cast<event_failure *>(ev.get_rep()));
            client_type *client(this->client);
            finish();
            if(client_type::is_valid_instance(client))
               client->on_failure(this, event->failure);
         }
         else if(ev->getType() == event_changed::event_id)
         {
            event_changed *event(static_cast<event_changed *>(ev.get_rep()));
            if(client_type::is_valid_instance(client))
               client->on_view_changed(this, event->view_desc);
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

      
      void ViewMonitor::describe_failure(std::ostream &out, client_type::failure_type failure)
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

         case client_type::failure_invalid_view_id:
            out << "the view that was specified does not exist";
            break;

         case client_type::failure_view_removed:
            out << "the view has been removed";
            break;
            
         default:
            ClientBase::describe_failure(out, corabase_failure_unknown);
            break;
         }
      } // describe_failure

      
      void ViewMonitor::on_corabase_ready()
      {
         Csi::Messaging::Message cmd(net_session, Messages::monitor_view_start_cmd);
         cmd.addUInt4(++last_tran_no);
         cmd.addUInt4(view_id);
         state = state_before_start;
         router->sendMessage(&cmd);
      } // on_corabase_ready

      
      void ViewMonitor::on_corabase_failure(corabase_failure_type failure_)
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

      
      void ViewMonitor::onNetMessage(
         Csi::Messaging::Router *router,
         Csi::Messaging::Message *message)
      {
         if(state == state_before_start || state == state_started)
         {
            if(message->getMsgType() == Messages::monitor_view_not)
            {
               uint4 tran_no;
               StrAsc view_desc;
               
               message->readUInt4(tran_no);
               message->readStr(view_desc);
               event_changed::cpost(this, view_desc);
               if(state == state_before_start)
               {
                  state = state_started;
                  event_started::cpost(this);
               }
            }
            else if(message->getMsgType() == Messages::monitor_view_stopped_not)
            {
               uint4 tran_no;
               uint4 reason;
               client_type::failure_type failure(client_type::failure_unknown);
               message->readUInt4(tran_no);
               message->readUInt4(reason);
               if(reason == 3)
               {
                  if(state == state_before_start)
                     failure = client_type::failure_invalid_view_id;
                  else
                     failure = client_type::failure_view_removed;
               }
               event_failure::cpost(this, failure);
            }
            else
               ClientBase::onNetMessage(router, message);
         }
         else
            ClientBase::onNetMessage(router, message);
      } // onNetMessage
   };
};


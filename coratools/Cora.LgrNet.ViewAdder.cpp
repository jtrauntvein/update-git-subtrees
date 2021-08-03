/* Cora.LgrNet.ViewAdder.cpp

   Copyright (C) 2012, Campbell Scientific, Inc

   Written by: Jon Trauntvein 
   Date Begun: Tuesday 21 August 2012
   Last Change: Tuesday 21 August 2012
   Last Commit: $Date: 2012-08-22 09:16:13 -0600 (Wed, 22 Aug 2012) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.LgrNet.ViewAdder.h"


namespace Cora
{
   namespace LgrNet
   {
      namespace
      {
         ////////////////////////////////////////////////////////////
         // class event_complete
         ////////////////////////////////////////////////////////////
         class event_complete: public Csi::Event
         {
         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            typedef ViewAdderClient::outcome_type outcome_type;
            event_complete(ViewAdder *adder, outcome_type outcome_, uint4 id):
               Event(event_id, adder),
               outcome(outcome_),
               view_id(id)
            { }
            
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // outcome
            ////////////////////////////////////////////////////////////
            outcome_type outcome;

            ////////////////////////////////////////////////////////////
            // view_id
            ////////////////////////////////////////////////////////////
            uint4 view_id;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(ViewAdder *adder, outcome_type outcome, uint4 view_id = 0xFFFFFFFF)
            {
               event_complete *event(new event_complete(adder, outcome, view_id));
               event->post();
            }
         };


         uint4 const event_complete::event_id(
            Csi::Event::registerType("Cora::LgrNet::ViewAdder::event_complete"));
      };


      ////////////////////////////////////////////////////////////
      // class ViewAdder definitions
      ////////////////////////////////////////////////////////////
      void ViewAdder::describe_outcome(std::ostream &out, client_type::outcome_type outcome)
      {
         switch(outcome)
         {
         default:
         case client_type::outcome_failure_unknown:
            ClientBase::describe_failure(out, corabase_failure_unknown);
            break;
            
         case client_type::outcome_success:
            out << "success";
            break;
            
         case client_type::outcome_failure_logon:
            ClientBase::describe_failure(out, corabase_failure_logon);
            break;
            
         case client_type::outcome_failure_session:
            ClientBase::describe_failure(out, corabase_failure_session);
            break;
            
         case client_type::outcome_failure_unsupported:
            ClientBase::describe_failure(out, corabase_failure_unsupported);
            break;
            
         case client_type::outcome_failure_security:
            ClientBase::describe_failure(out, corabase_failure_security);
            break;
            
         case client_type::outcome_failure_invalid_desc:
            out << "an invalid view description was specified";
            break;
         }
      } // describe_outcome

      
      void ViewAdder::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         if(ev->getType() == event_complete::event_id)
         {
            event_complete *event(static_cast<event_complete *>(ev.get_rep()));
            client_type *client = this->client;
            finish();
            if(client_type::is_valid_instance(client))
               client->on_complete(this, event->outcome, event->view_id);
         }
      } // receive

      
      void ViewAdder::on_corabase_ready()
      {
         Csi::Messaging::Message cmd(net_session, Messages::add_view_cmd);
         cmd.addUInt4(++last_tran_no);
         cmd.addStr(view_desc);
         state = state_active;
         router->sendMessage(&cmd);
      } // on_corabase_ready

      
      void ViewAdder::on_corabase_failure(corabase_failure_type failure_)
      {
         client_type::outcome_type outcome(client_type::outcome_failure_unknown);
         switch(failure_)
         {
         case corabase_failure_logon:
            outcome = client_type::outcome_failure_logon;
            break;
            
         case corabase_failure_session:
            outcome = client_type::outcome_failure_session;
            break;
            
         case corabase_failure_unsupported:
            outcome = client_type::outcome_failure_unsupported;
            break;
            
         case corabase_failure_security:
            outcome = client_type::outcome_failure_security;
            break;
         }
         event_complete::cpost(this, outcome);
      } // on_corabase_failure

      
      void ViewAdder::onNetMessage(
         Csi::Messaging::Router *router, Csi::Messaging::Message *message)
      {
         if(state == state_active)
         {
            if(message->getMsgType() == Messages::add_view_ack)
            {
               uint4 tran_no;
               uint4 response;
               uint4 view_id(0);
               client_type::outcome_type outcome(client_type::outcome_success);
               
               message->readUInt4(tran_no);
               message->readUInt4(response);
               if(response == 1)
                  message->readUInt4(view_id);
               else
                  outcome = client_type::outcome_failure_invalid_desc;
               event_complete::cpost(this, outcome, view_id);
            }
            else
               ClientBase::onNetMessage(router, message);
         }
         else
            ClientBase::onNetMessage(router, message);
      } // onNetMessage 
   };
};



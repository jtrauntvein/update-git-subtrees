/* Cora.LgrNet.LogsClearer.cpp

   Copyright (C) 2008, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 03 April 2008
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.LgrNet.LogsClearer.h"
#include "Cora.LgrNet.Defs.h"
#include "coratools.strings.h"


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
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // client
            ////////////////////////////////////////////////////////////
            typedef LogsClearer::client_type client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // outcome
            ////////////////////////////////////////////////////////////
            typedef client_type::outcome_type outcome_type;
            outcome_type outcome;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(LogsClearer *clearer, client_type *client, outcome_type outcome)
            {
               event_complete *event = new event_complete(clearer, client, outcome);
               event->post();
            }
            
         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_complete(
               LogsClearer *clearer, client_type *client_, outcome_type outcome_):
               Event(event_id, clearer),
               client(client_),
               outcome(outcome_)
            { }
         };
         uint4 const event_complete::event_id =
            Csi::Event::registerType("Cora::LgrNet::LogsClearer::event_complete");
      };

      
      ////////////////////////////////////////////////////////////
      // class LogsClearer definitions
      ////////////////////////////////////////////////////////////
      void LogsClearer::start(
         client_type *client, router_handle &router)
      {
         if(state != state_standby)
            throw exc_invalid_state();
         if(!client_type::is_valid_instance(client))
            throw std::invalid_argument("invalid client parameter");
         state = state_delegate;
         this->client = client;
         ClientBase::start(router);
      } // start

      
      void LogsClearer::start(
         client_type *client, ClientBase *other_component)
      {
         if(state != state_standby)
            throw exc_invalid_state();
         if(!client_type::is_valid_instance(client))
            throw std::invalid_argument("invalid client parameter");
         state = state_delegate;
         this->client = client;
         ClientBase::start(other_component);
      } // start

      
      void LogsClearer::finish()
      {
         state = state_standby;
         client = 0;
         ClientBase::finish();
      } // finish
      

      void LogsClearer::describe_outcome(std::ostream &out, client_type::outcome_type outcome)
      {
         using namespace LogsClearerStrings;
         switch(outcome)
         {
         case client_type::outcome_success:
            out << my_strings[strid_outcome_success];
            break;
            
         case client_type::outcome_failure_logon:
            describe_failure(out, corabase_failure_logon);
            break;
            
         case client_type::outcome_failure_session:
            describe_failure(out, corabase_failure_session);
            break;
            
         case client_type::outcome_failure_unsupported:
            describe_failure(out, corabase_failure_unsupported);
            break;
            
         case client_type::outcome_failure_clear_failed:
            out << my_strings[strid_outcome_clear_failed];
            break;
            
         default:
            describe_failure(out, corabase_failure_unknown);
            break;
         }
      } // describe_outcome

      
      void LogsClearer::on_corabase_ready()
      {
         Csi::Messaging::Message command(net_session, Messages::clear_logs_cmd);
         command.addUInt4(++last_tran_no);
         state = state_active;
         router->sendMessage(&command);
      } // on_corabase_ready

      
      void LogsClearer::on_corabase_failure(corabase_failure_type failure)
      {
         client_type::outcome_type outcome;
         switch(failure)
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
         event_complete::cpost(this, client, outcome);
      } // on_corabase_failure

      
      void LogsClearer::on_corabase_session_failure()
      {
         event_complete::cpost(this, client, client_type::outcome_failure_session);
      } // on_corabase_session_failure

      
      void LogsClearer::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         if(ev->getType() == event_complete::event_id)
         {
            event_complete *event = static_cast<event_complete *>(ev.get_rep());
            finish();
            if(client_type::is_valid_instance(event->client))
               event->client->on_complete(this, event->outcome);
         }
      } // receive

      
      void LogsClearer::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(state == state_active)
         {
            if(msg->getMsgType() == Messages::clear_logs_ack)
            {
               uint4 session_no;
               uint4 response;
               client_type::outcome_type outcome;
               
               msg->readUInt4(session_no);
               msg->readUInt4(response);
               switch(response)
               {
               case 1:
                  outcome = client_type::outcome_success;
                  break;
                  
               case 3:
                  outcome = client_type::outcome_failure_clear_failed;
                  break;
                  
               default:
                  outcome = client_type::outcome_unknown;
                  break;
               }
               event_complete::cpost(this, client, outcome);
            }
            else
               ClientBase::onNetMessage(rtr, msg);
         }
         else
            ClientBase::onNetMessage(rtr, msg);
      } // onNetMessage
   };
};


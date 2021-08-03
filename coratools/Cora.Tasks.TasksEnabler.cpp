/* Cora.Tasks.TasksEnabler.cpp

   Copyright (C) 2012, Campbell Scientific, Inc

   Written by: Jon Trauntvein 
   Date Begun: Wednesday 13 June 2012
   Last Change: Wednesday 13 June 2012
   Last Commit: $Date: 2012-06-13 10:50:06 -0600 (Wed, 13 Jun 2012) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Tasks.TasksEnabler.h"
#include "coratools.strings.h"


namespace Cora
{
   namespace Tasks
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
            // outcome
            ////////////////////////////////////////////////////////////
            typedef TasksEnablerClient::outcome_type outcome_type;
            outcome_type outcome;
            
            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(TasksEnabler *enabler, outcome_type outcome)
            {
               event_complete *event(new event_complete(enabler, outcome));
               event->post();
            }

         private:
            event_complete(TasksEnabler *enabler, outcome_type outcome_):
               Event(event_id, enabler),
               outcome(outcome_)
            { }
         };


         uint4 const event_complete::event_id(
            Csi::Event::registerType("Cora::Tasks::TasksEnabler::event_complete"));
      };


      ////////////////////////////////////////////////////////////
      // class TasksEnabler definitions
      ////////////////////////////////////////////////////////////
      void TasksEnabler::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         if(ev->getType() == event_complete::event_id)
         {
            event_complete *event(static_cast<event_complete *>(ev.get_rep()));
            client_type *report(client);
            finish();
            if(client_type::is_valid_instance(report))
               report->on_complete(this, event->outcome);
         }
      } // receive

      
      void TasksEnabler::describe_outcome(std::ostream &out, client_type::outcome_type outcome)
      {
         using namespace TaskAdderStrings;
         switch(outcome)
         {
         case client_type::outcome_success:
            out << my_strings[strid_outcome_success];
            break;

         case client_type::outcome_failure_logon:
            format_failure(out, tasksbase_failure_logon);
            break;

         case client_type::outcome_failure_session:
            format_failure(out, tasksbase_failure_session);
            break;

         case client_type::outcome_failure_unsupported:
            format_failure(out, tasksbase_failure_unsupported);
            break;

         case client_type::outcome_failure_security:
            format_failure(out, tasksbase_failure_security);
            break;
            
         default:
            format_failure(out, tasksbase_failure_unknown);
            break;
         }
      } // describe_outcome

      
      void TasksEnabler::on_tasks_ready()
      {
         Csi::Messaging::Message cmd(tasks_session, Messages::enable_cmd);
         cmd.addUInt4(++last_tran_no);
         cmd.addBool(tasks_enabled);
         state = state_active;
         router->sendMessage(&cmd);
      } // on_tasks_ready

      
      void TasksEnabler::on_tasks_failure(tasksbase_failure_type failure)
      {
         client_type::outcome_type outcome(client_type::outcome_unknown_failure);
         switch(failure)
         {
         case tasksbase_failure_logon:
            outcome = client_type::outcome_failure_logon;
            break;
            
         case tasksbase_failure_session:
            outcome = client_type::outcome_failure_session;
            break;
            
         case tasksbase_failure_unsupported:
            outcome = client_type::outcome_failure_unsupported;
            break;
            
         case tasksbase_failure_security:
            outcome = client_type::outcome_failure_security;
            break;
         }
         event_complete::cpost(this, outcome);
      } // on_tasks_failure

      
      void TasksEnabler::onNetMessage(
         Csi::Messaging::Router *router,  Csi::Messaging::Message *message)
      {
         if(state == state_active)
         {
            if(message->getMsgType() == Messages::enable_ack)
               event_complete::cpost(this, client_type::outcome_success);
            else
               TasksBase::onNetMessage(router, message);
         }
         else
            TasksBase::onNetMessage(router, message);
      } // onNetMessage
   }; 
};


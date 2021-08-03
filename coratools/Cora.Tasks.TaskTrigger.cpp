/* Cora.Tasks.TaskTrigger.cpp

   Copyright (C) 2012, Campbell Scientific, Inc

   Written by: Jon Trauntvein 
   Date Begun: Wednesday 13 June 2012
   Last Change: Wednesday 13 June 2012
   Last Commit: $Date: 2012-06-13 14:35:34 -0600 (Wed, 13 Jun 2012) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Tasks.TaskTrigger.h"
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
            typedef TaskTriggerClient::outcome_type outcome_type;
            outcome_type outcome;
            
            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(TaskTrigger *trigger, outcome_type outcome)
            {
               event_complete *event(new event_complete(trigger, outcome));
               event->post();
            }

         private:
            event_complete(TaskTrigger *trigger, outcome_type outcome_):
               Event(event_id, trigger),
               outcome(outcome_)
            { }
         };


         uint4 const event_complete::event_id(
            Csi::Event::registerType("Cora::Tasks::TaskTrigger::event_complete"));
      };


      ////////////////////////////////////////////////////////////
      // class TaskTrigger definitions
      ////////////////////////////////////////////////////////////
      void TaskTrigger::describe_outcome(std::ostream &out, client_type::outcome_type outcome)
      {
         using namespace TaskAdderStrings;
         switch(outcome)
         {
         case client_type::outcome_success:
            out << my_strings[strid_outcome_success];
            break;
            
         case client_type::outcome_invalid_task_id:
            out << my_strings[strid_outcome_invalid_task_id];
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

      
      void TaskTrigger::receive(Csi::SharedPtr<Csi::Event> &ev)
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

      
      void TaskTrigger::on_tasks_ready()
      {
         Csi::Messaging::Message command(tasks_session, Messages::trigger_task_cmd);
         command.addUInt4(++last_tran_no);
         command.addUInt4(task_id);
         state = state_active;
         router->sendMessage(&command);
      } // on_tasks_ready

      
      void TaskTrigger::on_tasks_failure(tasksbase_failure_type failure)
      {
         client_type::outcome_type outcome(client_type::outcome_failure_unknown);
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

      
      void TaskTrigger::onNetMessage(
            Csi::Messaging::Router *router, Csi::Messaging::Message *message)
      {
         if(state == state_active)
         {
            if(message->getMsgType() == Messages::trigger_task_ack)
            {
               uint4 tran_no;
               uint4 rcd;
               client_type::outcome_type outcome(client_type::outcome_failure_unknown);
               
               message->readUInt4(tran_no);
               message->readUInt4(rcd);
               switch(rcd)
               {
               case 1:
                  outcome = client_type::outcome_success;
                  break;

               case 2:
                  outcome = client_type::outcome_invalid_task_id;
                  break;
               }
               event_complete::cpost(this, outcome);
            }
            else
               TasksBase::onNetMessage(router, message);
         }
         else
            TasksBase::onNetMessage(router, message);
      } // onNetMessage
   };
};



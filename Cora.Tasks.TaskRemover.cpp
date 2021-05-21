/* Cora.Tasks.TaskRemover.cpp

   Copyright (C) 2012, Campbell Scientific, Inc

   Written by: Jon Trauntvein 
   Date Begun: Wednesday 16 May 2012
   Last Change: Wednesday 16 May 2012
   Last Commit: $Date: 2019-11-19 11:44:54 -0600 (Tue, 19 Nov 2019) $
   Last Changed by: $Author: amortenson $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Tasks.TaskRemover.h"
#include "coratools.strings.h"


namespace Cora
{
   namespace Tasks
   {
      namespace
      {
         class event_complete: public Csi::Event
         {
         public:
            static uint4 const event_id;
            typedef TaskRemover remover_type;
            typedef remover_type::client_type::outcome_type outcome_type;
            outcome_type outcome;

            static void cpost(remover_type *remover, outcome_type outcome)
            {
               event_complete *event(new event_complete(remover, outcome));
               event->post();
            }

         private:
            event_complete(remover_type *remover, outcome_type outcome_):
               Event(event_id, remover),
               outcome(outcome_)
            { }
         };


         uint4 const event_complete::event_id(
            Csi::Event::registerType("Cora::Tasks::TaskRemover::event_complete"));
      };


      void TaskRemover::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         if(ev->getType() == event_complete::event_id)
         {
            event_complete *event(static_cast<event_complete *>(ev.get_rep()));
            client_type *report(client);
            finish();
            if(client_type::is_valid_instance(report))
               report->on_complete(this, event->outcome);
         }
      }


      void TaskRemover::format_outcome(std::ostream &out, client_type::outcome_type outcome)
      {
         using namespace TaskAdderStrings;
         switch(outcome)
         {
         case client_type::outcome_success:
            out << my_strings[strid_outcome_success];
            break;

         case client_type::outcome_server_locked:
            out << my_strings[strid_outcome_server_locked];
            break;
            
         case client_type::outcome_invalid_logon:
            TasksBase::format_failure(out, tasksbase_failure_logon);
            break;
            
         case client_type::outcome_session_failure:
            TasksBase::format_failure(out, tasksbase_failure_session);
            break;
            
         case client_type::outcome_unsupported:
            TasksBase::format_failure(out, tasksbase_failure_unsupported);
            break;
            
         case client_type::outcome_permission_denied:
            TasksBase::format_failure(out, tasksbase_failure_security);
            break;
            
         case client_type::outcome_invalid_task_id:
            out << my_strings[strid_outcome_invalid_task_id];
            break;
            
         default:
            TasksBase::format_failure(out, tasksbase_failure_unknown);
            break;
         }
      } // format_outcome


      void TaskRemover::on_tasks_ready()
      {
         Csi::Messaging::Message command(tasks_session, Messages::remove_task_cmd);
         command.addUInt4(++last_tran_no);
         command.addUInt4(task_id);
         state = state_active;
         router->sendMessage(&command);
      } // on_tasks_ready


      void TaskRemover::on_tasks_failure(tasksbase_failure_type failure)
      {
         client_type::outcome_type outcome(client_type::outcome_unknown_failure);
         switch(failure)
         {
         case tasksbase_failure_logon:
            outcome = client_type::outcome_invalid_logon;
            break;
            
         case tasksbase_failure_session:
            outcome = client_type::outcome_session_failure;
            break;
            
         case tasksbase_failure_unsupported:
            outcome = client_type::outcome_unsupported;
            break;
            
         case tasksbase_failure_security:
            outcome = client_type::outcome_permission_denied;
            break;
         }
         event_complete::cpost(this, outcome);
      }


      void TaskRemover::onNetMessage(
         Csi::Messaging::Router *router, Csi::Messaging::Message *message)
      {
         if(state == state_active)
         {
            if(message->getMsgType() == Messages::remove_task_ack)
            {
               uint4 tran_no;
               uint4 rcd;
               client_type::outcome_type outcome(client_type::outcome_unknown_failure);
               
               message->readUInt4(tran_no);
               message->readUInt4(rcd);
               switch(rcd)
               {
               case 1:
                  outcome = client_type::outcome_success;
                  break;

               case 3:
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



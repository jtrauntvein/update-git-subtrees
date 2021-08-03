/* Cora.Tasks.TaskAdder.cpp

   Copyright (C) 2012, Campbell Scientific, Inc

   Written by: Jon Trauntvein 
   Date Begun: Wednesday 16 May 2012
   Last Change: Wednesday 16 May 2012
   Last Commit: $Date: 2012-05-16 13:59:30 -0600 (Wed, 16 May 2012) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Tasks.TaskAdder.h"
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
            typedef TaskAdder::client_type client_type;
            typedef client_type::outcome_type outcome_type;
            outcome_type outcome;
            uint4 task_id;

            static void cpost(TaskAdder *adder, outcome_type outcome, uint4 task_id = 0xFFFFFFFF)
            {
               event_complete *event = new event_complete(adder, outcome, task_id);
               event->post();
            }

         private:
            event_complete(TaskAdder *adder, outcome_type outcome_, uint4 task_id_):
               Event(event_id, adder),
               outcome(outcome_),
               task_id(task_id_)
            { }
         };

         uint4 const event_complete::event_id(
            Csi::Event::registerType("Cora::Tasks::TaskAdder::event_complete"));
      };

      
      void TaskAdder::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         if(ev->getType() == event_complete::event_id)
         {
            event_complete *event = static_cast<event_complete *>(ev.get_rep());
            client_type *report = client;
            finish();
            if(client_type::is_valid_instance(report))
               report->on_complete(this, event->outcome, event->task_id);
         }
      } // receive


      void TaskAdder::format_outcome(std::ostream &out, client_type::outcome_type outcome)
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
            
         default:
            TasksBase::format_failure(out, tasksbase_failure_unknown);
            break;
         }
      } // format_outcome

      
      void TaskAdder::on_tasks_ready()
      {
         Csi::Messaging::Message command(tasks_session, Messages::add_task_cmd);
         command.addUInt4(++last_tran_no);
         command.addWStr(task_name);
         command.addWStr(station_name);
         state = state_active;
         router->sendMessage(&command);
      } // on_tasks_ready


      void TaskAdder::on_tasks_failure(tasksbase_failure_type failure)
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
      } // on_tasks_failure


      void TaskAdder::onNetMessage(Csi::Messaging::Router *router, Csi::Messaging::Message *message)
      {
         if(state == state_active)
         {
            if(message->getMsgType() == Messages::add_task_ack)
            {
               uint4 tran_no;
               uint4 rcd;
               message->readUInt4(tran_no);
               message->readUInt4(rcd);
               if(rcd == 1)
               {
                  uint4 task_id;
                  message->readUInt4(task_id);
                  event_complete::cpost(this, client_type::outcome_success, task_id);
               }
               else
               {
                  client_type::outcome_type outcome = client_type::outcome_unknown_failure;
                  if(rcd == 2)
                     outcome = client_type::outcome_server_locked;
                  event_complete::cpost(this, outcome);
               }
            }
            else
               TasksBase::onNetMessage(router, message);
         }
         else
            TasksBase::onNetMessage(router, message);
      } // onNetMessage
   };
};



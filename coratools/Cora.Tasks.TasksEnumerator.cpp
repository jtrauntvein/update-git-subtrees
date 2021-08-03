/* Cora.Tasks.TasksEnumerator.cpp

   Copyright (C) 2012, 2015 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Tuesday 15 May 2012
   Last Change: Saturday 22 August 2015
   Last Commit: $Date: 2015-08-27 13:19:03 -0600 (Thu, 27 Aug 2015) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Tasks.TasksEnumerator.h"


namespace Cora
{
   namespace Tasks
   {
      namespace
      {
         class started_event: public Csi::Event
         {
         public:
            static uint4 const event_id;

            typedef TasksEnumerator lister_type;
            static void cpost(lister_type *lister)
            {
               started_event *event(new started_event(lister));
               event->post();
            }

         private:
            started_event(lister_type *lister):
               Event(event_id, lister)
            { }
         };


         uint4 const started_event::event_id(
            Csi::Event::registerType("Cora::Tasks::TasksEnumerator::event_started"));


         class failure_event: public Csi::Event
         {
         public:
            static uint4 const event_id;
            typedef TasksEnumerator lister_type;
            typedef lister_type::client_type::failure_type failure_type;
            failure_type failure;

            static void cpost(lister_type *lister, failure_type failure)
            {
               failure_event *event(new failure_event(lister, failure));
               event->post();
            }

         private:
            failure_event(lister_type *lister, failure_type failure_):
               Event(event_id, lister),
               failure(failure_)
            { }
         };


         uint4 const failure_event::event_id(
            Csi::Event::registerType("Cora::Tasks::TasksEnumerator::failure_event"));


         class task_event: public Csi::Event
         {
         public:
            static uint4 const event_id;
            typedef TasksEnumerator lister_type;
            typedef lister_type::client_type client_type;
            typedef client_type::task_event_type task_event_type;
            task_event_type event;
            uint4 task_id;
            StrUni task_name;
            Triggers::trigger_type trigger;
            StrUni station_name;
            uint4 parent_id;

            static void cpost(
               lister_type *lister,
               task_event_type event_code,
               uint4 task_id,
               StrUni const &task_name,
               Triggers::trigger_type trigger,
               StrUni const station_name,
               uint4 parent_id)
            {
               task_event *event(
                  new task_event(
                     lister, event_code, task_id, task_name, trigger, station_name, parent_id));
               event->post();
            }

         private:
            task_event(
               lister_type *lister,
               task_event_type event_,
               uint4 task_id_,
               StrUni const &task_name_,
               Triggers::trigger_type trigger_,
               StrUni const &station_name_,
               uint4 parent_id_):
               Event(event_id, lister),
               event(event_),
               task_id(task_id_),
               task_name(task_name_),
               trigger(trigger_),
               station_name(station_name_),
               parent_id(parent_id_)
            { }
         };


         uint4 const task_event::event_id(
            Csi::Event::registerType("Cora::Tasks::TaskEnumerator::task_event"));
      };


      void TasksEnumerator::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         if(ev->getType() == started_event::event_id)
         {
            if(client_type::is_valid_instance(client))
               client->on_started(this);
            else
               finish();
         }
         else if(ev->getType() == failure_event::event_id)
         {
            failure_event *event(static_cast<failure_event *>(ev.get_rep()));
            client_type *report(client);
            finish();
            if(client_type::is_valid_instance(report))
               report->on_failure(this, event->failure);
         }
         else if(ev->getType() == task_event::event_id)
         {
            task_event *event(static_cast<task_event *>(ev.get_rep()));
            if(client_type::is_valid_instance(client))
            {
               client->on_task_event(
                  this,
                  event->event,
                  event->task_id,
                  event->task_name,
                  event->trigger,
                  event->station_name,
                  event->parent_id);
            }
            else
               finish();
         }
      } // receive


      void TasksEnumerator::describe_failure(
         std::ostream &out, client_type::failure_type failure)
      {
         switch(failure)
         {
         case client_type::failure_connection_failed:
            format_failure(out, tasksbase_failure_session);
            break;

         case client_type::failure_invalid_logon:
            format_failure(out, tasksbase_failure_logon);
            break;

         case client_type::failure_server_security_blocked:
            format_failure(out, tasksbase_failure_security);
            break;

         case client_type::failure_unsupported:
            format_failure(out, tasksbase_failure_unsupported);
            break;
            
         default:
            format_failure(out, tasksbase_failure_unknown);
            break;
         }
      }
      

      void TasksEnumerator::on_tasks_ready()
      {
         Csi::Messaging::Message command(tasks_session, Messages::tasks_enum_start_cmd);
         command.addUInt4(++last_tran_no);
         state = state_initial;
         router->sendMessage(&command);
      } // on_tasks_ready


      void TasksEnumerator::on_tasks_failure(tasksbase_failure_type failure_)
      {
         client_type::failure_type failure(client_type::failure_unknown);
         switch(failure_)
         {
         case tasksbase_failure_logon:
            failure = client_type::failure_invalid_logon;
            break;

         case tasksbase_failure_session:
            failure = client_type::failure_connection_failed;
            break;

         case tasksbase_failure_unsupported:
            failure = client_type::failure_unsupported;
            break;

         case tasksbase_failure_security:
            failure = client_type::failure_server_security_blocked;
            break;
         }
         failure_event::cpost(this, failure);
      } // on_tasks_failure


      void TasksEnumerator::onNetMessage(
         Csi::Messaging::Router *router, Csi::Messaging::Message *message)
      {
         if(state == state_started || state == state_initial)
         {
            switch(message->getMsgType())
            {
            case Messages::tasks_enum_not:
               on_notification(message);
               break;

            case Messages::tasks_enum_stopped_not:
               on_stopped_notification(message);
               break;
               
            default:
               TasksBase::onNetMessage(router, message);
               break;
            }
         }
         else
            TasksBase::onNetMessage(router, message);
      } // onNetMessage


      void TasksEnumerator::on_notification(Csi::Messaging::Message *message)
      {
         uint4 tran_no;
         uint4 count;
         if(message->readUInt4(tran_no) && message->readUInt4(count))
         {
            uint4 task_id;
            StrUni task_name;
            uint4 event;
            uint4 trigger;
            StrUni station_name;
            uint4 parent_id;
            for(uint4 i = 0; i < count; ++i)
            {
               message->readUInt4(event);
               message->readUInt4(task_id);
               message->readWStr(task_name);
               message->readUInt4(trigger);
               message->readWStr(station_name);
               message->readUInt4(parent_id);
               task_event::cpost(
                  this,
                  static_cast<client_type::task_event_type>(event),
                  task_id,
                  task_name,
                  static_cast<Triggers::trigger_type>(trigger),
                  station_name,
                  parent_id);
            }
            if(message->whatsLeft() >= 1)
               message->readBool(tasks_enabled);
            else
               tasks_enabled = true;
            if(state == state_initial)
            {
               started_event::cpost(this);
               state = state_started;
            }
         }
         else
            failure_event::cpost(this, client_type::failure_unknown);
      } // on_notification


      void TasksEnumerator::on_stopped_notification(Csi::Messaging::Message *message)
      {
         failure_event::cpost(this, client_type::failure_connection_failed);
      } // on_stopped_notification
   };
};



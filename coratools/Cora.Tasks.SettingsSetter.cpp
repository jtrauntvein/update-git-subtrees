/* Cora.Tasks.SettingsSetter.cpp

   Copyright (C) 2012, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Tuesday 29 May 2012
   Last Change: Wednesday 30 May 2012
   Last Commit: $Date: 2016-05-03 16:18:03 -0600 (Tue, 03 May 2016) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Tasks.SettingsSetter.h"
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
            typedef SettingsSetterClient client_type;
            typedef client_type::outcome_type outcome_type;
            outcome_type outcome;

            
            ////////////////////////////////////////////////////////////
            // reports
            ////////////////////////////////////////////////////////////
            typedef client_type::settings_report_type settings_report_type;
            settings_report_type reports;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               SettingsSetter *setter,
               outcome_type outcome,
               settings_report_type const &report = settings_report_type())
            {
               event_complete *event(new event_complete(setter, outcome, report));
               event->post();
            }

            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_complete(
               SettingsSetter *setter,
               outcome_type outcome_,
               settings_report_type const &reports_):
               Event(event_id, setter),
               outcome(outcome_),
               reports(reports_)
            { }
         };


         uint4 const event_complete::event_id(
            Csi::Event::registerType("Cora::Tasks::SettingsSetter::event_complete"));
      };


      ////////////////////////////////////////////////////////////
      // class SettingsSetter definitions
      ////////////////////////////////////////////////////////////
      void SettingsSetter::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         if(ev->getType() == event_complete::event_id)
         {
            client_type *report = client;
            event_complete *event(static_cast<event_complete *>(ev.get_rep()));
            finish();
            if(client_type::is_valid_instance(report))
               report->on_complete(this, event->outcome, event->reports);
         }
      } // receive

      
      void SettingsSetter::format_outcome(std::ostream &out, client_type::outcome_type outcome)
      {
         using namespace TaskAdderStrings;
         switch(outcome)
         {
         case client_type::outcome_task_found:
            out << my_strings[strid_outcome_task_found];
            break;

         case client_type::outcome_failure_invalid_task_id:
            out << my_strings[strid_outcome_invalid_task_id];
            break;

         case client_type::outcome_failure_network_locked:
            out << my_strings[strid_outcome_server_locked];
            break;

         case client_type::outcome_failure_invalid_logon:
            TasksBase::format_failure(out, tasksbase_failure_logon);
            break;

         case client_type::outcome_failure_session:
            TasksBase::format_failure(out, tasksbase_failure_session);
            break;

         case client_type::outcome_failure_unsupported:
            TasksBase::format_failure(out, tasksbase_failure_unsupported);
            break;

         case client_type::outcome_failure_security:
            TasksBase::format_failure(out, tasksbase_failure_security);
            break;

         default:
            TasksBase::format_failure(out, tasksbase_failure_unknown);
            break;
         }
      } // format_outcome

      
      void SettingsSetter::format_setting_outcome(
         std::ostream &out, client_type::setting_outcome_type outcome)
      {
         using namespace TaskAdderStrings;
         switch(outcome)
         {
         case client_type::outcome_setting_set:
            out << my_strings[strid_outcome_setting_set];
            break;
            
         case client_type::outcome_invalid_setting_id:
            out << my_strings[strid_outcome_invalid_setting_id];
            break;
            
         case client_type::outcome_invalid_value:
            out << my_strings[strid_outcome_invalid_setting_value];
            break;
            
         case client_type::outcome_setting_read_only:
            out << my_strings[strid_outcome_setting_read_only];
            break;
            
         default:
            TasksBase::format_failure(out, tasksbase_failure_unknown);
            break;
         }
      } // format_setting_outcome

      
      void SettingsSetter::on_tasks_ready()
      {
         Csi::Messaging::Message command(tasks_session, Messages::set_settings_cmd);
         command.addUInt4(++last_tran_no);
         command.addUInt4(task_id);
         command.addUInt4((uint4)settings.size());
         for(settings_type::iterator si = settings.begin(); si != settings.end(); ++si)
         {
            uint4 setting_len_pos;
            uint4 setting_pos;
            setting_handle &setting(*si);
            command.addUInt4(setting->get_identifier());
            setting_len_pos = command.getBodyLen();
            command.addUInt4(0);
            setting_pos = command.getBodyLen();
            setting->write(&command);
            command.replaceUInt4(command.getBodyLen() - setting_pos, setting_len_pos);
         }
         state = state_active;
         router->sendMessage(&command);
      } // on_tasks_ready

      
      void SettingsSetter::on_tasks_failure(tasksbase_failure_type failure)
      {
         client_type::outcome_type outcome(client_type::outcome_failure_unknown);
         switch(failure)
         {
         case tasksbase_failure_logon:
            outcome = client_type::outcome_failure_invalid_logon;
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

      
      void SettingsSetter::onNetMessage(
         Csi::Messaging::Router *router, Csi::Messaging::Message *message)
      {
         if(state == state_active)
         {
            if(message->getMsgType() == Messages::set_settings_ack)
            {
               uint4 tran_no;
               uint4 response;
               client_type::outcome_type outcome(client_type::outcome_failure_unknown);
               client_type::settings_report_type reports;
               
               message->readUInt4(tran_no);
               message->readUInt4(response);
               if(response == 1)
               {
                  uint4 count;
                  message->readUInt4(count);
                  outcome = client_type::outcome_task_found;
                  for(uint4 i = 0; i < count; ++i)
                  {
                     uint4 setting_id;
                     uint4 setting_outcome;
                     message->readUInt4(setting_id);
                     message->readUInt4(setting_outcome);
                     if(setting_outcome >= client_type::outcome_setting_read_only)
                        setting_outcome = -1;
                     reports.push_back(
                        client_type::setting_report_type(
                           setting_id, static_cast<client_type::setting_outcome_type>(setting_outcome)));
                  }
               }
               else
               {
                  switch(response)
                  {
                  case 2:
                     outcome = client_type::outcome_failure_invalid_task_id;
                     break;

                  case 3:
                     outcome = client_type::outcome_failure_network_locked;
                     break;
                  }
               }
               event_complete::cpost(this, outcome, reports);
            }
            else
               TasksBase::onNetMessage(router, message);
         }
         else
            TasksBase::onNetMessage(router, message);
      } // onNetMessage
   };
};


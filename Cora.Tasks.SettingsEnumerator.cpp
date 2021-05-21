/* Cora.Tasks.SettingsEnumerator.cpp

   Copyright (C) 2012, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: sexta-feira 18 maio 2012
   Last Change: Tuesday 19 June 2012
   Last Commit: $Date: 2012-06-19 09:51:11 -0600 (Tue, 19 Jun 2012) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Tasks.SettingsEnumerator.h"
#include "coratools.strings.h"


namespace Cora
{
   namespace Tasks
   {
      namespace
      {
         ////////////////////////////////////////////////////////////
         // class event_started
         ////////////////////////////////////////////////////////////
         class event_started: public Csi::Event
         {
         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_started(SettingsEnumerator *lister):
               Event(event_id, lister)
            { }
            
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(SettingsEnumerator *lister)
            {
               event_started *event(new event_started(lister));
               event->post();
            }
         };


         uint4 const event_started::event_id(
            Csi::Event::registerType("Cora::Tasks::SettingsEnumerator::event_started"));


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
            typedef SettingsEnumerator::client_type::failure_type failure_type;
            failure_type failure;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(SettingsEnumerator *lister, failure_type failure)
            {
               event_failure *event(new event_failure(lister, failure));
               event->post();
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_failure(SettingsEnumerator *lister, failure_type failure_):
               Event(event_id, lister),
               failure(failure_)
            { }
         };


         uint4 const event_failure::event_id(
            Csi::Event::registerType("Cora::Tasks::SettingsEnumerator::event_failure"));


         ////////////////////////////////////////////////////////////
         // class event_setting
         ////////////////////////////////////////////////////////////
         class event_setting: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // setting
            ////////////////////////////////////////////////////////////
            typedef SettingsEnumeratorClient::setting_handle setting_handle;
            setting_handle setting;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(SettingsEnumerator *lister, setting_handle &setting)
            {
               event_setting *event(new event_setting(lister, setting));
               event->post();
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_setting(SettingsEnumerator *lister, setting_handle &setting_):
               Event(event_id, lister),
               setting(setting_)
            { }
         };


         uint4 const event_setting::event_id(
            Csi::Event::registerType("Cora::Tasks::SettingsEnumerator::event_setting"));
      };


      ////////////////////////////////////////////////////////////
      // class SettingsEnumerator definitions
      ////////////////////////////////////////////////////////////
      void SettingsEnumerator::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         if(ev->getType() == event_started::event_id)
         {
            if(client_type::is_valid_instance(client))
               client->on_started(this);
            else
               finish();
         }
         else if(ev->getType() == event_failure::event_id)
         {
            client_type *report(client);
            event_failure *event(static_cast<event_failure *>(ev.get_rep()));
            finish();
            if(client_type::is_valid_instance(report))
               report->on_failure(this, event->failure);
         }
         else if(ev->getType() == event_setting::event_id)
         {
            event_setting *event(static_cast<event_setting *>(ev.get_rep()));
            if(client_type::is_valid_instance(client))
               client->on_setting_changed(this, event->setting);
            else
               finish();
         }
      } // receive


      void SettingsEnumerator::format_failure(std::ostream &out, client_type::failure_type failure)
      {
         using namespace TaskAdderStrings;
         switch(failure)
         {
         case client_type::failure_session:
            TasksBase::format_failure(out, tasksbase_failure_session);
            break;
            
         case client_type::failure_invalid_logon:
            TasksBase::format_failure(out, tasksbase_failure_logon);
            break;
            
         case client_type::failure_permission_denied:
            TasksBase::format_failure(out, tasksbase_failure_security);
            break;
            
         case client_type::failure_invalid_task_id:
            out << my_strings[strid_outcome_invalid_task_id];
            break;
            
         case client_type::failure_unsupported:
            TasksBase::format_failure(out, tasksbase_failure_unsupported);
            break;
            
         default:
            TasksBase::format_failure(out, tasksbase_failure_unknown);
            break;
         }
      } // format_failure


      void SettingsEnumerator::on_tasks_ready()
      {
         Csi::Messaging::Message command(tasks_session, Messages::enum_settings_start_cmd);
         command.addUInt4(++last_tran_no);
         command.addUInt4(task_id);
         state = state_starting;
         router->sendMessage(&command);
      } // on_tasks_ready


      void SettingsEnumerator::on_tasks_failure(tasksbase_failure_type failure_)
      {
         client_type::failure_type failure(client_type::failure_unknown);
         switch(failure_)
         {
         case tasksbase_failure_logon:
            failure = client_type::failure_invalid_logon;
            break;
            
         case tasksbase_failure_session:
            failure = client_type::failure_session;
            break;
            
         case tasksbase_failure_unsupported:
            failure = client_type::failure_unsupported;
            break;
            
         case tasksbase_failure_security:
            failure = client_type::failure_permission_denied;
            break;
         }
         event_failure::cpost(this, failure);
      } // on_tasks_failure


      void SettingsEnumerator::onNetMessage(
         Csi::Messaging::Router *router, Csi::Messaging::Message *message)
      {
         if(state == state_starting || state == state_started)
         {
            switch(message->getMsgType())
            {
            case Messages::enum_settings_not:
               on_settings(message);
               break;
               
            case Messages::enum_settings_stopped_not:
               on_stopped_not(message);
               break;
               
            default:
               TasksBase::onNetMessage(router, message);
               break;
            }
         }
         else
            TasksBase::onNetMessage(router, message);
      } // onNetMessage


      void SettingsEnumerator::on_settings(Csi::Messaging::Message *message)
      {
         uint4 tran_no;
         uint4 count;
         message->readUInt4(tran_no);
         message->readUInt4(count);
         for(uint4 i = 0; i < count; ++i)
         {
            uint4 setting_id;
            uint4 setting_len;
            if(message->readUInt4(setting_id) && message->readUInt4(setting_len))
            {
               client_type::setting_handle setting(factory->make_setting(setting_id));
               if(setting != 0)
               {
                  setting->read(message);
                  event_setting::cpost(this, setting);
               }
               else
                  message->movePast(setting_len);
            }
            else
            {
               event_failure::cpost(this, client_type::failure_unknown);
               return;
            }
         }
         if(state == state_starting)
         {
            state = state_started;
            event_started::cpost(this);
         }
      } // on_settings


      void SettingsEnumerator::on_stopped_not(Csi::Messaging::Message *message)
      {
         uint4 tran_no;
         uint4 reason;
         client_type::failure_type failure(client_type::failure_unknown);
         message->readUInt4(tran_no);
         message->readUInt4(reason);
         switch(reason)
         {
         case 3:
            failure = client_type::failure_invalid_task_id;
            break;
         }
         event_failure::cpost(this, failure);
      } // on_stopped_not
   };
};

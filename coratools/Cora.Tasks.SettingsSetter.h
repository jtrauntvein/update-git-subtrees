/* Cora.Tasks.SettingsSetter.h

   Copyright (C) 2012, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Tuesday 29 May 2012
   Last Change: Tuesday 19 June 2012
   Last Commit: $Date: 2012-06-19 09:51:11 -0600 (Tue, 19 Jun 2012) $
   Last Changed by: $Author: jon $

*/

#ifndef Cora_Tasks_SettingsSetter_h
#define Cora_Tasks_SettingsSetter_h

#include "Cora.Tasks.TasksBase.h"
#include "Cora.Setting.h"
#include "Csi.Events.h"
#include <list>


namespace Cora
{
   namespace Tasks
   {
      ////////////////////////////////////////////////////////////
      // class SettingsSetterClient
      //
      // Defines the interface that an application object is expected to
      // implement in order to use the SettingsSetter component. 
      ////////////////////////////////////////////////////////////
      class SettingsSetter;
      class SettingsSetterClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_complete
         //
         // Called when the server transaction has been completed.
         ////////////////////////////////////////////////////////////
         enum outcome_type
         {
            outcome_failure_unknown = 0,
            outcome_task_found,
            outcome_failure_invalid_task_id,
            outcome_failure_network_locked,
            outcome_failure_invalid_logon,
            outcome_failure_session,
            outcome_failure_unsupported,
            outcome_failure_security
         };
         enum setting_outcome_type
         {
            outcome_setting_failure_unknown = -1,
            outcome_setting_set = 0,
            outcome_invalid_setting_id,
            outcome_invalid_value,
            outcome_setting_read_only
         };
         typedef std::pair<uint4, setting_outcome_type> setting_report_type;
         typedef std::list<setting_report_type> settings_report_type;
         virtual void on_complete(
            SettingsSetter *setter,
            outcome_type outcome,
            settings_report_type const &reports) = 0;
      };


      ////////////////////////////////////////////////////////////
      // class SettingsSetter
      //
      // Defines a component that wraps the CsiLgrNet Tasks Set Task Settings
      // transaction.  In order to use this component, the application must
      // provide an object that implements the SettingsSetterClient interface.
      // It should then create an instance of this class, set its properties
      // including the task ID and the settings to be transmitted, and call one
      // of the two variants of the start() method.  When the server
      // transaction is complete, the client's on_complete() method will be
      // called. 
      ////////////////////////////////////////////////////////////
      class SettingsSetter: public TasksBase, public Csi::EventReceiver
      {
      private:
         ////////////////////////////////////////////////////////////
         // client
         ////////////////////////////////////////////////////////////
         SettingsSetterClient *client;

         ////////////////////////////////////////////////////////////
         // task_id
         ////////////////////////////////////////////////////////////
         uint4 task_id;

         ////////////////////////////////////////////////////////////
         // state
         ////////////////////////////////////////////////////////////
         enum state_type
         {
            state_standby,
            state_delegate,
            state_active
         } state;

         ////////////////////////////////////////////////////////////
         // settings
         ////////////////////////////////////////////////////////////
      public:
         typedef Csi::SharedPtr<Setting> setting_handle;
         typedef std::list<setting_handle> settings_type;
      private:
         settings_type settings;

      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         SettingsSetter():
            client(0),
            state(state_standby)
         { }

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~SettingsSetter()
         { finish(); }

         ////////////////////////////////////////////////////////////
         // get_task_id
         ////////////////////////////////////////////////////////////
         uint4 get_task_id() const
         { return task_id; }

         ////////////////////////////////////////////////////////////
         // set_task_id
         ////////////////////////////////////////////////////////////
         void set_task_id(uint4 id)
         {
            if(state == state_standby)
               task_id = id;
            else
               throw exc_invalid_state();
         }

         // @group: methods to act as a container of settings

         ////////////////////////////////////////////////////////////
         // begin
         ////////////////////////////////////////////////////////////
         typedef settings_type::iterator iterator;
         typedef settings_type::const_iterator const_iterator;
         iterator begin()
         { return settings.begin(); }
         const_iterator begin() const
         { return settings.begin(); }

         ////////////////////////////////////////////////////////////
         // end
         ////////////////////////////////////////////////////////////
         iterator end()
         { return settings.end(); }
         const_iterator end() const
         { return settings.end(); }

         ////////////////////////////////////////////////////////////
         // size
         ////////////////////////////////////////////////////////////
         typedef settings_type::size_type size_type;
         size_type size() const
         { return settings.size(); }

         ////////////////////////////////////////////////////////////
         // empty
         ////////////////////////////////////////////////////////////
         bool empty() const
         { return settings.empty(); }

         ////////////////////////////////////////////////////////////
         // push_back
         ////////////////////////////////////////////////////////////
         typedef setting_handle value_type;
         void push_back(value_type setting)
         {
            if(state == state_standby)
               settings.push_back(setting);
            else
               throw exc_invalid_state();
         }

         ////////////////////////////////////////////////////////////
         // push_front
         ////////////////////////////////////////////////////////////
         void push_front(value_type setting)
         {
            if(state == state_standby)
               settings.push_front(setting);
            else
               throw exc_invalid_state();
         }

         ////////////////////////////////////////////////////////////
         // front
         ////////////////////////////////////////////////////////////
         value_type const &front() const
         { return settings.front(); }
         value_type &front()
         { return settings.front(); }

         ////////////////////////////////////////////////////////////
         // back
         ////////////////////////////////////////////////////////////
         value_type const &back() const
         { return settings.back(); }
         value_type &back()
         { return settings.back(); }
         
         // @endgroup

         ////////////////////////////////////////////////////////////
         // start (new connection)
         ////////////////////////////////////////////////////////////
         typedef SettingsSetterClient client_type;
         void start(client_type *client_, router_handle router)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("invalid client pointer");
            client = client_;
            state = state_delegate;
            TasksBase::start(router);
         }

         ////////////////////////////////////////////////////////////
         // start (shared connection)
         ////////////////////////////////////////////////////////////
         void start(client_type *client_, ClientBase *other_client)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("invalid client pointer");
            client = client_;
            state = state_delegate;
            TasksBase::start(other_client);
         } // start

         ////////////////////////////////////////////////////////////
         // finish
         ////////////////////////////////////////////////////////////
         virtual void finish()
         {
            client = 0;
            state = state_standby;
            TasksBase::finish();
         }

         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

         ////////////////////////////////////////////////////////////
         // format_outcome
         ////////////////////////////////////////////////////////////
         static void format_outcome(std::ostream &out, client_type::outcome_type outcome);

         ////////////////////////////////////////////////////////////
         // format_setting_outcome
         ////////////////////////////////////////////////////////////
         static void format_setting_outcome(
            std::ostream &out, client_type::setting_outcome_type outcome);
         
      protected:
         ////////////////////////////////////////////////////////////
         // on_tasks_ready
         ////////////////////////////////////////////////////////////
         virtual void on_tasks_ready();

         ////////////////////////////////////////////////////////////
         // on_tasks_failure
         ////////////////////////////////////////////////////////////
         virtual void on_tasks_failure(tasksbase_failure_type failure);

         ////////////////////////////////////////////////////////////
         // onNetMessage
         ////////////////////////////////////////////////////////////
         virtual void onNetMessage(
            Csi::Messaging::Router *router, Csi::Messaging::Message *message);
      };
   };
};



#endif


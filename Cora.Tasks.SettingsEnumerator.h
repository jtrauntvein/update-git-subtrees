/* Cora.Tasks.SettingsEnumerator.h

   Copyright (C) 2012, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: sexta-feira 18 maio 2012
   Last Change: Tuesday 19 June 2012
   Last Commit: $Date: 2012-06-19 09:51:11 -0600 (Tue, 19 Jun 2012) $
   Last Changed by: $Author: jon $

*/

#ifndef Cora_Tasks_SettingsEnumerator_h
#define Cora_Tasks_SettingsEnumerator_h

#include "Cora.Tasks.TasksBase.h"
#include "Cora.Tasks.TasksSettingFactory.h"
#include "Csi.Events.h"


namespace Cora
{
   namespace Tasks
   {
      /**
       * Defines the application interface for the settings enumerator component.
       */
      class SettingsEnumerator;
      class SettingsEnumeratorClient: public Csi::InstanceValidator
      {
      public:
         /**
          * Called when the server transaction has been started and the first
          * set of message notifications have been received.
          *
          * @param lister  The enumerator component that called this method.
          */
         virtual void on_started(SettingsEnumerator *lister)
         { }

         /**
          * Called when a failure has occurred that will prevent the enumerator
          * from continuing its task.
          *
          * @param lister The enumerator component that called this method
          * @param failure  An explanation of the failure.
          */
         enum failure_type
         {
            failure_unknown = 0,
            failure_session = 1,
            failure_invalid_logon = 2,
            failure_permission_denied = 3,
            failure_invalid_task_id = 4,
            failure_unsupported = 5
         };
         virtual void on_failure(SettingsEnumerator *lister, failure_type failure) = 0;

         /**
          * Called one or more times when the transaction is first started to
          * notify the application of initial setting values.  Thereafter, this
          * method will get called when setting values get changed.
          */
         typedef Csi::SharedPtr<Setting> setting_handle;
         virtual void on_setting_changed(SettingsEnumerator *lister, setting_handle &setting)
         { }
      };


      /**
       * Defines an object that wraps the CsiLgrNet Tasks Task Settings
       * Enumerate transaction.  In order to use this component, the
       * application must provide an object that extends class
       * SettingsEnumeratorClient.  It must then create an instance of this
       * class, set the identifier for the task to monitor (set_task_id()), and
       * call one of the two variants of start().  When the server transaction
       * begins, the component will call the client's on_setting_changed()
       * methods one or more times to report the initial values of the task
       * settings.  It will then call the client's on_started() method.  If the
       * comnponent encounters an error at any time, the client will be
       * notified via its on_failure() method.
       */
      class SettingsEnumerator: public TasksBase, public Csi::EventReceiver
      {
      private:
         ////////////////////////////////////////////////////////////
         // client
         ////////////////////////////////////////////////////////////
         SettingsEnumeratorClient *client;

         ////////////////////////////////////////////////////////////
         // state
         ////////////////////////////////////////////////////////////
         enum state_type
         {
            state_standby,
            state_delegate,
            state_starting,
            state_started
         } state;

         ////////////////////////////////////////////////////////////
         // task_id
         ////////////////////////////////////////////////////////////
         uint4 task_id;

         ////////////////////////////////////////////////////////////
         // factory
         ////////////////////////////////////////////////////////////
         Csi::SharedPtr<SettingFactory> factory;
         
      public:
         ////////////////////////////////////////////////////////////
         // default constructor
         ////////////////////////////////////////////////////////////
         SettingsEnumerator():
            client(0),
            state(state_standby),
            task_id(0),
            factory(new TasksSettingFactory)
         { }

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~SettingsEnumerator()
         { finish(); }
         
         /**
          * @return the current value of the task_id property
          */
         uint4 get_task_id() const
         { return task_id; }

         /**
          * Sets the task_id property.
          *
          * @param id  The new value of the task_id property.
          */
         void set_task_id(uint4 id)
         {
            if(state == state_standby)
               task_id = id;
            else
               throw exc_invalid_state();
         }

         /**
          * @return the settings factory
          */
         Csi::SharedPtr<SettingFactory> &get_factory()
         { return factory; }

         /**
          * Set the setting factory
          * 
          * @param factory_
          */
         void set_factory(Csi::SharedPtr<SettingFactory> factory_)
         {
            if(state == state_standby)
               factory = factory_;
            else
               throw exc_invalid_state();
         }
         
         /**
          * Start the server transaction using a newly created connection.
          *
          * @param client_  the application object that will receive
          * notifications from this component.
          * @param router  a newly created connection to the server.
          */
         typedef SettingsEnumeratorClient client_type;
         void start(client_type *client_, router_handle &router)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("invalid client pointer");
            client = client_;
            state = state_delegate;
            TasksBase::start(router);
         }

         /**
          * Start the server transaction using a connection borrowed from
          * another component
          *
          * @param client_  the application object that will receive
          * notifications from this component.
          * @param other_client  The component from which we will borrow the
          * connection. 
          */
         void start(client_type *client_, ClientBase *other_client)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("invalid client pointer");
            client = client_;
            state = state_delegate;
            TasksBase::start(other_client);
         }

         /**
          * Clears the tasks session and closes the enumeration transaction.
          */
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

         /**
          * Formats a failure code to the supplied stream
          */
         static void format_failure(std::ostream &out, client_type::failure_type failure);

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

      private:
         ////////////////////////////////////////////////////////////
         // on_settings
         ////////////////////////////////////////////////////////////
         void on_settings(Csi::Messaging::Message *message);

         ////////////////////////////////////////////////////////////
         // on_stopped_not
         ////////////////////////////////////////////////////////////
         void on_stopped_not(Csi::Messaging::Message *message);
      };
   };
};


#endif


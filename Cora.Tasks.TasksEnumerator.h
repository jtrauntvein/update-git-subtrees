/* Cora.Tasks.TasksEnumerator.h

   Copyright (C) 2012, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Tuesday 15 May 2012
   Last Change: Wednesday 13 June 2012
   Last Commit: $Date: 2015-08-27 13:19:03 -0600 (Thu, 27 Aug 2015) $
   Last Changed by: $Author: jon $

*/

#ifndef Cora_Tasks_TasksEnumerator_h
#define Cora_Tasks_TasksEnumerator_h

#include "Cora.Tasks.TasksBase.h"
#include "Csi.Events.h"


namespace Cora
{
   namespace Tasks
   {
      /**
       * Defines the interface that the application will be expected to provide
       * for the TasksEnumerator component.
       */
      class TasksEnumerator;
      class TasksEnumeratorClient: public Csi::InstanceValidator
      {
      public:
         /**
          * Called when the tasks enumeration transaction has been started and
          * all initial tasks have been reported.
          *
          * @param lister The component that is perfoming the transaction.
          */
         virtual void on_started(TasksEnumerator *lister)
         { }

         /**
          * Called when the enumerator has encountered a failure that prevents
          * it from monitoring tasks.
          *
          * @param lister  The component reporting the failure
          * @param failure  A code that represents the nature of the failure.
          */
         enum failure_type
         {
            failure_unknown = 0,
            failure_connection_failed = 1,
            failure_invalid_logon = 2,
            failure_server_security_blocked = 3,
            failure_unsupported = 4
         };
         virtual void on_failure(
            TasksEnumerator *lister,
            failure_type failure) = 0;

         /**
          * Called when an event affecting the lifetime or identification of a
          * task has been reported by the server.
          *
          * @param lister  The component calling this method
          * @param event   The type of event being reported
          * @param task_id  The identifier for the task
          * @param task_name  The name of the task
          * @param trigger  The trigger type for this task
          * @param station_name  The station name for this task
          * @param parent_id  The task that this task is to follow
          */
         enum task_event_type
         {
            task_added = 1,
            task_setting_changed = 2,
            task_removed = 3
         };
         virtual void on_task_event(
            TasksEnumerator *lister,
            task_event_type event,
            uint4 task_id,
            StrUni const &task_name,
            Triggers::trigger_type trigger,
            StrUni const &station_name,
            uint4 parent_id) = 0;
      };

      
      /**
       * Defines a component that enumerate the list of tasks in a LoggerNet
       * server.  In order to use this component, the application must provide
       * a client object that extends class TasksEnumeratorClient.  It must
       * then create an instance of the class and call one of the two variants
       * of the start() method.  Once the component has obtained a connection
       * to the tasks interface, it will start the tasks enumerate transaction.
       * It will call the client's on_tasks_changed() method for each task that
       * is reported by the server.  It will then call the client's
       * on_started() method.  If, at any time, a failure with the server is
       * observed, the client's on_failure() method will get called.
       */
      class TasksEnumerator: public TasksBase, public Csi::EventReceiver
      {
      private:
         /**
          * Specifies the client that will receive notifications from this
          * component
          */
         TasksEnumeratorClient *client;

         /**
          * Specifies the state of this component
          */
         enum state_type
         {
            state_standby,
            state_delegate,
            state_initial,
            state_started
         } state;

         ////////////////////////////////////////////////////////////
         // tasks_enabled
         ////////////////////////////////////////////////////////////
         bool tasks_enabled;

      public:
         /**
          * Default constructor
          */
         TasksEnumerator():
            client(0),
            state(state_standby),
            tasks_enabled(false)
         { }

         /**
          * destructor
          */
         virtual ~TasksEnumerator()
         { finish(); }

         /**
          * Starts the connection to the tasks interface and, eventually, the
          * tasks enumerate transaction.
          *
          * @param client_  The application object that will receive
          * notifications
          * @param router  A newly created router object.
          */
         typedef TasksEnumeratorClient client_type;
         void start(client_type *client_, router_handle &router)
         {
            if(state == state_standby)
            {
               if(client_type::is_valid_instance(client_))
               {
                  client = client_;
                  state = state_delegate;
                  TasksBase::start(router);
               }
               else
                  throw std::invalid_argument("invalid client pointer");
            }
            else
               throw exc_invalid_state();
         }
            

         /**
          * Starts this component using the connection from another component.
          *
          * @param client_ The application object that will receive
          * notifications.
          * @param other_client  The object with which we will share the
          * connection.
          */
         void start(client_type *client_, ClientBase *other_client)
         {
            if(state == state_standby)
            {
               if(client_type::is_valid_instance(client_))
               {
                  client = client_;
                  state = state_delegate;
                  TasksBase::start(other_client);
               }
               else
                  throw std::invalid_argument("invalid client pointer");
            }
            else
               throw exc_invalid_state();
         }

         /**
          * resets this object back to a standby state.
          */
         virtual void finish()
         {
            state = state_standby;
            client = 0;
            TasksBase::finish();
         }

         /**
          * Handles queue events.
          *
          * @param ev  The event that was queue.
          */
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

         ////////////////////////////////////////////////////////////
         // get_tasks_enabled
         ////////////////////////////////////////////////////////////
         bool get_tasks_enabled() const
         { return tasks_enabled; }

         /**
          * Formats the specified failure code to the specified output stream.
          *
          * @param out Specifies the output stream to which the failure will be written.
          *
          * @param failure Specifies the failure to describe.
          */
         static void describe_failure(std::ostream &out, client_type::failure_type failure);

      protected:
         /**
          * Called when we have made a connection to the tasks interface
          */
         virtual void on_tasks_ready();

         /**
          * Called when a failure has occurred in the tasks interface,
          *
          * @param failure a code that explains the failure.
          */
         virtual void on_tasks_failure(tasksbase_failure_type failure);

         /**
          * Called to handle incoming messages.
          *
          * @param router  Represents the connection to the server
          * @param message The content of the message
          */
         virtual void onNetMessage(
            Csi::Messaging::Router *router, Csi::Messaging::Message *message);

         /**
          * Called to handle a notification
          */
         void on_notification(Csi::Messaging::Message *message);

         /**
          * Called to handle a stopped notification
          */
         void on_stopped_notification(Csi::Messaging::Message *message);
      };
   };
};


#endif


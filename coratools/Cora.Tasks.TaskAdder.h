/* Cora.Tasks.TaskAdder.h

   Copyright (C) 2012, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Tuesday 15 May 2012
   Last Change: Wednesday 16 May 2012
   Last Commit: $Date: 2012-05-16 13:59:30 -0600 (Wed, 16 May 2012) $
   Last Changed by: $Author: jon $

*/

#ifndef Cora_Tasks_TaskAdder_h
#define Cora_Tasks_TaskAdder_h

#include "Cora.Tasks.TasksBase.h"
#include "Csi.Events.h"


namespace Cora
{
   namespace Tasks
   {
      /**
       * Defines the interface required for the application client object
       */
      class TaskAdder;
      class TaskAdderClient: public Csi::InstanceValidator
      {
      public:
         /**
          * Called when the transaction has been completed.
          *
          * @param adder  The component invoking this method
          * @param outcome  An encoded outcome for this task
          * @param task_id  The identifier that was allocated for the new task.
          */
         enum outcome_type
         {
            outcome_unknown_failure = 0,
            outcome_success = 1,
            outcome_server_locked = 2,
            outcome_invalid_logon = 3,
            outcome_session_failure = 4,
            outcome_unsupported = 5,
            outcome_permission_denied = 6
         };
         virtual void on_complete(TaskAdder *adder, outcome_type outcome, uint4 task_id) = 0;
      };


      /**
       * Defines a component that wraps the CsiLgrNet Tasks Add Task
       * transaction.  In order to use this component, the application must
       * provide an object that extends the TaskAdderClient protocol.  It must
       * then create an instance of this class, call appropriate methods such
       * as set_task_name() and set_station_name(), and invoke one of the two
       * versions of start().  When the server transaction is complete, the
       * component will call the client object's on_complete() method to report
       * the outcome.
       */
      class TaskAdder: public TasksBase, public Csi::EventReceiver
      {
      private:
         /**
          * Specifies the client object.
          */
         TaskAdderClient *client;

         /**
          * Specifies the name for the new task.
          */
         StrUni task_name;

         /**
          * Specifies the name of the station with which this task will be
          * associated.
          */
         StrUni station_name;

         /**
          * Specifies the state of this component
          */
         enum state_type
         {
            state_standby,
            state_delegate,
            state_active
         } state;

      public:
         /**
          * default constructor
          */
         TaskAdder():
            state(state_standby),
            client(0)
         { }

         /**                                    
          * destructor
          */
         virtual ~TaskAdder()
         { finish(); }

         /**
          * @return the station_name property
          */
         StrUni const &get_station_name() const
         { return station_name; }

         /**
          * Sets the value of the station_name property.
          *
          * @param name  The value for the property
          */
         void set_station_name(StrUni const &name)
         {
            if(state == state_standby)
               station_name = name;
            else
               throw exc_invalid_state();
         }

         /**
          * @return The value of the task_name property
          */
         StrUni const &get_task_name() const
         { return task_name; }

         /**
          * Sets the value of the task_name property
          *
          * @param name the value to set
          */
         void set_task_name(StrUni const &name)
         {
            if(state == state_standby)
               task_name = name;
            else
               throw exc_invalid_state();
         }

         /**
          * Starts this component with a newly allocated connection.\
          *
          * @param client_  The application object that will receive the
          * on_complete() call.
          * @param router  Represents the client connection.
          */
         typedef TaskAdderClient client_type;
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
          * Starts this component using a connection borrowed from another
          * component.
          *
          * @param client_  The application object that will receive the call
          * to on_complete().
          * @param other_client The other component
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
          * Clears the state of this component
          */
         virtual void finish()
         {
            client = 0;
            state = state_standby;
            TasksBase::finish();
         }

         /**
          * Handles the completion event.
          */
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

         /**
          * formats the specified outcome to a string.
          *
          * @param out  The stream to which the outcome will be formatted.
          * @param outcome  The outcome to be described
          */
         static void format_outcome(std::ostream &out, client_type::outcome_type outcome);
         
      protected:
         /**
          * Handles the event when the task session has been opened.
          */
         void on_tasks_ready();

         /**
          * Handles the event when the task session has failed.
          */
         void on_tasks_failure(tasksbase_failure_type failure);

         /**
          * Handles an incoming message event.
          */
         void onNetMessage(Csi::Messaging::Router *router, Csi::Messaging::Message *in);
      };
   };
};


#endif


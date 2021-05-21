/* Cora.Tasks.TaskRemover.h

   Copyright (C) 2012, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Wednesday 16 May 2012
   Last Change: Tuesday 19 June 2012
   Last Commit: $Date: 2012-06-19 09:51:11 -0600 (Tue, 19 Jun 2012) $
   Last Changed by: $Author: jon $

*/

#ifndef Cora_Tasks_TaskRemover_h
#define Cora_Tasks_TaskRemover_h

#include "Cora.Tasks.TasksBase.h"
#include "Csi.Events.h"


namespace Cora
{
   namespace Tasks
   {
      /**
       * Defines the protocol that the application needs to implement.
       */
      class TaskRemover;
      class TaskRemoverClient: public Csi::InstanceValidator
      {
      public:
         /**
          * Called when the server transaction  has been completed.
          *
          * @param remover  the component that is calling this method
          * @param outcome  specifies the outcome of the server transaction.
          */
         enum outcome_type
         {
            outcome_unknown_failure = 0,
            outcome_success = 1,
            outcome_server_locked = 2,
            outcome_invalid_logon = 3,
            outcome_session_failure = 4,
            outcome_unsupported = 5,
            outcome_permission_denied = 6,
            outcome_invalid_task_id = 7
         };
         virtual void on_complete(TaskRemover *remover, outcome_type outcome) = 0;
      };


      /**
       * Defines a component that wraps the CsiLgrNet Tasks Remove Task
       * transaction.  In order to use this transaction, the application must
       * provide an object that extends the TaskRemoveClient interface.  It
       * should then create an instance of this class, call its set_task_id()
       * method to specify the task to be deleted, and then call one of the two
       * variants of start().  When the server transaction is complete, the
       * component will call the client's on_complete() method.
       */
      class TaskRemover: public TasksBase, public Csi::EventReceiver
      {
      private:
         /**
          * Specifies the object that will receive completion notifications.
          */
         TaskRemoverClient *client;

         /**
          * Specifies the identifier for the task to be removed.
          */
         uint4 task_id;

         /**
          * specifies the state of this component.
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
         TaskRemover():
            client(0),
            task_id(0),
            state(state_standby)
         { }

         /**
          * destructor
          */
         virtual ~TaskRemover()
         { finish(); }

         /**
          * @return the task identifier
          */
         uint4 get_task_id() const
         { return task_id; }

         /**
          * Sets the task identifier property.
          *
          * @param id  The identifier for the task to be removed.
          */
         void set_task_id(uint4 id)
         {
            if(state == state_standby)
               task_id = id;
            else
               throw exc_invalid_state();
         }

         /**
          * Initiates the server transaction using a newly created connection.
          *
          * @client_  The client that will receive notification of completion.
          * @router  Represents the newly created connection.
          */
         typedef TaskRemoverClient client_type;
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
          * Initiates the server transaction using connection of another client.
          *
          * @client_  The client that will receive notification of completion.
          * @other_client  The client from which we will borrow the connection
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
          * Cleans up after this component.
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
          * Formats the specified outcome to the stream,
          *
          * @param out The output stream
          * @param outcome the code to be formatted.
          */
         static void format_outcome(std::ostream &out, client_type::outcome_type outcome);

      protected:
         /**
          * Handles the case where the task session is available.
          */
         virtual void on_tasks_ready();

         /**
          * Handles the case of the tasks session reporting a failure.
          */
         virtual void on_tasks_failure(tasksbase_failure_type failure);

         /**
          * Handles an incoming message.
          */
         virtual void onNetMessage(
            Csi::Messaging::Router *router, Csi::Messaging::Message *message);
      };
   };
};


#endif

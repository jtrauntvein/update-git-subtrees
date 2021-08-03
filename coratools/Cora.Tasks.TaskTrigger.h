/* Cora.Tasks.TaskTrigger.h

   Copyright (C) 2012, Campbell Scientific, Inc

   Written by: Jon Trauntvein 
   Date Begun: Wednesday 13 June 2012
   Last Change: Wednesday 13 June 2012
   Last Commit: $Date: 2012-06-13 14:35:34 -0600 (Wed, 13 Jun 2012) $
   Last Changed by: $Author: jon $

*/

#ifndef Cora_Tasks_TaskTrigger_h
#define Cora_Tasks_TaskTrigger_h

#include "Cora.Tasks.TasksBase.h"
#include "Csi.Events.h"


namespace Cora
{
   namespace Tasks
   {
      ////////////////////////////////////////////////////////////
      // class TaskTriggerClient
      ////////////////////////////////////////////////////////////
      class TaskTrigger;
      class TaskTriggerClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_complete
         ////////////////////////////////////////////////////////////
         enum outcome_type
         {
            outcome_failure_unknown = 0,
            outcome_success = 1,
            outcome_invalid_task_id = 2,
            outcome_failure_logon = 3,
            outcome_failure_session = 4,
            outcome_failure_unsupported = 5,
            outcome_failure_security = 6
         };
         virtual void on_complete(TaskTrigger *trigger, outcome_type outcome) = 0;
      };


      ////////////////////////////////////////////////////////////
      // class TaskTrigger
      //
      // This class defines a component that the application can use to
      // manually trigger a task in the LoggerNet server.  In order to use this
      // component, the application must provide a client object that extends
      // class TaskTriggerClient.  It must then create an instance of
      // ClassTrigger, set the task identifier, and call one of the two start()
      // methods.  When the server transaction is complete, the client's
      // on_complete() method will be invoked.
      ////////////////////////////////////////////////////////////
      class TaskTrigger: public TasksBase, public Csi::EventReceiver
      {
      private:
         ////////////////////////////////////////////////////////////
         // client
         ////////////////////////////////////////////////////////////
         TaskTriggerClient *client;

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
         
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         TaskTrigger():
            client(0),
            task_id(0),
            state(state_standby)
         { }

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~TaskTrigger()
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
            if(state != state_standby)
               throw exc_invalid_state();
            task_id = id;
         }

         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         typedef TaskTriggerClient client_type;
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

         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         void start(client_type *client_, ClientBase *other)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("invalid client pointer");
            client = client_;
            state = state_delegate;
            TasksBase::start(other);
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
         // describe_outcome
         ////////////////////////////////////////////////////////////
         static void describe_outcome(std::ostream &out, client_type::outcome_type outcome);

         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

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


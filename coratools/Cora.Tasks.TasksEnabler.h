/* Cora.Tasks.TasksEnabler.h

   Copyright (C) 2012, Campbell Scientific, Inc

   Written by: Jon Trauntvein 
   Date Begun: Wednesday 13 June 2012
   Last Change: Wednesday 13 June 2012
   Last Commit: $Date: 2012-06-13 10:50:06 -0600 (Wed, 13 Jun 2012) $
   Last Changed by: $Author: jon $

*/

#ifndef Cora_Tasks_TasksEnabler_h
#define Cora_Tasks_TasksEnabler_h

#include "Cora.Tasks.TasksBase.h"
#include "Csi.Events.h"


namespace Cora
{
   namespace Tasks
   {
      ////////////////////////////////////////////////////////////
      // class TasksEnablerClient
      ////////////////////////////////////////////////////////////
      class TasksEnabler;
      class TasksEnablerClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_complete
         ////////////////////////////////////////////////////////////
         enum outcome_type
         {
            outcome_unknown_failure = 0,
            outcome_success = 1,
            outcome_failure_logon = 2,
            outcome_failure_session = 3,
            outcome_failure_unsupported = 4,
            outcome_failure_security = 5
         };
         virtual void on_complete(
            TasksEnabler *enabler, outcome_type outcome) = 0;
      };


      ////////////////////////////////////////////////////////////
      // class TasksEnabler
      //
      // This class defines a component that can be used by the application to
      // enable or disable the automatic triggering of tasks in the LoggerNet
      // server.  In order to use this component, an application must provide
      // a client object that extends class TasksEnablerClient.  It should then
      // create an instance of class TasksEnabler, call set_tasks_enabled() to
      // specify whether tasks should be enabled, and call one of the two
      // versions of start().  When the transaction is complete, the client's
      // on_complete() method will be invoked.
      ////////////////////////////////////////////////////////////
      class TasksEnabler: public TasksBase, public Csi::EventReceiver
      {
      private:
         ////////////////////////////////////////////////////////////
         // tasks_enabled
         ////////////////////////////////////////////////////////////
         bool tasks_enabled;

         ////////////////////////////////////////////////////////////
         // client
         ////////////////////////////////////////////////////////////
         TasksEnablerClient *client;

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
         TasksEnabler():
            client(0),
            tasks_enabled(false),
            state(state_standby)
         { }

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~TasksEnabler()
         { finish(); }

         ////////////////////////////////////////////////////////////
         // get_tasks_enabled
         ////////////////////////////////////////////////////////////
         bool get_tasks_enabled() const
         { return tasks_enabled; }

         ////////////////////////////////////////////////////////////
         // set_tasks_enabled
         ////////////////////////////////////////////////////////////
         void set_tasks_enabled(bool enabled)
         {
            if(state == state_standby)
               tasks_enabled = enabled;
            else
               throw exc_invalid_state();
         }

         ////////////////////////////////////////////////////////////
         // start (from new connection)
         ////////////////////////////////////////////////////////////
         typedef TasksEnablerClient client_type;
         void start(client_type *client_, router_handle &router)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("invalid client pointer");
            client = client_;
            TasksBase::start(router);
         }

         ////////////////////////////////////////////////////////////
         // start (share connection)
         ////////////////////////////////////////////////////////////
         void start(client_type *client_, ClientBase *other)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("invalid client pointer");
            client = client_;
            TasksBase::start(other);
         }

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
         // describe_outcome
         ////////////////////////////////////////////////////////////
         static void describe_outcome(std::ostream &out, client_type::outcome_type outcome);
         
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
            Csi::Messaging::Router *router,  Csi::Messaging::Message *message);
      };
   };
};

#endif


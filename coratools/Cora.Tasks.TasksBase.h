/* Cora.Tasks.TasksBase.h

   Copyright (C) 2012, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Tuesday 15 May 2012
   Last Change: Wednesday 16 May 2012
   Last Commit: $Date: 2012-09-07 08:37:22 -0600 (Fri, 07 Sep 2012) $
   Last Changed by: $Author: jon $

*/

#ifndef Cora_Tasks_TasksBase_h
#define Cora_Tasks_TasksBase_h

#include "Cora.ClientBase.h"
#include "Cora.Tasks.Defs.h"
#include "Cora.Sec2.Defs.h"


namespace Cora
{
   namespace Tasks
   {
      /**
       * Defines a base class for components that access the functionality of the CsiLgrNet PakBus
       * Tasks interface.
       */
      class TasksBase: public ClientBase
      {
      public:
         /**
          * default constructor
          */
         TasksBase();

         /**
          * destructor
          */
         virtual ~TasksBase();

         /**
          * Starts a session with the Tasks interface using a newly created router.
          *
          * @param router  The object that represents the newly created connection to the LoggerNet
          * server.
          */
         virtual void start(router_handle &router);

         /**
          * Starts a sessio with the Tasks interface by sharing the connection with another
          * component.
          *
          * @param other_client  The component that owns the connection which we will share.
          */
         virtual void start(ClientBase *other_client);

         /**
          * Cancels any session that this component has started.
          */
         virtual void finish();

         ////////////////////////////////////////////////////////////
         // get_access_level
         ////////////////////////////////////////////////////////////
         typedef Sec2::AccessLevels::AccessLevelType access_level_type;
         access_level_type get_access_level() const
         { return access_level; }

      protected:
         /**
          * Called when the session with the Tasks interface has been successfully opened.
          */
         virtual void on_tasks_ready() = 0;

         /**
          * Called when an error on the Tasks interface has been reported by the server.
          *
          * @param failure  Reports the nature of the failure.
          */
         enum tasksbase_failure_type
         {
            tasksbase_failure_unknown,
            tasksbase_failure_logon,
            tasksbase_failure_session,
            tasksbase_failure_unsupported,
            tasksbase_failure_security
         };
         virtual void on_tasks_failure(tasksbase_failure_type failure) = 0;

         /**
          * Formats the specified failure code to the stream
          *
          * @param out  The output stream
          * @param failure  The failure code to format
          */
         static void format_failure(std::ostream &out, tasksbase_failure_type failure);

         /**
          * Handles an incoming message for this component.
          *
          * @param router  Represents our connection to the server.
          * @param message The content of the message
          */
         virtual void onNetMessage(
            Csi::Messaging::Router *router, Csi::Messaging::Message *message);

         /**
          * Handles the event when a session that this component owns has been broken.
          *
          * @param router  Represents the connection to the server.
          * @param session_no  Identifies the broken session.
          * @param reason  Encodes an explanation for the breakage.
          * @param why     A text explanation of the failure.
          */
         virtual void onNetSesBroken(
            Csi::Messaging::Router *router,
            uint4 session_no,
            uint4 reason,
            char const *why);

         /**
          * Handles the event where the base component has completed its connection.
          */
         virtual void on_corabase_ready();

         /**
          * Handles the event where the base component has reported a failure.
          *
          * @param failure  an encoded explanation of the failure.
          */
         virtual void on_corabase_failure(corabase_failure_type failure);

         /**
          * Handles the event where the base component has reported a session failure.
          */
         virtual void on_corabase_session_failure();

      protected:
         /**
          * Keeps track of our session with the tasks interface.
          */
         uint4 tasks_session;

         /**
          * Keeps track of the state of our connection with the tasks interface.
          */
         enum tasksbase_state_type
         {
            tasksbase_state_standby,
            tasksbase_state_delegate,
            tasksbase_state_attach,
            tasksbase_state_ready
         } tasksbase_state;

         ////////////////////////////////////////////////////////////
         // access_level
         ////////////////////////////////////////////////////////////
         access_level_type access_level;
      };
   };
};


#endif


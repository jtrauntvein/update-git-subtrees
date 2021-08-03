/* Cora.Tasks.TasksBase.cpp

   Copyright (C) 2012, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Tuesday 15 May 2012
   Last Change: Friday 07 September 2012
   Last Commit: $Date: 2012-09-07 08:37:22 -0600 (Fri, 07 Sep 2012) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Tasks.TasksBase.h"
#include "Cora.LgrNet.Defs.h"


namespace Cora
{
   namespace Tasks
   {
      TasksBase::TasksBase():
         tasks_session(0),
         tasksbase_state(tasksbase_state_standby),
         access_level(Sec2::AccessLevels::level_root)
      { }


      TasksBase::~TasksBase()
      { finish(); }


      void TasksBase::start(router_handle &router)
      {
         if(tasksbase_state == tasksbase_state_standby)
         {
            tasksbase_state = tasksbase_state_delegate;
            ClientBase::start(router);
         }
         else
            throw exc_invalid_state();
      } // start


      void TasksBase::start(ClientBase *other_client)
      {
         if(tasksbase_state == tasksbase_state_standby)
         {
            tasksbase_state = tasksbase_state_delegate;
            ClientBase::start(other_client);
         }
         else
            throw exc_invalid_state();
      } // start (other component)


      void TasksBase::finish()
      {
         if(router != 0 && tasks_session)
         {
            router->closeSession(tasks_session);
            tasks_session = 0;
         }
         tasksbase_state = tasksbase_state_standby;
         ClientBase::finish();
      } // finish


      void TasksBase::format_failure(std::ostream &out, tasksbase_failure_type failure)
      {
         switch(failure)
         {
         case tasksbase_failure_logon:
            ClientBase::describe_failure(out, corabase_failure_logon);
            break;
            
         case tasksbase_failure_session:
            ClientBase::describe_failure(out, corabase_failure_session);
            break;
            
         case tasksbase_failure_unsupported:
            ClientBase::describe_failure(out, corabase_failure_unsupported);
            break;

         case tasksbase_failure_security:
            ClientBase::describe_failure(out, corabase_failure_security);
            break;

         default:
            ClientBase::describe_failure(out, corabase_failure_unknown);
            break;
         }
      } // format_failure
      
      
      void TasksBase::onNetMessage(
         Csi::Messaging::Router *router, Csi::Messaging::Message *message)
      {
         if(message->getMsgType() == Messages::announce_access_level)
         {
            uint4 temp;
            message->readUInt4(temp);
            access_level = static_cast<access_level_type>(temp);
         }
         if(tasksbase_state == tasksbase_state_attach)
         {
            if(message->getMsgType() == LgrNet::Messages::open_tasks_session_ack)
            {
               uint4 tran_no;
               uint4 outcome;
               if(message->readUInt4(tran_no) && message->readUInt4(outcome))
               {
                  if(outcome == 1)
                  {
                     tasksbase_state = tasksbase_state_ready;
                     on_tasks_ready();
                  }
                  else
                  {
                     on_tasks_failure(tasksbase_failure_unknown);
                     finish();
                  }
               }
               else
               {
                  on_tasks_failure(tasksbase_failure_unknown);
                  finish();
               }
            }
            else
               ClientBase::onNetMessage(router, message);
         }
         else
            ClientBase::onNetMessage(router, message);
      } // onNetMessage


      void TasksBase::onNetSesBroken(
         Csi::Messaging::Router *router,
         uint4 session_no,
         uint4 reason,
         char const *message)
      {
         if(session_no == tasks_session)
         {
            tasks_session = 0;
            on_tasks_failure(tasksbase_failure_session);
         }
         else
            ClientBase::onNetSesBroken(router, session_no, reason, message);
      } // onNetSesBroken


      void TasksBase::on_corabase_ready()
      {
         // we can now send the message to request attachment to the tasks interface.
         Csi::Messaging::Message command(net_session, LgrNet::Messages::open_tasks_session_cmd);
         
         tasksbase_state = tasksbase_state_attach;
         tasks_session = router->openSession(this);
         command.addUInt4(++last_tran_no);
         command.addUInt4(tasks_session);
         router->sendMessage(&command);
      } // on_corabase_ready


      void TasksBase::on_corabase_failure(corabase_failure_type failure_)
      {
         tasksbase_failure_type failure(tasksbase_failure_unknown);
         switch(failure_)
         {
         case corabase_failure_logon:
            failure = tasksbase_failure_logon;
            break;

         case corabase_failure_session:
            failure = tasksbase_failure_session;
            break;

         case corabase_failure_unsupported:
            failure = tasksbase_failure_unsupported;
            break;

         case corabase_failure_security:
            failure = tasksbase_failure_security;
            break;
         }
         on_tasks_failure(failure);
      } // on_corabase_failure


      void TasksBase::on_corabase_session_failure()
      { on_tasks_failure(tasksbase_failure_session); }
   };
};



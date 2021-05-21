/* Csi.Posix.Thread.cpp

   Copyright (C) 2005, 2010 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 31 May 2005
   Last Change: Wednesday 03 November 2010
   Last Commit: $Date: 2011-10-17 15:39:36 -0600 (Mon, 17 Oct 2011) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Posix.Thread.h"
#include "Csi.OsException.h"
#include <signal.h>


namespace Csi
{
   namespace Posix
   {
      Thread::~Thread()
      { wait_for_end(); }


      void Thread::start()
      {
         if(!is_started)
         {
            int rcd = pthread_create(&thread_handle,0,thread_func,this);
            if(rcd != 0)
               throw OsException("Thread create failed");
            is_started = needs_to_join = true;
         }
      } // start


      void Thread::wait_for_end()
      {
         if(is_started || needs_to_join)
         {
            void *status_ptr;
            if(pthread_join(thread_handle,&status_ptr))
            {
               OsException error("pthread_join failed");
               trace("Csi::Posix::Thread::wait_for_end: %s", error.what());
            }
            is_started = needs_to_join = false;
         }
      } // wait_for_end


      void Thread::signal_urgent()
      {
         if(is_started)
            pthread_kill(thread_handle,SIGURG);
      } // signal_urgent


      namespace
      {
         ////////////////////////////////////////////////////////////
         // urgent_signal_handler
         //
         // Defines the handler for SIGURG.  This is mostly do-nothing since our purpose for the
         // signal is simply to interrupt a thread that is blocked.
         ////////////////////////////////////////////////////////////
         void urgent_signal_handler(int sig)
         { }
      };
      

      void *Thread::thread_func(void *arg)
      {
         // we will want to set the thread signal mask and action so that this thread can be
         // interrupted by SIGURG.
         struct sigaction action;

         memset(&action,0,sizeof(action));
         sigemptyset(&action.sa_mask);
         sigaddset(&action.sa_mask,SIGURG);
         action.sa_handler = urgent_signal_handler;
         sigaction(SIGURG,&action,0);
         pthread_sigmask(SIG_UNBLOCK,&action.sa_mask,0);
         
         // now we can execute the thread
         Thread *thread_obj = static_cast<Thread *>(arg);
         try
         {
            thread_obj->execute();
            thread_obj->is_started = false;
         }
         catch(std::exception &e)
         { trace("Csi::Posix::Thread::thread_func -- uncaught exception: %s", e.what()); }
         return thread_obj;
      } // thread_func
   };
};



/* Csi.Win32.Win32Thread.h

   Copyright (C) 2005, 2011 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 25 January 2005
   Last Change: Thursday 16 June 2011
   Last Commit: $Date: 2011-06-16 07:30:50 -0600 (Thu, 16 Jun 2011) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Csi_Win32_Win32Thread_h
#define Csi_Win32_Win32Thread_h

#include "Csi.Win32.Condition.h"


namespace Csi
{
   namespace Win32
   {
      ////////////////////////////////////////////////////////////
      // class Win32Thread
      //
      // Represents the state of the thread including the thread identifier as
      // well as whether it is started or stopped. Defines methods for
      // controlling the thread. Also defines a virtual method that will be
      // invoke after the thread has been started.
      ////////////////////////////////////////////////////////////
      class Win32Thread
      {
      protected:
         ////////////////////////////////////////////////////////////
         // thread_id
         //
         // Identifier for the thread in a started state
         ////////////////////////////////////////////////////////////
         uint4 thread_id;

         ////////////////////////////////////////////////////////////
         // is_started
         //
         // Set to true when the thread is in a started state
         ////////////////////////////////////////////////////////////
         bool is_started;

      private:
         ////////////////////////////////////////////////////////////
         // start_event
         //
         // Used to force the thread calling start() to block until this thread
         // has entered a started state.
         ////////////////////////////////////////////////////////////
         Csi::Win32::Condition start_event;

         ////////////////////////////////////////////////////////////
         // end_event
         //
         // Used to force the thread calling wait_for_end() to block until this
         // thread is completly finished.
         ////////////////////////////////////////////////////////////
         Csi::Win32::Condition end_event;

      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         Win32Thread();

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~Win32Thread();

         ////////////////////////////////////////////////////////////
         // start
         //
         // Checks to see if the thread is started.  If it is not, makes the
         // appropriate OS call to start the thread and waits for the
         // started_event object to become signalled. 
         ////////////////////////////////////////////////////////////
         virtual void start();

         ////////////////////////////////////////////////////////////
         // wait_for_end
         //
         // Blocks until the end event has become signalled. 
         ////////////////////////////////////////////////////////////
         virtual void wait_for_end();

         ////////////////////////////////////////////////////////////
         // signal_urgent
         //
         // Created for compatibility with the posix thread component which
         // dispatches a thread signal to interrupt the thread.  
         ////////////////////////////////////////////////////////////
         virtual void signal_urgent()
         { }
         
      protected:
         ////////////////////////////////////////////////////////////
         // execute
         //
         // Must be overloaded to perform the actual work that should be
         // accomplished within the thread.  The thread should be considered
         // effectively ended after this function has exited. 
         ////////////////////////////////////////////////////////////
         virtual void execute() = 0;

      private:
         ////////////////////////////////////////////////////////////
         // entry_point
         ////////////////////////////////////////////////////////////
         static void entry_point(void *param);
      };
   };
};


#endif

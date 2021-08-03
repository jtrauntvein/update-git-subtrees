/* Csi.Win32.AppExcluder.h

   Copyright (C) 2004, 2005 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 23 September 2004
   Last Change: Monday 28 March 2005
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)
   Last Changed by: $Author: tmecham $

*/

#ifndef Csi_Win32_AppExcluder_h
#define Csi_Win32_AppExcluder_h

#include "StrAsc.h"
#include "Csi.Events.h"
#include "Csi.Thread.h"
#include "Csi.Win32.Condition.h"
#include "Csi.Win32.Mutex.h"


namespace Csi
{
   namespace Win32
   {
      ////////////////////////////////////////////////////////////
      // class AppExcluder
      //
      // Defines a component that can be used by an application to exclude
      // other instances of itself from running.  It does this by creating a
      // mutex based upon the module name.  This mutex will be created and held
      // in a thread.  An event object will also be created that is based upon
      // the same name that can be used to signal the other app (if any) that
      // another instance was started.  If this event is signalled in the
      // thread (which will spend most of its time blocking on that event), a
      // Csi::Event object will be fired at a registered receiver to indicate
      // this occurrence.
      //
      // Only one of these objects should be created per application. 
      ////////////////////////////////////////////////////////////
      class AppExcluder: public Thread
      {
      public:
         ////////////////////////////////////////////////////////////
         // comm_event_signalled
         //
         // Specifies the identifier for the Csi::Event that will be posted
         // when comm_event is signalled. 
         ////////////////////////////////////////////////////////////
         static uint4 const comm_event_signalled;
         
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         AppExcluder(
            EventReceiver *receiver_,
            StrAsc const &app_name_ = ""); 

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~AppExcluder();

         //@group class Thread derived methods
         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         virtual void start();

         ////////////////////////////////////////////////////////////
         // wait_for_end
         ////////////////////////////////////////////////////////////
         virtual void wait_for_end();
         //@endgroup

         ////////////////////////////////////////////////////////////
         // get_owns_mutex
         ////////////////////////////////////////////////////////////
         bool get_owns_mutex() const
         { return owns_mutex; }

      protected:
         ////////////////////////////////////////////////////////////
         // execute
         ////////////////////////////////////////////////////////////
         virtual void execute();

      private:
         ////////////////////////////////////////////////////////////
         // app_name
         ////////////////////////////////////////////////////////////
         StrAsc app_name; 

         ////////////////////////////////////////////////////////////
         // start_wait_event
         //
         // This event object is used to force the start() method to delay
         // execution until the execute method has reached a steady state. 
         ////////////////////////////////////////////////////////////
         Condition start_wait_event;

         ////////////////////////////////////////////////////////////
         // app_mutex
         //
         // Specifies the mutex that will be used to block other apps from
         // starting.  The thread will attempt to lock the mutex and, failing
         // that, will signal the comm_event object before ending. 
         ////////////////////////////////////////////////////////////
         SharedPtr<Mutex> app_mutex;

         ////////////////////////////////////////////////////////////
         // comm_event
         //
         // This event is used by one app to signal another that has the mutex
         // that the second app attempted to obtain it.  It is also used to
         // force the thread to terminate.  
         ////////////////////////////////////////////////////////////
         SharedPtr<Condition> comm_event;

         ////////////////////////////////////////////////////////////
         // should_stop
         //
         // Flags the condition that the thread should terminate
         // (wait_for_end() has been called and is waiting).
         ////////////////////////////////////////////////////////////
         bool should_stop;

         ////////////////////////////////////////////////////////////
         // owns_mutex
         //
         // This flag is set to false when the thread is started and will only
         // be set to true if the mutex was succesfully claimed.  
         ////////////////////////////////////////////////////////////
         bool owns_mutex;

         ////////////////////////////////////////////////////////////
         // receiver
         //
         // Specifies the object (if any) that will receive notification when
         // an application signals the comm_event object.
         ////////////////////////////////////////////////////////////
         EventReceiver *receiver;
      };
   };
};


#endif

/* Csi.Win32.Win32Thread.cpp

   Copyright (C) 2005, 2014 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 25 January 2005
   Last Change: Thursday 25 September 2014
   Last Commit: $Date: 2014-09-25 08:10:49 -0600 (Thu, 25 Sep 2014) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#define WIN32_LEAN_AND_MEAN 
#include "Csi.Win32.Win32Thread.h"
#include "Csi.OsException.h"
#include "Csi.StrAscStream.h"
#define _WINSOCKAPI_ // prevent windows.h from including winsock.h
#include <windows.h>
#include <process.h>
#include <assert.h>
#include <typeinfo>


namespace Csi
{
   namespace Win32
   {
      ////////////////////////////////////////////////////////////
      // class Win32Thread definitions
      ////////////////////////////////////////////////////////////
      Win32Thread::Win32Thread():
         thread_id(0),
         is_started(false)
      { }


      Win32Thread::~Win32Thread()
      { wait_for_end(); }


      void Win32Thread::start()
      {
         if(!is_started)
         {
            // we need to make sure that the events that we wait on for starting and ending are in
            // their reset state. This is because the thread can be started and stopped several times
            // in the object lifetime.
            start_event.reset();
            end_event.reset();

            // we can now start the thread and wait for the start event to be signaled
            if(_beginthread(entry_point,0,this) == -1)
            {
               OStrAscStream msg;
               std::type_info const &thread_type(typeid(*this));
               msg << thread_type.name() << "\",\"thread start failed";
               throw OsException(msg.c_str());
            }
            start_event.wait();
         }
      } // start


      void Win32Thread::wait_for_end()
      {
         if(is_started)
            end_event.wait();
      } // wait_for_end


      void Win32Thread::entry_point(void *param)
      {
         try
         {
            Win32Thread *t = reinterpret_cast<Win32Thread *>(param);
            
            assert(t != 0);
            t->thread_id = GetCurrentThreadId();
            t->is_started = true;
            t->start_event.set();
            t->execute();
            t->thread_id = 0;
            t->is_started = false;
            t->end_event.set();
         }
         catch(std::exception &e)
         { trace("Uncaught thread exception: %s", e.what()); }
      } // entry_point 
   };
};


/* Csi.Win32.Mutex.cpp

   Copyright (C) 1998, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 29 November 1999
   Last Change: Tuesday 10 July 2012
   Committed: $Date: 2012-07-10 10:40:39 -0600 (Tue, 10 Jul 2012) $ 
   Committed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Win32.Mutex.h"
#include "Csi.OsException.h"
#include <assert.h>


namespace Csi
{
   namespace Win32
   {
      ////////////////////////////////////////////////////////////
      // class Mutex definitions
      ////////////////////////////////////////////////////////////
      Mutex::Mutex(char const *name, bool initial_owner):
         kernel_handle(0),
         is_locked(false)
      {
         kernel_handle = ::CreateMutexA(0,initial_owner,name);
         if(kernel_handle == 0)
            throw OsException("Csi::Win32::Mutex construction failed");
      } // constructor


      Mutex::~Mutex()
      {
         if(kernel_handle)
         {
            if(is_locked)
               ReleaseMutex(kernel_handle);
            CloseHandle(kernel_handle);
         }
      } // destructor


      void Mutex::lock()
      {
         assert(kernel_handle != 0);
         uint4 rcd = ::WaitForSingleObject(kernel_handle,INFINITE);

         if(rcd != WAIT_OBJECT_0)
            throw OsException("Mutex lock failure");
         is_locked = true;
      } // lock

      
      bool Mutex::try_lock()
      {
         assert(kernel_handle != 0);
         uint4 rcd = ::WaitForSingleObject(kernel_handle,0);
         bool rtn = false;

         if(rcd == WAIT_OBJECT_0)
            rtn = true;
         else if(rcd == WAIT_ABANDONED)
         {
            rcd = ::WaitForSingleObject(kernel_handle,0);
            if(rcd == WAIT_OBJECT_0)
               rtn = true;
         }
         is_locked = rtn;
         return rtn;
      } // try_lock


      void Mutex::release()
      {
         assert(kernel_handle != 0);
         ::ReleaseMutex(kernel_handle);
         is_locked = false;
      } // release
   };
};

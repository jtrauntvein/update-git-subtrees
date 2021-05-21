/* Csi.Win32.Mutex.h

   Copyright (C) 1998, 2011 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 29 November 1999
   Last Change: Wednesday 16 March 2011
   Committed: $Date: 2011-03-17 09:05:06 -0600 (Thu, 17 Mar 2011) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Csi_Win32_Mutex_h
#define Csi_Win32_Mutex_h

#define WIN32_LEAN_AND_MEAN
#define _WINSOCKAPI_    // stops windows.h including winsock.h
#include <windows.h>
#include "CsiTypeDefs.h"


namespace Csi
{
   namespace Win32
   {
      ////////////////////////////////////////////////////////////
      // class Mutex
      //////////////////////////////////////////////////////////// 
      class Mutex
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         //////////////////////////////////////////////////////////// 
         Mutex(char const *name = 0, bool initial_owner = false);

         ////////////////////////////////////////////////////////////
         // destructor
         //////////////////////////////////////////////////////////// 
         virtual ~Mutex();

         ////////////////////////////////////////////////////////////
         // lock
         //
         // Attempts to claim ownership of the mutex.  Will block until
         // ownership is gained.  try_lock() implements a non-blocking
         // version.  
         //////////////////////////////////////////////////////////// 
         void lock();

         ////////////////////////////////////////////////////////////
         // try_lock
         //
         // Attempts to claim immediate ownership of the lock.  If the mutex is
         // already locked, the return value will be false.  
         ////////////////////////////////////////////////////////////
         bool try_lock();

         ////////////////////////////////////////////////////////////
         // release
         //
         // releases the claim of ownership of the mutex object that was
         // created using lock()
         //////////////////////////////////////////////////////////// 
         void release();

      private:
         ////////////////////////////////////////////////////////////
         // kernel_handle
         //
         // Handle to the kernel object
         //////////////////////////////////////////////////////////// 
         HANDLE kernel_handle;

         ////////////////////////////////////////////////////////////
         // is_locked
         //
         // Set to true if a lock has been successfully obtained
         //////////////////////////////////////////////////////////// 
         bool is_locked;
      };
   };
};

#endif

/* Csi.Win32.Win32CriticalSection.h

   Copyright (C) 2005, 2011 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 25 January 2005
   Last Change: Wednesday 16 March 2011
   Last Commit: $Date: 2011-03-17 09:05:06 -0600 (Thu, 17 Mar 2011) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Csi_Win32_Win32CriticalSection_h
#define Csi_Win32_Win32CriticalSection_h

#define WIN32_LEAN_AND_MEAN
#define _WINSOCKAPI_ // prevent windows.h from including winsock.h
#include <windows.h>


namespace Csi
{
   namespace Win32
   {
      ////////////////////////////////////////////////////////////
      // class Win32CriticalSection
      //
      // Abstraction of a critical section object. This object can be locked to
      // prevent multiple access to blocks of code by multiple threads much as
      // a mutex can be used to block between processes. This is declared as a
      // separate class from the mutex because it can be implemented more
      // efficiently in windows.
      ////////////////////////////////////////////////////////////
      class Win32CriticalSection
      {
      private:
         ////////////////////////////////////////////////////////////
         // os_handle
         ////////////////////////////////////////////////////////////
         CRITICAL_SECTION os_handle;
         
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         Win32CriticalSection()
         { InitializeCriticalSection(&os_handle); }

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~Win32CriticalSection()
         { DeleteCriticalSection(&os_handle); }

         ////////////////////////////////////////////////////////////
         // lock
         ////////////////////////////////////////////////////////////
         void lock()
         { EnterCriticalSection(&os_handle); }

         ////////////////////////////////////////////////////////////
         // unlock
         ////////////////////////////////////////////////////////////
         void unlock()
         { LeaveCriticalSection(&os_handle); }
      };
   };
};


#endif

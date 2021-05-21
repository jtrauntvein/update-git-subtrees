/* Csi.Win32.Condition.h

   Copyright (C) 1999, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 28 March 2005
   Last Change: Tuesday 10 July 2012
   Last Commit: $Date: 2012-07-10 07:56:48 -0600 (Tue, 10 Jul 2012) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Csi_Win32_Condition_h
#define Csi_Win32_Condition_h

#define WIN_32_LEAN_AND_MEAN
#define _WINSOCKAPI_    // stops windows.h including winsock.h
#include <windows.h>
#include "CsiTypeDefs.h"
#include "StrUni.h"


namespace Csi
{
   namespace Win32
   {
      ////////////////////////////////////////////////////////////
      // class Condition
      //
      // Encapsulates a Win32 Event kernel object.  This is more or less the
      // same as a posix condition object. 
      ////////////////////////////////////////////////////////////
      class Condition
      {
      private:
         ////////////////////////////////////////////////////////////
         // kernel_handle
         ////////////////////////////////////////////////////////////
         HANDLE kernel_handle;

      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         Condition(
            char const *name_ = 0,
            bool initial_state = false,
            bool manual_reset = true)
         {
            StrUni name;
            if(name_)
               name.append_utf8(name_);
            kernel_handle = ::CreateEventW(
               0,
               manual_reset,
               initial_state,
               name.length() > 0 ? name.c_str() : 0);
         }

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~Condition()
         { ::CloseHandle(kernel_handle); }

         ////////////////////////////////////////////////////////////
         // set
         ////////////////////////////////////////////////////////////
         void set()
         { ::SetEvent(kernel_handle); }

         ////////////////////////////////////////////////////////////
         // reset
         ////////////////////////////////////////////////////////////
         void reset()
         { ::ResetEvent(kernel_handle); }

         ////////////////////////////////////////////////////////////
         // wait
         //
         // Waits up to the amount of time specified in milli-seconds for the
         // event to enter a signalled state.  Returns true if ownership was
         // established during the wait.
         ////////////////////////////////////////////////////////////
         bool wait(uint4 timeout = INFINITE)
         {
            uint4 rcd = ::WaitForSingleObject(kernel_handle,timeout);
            bool rtn = false;
            if(rcd == WAIT_OBJECT_0)
               rtn = true;
            return rtn;
         }

         ////////////////////////////////////////////////////////////
         // get_handle
         ////////////////////////////////////////////////////////////
         HANDLE get_handle()
         { return kernel_handle; }
      };
   };
};


#endif

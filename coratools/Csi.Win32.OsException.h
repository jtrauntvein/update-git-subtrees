/* Csi.Win32.OsException.h

   Copyright (C) 2005, 2005 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 24 March 2005
   Last Change: Friday 07 October 2005
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)
   Last Changed by: $Author: tmecham $

*/

#ifndef Csi_Win32_OsException_h
#define Csi_Win32_OsException_h

#include "CsiTypeDefs.h"
#include "Csi.MsgExcept.h"


namespace Csi
{
   namespace Win32
   {
      ////////////////////////////////////////////////////////////
      // class OsException
      //
      // Performs the same task as MsgExcept only it also reports on error
      // conditions based from the operating system using the Win32
      // GetLastError method. The idea is that if an OS error occurs, code can
      // create an object of this class and the part of the message will
      // contain an explanation of the operating system error.
      ////////////////////////////////////////////////////////////
      class OsException: public MsgExcept
      {
      protected:
         ////////////////////////////////////////////////////////////
         // os_error
         ////////////////////////////////////////////////////////////
         uint4 os_error;

      public:
         ////////////////////////////////////////////////////////////
         // constructor (with error code)
         ////////////////////////////////////////////////////////////
         OsException(uint4 os_error_, char const *msg_);
         
         ////////////////////////////////////////////////////////////
         // constructor
         //
         // This constructor behaves two ways:
         //
         //  1 - If the return value of GetLastError() is zero, this
         //  constructor behaves the same ways as the constructor for MsgExcept
         //
         //  2 - If the return value of GetLastError is non-zero, this
         //  constructor will attempt to locate the error message from the
         //  operating system associated with the error code. It will place the
         //  code, the explanation, and the message provided in the internal
         //  message field
         ////////////////////////////////////////////////////////////
         OsException(char const *msg_);

         ////////////////////////////////////////////////////////////
         // get_osError
         ////////////////////////////////////////////////////////////
         uint4 get_osError() const
         { return os_error; }

         ////////////////////////////////////////////////////////////
         // set_osError
         ////////////////////////////////////////////////////////////
         void set_osError(uint4 os_error_, char const *msg_);
      };
   };
};


#endif

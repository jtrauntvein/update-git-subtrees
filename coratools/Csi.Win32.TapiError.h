/* Csi.Win32.TapiError.h

   Copyright (C) 2000, 2005 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 28 September 2000
   Last Change: Thursday 24 March 2005
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)

*/

#ifndef Csi_Win32_TapiError_h
#define Csi_Win32_TapiError_h

#include "CsiTypeDefs.h"
#include "Csi.MsgExcept.h"

namespace Csi
{
   namespace Win32
   {
      ////////////////////////////////////////////////////////////
      // class TapiError
      //
      // Defines an exception class based on std::exception that stores an error code and overloads
      // what() to describe the specific error.
      ////////////////////////////////////////////////////////////
      class TapiError: public MsgExcept
      {
      private:
         ////////// error_code
         int error_code;
         
      public:
         ////////// constructor
         TapiError(int4 error_code_);

         ////////// get_error_code
         int4 get_error_code() const { return error_code; }
      };



   };
};

#endif

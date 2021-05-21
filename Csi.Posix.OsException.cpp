/* Csi.Posix.OsException.cpp

   Copyright (C) 2005, 2005 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 24 March 2005
   Last Change: Tuesday 20 December 2005
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)
   Last Changed by: $Author: tmecham $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Posix.OsException.h"
#include <string.h>
#include <sstream>


namespace Csi
{
   namespace Posix
   {
      ////////////////////////////////////////////////////////////
      // class OsException definitions
      ////////////////////////////////////////////////////////////
      OsException::OsException(int error_code, char const *msg):
         MsgExcept("")
      { set_error(msg,error_code); }
      
         
      OsException::OsException(char const *msg_):
         MsgExcept("")
      {
         set_error(msg_,errno);
         errno = 0;
      } // constructor


      void OsException::set_error(
         char const *message,
         int error_code)
      {
         // form the initial message
         std::ostringstream out;
         out << message;

         // check to see if a system error has occurred by checking errno
         os_error = error_code;
         if(os_error)
            out << "\",\"" << os_error << "\",\"" << strerror(os_error);
         msg.setContents(
            out.str().c_str(),
            out.str().length());
      } // set_error
   };
};


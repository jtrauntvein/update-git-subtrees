/* Csi.Win32.OsException.cpp

   Copyright (C) 1996, 2012 Campbell Scientific, Inc.
   
   Written by Jon Trauntvein
   Date Begun: Wednesday 11 December 1996
   Last Change: Tuesday 10 July 2012
   Last Commit: $Date: 2012-07-10 10:40:39 -0600 (Tue, 10 Jul 2012) $ 
   Committed by: $Author: jon $
   
*/


#define STRICT
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX                // disable definition of min() and max()
#define _WINSOCKAPI_ // prevent windows.h from including winsock.h
#include <windows.h>
#pragma hdrstop
#include <string.h>
#include <stdio.h>
#include <sstream>
#include "Csi.Win32.OsException.h"


namespace Csi
{
   namespace Win32
   {
      ////////////////////////////////////////////////////////////
      // class OsException definitions
      //////////////////////////////////////////////////////////// 
      OsException::OsException(const char *msg_):
         MsgExcept("")
      {
         set_osError(GetLastError(),msg_);
         SetLastError(0);
      } // constructor


      OsException::OsException(uint4 os_error_, char const *msg_):
         MsgExcept("")
      { set_osError(os_error_,msg_); }

      
      void OsException::set_osError(uint4 os_error_, char const *msg_)
      {
         // form the initial message
         std::ostringstream out; 
         out << msg_;
         
         // check to see if an operating system error occurred
         os_error = os_error_;
         if(os_error != ERROR_SUCCESS)
         {
            // add the error number to the message string
            out << "\",\"" << os_error;
            
            // get the operating system message
            char *msgBuff;
            
            if(FormatMessageA(
                  FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
                  NULL,
                  os_error,
                  MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
                  (LPSTR)&msgBuff,
                  0,
                  NULL) && msgBuff)
            {
               char *line_end_pos = strstr(msgBuff,"\r\n");
               if(line_end_pos)
                  line_end_pos[0] = 0;
               out << "\",\"" << msgBuff;
               LocalFree(msgBuff);
            }
         }
         msg.append(
            out.str().c_str(),
            out.str().length());
      } // set_lastError
   };
};
      

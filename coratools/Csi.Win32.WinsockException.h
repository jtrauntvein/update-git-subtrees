/* Csi.Win32.WinsockException.h

   Copyright (C) 2002, 2005 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 05 March 2002
   Last Change: Monday 12 September 2005
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)
   Last Changed by: $Author: tmecham $

*/

#ifndef Csi_Win32_WinsockException_h
#define Csi_Win32_WinsockException_h


#include "CsiTypeDefs.h"
#include "Csi.MsgExcept.h"


namespace Csi
{
   namespace Win32
   {
      ////////////////////////////////////////////////////////////
      // class WinsockException
      //
      // Exception that can be thrown from winsock related classes that handles the formatting of
      // winsock specific errors.  
      ////////////////////////////////////////////////////////////
      class WinsockException: public MsgExcept
      {
      protected:
         uint4 socket_error;

      public:
         ////////////////////////////////////////////////////////////
         // constructor
         //
         // Initialises the internal message buffer with the message passed in as well as the
         // socket_error default parameter.  If socket_error is zero, then the value returned from
         // WSAGetlastError() will be used in its place.  If this error is non-zero and known, the
         // error number and description will be formatted along with the specified message.
         ////////////////////////////////////////////////////////////
         WinsockException(
            char const *message,
            uint4 socket_error_ = 0);

         ////////////////////////////////////////////////////////////
         // get_winsock_error
         ////////////////////////////////////////////////////////////
         uint4 get_winsock_error() const
         { return socket_error; }
         
         ////////////////////////////////////////////////////////////
         // get_socket_error
         ////////////////////////////////////////////////////////////
         uint4 get_socket_error() const { return socket_error; }
      };
   };
};


#endif

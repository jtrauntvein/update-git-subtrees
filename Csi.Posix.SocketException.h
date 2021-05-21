/* Csi.Posix.SocketException.h

   Copyright (C) 2005, Campbell Scientific, Inc

   Written by: Jon Trauntvein
   Date Begun: Monday 12 September 2005
   Last Change: Monday 12 September 2005
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)
   Last Changed by: $Author: tmecham $

*/

#ifndef Csi_Posix_SocketException_h
#define Csi_Posix_SocketException_h

#include "Csi.Posix.OsException.h"


namespace Csi
{
   namespace Posix
   {
      ////////////////////////////////////////////////////////////
      // class SocketException
      ////////////////////////////////////////////////////////////
      class SocketException: public OsException
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         SocketException(
            char const *message,
            int socket_error = 0):
            OsException(socket_error,message)
         { }

         ////////////////////////////////////////////////////////////
         // get_socket_error
         ////////////////////////////////////////////////////////////
         int get_socket_error() const
         { return os_error; } 
      };
   };
};


#endif

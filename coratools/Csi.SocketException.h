/* Csi.SocketException.h

   Copyright (C) 2005, Campbell Scientific, Inc

   Written by: Jon Trauntvein
   Date Begun: Monday 12 September 2005
   Last Change: Monday 12 September 2005
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)
   Last Changed by: $Author: tmecham $

*/

#ifndef Csi_SocketException_h
#define Csi_SocketException_h

#ifdef _WIN32
#include "Csi.Win32.WinsockException.h"
#else
#include "Csi.Posix.SocketException.h" 
#endif


namespace Csi
{
#ifdef _WIN32
   typedef Win32::WinsockException SocketException;
#else
   using Posix::SocketException;
#endif
};


#endif

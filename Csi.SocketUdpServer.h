/* Csi.SocketUdpServer.h

   Copyright (C) 2005, 2011 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 21 September 2005
   Last Change: Wednesday 16 March 2011
   Last Commit: $Date: 2011-03-16 07:53:24 -0600 (Wed, 16 Mar 2011) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Csi_SocketUdpServer_h
#define Csi_SocketUdpServer_h

#ifdef _WIN32
#include "Csi.Win32.SocketUdpServer.h"
#else
#include "Csi.Posix.SocketUdpServer.h"
#endif


namespace Csi
{
#ifdef _WIN32
   using Win32::SocketUdpServer;
#else
   using Posix::SocketUdpServer;
#endif
};


#endif

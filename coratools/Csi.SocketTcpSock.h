/* Csi.SocketTcpSock.h

   Copyright (C) 2005, 2010 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 30 August 2005
   Last Change: Thursday 04 November 2010
   Last Commit: $Date: 2010-11-05 15:29:26 -0600 (Fri, 05 Nov 2010) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Csi_SocketTcpSock_h
#define Csi_SocketTcpSock_h

#ifdef _WIN32
#include "Csi.Win32.WinSockBufferingSocket.h"
#else
#include "Csi.Posix.SocketTcpSock.h"
#endif


namespace Csi
{
#ifdef _WIN32
   using Win32::SocketTcpSock;
#else
   using Posix::SocketTcpSock;
#endif   
};


#endif

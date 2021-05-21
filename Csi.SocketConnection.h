/* Csi.SocketConnection.h

   Copyright (C) 2008, Campbell Scientific, Inc

   Written by: Jon Trauntvein
   Date Begun: Friday 05 September 2008
   Last Change: Friday 05 September 2008
   Last Commit: $Date: 2017-10-18 09:11:43 -0600 (Wed, 18 Oct 2017) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Csi_SocketConnection_h
#define Csi_SocketConnection_h

#ifdef _WIN32
#include "Csi.Win32.SocketConnection.h"
#else
#include "Csi.Posix.SocketConnection.h"
#endif


namespace Csi
{
#ifdef _WIN32
   using Win32::SocketConnection;
#else
   using Posix::SocketConnection;
#endif
};


#endif

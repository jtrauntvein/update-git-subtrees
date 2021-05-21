/* Csi.OsException.h

   Copyright (C) 2005, 2005 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 24 March 2005
   Last Change: Friday 25 March 2005
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)
   Last Changed by: $Author: tmecham $

*/

#ifndef Csi_OsException_h
#define Csi_OsException_h

#ifdef _WIN32
#include "Csi.Win32.OsException.h"
#else
#include "Csi.Posix.OsException.h"
#endif


namespace Csi
{
#ifdef _WIN32
   using Win32::OsException;
#else
   using Posix::OsException;
#endif
};


#endif

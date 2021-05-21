/* Csi.RegistryManager.h

   Copyright (C) 2005, 2005 Campbell Scientific, Inc.

   Written by: 
   Date Begun: Tuesday 10 May 2005
   Last Change: Thursday 12 May 2005
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)
   Last Changed by: $Author: tmecham $

*/

#ifndef Csi_RegistryManager_h
#define Csi_RegistryManager_h

#ifdef _WIN32
#include "Csi.Win32.RegistryManager.h"
#else
#include "Csi.Posix.RegistryManager.h"
#endif


namespace Csi
{
#ifdef _WIN32
   using Win32::RegistryManager;
#else
   using Posix::RegistryManager;
#endif
};


#endif

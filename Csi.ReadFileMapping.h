/* Csi.ReadFileMapping.h

   Copyright (C) 2005, Campbell Scientific, Inc

   Written by: Jon Trauntvein
   Date Begun: Thursday 14 April 2005
   Last Change: Thursday 14 April 2005
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)
   Last Changed by: $Author: tmecham $

*/

#ifndef Csi_ReadFileMapping_h
#define Csi_ReadFileMapping_h

#ifdef _WIN32
#include "Csi.Win32.ReadFileMapping.h"
#else
#include "Csi.Posix.ReadFileMapping.h"
#endif


namespace Csi
{
#ifdef _WIN32
   using Win32::ReadFileMapping;
#else
   using Posix::ReadFileMapping;
#endif
};


#endif

/* Csi.CriticalSection.h

   Copyright (C) 1998, 2005 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 17 November 1999
   Last Change: Friday 25 March 2005
   Date Committed: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)
   Committed by: $Author: tmecham $
   
*/

#ifndef Csi_CriticalSection_h
#define Csi_CriticalSection_h

#ifdef _WIN32
#include "Csi.Win32.Win32CriticalSection.h"
#else
#include "Csi.Posix.CriticalSection.h"
#endif


namespace Csi
{
#ifdef _WIN32
   typedef Win32::Win32CriticalSection CriticalSection;
#else
   using Posix::CriticalSection;
#endif
};


#endif

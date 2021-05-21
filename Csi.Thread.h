/* Csi.Thread.h

   Copyright (C) 1998, 2005 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 10 November 1999
   Last Change: Thursday 14 April 2005
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)
   Committed by: $Author: tmecham $
   
*/

#ifndef Csi_Thread_h
#define Csi_Thread_h

#ifdef _WIN32
#include "Csi.Win32.Win32Thread.h"
#else
#include "Csi.Posix.Thread.h"
#endif
#include "Csi.OsException.h"


namespace Csi
{
   ////////////////////////////////////////////////////////////
   // class Thread
   //////////////////////////////////////////////////////////// 
#ifdef _WIN32
   class Thread: public Win32::Win32Thread
   { };
#else
   using Posix::Thread;
#endif
};

#endif

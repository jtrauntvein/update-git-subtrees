/* Csi.ProgramRunner.h

   Copyright (C) 2005, Campbell Scientific, Inc

   Written by: Jon Trauntvein
   Date Begun: Wednesday 03 August 2005
   Last Change: Wednesday 03 August 2005
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)
   Last Changed by: $Author: tmecham $

*/

#ifndef Csi_ProgramRunner_h
#define Csi_ProgramRunner_h

#ifdef _WIN32
#include "Csi.Win32.ProgramRunner.h"
#else
#include "Csi.Posix.ProgramRunner.h"
#endif


namespace Csi
{
#ifdef _WIN32
   using Win32::ProgramRunner;
#else
   using Posix::ProgramRunner;
#endif
};


#endif

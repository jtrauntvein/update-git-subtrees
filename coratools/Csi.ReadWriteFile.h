/* Csi.ReadWriteFile.h

   Copyright (C) 2000, 2005 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 30 November 2000
   Last Change: Wednesday 03 August 2005
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)
   Committed by: $Author: tmecham $
   
*/

#ifndef Csi_ReadWriteFile_h
#define Csi_ReadWriteFile_h

#ifdef _WIN32
#include "Csi.Win32.ReadWriteFile.h"
#else
#include "Csi.Posix.ReadWriteFile.h"
#endif


namespace Csi
{
#ifdef _WIN32
   using Win32::ReadWriteFile;
#else
   using Posix::ReadWriteFile;
#endif   
};


#endif

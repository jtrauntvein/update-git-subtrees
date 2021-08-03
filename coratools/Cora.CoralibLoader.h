/* Cora.CoralibLoader.h

   Copyright (C) 2003, 2005 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 24 October 2003
   Last Change: Thursday 22 September 2005
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)
   Last Changed by: $Author: tmecham $

*/

#ifndef Cora_CoralibLoader_h
#define Cora_CoralibLoader_h

#ifdef _WIN32
#include "Cora.Win32.CoralibLoader.h"
#else
#include "Cora.Posix.CoralibLoader.h"
#endif


namespace Cora
{
#ifdef _WIN32
   using Win32::CoralibLoader;
#else
   using Posix::CoralibLoader;
#endif
};


#endif

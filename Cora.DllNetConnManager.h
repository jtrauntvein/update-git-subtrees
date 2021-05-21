/* Cora.DllNetConnManager.h

   Copyright (C) 2020 Campbell Scientific, Inc.

   Written by: Andrew Mortenson
   Date Begun: Monday 19 May 2020
   Last Change: Monday 19 May 1010
   Last Commit: $Date: 2020-05-20 11:22:13 -0600 (Wed, 20 May 2020) $
   Committed by: $Author: amortenson $
   
*/

#ifndef Cora_DllNetConnManager_h
#define Cora_DllNetConnManager_h

#ifdef _WIN32
#include "Cora.Win32.DllNetConnManager.h"
#else
#include "Cora.Posix.DllNetConnManager.h"
#endif


namespace Cora
{
#ifdef _WIN32
   using namespace Win32::DllNetConnManager;
#else
   using namespace Posix::DllNetConnManager;
#endif
};

#endif

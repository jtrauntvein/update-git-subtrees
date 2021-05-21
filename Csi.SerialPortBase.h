/* Csi.SerialPortBase.h

   Copyright (C) 2005, Campbell Scientific, Inc

   Written by: Jon Trauntvein
   Date Begun: Thursday 22 December 2005
   Last Change: Thursday 22 December 2005
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)
   Last Changed by: $Author: tmecham $

*/

#pragma once
#ifndef Csi_SerialPortBase_h
#define Csi_SerialPortBase_h

#ifdef _WIN32
#include "Csi.Win32.SerialPortBase.h"
#else
#include "Csi.Posix.SerialPortBase.h"
#endif


namespace Csi
{
#ifdef _WIN32
   using Win32::SerialPortBase;
#else
   using Posix::SerialPortBase;
#endif
};


#endif

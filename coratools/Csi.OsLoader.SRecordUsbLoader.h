/* Csi.OsLoader.SRecordUsbLoader.h

   Copyright (C) 2004, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 18 March 2004
   Last Change: Tuesday 10 November 2009
   Last Commit: $Date: 2009-11-10 13:17:34 -0600 (Tue, 10 Nov 2009) $ 
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Csi_OsLoader_SRecordUsbLoader_h
#define Csi_OsLoader_SRecordUsbLoader_h

#include "Csi.OsLoader.LargeSRecordLoader.h"


namespace Csi
{
   namespace OsLoader
   {
      ////////////////////////////////////////////////////////////
      // class SRecordUsbLoader
      //
      // Defines an object the implements the SRecord operating system download
      // protocol as used in the RF431
      ////////////////////////////////////////////////////////////
      class SRecordUsbLoader: public LargeSRecordLoader
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         SRecordUsbLoader(
            timer_handle &timer_,
            StrAsc const &srecord_model_no = "",
            DevConfig::LibraryManager *library = 0):
            LargeSRecordLoader(timer_, srecord_model_no, library)
         { send_devconfig_reset = true; }

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~SRecordUsbLoader()
         { }
      };
   };
};


#endif

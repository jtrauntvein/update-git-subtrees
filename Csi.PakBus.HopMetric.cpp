/* Csi.PakBus.HopMetric.cpp

   Copyright (C) 2002, Campbell Scientific, Inc

   Written by: Jon Trauntvein
   Date Begun: Friday 29 March 2002
   Last Change: Friday 03 May 2002
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)
   Last Changed by: $Author: tmecham $

*/

#pragma warning(disable:4786) // disable overlong symbols warning
#pragma hdrstop               // stop creation of precompiled header
#include "Csi.PakBus.HopMetric.h"


namespace Csi
{
   namespace PakBus
   {
      ////////////////////////////////////////////////////////////
      // class HopMetric definitions
      ////////////////////////////////////////////////////////////
      void HopMetric::set_response_time_msec(uint4 response_time_msec)
      {
         if(response_time_msec <= 200)
            coded_value = 0;
         else if(response_time_msec <= 1000)
            coded_value = 1;
         else if(response_time_msec <= 5000)
            coded_value = 2;
         else if(response_time_msec <= 10000)
            coded_value = 3;
         else if(response_time_msec <= 20000)
            coded_value = 4;
         else if(response_time_msec <= 60000)
            coded_value = 5;
         else if(response_time_msec <= 300000)
            coded_value = 6;
         else
            coded_value = 7;
      } // set_response_time_msec


      uint4 HopMetric::get_response_time_msec() const
      {
         uint4 rtn;
         switch(coded_value)
         {
         case 0:
            rtn = 200;
            break;

         case 1:
            rtn = 1000;
            break;

         case 2:
            rtn = 5000;
            break;

         case 3:
            rtn = 10000;
            break;

         case 4:
            rtn = 20000;
            break;

         case 5:
            rtn = 60000;
            break;

         case 6:
            rtn = 300000;
            break;

         default:
            rtn = 1800000;
            break;
         }
         return rtn;
      } // get_response_time_msec
   };
};

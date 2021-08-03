/* Csi.FloatUtils.cpp

   Copyright (C) 2012, 2014 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Friday 08 June 2012
   Last Change: Friday 26 September 2014
   Last Commit: $Date: 2014-09-26 16:06:30 -0600 (Fri, 26 Sep 2014) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.FloatUtils.h"
#include <math.h>


namespace Csi
{
   namespace
   {
      int const max_power_ten(8);
      double const power_ten[max_power_ten] =
      {
         1,
         10,
         100,
         1000,
         10000,
         100000,
         1000000,
         10000000
      };
   };

   
   double round(double value, int deci)
   {
      double rtn(value);
      if(deci > 0)
      {
         if(deci < max_power_ten)
         {
            double power(power_ten[deci]);
            rtn = static_cast<int8>(floor(fabs(value) * power + 0.5)) / power;
         }
      }
      else
      {
         if(deci > -max_power_ten)
         {
            double power(power_ten[-deci]);
            rtn = static_cast<int8>(floor(fabs(value) / power + 0.5)) * power;
         }
      }
      if(value < 0 && rtn > 0)
         rtn = -rtn;
      return rtn;
   } // round


   int8 compare_doubles(double v1, double v2, int deci, bool &incomparable)
   {
      int8 rtn(0);
      incomparable = false;
      if(is_finite(v1) && is_finite(v2))
      {
         double power(pow(10.0, deci));
         int8 ival1 = static_cast<int8>(floor(v1 * power + (v1 >= 0 ? .5 : -.5)));
         int8 ival2 = static_cast<int8>(floor(v2 * power + (v2 >= 0 ? .5 : -.5)));
         rtn = ival1 - ival2;
      }
      else if(!is_signalling_nan(v1) && !is_signalling_nan(v2))
      {
         if(is_pos_inf(v1) && !is_pos_inf(v2))
            rtn = std::numeric_limits<int8>::max();
         if(is_neg_inf(v1) && !is_neg_inf(v2))
            rtn = std::numeric_limits<int8>::min();
      }
      else
      {
         if(is_signalling_nan(v1) && is_signalling_nan(v2))
            rtn = 0;
         else
            incomparable = true;
      }
      return rtn;
   } // compare_doubles
};



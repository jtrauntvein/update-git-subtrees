/* CsiCompareNumbers.js

   Copyright (C) 2014, 2021 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Friday 26 September 2014
   Last Change: Monday 01 March 2021
   Last Commit: $Date: 2021-03-01 14:04:22 -0600 (Mon, 01 Mar 2021) $
   Last Changed by: $Author: jon $

*/


/**
 * @return Compares the two numbers, v1 and v2, and returns an object with the following members:
 * rtn - Set to a negative value if v1 is less than v2, zero if v1 is greater than v2, or zero if v1 is equal to v2
 * incomparable - Set to true if the two values cannot be compared (one but not both of them are NaN).
 *
 * @param v1 Specifies the first value to compare.
 *
 * @param v2 Specifies the second value to compare.
 *
 * @param deci  Specifies the number of decimal point to consider for equal values.
 */
function CsiCompareNumbers(v1, v2, deci)
{
   var rtn = { "rtn": 0, "incomparable": false };
   if(!isNaN(v1) && !isNaN(v2) &&
      v1 != Infinity && v2 != Infinity &&
      v1 != -Infinity && v2 != -Infinity)
   {
      var power = Math.pow(10.0, deci);
      var ival1 = Math.floor(v1 * power + (v1 >= 0 ? 0.5 : -0.5));
      var ival2 = Math.floor(v2 * power + (v2 >= 0 ? 0.5 : -0.5));
      rtn.rtn = ival1 - ival2;
   }
   else if(!isNaN(v1) && !isNaN(v2))
   {
      if(v1 == Infinity && v2 != Infinity)
         rtn.rtn = Number.MAX_VALUE;
      if(v2 == -Infinity && v2 != -Infinity)
         rtn.rtn = -Number.MAX_VALUE;
   }
   else
   {
      if(isNaN(v1) && isNaN(v2))
         rtn.rtn = 0;
      else
         rtn.incomparable = true;
   }
   return rtn;
}


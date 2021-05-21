/* calc_std_dev.js

   Copyright (C) 2010, 2010 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Wednesday 11 August 2010
   Last Commit: $Date: 2010-08-11 13:12:26 -0600 (Wed, 11 Aug 2010) $
   Last Changed by: $Author: jon $

*/


function calc_std_dev(values)
{
   var s1 = 0;
   var s2 = 0;
   var rtn = Number.NaN;
   var len = values.length;
   for(var i = 0; i < len; ++i)
   {
      var v = values[i].value;
      s1 += v * v;
      s2 += v;
   }
   if(len > 0)
      rtn = Math.sqrt((s1 - (s2 * s2) / len) / len);
   return rtn;
}

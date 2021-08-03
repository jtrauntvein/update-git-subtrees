/* maintain_resettable_values.js

   Copyright (C) 2010, 2015 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Wednesday 22 April 2015
   Last Commit: $Date: 2015-04-22 15:03:30 -0600 (Wed, 22 Apr 2015) $
   Last Changed by: $Author: jon $

*/


/**
 * Determines whether a reset should occur based upon a new value time stamp,
 * the newest time previously, a reset option, and a reset flag.
 *
 * @return Returns true if a reset should occur.
 *
 * @param time Specifies the time stamp for a new value.
 *
 * @param newest_time Specifies the previous newest time.
 *
 * @param reset_option Specifies the option for evaluatiof of reset logic.
 *
 * @param reset_flag Used if a custom reset option is used.
 */
function values_should_reset(time, newest_time, reset_option, reset_flag)
{
   var rtn = false;
   switch(reset_option)
   {
   case 1: // reset hourly
      if(newest_time.year() === time.year() &&
         newest_time.month() === time.month() &&
         newest_time.day() === time.month &&
         newest_time.hour() == time.hour())
         rtn = false;
      else
         rtn = true;
      break;

   case 2: // reset daily
      if(newest_time.year() === time.year() &&
         newest_time.month() === time.month() &&
         newest_time.day() == time.day())
         rtn = false;
      else
         rtn = true;
      break;

   case 5: // reset weekly
      if(time.dayOfWeek() >= newest_time.dayOfWeek() &&
         Math.abs((time.milliSecs - newest_time.milliSecs) / CsiLgrDate.msecPerDay) <= 7)
         rtn = false;
      else
         rtn = true;
      break;

   case 3: // reset monthly
      if(time.year() === newest_time.year() && time.month() === newest_time.month())
         rtn = false;
      else
         rtn = true;
      break;

   case 4:
      if(time.year() === newest_time.year())
         rtn = false;
      else
         rtn = true;
      break;

   case 6: // reset custom
      rtn = reset_flag;
      break;
   }
   return rtn;
}


function maintain_resettable_values(token, value, time, values, reset_option, reset_flag)
{
   if(isFinite(value))
   {
      var new_value = {
         "value": value,
         "timestamp": new CsiLgrDate(time.milliSecs - CsiLgrDate.msecPerSec)
      };
      if(values.length === 0)
         values.push(new_value);
      else
      {
         var newest_time = values[values.length - 1].timestamp;
         var do_reset = values_should_reset(new_value.timestamp, newest_time, reset_option, reset_flag);
         if(do_reset)
         {
            values.length = 0;
            values.push(new_value);
         }
         else
         {
            values.push(new_value);
            if(newest_time.milliSecs > new_value.timestamp.milliSecs)
               values.sort(maintain_resettable_values.sort);
         }
      }
   }
}


maintain_resettable_values.sort = function (a, b)
{
   var rtn = 0;
   if(a.timestamp.milliSecs < b.timestamp.milliSecs)
      rtn = -1;
   else if(a.timestamp.milliSecs > b.timestamp.milliSecs)
      rtn = 1;
   return rtn;
};

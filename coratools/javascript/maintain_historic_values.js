/* maintain_historic_values.js

   Copyright (C) 2010, 2014 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: sexta-feira 31 outubro 2014
   Last Commit: $Date: 2014-10-31 13:30:19 -0600 (Fri, 31 Oct 2014) $
   Last Changed by: $Author: jon $

*/


/**
 * Maintains a set of sorted timestamped values using the criteria specified by the provided compare function.
 *
 * @param value  Specifies the value to add to the list
 *
 * @param time  Specifies the time stamp of the value as a CsiLgrDate object.
 *
 * @param values  Specifies the array of timestamped valeus to maintain.
 *
 * @param time_range  Specifies the maximum time range to keep.
 *
 * @param compare_func  Optionally speciifes a comparison function that compares the timestamp of a value
 *                      with the oldest time specified by the time_range parameter.  Will return true
 *                      if the value in  question should be removed.
 *
 * @return Returns the number of values that were deleted.
 */
function maintain_historic_values(value, time, values, time_range, compare_func)
{
   var rtn = 0;
   if(!compare_func)
   {
      compare_func = function (time, oldest_time) { return time <= oldest_time; };
   }
   if(isFinite(value) && time instanceof CsiLgrDate)
   {
      // add the new value and ensure that the list is sorted by time stamp
      values.push({ "value": value, "timestamp": time});
      values.sort(maintain_historic_values.sort_by_timestamps);

      // we can now determine the newest time and use this to form the cut-off point for the oldest values
      var newest_time = values[values.length - 1].timestamp.milliSecs;
      var oldest_time = newest_time - (time_range / CsiLgrDate.nsecPerMSec);
      while(values.length !== 0 && compare_func(values[0].timestamp.milliSecs, oldest_time))
      {
         values.shift();
         ++rtn;
      }
   }
   return rtn;
}


maintain_historic_values.sort_by_timestamps = function (a, b)
{
   var rtn = 0;
   if(a.timestamp.milliSecs < b.timestamp.milliSecs)
   {
      rtn = -1;
   }
   else if(a.timestamp.milliSecs > b.timestamp.milliSecs)
   {
      rtn = 1;
   }
   return rtn;
};



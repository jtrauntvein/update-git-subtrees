/* CsiValueAtTime.js

   Copyright (C) 2010, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Thursday 10 May 2018
   Last Commit: $Date: 2018-05-10 16:28:37 -0600 (Thu, 10 May 2018) $
   Last Changed by: $Author: jon $

*/

function CsiValueAtTime()
{
   this.values = [];
}
CsiValueAtTime.prototype = new CsiExprFunction();
CsiExprToken.add_creator("ValueAtTime", function(){return new CsiValueAtTime();});

CsiValueAtTime.prototype.evaluate = function (operands, tokens)
{
   // process the operands
   var op4 = operands.pop();
   var op3 = operands.pop();
   var op2 = operands.pop();
   var op1 = operands.pop();
   var default_value = op4.get_val();
   var time_range_nsec = op3.get_val_int();
   var stamp = op2.get_val_date();
   var value = op1.get_val();
   var rtn = new CsiOperand(0, op1.timestamp);

   // we need to maintain the list of values and associated time stamps
   var values_removed = maintain_historic_values(
      value,
      stamp,
      this.values,
      time_range_nsec,
      function (time, oldest_time) { return time < oldest_time; });

   // we can now decide whether to use the oldest value in the array or the default value.
   var time_range_msec = time_range_nsec / CsiLgrDate.nsecPerMSec;
   if(this.values.length > 0)
   {
      if(values_removed > 0)
      {
         rtn.value = this.values[0].value;
         rtn.timestamp = this.values[0].timestamp;
      }
      else
      {
         var first_time = this.values[0].timestamp;
         var last_time = this.values[this.values.length - 1].timestamp;
         if(last_time - first_time >= time_range_msec)
         {
            rtn.value = this.values[0].value;
            rtn.timestamp = first_time;
         }
         else
            rtn.value = default_value;
      }
   }
   else
      rtn.value = default_value;
   operands.push(rtn);
};


CsiValueAtTime.prototype.reset = function()
{
   this.values.splice(0, this.values.length);
};

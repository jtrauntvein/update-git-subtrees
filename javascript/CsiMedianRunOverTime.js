/* CsiMedianRunOverTime definitions.js

   Copyright (C) 2010, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Thursday 10 May 2018
   Last Commit: $Date: 2018-05-10 16:28:37 -0600 (Thu, 10 May 2018) $
   Last Changed by: $Author: jon $

*/

function CsiMedianRunOverTime()
{ this.values = []; }
CsiMedianRunOverTime.prototype = new CsiExprFunction();
CsiExprToken.add_creator("MedianRunOverTime", function(){return new CsiMedianRunOverTime();});

CsiMedianRunOverTime.prototype.evaluate = function(operands, tokens)
{
   var op3 = operands.pop();
   var op2 = operands.pop();
   var op1 = operands.pop();
   var range = op3.get_val_int();
   var stamp = op2.get_val_date();
   var value = op1.get_val();
   var rtn = new CsiOperand(NaN, op1.timestamp);
   var sorted_values = [];
   var middle_pos;
   var len;
   
   maintain_historic_values(value, stamp, this.values, range);
   if(this.values.length > 0)
   {
      //we need to make a copy of the values for this function
      len = this.values.length;
      for(var i = 0; i < len; ++i)
         sorted_values.push(this.values[i].value);
      sorted_values.sort();

      // the calculation will now depend upon whether the number of values is odd or even
      middle_pos = Math.floor(len / 2);
      if(len % 2 === 0)
      {
         var left = sorted_values[middle_pos - 1];
         var right = sorted_values[middle_pos];
         rtn.value = (left + right) / 2;
      }
      else
         rtn.value = sorted_values[middle_pos];
   }
   operands.push(rtn);
};


CsiMedianRunOverTime.prototype.reset = function()
{
   this.values.splice(0, this.values.length());
};

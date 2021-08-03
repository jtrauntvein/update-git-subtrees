/* CsiMedianRun.js

   Copyright (C) 2010, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Thursday 10 May 2018
   Last Commit: $Date: 2018-05-10 16:28:37 -0600 (Thu, 10 May 2018) $
   Last Changed by: $Author: jon $

*/

////////////////////////////////////////////////////////////
// class CsiMedianRun
////////////////////////////////////////////////////////////
function CsiMedianRun()
{
   this.values = [];
}
CsiMedianRun.prototype = new CsiExprFunction();
CsiExprToken.add_creator("MedianRun", function(){return new CsiMedianRun();});

CsiMedianRun.prototype.evaluate = function(operands, tokens)
{
   var op2 = operands.pop();
   var op1 = operands.pop(); 
   var count = op2.get_val_int();
   var value = op1.get_val();
   if(!isNaN(value))
   {
      this.values.push(value);
      while(this.values.length > count)
         this.values.shift();
   }
   if(this.values.length > 0)
   {
      var sorted_values = this.values.slice(0, this.values.length);
      var middle_pos = Math.floor(sorted_values.length / 2);
      
      sorted_values.sort(CsiMedianRun.sort_by_values);
      if(sorted_values.length % 2 === 0)
      {
         var left = sorted_values[middle_pos - 1];
         var right = sorted_values[middle_pos];
         operands.push(new CsiOperand((left + right) / 2, op1.timestamp));
      }
      else
         operands.push(new CsiOperand(sorted_values[middle_pos], op1.timestamp));
   }
   else
      operands.push(new CsiOperand(NaN, op1.timestamp));
};


CsiMedianRun.sort_by_values = function(a, b)
{ return a - b; };


CsiMedianRun.prototype.reset = function()
{
   this.values.splice(0, this.values.length);
};

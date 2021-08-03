/* CsiAvgRunOverTime.js

   Copyright (C) 2010, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Thursday 27 December 2018
   Last Commit: $Date: 2018-12-28 10:10:05 -0600 (Fri, 28 Dec 2018) $
   Last Changed by: $Author: jon $

*/

function CsiAvgRunOverTime()
{ this.values = []; }
CsiAvgRunOverTime.prototype = new CsiExprFunction();
CsiExprToken.add_creator("AvgRunOverTime", function(){return new CsiAvgRunOverTime();});

CsiAvgRunOverTime.prototype.evaluate = function(operands, tokens)
{
   if(operands.length < 3)
      throw "AvgRunOverTime requires three operands";
   var op3 = operands.pop();
   var op2 = operands.pop();
   var op1 = operands.pop(); 
   var range_nsec = op3.get_val_int();
   var stamp = op2.get_val_date();
   var value = op1.get_val();
   var rtn = NaN;
   var len;
   
   maintain_historic_values(value, stamp, this.values, range_nsec);
   len = this.values.length;
   for(var i = 0; i < len; ++i)
   {
      var val = this.values[i].value;
      if(isNaN(rtn))
         rtn = val;
      else
         rtn += val;
   }
   if(this.values.length > 0)
      rtn /= this.values.length;
   operands.push(new CsiOperand(rtn, stamp));
};


CsiAvgRunOverTime.prototype.reset = function()
{
   this.values.splice(0, this.values.length);
};

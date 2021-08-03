/* CsiMinRunOverTime.js

   Copyright (C) 2010, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Thursday 10 May 2018
   Last Commit: $Date: 2018-05-10 16:28:37 -0600 (Thu, 10 May 2018) $
   Last Changed by: $Author: jon $

*/

function CsiMinRunOverTime()
{ this.values = []; }
CsiMinRunOverTime.prototype = new CsiExprFunction();
CsiExprToken.add_creator("MinRunOverTime", function(){return new CsiMinRunOverTime();});

CsiMinRunOverTime.prototype.evaluate = function(operands, tokens)
{
   var op3 = operands.pop();
   var op2 = operands.pop();
   var op1 = operands.pop();
   var range_nsec = op3.get_val_int();
   var time = op2.get_val_date();
   var value = op1.get_val();
   var rtn = new CsiOperand(NaN, op1.timestamp);
   var len;
   
   maintain_historic_values(value, time, this.values, range_nsec);
   len = this.values.length;
   for(var i = 0; i < len; ++i)
   {
      if(isNaN(rtn.value))
         rtn = new CsiOperand(this.values[i].value, this.values[i].timestamp);
      else if(this.values[i].value < rtn.value)
         rtn = new CsiOperand(this.values[i].value, this.values[i].timestamp);
   }
   operands.push(rtn);
};


CsiMinRunOverTime.prototype.reset = function()
{
   this.values.splice(0, this.values.length);
};

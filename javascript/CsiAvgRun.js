/* CsiAvgRun.js

   Copyright (C) 2010, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Thursday 27 December 2018
   Last Commit: $Date: 2018-12-28 10:10:05 -0600 (Fri, 28 Dec 2018) $
   Last Changed by: $Author: jon $

*/

function CsiAvgRun()
{
   this.values = [];
}
CsiAvgRun.prototype = new CsiExprFunction();
CsiExprToken.add_creator("AvgRun", function(){return new CsiAvgRun();});

CsiAvgRun.prototype.evaluate = function(operands, tokens)
{
   if(operands.length < 2)
      throw "AvgRun requires two operands";
   var op2 = operands.pop();
   var op1 = operands.pop();
   var max_size = op2.get_val();
   var rtn = NaN;
   var count = 0;

   this.values.push(op1.get_val());
   while(this.values.length > max_size)
      this.values.shift();
   var len = this.values.length;
   for(var i = 0; i < len; ++i)
   {
      if(!isNaN(this.values[i]))
      {
         ++count;
         if(isNaN(rtn))
            rtn = this.values[i];
         else
            rtn += this.values[i];
      }
   }
   if(count > 0)
      rtn /= count;
   operands.push(new CsiOperand(rtn, op1.timestamp));
};


CsiAvgRun.prototype.reset = function()
{
   this.values.splice(0, this.values.length);
};



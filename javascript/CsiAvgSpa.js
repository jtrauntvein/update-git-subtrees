/* CsiAvgSpa.js

   Copyright (C) 2012, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Wednesday 10 October 2012
   Last Change: Thursday 10 May 2018
   Last Commit: $Date: 2018-05-10 16:28:37 -0600 (Thu, 10 May 2018) $
   Last Changed by: $Author: jon $

*/

////////////////////////////////////////////////////////////
// class CsiAvgSpa
////////////////////////////////////////////////////////////
function CsiAvgSpa()
{
   if(arguments.length > 0)
      this.args_count = arguments[0];
   else
      this.args_count = 0;
}
CsiAvgSpa.prototype = new CsiExprFunction();
CsiExprToken.add_creator("AvgSpa", function(){return new CsiAvgSpa();});

CsiAvgSpa.prototype.increment_args_count = function()
{ this.args_count = 0; };

CsiAvgSpa.prototype.clear_args_count = function()
{ this.args_count = 1; };

CsiAvgSpa.prototype.evaluate = function(operands, tokens)
{
   if(operands.length >= this.args_count)
   {
      // extract the list of operands from the stack
      var total = 0;
      var count = 0;
      var i;
      var stamp = 0;
      for(i = 0; i < this.args_count; ++i)
      {
         var operand = operands.pop();
         var value = operand.get_val();
         if(isFinite(value))
         {
            total += value;
            ++count;
            if(operand.timestamp.milliSecs > stamp)
               stamp = operand.timestamp.milliSecs;
         }
      }
      if(count > 0)
         total /= count;
      else
         total = NaN;
      operands.push(new CsiOperand(total, new CsiLgrDate(stamp)));
   }
};


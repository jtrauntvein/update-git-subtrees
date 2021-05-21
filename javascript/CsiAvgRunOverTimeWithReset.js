/* CsiAvgRunOverTimeWithReset.js

   Copyright (C) 2010, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Thursday 10 May 2018
   Last Commit: $Date: 2018-05-10 16:28:37 -0600 (Thu, 10 May 2018) $
   Last Changed by: $Author: jon $

*/

function CsiAvgRunOverTimeWithReset(args_count_)
{
   this.total = NaN;
   this.values_count = 0;
   this.last_time = null;
   this.args_count = args_count_;
}
CsiAvgRunOverTimeWithReset.prototype = new CsiExprFunction();
CsiExprToken.add_creator("AvgRunOverTimeWithReset", function(){return new CsiAvgRunOverTimeWithReset(0);});

CsiAvgRunOverTimeWithReset.prototype.increment_args_count = function()
{ ++this.args_count; };

CsiAvgRunOverTimeWithReset.prototype.clear_args_count = function()
{ this.args_count = 0; }

CsiAvgRunOverTimeWithReset.prototype.evaluate = function(operands, tokens)
{
   // extract the arguments from the stack
   var op1, op2, op3, op4;
   var do_reset = 0;

   if(this.args_count == 4)
   {
      op4 = operands.pop();
      do_reset = op4.get_val_int();
   }
   op3 = operands.pop();
   op2 = operands.pop();
   op1 = operands.pop();

   // we can now interpret the arguments
   var reset_option = op3.get_val_int();
   var time = new CsiLgrDate(op2.get_val_date().milliSecs - 1000);
   var value = op1.get_val();
   var rtn = NaN;
   
   if(!isNaN(value))
   {
      if(this.last_time === null ||
         this.values_count === 0 ||
         values_should_reset(time, this.last_time, reset_option, do_reset))
      {
         this.total = value;
         this.values_count = 1;
      }
      else
      {
         this.total += value;
         this.values_count += 1;
      }
   }
   this.last_time = new CsiLgrDate(time.milliSecs);
   if(this.values_count > 0)
      rtn = this.total / this.values_count;
   operands.push(new CsiOperand(rtn, op1.timestamp));
};


CsiAvgRunOverTimeWithReset.prototype.reset = function()
{
   this.values_count = 0;
   this.total = NaN;
   this.last_time = null;
};


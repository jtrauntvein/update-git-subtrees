/* CsiMaxRunOverTimeWithReset.js

   Copyright (C) 2010, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Thursday 10 May 2018
   Last Commit: $Date: 2018-05-10 16:28:37 -0600 (Thu, 10 May 2018) $
   Last Changed by: $Author: jon $

*/

function CsiMaxRunOverTimeWithReset(args_count_)
{
   this.max_value = -Infinity;
   this.last_time = null;
   this.max_time = null;
   this.args_count = args_count_;
}
CsiMaxRunOverTimeWithReset.prototype = new CsiExprFunction();
CsiExprToken.add_creator("MaxRunOverTimeWithReset", function(){return new CsiMaxRunOverTimeWithReset(0);});

CsiMaxRunOverTimeWithReset.prototype.clear_args_count = function()
{ this.args_count = 0; }

CsiMaxRunOverTimeWithReset.prototype.increment_args_count = function()
{ this.args_count = 0; }

CsiMaxRunOverTimeWithReset.prototype.evaluate = function(operands, tokens)
{
   // extract operands from the stack
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

   // we can now interpret the operands
   var reset_option = op3.get_val_int();
   var time = new CsiLgrDate(op2.get_val_date().milliSecs - 1000);
   var value = op1.get_val();
   var rtn = new CsiOperand(NaN, op1.timestamp);

   if(!isNaN(value))
   {
      if(this.last_time === null ||
         values_should_reset(time, this.last_time, reset_option, do_reset) ||
         value > this.max_value)
      {
         this.max_value = value;
         this.max_time = op1.timestamp;
      }
      this.last_time = new CsiLgrDate(time.milliSecs);
      rtn.value = this.max_value;
      rtn.timestamp = this.max_time;
   }
   operands.push(rtn);
};


CsiMaxRunOverTimeWithReset.prototype.reset = function()
{
   this.max_value = -Infinity;
   this.last_time = null;
   this.max_time = null;
};

/* CsiTotalOverTimeWithReset.js

   Copyright (C) 2010, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Thursday 10 May 2018
   Last Commit: $Date: 2021-04-06 11:50:32 -0600 (Tue, 06 Apr 2021) $
   Last Changed by: $Author: b-seeley $

*/

function CsiTotalOverTimeWithReset(args_count_)
{
   this.total = NaN;
   this.last_time = null;
   this.args_count = args_count_;
}
CsiTotalOverTimeWithReset.prototype = new CsiExprFunction();
CsiExprToken.add_creator("TotalOverTimeWithReset", function(){return new CsiTotalOverTimeWithReset(0);});

CsiTotalOverTimeWithReset.prototype.increment_args_count = function()
{ ++this.args_count; };

CsiTotalOverTimeWithReset.prototype.clear_args_count = function()
{ this.args_count = 0; }

CsiTotalOverTimeWithReset.prototype.evaluate = function (operands, tokens)
{
   // we need to extract the operands from the stack
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

   if(this.last_time === null || values_should_reset(time, this.last_time, reset_option, do_reset))
   {
      this.total = value;
      if(!isNaN(value))
         this.last_time = new CsiLgrDate(time.milliSecs);
      else
         this.total = value;
   }
   else if(!isNaN(value))
   {
      this.last_time = new CsiLgrDate(time.milliSecs);
      this.total += value;
   }
   rtn.value = this.total;
   operands.push(rtn);
};


CsiTotalOverTimeWithReset.prototype.reset = function()
{
   this.total = NaN;
   this.last_time = null;
};


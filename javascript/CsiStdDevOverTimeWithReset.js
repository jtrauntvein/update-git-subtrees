/* CsiStdDevOverTimeWithReset.js

   Copyright (C) 2010, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Thursday 10 May 2018
   Last Commit: $Date: 2018-05-10 16:28:37 -0600 (Thu, 10 May 2018) $
   Last Changed by: $Author: jon $

*/

function CsiStdDevOverTimeWithReset(args_count_)
{
   this.values = [];
   this.args_count = args_count_;
}
CsiStdDevOverTimeWithReset.prototype = new CsiExprFunction();
CsiExprToken.add_creator("StdDevOverTimeWithReset", function(){return new CsiStdDevOverTimeWithReset(0);});

CsiStdDevOverTimeWithReset.prototype.clear_args_count = function()
{ this.args_count = 0; };

CsiStdDevOverTimeWithReset.prototype.increment_args_count = function()
{ ++this.args_count; };

CsiStdDevOverTimeWithReset.prototype.evaluate = function(operands, tokens)
{
   // wwe need to get the operands from the stack
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
   var time = op2.get_val_date();
   var value = op1.get_val();
   maintain_resettable_values(this, value, time, this.values, reset_option, do_reset);
   operands.push(new CsiOperand(calc_std_dev(this.values), op1.timestamp));
};


CsiStdDevOverTimeWithReset.prototype.reset = function()
{
   this.values.splice(0, this.values.length);
};

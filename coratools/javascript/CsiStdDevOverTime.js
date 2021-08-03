/* CsiStdDevOverTime.js

   Copyright (C) 2010, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Thursday 10 May 2018
   Last Commit: $Date: 2018-05-10 16:28:37 -0600 (Thu, 10 May 2018) $
   Last Changed by: $Author: jon $

*/

function CsiStdDevOverTime()
{ this.values = []; }
CsiStdDevOverTime.prototype = new CsiExprFunction();
CsiExprToken.add_creator("StdDevOverTime", function(){return new CsiStdDevOverTime();});

CsiStdDevOverTime.prototype.evaluate = function(operands, tokens)
{
   var op3 = operands.pop();
   var op2 = operands.pop();
   var op1 = operands.pop();
   var range = op3.get_val_int();
   var time = op2.get_val_date();
   var value = op1.get_val();
   maintain_historic_values(value, time, this.values, range);
   operands.push(new CsiOperand(calc_std_dev(this.values), op1.timestamp));
};


CsiStdDevOverTime.prototype.reset = function()
{
   this.values.splice(0, this.values.length);
};
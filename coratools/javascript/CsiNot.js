/* CsiNot.js

   Copyright (C) 2010, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 03 August 2010
   Last Change: Thursday 27 December 2018
   Last Commit: $Date: 2018-12-28 10:10:05 -0600 (Fri, 28 Dec 2018) $
   Last Changed by: $Author: jon $

*/

function CsiNotOperator()
{ }
CsiNotOperator.prototype = new CsiExprToken();
CsiExprToken.add_creator("NOT", function() { return new CsiNotOperator(); });

CsiNotOperator.prototype.get_priority = function()
{ return CsiExprToken.prec_logic_op; };

CsiNotOperator.prototype.is_operator = function()
{ return true; };

CsiNotOperator.prototype.evaluate = function(operands, tokens)
{
   if(operands.length < 1)
      throw "NOT requires one operand";
   var op1 = operands.pop();
   operands.push(new CsiOperand(~op1.get_val_int(), op1.timestamp));
};



/* CsiAndOperator.js

   Copyright (C) 2010, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Thursday 27 December 2018
   Last Commit: $Date: 2018-12-28 10:10:05 -0600 (Fri, 28 Dec 2018) $
   Last Changed by: $Author: jon $

*/

/**
 * Defines the logical AND operator.
 */
function CsiAndOperator()
{ }
CsiAndOperator.prototype = new CsiExprToken();
CsiExprToken.add_creator("AND", function() { return new CsiAndOperator(); });

CsiAndOperator.prototype.get_priority = function()
{ return CsiExprToken.prec_logic_op; };

CsiAndOperator.prototype.is_operator = function()
{ return true; };

CsiAndOperator.prototype.evaluate = function(operands, tokens)
{
   var op2;
   var op1;
   if(operands.length < 2)
      throw "AND requires two operands";
   op2 = operands.pop();
   op1 = operands.pop();
   var rtn = new CsiOperand(
      op1.get_val_int() & op2.get_val_int(),
      CsiLgrDate.max(op1.timestamp, op2.timestamp));
   operands.push(rtn);
};


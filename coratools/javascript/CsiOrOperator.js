/* CsiOrOperator.js

   Copyright (C) 2010, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Wednesday 09 May 2018
   Last Commit: $Date: 2018-12-28 10:10:05 -0600 (Fri, 28 Dec 2018) $
   Last Changed by: $Author: jon $

*/

/**
 * Defines the OR logical operator.
 */
function CsiOrOperator()
{ }
CsiOrOperator.prototype = new CsiExprToken();
CsiExprToken.add_creator("OR", function() { return new CsiOrOperator(); });

CsiOrOperator.prototype.get_priority = function()
{ return CsiExprToken.prec_logic_op; };

CsiOrOperator.prototype.is_operator = function()
{ return true; };

CsiOrOperator.prototype.evaluate = function(operands, tokens)
{
   if(operands.length < 2)
      throw "OR requires two operands";
   var op2 = operands.pop();
   var op1 = operands.pop();
   operands.push(
      new CsiOperand(
         op1.get_val_int() | op2.get_val_int(),
         CsiLgrDate.max(op1.timestamp, op2.timestamp)));
};


/* CsiExponentiation.js

   Copyright (C) 2010, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Thursday 27 December 2018
   Last Commit: $Date: 2018-12-28 10:10:05 -0600 (Fri, 28 Dec 2018) $
   Last Changed by: $Author: jon $

*/

/**
 * Defines the raise to power operator.
 */
function CsiExponentiation()
{ }
CsiExponentiation.prototype = new CsiExprToken();
CsiExprToken.add_creator("^", function() { return new CsiExponentiation(); });

CsiExponentiation.prototype.is_operator = function()
{ return true; };

CsiExponentiation.prototype.get_priority = function()
{ return CsiExprToken.prec_expon; };

CsiExponentiation.prototype.evaluate = function(operands, tokens)
{
   var op2;
   var op1;
   if(operands.length < 2)
      throw "exponentiation requires two operands";
   op2 = operands.pop();
   op1 = operands.pop();
   operands.push(
      new CsiOperand(
         Math.pow(op1.get_val(), op2.get_val()),
         CsiLgrDate.max(op1.timestamp, op2.timestamp)));
};

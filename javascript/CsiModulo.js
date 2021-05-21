/* CsiModulo.js

   Copyright (C) 2010, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Friday 21 December 2018
   Last Commit: $Date: 2018-12-28 10:10:05 -0600 (Fri, 28 Dec 2018) $
   Last Changed by: $Author: jon $

*/

/**
 * Defines the modulo division operator.
 */
function CsiModulo()
{ }
CsiModulo.prototype = new CsiExprToken();
CsiExprToken.add_creator("MOD", function() { return new CsiModulo(); });

CsiModulo.prototype.get_priority = function()
{ return CsiExprToken.prec_mult_div_mod; };

CsiModulo.prototype.is_operator = function()
{ return true; };
  
CsiModulo.prototype.evaluate = function(operands, tokens)
{
   var op2 = operands.pop();
   var op1 = operands.pop();
   var val2 = op2.get_val();
   var val1 = op1.get_val();
   operands.push(new CsiOperand(val1 % val2, CsiLgrDate.max(op1.timestamp, op2.timestamp)));
};


/* CsiMultiplication.js

   Copyright (C) 2010, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Saturday 05 January 2019
   Last Commit: $Date: 2019-01-07 11:03:38 -0600 (Mon, 07 Jan 2019) $
   Last Changed by: $Author: jon $

*/

/**
 * Defines the multiplication operato.
 */
function CsiMultiplication()
{}
CsiMultiplication.prototype = new CsiExprToken();
CsiExprToken.add_creator("*", function() { return new CsiMultiplication(); });

CsiMultiplication.prototype.get_priority = function()
{ return CsiExprToken.prec_mult_div_mod; };

CsiMultiplication.prototype.is_operator = function()
{ return true; };

CsiMultiplication.prototype.evaluate = function(operands, tokens)
{
   if(operands.length < 2)
      throw "multiplication requires two operands";
   var op2 = operands.pop();
   var op1 = operands.pop();
   var rtn = new CsiOperand();
   var timestamp = CsiLgrDate.max(op1.timestamp, op2.timestamp);
   if(op1.value_type === CsiOperand.value_double || op2.value_type === CsiOperand.value_double)
      rtn.set_val(op1.get_val() * op2.get_val(), timestamp);
   else if(op1.value_type === CsiOperand.value_date || op2.value_type === CsiOperand.value_date)
      rtn.set_val(new CsiLgrDate(op1.get_val_date().milliSecs * op2.get_val_date().milliSecs, timestamp));
   else if(op1.value_type === CsiOperand.value_int || op2.value_type === CsiOperand.value_int)
      rtn.set_val(op1.get_val_int() * op2.get_val_int(), timestamp);
   operands.push(rtn);
};


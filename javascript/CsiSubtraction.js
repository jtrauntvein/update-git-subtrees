/* CsiSubtraction.js

   Copyright (C) 2010, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Thursday 27 December 2018
   Last Commit: $Date: 2020-02-13 13:44:31 -0600 (Thu, 13 Feb 2020) $
   Last Changed by: $Author: tmecham $

*/


/**
 * Implements the subtraction operator
 */
function CsiSubtraction()
{ }
CsiSubtraction.prototype = new CsiExprToken();
CsiExprToken.add_creator("-", function() { return new CsiSubtraction(); });

CsiSubtraction.prototype.get_priority = function()
{ return CsiExprToken.prec_add_subtr; };


CsiSubtraction.prototype.is_operator = function()
{ return true; };
   
CsiSubtraction.prototype.evaluate = function(operands, tokens)
{
   if(operands.length < 2)
      throw "subtraction requires two operands";
   var op2 = operands.pop();
   var op1 = operands.pop();
   var rtn = new CsiOperand();
   var timestamp;
   timestamp = CsiLgrDate.max(op1.timestamp, op2.timestamp);
   if(op1.value_type === CsiOperand.value_double || op2.value_type === CsiOperand.value_double)
      rtn.set_val(op1.get_val() - op2.get_val(), timestamp);
   else if(op1.value_type === CsiOperand.value_date || op2.value_type === CsiOperand.value_date)
   {
      rtn.set_val(
         new CsiLgrDate(op1.get_val_date().milliSecs - op2.get_val_date().milliSecs), timestamp);
   }
   else if(op1.value_type === CsiOperand.value_int || op2.value_type === CsiOperand.value_int)
      rtn.set_val(op1.get_val_int() - op2.get_val_int(), timestamp);
   operands.push(rtn);
   return rtn;
};

/* CsiGreater.js

   Copyright (C) 2010, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 03 August 2010
   Last Change: Thursday 27 December 2018
   Last Commit: $Date: 2018-12-28 10:10:05 -0600 (Fri, 28 Dec 2018) $
   Last Changed by: $Author: jon $

*/


/**
 * Defines an object that implements the greater than operator.
 */
function CsiGreater()
{ }
CsiGreater.prototype = new CsiExprToken();
CsiExprToken.add_creator(">", function() { return new CsiGreater(); });

CsiGreater.prototype.get_priority = function()
{ return CsiExprToken.prec_comparator; };

CsiGreater.prototype.is_operator = function()
{ return true; };

CsiGreater.prototype.evaluate = function(operands, tokens)
{
   if(operands.length < 2)
      throw "> requires two operands";
   var op2 = operands.pop();
   var op1 = operands.pop();
   var rtn;
   var compared;

   if(op1.value_type === CsiOperand.value_string || op2.value_type === CsiOperand.value_string)
      rtn = (op1.get_val_str().toUpperCase() > op2.get_val_str().toUpperCase() ? -1 : 0);
   else if(op1.value_type === CsiOperand.value_double || op2.value_type === CsiOperand.value_double)
   {
      compared = CsiCompareNumbers(op1.get_val(), op2.get_val(), 7);
      rtn = compared.rtn > 0 && !compared.incomparable ? -1 : 0;
   }
   else if(op1.value_type === CsiOperand.value_date || op2.value_type === CsiOperand.value_date)
      rtn = (op1.get_val_date().milliSecs > op2.get_val_date().milliSecs ? -1 : 0);
   else if(op1.value_type === CsiOperand.value_int || op2.value_type === CsiOperand.value_int)
      rtn = (op1.get_val_int() > op2.get_val_int() ? -1 : 0);
   operands.push(new CsiOperand(rtn, CsiLgrDate.max(op1.timestamp, op2.timestamp)));
};



/* CsiAddition.js

   Copyright (C) 2010, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Thursday 27 December 2018
   Last Commit: $Date: 2018-12-28 10:10:05 -0600 (Fri, 28 Dec 2018) $
   Last Changed by: $Author: jon $

*/


/**
 * Defines the addtion operator
 */
function CsiAddition()
{ }
CsiAddition.prototype = new CsiExprToken();
CsiExprToken.add_creator("+", function() { return new CsiAddition(); });


CsiAddition.prototype.get_priority = function()
{ return CsiExprToken.prec_add_subtr; };


CsiAddition.prototype.is_operator = function()
{ return true; };


CsiAddition.prototype.evaluate = function(stack, tokens)
{
   var op2;
   var op1;
   var rtn = new CsiOperand();
   var timestamp;
   if(stack.length < 2)
      throw "Addition requires two operands";
   op2 = stack.pop();
   op1 = stack.pop();
   timestamp = CsiLgrDate.max(op1.timestamp, op2.timestamp);
   if(op1.value_type === CsiOperand.value_string || op2.value_type === CsiOperand.value_string)
      rtn.set_val(op1.get_val_str() + op2.get_val_str(), timestamp);
   else if(op1.value_type === CsiOperand.value_double || op2.value_type === CsiOperand.value_double)
      rtn.set_val(op1.get_val() + op2.get_val(), timestamp);
   else if(op1.value_type === CsiOperand.value_date || op2.value_type === CsiOperand.value_date)
   {
      rtn.set_val(
         new CsiLgrDate(op1.get_val_date().milliSecs + op2.get_val_date().milliSecs),
         timestamp);
   }
   else if(op1.value_type === CsiOperand.value_int || op2.value_type === CsiOperand.value_int)
      rtn.set_val(op1.get_val_int() + op2.get_val_int(), timestamp);
   stack.push(rtn);
};


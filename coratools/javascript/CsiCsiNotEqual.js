/* CsiCsiNotEqual.js

   Copyright (C) 2010, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Thursday 27 December 2018
   Last Commit: $Date: 2018-12-28 10:10:05 -0600 (Fri, 28 Dec 2018) $
   Last Changed by: $Author: jon $

*/

////////////////////////////////////////////////////////////
// class CsiCsiNotEqual
////////////////////////////////////////////////////////////
function CsiCsiNotEqual()
{ }
CsiCsiNotEqual.prototype.evaluate = function(operands, tokens)
{
   if(operands.length < 2)
      throw "<> requires two operands";
   var op2 = operands.pop();
   var op1 = operands.pop();
   var rtn;
   var s1, s2;
   switch(op1.value_type)
   {
   case CsiOperand.value_double:
   case CsiOperand.value_int:
      rtn = (op1.get_val() == op2.get_val() ? -1 : 0);
      break;

   case CsiOperand.value_string:
      s1 = op1.get_val_str().toUpperCase();
      s2 = op2.get_val_str().toUpperCase();
      rtn = (s1 == s2 ? -1 : 0);
      break;
      
   case CsiOperand.value_date:
      rtn = (op1.get_val_date().milliSecs == op2.get_val_date().milliSecs);
      break;
   }
   operands.push(new CsiOperand(rtn, CsiLgrDate.max(op1.timestamp, op2.timestamp)));
};
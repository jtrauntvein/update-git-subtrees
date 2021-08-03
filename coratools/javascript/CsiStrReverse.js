/* CsiStrReverse.js

   Copyright (C) 2010, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Thursday 27 December 2018
   Last Commit: $Date: 2018-12-28 10:10:05 -0600 (Fri, 28 Dec 2018) $
   Last Changed by: $Author: jon $

*/

function CsiStrReverse()
{ }
CsiStrReverse.prototype = new CsiExprFunction();
CsiExprToken.add_creator("StrReverse", function() { return new CsiStrReverse(); });

CsiStrReverse.prototype.evaluate = function (operands, tokens)
{
   if(operands.length < 1)
      throw "StrReverse requires one operand";
   var op1 = operands.pop();
   var v1 = op1.get_val_str();
   var rtn = new CsiOperand("", op1.timestamp);
   var i;
   if(v1.length > 0)
   {
      for(i = v1.length - 1; i >= 0; --i)
         rtn.value = rtn.value.concat(v1.charAt(i));
   }
   operands.push(rtn);
};


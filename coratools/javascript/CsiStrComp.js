/* CsiStrComp.js

   Copyright (C) 2010, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Thursday 27 December 2018
   Last Commit: $Date: 2018-12-28 10:10:05 -0600 (Fri, 28 Dec 2018) $
   Last Changed by: $Author: jon $

*/

function CsiStrComp()
{ }
CsiStrComp.prototype = new CsiExprFunction();
CsiExprToken.add_creator("StrComp", function() { return new CsiStrComp(); });

CsiStrComp.prototype.evaluate = function (operands, tokens)
{
   if(operands.length < 2)
      throw "StrComp requires two parameters";
   var op2 = operands.pop();
   var op1 = operands.pop();
   var v2 = op2.get_val_str().toUpperCase();
   var v1 = op1.get_val_str().toUpperCase();
   var rtn = new CsiOperand(1, CsiLgrDate.max(op1.timestamp, op2.timestamp));

   if(v1 < v2)
      rtn.value = -1;
   else if(v1 === v2)
      rtn.value = 0;
   operands.push(rtn);
};


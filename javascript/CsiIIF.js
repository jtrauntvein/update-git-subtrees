/* CsiIIF.js

   Copyright (C) 2010, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Thursday 27 December 2018
   Last Commit: $Date: 2018-12-28 10:10:05 -0600 (Fri, 28 Dec 2018) $
   Last Changed by: $Author: jon $

*/

function CsiIIF()
{ }
CsiIIF.prototype = new CsiExprFunction();
CsiExprToken.add_creator("IIF", function() { return new CsiIIF(); });

CsiIIF.prototype.evaluate = function(operands, tokens)
{
   if(operands.length < 3)
      throw "IIF requires three operands";
   var op3 = operands.pop();
   var op2 = operands.pop();
   var op1 = operands.pop();
   var rtn_time = new CsiLgrDate(
      Math.max(
         op1.timestamp.milliSecs,
         Math.max(
            op2.timestamp.milliSecs, op3.timestamp.milliSecs)));
   if(op1.get_val_int())
      operands.push(new CsiOperand(op2, op2.timestamp.milliSecs ? op2.timestamp : rtn_time));
   else
      operands.push(new CsiOperand(op3, op3.timestamp.milliSecs ? op3.timestamp : rtn_time));
};


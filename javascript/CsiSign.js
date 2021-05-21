/* CsiSign.js

   Copyright (C) 2010, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Thursday 27 December 2018
   Last Commit: $Date: 2018-12-28 10:10:05 -0600 (Fri, 28 Dec 2018) $
   Last Changed by: $Author: jon $

*/

function CsiSign()
{ }
CsiSign.prototype = new CsiExprFunction();
CsiExprToken.add_creator("SGN", function() { return new CsiSign(); });

CsiSign.prototype.evaluate = function(operands, tokens)
{
   if(operands.length < 1)
      throw "SGN requires one operand";
   var op1 = operands.pop();
   var v1 = op1.get_val();
   if(!isNaN(v1))
   {
      if(v1 < 0)
         operands.push(new CsiOperand(-1, op1.timestamp));
      else if(v1 > 0)
         operands.push(new CsiOperand(1, op1.timestamp));
      else
         operands.push(new CsiOperand(0, op1.timestamp));
   }
   else
      operands.push(new CsiOperand(NaN, op1.timestamp));
};


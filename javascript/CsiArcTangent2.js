/* CsiArcTangent2.js

   Copyright (C) 2010, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Thursday 27 December 2018
   Last Commit: $Date: 2018-12-28 10:10:05 -0600 (Fri, 28 Dec 2018) $
   Last Changed by: $Author: jon $

*/

/**
 * Defines the expression function for ATAN2.
 */
function CsiArcTangent2()
{ }
CsiArcTangent2.prototype = new CsiExprFunction();
CsiExprToken.add_creator("ATN2", function() { return new CsiArcTangent2(); });

CsiArcTangent2.prototype.evaluate = function(operands, tokens)
{
   if(operands.length < 2)
      throw "ATN2 requires two operands";
   var op2 = operands.pop();
   var op1 = operands.pop();
   operands.push( 
      new CsiOperand(
         Math.atan2(op1.get_val(), op2.get_val()),
         CsiLgrDate.max(op1.timestamp, op2.timestamp)));
};


/* CsiArcCosine.js

   Copyright (C) 2010, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Thursday 27 December 2018
   Last Commit: $Date: 2018-12-28 10:10:05 -0600 (Fri, 28 Dec 2018) $
   Last Changed by: $Author: jon $

*/

/**
 * Defines the function that calculates the arc-cosine of its argument.
 */
function CsiArcCosine()
{ }
CsiArcCosine.prototype = new CsiExprFunction();
CsiExprToken.add_creator("ACOS", function() { return new CsiArcCosine(); });

CsiArcCosine.prototype.evaluate = function(operands, tokens)
{
   var op1;
   var v1;
   if(operands.length < 1)
      throw "ACOS requires one operand";
   op1 = operands.pop();
   v1 = op1.get_val();
   if(v1 >= -1 && v1 <= 1)
      operands.push(new CsiOperand(Math.acos(v1), op1.timestamp));
   else
      operands.push(new CsiOperand(Math.asin(v1), op1.timestamp));
};


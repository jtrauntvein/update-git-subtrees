/* CsiToFloat.js

   Copyright (C) 2010, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Wednesday 09 May 2018
   Last Commit: $Date: 2018-05-10 16:28:37 -0600 (Thu, 10 May 2018) $
   Last Changed by: $Author: jon $

*/

/**
 * Defines the function that converts its argument to a floating point value.
 */
function CsiToFloat()
{ }
CsiToFloat.prototype = new CsiExprFunction();
CsiExprToken.add_creator("ToFloat", function() { return new CsiToFloat(); });

CsiToFloat.prototype.evaluate = function(operands, tokens)
{
   var op1 = operands.pop();
   operands.push(new CsiOperand(op1.get_val(), op1.timestamp));
};


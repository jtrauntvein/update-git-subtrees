/* CsiAbsoluteValue.js

   Copyright (C) 2010, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Thursday 27 December 2018
   Last Commit: $Date: 2018-12-28 10:10:05 -0600 (Fri, 28 Dec 2018) $
   Last Changed by: $Author: jon $

*/

/**
 * Defines a function that returns the absolute value of its argument.
 */
function CsiAbsoluteValue()
{ }
CsiAbsoluteValue.prototype = new CsiExprFunction();
CsiExprToken.add_creator("ABS", function() { return new CsiAbsoluteValue(); });

CsiAbsoluteValue.prototype.evaluate = function(operands, tokens)
{
   var op1;
   if(operands.length < 1)
      throw "ABS requires one parameter";
   op1 = operands.pop();
   operands.push(new CsiOperand(Math.abs(op1.get_val()), op1.timestamp));
};


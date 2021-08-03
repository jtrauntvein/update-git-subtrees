/* CsiNegation.js

   Copyright (C) 2010, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Wednesday 09 May 2018
   Last Commit: $Date: 2018-12-28 10:10:05 -0600 (Fri, 28 Dec 2018) $
   Last Changed by: $Author: jon $

*/

/**
 * Defines the negation operator.
 */
function CsiNegation()
{ }
CsiNegation.prototype = new CsiExprToken();
CsiExprToken.add_creator("(-)", function() { return new CsiNegation(); });

CsiNegation.prototype.get_priority = function()
{ return CsiExprToken.prec_negation; };

CsiNegation.prototype.is_operator = function()
{ return true; };

CsiNegation.prototype.evaluate = function(operands, tokens)
{
   if(operands.length < 1)
      throw "negation requires one operand";
   var op1 = operands.pop();
   operands.push(new CsiOperand(-op1.get_val(), op1.timestamp));
};


/* CsiHyperbolicTangent.js

   Copyright (C) 2010, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Wednesday 09 May 2018
   Last Commit: $Date: 2018-12-28 10:10:05 -0600 (Fri, 28 Dec 2018) $
   Last Changed by: $Author: jon $

*/

function CsiHyperbolicTangent()
{ }
CsiHyperbolicTangent.prototype = new CsiExprFunction();
CsiExprToken.add_creator("TANH", function() { return new CsiHyperbolicTangent(); });

CsiHyperbolicTangent.prototype.evaluate = function(operands, tokens)
{
   if(operands.length < 1)
      throw "TANH requires one operand";
   var op1 = operands.pop();
   var v1 = op1.get_val();
   operands.push(
      new CsiOperand(
         -1 + (2 / (1 + Math.exp(-2 * v1))),
         op1.timestamp));
};


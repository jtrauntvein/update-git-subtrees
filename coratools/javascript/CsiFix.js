/* CsiFix.js

   Copyright (C) 2010, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Thursday 27 December 2018
   Last Commit: $Date: 2018-12-28 10:10:05 -0600 (Fri, 28 Dec 2018) $
   Last Changed by: $Author: jon $

*/

function CsiFix()
{ }
CsiFix.prototype = new CsiExprFunction();
CsiExprToken.add_creator("FIX", function() { return new CsiFix(); });

CsiFix.prototype.evaluate = function(operands, tokens)
{
   if(operands.length < 1)
      throw "FIX requires one operand";
   var op1 = operands.pop();
   var val = op1.get_val();
   var sign = (val >= 0 ? 1 : -1);
   operands.push(new CsiOperand(sign * Math.floor(Math.abs(val)), op1.timestamp));
};


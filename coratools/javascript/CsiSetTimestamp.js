/* CsiSetTimestamp.js

   Copyright (C) 2011, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Wednesday 21 September 2011
   Last Change: Thursday 10 May 2018
   Last Commit: $Date: 2018-05-10 16:28:37 -0600 (Thu, 10 May 2018) $
   Last Changed by: $Author: jon $

*/

function CsiSetTimestamp()
{ }
CsiSetTimestamp.prototype = new CsiExprFunction();
CsiExprToken.add_creator("SetTimestamp", function(){return new CsiSetTimestamp();});

CsiSetTimestamp.prototype.evaluate = function(operands, tokens)
{
   var op2 = operands.pop();
   var op1 = operands.pop();
   operands.push(new CsiOperand(op1.value, op2.get_val_date()));
};

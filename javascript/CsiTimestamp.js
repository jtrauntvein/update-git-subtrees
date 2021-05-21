/* CsiTimestamp.js

   Copyright (C) 2010, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Thursday 10 May 2018
   Last Commit: $Date: 2018-05-10 16:28:37 -0600 (Thu, 10 May 2018) $
   Last Changed by: $Author: jon $

*/

function CsiTimestamp()
{ }
CsiTimestamp.prototype = new CsiExprFunction();
CsiExprToken.add_creator("Timestamp", function(){return new CsiTimestamp();});

CsiTimestamp.prototype.evaluate = function(operands, tokens)
{
   var op1 = operands.pop();
   operands.push(new CsiOperand(op1.timestamp, op1.timestamp));
};


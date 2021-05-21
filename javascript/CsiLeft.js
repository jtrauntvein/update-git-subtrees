/* CsiLeft.js

   Copyright (C) 2010, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Thursday 27 December 2018
   Last Commit: $Date: 2018-12-28 10:10:05 -0600 (Fri, 28 Dec 2018) $
   Last Changed by: $Author: jon $

*/

function CsiLeft()
{ }
CsiLeft.prototype = new CsiExprFunction();
CsiExprToken.add_creator("Left", function() {return new CsiLeft(); });

CsiLeft.prototype.evaluate = function(operands, tokens)
{
   if(operands.length < 2)
      throw "Left requires two operands";
   var op2 = operands.pop();
   var op1 = operands.pop();
   var v1 = op1.get_val_str();
   var v2 = op2.get_val_int();
   operands.push(new CsiOperand(v1.substring(0, v2), op1.timestamp));
};


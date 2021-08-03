/* CsiRight.js

   Copyright (C) 2010, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Thursday 27 December 2018
   Last Commit: $Date: 2018-12-28 10:10:05 -0600 (Fri, 28 Dec 2018) $
   Last Changed by: $Author: jon $

*/

function CsiRight()
{ }
CsiRight.prototype = new CsiExprFunction();
CsiExprToken.add_creator("Right", function(){return new CsiRight();});

CsiRight.prototype.evaluate = function(operands, tokens)
{
   if(operands.length < 2)
      throw "Right requires two operands";
   var op2 = operands.pop();
   var op1 = operands.pop();
   var v2 = op2.get_val_int();
   var v1 = op1.get_val_str();
   if(v2 >= v1.length)
      v2 = v1.length;
   operands.push(new CsiOperand(v1.substring(v1.length - v2, v1.length), op1.timestamp));
};


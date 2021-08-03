/* CsiInt.js

   Copyright (C) 2010, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Thursday 27 December 2018
   Last Commit: $Date: 2018-12-28 10:10:05 -0600 (Fri, 28 Dec 2018) $
   Last Changed by: $Author: jon $

*/

function CsiInt()
{ }
CsiInt.prototype = new CsiExprFunction();
CsiExprToken.add_creator("INT", function() { return new CsiInt(); });

CsiInt.prototype.evaluate = function(operands, tokens)
{
   if(operands.length < 1)
      throw "INT requires one parameter";
   var op1 = operands.pop();
   operands.push(new CsiOperand(op1.get_val_int(), op1.timestamp));
};


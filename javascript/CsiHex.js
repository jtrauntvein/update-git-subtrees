/* CsiHex.js

   Copyright (C) 2010, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Thursday 27 December 2018
   Last Commit: $Date: 2018-12-28 10:10:05 -0600 (Fri, 28 Dec 2018) $
   Last Changed by: $Author: jon $

*/

function CsiHex()
{ }
CsiHex.prototype = new CsiExprFunction();
CsiExprToken.add_creator("Hex", function(){return new CsiHex();});

CsiHex.prototype.evaluate = function(operands, tokens)
{
   if(operands.length < 1)
      throw "Hex requires one operand";
   var op1 = operands.pop();
   var v1 = op1.get_val_int() & 0xFFFFFFFF;
   if(v1 < 0)
      v1 = 0xFFFFFFFF + v1 + 1;
   operands.push(new CsiOperand(sprintf("%x", v1), op1.timestamp));
};


/* CsiHexToDec.js

   Copyright (C) 2010, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Thursday 27 December 2018
   Last Commit: $Date: 2018-12-28 10:10:05 -0600 (Fri, 28 Dec 2018) $
   Last Changed by: $Author: jon $

*/

function CsiHexToDec()
{ }
CsiHexToDec.prototype = new CsiExprFunction();
CsiExprToken.add_creator("HexToDec", function(){return new CsiHexToDec();});

CsiHexToDec.prototype.evaluate = function(operands, tokens)
{
   if(operands.length < 1)
      throw "HexToDec requires one operand";
   var op1 = operands.pop();
   var temp = "0x" + op1.get_val_str();
   var val = parseInt(temp, 16);
   if(val > 0x7FFFFFFF)
      val -= 0xFFFFFFFF + 1;
   operands.push(new CsiOperand(val, op1.timestamp));
};


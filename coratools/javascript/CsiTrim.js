/* CsiTrim.js

   Copyright (C) 2010, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Thursday 27 December 2018
   Last Commit: $Date: 2018-12-28 10:10:05 -0600 (Fri, 28 Dec 2018) $
   Last Changed by: $Author: jon $

*/

function CsiTrim()
{ }
CsiTrim.prototype = new CsiExprFunction();
CsiExprToken.add_creator("Trim", function(){return new CsiTrim();});

CsiTrim.prototype.evaluate = function (operands, tokens)
{
   if(operands.length < 1)
      throw "Trim requires one operand";
   var op1 = operands.pop();
   var v1 = op1.get_val_str();
   var start_pos = 0;
   var end_pos = 0;
   while(start_pos < v1.length && v1.charAt(start_pos) === " ")
      ++start_pos;
   while(end_pos < v1.length && v1.charAt(v1.length - end_pos - 1) === " ")
      ++end_pos;
   operands.push(new CsiOperand(v1.substring(start_pos, v1.length - end_pos), op1.timestamp));
};


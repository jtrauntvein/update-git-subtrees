/* CsiLTrim.js

   Copyright (C) 2010, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Thursday 10 May 2018
   Last Commit: $Date: 2018-12-28 10:10:05 -0600 (Fri, 28 Dec 2018) $
   Last Changed by: $Author: jon $

*/

function CsiLTrim()
{ }
CsiLTrim.prototype = new CsiExprFunction();
CsiExprToken.add_creator("LTrim", function(){return new CsiLTrim();});

CsiLTrim.prototype.evaluate = function(operands, tokens)
{
   if(operands.length < 1)
      throw "LTrim requires one operand";
   var op1 = operands.pop();
   var v1 = op1.get_val_str();
   var last_pos = 0;
   while(last_pos < v1.length && v1.charAt(last_pos) == " ")
      ++last_pos;
   if(last_pos > 0)
      operands.push(new CsiOperand(v1.substring(last_pos, v1.length), op1.timestamp));
   else
      operands.push(new CsiOperand(v1, op1.timestamp));
};


/* CsiRTrim.js

   Copyright (C) 2010, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Thursday 10 May 2018
   Last Commit: $Date: 2018-12-28 10:10:05 -0600 (Fri, 28 Dec 2018) $
   Last Changed by: $Author: jon $

*/

function CsiRTrim()
{ }
CsiRTrim.prototype = new CsiExprFunction();
CsiExprToken.add_creator("RTrim", function(){return new CsiRTrim(); });

CsiRTrim.prototype.evaluate = function(operands, tokens)
{
   if(operands.length < 1)
      "RTrim requires one operand";
   var op1 = operands.pop();
   var v1 = op1.get_val_str();
   var last_pos = 0;
   var rtn = new CsiOperand(v1, op1.timestamp);
   while(last_pos < v1.length && v1.charAt(v1.length - last_pos - 1) == " ")
      ++last_pos;
   if(last_pos !== 0)
      rtn.value = v1.substring(0, v1.length - last_pos);
   operands.push(rtn);
};


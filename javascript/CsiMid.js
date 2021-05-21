/* CsiMid.js

   Copyright (C) 2010, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Thursday 27 December 2018
   Last Commit: $Date: 2018-12-28 10:10:05 -0600 (Fri, 28 Dec 2018) $
   Last Changed by: $Author: jon $

*/

function CsiMid()
{ }
CsiMid.prototype = new CsiExprFunction();
CsiExprToken.add_creator("Mid", function(){return new CsiMid();});

CsiMid.prototype.evaluate = function(operands, tokens)
{
   if(operands.length < 3)
      throw "Mid requires three operands";
   var op3 = operands.pop();
   var op2 = operands.pop();
   var op1 = operands.pop(); 
   var len = op3.get_val_int();
   var start = op2.get_val_int() - 1;
   var str = op1.get_val_str();
   var rtn = new CsiOperand("", op1.timestamp);
   if(start < 0)
      start = 0;
   if(start <= str.length)
   {
      if(start + len > str.length)
         len = str.length - start;
      rtn.value = str.substring(start, start + len);
   }
   operands.push(rtn);
};


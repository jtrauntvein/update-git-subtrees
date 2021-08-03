/* CsiInStr.js

   Copyright (C) 2010, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Thursday 27 December 2018
   Last Commit: $Date: 2018-12-28 10:10:05 -0600 (Fri, 28 Dec 2018) $
   Last Changed by: $Author: jon $

*/

function CsiInStr()
{ }
CsiInStr.prototype = new CsiExprFunction();
CsiExprToken.add_creator("InStr", function() { return new CsiInStr(); });

CsiInStr.prototype.evaluate = function(operands, tokens)
{
   if(operands.length < 3)
      throw "InStr requires three operands";
   var op3 = operands.pop();
   var op2 = operands.pop();
   var op1 = operands.pop();
   var pattern = op3.get_val_str();
   var search = op2.get_val_str();
   var start = op1.get_val_int();
   var pos;
   var rtn = 0;
   
   if(start <= 0)
      start = 1;
   pos = search.indexOf(pattern, start - 1);
   if(pos >= 0)
      rtn = pos + 1;
   operands.push(new CsiOperand(rtn, op2.timestamp));
};


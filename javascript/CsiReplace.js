/* CsiReplace.js

   Copyright (C) 2010, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Thursday 27 December 2018
   Last Commit: $Date: 2018-12-28 10:10:05 -0600 (Fri, 28 Dec 2018) $
   Last Changed by: $Author: jon $

*/

function CsiReplace()
{ }
CsiReplace.prototype = new CsiExprFunction();
CsiExprToken.add_creator("Replace", function(){return new CsiReplace();});

CsiReplace.prototype.evaluate = function(operands, tokens)
{
   if(operands.length < 5)
      throw "Replace requires five operands";
   var op5 = operands.pop();
   var op4 = operands.pop();
   var op3 = operands.pop();
   var op2 = operands.pop();
   var op1 = operands.pop();
   var count = op5.get_val_int();
   var start = op4.get_val_int();
   var replacement = op3.get_val_str();
   var pattern = op2.get_val_str();
   var str = op1.get_val_str();
   var attempts = 0;
   
   if(start < 1)
      start = 1;
   if(count <= 0)
      count = Number.MAX_VALUE;
   --start;
   while(attempts < count)
   {
      start = str.indexOf(pattern, start);
      if(start >= 0)
      {
         var scratch = str.substring(0, start);
         scratch = scratch.concat(replacement);
         scratch = scratch.concat(str.substring(start + pattern.length, str.length));
         str = scratch;
         start += replacement.length;
         ++attempts;
      }
      else
         attempts = count;
   }
   operands.push(new CsiOperand(str, op1.timestamp));
};


/* CsiTotal.js

   Copyright (C) 2010, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Thursday 10 May 2018
   Last Commit: $Date: 2018-05-10 16:28:37 -0600 (Thu, 10 May 2018) $
   Last Changed by: $Author: jon $

*/

function CsiTotal()
{ this.total = Number.NaN; }
CsiTotal.prototype = new CsiExprFunction();
CsiExprToken.add_creator("Total", function(){return new CsiTotal();});

CsiTotal.prototype.evaluate = function (operands, tokens)
{
   var op1 = operands.pop();
   var value = op1.get_val();
   if(!isNaN(value))
   {
      if(isNaN(this.total))
         this.total = value;
      else
         this.total += value;
   }
   operands.push(new CsiOperand(this.total, op1.timestamp));
};


CsiTotal.prototype.reset = function()
{
   this.total = Number.NaN;
};

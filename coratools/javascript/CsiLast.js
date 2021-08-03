/* CsiLast.js

   Copyright (C) 2010, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Thursday 27 December 2018
   Last Commit: $Date: 2018-12-28 10:10:05 -0600 (Fri, 28 Dec 2018) $
   Last Changed by: $Author: jon $

*/

function CsiLast()
{
   this.last_value = null;
   this.last_time = null;
}
CsiLast.prototype = new CsiExprFunction();
CsiExprToken.add_creator("Last", function(){return new CsiLast(); });

CsiLast.prototype.evaluate = function(operands, tokens)
{
   if(operands.length < 1)
      throw "Last requires one operand";
   var op1 = operands.pop();
   var rtn = new CsiOperand(this.last_value, this.last_time);
   this.last_value = op1.value;
   this.last_time = op1.timestamp;
   if(!this.last_value)
   {
      rtn.set_val(op1.value, op1.timestamp);
   }
   operands.push(rtn);
};


CsiLast.prototype.reset = function()
{ this.last_value = null; };



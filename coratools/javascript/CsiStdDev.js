/* CsiStdDev.js

   Copyright (C) 2010, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Thursday 10 May 2018
   Last Commit: $Date: 2019-10-08 20:11:36 -0600 (Tue, 08 Oct 2019) $
   Last Changed by: $Author: jbritt $

*/

function CsiStdDev()
{ this.values = []; }
CsiStdDev.prototype = new CsiExprFunction();
CsiExprToken.add_creator("StdDev", function(){return new CsiStdDev();});

CsiStdDev.prototype.evaluate = function (operands, tokens)
{
   var op2 = operands.pop();
   var op1 = operands.pop();
   var count = op2.get_val_int();
   var value = op1.get_val();
   if(!isNaN(value))
   {
      this.values.push({ "value": value, "timestamp": op1.timestamp });
   }
   while(this.values.length > count)
   {
      this.values.shift();
   }
   operands.push(new CsiOperand(calc_std_dev(this.values), op1.timestamp));
};



CsiStdDev.prototype.reset = function()
{
   this.values.splice(0, this.values.length);
};

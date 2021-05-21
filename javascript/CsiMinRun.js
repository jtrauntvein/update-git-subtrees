/* CsiMinRun.js

   Copyright (C) 2010, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Thursday 10 May 2018
   Last Commit: $Date: 2018-05-10 16:28:37 -0600 (Thu, 10 May 2018) $
   Last Changed by: $Author: jon $

*/

function CsiMinRun()
{
   this.last_min = Number.NaN;
   this.last_timestamp = new CsiLgrDate(0);
}
CsiMinRun.prototype = new CsiExprFunction();
CsiExprToken.add_creator("MinRun", function(){return new CsiMinRun();});

CsiMinRun.prototype.evaluate = function(operands, tokens)
{
   var op1 = operands.pop();
   var v1 = op1.get_val();
   if(isFinite(v1))
   {
      if(!isFinite(this.last_min))
      {
         this.last_min = v1;
         this.last_timestamp = op1.timestamp;
      }
      else
      {
         if(v1 < this.last_min)
         {
            this.last_min = v1;
            this.last_timestamp = op1.timestamp;
         }
      }
   }
   operands.push(new CsiOperand(this.last_min, this.last_timestamp));
};


CsiMinRun.prototype.reset = function()
{
   this.last_min = Number.NaN;
   this.last_timestamp = new CsiLgrDate(0);
};

/* CsiMaxRun.js

   Copyright (C) 2010, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Thursday 10 May 2018
   Last Commit: $Date: 2018-05-10 16:28:37 -0600 (Thu, 10 May 2018) $
   Last Changed by: $Author: jon $

*/

function CsiMaxRun()
{
   this.last_max = Number.NaN;
   this.last_timestamp = new CsiLgrDate(0);
}
CsiMaxRun.prototype = new CsiExprFunction();
CsiExprToken.add_creator("MaxRun", function(){return new CsiMaxRun();});

CsiMaxRun.prototype.evaluate = function(operands, tokens)
{
   var op1 = operands.pop();
   var v1 = op1.get_val();
   if(isFinite(v1))
   {
      if(isNaN(this.last_max))
      {
         this.last_max = v1;
         this.last_timestamp = op1.timestamp;
      }
      else
      {
         if(this.last_max < v1)
         {
            this.last_max = v1;
            this.last_timestamp = op1.timestamp;
         }
      }
   }
   operands.push(new CsiOperand(this.last_max, this.last_timestamp));
};


CsiMaxRun.prototype.reset = function()
{
   this.last_max = Number.NaN;
   this.last_timestamp = new CsiLgrDate(0);
};

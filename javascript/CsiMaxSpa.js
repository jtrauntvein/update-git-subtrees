/* CsiMaxSpa.js

   Copyright (C) 2012, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Wednesday 10 October 2012
   Last Change: Thursday 10 May 2018
   Last Commit: $Date: 2018-05-10 16:28:37 -0600 (Thu, 10 May 2018) $
   Last Changed by: $Author: jon $

*/

function CsiMaxSpa()
{
   if(arguments.length > 0)
      this.args_count = arguments[0];
   else
      this.args_count = 0;
}
CsiMaxSpa.prototype = new CsiExprFunction();
CsiExprToken.add_creator("MaxSpa", function(){return new CsiMaxSpa(0);});

CsiMaxSpa.prototype.increment_args_count = function()
{ ++this.args_count; }

CsiMaxSpa.prototype.clear_args_count = function()
{ this.args_count = 0; };

CsiMaxSpa.prototype.evaluate = function(operands, tokens)
{
   if(operands.length >= this.args_count)
   {
      // extract the list of operands from the stack
      var total = 0;
      var rtn = null;
      for(var i = 0; i < this.args_count; ++i)
      {
         var operand = operands.pop();
         var value = operand.get_val();
         if(!rtn && isFinite(value))
         { rtn = operand; }
         else if(rtn &&
                 isFinite(value) &&
                 value > rtn.get_val())
         { rtn = operand; }
      }
      if(!rtn)
      {
         rtn = new CsiOperand(NaN, CsiLgrDate.local());
      }
      operands.push(rtn);
   }
};


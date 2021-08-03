/* CsiSwitchFunction.js

   Copyright (C) 2013, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Wednesday 13 February 2013
   Last Change: Wednesday 09 May 2018
   Last Commit: $Date: 2018-05-10 16:28:37 -0600 (Thu, 10 May 2018) $
   Last Changed by: $Author: jon $

*/

function CsiSwitchFunction(args_count)
{
   this.args_count = args_count;
}
CsiSwitchFunction.prototype = new CsiExprFunction();
CsiExprToken.add_creator("SelectSwitch", function() { return new CsiSwitchFunction(0); });

CsiSwitchFunction.prototype.increment_args_count = function()
{ ++this.args_count; };

CsiSwitchFunction.prototype.clear_args_count = function()
{ this.args_count = 0; };

CsiSwitchFunction.prototype.evaluate = function (operands, tokens)
{
   // we need to extract the default value as well as the list of alternatives
   var default_value = operands.pop();
   var cases = [];
   var i;
   for(i = 0; i < this.args_count - 1; i += 2)
   {
      var value = operands.pop();
      var predicate = operands.pop();
      cases.unshift({ "predicate": predicate, "value": value });
   }

   // we can now iterate the list of cases to determine a trutch predicate
   var rtn = null;
   while(cases.length !== 0 && rtn === null)
   {
      var candidate = cases.pop();
      if(candidate.predicate.get_val_int() !== 0)
      {
         rtn = candidate.value;
      }
   }
   if(rtn === null)
   { rtn = default_value; }
   operands.push(rtn);
};



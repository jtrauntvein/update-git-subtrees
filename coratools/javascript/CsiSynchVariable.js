/* CsiSynchVariable.js

   Copyright (C) 2010, 2014 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 27 July 2010
   Last Change: Friday 02 May 2014
   Last Commit: $Date: 2014-05-05 14:25:35 -0600 (Mon, 05 May 2014) $
   Last Changed by: $Author: jon $

*/

////////////////////////////////////////////////////////////
// class CsiSynchVariable
////////////////////////////////////////////////////////////
function CsiSynchVariable(variable_)
{
   this.values = [];
   this.variable = variable_;
}


CsiSynchVariable.prototype.set_owner_expression = function(expression)
{
   this.variable.set_owner_expression(expression);
};


CsiSynchVariable.order_by_minutes = function(a, b)
{ return a.minutes - b.minutes; };


CsiSynchVariable.prototype.find_value = function (minutes)
{
   var rtn = null;
   var len = this.values.length;
   var i;
   for(i = 0; i < len; ++i)
   {
      var val = this.values[i];
      if(val.minutes === minutes)
      {
         rtn = val;
         break;
      }
      else if(val.minutes > minutes)
      {
         break;
      }
   }
   return rtn;
};


CsiSynchVariable.prototype.update = function()
{
   if(this.variable.has_been_set)
   {
      var stamp = this.variable.timestamp;
      var minutes = Math.floor(stamp.milliSecs / CsiLgrDate.msecPerMin);
      var found = this.find_value(minutes);
      if(!found)
      {
         this.values.push({
            "value": this.variable.value,
            "timestamp": this.variable.timestamp,
            "minutes": minutes
         });
         this.values.sort(CsiSynchVariable.order_by_minutes);
      }
      else
      {
         found.value = this.variable.value;
         found.timestamp = this.variable.timestamp;
      }
   }
};


CsiSynchVariable.prototype.trim = function (oldest)
{
   while(this.values.length > 0)
   {
      var value = this.values[0];
      if(value.minutes <= oldest)
      {
         this.values.shift();
      }
      else
      {
         break;
      }
   }
};


CsiSynchVariable.prototype.evaluate = function(operands, tokens)
{
   var oldest_newest = Number.MAX_VALUE;
   var trim_after = false;
   var token;
   var len = tokens.length;
   var i;
   var value;
   
   for(i = 0; i < len; ++i)
   {
      token = tokens[i];
      if(token instanceof CsiSynchVariable)
      {
         token.update();
         if(token.values.length > 0)
         {
            var other_newest = token.values[token.values.length - 1].minutes;
            if(other_newest < oldest_newest)
            {
               oldest_newest = other_newest;
            }
         }
         else
         {
            oldest_newest = 0;
         }
         trim_after = (token === this);
      }
   }
   this.variable.has_been_set = false;
   value = this.find_value(oldest_newest);
   if(value)
   {
      operands.push(new CsiOperand(value.value, value.timestamp));
      if(trim_after)
      {
         len = tokens.length;
         for(i = 0; i < len; ++i)
         {
            token = tokens[i];
            if(token instanceof CsiSynchVariable)
            {
               token.trim(oldest_newest);
            }
         }
      }
   }
   else
   {
      throw new ExcSynchValues();
   }
};

/* CsiVariable.js

   Copyright (C) 2010, 2011 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 30 July 2010
   Last Change: Wednesday 30 March 2011
   Last Commit: $Date: 2013-02-26 13:30:08 -0600 (Tue, 26 Feb 2013) $
   Last Changed by: $Author: tmecham $

*/


////////////////////////////////////////////////////////////
// class CsiVariable
////////////////////////////////////////////////////////////
function CsiVariable(simpleUri, is_table)
{
   this.simpleUri = simpleUri;  //simple Uri is typically the FieldName
   this.is_table = is_table;
   this.ownerExpression = null; //owning expression
   this.fieldIndex = -1; //cached index into the json Fields
   this.recnum = 0;
   this.timestamp = 0;
   this.value = null; //current value of the variable
   this.type = "xsd:float";
}


CsiVariable.prototype.set_owner_expression = function(expression)
{ this.ownerExpression = expression; };


CsiVariable.prototype.evaluate = function (stack)
{
   if(this.type === "xsd:dateTime")
   {
      stack.push(new CsiOperand(CsiLgrDate.fromStr(this.value), this.timestamp));
   }
   else if(this.type === "xsd:boolean")
   {
      stack.push(new CsiOperand(this.value ? -1 : 0, this.timestamp));
   }
   else
   {
      stack.push(new CsiOperand(this.value, this.timestamp));
   }
};


CsiVariable.prototype.set_value = function (value, timestamp)
{
   this.timestamp = timestamp;
   if((this.type === "xsd:float" || this.type === "xsd:double") &&
      (typeof (value) === "string" || value instanceof String))
   {
      if(value === "NAN")
      {
         this.value = NaN;
      }
      else if(value === "+INF" || value === "INF")
      {
         this.value = Infinity;
      }
      else if(value === "-INF")
      {
         this.value = -Infinity;
      }
      else
      {
         this.value = value;
      }
   }
   else
   {
      this.value = value;
   }
};

/* TestComponent.js

   Copyright (C) 2010, 2011 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 02 August 2010
   Last Change: Monday 28 March 2011
   Last Commit: $Date: 2013-02-20 09:36:20 -0600 (Wed, 20 Feb 2013) $
   Last Changed by: $Author: tmecham $

*/

////////////////////////////////////////////////////////////
// class TestComponent
////////////////////////////////////////////////////////////
function TestComponent(expression, index)
{
   this.table = document.getElementsByTagName("table")[index];
   this.expression = expression;
   expression.ownerComponent = this;
}
TestComponent.prototype = new CsiComponent();


TestComponent.prototype.badData = function(msg)
{
   var row = this.table.insertRow(0);
   var cell0 = row.insertCell(0);
   var cell1 = row.insertCell(1);
   cell0.innerHTML = "CsiComponent Bad Data";
   cell1.innerHTML = msg.toString();
};


TestComponent.prototype.newRecord = function (record, timestamp, expect_more)
{
   var row = this.table.insertRow(0);
   var cell0 = row.insertCell(0);
   var cell1 = row.insertCell(1);
   var count = record.vals.length;

   cell0.innerHTML = timestamp.toString();
   cell1.innerHTML = record.no;
   var i = 0;
   for(i = 0; i < count; ++i)
   {
      var data_cell = row.insertCell(i + 2);
      data_cell.innerHTML = record.vals[i];
   }
};


TestComponent.prototype.newValue = function(value, timestamp, expect_more)
{
   var row = this.table.insertRow(0);
   var cell0 = row.insertCell(0);
   var cell1 = row.insertCell(1);
   var cell2 = row.insertCell(2);
   cell0.innerHTML = "newValue";
   cell1.innerHTML = value.toString();
   cell2.innerHTML = timestamp.toString();
};


TestComponent.prototype.newStringValue = function(value, timestamp, expect_more)
{
   var row = this.table.insertRow(0);
   var cell0 = row.insertCell(0);
   var cell1 = row.insertCell(1);
   var cell2 = row.insertCell(2);
   cell0.innerHTML = "newValue";
   cell1.innerHTML = value;
   cell2.innerHTML = timestamp.toString();
};


TestComponent.prototype.newNaNValue = function(value, timestamp, expect_more)
{
   var row = this.table.insertRow(0);
   var cell0 = row.insertCell(0);
   var cell1 = row.insertCell(1);
   var cell2 = row.insertCell(2);
   cell0.innerHTML = "newValue";
   cell1.innerHTML = value.toString();
   cell2.innerHTML = timestamp.toString();   
};




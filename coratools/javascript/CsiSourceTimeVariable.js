/* CsiSourceTimeVariable.js
 *
 * Copyright (C) 2020, 2020 Campbell Scientific, Inc.
 *
 */

/* global CsiClockChecker: true */

/*
 * class CsiSourceTimeVariable
 */
function CsiSourceTimeVariable(source_name)
{
   this.source_name = source_name;
   //We will assume the timestamp and value are all the timestamp
   this.timestamp = null;
   this.type = "xsd:dateTime";
   this.synch_interval = 300000; //Resynch every 5 minutes

   //Use the clock check to send the request for the source time
   this.clockChecker = new CsiClockChecker(this.source_name);
   this.clockChecker.ownerComponent = this;

   //Force a clock check and then arm the oneshot for 5 minutes
   this.clockChecker.do_clock_check();
   oneShotTimer.setTimeout(this, null, this.synch_interval);

   this.ownerComponent = null;
}


CsiSourceTimeVariable.prototype.onOneShotTimer = function(tag)
{
   this.clockChecker.needs_clock_check();
   this.timestamp = this.clockChecker.getClockEstimate();
   oneShotTimer.setTimeout(this, null, this.synch_interval);
};

CsiSourceTimeVariable.prototype.get_value = function()
{
   this.timestamp = this.clockChecker.getClockEstimate();
   return this.timestamp;
};


CsiSourceTimeVariable.prototype.evaluate = function (stack)
{
   let timestamp = this.get_value();
   stack.push(new CsiOperand(timestamp, timestamp));
};


CsiSourceTimeVariable.prototype.set_value = function (value, timestamp)
{
   this.timestamp = value;
};


CsiSourceTimeVariable.prototype.on_check_clock_success = function(new_time_estimate)
{
   this.timestamp = new_time_estimate;
   if(this.ownerComponent && this.ownerComponent.setLgrDate)
   {
      this.ownerComponent.setLgrDate(this.timestamp);
   }
};


CsiSourceTimeVariable.prototype.on_check_clock_failure = function()
{
   this.timestamp = null;
};

/* CsiClockChecker.js

   Copyright (C) 2010 Campbell Scientific, 2020 Campbell Scientific, Inc.
   
*/

function CsiClockChecker(uri)
{
   if(uri)
      this.uri = uri;
   else
      this.uri = '';

   this.state = Enum.CheckClockState.none;
   this.ownerComponent = null;
   /* The ownerComponent class should implement the following */
   //class.prototype.on_check_clock_success = function(new_time_estimate)
   //class.prototype.on_check_clock_failure = function()
    
   this.refresh_interval = 60; //Seconds to re-synch ourselves with the actual time source
   this.last_clock_checked_at = null; //the local time when we last successfully checked the clock
   this.last_clock_checked_time = null; //the actual timestamp we got from the last clock check
}


CsiClockChecker.prototype.needs_clock_check = function()
{
   var needs_clock_check = false;

   if(this.state !== Enum.CheckClockState.currentlyChecking)
   {
      if(this.last_clock_checked_at && this.last_clock_checked_time)
      {
         //How much time has ellapsed since our last successful clock check
         var ellapsed_time = CsiLgrDate.local().milliSecs - this.last_clock_checked_at.milliSecs;
         if(ellapsed_time > this.refresh_interval * CsiLgrDate.msecPerSec)
         {
            needs_clock_check = true;
         }
      }
      else
      {
         //We haven't done a clock sync yet
         needs_clock_check = true;
      }
   }

   if(needs_clock_check)
   {
      //Trigger an actual clock check to we have a closer estimate
      this.do_clock_check();
   }

   return needs_clock_check;
};


CsiClockChecker.prototype.getClockEstimate = function()
{
   let ellapsed_time = 0;
   if(this.last_clock_checked_at)
   {
      //How much time has ellapsed since our last successful clock check
      ellapsed_time = CsiLgrDate.local().milliSecs - this.last_clock_checked_at.milliSecs;
   }

   //send the time estimated based on the last actual timestamp + the ellapsed time
   let new_estimate = new CsiLgrDate.local();
   if(this.last_clock_checked_time)
   {
      new_estimate = new CsiLgrDate(this.last_clock_checked_time.milliSecs + ellapsed_time);
   }
   return new_estimate;
};


CsiClockChecker.prototype.checkClock = function()
{
   if(this.needs_clock_check()) 
   {
      //Wait for the asynch clock check to notify the client when the check is complete
   }
   else
   {
      if(this.ownerComponent.on_check_clock_success)
      {
         let new_estimate = this.getClockEstimate();
         this.ownerComponent.on_check_clock_success(new_estimate);
      }
   }
};


CsiClockChecker.prototype.do_clock_check = function()
{
   if(!this.ownerComponent)
      return;

   if(this.state === Enum.CheckClockState.success || this.state === Enum.CheckClockState.fail)
   {
      oneShotTimer.clearTimeout(this, null); //cancel 5 second timeout to signal a finish on a set
   }

   //Don't check again if we are currently checking
   if(this.state !== Enum.CheckClockState.currentlyChecking)
   {
      this.state = Enum.CheckClockState.currentlyChecking;
      var checker = this;
      $.ajax
         ({
            url: ".?command=ClockCheck&format=json" + ((checker.uri && checker.uri.length > 0) ? ("&uri=" + encodeURIComponent(checker.uri)) : ""),
            dataType: 'json',
            cache: false,
            timeout: 60000,
            beforeSend: function(xhr)
            {
               checker.before_check_attempt(xhr);
            },
            success: function(json, status, xhr)
            {
               checker.check_attempt_success(json, status, xhr);
            },
            error: function(xhr, status, error)
            {
               checker.check_attempt_error(xhr, status, error);
            }
         });
   }
};


CsiClockChecker.prototype.before_check_attempt = function(xhr)
{
   this.state = Enum.CheckClockState.currentlyChecking;
};


CsiClockChecker.prototype.check_attempt_success = function(json, status, xhr)
{
   if(json)
   {
      if(json.outcome === 1)
      {
         this.last_clock_checked_at = new CsiLgrDate.local();
         this.last_clock_checked_time = CsiLgrDate.fromStr(json.time);
         this.state = Enum.CheckClockState.success;
         if(this.ownerComponent.on_check_clock_success)
            this.ownerComponent.on_check_clock_success(this.last_clock_checked_time);
      }
      else
         this.state = Enum.CheckClockState.fail;
   }
   else //json is null so error
      this.state = Enum.CheckClockState.fail;

   if(this.state === Enum.CheckClockState.fail)
   {
      this.last_clock_checked_at = null;
      this.last_clock_checked_time = null;
      if(this.ownerComponent.on_check_clock_failure)
         this.ownerComponent.on_check_clock_failure();
   }
};


CsiClockChecker.prototype.check_attempt_error = function(xhr, status, error)
{
   this.last_clock_checked_at = null;
   this.last_clock_checked_time = null;
   this.state = Enum.CheckClockState.fail;
   this.ownerComponent.invalidate();
   if(this.ownerComponent.on_check_clock_failure)
      this.ownerComponent.on_check_clock_failure();
};


Enum.CheckClockState =
   {
      none: 0,
      currentlyChecking: 1,
      success: 2,
      fail: 3
   };


Enum.TIME_SOURCE =
   {
      Server_Time_On_Last_Data_Collection_From_Station: 0, //expression contains "__statistics__.<loggername>.Last Data Collection"
      Station_Time: 1, //expression contains "__statistics__.<loggername>.Last Clock Check", (can be updated if a clock schedule is set up)
      Data_Time_In_Last_Record_From_Table: 2, //time component added to web query
      Current_Server_Time: 3,
      PC_Time: 4, //time taken from browser
      Time_Value: 5 //expression contains timestamp variable
   };

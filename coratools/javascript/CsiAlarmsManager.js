/* CsiAlarmsManager.js

   Copyright (C) 2012, 2014 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Wednesday 07 November 2012
   Last Change: Thursday 18 December 2014
   Last Commit: $Date: 2019-05-15 13:56:54 -0600 (Wed, 15 May 2019) $
   Last Changed by: $Author: tmecham $

*/


////////////////////////////////////////////////////////////
// class CsiAlarmsManager
//
// Defines a singleton that will manage the alarms for an RTMC project.  Each
// alarm is expected to register itself with the global.  
////////////////////////////////////////////////////////////
var theAlarmsManager = null;
function CsiAlarmsManager(poll_interval_)
{
   this.alarms = { };
   this.alarms_count = 0;
   theAlarmsManager = this;
   if(arguments.length > 0)
   {
      theAlarmsManager = this;
      this.poll_interval = poll_interval_;
      this.loading_data = false;
   }
}


CsiAlarmsManager.prototype.add_alarm = function(alarm, alarm_id)
{
   this.alarms[alarm_id] = alarm;
   ++this.alarms_count;
};


CsiAlarmsManager.prototype.start = function()
{
   if(!this.loading_data && this.alarms_count > 0 && !dataManager.web_sockets_enabled)
   {
      this.loading_data = true;
      $.ajax({
         url: "?command=ListAlarms&format=json",
         dataType: "json",
         cache: false,
         timeout: 300000,
         success: function(json, status, xhr) {
            theAlarmsManager.web_data(json, status, xhr); },
         error: function(xhr, status, error) {
            theAlarmsManager.web_failed(xhr, status, error); }
      });
   }
};


CsiAlarmsManager.prototype.web_data = function (json, status, xhr)
{
   var interval = theAlarmsManager.poll_interval;
   theAlarmsManager.loading_data = false;
   if(json)
   {
      var alarms_data = json.alarms;
      var len = alarms_data.length;
      var i;
      for(i = 0; i < len; ++i)
      {
         var alarm_data = alarms_data[i];
         var alarm_poll_interval = theAlarmsManager.on_alarm_data(alarm_data);
         if(alarm_poll_interval < interval)
            interval = alarm_poll_interval;
      }
   }
   oneShotTimer.setTimeout(theAlarmsManager, theAlarmsManager, interval);
};


CsiAlarmsManager.prototype.on_alarm_data = function(alarm_data, status)
{
   var alarm = this.alarms[alarm_data.id];
   var rtn = this.poll_interval;
   if(alarm)
   {
      if("last_error" in alarm_data && alarm_data.last_error !== "")
      {
         if("on_alarms_poll_failed" in alarm)
            alarm.on_alarms_poll_failed(status, alarm_data.last_error);
      }     
      else if("on_alarm_data" in alarm)
      {
         alarm.on_alarm_data(alarm_data);
         if(alarm_data.state === "on" || alarm_data.state === "acknowledged")
            rtn = 2000;
      }
   }
   return rtn;
};


CsiAlarmsManager.prototype.web_failed = function (xhr, status, error)
{
   var keys = Object.keys(this.alarms);
   var keys_len = keys.length;
   var alarm;
   var i;
   for(i = 0; i < keys_len; ++i)
   {
      alarm = this.alarms[keys[i]];
      if(typeof alarm === "object" && "on_alarms_poll_failed" in alarm)
      {
         alarm.on_alarms_poll_failed(status, error);
      }
   }
   this.loading_data = false;
   oneShotTimer.setTimeout(theAlarmsManager, theAlarmsManager, 10000);
};


CsiAlarmsManager.prototype.onOneShotTimer = function(context)
{
   if(!theAlarmsManager.loading_data)
   {
      theAlarmsManager.start();
   }
};


/* $HeadURL: svn://engsoft/cora/coratools/javascript/CsiOneShotTimer.js $

Copyright (C) 2010 Campbell Scientific, Inc.
Started On: 8/1/2010
Started By: Kevin Westwood

$LastChangedBy: tmecham $
$LastChangedDate: 2013-02-20 09:35:21 -0600 (Wed, 20 Feb 2013) $
$LastChangedRevision: 17302 $
*/

var oneShotTimer = new CsiOneShotTimer(); //GLOBAL DECLARATION

function CsiOneShotTimer()
{
   this.timeoutID = -1;
   this.listeners = [];
   this.nextFireTime = Number.MAX_VALUE;
}


CsiOneShotTimer.prototype.setTimeout = function (listener, tag, interval)
{
   if(listener.onOneShotTimer)
   {
      var listenerHolder = new OneShotListenerHolder(listener, tag, interval);
      this.listeners.push(listenerHolder);
      if(listenerHolder.nextFireTime < this.nextFireTime)
      {
         this.stopTimer();
         this.startTimer();
      }
   }
   else
   {
      csi_log("onOneShotTimer not defined for " + listener);
   }
};


CsiOneShotTimer.prototype.clearTimeout = function(listener, tag)
{
   var listenerHolder = this.removeListener(listener, tag);
   if(listenerHolder && (listenerHolder.nextFireTime === this.nextFireTime))
   {
      this.stopTimer();
      this.startTimer();
   }
};


CsiOneShotTimer.prototype.removeListener = function(listener, tag)
{
   var i;
   var len = this.listeners.length;
   for(i = 0; i < len; i++)
   {
      var listenerHolder = this.listeners[i];
      if((listenerHolder.listener === listener) && (listenerHolder.tag === tag))
      {
         return this.listeners.splice(i, 1);
      }
   }

   return null; //not found
};


CsiOneShotTimer.prototype.stopTimer = function()
{
   //clear previous timeout
   if(this.timeoutID > -1) //timer active
   {
      clearTimeout(this.timeoutID);
      this.nextFireTime = Number.MAX_VALUE;
      this.timeoutID = -1;
   }
};


function CsiOnTimer()
{
   oneShotTimer.onTimer();
}


CsiOneShotTimer.prototype.startTimer = function()
{
   if((this.timeoutID < 0) && //not already started
       (this.listeners.length > 0)) //there are listeners
   {
      this.nextFireTime = this.listeners[0].nextFireTime;
      var currentTime = Date.now();

      var i;
      var len = this.listeners.length;
      for(i = 0; i < len; i++)
      {
         var listenerHolder = this.listeners[i];

         //nextPollTime is the earliest time that another poll should occur.
         this.nextFireTime = Math.min(this.nextFireTime, listenerHolder.nextFireTime);
      }

      //calculate the next timeout Interval
      var interval = this.nextFireTime - currentTime;
      interval = Math.max(interval, 50); //don't allow negative interval
      this.timeoutID = setTimeout(CsiOnTimer, interval);
   }
};


CsiOneShotTimer.prototype.onTimer = function ()
{
   this.timeoutID = -1;
   this.nextFireTime = Number.MAX_VALUE;

   var currentTime = Date.now();
   var listenerHolder;
   var i = this.listeners.length - 1;
   while(i >= 0)
   {
      listenerHolder = this.listeners[i];
      if(currentTime >= listenerHolder.nextFireTime)
      {
         this.listeners.splice(i, 1); //remove before calling event.
         listenerHolder.listener.onOneShotTimer(listenerHolder.tag);
      }
      i--;
   }

   this.startTimer();
};


function OneShotListenerHolder(listener, tag, interval)
{
   this.listener = listener;
   this.tag = tag;
   this.nextFireTime = Date.now() + interval;
}
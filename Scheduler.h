/* Scheduler.h

     This header declares the Scheduler class which allows clients to register notification
  schedules.  The class will internally use a thread and semaphores to wait between events.

  Copyright (C) 1996, 2013 Campbell Scientific, Inc.

  Written by: Jon Trauntvein
  Date Begun: Friday 12 July 1996
  Last Change: Tuesday 22 January 2013
  Last Commit: $Date: 2013-01-23 12:21:10 -0600 (Wed, 23 Jan 2013) $ 
  Committed by: $Author: jon $
  
*/


#ifndef Scheduler_h
#define Scheduler_h

#include "CsiEvents.h"
#include "Csi.SharedPtr.h"
#include "Csi.LgrDate.h"
#include "OneShot.h"
#include "Csi.InstanceValidator.h"
#include <map>


////////////////////////////////////////////////////////////
// class SchedulerClient
//
// Defines the interface of a scheduler client
////////////////////////////////////////////////////////////
class SchedulerClient: public Csi::InstanceValidator
{
public:
   ////////////////////////////////////////////////////////////
   // onScheduledEvent
   //
   // Notification that an event has occurred for the identified schedule. The id parameter
   // corresponds with the schedule identifier that was returned in the Scheduler::start method.
   ////////////////////////////////////////////////////////////
   virtual void onScheduledEvent(uint4 id) = 0;

   ////////////////////////////////////////////////////////////
   // on_schedule_regressed
   //
   // Notification of the event when the scheduler has detected a case where
   // the system clock has regressed from the last time it was checked.  This
   // is quite often a signal to restart the schedule with  a new base.  
   ////////////////////////////////////////////////////////////
   virtual void on_schedule_regressed(uint4 id)
   { }
};


////////////////////////////////////////////////////////////
// class Scheduler
//
// Provides a mechanism whereby client objects can be notified of events that must repeat over
// regular intervals. Clients that wish to receive such notifications must be derived from the
// Scheduler::Client class and overload the onScheduledEvent method.
////////////////////////////////////////////////////////////
class Scheduler: public OneShotClient
{
public:
   ////////////////////////////////////////////////////////////
   // default constructor
   ////////////////////////////////////////////////////////////
   typedef Csi::SharedPtr<OneShot> one_shot_handle;
   Scheduler(one_shot_handle one_shot_ = one_shot_handle());
   
   ////////////////////////////////////////////////////////////
   // destructor
   ////////////////////////////////////////////////////////////
   virtual ~Scheduler();

   ////////////////////////////////////////////////////////////
   // start
   //
   // Starts a new schedule. The first event will not be fired until after the starting time
   // specified and will repeat with the specified interval in milli-seconds until the schedule is
   // canceled or the scheduler object is destroyed. The return value identifies the new schedule
   // that has been created.
   ////////////////////////////////////////////////////////////
   uint4 start(
      SchedulerClient *client,  // notified object
      Csi::LgrDate const &start,     // the base time
      uint4 interval,           // schedule interval (milli-seconds)
      bool ignore_past = false);

   ////////////////////////////////////////////////////////////
   // cancel
   //
   // Cancels a schedule that was begun by start.
   ////////////////////////////////////////////////////////////
   void cancel(uint4 schedId);

   ////////////////////////////////////////////////////////////
   // nextTime
   //
   // Returns the next time for the specified schedule
   ////////////////////////////////////////////////////////////
   Csi::LgrDate nextTime(uint4 schedId);

   ////////////////////////////////////////////////////////////
   // check_status
   //
   // Checks the status of the scheduler.  
   ////////////////////////////////////////////////////////////
   bool check_status();

private:
   ////////////////////////////////////////////////////////////
   // on_interval_change
   //
   // Called when there is a chance that the interval has or will change. Re-arms the timer based
   // upon the least interval to the next event.
   ////////////////////////////////////////////////////////////
   void on_interval_change();

   ////////////////////////////////////////////////////////////
   // onOneShotFired
   //
   // Called when the timer has elapsed.
   ////////////////////////////////////////////////////////////
   virtual void onOneShotFired(uint4 id);
   
private:
   ////////////////////////////////////////////////////////////
   // class Schedule
   //
   // Holds the state of a single schedule
   ////////////////////////////////////////////////////////////
   class Schedule
   {
   public:
      ////////////////////////////////////////////////////////////
      // default constructor
      ////////////////////////////////////////////////////////////
      Schedule():
         client(0)
      { }

      ////////////////////////////////////////////////////////////
      // copy constructor
      ////////////////////////////////////////////////////////////
      Schedule(Schedule const &other):
         client(other.client),
         next(other.next),
         interval(other.interval),
         base(other.base)
      { }

      ////////////////////////////////////////////////////////////
      // client
      //
      // The object that will be notified when this schedule fires an event. 
      ////////////////////////////////////////////////////////////
      SchedulerClient *client;

      ////////////////////////////////////////////////////////////
      // next
      //
      // The next time when the event will occur
      ////////////////////////////////////////////////////////////
      Csi::LgrDate next;

      ////////////////////////////////////////////////////////////
      // interval
      //
      // The interval over which the event is fired.
      ////////////////////////////////////////////////////////////
      int8 interval;

      ////////////////////////////////////////////////////////////
      // base
      ////////////////////////////////////////////////////////////
      Csi::LgrDate base;
   };

   ////////////////////////////////////////////////////////////
   // schedules
   //
   // Collection of active schedules
   ////////////////////////////////////////////////////////////
   typedef std::map<uint4, Csi::SharedPtr<Schedule> > schedules_type;
   schedules_type schedules;

   ////////////////////////////////////////////////////////////
   // leastWait
   //
   // The least amount of waiting until the next amount (expressed in milli-seconds)
   ////////////////////////////////////////////////////////////
   uint4 leastWait;

   ////////////////////////////////////////////////////////////
   // current_wait_id
   //
   // The identifier for the current one shot timer
   ////////////////////////////////////////////////////////////
   uint4 current_wait_id;

   ////////////////////////////////////////////////////////////
   // one_shot
   //
   // The one shot timer that will be used to time between events
   ////////////////////////////////////////////////////////////
   one_shot_handle one_shot;

   ////////////////////////////////////////////////////////////
   // last_schedule_id
   //
   // Identifies the last schedule identifier that was used.  The next identifier allocated will be
   // calculated from this value. 
   ////////////////////////////////////////////////////////////
   uint4 last_schedule_id;

   ////////////////////////////////////////////////////////////
   // last_checked
   //
   // Used to determine whether the system clock has regressed (run backward).
   // This could happen when daylight savings ends or when the time zone gets
   // changed.  
   ////////////////////////////////////////////////////////////
   Csi::LgrDate last_checked;
};

#endif

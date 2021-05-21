/* Scheduler.cpp

   This module implements the methods and message map for the Scheduler class.

   Copyright (C) 1996, 2013 Campbell Scientific, Inc.
   
   Written by: Jon Trauntvein
   Date Begun: Friday 12 July 1996
   Last Change: Tuesday 15 October 2013
   Last Commit: $Date: 2013-10-15 16:58:01 -0600 (Tue, 15 Oct 2013) $ 
   Committed by: $Author: jon $
   
*/

#pragma hdrstop
#include <assert.h>
#include "Scheduler.h"
#include "MsgExcept.h"


////////////////////////////////////////////////////////////
// class Scheduler definitions
////////////////////////////////////////////////////////////
Scheduler::Scheduler(one_shot_handle one_shot_):
   leastWait(60000),
   current_wait_id(0),
   last_schedule_id(0),
   one_shot(one_shot_)
{
   if(one_shot == 0)
      one_shot.bind(new OneShot);
} // constructor


Scheduler::~Scheduler()
{
   if(current_wait_id != 0)
      one_shot->disarm(current_wait_id);
   one_shot.clear();
} // destructor


uint4 Scheduler::start(
   SchedulerClient *client,
   Csi::LgrDate const &start,
   uint4 interval,
   bool ignore_past)
{
   // determine the appropriate next time from the start time
   Csi::LgrDate now = Csi::LgrDate::system();
   Csi::LgrDate temp = start;
   int8 passed, diff;

   if(interval == 0)
      throw MsgExcept("Schedule started with an invalid interval");
   if(start < now)
   {
      // calculate the number of intervals that have elapsed
      temp = now - start;
      diff = temp.get_nanoSec() / Csi::LgrDate::nsecPerMSec;
      passed = diff/interval;  // number of intervals elapsed

      // now calculate the real start time
      temp = start + Csi::LgrDate(passed * interval* Csi::LgrDate::nsecPerMSec);
      if(ignore_past)
         temp += interval * Csi::LgrDate::nsecPerMSec;
   }

   // allocate and initialise a new schedule record
   uint4 id = last_schedule_id + 1;
   bool unique_schedule_id = false;

   while(!unique_schedule_id)
   {
      if(id == 0)
         id = 1;
      if(schedules.find(id) == schedules.end())
      {
         last_schedule_id = id;
         unique_schedule_id = true;
      }
      else
         ++id;
   }
   
   // create a new schedule object
   Csi::SharedPtr<Schedule> sched(new Schedule);

   sched->client = client;
   sched->next = temp;
   sched->interval = static_cast<int8>(interval) * Csi::LgrDate::nsecPerMSec;
   sched->base = start;
   schedules[id] = sched;

   // the schedule might need to fire now but we first need to return the schedule identifier. In
   // order to make that work, we will readjust the timer so that it fires very soon (<100 msec).
   if(current_wait_id != 0)
      one_shot->disarm(current_wait_id);
   current_wait_id = one_shot->arm(this,10);
   return id;
} // start


void Scheduler::cancel(uint4 schedId)
{
   // find the identified schedule
   schedules_type::iterator i = schedules.find(schedId);

   if(i != schedules.end())
   {
      schedules.erase(i);
      on_interval_change();
   }
} // cancel


Csi::LgrDate Scheduler::nextTime(uint4 schedId)
{
   // find the identified schedule
   Csi::LgrDate rtn;
   schedules_type::iterator i = schedules.find(schedId);

   if(i != schedules.end())
      rtn = i->second->next;
   return rtn;
} // nextTime


bool Scheduler::check_status()
{
   bool rtn = one_shot->is_working();
   if(rtn && !schedules.empty())
   {
      if(current_wait_id == 0 || leastWait > 60000)
         rtn = false;
   }
   return rtn;
} // check_status


void Scheduler::on_interval_change()
{
   // cancel the current timer (if any)
   if(current_wait_id != 0)
      one_shot->disarm(current_wait_id);

   // we need to determine whether the system clock has regressed since the last time we checked.
   Csi::LgrDate now(Csi::LgrDate::system());
   Csi::LgrDate temp; 
   schedules_type local_schedules;
   if(last_checked > now && last_checked - now > 5 * Csi::LgrDate::nsecPerMin)
   {
      local_schedules = schedules;
      for(schedules_type::iterator si = local_schedules.begin(); si != local_schedules.end(); ++si)
      {
         Csi::SharedPtr<Schedule> schedule(si->second);
         if(SchedulerClient::is_valid_instance(schedule->client))
         {
            int8 diff(now.get_nanoSec() - schedule->base.get_nanoSec());
            int8 passed((diff / schedule->interval) + 1);
            schedule->next = schedule->base + passed * schedule->interval;
            schedule->client->on_schedule_regressed(si->first);
         }
      }
   }
   last_checked = now;
   
   // scan the list of schedules to see which should be triggered
   //
   // While processing the schedule list, some clients may react by adding or removed schedules. We
   // deal with this by making a local copy of the schedules and iterating through that local
   // copy.
   int8 least_wait(10000);
   int8 interval;

   local_schedules = schedules;
   for(schedules_type::iterator si = local_schedules.begin();
       si != local_schedules.end();
       si++)
   {
      // compare now with the scheduled time
      Csi::SharedPtr<Schedule> sched = si->second;
      
      if(now >= sched->next)
      {
         // This event might have been held up for a period greater than the interval, we will
         // calculate the next interval into the future
         Csi::LgrDate passed = (now - sched->next)/sched->interval;
         
         assert(passed >= 0);
         sched->next += sched->interval * (passed + 1);
         assert(sched->next > now);
         
         // inform the client of the event
         if(SchedulerClient::is_valid_instance(sched->client))
            sched->client->onScheduledEvent(si->first);
         else
            schedules.erase(si->first);
      }
      
      // calculate the interval to the schedule's time
      temp = sched->next - now;
      interval = temp.get_nanoSec() / Csi::LgrDate::nsecPerMSec;
      if(interval < least_wait)
         least_wait = interval;
   }

   // now set up the timer to wait for the next interval
   current_wait_id = one_shot->arm(this, static_cast<uint4>(least_wait));
} // on_interval_change



void Scheduler::onOneShotFired(uint4 id)
{
   if(id == current_wait_id)
      current_wait_id = 0;
   on_interval_change();
} // onOneShotFired

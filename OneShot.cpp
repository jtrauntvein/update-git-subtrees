/* OneShot.cpp

   Copyright (C) 1998, 2010 Campbell Scientific, Inc.
   
   Written by: Jon Trauntvein
   Date Begun: Thursday 13 February 1997
   Last Change: Thursday 19 August 2010
   Last Commit: $Date: 2010-08-19 16:04:25 -0600 (Thu, 19 Aug 2010) $ 
   Committed by: $Author: tmecham $
   
*/

#pragma hdrstop
#include "OneShot.h"
#include "Csi.Utils.h"
#include "trace.h"
#include <assert.h>
#include <typeinfo>


////////////////////////////////////////////////////////////
// class OneShotClient definitions
////////////////////////////////////////////////////////////
OneShotClient::~OneShotClient()
{
   if(OneShot::is_valid_instance(last_timer_used))
      last_timer_used->disarm_all_for_client(this);
} // destructor


namespace OneShotHelpers
{
   ////////////////////////////////////////////////////////////
   // class event_thread_fired
   ////////////////////////////////////////////////////////////
   class event_thread_fired: public Csi::Event
   {
   public:
      ////////////////////////////////////////////////////////////
      // event_id
      ////////////////////////////////////////////////////////////
      static uint4 const event_id;

   private:
      ////////////////////////////////////////////////////////////
      // constructor
      ////////////////////////////////////////////////////////////
      event_thread_fired(OneShot *timer):
         Event(event_id,timer)
      { }

   public:
      ////////////////////////////////////////////////////////////
      // create
      ////////////////////////////////////////////////////////////
      static event_thread_fired *create(OneShot *timer)
      { return new event_thread_fired(timer); }

      ////////////////////////////////////////////////////////////
      // destructor
      ////////////////////////////////////////////////////////////
      virtual ~event_thread_fired()
      { }
   };


   uint4 const event_thread_fired::event_id =
   Csi::Event::registerType("OneShot::event_thread_fired");
};


////////////////////////////////////////////////////////////
// class OneShot definitions
//////////////////////////////////////////////////////////// 
uint4 OneShot::timerEvId = Csi::Event::registerType("OneShot::timerEv");


OneShot::OneShot():
   leastWait(0),
   shouldDie(false),
   forceCycle(false),
   died_violently(false),
   arm_event("",false,false),
#ifdef _WIN32
   die_event("",false,false),
#endif
   last_timer_id(0)
{
   // start the thread
   Thread::start();
} // constructor


OneShot::~OneShot()
{
   // wait for the thread to die
   kill();
} // destructor


uint4 OneShot::arm(OneShotClient *client, uint4 msec)
{
   // allocate a new id for the new timer object
   uint4 id = last_timer_id + 1;
   bool is_unique = false;

   while(!is_unique)
   {
      if(id == 0)
         id = 1;
      events_type::iterator ei = events.find(id);
      if(ei == events.end())
      {
         last_timer_id = id;
         is_unique = true;
      }
      else
         ++id;
   }
   
   // create a new event object
   Timer event;

   event.client = client;
   client->last_timer_used = this;
   event.delay = msec;
   event.base = Csi::counter(0);
   event.id = id;
   events[id] = event;
   
   // force the thread cycle
   if(!forceCycle)
   {
      forceCycle = true;
      arm_event.set();
   }
   return id;
} // arm


void OneShot::disarm(uint4 &id)
{
   events_type::iterator ei = events.find(id);
   if(ei != events.end())
      events.erase(ei);
   id = 0;
} // disarm


void OneShot::reset(uint4 id)
{
   events_type::iterator i = events.find(id);

   if(i != events.end())
      i->second.base = Csi::counter(0);
} // reset


void OneShot::receive(Csi::SharedPtr<Csi::Event> &ev)
{
   using namespace OneShotHelpers;
   if(ev->getType() == event_thread_fired::event_id)
   {
      // scan through the events to see if any have expired
      events_type::iterator ei = events.begin();
      events_type fired_timers;

      
      leastWait = 60000;
      forceCycle = false;
      while(ei != events.end())
      {
         uint4 elapsed = Csi::counter(ei->second.base);
         
         if(elapsed >= ei->second.delay)
         {
            events_type::iterator dei = ei++;
            fired_timers[dei->first] = dei->second;
            events.erase(dei);
         }
         else
         {
            if(ei->second.delay - elapsed < leastWait) 
               leastWait = ei->second.delay - elapsed;
            ++ei;
         }
      }

      // rearm the thread
      arm_event.set();

      // notify all of the timers that have been fired
      for(ei = fired_timers.begin(); ei != fired_timers.end(); ++ei)
      {
         Timer &timer = ei->second;
         if(OneShotClient::is_valid_instance(timer.client))
            timer.client->onOneShotFired(timer.id);
      }
   }
} // receive 


bool OneShot::is_working() const
{
   bool rtn = !died_violently && is_started;
   if(rtn)
      rtn = leastWait <= 60000;
   return rtn;
} // is_working


void OneShot::disarm_all_for_client(OneShotClient *client)
{
   events_type::iterator ei = events.begin();
   while(ei != events.end())
   {
      if(ei->second.client == client)
      {
         events_type::iterator dei = ei++;
         events.erase(dei);
      }
      else
         ++ei;
   }
} // disarm_all_for_client


uint4 OneShot::get_time_remaining(uint4 id)
{
   uint4 rtn = 0xffffffff;
   events_type::iterator ti = events.find(id);
   if(ti != events.end())
      rtn = Csi::counter(ti->second.base);
   return rtn;
} // get_time_remaining


void OneShot::execute()
{
   try
   {
      while(!shouldDie && !died_violently)
      {
         // wait for the event to be armed
         arm_event.wait(100);
         if(!shouldDie)
         {
            // now do the timed wait
            if(!forceCycle && leastWait > 10)
               arm_event.wait(leastWait);
            if(!shouldDie)
            {
               Csi::Event *ev = OneShotHelpers::event_thread_fired::create(this);
               ev->post();
            }
         }
      } // end thread loop
   }
   catch(std::exception &)
   {
      died_violently = true;
   }

   // kick the main thread loose if it is waiting for us to die
   if(shouldDie)
      die_event.set();
} // execute


void OneShot::kill()
{
   shouldDie = true;
   arm_event.set();
   if(!died_violently)
   {
      die_event.wait();
   }
} // kill

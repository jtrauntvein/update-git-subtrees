/* Csi.SimpleDispatch.cpp

   Copyright (C) 2005, 2005 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 08 August 2005
   Last Change: Friday 30 December 2005
   Last Commit: $Date: 2010-11-11 13:46:40 -0600 (Thu, 11 Nov 2010) $ (UTC)
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.SimpleDispatch.h"


namespace Csi
{
   ////////////////////////////////////////////////////////////
   // class SimpleDispatch definitions
   ////////////////////////////////////////////////////////////
   SimpleDispatch::SimpleDispatch():
      should_quit(false),
      quit_code(0),
      ev_ready(
         "Csi::SimpleDispatch::ev_ready",
         false,                 // initial state
         false)                  // manual reset
   { }

   
   SimpleDispatch::~SimpleDispatch()
   { }

   
   void SimpleDispatch::post(Event *ev_)
   {
      SharedPtr<Event> ev(ev_);
      if(isDeliverable(ev->getRcvr()))
      {
         events_type::key_type key(events);
         key->push_back(ev);
         ev_ready.set();
         key.release(); 
      }
      else
         throw Event::BadPost();
   } // post


   void SimpleDispatch::post_quit_message(int quit_code_)
   {
      should_quit = true;
      quit_code = quit_code_;
   } // post_quit_message

   
   bool SimpleDispatch::do_dispatch()
   {
      if(!should_quit)
      {
         // the condition variable should be ready to roll
         ev_ready.wait(1000);
         
         // we will first copy all of the events into a local structure.  This will prevent us from
         // having to release the queue each time we dispatch an event. 
         events_type::key_type key(events);
         event_queue_type current(*key);
         key->clear();
         key.release();
         
         // dispatch the events
         while(!current.empty() && !should_quit)
         {
            SharedPtr<Event> ev(current.front());
            current.pop_front();
            ev->set_was_dispatched();
            if(isDeliverable(ev->getRcvr()) && ev->get_can_deliver())
               ev->getRcvr()->receive(ev);
         }
      }
      return !should_quit;
   } // do_dispatch

   
   void SimpleDispatch::unregisterRcvr(EventReceiver *rcvr)
   {
      events_type::key_type key(events);
      for(event_queue_type::iterator ei = key->begin();
          ei != key->end();
          ++ei)
      {
         SharedPtr<Event> &event = *ei;
         if(event->getRcvr() == rcvr)
            event->disable_delivery(); 
      }
   } // unregisterRcvr

};


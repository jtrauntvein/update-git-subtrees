/* Csi.Win32Dispatch.cpp

   Copyright (C) 1998, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 10 November 1999
   Last Change: Tuesday 10 July 2012
   Last Commit: $Date: 2012-07-10 07:56:48 -0600 (Tue, 10 Jul 2012) $ 
   Committed by: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Win32Dispatch.h"
#include "Csi.Utils.h"
#include "Csi.OsException.h"
#include "trace.h"
#include <assert.h>
#include <iterator>


namespace
{
   uint4 const wm_event = RegisterWindowMessageW(L"WM_CsiWin32Dispatch");
};


namespace Csi
{
   ////////////////////////////////////////////////////////////
   // class Win32Dispatch definitions
   ////////////////////////////////////////////////////////////
   Win32Dispatch::Win32Dispatch(
      bool allow_reentrancy_):
      MessageWindow("Win32 Dispatcher"),
      post_disabled(false),
      allow_reentrancy(allow_reentrancy_),
      last_post_timer(0),
      thread_id(::GetCurrentThreadId())
   { InitializeCriticalSection(&queue_protector); }


   Win32Dispatch::~Win32Dispatch()
   {
      event_queue.clear();
      current_events.clear();
      DeleteCriticalSection(&queue_protector);
   } // desctructor


   void Win32Dispatch::post(Event *ev)
   {
      bool bad_post = false;
      try
      {
         SharedPtr<Event> event_handle(ev);
         if(isDeliverable(ev->getRcvr()))
         {
            // add the event to the event queue
            EnterCriticalSection(&queue_protector);
            event_queue.push_back(event_handle);
            if(!post_disabled || last_post_timer == 0 || Csi::counter(last_post_timer) > 100)
            {
               try
               {
                  last_post_timer = Csi::counter(0);
                  post_message(wm_event);
                  post_disabled = true;
               }
               catch(std::exception &e)
               {
                  OsException os_error(e.what());
                  trace("Csi::Win32Dispatch post failed -- %s",os_error.what());
               }
            }
            LeaveCriticalSection(&queue_protector);
         }
         else
            bad_post = true;
      }
      catch(std::exception &e)
      {
         trace("Csi::Win32Dispatch -- post exception=%s",e.what());
         bad_post = true;
      }
      if(bad_post)
         throw Event::BadPost();
   } // post


   void Win32Dispatch::post_quit_message(int code)
   {
      if(thread_id == 0)
         ::PostQuitMessage(code);
      else
         ::PostThreadMessage(thread_id, WM_QUIT, code, 0);
   } // post_quit_message


   LRESULT Win32Dispatch::on_message(uint4 message_id, WPARAM p1, LPARAM p2)
   {
      LRESULT rtn = 0;
      if(message_id == wm_event)
      {
         // this method can be considered re-entrant if the current_events queue is not empty on
         // entry.  This must be considered after the events are copied.
         bool is_reentrant = !current_events.empty();
         
         // The critical section is required here because other threads can put events in the
         // queue while we are in the dispatch loop. The critical section must be released before
         // an event is dispatched. Otherwise, a deadlock situation can occur.
         EnterCriticalSection(&queue_protector);
         std::copy(
            event_queue.begin(),
            event_queue.end(),
            std::back_inserter(current_events));
         event_queue.clear();
         post_disabled = false;
         LeaveCriticalSection(&queue_protector);

         // we now need to dispatch all of the events left in the current_events queue
         while(!current_events.empty() && (!is_reentrant || allow_reentrancy))
         {
            // grab the first event and pop it off the queue
            SharedPtr<Event> ev(current_events.front());
            current_events.pop_front();
            
            ev->set_was_dispatched();
            if(isDeliverable(ev->getRcvr()) && ev->get_can_deliver())
               ev->getRcvr()->receive(ev);
            else
               trace(
                  "Event undeliverable -- %p, %p, %u",
                  ev.get_rep(),
                  ev->getRcvr(),
                  ev.get_reference_count());
         }                  
      }
      else
         rtn = MessageWindow::on_message(message_id,p1,p2);
      return rtn;
   } // on_message


   void Win32Dispatch::unregisterRcvr(EventReceiver *rcvr)
   {
      EnterCriticalSection(&queue_protector);
      for(event_queue_type::iterator ei = event_queue.begin();
          ei != event_queue.end();
          ++ei)
      {
         Csi::SharedPtr<Event> &event = *ei;
         if(event->getRcvr() == rcvr)
         {
            trace("Csi::Win32Dispatch::unregisterRcvr(%p)",rcvr);
            event->disable_delivery();
         }
      }
      LeaveCriticalSection(&queue_protector);
   } // unregisterRcvr
};


   
   

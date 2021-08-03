/* Csi.Events.cpp

   Copyright (c) 1998, 2016 Campbell Scientific, Inc.
   
   Written by: Jon Trauntvein
   Date Begun: Friday 6 February 1998
   Last Change: Friday 29 April 2016
   Last Commit: $Date: 2020-09-26 07:54:05 -0600 (Sat, 26 Sep 2020) $ 
   Committed by: $Author: jon $
   
*/

#pragma hdrstop                 // stop creation of precompiled header
#include "Csi.Events.h"
#include "Csi.Utils.h"
#include "trace.h"
#include <assert.h>


namespace Csi
{
   ////////////////////////////////////////////////////////////
   // class Event definitions
   ////////////////////////////////////////////////////////////
   EventDispatcher *Event::dispatcher = 0;


   uint4 Event::registerType(char const *typeStr)
   {
      // we will use the value returned by the crc32 function
      return Csi::crc32(typeStr, (uint4)strlen(typeStr));
   } // registerType


   Event::Event(uint4 type_, EventReceiver *rcvr_):
      type(type_),
      rcvr(rcvr_),
      was_dispatched(false),
      rcvr_id(0xFFFFFFFF)
   {
      if(EventReceiver::is_valid_instance(rcvr))
         rcvr_id = rcvr->get_object_id();
   } // constructor


   Event::~Event()
   { }


   void Event::post(bool catch_bad_post)
   {
      was_dispatched = false;
      if(dispatcher)
         dispatcher->post(this);
      else if(!catch_bad_post)
         throw BadPost();
      else
         delete this;
   } // post


   void Event::set_dispatcher(EventDispatcher *dispatcher_)
   {
      if(dispatcher)
         delete dispatcher;
      dispatcher = dispatcher_;
   } // set_dispatcher


   EventDispatcher *Event::get_dispatcher()
   { return dispatcher; }


   void Event::unregisterRcvr(EventReceiver *rcvr_)
   {
      if(dispatcher)
         dispatcher->unregisterRcvr(rcvr_);
   } // unregisterRcvr


   void Event::post_quit_message(int code)
   {
      if(dispatcher)
         dispatcher->post_quit_message(code);
   } // post_quit_message


   ////////////////////////////////////////////////////////////
   // class EventReceiver definitions
   ////////////////////////////////////////////////////////////
   namespace
   {
      uint4 last_receiver_object_id = 0xFFFFFFFF;
   };


   EventReceiver::EventReceiver()
   {
      object_id = ++last_receiver_object_id; 
   } // constructor

   
   EventReceiver::~EventReceiver()
   { Event::unregisterRcvr(this); }


   ////////////////////////////////////////////////////////////
   // class EventDispatcher definitions
   ////////////////////////////////////////////////////////////
   EventDispatcher::EventDispatcher()
   {} // default constructor


   EventDispatcher::~EventDispatcher()
   {} // destructor


   bool EventDispatcher::isDeliverable(EvReceiver *rcvr)
   { return EventReceiver::is_valid_instance(rcvr); }
};

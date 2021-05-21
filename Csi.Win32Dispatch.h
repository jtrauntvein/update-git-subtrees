/* Csi.Win32Dispatch.h

   Copyright (C) 1998, 2005 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 10 November 1999
   Last Change: Tuesday 20 September 2005
   Last Commit: $Date: 2011-10-28 06:43:32 -0600 (Fri, 28 Oct 2011) $ (UTC)
   Committed by: $Author: jon $
   
*/

#ifndef Csi_Win32Dispatch_h
#define Csi_Win32Dispatch_h

#include <list>
#include "Csi.Events.h"
#include "Csi.MessageWindow.h"


namespace Csi
{
   ////////////////////////////////////////////////////////////
   // class Win32Dispatch
   //
   // Defines an implementation of the abstract EvDispatcher class that uses
   // Win32 calls to post messages for event dispatch. This code was adapted
   // from the earlier MfcDispatch implementation
   //////////////////////////////////////////////////////////// 
   class Win32Dispatch: public EventDispatcher, public MessageWindow
   {
   public:
      ////////////////////////////////////////////////////////////
      // default constructor
      //////////////////////////////////////////////////////////// 
      Win32Dispatch(
         bool allow_reentracy_ = false);
      
      ////////////////////////////////////////////////////////////
      // destructor
      //////////////////////////////////////////////////////////// 
      virtual ~Win32Dispatch();
      
      ////////////////////////////////////////////////////////////
      // post
      //////////////////////////////////////////////////////////// 
      virtual void post(Event *ev);

      ////////////////////////////////////////////////////////////
      // post_quit_message
      ////////////////////////////////////////////////////////////
      virtual void post_quit_message(int code);
      
   protected:
      ////////////////////////////////////////////////////////////
      // on_message
      //
      // Overloads MessageWindow::on_message() to handle event messages
      //////////////////////////////////////////////////////////// 
      virtual LRESULT on_message(uint4 message_id, WPARAM p1, LPARAM p2);

      ////////////////////////////////////////////////////////////
      // unregisterRcvr
      //
      // Overloads the base class' version to mark all events that are pending
      // for the intended receiver as invalid so that the dispatcher will not
      // attempt to send the events.
      //////////////////////////////////////////////////////////// 
      virtual void unregisterRcvr(EventReceiver *rcvr);
      
   private:
      ////////////////////////////////////////////////////////////
      // queue_protector
      //
      // Handle to a critical section object that prevents the event queue
      // object from being accessed simultaneously from multiple threads
      //////////////////////////////////////////////////////////// 
      CRITICAL_SECTION queue_protector;

      ////////////////////////////////////////////////////////////
      // event_queue
      //
      // Holds the events that have not been initially processed.  This object
      // is guarded by the critical section above.
      //////////////////////////////////////////////////////////// 
      typedef std::list<Csi::SharedPtr<Event> > event_queue_type;
      event_queue_type event_queue;

      ////////////////////////////////////////////////////////////
      // current_events
      //
      // Holds the events that are pending for processing.  There is no
      // protection for this queue because it is expected that all events will
      // be processed by the same thread. 
      ////////////////////////////////////////////////////////////
      event_queue_type current_events;

      ////////////////////////////////////////////////////////////
      // post_disabled
      //
      // With heavy usage, more than one event can stack up in the queue while
      // a windows message is making its way through the queue. In previous
      // implementations, each event that was posted had a corresponding
      // windows message posted. This was causing the windows message queue to
      // overflow under heavy usage. This variable prevents more windows
      // messages from being posted while this object is waiting for a windows
      // message that had already been posted.
      //////////////////////////////////////////////////////////// 
      bool post_disabled;

      ////////////////////////////////////////////////////////////
      // allow_reentrancy
      //
      // This flag can be set in the constructor to allow re-entrant processing
      // of events.  If it is set, the application can drive the windows
      // message queue in a separate loop and event posting will be allowed.
      // If it is clear, on_message() will not process any other events.
      ////////////////////////////////////////////////////////////
      bool allow_reentrancy;

      ////////////////////////////////////////////////////////////
      // last_post_timer
      //
      // Used to calculate the amount of time that has elapsed since the last
      // post.  There may be occasions when a windows message can get missed
      // and we don't want the dispatch queue being held up.  
      ////////////////////////////////////////////////////////////
      uint4 last_post_timer;

      ////////////////////////////////////////////////////////////
      // thread_id
      ////////////////////////////////////////////////////////////
      uint4 const thread_id;
   };
};

#endif

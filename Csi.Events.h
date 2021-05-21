/* Csi.Events.h

   Copyright (C) 2001, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 13 July 2001
   Last Change: Thursday 15 November 2012
   Last Commit: $Date: 2020-09-26 07:54:05 -0600 (Sat, 26 Sep 2020) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Csi_Events_h
#define Csi_Events_h

#include "CsiTypeDefs.h"
#include "Csi.SharedPtr.h"
#include "Csi.InstanceValidator.h"


namespace Csi
{
   class Event;
   class EventDispatcher;
   
   ////////////////////////////////////////////////////////////
   // class EventReceiver
   //
   // Defines a base class for all objects that need to receive events.
   //
   // Limitations:
   //
   //   - EventReceiver objects (including those created from derived classes)
   //   must not be created before the dispatch mechanism has been initialised
   //   by invoking Event::set_dispatcher with a valid dispatcher object.
   ////////////////////////////////////////////////////////////
   class EventReceiver: public InstanceValidator
   {
   private:
      ////////////////////////////////////////////////////////////
      // object_id
      //
      // Specifies an identifier that is set when an object of class
      // EventReceiver is constructed.  This value will be extracted from the
      // receiver when the event is posted and will be compared to the event
      // member before the receive method is invoked. 
      ////////////////////////////////////////////////////////////
      uint4 object_id;
      
   public:
      ////////////////////////////////////////////////////////////
      // constructor
      ////////////////////////////////////////////////////////////
      EventReceiver();
      
      ////////////////////////////////////////////////////////////
      // destructor
      ////////////////////////////////////////////////////////////
      virtual ~EventReceiver();

      ////////////////////////////////////////////////////////////
      // receive
      //
      // Defines the mechanism used to deliver events to the user. The ev
      // parameter describes the event. If the receiver needs to extend the
      // lifetime of the event object beyond the call to this method, it should
      // make its own handle to the event.
      ////////////////////////////////////////////////////////////
      typedef SharedPtr<Event> event_handle;
      virtual void receive(event_handle &ev) = 0;

      ////////////////////////////////////////////////////////////
      // get_object_id
      ////////////////////////////////////////////////////////////
      uint4 get_object_id() const
      { return object_id; }
   };
   typedef class EventReceiver EvReceiver;

   
   ////////////////////////////////////////////////////////////
   // class Event
   //
   // Defines the basic parameters of an event that can be passed through the
   // event handling mechanism. Using the event type registration mechanism,
   // this class can be treated as concrete. It can also be used as a base
   // class for custom event classes. Objects of this class should be created
   // only using the new operator.
   //
   // All of the interface to the event handling mechanism is declared between
   // the EvReciever class and the
   //
   // Limitations:
   //    - Event objects should only be created using new
   //
   //    - Once an event object has been successfully posted, the client code
   //    should not attempt to delete it. Attempting to do so will result in
   //    heap corruption and invalid pointer access.
   ////////////////////////////////////////////////////////////
   class Event
   {
   public:
      ////////////////////////////////////////////////////////////
      // class BadPost
      //
      // Defines the exception class that will be thrown when a post failure
      // occurs
      ////////////////////////////////////////////////////////////
      class BadPost: public std::exception
      {
      public:
         virtual char const *what() const throw ()
         { return "Event post failed"; }
      };
      
   public:
      ////////////////////////////////////////////////////////////
      // create
      //
      // Static method that can be used to create objects of this class
      ////////////////////////////////////////////////////////////
      static Event *create(uint4 type, EventReceiver *rcvr)
      { return new Event(type,rcvr); }

      ////////////////////////////////////////////////////////////
      // destructor
      ////////////////////////////////////////////////////////////
      virtual ~Event();

      ////////////////////////////////////////////////////////////
      // getType
      //
      // Returns the type code assigned to this event object.
      ////////////////////////////////////////////////////////////
      virtual uint4 getType() const
      { return type; }

      ////////////////////////////////////////////////////////////
      // getRcvr
      //
      // Returns the receiver pointer
      ////////////////////////////////////////////////////////////
      EventReceiver *getRcvr() 
      { return rcvr; }
      
      ////////////////////////////////////////////////////////////
      // post
      //
      // Causes the event object represented by this to be sent to the
      // dispatcher. If this call succeeds, the client code should not attempt
      // to delete the event object. Provided that the dispatcher is properly
      // implemented, the message will be posted asynchronously
      ////////////////////////////////////////////////////////////
      void post(bool catch_bad_post = true);

      ////////////////////////////////////////////////////////////
      // set_dispatcher
      //
      // Initialises the static dispatcher member. This method should be used
      // to initialise the event handling mechanism. It should be called before
      // any EventReceiver objects are created or before any Event objects are
      // posted.
      ////////////////////////////////////////////////////////////
      static void set_dispatcher(EventDispatcher *dispatcher_);

      /**
       * @return Returns the currently assigned dispatcher or null if there is none.
       */
      static EventDispatcher *get_dispatcher();

      ////////////////////////////////////////////////////////////
      // post_quit_message
      ////////////////////////////////////////////////////////////
      static void post_quit_message(int code);

      ////////////////////////////////////////////////////////////
      // registerType
      //
      // Registers an event type. Will create a unique type identifier
      ////////////////////////////////////////////////////////////
      static uint4 registerType(char const *typeStr);

      ////////////////////////////////////////////////////////////
      // same_receiver
      //
      // Returns true if the receiver specified in the parameter is the same as
      // the receiver specified for this event
      ////////////////////////////////////////////////////////////
      bool same_receiver(EventReceiver *r)
      { return rcvr == r; }

      ////////////////////////////////////////////////////////////
      // disable_delivery
      //
      // Marks this message as undeliverable. This method is provided so that
      // event dispatch implementation classes can invalidate events when a
      // receiver becomes invalid.
      ////////////////////////////////////////////////////////////
      void disable_delivery()
      { rcvr = 0; was_dispatched = true; }

      ////////////////////////////////////////////////////////////
      // get_can_deliver
      //
      // Returns true if the message is marked to be able to be delivered
      ////////////////////////////////////////////////////////////
      bool get_can_deliver() const
      { return rcvr != 0  && rcvr->get_object_id() == rcvr_id; }

      ////////////////////////////////////////////////////////////
      // set_was_dispatched
      ////////////////////////////////////////////////////////////
      void set_was_dispatched()
      { was_dispatched = true; }

   protected:
      ////////////////////////////////////////////////////////////
      // constructor
      //
      // Defined to be protected to encourage event objects to only be created
      // using new.
      ////////////////////////////////////////////////////////////
      Event(uint4 type_, EventReceiver *rcvr_);
      
   private:
      friend class EventReceiver;

      ////////////////////////////////////////////////////////////
      // unregisterRcvr
      //
      // Called to mark any events destined for the specified receiver as
      // undeliverable.  This method is invoked by the destructor for the
      // EventReceiver class.
      ////////////////////////////////////////////////////////////
      static void unregisterRcvr(EventReceiver *rcvr);

   private:
      ////////////////////////////////////////////////////////////
      // type
      //
      // A code that should uniquely identify the event type to the
      // application.  The easiest way for the application to assign this value
      // is to use registerType() passing in as the string the class name of
      // the specific derived event.
      ////////////////////////////////////////////////////////////
      uint4 type;

      ////////////////////////////////////////////////////////////
      // rcvr
      //
      // The object for which this event is destined
      ////////////////////////////////////////////////////////////
      EventReceiver *rcvr;

      ////////////////////////////////////////////////////////////
      // rcvr_id
      //
      // Specifies the object ID of the receiver at the time that the event was
      // created.  This will be checked when get_can_deliver() is called to
      // determine if the original receiver for the event is the same object as
      // the current receiver for the event. 
      ////////////////////////////////////////////////////////////
      uint4 rcvr_id;

      ////////////////////////////////////////////////////////////
      // was_dispatched
      //
      // Set when this event class is dispatched to indicate that the
      // dispatching has taken place.  This value should be set to true by the
      // dispatcher and false by post().  An assertion will check this value in
      // the destructor.
      ////////////////////////////////////////////////////////////
      bool was_dispatched;
      
      ////////////////////////////////////////////////////////////
      // dispatcher
      //
      // Maintains the reference to the dispatcher. This reference is set
      // initially to null but should be set via a call to set_dispatcher
      // sometime when the program is initialised
      ////////////////////////////////////////////////////////////
      static EventDispatcher *dispatcher;
   };

   
   ////////////////////////////////////////////////////////////
   // class EventDispatcher
   //
   // Defines a base class for event dispatchers. Classes derived will
   // implement event handling in an environment specific way.
   ////////////////////////////////////////////////////////////
   class EventDispatcher
   {
   public:
      ////////////////////////////////////////////////////////////
      // default constructor
      ////////////////////////////////////////////////////////////
      EventDispatcher();

      ////////////////////////////////////////////////////////////
      // destructor
      ////////////////////////////////////////////////////////////
      virtual ~EventDispatcher();

   protected:
      friend class Event;

      ////////////////////////////////////////////////////////////
      // post
      //
      // Must be overloaded by derived classes to process the event. The
      // overloaded version must be written such that the event will not be
      // delivered until after the post method has returned. In other words,
      // the implementation must guarantee asynchronous delivery.
      ////////////////////////////////////////////////////////////
      virtual void post(Event *ev) = 0;

      ////////////////////////////////////////////////////////////
      // unregisterRcvr
      //
      // Removes a receiver pointer from the list of registered receivers.
      ////////////////////////////////////////////////////////////
      virtual void unregisterRcvr(EventReceiver *rcvr) { }

      ////////////////////////////////////////////////////////////
      // post_quit_message
      ////////////////////////////////////////////////////////////
      virtual void post_quit_message(int code) = 0;

      ////////////////////////////////////////////////////////////
      // isDeliverable
      //
      // Returns true if the specified receiver is still registered
      ////////////////////////////////////////////////////////////
      bool isDeliverable(EventReceiver *rcvr);
   }; 
   typedef class EventDispatcher EvDispatcher;
};


#endif

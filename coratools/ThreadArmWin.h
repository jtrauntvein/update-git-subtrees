/* ThreadArmWin.h

   Copyright (C) 1998, 2009 Campbell Scientific, Inc.
   
   Written by: Jon Trauntvein
   Date Begun: Monday 11 January 1999
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Committed by: $Author: jon $
   
*/

#ifndef ThreadArmWin_h
#define ThreadArmWin_h

#include "CsiEvents.h"
#include "Csi.Thread.h"

namespace Csi
{
   ////////// class ThreadArmWin
   // Defines a class that can be used as a base to define classes that use threads in an arm
   // pattern. In this pattern, the thread is "armed" to perform a unit of work. When that unit is
   // complete, the thread posts an event that is handled in the applications main event queue and
   // goes into a state of waiting for the next "arm" event.
   class ThreadArmWin: public Thread, public EvReceiver
   {
   public:
      ////////// constructor
      ThreadArmWin();

      ////////// destructor
      virtual ~ThreadArmWin();

   protected:
      ////////// arm
      // Turns the worker thread loose on the arm task
      void arm();
      
      ////////// doBeforeArm
      // Provides a hook that allows the derived class to perform work before the thread moves into
      // a state of waiting to be armed. It should be remembered that this method will be invoked
      // within the worker thread and not within the main thread.
      virtual void doBeforeArm()
      { }

      ////////// doOnArm
      // Defines a hook that allows the derived class to perform work when an arm event occurs. This
      // method must be overridden. It should be remembered that this method will be invoked within
      // the worker thread and not within the main thread.
      virtual void doOnArm() = 0;

      ////////// doAfterArm
      // Defines a hook that is invoked after the main thread has received an event notification
      // that the arm is complete.
      virtual void doAfterArm() = 0;

      ////////// startThread
      // Starts the thread if it has been stopped
      void startThread();

      ////////// stopThread
      // Sets the conditions and waits for the thread to die
      void stopThread();

      ////////// threadShouldDie
      // Returns true if the thread should die
      bool threadShouldDie();

   private:
      ////////// receive
      // Receives the event from the thread that indicates that it has completed the conditions of
      // the arm event
      virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

      ////////// execute
      // Performs the work for the thread
      virtual void execute();
      
   private:
      ////////// armEvent
      // win32 kernel object that handles the arm signal to the thread
      HANDLE armEvent;

      ////////// lifeEvent
      // win32 kernel object that is used to force the main thread to wait until the thread is in an
      // armed state before continuing and signals the main thread when the thread has stopped
      HANDLE lifeEvent;

      ////////// shouldDie
      // Variable set by the main thread to indicate that the arm thread should die
      bool shouldDie;

      ////////// eventId
      // Identifies the event that is posted when an arm cycle is complete
      static const uint4 eventId;

      ////////// threadIsRunning
      // Set to true if the thread has been started. Set to true by the startThread method and false
      // by the stopThread method.
      bool threadIsRunning;
   };
};

#endif

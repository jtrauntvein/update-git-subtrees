/* ThreadArmWin.cpp

   Copyright (C) 1998, 2009 Campbell Scientific, Inc.
   
   Written by: Jon Trauntvein
   Date Begun: Monday 11 January 1999
   Last Change: Friday 23 October 2009

*/

#pragma warning(disable: 4786)
#pragma hdrstop
#include "ThreadArmWin.h"

namespace Csi
{
   ////////////////////////////////////////////////////////////
   // class ThreadArmWin definitions
   ////////////////////////////////////////////////////////////

   const uint4 ThreadArmWin::eventId = Event::registerType("ThreadArmWin::eventId");


   ThreadArmWin::ThreadArmWin():
      armEvent(INVALID_HANDLE_VALUE),
      lifeEvent(INVALID_HANDLE_VALUE),
      shouldDie(false),
      threadIsRunning(false)
   {
      // initialise the kernel objects
      armEvent = CreateEvent(0,FALSE,FALSE,0);
      lifeEvent = CreateEvent(0,FALSE,FALSE,0);

      // start the thread
      startThread();
   } // constructor


   ThreadArmWin::~ThreadArmWin()
   {
      // make sure that the thread is stopped
      if(threadIsRunning)
         stopThread();

      // now delete the kernel objects
      CloseHandle(armEvent);
      CloseHandle(lifeEvent);
   } // destructor


   void ThreadArmWin::arm()
   {
      if(!threadIsRunning)
         startThread();
      SetEvent(armEvent);
   } // arm


   void ThreadArmWin::startThread()
   {
      if(!threadIsRunning)
      {
         shouldDie = false;
         Thread::start();
         WaitForSingleObject(lifeEvent,INFINITE);
         threadIsRunning = true;
      }
   } // startThread


   void ThreadArmWin::stopThread()
   {
      if(threadIsRunning)
      {
         shouldDie = true;
         SetEvent(armEvent);
         WaitForSingleObject(lifeEvent,INFINITE);
         threadIsRunning = false;
      }
   } // stopThread


   bool ThreadArmWin::threadShouldDie()
   { return shouldDie; }


   void ThreadArmWin::receive(Csi::SharedPtr<Csi::Event> &ev)
   {
      if(ev->getType() == eventId)
         doAfterArm();
   } // receive


   void ThreadArmWin::execute()
   {
      bool first_through = false;  // indicates whether the loop has been executed the first time
      while(!shouldDie)
      {
         doBeforeArm();
         if(!first_through)
         {
            SetEvent(lifeEvent);   // signal the main thread that we are started
            first_through = true;
         }
         WaitForSingleObject(armEvent,INFINITE);
         if(!shouldDie)
         {
            doOnArm();
            try
            {
               Csi::Event *ev = Csi::Event::create(eventId,this);
               ev->post();
            }
            catch(Event::BadPost)
            { shouldDie = true; }
         }
      }

      // signal the main thread that this thread is shut down
      SetEvent(lifeEvent);
   } // execute 
};

/* Csi.WorkerThread.cpp

   Copyright (C) 2009, Campbell Scientific, Inc

   Written by: Jon Trauntvein
   Date Begun: Friday 20 November 2009
   Last Change: Friday 20 November 2009
   Last Commit: $Date: 2009-11-20 11:06:41 -0600 (Fri, 20 Nov 2009) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.WorkerThread.h"


namespace Csi
{
   ////////////////////////////////////////////////////////////
   // class EventWorkerThreadCommandComplete definitions
   ////////////////////////////////////////////////////////////
   uint4 const EventWorkerThreadCommandComplete::event_id(
      Event::registerType("Csi::WorkerThreadCommandComplete"));

   
   ////////////////////////////////////////////////////////////
   // class WorkerThread definitions
   ////////////////////////////////////////////////////////////
   WorkerThread::WorkerThread():
      should_quit(false),
      condition(0, false, false)
   { start(); }


   void WorkerThread::start()
   {
      should_quit = false;
      Thread::start();
   } // start
   

   void WorkerThread::wait_for_end()
   {
      should_quit = true;
      condition.set();
      Thread::wait_for_end();
   } // wait_for_end


   void WorkerThread::add_command(value_type command)
   {
      commands_protector.lock();
      commands.push_back(command);
      commands_protector.unlock();
      condition.set();
   } // add_command
   

   void WorkerThread::execute()
   {
      while(!should_quit)
      {
         // check to see if there is a command to execute
         value_type current_command;
         commands_protector.lock();
         if(!commands.empty())
         {
            current_command = commands.front();
            commands.pop_front();
         }
         commands_protector.unlock();

         // we are now ready to execute the command
         if(current_command != 0 && !should_quit)
         {
            if(EventReceiver::is_valid_instance(current_command->client))
            {
               current_command->execute(this);
               EventWorkerThreadCommandComplete::cpost(current_command);
            }
         }
         else if(!should_quit)
            condition.wait(1000);
      }
   } // execute
};


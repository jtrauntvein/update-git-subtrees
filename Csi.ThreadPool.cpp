/* Csi.ThreadPool.cpp

   Copyright (C) 2013, 2013 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Monday 20 May 2013
   Last Change: Friday 07 June 2013
   Last Commit: $Date: 2018-10-05 12:01:38 -0600 (Fri, 05 Oct 2018) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.ThreadPool.h"


namespace Csi
{
   namespace ThreadPoolHelpers
   {
      ////////////////////////////////////////////////////////////
      // class ThreadPoolThread definitions
      ////////////////////////////////////////////////////////////
      ThreadPoolThread::ThreadPoolThread(ThreadPool *pool_):
         should_quit(false),
         pool(pool_),
         condition(0, false, false)
      { start(); }


      ThreadPoolThread::~ThreadPoolThread()
      { wait_for_end(); }


      void ThreadPoolThread::start()
      {
         should_quit = false;
         Thread::start();
      } // start
      

      void ThreadPoolThread::wait_for_end()
      {
         should_quit = true;
         condition.set();
         Thread::wait_for_end();
      } // wait_for_end


      void ThreadPoolThread::abort()
      {
         Csi::SharedPtr<WorkerThreadCommand> command(current_command);
         if(command != 0)
            command->abort();
      } // abort

      
      void ThreadPoolThread::execute()
      {
         while(!should_quit)
         {
            // check to see if there is a command to execute
            pool->commands_protector.lock();
            if(!pool->commands.empty())
            {
               current_command = pool->commands.front();
               pool->commands.pop_front();
            }
            busy = false;
            pool->commands_protector.unlock();

            // we are now ready to execute the command
            if(current_command != 0 && !should_quit)
            {
               if(EventReceiver::is_valid_instance(current_command->client))
               {
                  busy = true;
                  current_command->execute(0);
                  EventWorkerThreadCommandComplete::cpost(current_command);
                  current_command.clear();
                  busy = false;
               }
            }
            else if(!should_quit)
               condition.wait(1000);
         }
      } // execute
   };


   ////////////////////////////////////////////////////////////
   // class ThreadPool definitions
   ////////////////////////////////////////////////////////////
   ThreadPool::ThreadPool()
   { }


   void ThreadPool::start()
   { }


   void ThreadPool::wait_for_end()
   {
      while(!threads.empty())
      {
         thread_handle thread(threads.front());
         threads.pop_front();
         thread->wait_for_end();
      }
   } // wait_for_end


   void ThreadPool::add_command(value_type command)
   {
      // we need to search for a thread that is not busy
      bool dispatched(false);
      commands_protector.lock();
      commands.push_back(command);
      for(auto ti = threads.begin(); !dispatched && ti != threads.end(); ++ti)
      {
         auto &thread(*ti);
         if(!thread->get_busy())
         {
            thread->set_busy();
            thread->condition.set();
            dispatched = true;
         }
      }
      commands_protector.unlock();


      // if all of the threads are busy and the number of threads started is less than  the number
      // of cores, we can start another thread
      if(!dispatched && threads.size() < 8)
      {
         thread_handle thread(new thread_type(this));
         threads.push_back(thread);
         thread->start();
      }
   } // add_command


   uint4 ThreadPool::commands_size()
   {
      uint4 rtn(0);
      commands_protector.lock();
      rtn = commands.size();
      commands_protector.unlock();
      return rtn;
   } // commands_size


   void ThreadPool::abort()
   {
      // we need to notify all of the pool threads that they should abort their commands
      for(auto ti = threads.begin(); ti != threads.end(); ++ti)
         (*ti)->abort();
      
      // notify all pending commands that they will abort
      commands_protector.lock();
      while(!commands.empty())
      {
         value_type command(commands.front());
         commands.pop_front();
         command->abort();
      }
      commands_protector.unlock();
   } // abort
};


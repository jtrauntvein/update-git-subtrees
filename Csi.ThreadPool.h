/* Csi.ThreadPool.h

   Copyright (C) 2013, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Monday 20 May 2013
   Last Change: Thursday 30 August 2018
   Last Commit: $Date: 2018-10-05 12:01:38 -0600 (Fri, 05 Oct 2018) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_ThreadPool_h
#define Csi_ThreadPool_h
#include "Csi.WorkerThread.h"


namespace Csi
{
   class ThreadPool;
   namespace ThreadPoolHelpers
   {
      /**
       * Defines an object that acts as one of the threads used by the thread pool.
       */
      class ThreadPoolThread: public Thread
      {
      public:
         /**
          * Constructor
          *
          * @param pool_ Specifies the pool that owns this thread.
          */
         ThreadPoolThread(ThreadPool *pool_);

         /**
          * Destructor
          */
         virtual ~ThreadPoolThread();

         /**
          * Overloads the start method to set up the internal properties.
          */
         virtual void start();

         /**
          * Overloads the base class version to force the thread to quit.
          */
         virtual void wait_for_end();

         /**
          * @return Returns true if this thread is busy on a command.
          */
         bool get_busy() const
         { return busy; }

         /**
          * DSets the busy flag for this thread.
          */
         void set_busy()
         { busy = true; }

         /**
          * Forces this thread to be aborted.
          */
         void abort();

      protected:
         /**
          * Overloaded to work in a loop to wait for new commands to be added to the pool.
          */
         virtual void execute();

      private:
         /**
          * Specifies the signal that we will use to free the thread.
          */
         Condition condition;

         /**
          * Set to true if this thread should quit.
          */
         bool should_quit;

         /**
          * Specifies the pool that owns this thread.
          */
         ThreadPool *pool;

         /**
          * Set to true when this thread is working on a command.
          */
         bool busy;

         /**
          * Specifies the current command.
          */
         Csi::SharedPtr<WorkerThreadCommand> current_command;

         friend class ThreadPool;
      };
   };


   /**
    * Defines an object that will control one or more worker threads and a queue of commands that
    * can be executed by any of those workers.  This object implements the same interface as class
    * WorkerThread.
    */
   class ThreadPool
   {
   public:
      /**
       * Constructor
       */
      ThreadPool();

      /**
       * Destructor
       */
      virtual ~ThreadPool()
      { wait_for_end(); }

      /**
       * Initialises this pool to wor on commands.
       */
      virtual void start();

      /**
       * Forces all queued commands to be aborted.
       */
      void abort();

      ////////////////////////////////////////////////////////////
      // wait_for_end
      ////////////////////////////////////////////////////////////
      virtual void wait_for_end();

      ////////////////////////////////////////////////////////////
      // add_command
      ////////////////////////////////////////////////////////////
      typedef SharedPtr<WorkerThreadCommand> value_type;
      void add_command(value_type command);

      ////////////////////////////////////////////////////////////
      // commands_size
      ////////////////////////////////////////////////////////////
      uint4 commands_size();
      
   private:
      ////////////////////////////////////////////////////////////
      // commands
      ////////////////////////////////////////////////////////////
      typedef std::list<value_type> commands_type;
      CriticalSection commands_protector;
      commands_type commands;

      ////////////////////////////////////////////////////////////
      // threads
      ////////////////////////////////////////////////////////////
      typedef ThreadPoolHelpers::ThreadPoolThread thread_type;
      typedef SharedPtr<thread_type> thread_handle;
      typedef std::list<thread_handle> threads_type;
      threads_type threads;
      friend class thread_type;
   };
};


#endif

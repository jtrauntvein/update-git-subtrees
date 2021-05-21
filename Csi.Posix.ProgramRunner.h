/* Csi.Posix.ProgramRunner.h

   Copyright (C) 2005, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 03 August 2005
   Last Change: Thursday 07 June 2012
   Last Commit: $Date: 2012-06-07 07:50:47 -0600 (Thu, 07 Jun 2012) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Csi_Posix_ProgramRunner_h
#define Csi_Posix_ProgramRunner_h

#include "Csi.Posix.Thread.h"
#include "Csi.Events.h"
#include "StrAsc.h"


// @group: These macros define the set of values that might be used with the show_mode parameter
// (ignored in linux) of the constructor
#define SW_HIDE 0
#define SW_FORCEMINIMIZE 11
#define SW_MAXIMIZE 3
#define SW_MINIMIZE 6
#define SW_RESTORE 9
#define SW_SHOW    5
#define SW_SHOWDEFAULT 10
#define SW_SHOWMAXIMIZED 3
#define SW_SHOWMINIMIZED 2
#define SW_SHOWMINNOACTIVE 4
#define SW_SHOWNORMAL 1
// @endgroup:


namespace Csi
{
   namespace Posix
   {
      ////////////////////////////////////////////////////////////
      // class ProgramRunner
      //
      // Defines an object that will execute a specified program with the
      // option to either wait until the child program finishes, let the child
      // program alone, or to give the child program a certain amount of time
      // to finish on its own.  The object also provides the object of blocking
      // while the child program is executing.  If the blocking option is not
      // used, an event of type event_program_ended will be posted when the
      // program does finish. 
      ////////////////////////////////////////////////////////////
      class ProgramRunner: public Thread
      {
      public:
         ////////////////////////////////////////////////////////////
         // class event_program_ended
         ////////////////////////////////////////////////////////////
         enum outcome_type
         {
            outcome_terminated_normally,
            outcome_timed_out,
            outcome_start_failed,
            outcome_aborted,
         };
         class event_program_ended: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // outcome
            ////////////////////////////////////////////////////////////
            outcome_type outcome;

            ////////////////////////////////////////////////////////////
            // start_error
            //
            // Specifies the operating system error code if the process didn't
            // run to completion or couldn't be started.
            ////////////////////////////////////////////////////////////
            StrAsc start_error;

            ////////////////////////////////////////////////////////////
            // runner
            ////////////////////////////////////////////////////////////
            ProgramRunner *runner;

            ////////////////////////////////////////////////////////////
            // exit_code
            ////////////////////////////////////////////////////////////
            int exit_code;

            ////////////////////////////////////////////////////////////
            // create_and_post
            ////////////////////////////////////////////////////////////
            static void create_and_post(
               EventReceiver *receiver,
               ProgramRunner *runner,
               outcome_type outcome,
               int exit_code);

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_program_ended(
               EventReceiver *receiver,
               ProgramRunner *runner_,
               outcome_type outcome_,
               int exit_code_);
         };


      public:
         ////////////////////////////////////////////////////////////
         // constructor
         //
         // Constructs the object without calling the child program.  This can
         // be done by calling start().  The show_mode and verb parameters are
         // ignored in the posix version but are present in order to create a
         // call-level compatible interface with the win32 version. 
         ////////////////////////////////////////////////////////////
         ProgramRunner(
            EventReceiver *receiver_,
            char const *program_name_,
            char const *command_line_,
            uint4 program_wait_timeout_ = UInt4_Max,
            int show_mode_ = 0,
            bool use_thread_ = true,
            bool detach_after_start_ = false,
            char const *verb_ = "");

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~ProgramRunner();

         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         virtual void start();

         ////////////////////////////////////////////////////////////
         // wait_for_end
         ////////////////////////////////////////////////////////////
         virtual void wait_for_end();

         ////////////////////////////////////////////////////////////
         // get_kill_on_exit
         ////////////////////////////////////////////////////////////
         bool get_kill_on_exit()
         { return kill_on_exit; }

         ////////////////////////////////////////////////////////////
         // set_kill_on_exit
         ////////////////////////////////////////////////////////////
         void set_kill_on_exit(bool kill)
         { kill_on_exit = kill; }

         ////////////////////////////////////////////////////////////
         // get_working_dir
         ////////////////////////////////////////////////////////////
         StrAsc const &get_working_dir() const
         { return working_dir; }

         ////////////////////////////////////////////////////////////
         // set_working_dir
         ////////////////////////////////////////////////////////////
         void set_working_dir(StrAsc const &dir)
         { working_dir = dir; }

      protected:
         ////////////////////////////////////////////////////////////
         // execute
         ////////////////////////////////////////////////////////////
         virtual void execute();

      private:
         ////////////////////////////////////////////////////////////
         // receiver
         ////////////////////////////////////////////////////////////
         EventReceiver *receiver;

         ////////////////////////////////////////////////////////////
         // program_name
         ////////////////////////////////////////////////////////////
         StrAsc program_name;

         ////////////////////////////////////////////////////////////
         // command_line
         ////////////////////////////////////////////////////////////
         StrAsc command_line;

         ////////////////////////////////////////////////////////////
         // program_wait_timeout
         ////////////////////////////////////////////////////////////
         uint4 program_wait_timeout;

         ////////////////////////////////////////////////////////////
         // should_quit
         ////////////////////////////////////////////////////////////
         bool should_quit;

         ////////////////////////////////////////////////////////////
         // use_thread
         ////////////////////////////////////////////////////////////
         bool use_thread;

         ////////////////////////////////////////////////////////////
         // detach_after_start
         ////////////////////////////////////////////////////////////
         bool detach_after_start;

         ////////////////////////////////////////////////////////////
         // kill_on_exit
         //
         // This flag controls whether the child process will get killed if
         // this runner is forced to stop while the attached process is still
         // running.  If set to true (its default value), the process will be
         // killed when the thread exits.
         ////////////////////////////////////////////////////////////
         bool kill_on_exit;

         ////////////////////////////////////////////////////////////
         // working_dir
         ////////////////////////////////////////////////////////////
         StrAsc working_dir;
      };
   };
};


#endif

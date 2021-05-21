/* Csi.Win32.ProgramRunner.h

   Copyright (C) 2002, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 16 December 2002
   Last Change: Thursday 17 September 2020
   Last Commit: $Date: 2020-09-17 09:41:29 -0600 (Thu, 17 Sep 2020) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Csi_Win32_ProgramRunner_h
#define Csi_Win32_ProgramRunner_h

#include "Csi.Thread.h"
#include "Csi.Events.h"
#include "StrAsc.h"


namespace Csi
{
   namespace Win32
   {
      /**
       * Defines an object that will start a separate thread in which a program will be executed
       * using the ShellExecuteEx() WIN32 API function.  The started program can be set up to be
       * monitored so that an event will be posted to a receiver when the program has completed. 
       */
      class ProgramRunner: public Thread
      {
      public:
         /**
          * Defines the type of event that will be posted to the application receiver when the
          * program has been terminated.
          */
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
            /**
             * Uniquely identifies the event type.
             */
            static uint4 const event_id;

            /**
             * Specifies the outcome of running the program.
             */
            outcome_type outcome;

            /**
             * Specifies a description string for any errors reported in starting the executable
             */
            StrAsc start_error;

            /**
             * Specifies the exeit code of the process after it has ended.
             */
            int4 exit_code;

            /**
             * Specifies a pointer to the program runner reporting this event.
             */
            ProgramRunner *runner;

            /**
             * Creates an posts this event type.
             */
            static void create_and_post(
               EventReceiver *receiver,
               ProgramRunner *runner,
               outcome_type outcome,
               int4 exit_code = 0);

         private:
            /**
             * Constructor
             */
            event_program_ended(
               EventReceiver *receiver,
               ProgramRunner *runner_,
               outcome_type outcome_,
               int4 exit_code_);
         };

         
      public:
         /**
          * Constructs the object without starting the thread or the child process.  The program can
          * be launched by calling start().
          *
          * @param receiver_ Specifies an application object that will receive the completion event
          * from this component.  If null, no such event will be posted.
          *
          * @param program_name_ Specifies the path and name of the program to execute.
          *
          * @param command_line_ Specifies the command line to be passed to the program.
          *
          * @param program_wait_timeout_ Specifies the amount of time to wait for the program to
          * finish running before forcing the program to terminate and reporting a time out.  If set
          * to the default value of 0xFFFFFFFF, the thread will wait indefinitely for the program to
          * finish running.
          *
          * @param show_mode_ Specifies a value that controls whether the application will be shown
          * on the user desktop.  Can be one of SW_HIDE, SW_MAXIMIZE, SW_MINIMIZE, SW_RESTORE,
          * SW_SHOW, SW_SHOWDEFAULT, SW_SHOWMAXIMIZED, SW_SHOWMINIMIZED, SW_SHOWMINNOACTIVE,
          * SW_SHOWNA, SW_SHOWNOACTIVE, SW_SHOWNORMAL.
          *
          * @param use_thread_ Set to true (the default) if the program should be started from a
          * worker thread.
          *
          * @param detach_after_start_ Set to true if the componnet should detach from the child
          * process after it has been started.
          *
          * @param verb_ Specifies the verb to be passed to the shell.
          */
         ProgramRunner(
            EventReceiver *receiver_,
            char const *program_name_,
            char const *command_line_,
            uint4 program_wait_timeout_ = UInt4_Max,
            int show_mode_ = 0,
            bool use_thread_ = true,
            bool detach_after_start_ = false,
            char const *verb_ = "open");

         /**
          * Destructor
          */
         virtual ~ProgramRunner();

         /**
          * Called to start the thread and the child process.
          */
         virtual void start() override;

         /**
          * Overloads the base class to block until the thread has closed.
          */
         virtual void wait_for_end() override;

         /**
          * @return Returns the child process exit code.
          */
         DWORD get_exit_code() const
         { return exit_code; }

         /**
          * @return Returns the name of the program to execute.
          */
         StrUni const &get_program_name() const
         { return program_name; }

         /**
          * Returns the command line parameters.
          */
         StrUni const &get_command_line() const
         { return command_line; }

         /**
          * @return Returns true if the process is to be killed when the thread exits.
          */
         bool get_kill_on_exit()
         { return kill_on_exit; }

         /**
          * @param kill Set to true if the child process is to be killed on exit.
          */
         void set_kill_on_exit(bool kill)
         { kill_on_exit = kill; }

         /**
          * @return Returns the working directory for the child process.
          */
         StrUni const &get_working_dir() const
         { return working_dir; }

         /**
          * Sets the working directory for the child process.
          */
         void set_working_dir(StrUni const &dir)
         { working_dir = dir; }

      protected:
         /**
          * Overloads the base class version to run the program.
          */
         virtual void execute() override;
         
      private:
         /**
          * Specifies the application object that will be notified on completion.
          */
         EventReceiver *receiver;
         
         /**
          * Specifies the name of the program to execute.
          */
         StrUni program_name;

         /**
          * Specifies the command line parameters for the program.
          */
         StrUni command_line;

         /**
          * Specifies the maximum interval, in milliseconds, tht this component will wait for the
          * child process to finish.
          */
         uint4 program_wait_timeout;

         /**
          * Specifies the mode that will control the way that the child process will display itself.
          */
         int show_mode;

         /**
          * Passed to ShellExecuteEx() to specify the verb to open the child process.
          */
         StrUni verb;

         /**
          * Set to true if the child process and the worker thread should be stopped.
          */
         bool should_quit;

         /**
          * Set to true if a worker thread should be used to launch the child process.
          */
         bool use_thread;

         /**
          * Set to true if we should detach from the child process after it has been started.
          */
         bool detach_after_start;

         /**
          * Set to true (the default value) if the child process must be killed when the worker
          * thread is forced to stop.
          */
         bool kill_on_exit;

         /**
          * Specifies the child process exit code.
          */
         DWORD exit_code;

         /**
          * Specifies the working directory for the child process.
          */
         StrUni working_dir;
      };
   };
};


#endif

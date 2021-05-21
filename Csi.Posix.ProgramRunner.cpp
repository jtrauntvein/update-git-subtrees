/* Csi.Posix.ProgramRunner.cpp

   Copyright (C) 2005, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 03 August 2005
   Last Change: Thursday 07 June 2012
   Last Commit: $Date: 2018-05-22 15:32:41 -0600 (Tue, 22 May 2018) $ 
   Last Changed by: $Author: cjensen $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Posix.ProgramRunner.h"
#include "Csi.Utils.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>


namespace Csi
{
   namespace Posix
   {
      ////////////////////////////////////////////////////////////
      // class ProgramRunner definitions
      ////////////////////////////////////////////////////////////
      ProgramRunner::ProgramRunner(
         EventReceiver *receiver_,
         char const *program_name_,
         char const *command_line_,
         uint4 program_wait_timeout_,
         int show_mode_,
         bool use_thread_,
         bool detach_after_start_,
         char const *verb_):
         receiver(receiver_),
         program_name(program_name_),
         command_line(command_line_),
         program_wait_timeout(program_wait_timeout_),
         use_thread(use_thread_),
         detach_after_start(detach_after_start_),
         kill_on_exit(true)
      { }


      ProgramRunner::~ProgramRunner()
      { wait_for_end(); }


      void ProgramRunner::start()
      {
         should_quit = false;
         if(use_thread)
            Thread::start();
         else
            execute();
      } // start


      void ProgramRunner::wait_for_end()
      {
         should_quit = true;
         Thread::wait_for_end();
      } // wait_for_end


      void ProgramRunner::execute()
      {
         // we will first fork the process.  All subsequent actions depend upon whether we are the
         // parent process or the child process
         pid_t child_id = fork();
         outcome_type outcome = outcome_aborted;
         int status = 0;
         if(child_id == 0)
         {
            // this is the child process.  We may need to change its working dir.
            if(working_dir.length())
               ::chdir(working_dir.c_str());

            // we will use the shell to parse the command line and execute the file.  In order to do
            // this, we need to get the path the shell program.  We can read this using getenv()
            StrAsc shell_name(getenv("SHELL"));
            StrAsc bash_command(program_name);
            
            if(shell_name.length() == 0)
               shell_name = "/bin/sh";
            bash_command.append(' ');
            bash_command += command_line;
            
            // we will now invoke the execl function using the shell as the program to execute.
            int rcd = execl(
               shell_name.c_str(),
               shell_name.c_str(),
               "-c",
               bash_command.c_str(),
               static_cast<char *>(0));
            if(rcd == -1)
               _exit(errno);
         }
         else if(child_id != -1)
         {
            // this is the parent process

            // we need to wait for the child process to terminate
            uint4 started_base = counter(0);
            while(!detach_after_start &&
                  !should_quit &&
                  (program_wait_timeout == UInt4_Max ||
                   counter(started_base) < program_wait_timeout))
            {
               int rcd = waitpid(child_id,&status,WNOHANG);
               if(rcd == -1 && errno != EINTR)
                  should_quit = true;
               else if(rcd == child_id)
                  outcome = outcome_terminated_normally;
               else
               {
                  // since we are using WNOHANG as a wait option, we could wind up polling very
                  // rapidly.  In order to prevent the thread from hogging the CPU, we will slow
                  // down by using usleep()
                  usleep(100000); 
               }
            }
         }
         else 
            outcome = outcome_start_failed;

         // the outcome should reflect the reason why the loop stopped execution.  If not terminated
         // normally and the detach flag was not speciifed, we need to signal the process to close
         if(!detach_after_start && outcome != outcome_terminated_normally && kill_on_exit)
            kill(child_id,SIGKILL);

         if(child_id != 0 && EventReceiver::is_valid_instance(receiver))
            event_program_ended::create_and_post(
               receiver, this, outcome, WEXITSTATUS(status));
      } // execute

      
      ////////////////////////////////////////////////////////////
      // class ProgramRunner::event_program_ended definitions
      ////////////////////////////////////////////////////////////
      uint4 const ProgramRunner::event_program_ended::event_id =
      Csi::Event::registerType("Csi::Win32::ProgramRunner::event_program_ended");


      void ProgramRunner::event_program_ended::create_and_post(
         EventReceiver *receiver,
         ProgramRunner *runner,
         outcome_type outcome,
         int exit_code)
      {
         try
         {
            event_program_ended *ev = new event_program_ended(
               receiver, runner, outcome, exit_code);
            ev->post();
         }
         catch(Event::BadPost &)
         { }
      } // create_and_post


      ProgramRunner::event_program_ended::event_program_ended(
         EventReceiver *receiver,
         ProgramRunner *runner_,
         outcome_type outcome_,
         int exit_code_):
         Event(event_id,receiver),
         runner(runner_),
         outcome(outcome_),
         exit_code(exit_code_)
      {
         if(outcome == outcome_start_failed)
         {
            OsException error("program start failed");
            start_error = error.what();
         }
      } // constructor
   };
};



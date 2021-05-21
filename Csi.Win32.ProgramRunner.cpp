/* Csi.Win32.ProgramRunner.cpp

   Copyright (C) 2002, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 16 December 2002
   Last Change: Thursday 17 September 2020
   Last Commit: $Date: 2020-09-17 09:41:29 -0600 (Thu, 17 Sep 2020) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Win32.ProgramRunner.h"
#include "Csi.Utils.h"
#include "Shellapi.h"


namespace Csi
{
   namespace Win32
   {
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
         program_name(program_name_, true),
         command_line(command_line_, true),
         program_wait_timeout(program_wait_timeout_),
         show_mode(show_mode_),
         use_thread(use_thread_),
         detach_after_start(detach_after_start_),
         verb(verb_, true),
         exit_code(0),
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
         // initialise the controlling structure
         SHELLEXECUTEINFOW info;
         outcome_type outcome = outcome_aborted;
         exit_code = 0;
         
         memset(&info,0,sizeof(info));
         info.cbSize = sizeof(info);
         info.fMask = SEE_MASK_DOENVSUBST | SEE_MASK_FLAG_NO_UI | SEE_MASK_NOCLOSEPROCESS;
         info.lpVerb = verb.c_str();
         info.lpFile = program_name.c_str();
         info.lpParameters = command_line.c_str();
         info.nShow = show_mode;
         if(working_dir.length() > 0)
            info.lpDirectory = working_dir.c_str();
         if(ShellExecuteExW(&info))
         {
            // we will assume here that the process handle must be returned
            if(info.hProcess != 0)
            {
               uint4 started_base = counter(0); 
               while(!detach_after_start &&
                     !should_quit &&
                     (program_wait_timeout == UInt4_Max ||
                      counter(started_base) < program_wait_timeout))
               {
                  // we will wait for a second and then check to see if we need to quit
                  uint4 rcd = WaitForSingleObject(info.hProcess,1000);
                  if(rcd == WAIT_OBJECT_0)
                  {
                     outcome = outcome_terminated_normally;
                     break;
                  }
                  else if(rcd == WAIT_ABANDONED)
                     break;
               }

               // the outcome should reflect why the loop was left.  If not terminated normally, we
               // need to terminate the process
               if(!detach_after_start && outcome != outcome_terminated_normally && kill_on_exit)
                  TerminateProcess(info.hProcess,0);
               if(!detach_after_start && outcome == outcome_terminated_normally)
                  GetExitCodeProcess(info.hProcess, &exit_code);
               CloseHandle(info.hProcess);
            }
            else
               outcome = outcome_start_failed;
         }
         else
            outcome = outcome_start_failed;

         // post the event
         if(EventReceiver::is_valid_instance(receiver))
            event_program_ended::create_and_post(receiver, this, outcome, static_cast<int4>(exit_code));
      } // execute


      uint4 const ProgramRunner::event_program_ended::event_id =
      Csi::Event::registerType("Csi::Win32::ProgramRunner::event_program_ended");


      void ProgramRunner::event_program_ended::create_and_post(
         EventReceiver *receiver,
         ProgramRunner *runner,
         outcome_type outcome,
         int4 exit_code)
      {
         try
         {
            event_program_ended *ev = new event_program_ended(receiver, runner, outcome, exit_code);
            ev->post();
         }
         catch(Event::BadPost &)
         { }
      } // create_and_post


      ProgramRunner::event_program_ended::event_program_ended(
         EventReceiver *receiver,
         ProgramRunner *runner_,
         outcome_type outcome_,
         int4 exit_code_):
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

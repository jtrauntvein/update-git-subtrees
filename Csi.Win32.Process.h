/* Csi.Win32.Process.h

    Copyright (C) 2020 Campbell Scientific, Inc.
    
    Written by: Andrew Mortenson
    Date Begun: Friday 2 October 2020
    Last Change: Friday 2 October 2020
    Last Commit: $Date: 2020-10-06 12:12:12 -0600 (Tue, 06 Oct 2020) $
    Last Changed by: $Author: jon $
*/

#ifndef Csi_Win32_Process_h
#define Csi_Win32_Process_h

#include "StrUni.h"
#include <deque>
#include "Csi.SharedPtr.h"

namespace Csi
{
    namespace Win32
    {
        /**
         * Component that encapsulates the detais of a process running on windows. 
         */
        class Process
        {
        private:
            /**
            * Parses the executable_full_name and sets the executable name and path 
            * attributes.
            */
            void  set_executable_name();
            /**
            * Specifies the full name of the path to the executable that started 
            * the process.
            */
            StrUni executable_full_name;
            /**
            * Specifies the name of the executable that started the process.
            */
            StrUni executable_name; 
            /**
            * Specifies the name of the path to the executable that started the 
            * process.
            */
            StrUni executable_path;
            /**
            * Specifies the id of the process
            */
            unsigned int process_id;
        public:
            /**
            * Constructor for querying information for the current runnig process.
            */
            Process();
            /**
            * Constructor for querying information about the process specified by the 
            * process_id_ parameter.
            */
            Process(unsigned int process_id_);
            /**
            * Copy Constructor
            */
            Process &operator =(Process const &other);
            /**
            * Gets the process id
            */
            unsigned int get_process_id() { return process_id; }
            /**
            * Get the name of the executable that started the process. If the executable 
            * did not have a name associated it will return an empty string.
            */
            StrUni get_exe_name() {
               if (executable_name == L"")
                  set_executable_name();
               return executable_name;
            }
            /**
            * Get the path of the executable that started the process. If the executable 
            * did not have a name associated it will return an empty string.
            */
            StrUni get_exe_path() {
               if (executable_path == L"")
                  set_executable_name();
               return executable_path;
            }
            /**
            * Get the full name of the executable that started the process. If the 
            * executable did not have a name associated it will return an empty string.
            */
            StrUni get_exe_full_name() {
               return executable_full_name;
            }
            /**
            * Gathers a list of all processes currently running.
            */
            typedef SharedPtr<Process> process_type;
            typedef std::deque<process_type> processes_type;
            static processes_type getAllProcesses();
        };
    }
}
#endif // Csi_Win32_Process_h
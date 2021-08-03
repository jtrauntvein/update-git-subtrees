/* Csi.Win32.Process.cpp

    Copyright (C) 2020 Campbell Scientific, Inc.
    
    Written by: Andrew Mortenson
    Date Begun: Friday 2 October 2020
    Last Change: Friday 2 October 2020
    Last Commit: $Date: 2020-10-06 12:12:12 -0600 (Tue, 06 Oct 2020) $
    Last Changed by: $Author: jon $
*/


#pragma hdrstop

#include "Csi.Win32.Process.h"
#include "Csi.StrAscStream.h"
#include <psapi.h>
#include "handleapi.h"

namespace Csi
{
    namespace Win32
    {
       Process::Process():
          executable_full_name(""),
          executable_name(""),
          executable_path("")
        {
            process_id = GetCurrentProcessId();
            HANDLE process_handle = GetCurrentProcess();
            wchar_t exe_name[1024];
            DWORD size = 1024;
            if
            (
               QueryFullProcessImageName(
                  process_handle,
                  0,
                  exe_name,
                  &size
               )
            ) 
            {
                executable_full_name = exe_name;
            }
            else
                throw std::invalid_argument("Failed to collect Process File Name");

            CloseHandle(process_handle);
        }

        Process::Process(unsigned int process_id_)
        {
            process_id = process_id_; 
            HANDLE process_handle = OpenProcess(
               PROCESS_VM_READ | PROCESS_QUERY_INFORMATION,
                true,
                process_id
            );

            wchar_t exe_name[1024];
            DWORD size = 1024;
            if
               (
                  QueryFullProcessImageName(
                     process_handle,
                     0,
                     exe_name,
                     &size
                  )
               )
            {
               executable_full_name = exe_name;
            }

            CloseHandle(process_handle);
        }

        Process& Process::operator=(Process const& other)
        {

           this->executable_full_name = other.executable_full_name;
           this->executable_name = other.executable_name;
           this->executable_path = other.executable_path;
           this->process_id = other.process_id;
           return *this;
        }

        void Process::set_executable_name() {
            
           if (executable_full_name != StrUni(""))
           {
               StrUni rtn;
               size_t str_pos = executable_full_name.findRev(L"\\");
               if (!(str_pos >= executable_full_name.length()))
               {
                  executable_name = executable_full_name;
                  executable_name.cut(0,str_pos+1);
                  executable_path = executable_full_name;
                  executable_path.cut(str_pos);
               }
               else
               {
                  throw std::invalid_argument("Unable to parse executable path");
               }
           }

        }

        Process::processes_type Process::getAllProcesses()
        {
            Process::processes_type rtn;
            DWORD processes[1024], bytes_returned; 
            if(!EnumProcesses(processes, sizeof(processes), &bytes_returned ))
                throw std::invalid_argument("Failed to enumerate processes");

            unsigned int num_processes = bytes_returned/ sizeof(DWORD);

            for(unsigned int i = 0; i < num_processes; i++)
            {
                Process::process_type process;  
                process.bind(new Process(processes[i]));
                rtn.push_back(process);
            }
            return rtn;
        }

    } // namespace Win32
} // namespace Csi

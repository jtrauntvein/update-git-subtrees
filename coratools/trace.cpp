/* trace.cpp

   Copyright (C) 1998, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 15 September 1998
   Last Change: Tuesday 10 July 2012
   Last Commit: $Date: 2014-05-16 14:47:29 -0600 (Fri, 16 May 2014) $ 
   Committed by: $Author: tmecham $
   
*/


#pragma hdrstop               // stop creation of precompiled header
#define _CRT_SECURE_NO_DEPRECATE 1
#include "trace.h"
#include "CsiTypeDefs.h"
#include <iostream>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX                // disable definition of min() and max()
#define _WINSOCKAPI_ // prevent windows.h from including winsock.h
#include <windows.h>
#else
#include "Csi.Utils.h"
#endif

namespace
{
   bool trace_enabled = true;
   char rpt_file_name[FILENAME_MAX];
   char rpt_version_info[FILENAME_MAX];
};


#ifdef __cplusplus
extern "C"
{
#endif

   void enable_trace(int trace_enabled_)
   {
      if(trace_enabled_)
         trace_enabled = true;
      else
         trace_enabled = false;
   } // enable_trace
   
   
   void trace(char const *format, ...)
   {
#ifdef _DEBUG
      if(trace_enabled)
      {
         char temp[1024];
         
         va_list args;
         va_start(args,format);
#ifdef _WIN32
#pragma warning(disable: 4996)
         _vsnprintf(temp,sizeof(temp),format,args);
#pragma warning(default: 4996)
#else
         vsnprintf(temp,sizeof(temp),format,args);
#endif
         va_end(args);
#ifdef _WIN32
         OutputDebugStringA(temp);
         OutputDebugStringA("\n");
#else
         std::cerr << temp << std::endl;
#endif
      }
#endif
   } // trace
   
   
   void traceSlot(char const *slotName, char const *msg)
   {
#ifdef _WIN32
#ifdef _DEBUG
      if(trace_enabled)
      {
         // the write only makes sense if we are writing out more than zero bytes
         uint4 len = strlen(msg);
         if(len > 0)
         {
            // Attempt to open the mail slot. This will only succeed if a client already has already
            // created the slot.
            HANDLE slot;
            slot = CreateFileA(
               slotName,
               GENERIC_WRITE, // open for write access
               FILE_SHARE_READ, // subsequent open will succeed only if read is requested 
               0,         // default security attributes
               OPEN_EXISTING, // open an existing slot
               FILE_ATTRIBUTE_NORMAL, // no special attributes
               0);        // no template file
            if(slot != INVALID_HANDLE_VALUE)
            {
               uint4 count;
               
               WriteFile(slot,msg,len,&count,0);
               CloseHandle(slot);
            }
         }
      }
#endif
#endif
   } // traceSlot
   
   
   void set_rpt_file_name(char const *rpt_file_name_)
   {
      memset(rpt_file_name,0,sizeof(rpt_file_name));
      strncpy(rpt_file_name,rpt_file_name_,sizeof(rpt_file_name) - 1);
   } // set_rpt_file_name
   
   
   char const *get_rpt_file_name()
   {
      if(rpt_file_name[0] == 0)
      {
#ifdef _WIN32
         char module_file_path[FILENAME_MAX];
         char drive[_MAX_DRIVE], dir[_MAX_DIR], name[_MAX_FNAME], ext[_MAX_EXT];
         
         ::GetModuleFileNameA(
            ::GetModuleHandle(0),
            module_file_path,
            sizeof(module_file_path));
         _splitpath(module_file_path,drive,dir,name,ext);
         _makepath(rpt_file_name,drive,dir,name,".rpt");
#else
         StrAsc program_path;
         size_t last_slash_pos;
         size_t period_pos;
         
         Csi::get_program_path(program_path);
         last_slash_pos = program_path.rfind("\\");
         if(last_slash_pos >= program_path.length())
            last_slash_pos = program_path.rfind("/");
         period_pos = program_path.rfind(".");
         if(period_pos > last_slash_pos)
            program_path.cut(period_pos);
         program_path.append(".rpt");
         memset(rpt_file_name,0,sizeof(rpt_file_name));
         strncpy(rpt_file_name,program_path.c_str(),sizeof(rpt_file_name) - 1);
#endif 
      }
      return rpt_file_name;
   } // get_rpt_file_name
   
   
   void set_rpt_version_info(char const *rpt_version_info_)
   {
      memset(rpt_version_info,0,sizeof(rpt_version_info));
      strncpy(rpt_version_info,rpt_version_info_,sizeof(rpt_version_info) - 1);
   } // set_rpt_version_info
   
   
   char const *get_rpt_version_info()
   {
      return rpt_version_info;
   } // get_rpt_version_info
   
   
   FILE *open_rpt_file()
   {
      // we need to put together a file name from the module handle
      FILE *rtn = fopen(get_rpt_file_name(),"at");
      if(rtn)
      {
         time_t system_time = time(0);
         struct tm *system_time_ptr = localtime(&system_time);
         fprintf(
            rtn,
            "\n\n////////////////////////////////////////////////////////////"
            "\nCurrent time: %s",
            asctime(system_time_ptr));
         if(rpt_version_info[0] != 0)
         {
            fprintf(
               rtn,
               "Version info: %s\n",
               rpt_version_info);
         }
      }
      return rtn;
   } // open_rpt_file


#ifdef __cplusplus
}
#endif

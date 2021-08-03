/* Cora.Win32.CoralibLoader.cpp

   Copyright (C) 2003, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 24 October 2003
   Last Change: Wednesday 15 January 2020
   Last Commit: $Date: 2020-10-30 11:07:15 -0600 (Fri, 30 Oct 2020) $ 
   Last Changed by: $Author: amortenson $

*/

#pragma hdrstop               // stop creation of precompiled header

#include "Cora.Win32.CoralibLoader.h"
#include "Csi.OsException.h"
#include <dbt.h>


namespace Cora
{
   namespace Win32
   {
      namespace
      {
         CoralibLoader *the_loader = 0;
      };

   
      CoralibLoader::CoralibLoader(char const *dll_name):
         server_was_started(false),
         coralib_module(0)
      {
         if(the_loader == 0)
         {
            coralib_module = ::LoadLibraryA(dll_name);
            if(coralib_module == 0)
            {
               StrAsc msg("CoralibLoader: LoadLibrary failed to load DLL ");
               msg.append(dll_name);
               throw Csi::OsException(msg.c_str());
            }
         }
         else
            throw Csi::MsgExcept("CoralibLoader should be a singleton");
      } // constructor

   
      CoralibLoader::~CoralibLoader()
      {
         if(coralib_module)
         {
            stop_server();
            ::FreeLibrary(coralib_module);
            coralib_module = 0;
         }
         the_loader = 0;
      } // destructor
   
     
      void CoralibLoader::set_working_directory(char const *working_directory)
      {
         typedef int (__stdcall function_type)(char const *);
         function_type *function = reinterpret_cast<function_type *>(
            GetProcAddress(
               coralib_module,
               "cora_set_working_directory"));
         if(function != 0)
         {
            if(function(working_directory) == 0)
               throw Csi::MsgExcept("Invalid server state");
         }
         else
            throw Csi::OsException("set_working_directory");
      } // set_working_directory

   
      StrAsc CoralibLoader::get_working_directory()
      {
         typedef char * (__stdcall function_type)(char *, uint4);
         function_type *function = reinterpret_cast<function_type *>(
            GetProcAddress(
               coralib_module,
               "cora_get_working_directory"));
         StrAsc rtn;
      
         if(function != 0)
         {
            char path[MAX_PATH];
            function(path,sizeof(path));
            rtn = path;
         }
         else
            throw Csi::OsException("get_working_directory");
         return rtn;
      } // get_working_directory

      StrAsc CoralibLoader::get_cache_directory()
      {
         typedef char * (__stdcall function_type)(char *, uint4);
         function_type *function(
            reinterpret_cast<function_type *>(
               ::GetProcAddress(coralib_module, "cora_get_cache_directory")));
         StrAsc rtn;
         if(function != 0)
         {
            char path[MAX_PATH];
            function(path, sizeof(path));
            rtn = path;
         }
         else
            throw Csi::OsException("get_cache_directory");
         return rtn;
      } // get_cache_directory

      void CoralibLoader::set_cache_directory(char const *value)
      {
         typedef int (__stdcall *function_type)(char const *);
         function_type function(
            reinterpret_cast<function_type>(
               ::GetProcAddress(coralib_module, "cora_set_cache_directory")));
         if(function != 0)
         {
            if(!function(value))
               throw Csi::MsgExcept("cora_set_cache_directory");
         }
         else
            throw Csi::OsException("cora_set_cache_directory");
      } // set_cache_directory
   
      void CoralibLoader::set_app_directory(char const *app_directory)
      {
         typedef int (__stdcall *function_type)(char const *);
         function_type function = reinterpret_cast<function_type>(
            ::GetProcAddress(
               coralib_module,
               "cora_set_app_directory"));
         if(function)
         {
            if(!function(app_directory))
               throw Csi::MsgExcept("set_app_directory failure");
         }
         else
            throw Csi::OsException("set_app_directory failure");
      } // set_app_directory

   
      StrAsc CoralibLoader::get_app_directory()
      {
         typedef char * (__stdcall *function_type)(char *, uint4);
         function_type function = reinterpret_cast<function_type>(
            ::GetProcAddress(
               coralib_module,
               "cora_get_app_directory"));
         StrAsc rtn;
      
         if(function != 0)
         {
            char path[MAX_PATH];
            function(path,sizeof(path));
            rtn = path;
         }
         else
            throw Csi::OsException("get_app_directory");
         return rtn;
      } // get_app_directory

   
      void CoralibLoader::set_log_file_directory(char const *log_file_directory)
      {
         typedef int (__stdcall *function_type)(char const *);
         function_type function = reinterpret_cast<function_type>(
            ::GetProcAddress(
               coralib_module,
               "cora_set_log_file_directory"));
         if(function != 0)
         {
            if(!function(log_file_directory))
               throw Csi::MsgExcept("set_log_file_directory failure");
         }
         else
            throw Csi::OsException("set_log_file_directory failure");
      } // set_log_file_directory

   
      StrAsc CoralibLoader::get_log_file_directory()
      {
         typedef char * (__stdcall *function_type)(char *, uint4);
         function_type function = reinterpret_cast<function_type>(
            ::GetProcAddress(
               coralib_module,
               "cora_get_log_file_directory"));
         StrAsc rtn;
      
         if(function != 0)
         {
            char path[MAX_PATH];
            function(path,sizeof(path));
            rtn = path;
         }
         else
            throw Csi::OsException("get_log_file_directory");
         return rtn;
      } // get_log_file_directory

   
      void CoralibLoader::set_security_feature_enabled(bool should_enforce)
      {
         typedef int (__stdcall *function_type)(int);
         function_type function = reinterpret_cast<function_type>(
            ::GetProcAddress(
               coralib_module,
               "cora_set_should_enforce_security"));
         if(function != 0)
         {
            if(function(should_enforce) == 0)
               throw Csi::MsgExcept("set_security_feature_enabled failure");
         }
         else
            throw Csi::OsException("set_security_feature_enabled failure");
      } // set_security_feature_enabled

   
      bool CoralibLoader::get_should_enforce_security()
      {
         typedef int (__stdcall *function_type)();
         function_type function = reinterpret_cast<function_type>(
            GetProcAddress(
               coralib_module,
               "cora_get_should_enforce_security"));
         bool rtn;
      
         if(function != 0)
            rtn = function() ? true : false;
         else
            throw Csi::OsException("get_should_enforce_security");
         return rtn;
      } // get_should_enforce_security

   
      void CoralibLoader::set_should_support_tcp(bool should_support)
      {
         typedef int (__stdcall *function_type)(int);
         function_type function = reinterpret_cast<function_type>(
            ::GetProcAddress(
               coralib_module,
               "cora_set_should_support_ip"));
         if(function != 0)
         {
            if(function(should_support) == 0)
               throw Csi::MsgExcept("set_should_support_tcp failure");
         }
         else
            throw Csi::OsException("set_should_support_tcp failure");
      } // set_should_support_tcp

   
      bool CoralibLoader::get_should_support_tcp()
      {
         typedef int (__stdcall *function_type)();
         function_type function = reinterpret_cast<function_type>(
            GetProcAddress(
               coralib_module,
               "cora_get_should_support_ip"));
         bool rtn;
      
         if(function != 0)
            rtn = function() ? true : false;
         else
            throw Csi::OsException("get_should_support_tcp");
         return rtn;
      } // get_should_support_tcp

   
      void CoralibLoader::set_tcp_port(uint2 tcp_port)
      {
         typedef int (__stdcall *function_type)(uint2);
         function_type function = reinterpret_cast<function_type>(
            ::GetProcAddress(
               coralib_module,
               "cora_set_ip_port"));
         if(function != 0)
         {
            if(function(tcp_port) == 0)
               throw Csi::MsgExcept("set_tcp_port failure");
         }
         else
            throw Csi::OsException("set_tcp_port failure");
      } // set_tcp_port

   
      uint2 CoralibLoader::get_tcp_port()
      {
         typedef uint2 (__stdcall function_type)();
         function_type *function = reinterpret_cast<function_type *>(
            GetProcAddress(
               coralib_module,
               "cora_get_ip_port"));
         uint2 rtn;
      
         if(function != 0)
            rtn = function();
         else
            throw Csi::OsException("get_tcp_port");
         return rtn;
      } // get_tcp_port

   
      void CoralibLoader::set_tcp_bind_address(char const *address)
      {
         typedef int (__stdcall *function_type)(char const *);
         function_type function = reinterpret_cast<function_type>(
            ::GetProcAddress(
               coralib_module,
               "cora_set_ip_bind_address"));
         if(function == 0 || function(address) == 0)
            throw Csi::MsgExcept("set_tcp_binbd_address failure");
      } // set_tcp_bind_address

   
      StrAsc CoralibLoader::get_tcp_bind_address()
      {
         typedef char * (__stdcall function_type)(char *, uint4);
         function_type *function = reinterpret_cast<function_type *>(
            GetProcAddress(
               coralib_module,
               "cora_get_ip_bind_address"));
         StrAsc rtn;
      
         if(function != 0)
         {
            char path[25];
            function(path,sizeof(path));
            rtn = path;
         }
         else
            throw Csi::OsException("get_tcp_bind_address");
         return rtn;
      } // get_tcp_bind_address

   
      CoralibLoader::add_relation_rtn CoralibLoader::add_relation(
         uint4 parent_type,
         uint4 child_type)
      {
         add_relation_rtn rtn = cora_add_relation_unsupported;
         typedef int (__stdcall *function_type)(uint4, uint4);
         function_type function = reinterpret_cast<function_type>(
            ::GetProcAddress(
               coralib_module,
               "cora_add_relation"));
         if(function != 0)
         {
            rtn = static_cast<add_relation_rtn>(
               function(parent_type,child_type));
         }
         else
            throw Csi::MsgExcept("add_relation failure");
         return rtn;
      } // add_relation

   
      void CoralibLoader::use_default_creation_map()
      {
         typedef int (__stdcall *function_type)();
         function_type function(
            (function_type)::GetProcAddress(
               coralib_module,
               "cora_use_default_creation_map"));
         if(function != 0)
         {
            if(function() == 0)
               throw Csi::MsgExcept("use_default_creation_map failure"); 
         }
         else
            throw Csi::OsException("use_default_creation_map failure");
      } // use_default_creation_map


      void CoralibLoader::set_lgrnet_plus_mode(bool plus_mode)
      {
         typedef int (__stdcall *function_type)(int);
         function_type function = reinterpret_cast<function_type>(
            ::GetProcAddress(
               coralib_module,
               "cora_set_lgrnet_plus_mode"));
         if(function != 0)
         {
            if(function(plus_mode ? 1 : 0) == 0)
               throw Csi::MsgExcept("set_lgrnet_plus_mode failure");
         }
         else
            throw Csi::OsException("set_lgrnet_plus_mode failure");
      } // set_lgrnet_plus_mode


      bool CoralibLoader::get_lgrnet_plus_mode()
      {
         typedef int (__stdcall *function_type)();
         function_type function(
            (function_type)::GetProcAddress(
               coralib_module,
               "cora_get_lgrnet_plus_mode"));
         bool rtn = false;
      
         if(function != 0)
            rtn = function() ? true : false;
         else
            throw Csi::OsException("get_lgrnet_plus_mode failure");
         return rtn;
      } // get_lgrnet_plus_mode


      void CoralibLoader::set_tasks_supported(bool supported)
      {
         typedef int (__stdcall *function_type)(int);
         function_type function = reinterpret_cast<function_type>(
            ::GetProcAddress(
               coralib_module,
               "cora_set_tasks_supported"));
         if(function != 0)
         {
            if(function(supported ? 1 : 0) == 0)
               throw Csi::MsgExcept("set_tasks_supported failure");
         }
         else
            throw Csi::OsException("set_tasks_supported failure");
      } // set_tasks_supported


      bool CoralibLoader::get_tasks_supported()
      {
         typedef int (__stdcall *function_type)();
         function_type function = (function_type)::GetProcAddress(
            coralib_module,
            "cora_get_tasks_supported");
         bool rtn = false;
      
         if(function != 0)
            rtn = function() ? true : false;
         else
            throw Csi::OsException("get_tasks_supported failure");
         return rtn;
      } // get_tasks_supported


      bool CoralibLoader::get_ipv6_service_enabled()
      {
         typedef int (__stdcall *function_type)();
         function_type function(
            (function_type)::GetProcAddress(coralib_module, "cora_get_ipv6_service_enabled"));
         bool rtn(false);
         if(function != 0)
            rtn = function() ? true : false;
         return rtn;
      } // get_ipv6_service_enabled


      void CoralibLoader::set_ipv6_service_enabled(bool enabled)
      {
         typedef int (__stdcall *function_type)(int);
         function_type function(
            reinterpret_cast<function_type>(
               ::GetProcAddress(coralib_module, "cora_set_ipv6_service_enabled")));
         if(function != 0)
            function(enabled ? 1 : 0);
      } // set_ipv6_service_enabled


      namespace
      {
         uint4 const event_server_stopped =
            ::RegisterWindowMessageW(L"Cora::CoralibLoader::event_server_stopped");
      

         void __stdcall on_server_thread_stop()
         {
            if(CoralibLoader::is_valid_instance(the_loader))
               the_loader->post_message(event_server_stopped,0,0);
         } // on_server_thread_stop
      };

   
      void CoralibLoader::start_server()
      {
         if(!server_was_started)
         {
            // we need to capture the function pointers for cora_start_server, cora_register_stop_hook,
            // and cora_get_start_server_error
            typedef int (__stdcall *start_func_type)();
            typedef int (__stdcall *register_hook_type)(void (__stdcall *)());
            typedef char *(__stdcall *get_error_type)(char *,uint4);
            start_func_type start_func;
            register_hook_type register_hook;
            get_error_type get_error;
         
            start_func = (start_func_type)::GetProcAddress(
               coralib_module, "cora_start_server");
            if(start_func == 0)
               throw Csi::OsException("start_server");
            register_hook = reinterpret_cast<register_hook_type>(
               ::GetProcAddress(
                  coralib_module,
                  "cora_register_stop_hook"));
            if(register_hook == 0)
               throw Csi::OsException("start_server");
            get_error = reinterpret_cast<get_error_type>(
               ::GetProcAddress(
                  coralib_module,
                  "cora_get_start_server_error"));
            if(get_error == 0)
               throw Csi::OsException("start_server");

            // now we can go ahead and start the server
            register_hook(on_server_thread_stop);
            if(start_func())
               server_was_started = true;
            else
            {
               char failure_reason[256];
               get_error(failure_reason,sizeof(failure_reason));
               throw Csi::MsgExcept(failure_reason);
            }
         }
         else
            throw Csi::MsgExcept("server already started");
      } // start_server

   
      void CoralibLoader::start_server_without_automation()
      {
         if(!server_was_started)
         {
            // we need to capture the function pointers for cora_start_server, cora_register_stop_hook,
            // and cora_get_start_server_error
            typedef int (__stdcall *start_func_type)();
            typedef int (__stdcall *register_hook_type)(void (__stdcall *)());
            typedef char *(__stdcall *get_error_type)(char *,uint4);
            start_func_type start_func;
            register_hook_type register_hook;
            get_error_type get_error;
         
            start_func = (start_func_type)::GetProcAddress(
               coralib_module,
               "cora_start_server_without_automation");
            if(start_func == 0)
               throw Csi::OsException("start_server_without_automation");
            register_hook = reinterpret_cast<register_hook_type>(
               ::GetProcAddress(
                  coralib_module,
                  "cora_register_stop_hook"));
            if(register_hook == 0)
               throw Csi::OsException("start_server_without_automation");
            get_error = reinterpret_cast<get_error_type>(
               ::GetProcAddress(
                  coralib_module,
                  "cora_get_start_server_error"));
            if(get_error == 0)
               throw Csi::OsException("start_server_without_automation");

            // now we can go ahead and start the server
            register_hook(on_server_thread_stop);
            if(start_func())
               server_was_started = true;
            else
            {
               char failure_reason[256];
               get_error(failure_reason,sizeof(failure_reason));
               throw Csi::MsgExcept(failure_reason);
            }
         }
         else
            throw Csi::MsgExcept("server already started");
      } // start_server_without_automation

   
      void CoralibLoader::enable_automation()
      {
         typedef int (__stdcall *enable_automation_type)();
         enable_automation_type enable_automation = (enable_automation_type)::GetProcAddress(
            coralib_module,
            "cora_enable_automation");

         if(enable_automation != 0)
         {
            if(enable_automation() == 0)
               throw Csi::MsgExcept("enable_automation failure");
         }
         else
            throw Csi::OsException("enable_automation failure");
      } // enable_automation

   
      void CoralibLoader::stop_server()
      {
         if(server_was_started)
         {
            typedef void (__stdcall *stop_server_type)();
            stop_server_type stop_server = reinterpret_cast<stop_server_type>(
               ::GetProcAddress(
                  coralib_module,
                  "cora_stop_server"));
         
            if(stop_server != 0)
               stop_server();
            server_was_started = false;
         }
      } // stop_server

   
      uint4 CoralibLoader::get_current_connection_count()
      {
         typedef uint4 (__stdcall function_type)();
         function_type *function = reinterpret_cast<function_type *>(
            GetProcAddress(
               coralib_module,
               "cora_get_current_connection_count"));
         uint4 rtn;
      
         if(function != 0)
            rtn = function();
         else
            throw Csi::OsException("get_tcp_port");
         return rtn;
      } // get_current_connection_count

   
      StrAsc CoralibLoader::get_version()
      {
         typedef char * (__stdcall function_type)();
         function_type *function = reinterpret_cast<function_type *>(
            GetProcAddress(
               coralib_module,
               "cora_get_version"));
         StrAsc rtn;
      
         if(function != 0)
            rtn = function();
         else
            throw Csi::OsException("get_version");
         return rtn;
      } // get_version

   
      StrAsc CoralibLoader::get_build_date()
      {
         typedef char * (__stdcall function_type)();
         function_type *function = reinterpret_cast<function_type *>(
            GetProcAddress(
               coralib_module,
               "cora_get_build_date"));
         StrAsc rtn;
      
         if(function != 0)
            rtn = function();
         else
            throw Csi::OsException("get_build_date");
         return rtn;
      } // get_build_date

   
      StrAsc CoralibLoader::get_server_name()
      {
         StrAsc rtn("coralib3.dll");
         typedef char const * (__stdcall function_type)();
         function_type *function = reinterpret_cast<function_type *>(
            GetProcAddress(
               coralib_module,
               "cora_get_server_name"));
         if(function != 0)
            rtn = function();
         return rtn;
      } // get_server_name


      uint4 CoralibLoader::get_max_stations_count()
      {
         uint4 rtn(0xffffffff);
         typedef unsigned int (__stdcall function_type)();
         function_type *function(
            reinterpret_cast<function_type *>(
               ::GetProcAddress(coralib_module, "cora_get_max_stations_count")));
         if(function)
            rtn = static_cast<uint4>(function());
         return rtn;
      } // get_max_stations_count


      void CoralibLoader::set_max_stations_count(uint4 count)
      {
         typedef int * (__stdcall function_type)(unsigned int value);
         function_type *function(
            reinterpret_cast<function_type *>(
               ::GetProcAddress(coralib_module, "cora_set_max_stations_count")));
         if(function)
            function(count);
      } // set_max_stations_count
      
      bool CoralibLoader::get_server_initialised()
      {
         bool rtn(false);
         typedef int * (__stdcall function_type)();
         function_type *function(
            reinterpret_cast<function_type *>(
               ::GetProcAddress(coralib_module, "cora_server_initialised")));
         if(function)
            rtn = function();
         return rtn;
      } // get_server_initialised
   
      LRESULT CoralibLoader::on_message(
         uint4 message_id,
         WPARAM wparam,
         LPARAM lparam)
      {
         LRESULT rtn = 0;
         if(message_id == event_server_stopped && server_was_started)
         {
            typedef char *(__stdcall *get_error_type)(char *, uint4);
            get_error_type get_error = reinterpret_cast<get_error_type>(
               ::GetProcAddress(
                  coralib_module,
                  "cora_get_start_server_error"));
            if(get_error != 0)
            {
               char reason[256];
               get_error(reason,sizeof(reason));
               on_server_failed(reason);
            }
            server_was_started = false;
         }
         else if(message_id == WM_DEVICECHANGE)
         {
            typedef void (__stdcall *function_type)();
            function_type function(
               reinterpret_cast<function_type>(
                  ::GetProcAddress(
                     coralib_module, "cora_on_device_change")));
            if(function)
               function();
            rtn = TRUE;
         }
         else
            rtn = MessageWindow::on_message(message_id,wparam,lparam);
         return rtn;
      } // on_message
   };
};   

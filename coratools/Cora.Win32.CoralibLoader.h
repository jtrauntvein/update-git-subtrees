/* Cora.Win32.CoralibLoader.h

   Copyright (C) 2003, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 24 October 2003
   Last Change: Tuesday 13 October 2020
   Last Commit: $Date: 2020-10-30 11:07:15 -0600 (Fri, 30 Oct 2020) $ 
   Last Changed by: $Author: amortenson $

*/

#ifndef Cora_Win32_CoralibLoader_h
#define Cora_Win32_CoralibLoader_h

#include "CsiTypeDefs.h"
#include "Csi.MessageWindow.h"
#include "StrAsc.h"


namespace Cora
{
   namespace Win32
   {
      /**
       * Defines an object that loads the LoggerNet server DLL and exposes its entry points as
       * methods.  The DLL will be loaded in the constructor and the DLL functions will be
       * dynamically mapped.  This object will also keep track of the running state of the server so
       * that the application can get notifications (via virtual methods) of server errors.
       */
      class CoralibLoader: public Csi::MessageWindow
      {
      public:
         /**
          * Constructor
          *
          * @param dll_name Specifies the name and, optionally, the path of the server DLL..
          */
         CoralibLoader(char const *dll_name = "coralib3.dll");
         
         /**
          * Destructor
          */
         virtual ~CoralibLoader();
         
         /**
          * @param working_directory Specifies the working directory for the server.
          */
         void set_working_directory(char const *working_directory);

         /**
          * @return Returns the server working directory.
          */
         StrAsc get_working_directory();

         /**
          * @return Returns the server cache table file directory.
          */
         StrAsc get_cache_directory();

         /**
          * @param value Specifies the cache table file directory.
          */
         void set_cache_directory(char const *value);

         /**
          * @param app_directory Specifis the server application directtory.
          */
         void set_app_directory(char const *app_directory);

         /**
          * @return Returns the server application directory.
          */
         StrAsc get_app_directory();

         /**
          * @param log_file_directory Specifies the path for the server log files.
          */
         void set_log_file_directory(char const *log_file_directory);
         
         /**
          * @return Returns the server log file directory.
          */
         StrAsc get_log_file_directory();

         /**
          * @param should_enforce Set to true if the server should enforce security.
          */
         void set_security_feature_enabled(bool should_enforce);
         
         /**
          * @return Returns true if the server should enforce security.
          */
         bool get_should_enforce_security();

         /**
          * @param should_support Set to true if the server should support client connections over
          * TCP.
          */
         void set_should_support_tcp(bool should_support);
         
         /**
          * @return Returns true if the server should support client connections over TCP.
          */
         bool get_should_support_tcp();

         /**
          * @param tcp_port Specifies the TCP port on which the server should accept client
          * connections.
          */
         void set_tcp_port(uint2 tcp_port);
         
         /**
          * @return Returns the TCP port on which the server will accept client connections.
          */
         uint2 get_tcp_port();

         /**
          * @param address Set to "" or " " if the server is to allow connections on all
          * interfaces.  Set to other if the server is to allow connections only on loopback
          * interfaces.
          */
         void set_tcp_bind_address(char const *address);

         /**
          * @return Returns the tcp bind address.
          */
         StrAsc get_tcp_bind_address();

         /**
          * Creates a parent/child type relation in the server's device creation map.  This map is
          * used to determine what devices can be created as well as what connections are allowed
          * between devices.  In order to use the default map, the application should invoke
          * use_default_creation_map().  Will throw a std::exception derived object if the server
          * thread has already been started.
          *
          * @param parent_type Specifies the parent device type.  Set to zero if the device type is
          * a root level device.
          *
          * @param child_type Specifies the child device type.
          */
         enum add_relation_rtn
         {
            cora_add_relation_success = 1,
            cora_add_relation_already_started = 2,
            cora_add_relation_unsupported = 3
         };
         add_relation_rtn add_relation(uint4 parent_type, uint4 child_type);

         /**
          * Sets up the server's default creation map (generally all allowable connections and
          * devices).  Will throw a std::exception derived object if the server thread has already
          * been started.
          */
         void use_default_creation_map();

         /**
          * @param plus_mode Set to true if loggernet admin features should be enabled.
          */
         void set_lgrnet_plus_mode(bool plus_mode);

         /**
          * @return Returns true if loggernet admin features should be enabled.
          */
         bool get_lgrnet_plus_mode();

         /**
          * @return Returns true if the task engine should be active.
          */
         bool get_tasks_supported();

         /**
          * @param supported Set to true if the task engine should be active.
          */
         void set_tasks_supported(bool supported);

         /**
          * Starts the server running in its own thread.  Will throw an object derived from class
          * std::exception if the start attempt fails.
          */
         void start_server();

         /**
          * Starts the server thread but leaves automatic operations such as scheduled data
          * collection disabled.  Automatic operations will remain disabled until
          * enable_automation() is invoked.  Will throw a std::exception derived object if the start
          * attempt fails.
          */
         void start_server_without_automation();

         /**
          * Enables automatic operations such as scheduled data collection for the server.  Unlike
          * most other feature enabling methods, this method must be invoked AFTER the server thread
          * has been started with start_server_without_automation().  Will throw a std::exception
          * derived object if the call fails.
          */
         void enable_automation();

         /**
          * Stops the server thread if it is currently running.  Will have no effect if the server
          * thread is not already running.
          */
         void stop_server();

         /**
          * @return Returns the number of client connections that the server is currently servicing.
          */
         uint4 get_current_connection_count();

         /**
          * @return Returns the server's version string.
          */
         StrAsc get_version();

         /**
          * @return Returns the server's build date string in the following form:
          *
          *  "$Date: " year "/" month "/" day "
          *  " hour ":" minute ":" second " $
          */
         StrAsc get_build_date();

         /**
          * @return Returns true if the server thread is currently running
          */
         bool server_is_started()
         { return server_was_started; }
      
         /**
          * @return Returns a string that identifiers the name and version of the server.
          */
         StrAsc get_server_name();

         /**
          * @return Returns true if IPv6 client connections are enabled.
          */
         bool get_ipv6_service_enabled();

         /**
          * @param enabled Set to true if IPv6 connections are to be enabled.
          */
         void set_ipv6_service_enabled(bool enabled);

         /**
          * @return Returns the maximum stations count.
          */
         uint4 get_max_stations_count();

         /**
          * @param count Specifies the new value for the max stations count.
          */
         void set_max_stations_count(uint4 count);

         /**
         * @return Returns true if the servers constructor has completed.
         */
         bool get_server_initialised();
         
         /**
          * @return Returns the server DLL module handle.
          */
         HMODULE get_coralib_module()
         { return coralib_module; }

      protected:
         /**
          * Invoked if the server thread stopped without the application invoking
          * stop_server().
          *
          * @param failure_reason Specifies a string the describes the reason for the failure.
          */
         virtual void on_server_failed(StrAsc const &failure_reason)
         { }
      
         /**
          * Overloads the base class version to handle asynch messages for this object.
          */
         virtual LRESULT on_message(
            uint4 message_id,
            WPARAM wparam,
            LPARAM lparam);

      private:
         /**
          * Identifies the coralib DLL handle after it has been loaded.  This
          * handle is used to for unloading in the destructor as well as looking
          * up function calls.
          */
         HMODULE coralib_module;

         /**
          * Keeps track of whether the server was started by the application.
          */
         bool server_was_started;
      };
   };
};


#endif

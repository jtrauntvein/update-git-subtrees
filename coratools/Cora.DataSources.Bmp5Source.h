/* Cora.DataSources.Bmp5Source.h

   Copyright (C) 2016, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Monday 03 October 2016
   Last Change: Saturday 05 May 2018
   Last Commit: $Date: 2018-05-05 10:28:34 -0600 (Sat, 05 May 2018) $
   Last Changed by: $Author: jon $

*/

#ifndef Cora_DataSources_Bmp5Source_h
#define Cora_DataSources_Bmp5Source_h

#include "Cora.DataSources.SourceBase.h"
#include "Csi.PakBus.Router.h"
#include "Csi.PakBus.SerialPort.h"
#include "Csi.PakBus.TcpSerialPort.h"
#include "Scheduler.h"


namespace Cora
{
   namespace DataSources
   {
      namespace Bmp5SourceHelpers
      {
         class SourceRouter;
         class SourceOperation;
         class OpGetTableDefs;
         class OpDataRequest;
      };

      
      /**
       * Defines a data source that uses the BMP5 protocol with a serial connection to get data
       * directly from the datalogger.
       */
      class Bmp5Source: public SourceBase, public OneShotClient, public SchedulerClient
      {
      public:
         /**
          * Constructor
          *
          * @param name_ Specifies the name for this data source.
          */
         Bmp5Source(StrUni const &name);

         /**
          * Destructor
          */
         virtual ~Bmp5Source();

         /**
          * Overloads the base class method to connect to the datalogger.
          */
         virtual void connect();

         /**
          * Overloads the base class method to disconnect from the datalogger.
          */
         virtual void disconnect();

         /**
          * @return Returns true if we have an active connection to the datalogger.
          */
         virtual bool is_connected() const;

         /**
          * @param prop_xml Specifies an XML structure that contains the properties for this
          * source. 
          */
         virtual void set_properties(Csi::Xml::Element &prop_xml);

         /**
          * @param prop_xml Specifies an XML element to which the properties for this source will be
          * written.
          */
         virtual void get_properties(Csi::Xml::Element &prop_xml);

         /**
          * Overloads the base class version to add the specified data request.
          */
         virtual void add_request(request_handle &request, bool more_to_follow = false);

         /**
          * Overlods the base class version to remove the specified request.
          */
         virtual void remove_request(request_handle &request);

         /**
          * Overloads the base class version to remove all current requests.
          */
         virtual void remove_all_requests();

         /**
          * Overloads the base class version to activate any pending requests.
          */
         virtual void activate_requests();

         /**
          * Overloads the base class version to stop this source.
          */
         virtual void stop();

         /**
          * Overloads the base class to set the manager for this source.
          */
         virtual void set_manager(Manager *manager);

         /**
          * Overloads the base class version to initiate a set version operation.
          */
         virtual bool start_set_value(SinkBase *sink, StrUni const &uri, ValueSetter const &value);

         /**
          * Overloads the base class version to start the process of sending a file.
          */
         virtual bool start_send_file(
            SinkBase *sink,
            StrUni const &uri,
            StrUni const &dest_file_name,
            StrUni const &file_name);

         /**
          * Overloads the base class version to start the process of retrieving a file.
          */
         virtual bool start_get_newest_file(
            SinkBase *sink,
            StrUni const &uri,
            StrUni const &pattern);

         /**
          * Overloads the base class version to start a clock check operation.
          */
         virtual bool start_clock_check(
            SinkBase *sink, StrUni const &uri, bool should_set, bool send_server_time, Csi::LgrDate const &server_time);

         /**
          * Overloads the base class version to start a file control operation.
          */
         virtual bool start_file_control(
            SinkBase *sink,
            StrUni const &uri,
            uint4 command,
            StrAsc const &p1,
            StrAsc const &p2);

         /**
          * Overloads the base class version to start a terminal transaction with the datalogger.
          */
         virtual bool start_terminal(
            TerminalSinkBase *sink, StrUni const &uri, int8 sink_token);

         /**
          * Overloads the base class version to send data on the specified terminal transaction.
          */
         virtual void send_terminal(
            TerminalSinkBase *sink, int8 sink_token, void const *buff, size_t buff_len);

         /**
          * Overloads the base class version to shut down the terminal transaction.
          */
         virtual void close_terminal(
            TerminalSinkBase *sink, int8 sink_token);

         /**
          * Overloads the base class version to determine the access level with the datalogger.
          */
         virtual bool start_get_access_level(SinkBase *sink);

         /**
          * @return Overloads the base class version to return a devconfig session.
          */
         virtual devconfig_session_handle make_devconfig_session(devconfig_library_handle library);

         /**
          * @return Overloads the base class version to return the source configured security code,
          */
         virtual uint2 get_devconfig_security_code(StrUni const &station_uri)
         { return security_code; }

         /**
          * @return Overloads the base class version to start the list files transaction and to
          * indicate that the operation is supported.
          */
         virtual bool start_list_files(
            SinkBase *sink, StrUni const &station_uri, int8 transaction, StrAsc const &filter = "");
         
         /**
          * Overloads the base class version to break down the specified URI into component symbols.
          */
         virtual void breakdown_uri(symbols_type &symbols, StrUni const &uri);

         /**
          * Overloads the base class version to return the root symbol for this source.
          */
         virtual symbol_handle get_source_symbol();

         /**
          * @return Overloaded to return the source type code.
          */
         virtual SymbolBase::symbol_type_code get_type()
         { return SymbolBase::type_bmp5_source; }

         /**
          * @return Returns the PakBus router
          */
         typedef Bmp5SourceHelpers::SourceRouter router_type;
         typedef Csi::PolySharedPtr<Csi::PakBus::Router, router_type> router_handle;
         router_handle get_router()
         { return router; }

         /**
          * @return Returns the effective PakBus address.
          */
         uint2 get_pakbus_address() const
         { return pakbus_address; }

         /**
          * Adds an operation to the set managed by this source.
          */
         typedef Bmp5SourceHelpers::SourceOperation operation_type;
         typedef Csi::SharedPtr<operation_type> operation_handle;
         void add_operation(operation_handle operation);
         
         /**
          * Removes the specified operation from this source.
          *
          * @param operation Specifies the operation to close.
          *
          * @param reboot_interval Optionally specifies an interval for which the datalogger will be
          * unresponsive. 
          */
         void remove_operation(operation_type *operation, uint4 reboot_interval = 0);

         /**
          * Sets the PakBus address of the first neighbour that was discovered.
          */
         void set_pakbus_address(uint2 value);

         /**
          * @return Returns the datalogger security code.
          */
         uint2 get_security_code() const
         { return security_code; }

         /**
          * Handles the timer event for reconnecting after a failure.
          */
         virtual void onOneShotFired(uint4 id);

         /**
          * Overloads the base class version to handle the poll schedile.
          */
         virtual void onScheduledEvent(uint4 id);

         /**
          * Handles the notification that the connection has failed.
          */
         void on_connect_failure(StrAsc const &reason);

         /**
          * Called when the connection to the datalogger has succeeded.
          */
         void on_connect_complete();

         /**
          * Called to set the timer to retry one or more failed requests.
          */
         void set_retry_requests();

      private:
         /**
          * Specifies the name of the serial port that this source will use.
          */
         StrAsc serial_port_name;

         /**
          * Specifies the baud rate that this source will use with the serial port.
          */
         uint4 baud_rate;

         /**
          * Set to true if a TCP link should be used.
          */
         bool use_tcp_port;

         /**
          * Specifies the TCP server address.
          */
         StrAsc tcp_server_address;

         /**
          * Specifies the TCP server port.
          */
         uint2 tcp_server_port;

         /**
          * Specifies the PakBus address for this source.
          */
         uint2 my_pakbus_address;

         /**
          * Specifies the PakBus address of the datalogger to which we will connect.  If thisa value
          * is invalid (0 or greater than or equal to 4095).
          */
         uint2 pakbus_address;

         /**
          * Specifies the PakBus PakBus address of any neighbour used to reach the datalogger.  If
          * this value is invalid, the logger will be assumed to be a neighbour.
          */
         uint2 neighbour_address;

         /**
          * Specifies the key used for PakBus encryption.
          */
         StrAsc pakbus_encryption_key;

         /**
          * Specifies the security code for the datalogger.
          */
         uint2 security_code;

         /**
          * Specifies the datalogger operating system version.
          */
         StrAsc os_version;

         /**
          * Specifies the datalogger operating system signature.
          */
         uint2 os_sig;

         /**
          * Specifies the datalogger serial number.
          */
         StrAsc serial_no;

         /**
          * Specifies the datalogger power up program.
          */
         StrAsc power_up_program;

         /**
          * Specifies the program compile state.
          */
         enum compile_state_type
         {
            compile_no_program = 0,
            compile_program_running = 1,
            compile_program_failed = 2,
            compile_program_paused = 3
         } compile_state;

         /**
          * Specifies the program name.
          */
         StrAsc program_name;

         /**
          * Specifies the program signature.
          */
         uint2 program_sig;

         /**
          * Specifies the time statmp when the program was compiled.
          */
         Csi::LgrDate compile_time;
         
         /**
          * Specifies the compile result.
          */
         StrAsc compile_result;

         /**
          * Specifies the logger model number.
          */
         StrAsc model_no;

         /**
          * Specifies the logger station name.
          */
         StrAsc station_name;
         
         /**
          * Specifies the PakBus router that will be used by this source.
          */
         router_handle router;

         /**
          * Reference to the serial port used for communication.
          */
         typedef Csi::PakBus::SerialPort serial_port_type;
         Csi::SharedPtr<serial_port_type> serial_port;

         /**
          * Reference to the TCP port used for communication.
          */
         typedef Csi::PakBus::TcpSerialPort tcp_port_type;
         Csi::SharedPtr<tcp_port_type> tcp_port;

         /**
          * Specifies the symbol that represents this source.
          */
         symbol_handle source_symbol;

         /**
          * Specifies the current state of being connected.
          */
         enum connect_state_type
         {
            connect_state_offline,
            connect_state_searching,
            connect_state_getting_table_defs,
            connect_state_connected
         } connect_state;

         /**
          * Specifies the collection of operations that are active for this source.
          */
         typedef std::deque<operation_handle> operations_type;
         operations_type operations;

         /**
          * Specifies the one shot timer used by this class as well as the PakBus router.
          */
         Csi::SharedPtr<OneShot> timer;

         /**
          * Specifies the timer used to retry a failed connection.
          */
         uint4 retry_id;

         /**
          * Specifies the timer used to retry failed data requests.
          */
         uint4 retry_requests_id;

         /**
          * Specifies the timer that is used to time the disabling of communications on the port.
          */
         uint4 disable_comms_id;

         /**
          * Set to true if logging is enabled.
          */
         bool log_enabled;

         /**
          * Specifies the path of the log file.
          */
         StrAsc log_path;

         /**
          * Specifies the name of the log file.
          */
         StrAsc log_file;

         /**
          * Specifies the maximum bale size for the log.
          */
         uint4 log_bale_size;

         /**
          * Specifies the maximum number of baled log files.
          */
         uint4 log_bale_count;

         /**
          * Set to true if time based baling should be used.
          */
         bool log_time_based_baling;

         /**
          * Specifies the time based baling interval.
          */
         int8 log_time_based_interval;

         /**
          * Specifies the logger used by this source
          */
         Csi::SharedPtr<Csi::LogByte> log;

         /**
          * Specifies the object used to drive the poll schedule.
          */
         Csi::SharedPtr<Scheduler> scheduler;

         /**
          * Specifies the poll schedule identifier.
          */
         uint4 poll_schedule_id;

         /**
          * Specifies the poll interval;
          */
         uint4 poll_interval;

         /**
          * Specifies the poll schedule base.
          */
         Csi::LgrDate poll_base;

         friend class Bmp5SourceHelpers::SourceRouter;
         friend class Bmp5SourceHelpers::OpGetTableDefs;
         friend class Bmp5SourceHelpers::OpDataRequest;
      };
   };
};


#endif

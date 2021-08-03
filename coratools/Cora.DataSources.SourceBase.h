/* Cora.DataSources.SourceBase.h

   Copyright (C) 2008, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 01 August 2008
   Last Change: Wednesday 13 May 2020
   Last Commit: $Date: 2020-05-13 13:50:49 -0600 (Wed, 13 May 2020) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Cora_DataSources_SourceBase_h
#define Cora_DataSources_SourceBase_h

#include "Cora.DataSources.Request.h"
#include "Cora.DataSources.SymbolBase.h"
#include "Csi.Xml.Element.h"
#include "Csi.DevConfig.SessionBase.h"
#include "Csi.DevConfig.LibraryManager.h"
#include "Csi.Events.h"
#include <stdexcept>


namespace Cora
{
   namespace DataSources
   {
      // @group: class forward declarations

      class Manager;
      class ValueSetter;

      // @endgroup:

      /**
       * Defines an application object that can participate in terminal services with a data
       * source.
       */
      class TerminalSinkBase: public Csi::InstanceValidator
      {
      public:
         /**
          * Defines a notification that will be called when the terminal has to be closed.
          *
          * @param sender Specifies the manager that owns the source.
          *
          * @param sink_token Specifies the sink token.
          *
          * @param failure Specifies a code that describes the reason for the failure.
          */
         enum terminal_failure_type
         {
            terminal_failure_unknown,
            terminal_failure_connection,
            terminal_failure_invalid_logon,
            terminal_failure_server_security_blocked,
            terminal_failure_logger_security_blocked,
            terminal_failure_invalid_device_name,
            terminal_failure_communication,
            terminal_failure_communication_disabled
         };
         virtual void on_terminal_failed(
            Manager *sender, int8 sink_token, terminal_failure_type failure) = 0;

         /**
          * Defines an event notification to handle received terminal data.
          *
          * @param sender Specifies the data source manager.
          *
          * @param sink_token Specifies the terminal session for which data has been received.
          *
          * @param content Specifies the content that has been received.
          */
         virtual void on_terminal_content(
            Manager *sender, int8 sink_token, StrBin const &content) = 0;
      };
      
      
      /**
       * Defines the base class for all data sources.
       */
      class SourceBase
      {
      public:
         /**
          * @param name_ Specifies the name for this data source. This value must be unique in the
          * scope of all data sources in the associated manager.
          */
         SourceBase(StrUni const &name_);

         /**
          * Destructor
          */
         virtual ~SourceBase()
         { }

         /**
          * Must be overloaded to establish a connection to the data source.
          */
         virtual void connect() = 0;

         /**
          * Must be overloaded to close any established connection to the data source.
          */
         virtual void disconnect() = 0;
         
         /**
          * Starts all pending requests for this data source.
          */
         virtual void start();

         /**
          * Stops all requests associated with this data source.
          */
         virtual void stop();

         /**
          * @return Must be overloaded to return true if this data source's connection is valid.
          */
         virtual bool is_connected() const = 0;
         
         /**
          * @return Returns true if this data source has been started.
          */
         virtual bool is_started() const
         { return was_started; }

         /**
          * @return Return the timestamp for the source
          */
         virtual Csi::LgrDate get_source_time()
         {
            return 0;
         }

         /**
          * Must be overloaded to write the properties for this data source to the specified XML
          * structure.
          *
          * @param prop_xml Specifies the XML structure to which properties will be written.
          */
         static StrUni const settings_name;
         virtual void get_properties(Csi::Xml::Element &prop_xml) = 0;

         /**
          * Must be overloaded to read data source properties from the specified XML structure.
          *
          * @param prop_xml Specifies the XML structure that contains properties for this source.
          */
         virtual void set_properties(Csi::Xml::Element &prop_xml) = 0;

         /**
          * @return Must be overloaded to return the symbol type code for this data source.
          */
         virtual SymbolBase::symbol_type_code get_type() = 0;

         /**
          * @param manager_ Specifies the data source manager that will own this source.
          */
         virtual void set_manager(Manager *manager_)
         { manager = manager_; }

         /**
          * @return Returns the data source manager that owns this source.
          */
         virtual Manager *get_manager() const
         { return manager; }

         /**
          * @return Returns the name of this data source.
          */
         virtual StrUni const &get_name() const
         { return name; }

         /**
          * Called by the manager when a request for this source is being added.  This method will
          * also be called when this source is "started".  In this latter case, the requests will be
          * added sequentially and the more_to_follow flag will be set.
          *
          * @param request Specifies the request to add for this source.
          *
          * @param more_to_follow Set to true if the application anticipates that more requests will
          * immediately follow for this source.
          */
         typedef Csi::SharedPtr<Request> request_handle;
         virtual void add_request(
            request_handle &request, bool more_to_follow = false) = 0;

         /**
          * Called in the context of start() and can be overloaded to "activate" any requests that
          * were added with the more_to_follow flag set.
          */
         virtual void activate_requests()
         { }
         
         /**
          * Removes the specified request from this data source.
          */
         virtual void remove_request(request_handle &request) = 0;

         /**
          * Removes all requests for this data source.
          */
         virtual void remove_all_requests() = 0;

         /**
          * @return Must be overloaded to return a handle to a symbol that can be used to browse the
          * symbols accessable for this source.
          */
         typedef Csi::SharedPtr<SymbolBase> symbol_handle;
         virtual symbol_handle get_source_symbol() = 0;

         /**
          * Can be overloaded to format the URI for the specified statistic given a URI to a
          * station.
          */
         virtual void get_statistic_uri(
            std::ostream &uri, StrUni const &station_uri, StrUni const &statistic_name)
         { }

         /**
          * Can be overloaded to return a URI to the station given a URI to a statistic.  This is
          * the inverse of get_statistic_uri().
          */
         virtual void get_statistic_station(
            std::ostream &uri, StrUni const &statistic_uri)
         { }

         /**
          * Can be overloaded to start a set value operation for the source.  Not all sources can
          * support set value (database, & file).
          *
          * @return Returns true if this data source supports the operation.
          *
          * @param uri Specifies the uri for the value to be changed.
          *
          * @param value Specifies the value that will be changed.
          */
         virtual bool start_set_value(
            SinkBase *sink,
            StrUni const &uri,
            ValueSetter const &value)
         { return false; }
         
         /**
          * Must be overloaded to implement the parsing code that breaks down a
          * URI into a collection of symbols. 
          */
         typedef std::pair<StrUni, SymbolBase::symbol_type_code> symbol_type;
         typedef std::list<symbol_type> symbols_type;
         virtual void breakdown_uri(symbols_type &symbols, StrUni const &uri) = 0;

         /**
          * Can be overloaded by a derived class to determine the range of time
          * stamps for the specified table.  When the operation is complete,
          * the results will be posted as a GetTableRangeCompleteEvent object
          * to the specified client object.  If this method is not overloaded,
          * the default version will post a completion event with a response
          * code indicating that the operation is not supported.
          */
         virtual void get_table_range(
            Csi::EventReceiver *client,
            StrUni const &uri);

         /**
          * Writes a log message for this data source.
          */
         virtual void log_event(StrAsc const &message);

         /**
          * Can be overloaded by a derived class to control whether scheduled
          * polling for new data from taking place.  Scheduled polling used in
          * some data sources (db, file, and http) to check for new data for
          * already started requests.  For some applications, like the web
          * server, this polling can be counter-productive since polling is
          * driven by web requests.
          */
         virtual void enable_poll_schedule(bool enabled = true)
         { }

         /**
          * @return Returns true if the specified URI identifies a value from classic datalogger
          * input locations.
          */
         virtual bool is_classic_inlocs(StrUni const &uri)
         { return false; }

         /**
          * Can be overloaded to start a send file operation
          *
          * @return Returns true of the data source identified on the uri parameter supports this
          * operation.
          *
          * @param sink Specifies the sink that will receive notification on completion.'*
          *
          * @param uri Specifies the URI that identifies the station to which the file will be
          * sent.
          *
          * @param dest_file_name Specifies the name and path of the file that will be stored on the
          * device.
          *
          * @param file_name Specifies the name of the file to send.
          */
         virtual bool start_send_file(
            SinkBase *sink,
            StrUni const &uri,
            StrUni const &dest_file_name,
            StrUni const &file_name)
         { return false; }

         /**
          * Can be overloaded to start an operation to get the newest file from the data source that
          * matches the specified pattern.
          *
          * @param sink Specifies the sink that will receive notification and fragments when the
          * operation is complete.
          *
          * @param uri Specifies the data source URI that will identify the station from which the
          * file will be received.
          *
          * @param pattern Specifies the wild card pattern used to identify the file(s).
          *
          * @return Returns true if this operation is supported.
          */
         virtual bool start_get_newest_file(
            SinkBase *sink,
            StrUni const &uri,
            StrUni const &pattern)
         { return false; }

         /**
          * Can be overloaded to start an operation to get the datalogger files through
          * this data source.
          *
          * @param sink Specifies the sink that will receive notification of completion.
          *
          * @param uri Specifies the data source URI that will identify the station for which the
          * clock will be checked.
          *
          * @param path is the path that will be looked at on the datasource (EX CPU/ etc).
          *
          * @return Returns true if this operation is supported by the source.
          */
         virtual bool start_list_files(
            SinkBase *sink,
            StrUni const &uri,
            StrUni const &path)
         { return false; }

         /**
          * Can be overloaded to start an operation to check or set the datalogger clock through
          * this data source.
          *
          * @param sink Specifies the sink that will receive notification of completion.
          *
          * @param uri Specifies the data source URI that will identify the station for which the
          * clock will be checked.
          *
          * @param should_set Set to true if the datalogger clock should be set.
          *
          * @param send_server_time Set to true if the datalogger clock should be set to the value
          * specified by server_time.
          *
          * @param server_time Specifies the timestamp for the datalogger that will be used if
          * should_set and send_server_time are both true.
          *
          * @return Returns true if this operation is supported by the source.
          */
         virtual bool start_clock_check(
            SinkBase *sink,
            StrUni const &uri,
            bool should_set = false,
            bool send_server_time = false,
            Csi::LgrDate const &server_time = 0)
         { return false; }

         /**
          * Can be overloaded to support the file control operation with the datalogger identified
          * by the provided uri.
          *
          * @param sink Specifies the object that will receive notification on completion.
          *
          * @param uri Specifies the data source URI that will identify the station on which to
          * operate.
          *
          * @param command Specifies the numeric operation code.
          *
          * @param p1 Specifies the first operation parameter.
          *
          * @param p2 Specifies the second operation parameter.
          *
          * @return Returns true if the file control request can be supported or false by default.
          */
         virtual bool start_file_control(
            SinkBase *sink,
            StrUni const &uri,
            uint4 command,
            StrAsc const &p1,
            StrAsc const &p2)
         { return false; }

         /**
          * Can be overloaded tp starts terminal services for the station specified by station_uri
          * for the specified terminal sink.
          *
          * @return Returns true if the service is supported by this source.
          *
          * @param sink Specifies the terminal sink that will receive notifications regarding
          * terminal events.
          *
          * @param station_uri Specifies the data source URI for the station.
          *
          * @param sink_token Specifies an application defined token that must be unique for all
          * tokens associated with the specified sink.
          */
         virtual bool start_terminal(
            TerminalSinkBase *sink, StrUni const &station_uri, int8 sink_token)
         { return false; }

         /**
          * Must be overloaded by data sources that support terminal services to transmit the
          * specified data to the datalogger.
          *
          * @param sink Specifies the sink for which the service was started.
          *
          * @param sink_token Specifies the application sink token for the service that was started.
          *
          * @param buff Specifies the start of the data buffer to transmit.
          *
          * @param buff_len Specifies the number of bytes in the buffer.
          */
         virtual void send_terminal(
            TerminalSinkBase *sink, int8 sink_token, void const *buff, size_t buff_len)
         { }

         /**
          * Must be overloaded to close the terminal service associated with the specified sink and
          * application token.
          *
          * @param sink Specifies the application sink.
          *
          * @param sink_token Specifies the unique application token for the service to be shut
          * down.
          */
         virtual void close_terminal(
            TerminalSinkBase *sink, int8 sink_token)
         { }

         /**
          * Can be overloaded to evaluate the access level for this source given its current
          * properties.
          *
          * @return Returns true if this operation is supported.
          *
          * @param sink Specifies the object that will receive notification of completion.
          */
         virtual bool start_get_access_level(SinkBase *sink)
         { return false; }

         /**
          * @return Can be overloaded to create a session object for the specified station uri.
          * The default implementation returns a null pointer which indicates that devconfig is not
          * supported for the specified uri or this source type.
          *
          * @param library Specifies the devconfig library manager that may need to be used by the
          * session.
          */
         typedef Csi::SharedPtr<Csi::DevConfig::SessionBase> devconfig_session_handle;
         typedef Csi::SharedPtr<Csi::DevConfig::LibraryManager> devconfig_library_handle;
         virtual devconfig_session_handle make_devconfig_session(devconfig_library_handle library)
         { return devconfig_session_handle(0); }

         /**
          * @return Can be overloaded to return the security code that should be used for the
          * specified station URI.
          *
          * @param station_uri Specifies the URI for the station.
          */
         virtual uint2 get_devconfig_security_code(StrUni const &station_uri)
         { return 0; }

         /**
          * Can be optionally overloaded to start an operation to get a list of files from a
          * datalogger that is identified through a provided data source uri.
          *
          * @return Returns true if the data source provides this service or false otherwise.
          *
          * @param sink Specifies the sink object that will receive the results.
          *
          * @param station_uri Specifies a data source uri that will identify the station for which
          * the files list will be obtained.
          *
          * @param transaction Specifies an application provided identifier for this operation.
          *
          * @param filter Optionally specifies a filter string that will be used to restrict the set
          * of files returned from the datalogger.
          */
         virtual bool start_list_files(
            SinkBase *sink, StrUni const &station_uri, int8 transaction, StrAsc const &filter = "")
         { return false; }
         
      protected:
         /**
          * Specifies the manager that owns this source.
          */
         Manager *manager;

         /**
          * Specifies the name of this source.
          */
         StrUni name;

         /**
          * Reflects whether the application wants to get data from this source.
          */
         bool was_started;
      };
   };
};


#endif

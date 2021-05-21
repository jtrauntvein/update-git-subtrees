/* Cora.DataSources.HttpSource.h

   Copyright (C) 2010, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 12 October 2010
   Last Change: Wednesday 17 January 2018
   Last Commit: $Date: 2018-01-17 12:37:30 -0600 (Wed, 17 Jan 2018) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Cora_DataSources_HttpSource_h
#define Cora_DataSources_HttpSource_h

#include "Cora.DataSources.SourceBase.h"
#include "Csi.HttpClient.h"
#include "Csi.HttpClient.WebSocket.h"
#include "Csi.PolySharedPtr.h"
#include "Scheduler.h"


namespace Cora
{
   namespace DataSources
   {
      // @group: class forward declarations

      namespace HttpSourceHelpers
      {
         class Cursor;
         class VariableSetter;
         class FileSender;
         class NewestFileGetter;
         class ClockChecker;
         class FileControl;
         class AccessChecker;
      };
      class HttpSource;
      
      // @endgroup


      /**
       * Represents a symbol obtained from an HTTP data source.
       */
      class HttpSymbol:
         public SymbolBase,
         public Csi::HttpClient::RequestClient
      {
      public:
         /**
          * @param source Specifies the source with which this symbol is associated.
          *
          * @param name Specifies the name of this symbol.
          *
          * @param parent Specifies the parent symbol for this symbol.
          *
          * @param symbol_type Specifies the code for the type for this symbol.
          */
         HttpSymbol(
            HttpSource *source,
            StrUni const &name,
            HttpSymbol *parent,
            symbol_type_code symbol_type);

         /**
          * Destructor
          */
         virtual ~HttpSymbol();

         /**
          * @return Overloads the return the symbol type code.
          */
         virtual symbol_type_code get_symbol_type() const
         { return symbol_type; }

         /**
          * @return Overloaded to return true if the source is connected.
          */
         virtual bool is_connected() const;

         /**
          * @return Overloaded to return true if this symbol is enabled for collection.
          */
         virtual bool is_enabled() const
         { return enabled; }

         /**
          * @return Overloaded to return true if this symbol is considered read-only.
          */
         virtual bool is_read_only() const
         { return read_only; }

         /**
          * @return Overloaded to return true if this symvol can be expanded.
          */
         virtual bool can_expand() const;

         /**
          * Starts the process of expanding this symbol.
          */
         virtual void start_expansion();

         /**
          * Starts the process of requesting new children for this symbol.
          */
         virtual void refresh();

         /**
          * Handles a failure from the HTTP client component.
          */
         typedef Csi::HttpClient::Request http_request_type;
         virtual bool on_failure(http_request_type *request);

         /**
          * Overloaded to handle a completion event from the HTTP client.
          */
         virtual bool on_response_complete(http_request_type *request);

         /**
          * Overloaded to format the URI for this symbol to the specified stream.
          *
          * @param out Specifies the destination stream.
          */
         virtual void format_uri(std::ostream &out) const;
         virtual void format_uri(std::wostream &out) const;

      private:
         /**
          * Specifies the symbol type code.
          */
         symbol_type_code const symbol_type;

         /**
          * Set to true if this symbol is read-only.
          */
         bool read_only;

         /**
          * Set to true if this symbol is enabled for collection.
          */
         bool enabled;

         /**
          * Specifies the request used to refresh or expand this symbol.
          */
         Csi::SharedPtr<http_request_type> http_request;

         /**
          * Specifies the shared HTTP connection.
          */
         typedef Csi::HttpClient::Connection connection_type;
         typedef Csi::SharedPtr<connection_type> connection_handle;
         connection_handle connection;
      };
      
      
      /**
       * Defines a data source that uses the datalogger web services to respond to application
       * requests.
       */
      class HttpSource:
         public SourceBase,
         public OneShotClient,
         public SchedulerClient,
         public Csi::HttpClient::RequestClient,
         public Csi::HttpClient::WebSocketClient
      {
      public:
         /**
          * @param source_name Specifies the name for this data source.
          */
         HttpSource(StrUni const &source_name);

         /**
          * Destructor
          */
         virtual ~HttpSource();

         /**
          * Overloaded to set the data source manager.
          */
         virtual void set_manager(Manager *manager);
         
         /**
          * Overloaded to connect the web server.
          */
         virtual void connect();

         /**
          * Overloaded to disconnect from the web server.
          */
         virtual void disconnect();

         /**
          * @return Overloaded to return true if this source has been connected.
          */
         virtual bool is_connected() const;

         /**
          * Overloaded to write the properties for this source to the specified XML element.
          *
          * @param prop_xml Specifies the XML element to which properties will be written.
          */
         virtual void get_properties(Csi::Xml::Element &prop_xml);

         /**
          * Overloaded to set the properties for this source from the specified XML element.
          *
          * @param prop_xml Specifies the XML element containing the properties for this source.
          */
         virtual void set_properties(Csi::Xml::Element &prop_xml);

         /**
          * Overloaded to start all pending requests for this source.
          */
         virtual void start();

         /**
          * Overloads the base class version to stop the poll schedule.
          */
         virtual void stop();
         
         /**
          * Overloaded to handle a new data request.
          */
         virtual void add_request(
            request_handle &request, bool more_to_follow = false);

         /**
          * Overloaded to remove a data request.
          */
         virtual void remove_request(request_handle &request);

         /**
          * Overloaded to remove all active data requests.
          */
         virtual void remove_all_requests();

         /**
          * Overloaded to start all pending requests.
          */
         virtual void activate_requests();

         /**
          * @return Overloaded to return the source symbol type code.
          */
         virtual SymbolBase::symbol_type_code get_type()
         { return SymbolBase::type_http_source; }

         /**
          * @return Overloaded to return the base sybol for this source.
          */
         virtual symbol_handle get_source_symbol();

         /**
          * Overloaded to break down the specified URI string into a collection of symbol names.
          */
         virtual void breakdown_uri(symbols_type &symbols, StrUni const &uri);

         /**
          * Overloaded to enable the poll schedule.
          */
         virtual void enable_poll_schedule(bool enabled)
         { poll_schedule_enabled = enabled; }

         // @group: property names

         static StrUni const server_address_name;
         static StrUni const server_port_name;
         static StrUni const user_name_name;
         static StrUni const password_name;
         static StrUni const poll_schedule_base_name;
         static StrUni const poll_schedule_interval_name;
         static StrUni const use_https_name;
         
         // @endgroup:

         // @group: class RequestClient derived definitions

         /**
          * Overloaded to handle a failure notification for an HTTP request.
          */
         typedef Csi::HttpClient::Request http_request;
         virtual bool on_failure(http_request *request);

         /**
          * Overloaded to handle a completion notification for an HTTP request.
          */
         virtual bool on_response_complete(http_request *request);
         
         // @endgroup

         // @group: Overloads the websocket client methods.
         
         /**
          * Overloads the connected handler for web sockets.
          */
         typedef Csi::HttpClient::WebSocket websock_type;
         virtual void on_connected(websock_type *sender);

         /**
          * Overloads the failure handler for the web socket.
          */
         virtual void on_failure(
            websock_type *sender,
            WebSocketClient::failure_type failure,
            int http_response);

         /**
          * Overloaded to handle a fragment for an incoming message.
          */
         virtual void on_message(
            websock_type *sender,
            void const *content,
            uint4 content_len,
            Csi::HttpClient::websock_op_code op_code,
            bool fin);
         
         // @endgroup:

         /**
          * Overloade to handle timed events.
          */
         virtual void onOneShotFired(uint4 id);

         /**
          * Overloade to handle the poll schedule event.
          */
         virtual void onScheduledEvent(uint4 id);

         /**
          * @return Returns the server address property.
          */
         StrAsc const &get_server_address() const
         { return server_address; }

         /**
          * @return Returns true if HTTPS should be used instead of HTTP.
          */
         bool get_use_https() const
         { return use_https; }
         
         /**
          * @return Returns the server TCP port.
          */
         uint2 get_server_port() const
         { return server_port; }

         /**
          * @return Returns the HTTP authorisation name.
          */
         StrAsc const &get_user_name() const
         { return user_name; }

         /**
          * @return Returns the HTTP authorisation password.
          */
         StrAsc const &get_password() const
         { return password; }

         /**
          * @return Returns the HTTP connection object used to service non-websocket requests.
          */
         typedef Csi::HttpClient::Connection http_connection_type;
         typedef Csi::SharedPtr<http_connection_type> http_connection_handle;
         http_connection_handle get_connection()
         { return connection; }

         /**
          * @return Returns the watcher (log) for the connection.
          */
         typedef Csi::HttpClient::ConnectionWatcher watcher_type;
         typedef Csi::SharedPtr<watcher_type> watcher_handle;
         watcher_handle get_watcher()
         { return watcher; }

         /**
          * Removes the specified cursor pointer.
          *
          * @param cursor Specifies the pointer to the cursor to remove.
          */
         typedef HttpSourceHelpers::Cursor cursor_type;
         void remove_cursor(cursor_type *cursor);

         /**
          * Starts a request to set a value for this source.
          */
         virtual bool start_set_value(
            SinkBase *sink, StrUni const &uri, ValueSetter const &value);

         /**
          * Cancels any pending attempt to set a value for this source.
          */
         typedef HttpSourceHelpers::VariableSetter value_setter_type;
         void end_set_value(value_setter_type *setter);

         /**
          * Overloads the base class version to start an HTTP put operation for the datalogger.
          */
         virtual bool start_send_file(
            SinkBase *sink,
            StrUni const &uri,
            StrUni const &dest_file_name,
            StrUni const &file_name);

         /**
          * Cancels any pending attempt to send a file.
          */
         typedef HttpSourceHelpers::FileSender file_sender_type;
         void end_send_file(file_sender_type *sender);

         /**
          * Overloads the base class version to start an HTTP request to get the file from the
          * datalogger.
          */
         virtual bool start_get_newest_file(
            SinkBase *sink,
            StrUni const &uri,
            StrUni const &pattern);

         /**
          * Canceks any pending attempt to get a newest file.
          */
         typedef HttpSourceHelpers::NewestFileGetter newest_file_getter_type;
         void end_get_newest_file(newest_file_getter_type *getter);

         /**
          * Overloads the base class version to start an HTTP request to get or set the datalogger
          * clock.
          */
         virtual bool start_clock_check(
            SinkBase *sink,
            StrUni const &uri,
            bool should_set,
            bool send_server_time,
            Csi::LgrDate const &server_time);

         /**
          * Cancels any pending attempt to check the logger clock.
          */
         typedef HttpSourceHelpers::ClockChecker clock_checker_type;
         void end_clock_checker(clock_checker_type *checker);

         /**
          * Overloads the base class version to perform a file control operation over HTTP.
          */
         virtual bool start_file_control(
            SinkBase *sink,
            StrUni const &uri,
            uint4 command,
            StrAsc const &p1,
            StrAsc const &p2);

         /**
          * Releases the resources for the specified file controller.
          *
          * @param control Specifies the control operation.
          *
          * @param hold_off Specifies the hold-off time in seconds that was sent by the datalogger.
          */
         typedef HttpSourceHelpers::FileControl file_control_type;
         void end_file_control(file_control_type *control, uint4 hold_off = 0);
         
         /**
          * Allocates a new transaction number for the specified cursor and transmits the request as
          * a web socket message.
          *
          * @param cursor Specifies the cursor sending this message.
          *
          * @param request Specifies the JSON object that will be sent as a data request.  The data
          * source will set the transaction property of the object before sending it to the server.
          */
         int4 start_websock_request(cursor_type *cursor, Csi::Json::ObjectHandle &request);

         /**
          * @return Returns true if web sockets should be used.
          */
         bool get_use_websocket() const
         { return use_websocket; }

         /**
          * Overloads the base class method to implement support for terminal connections over
          * websocket.
          */
         virtual bool start_terminal(
            TerminalSinkBase *sink, StrUni const &station_uri, int8 sink_token);

         /**
          * Overloads the base class method to send the specified data using web sockets.
          */
         virtual void send_terminal(
            TerminalSinkBase *sink, int8 sink_token, void const *buff, size_t buff_len);

         /**
          * Overloads the base class method to cancel the terminal service.
          */
         virtual void close_terminal(
            TerminalSinkBase *sink, int8 sink_token);

         /**
          * Overloads the base class method to attempt to evaluate the datalogger access level.
          */
         virtual bool start_get_access_level(SinkBase *sink);

         /**
          * Closes the operation for checking access level.
          */
         typedef HttpSourceHelpers::AccessChecker access_checker_type;
         void end_get_access_level(access_checker_type *checker);

      private:
         /**
          * Handles the websocket message that reports that a request has been started.
          */
         void on_request_started(Csi::Json::Object &message);

         /**
          * Handles the web socket message that reports that records have been received for a
          * request.
          */
         void on_request_records(Csi::Json::Object &message);

         /**
          * Handles the web socket message that reports that a request has failed.
          */
         void on_request_failed(Csi::Json::Object &message);

         /**
          * Handles the terminal data message.
          */
         void on_terminal_data(Csi::Json::Object &message);
         
      private:
         /**
          * Specifies the IP or DNS  address for the web server.
          */
         StrAsc server_address;

         /**
          * Specifies the TCP port for the web server.
          */
         uint2 server_port;

         /**
          * Set to true to indicate that HTTPS should be used.
          */
         bool use_https;

         /**
          * Specifies the HTTP authorisation user name.
          */
         StrAsc user_name;

         /**
          * Specifis the HTTP authoration password.
          */
         StrAsc password;

         /**
          * Specifies the base time for the poll schedule.
          */
         Csi::LgrDate poll_schedule_base;

         /**
          * Specifies the interval for the poll schedule.
          */
         uint4 poll_schedule_interval;

         /**
          * Set to true if this source has been connected.
          */
         bool was_connected;

         /**
          * Set to true if the connection is active.
          */
         bool connect_active;

         /**
          * Specifies the connection used for non-websocket requests.
          */
         Csi::SharedPtr<http_connection_type> connection;
         
         /**
          * Specifies the request used for making the initial connection.
          */
         Csi::SharedPtr<http_request> connect_request;

         /**
          * Specifies the identifier for the timer used to retry connections.
          */
         uint4 retry_id;

         /**
          * Specifies the identifier for the timer used to hold off connection attempts following a
          * file control.
          */
         uint4 hold_off_id;

         /**
          * Specifies the timer object.
          */
         Csi::SharedPtr<OneShot> timer;

         /**
          * Specifies the cursor poll scheduler.
          */
         Csi::SharedPtr<Scheduler> scheduler;

         /**
          * Identifies the poll schedule.
          */
         uint4 poll_schedule_id;

         /**
          * Set to true if the poll schedule should be active.
          */
         bool poll_schedule_enabled;

         /**
          * Specifies the collection of cursors managed by this source.
          */
         typedef Csi::SharedPtr<cursor_type> cursor_handle;
         typedef std::list<cursor_handle> cursors_type;
         cursors_type cursors;

         /**
          * Specifies the base symbol for this source.
          */
         Csi::PolySharedPtr<SymbolBase, HttpSymbol> symbol;

         /**
          * Specifies the collection of pending value setters.
          */
         typedef Csi::SharedPtr<value_setter_type> value_setter_handle;
         typedef std::list<value_setter_handle> value_setters_type;
         value_setters_type value_setters;

         /**
          * Specifies the collection of pending file senders.
          */
         typedef Csi::SharedPtr<file_sender_type> file_sender_handle;
         typedef std::deque<file_sender_handle> file_senders_type;
         file_senders_type file_senders;

         /**
          * Specifies the collection of pending newest file getters
          */
         typedef Csi::SharedPtr<newest_file_getter_type> newest_file_getter_handle;
         typedef std::deque<newest_file_getter_handle> newest_file_getters_type;
         newest_file_getters_type newest_file_getters;

         /**
          * Specifies the list of pending clock checkers.
          */
         typedef Csi::SharedPtr<clock_checker_type> clock_checker_handle;
         typedef std::deque<clock_checker_handle> clock_checkers_type;
         clock_checkers_type clock_checkers;

         /**
          * Specifies the list of pending file control operations.
          */
         typedef Csi::SharedPtr<file_control_type> file_control_handle;
         typedef std::deque<file_control_handle> file_controls_type;
         file_controls_type file_controls;

         /**
          * Specifies the collection of objects checking ofr access level.
          */
         typedef Csi::SharedPtr<access_checker_type> access_checker_handle;
         typedef std::deque<access_checker_handle> access_checkers_type;
         access_checkers_type access_checkers;

         /**
          * Specifies the websocket connection used for this source.
          */
         typedef Csi::SharedPtr<websock_type> websock_handle;
         websock_handle websocket;

         /**
          * Set to true if web sockets should be used.  This value will be set to false if an
          * unsupported failure is received.
          */
         bool use_websocket;

         /**
          * Used to buffer the contents of websocket messages.
          */
         StrBin websock_buffer;

         /**
          * Specifies the last websocket request transaction number used.
          */
         int4 last_websock_tran;

         /**
          * Specifies the collection of cursors that have active websocket requests.  This container
          * is keyed by the request transaction numbers.
          */
         typedef std::map<int4, cursor_handle> websock_requests_type;
         websock_requests_type websock_requests;

         /**
          * Specifies the stream used to format websocket messages sent to the server.
          */
         Csi::OStrAscStream request_stream;

         /**
          * Specifies the current terminal sink and token.
          */
         TerminalSinkBase *terminal_sink;
         int8 terminal_sink_token;

         /**
          * Specifies the watcher used to log the HTTP connections(s) used by this class.
          */
         watcher_handle watcher;

         /**
          * Specifies the properties for controlling logging with this source.
          */
         bool log_enabled;
         StrAsc log_path;
         StrAsc log_file;
         int8 log_interval;
      };
   };
};


#endif

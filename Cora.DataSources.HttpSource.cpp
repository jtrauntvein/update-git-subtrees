/* Cora.DataSources.HttpSource.cpp

   Copyright (C) 2010, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 12 October 2010
   Last Change: Friday 14 February 2020
   Last Commit: $Date: 2020-02-14 09:53:04 -0600 (Fri, 14 Feb 2020) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.DataSources.HttpSource.h"
#include "Cora.DataSources.TableFieldUri.h"
#include "Cora.DataSources.Manager.h"
#include "Cora.DataSources.SymbolBrowser.h"
#include "Cora.Broker.ValueName.h"
#include "Csi.ByteQueueStream.h"
#include "Csi.Json.h"
#include "Csi.ByteOrder.h"
#include "Csi.Utils.h"
#include "Csi.Base64.h"
#include <set>
#include <iterator>


namespace Cora
{
   namespace DataSources
   {
      namespace
      {
         StrUni const log_name(L"log");
         StrUni const log_path_name(L"path");
         StrUni const log_file_name(L"file");
         StrUni const log_interval_name(L"interval");
      };

      
      namespace HttpSourceHelpers
      {
         /**
          * Represents a connection to the HTTP server for a collection of similar requests.
          */
         class Cursor: public Csi::HttpClient::RequestClient
         {
         public:
            /**
             * @param source_ Specifies the source that will own this cursor.
             *
             * @param request Specifies the first request for this cursor.
             */
            typedef HttpSource::request_handle request_handle;
            Cursor(HttpSource *source_, request_handle &request);

            /**
             * Destructor.
             */
            virtual ~Cursor();

            /**
             * Adds the specified request to this cursor.
             *
             * @param request Specifies the request to add.
             *
             * @return Returns true if the request is compatible with other requests or false if
             * not.
             */
            bool add_request(request_handle &request);

            /**
             * Removes the specified request from the cursor.
             *
             * @param request Specifies the request to add.
             *
             * @return Returns true if the specified request was managed by this cursor.
             */
            bool remove_request(request_handle &request);

            /**
             * @return Returns true if this cursor is not servicing any requests.
             */
            bool has_no_requests();

            /**
             * Starts the process of polling data for the requests associated with this cursor.
             */
            void poll();
            
            /**
             * Overloaded to handle a failure report from the HttpClient component.
             */
            typedef Csi::HttpClient::Request http_request_type;
            virtual bool on_failure(http_request_type *request);

            /**
             * Overloaded to handle a completion event from the HttpClient component.
             */
            virtual bool on_response_complete(http_request_type *request);

            /**
             * @return Returns true if this cursor has been polled before.
             */
            bool get_previously_polled() const
            { return previously_polled; }

            /**
             * Reports a failure for the request associated with this cursor.
             */
            void on_failure(SinkBase::sink_failure_type failure);

            /**
             * @return Returns the websocket transaction number for this cursor.
             */
            int4 get_websock_tran() const
            { return websock_tran; }

            /**
             * Called to handle the receipt of the request started notification.
             */
            void on_request_started(Csi::Json::Object &message);

            /**
             * Called to handle the request records notification.
             */
            void on_request_records(Csi::Json::Object &message);

            /**
             * Called to handle the request failed notification.
             */
            void on_request_failed(Csi::Json::Object &message);
            
         private:
            /**
             * Specifies the source that manages this cursor.
             */
            HttpSource *source;

            /**
             * Specifies the list of requests serviced by this cursor.
             */
            typedef std::list<request_handle> requests_type;
            requests_type requests;

            /**
             * Set to true if this cursor has been polled before.
             */
            bool previously_polled;
            
            /**
             * Specifies the table definition signature.
             */
            int4 table_def_sig;

            /**
             * A description for the records reported by this cursor.
             */
            typedef Broker::RecordDesc record_desc_type;
            Csi::SharedPtr<record_desc_type> record_desc;

            /**
             * Specifies a collection of records that were used before and can be used to report new
             * data to the request clients.
             */
            typedef Csi::SharedPtr<Broker::Record> record_handle;
            typedef std::deque<record_handle> cache_type;
            cache_type cache;

            /**
             * Specifies the last record number received.
             */
            uint4 last_record_no;

            /**
             * Specifies the time stamp for the last record received.
             */
            Csi::LgrDate last_time;

            /**
             * Specifies the HTTP request that is currently being used for polling.
             */
            Csi::SharedPtr<http_request_type> http_request;

            /**
             * Specifies the websocket transaction number.
             */
            int4 websock_tran;
         };


         /**
          * Represents an object that maintains the state for setting a variable for an HTTP data
          * source.
          */
         class VariableSetter: public Csi::HttpClient::RequestClient
         {
         public:
            /**
             * @param sink_ Specifies the data sink that made this request.
             *
             * @param uri_ Specifies the data source URI for the variable to be set.
             *
             * @param value Specifies the new value for the variable.
             *
             * @parma source_ Specifies the data source.
             */
            VariableSetter(
               SinkBase *sink_,
               StrAsc const &uri_,
               ValueSetter const &value,
               HttpSource *source_);

            /**
             * Destructor
             */
            virtual ~VariableSetter();

            /**
             * Overloaded to handle a failure from the HttpClient component.
             */
            typedef Csi::HttpClient::Request request_type;
            virtual bool on_failure(request_type *request);

            /**
             * Overloaded to handle a completion event from the HttpClient component.
             */
            virtual bool on_response_complete(request_type *request);

         private:
            /**
             * Specifies the source that owns this setter.
             */
            HttpSource *source;

            /**
             * Specifies the data source sink that will receive a completion notification.
             */
            SinkBase *sink;

            /**
             * Specifies the uri for the data source variable.
             */
            StrAsc uri;

            /**
             * Specifies the connection used to send the HTTP request.
             */
            typedef HttpSource::http_connection_handle http_connection_handle;
            http_connection_handle http_connection;

            /**
             * Specifies the HTTP request.
             */
            Csi::SharedPtr<request_type> request;
         };


         VariableSetter::VariableSetter(
            SinkBase *sink_,
            StrAsc const &uri_,
            ValueSetter const &value,
            HttpSource *source_):
            sink(sink_),
            uri(uri_),
            source(source_),
            http_connection(source_->get_connection())
         {
            // make sure that the connection is allocated
            if(http_connection == 0)
            {
               http_connection.bind(new Csi::HttpClient::Connection);
               http_connection->set_watcher(source->get_watcher());
            }

            // we now need to form the request
            Csi::Uri web_uri;
            Csi::OStrAscStream temp;

            if(uri.first() == '\"')
               uri.cut(0, 1);
            if(uri.last() == '\"')
               uri.cut(uri.length() - 1);
            temp.imbue(std::locale::classic());
            web_uri.set_server_address(source->get_server_address());
            web_uri.set_server_port(source->get_server_port());
            if(source->get_use_https())
               web_uri.set_protocol("https");
            web_uri.insert("command", "SetValueEx");
            web_uri.insert("uri", uri);
            web_uri.insert("format", "json");
            switch(value.value_type)
            {
            case ValueSetter::value_type_bool:
               temp << (value.value_variant.v_bool ? "-1" : "0");
               break;
               
            case ValueSetter::value_type_float:
               csiFloatToStream(temp, value.value_variant.v_float);
               break;
               
            case ValueSetter::value_type_uint4:
               temp << value.value_variant.v_uint4;
               break;
               
            case ValueSetter::value_type_int4:
               temp << value.value_variant.v_int4;
               break;
               
            case ValueSetter::value_type_uint2:
               temp << value.value_variant.v_uint2;
               break;
               
            case ValueSetter::value_type_int2:
               temp << value.value_variant.v_int2;
               break;
               
            case ValueSetter::value_type_uint1:
               temp << static_cast<uint2>(value.value_variant.v_uint1);
               break;
               
            case ValueSetter::value_type_int1:
               temp << static_cast<int2>(value.value_variant.v_int1);
               break;
               
            case ValueSetter::value_type_string:
            default:
               temp << value.value_string;
               break;
            }
            web_uri.insert("value", temp.str());
            request.bind(new request_type(this, web_uri));
            request->set_authentication(source->get_user_name(), source->get_password());
            http_connection->add_request(request);
         } // constructor


         VariableSetter::~VariableSetter()
         {
            request.clear();
            http_connection.clear();
         } // destructor


         bool VariableSetter::on_failure(request_type *request)
         {
            SinkBase::set_outcome_type outcome(SinkBase::set_outcome_connection_failed);
            if(request->get_response_code() == 401)
               outcome = SinkBase::set_outcome_invalid_logon;
            if(SinkBase::is_valid_instance(sink))
               sink->on_set_complete(source->get_manager(), uri, outcome);
            source->end_set_value(this);
            return true;
         } // on_failure


         bool VariableSetter::on_response_complete(request_type *request)
         {
            using namespace Csi::Json;
            SinkBase::set_outcome_type rtn(SinkBase::set_outcome_unknown);
            try
            {
               // parse the server response
               Csi::IByteQueueStream input(&request->get_receive_buff());
               Object response;
               NumberHandle outcome;
               
               response.parse(input);
               outcome = response["outcome"];
               if(outcome->get_value_uint4() < 14)
                  rtn = static_cast<SinkBase::set_outcome_type>(outcome->get_value_uint4()); 
            }
            catch(std::exception &)
            { rtn = SinkBase::set_outcome_unknown; }
            if(SinkBase::is_valid_instance(sink))
               sink->on_set_complete(source->get_manager(), uri, rtn);
            source->end_set_value(this);
            return true;
         } // on_response_complete


         /**
          * Defines an object that maintains the state for a sending a file to an HTTP source.
          */
         class FileSender: public Csi::HttpClient::RequestClient
         {
         public:
            /**
             * @param sink_ Specifies the sink making this request.
             *
             * @param uri_ Specifies the uri for the datalogger.
             *
             * @param dest_file_name_ Specifies the destination file name.
             *
             * @param file_name_ Specifies the name of the file to be sent.
             *
             * @param source_ Specifies the source.
             */
            FileSender(
               SinkBase *sink_,
               StrUni const &uri_,
               StrUni const &dest_file_name_,
               StrUni const &file_name_,
               HttpSource *source_);

            /**
             * Destructor
             */
            virtual ~FileSender();

            /**
             * Overloads the failure handler.
             */
            typedef Csi::HttpClient::Request request_type;
            virtual bool on_failure(request_type *request);

            /**
             * Overloads the completion event.
             */
            virtual bool on_response_complete(request_type *request);

         private:
            /**
             * Specifies the sink.
             */
            SinkBase *sink;

            /**
             * Specifies the source.
             */
            HttpSource *source;

            /**
             * Specifies the data source uri.
             */
            StrUni const uri;

            /**
             * Specifies the destination file name.
             */
            StrUni const &dest_file_name;

            /**
             * Specifies the connection used to send the HTTP request.
             */
            typedef HttpSource::http_connection_handle http_connection_handle;
            http_connection_handle http_connection;

            /**
             * Specifies the HTTP request.
             */
            Csi::SharedPtr<request_type> request;
         };


         FileSender::FileSender(
            SinkBase *sink_,
            StrUni const &uri_,
            StrUni const &dest_file_name_,
            StrUni const &file_name,
            HttpSource *source_):
            sink(sink_),
            uri(uri_),
            dest_file_name(dest_file_name_),
            source(source_),
            http_connection(source_->get_connection())
         {
            // make sure that the connection is allocated.
            if(http_connection == 0)
            {
               http_connection.bind(new Csi::HttpClient::Connection);
               http_connection->set_watcher(source->get_watcher());
            }

            // we need to format the destination path
            StrAsc path(dest_file_name.to_utf8());
            if(path.first() == '\"')
               path.cut(0, 1);
            if(path.last() == '\"')
               path.cut(path.length() - 1);
            path.replace(":", "/");
            
            // we need to form the request
            Csi::Uri web_uri;
            web_uri.set_server_address(source->get_server_address());
            web_uri.set_server_port(source->get_server_port());
            if(source->get_use_https())
               web_uri.set_protocol("https");
            web_uri.set_path(path);
            request.bind(new request_type(this, web_uri, request_type::method_put, false));
            request->set_authentication(source->get_user_name(), source->get_password());
            
            // we need to open the file and add it to the request.
            FILE *input = Csi::open_file(file_name, "rb");
            byte buff[1024];
            size_t bytes_read;
            if(input)
            {
               while((bytes_read = fread(buff, 1, sizeof(buff), input)) > 0)
                  request->add_bytes(buff, bytes_read, bytes_read < sizeof(buff));
               fclose(input);
            }
            http_connection->add_request(request);
         } // constructor


         FileSender::~FileSender()
         {
            request.clear();
            http_connection.clear();
         } // destructor


         bool FileSender::on_failure(request_type *request)
         {
            SinkBase::send_file_outcome_type outcome(SinkBase::send_file_connection_failed);
            if(request->get_response_code() == 401)
               outcome = SinkBase::send_file_invalid_logon;
            if(SinkBase::is_valid_instance(sink))
               sink->on_send_file_complete(source->get_manager(), uri, dest_file_name, outcome);
            source->end_send_file(this);
            return true;
         } // on_failure


         bool FileSender::on_response_complete(request_type *request)
         {
            // we'll look at the response to see if its format is JSON.  if it is, we will try to
            // extract the outcome code from the response
            SinkBase::send_file_outcome_type outcome(SinkBase::send_file_success);
            if(request->get_content_type() == "application/json")
            {
               try
               {
                  Csi::Json::Object response;
                  Csi::IByteQueueStream response_str(&request->get_receive_buff());
                  int4 response_outcome;
                  response.parse(response_str);
                  response_outcome = static_cast<int4>(response.get_property_number("outcome"));
                  if(response_outcome <= SinkBase::send_file_unknown || response_outcome >= SinkBase::send_file_invalid_uri)
                     outcome = SinkBase::send_file_unknown;
                  else
                     outcome = static_cast<SinkBase::send_file_outcome_type>(response_outcome);
               }
               catch(std::exception &)
               { }
            }
            
            // we can now send the response to the sink
            if(SinkBase::is_valid_instance(sink))
               sink->on_send_file_complete(source->get_manager(), uri, dest_file_name, outcome);
            source->end_send_file(this);
            return true;
         } // on_response_complete


         class NewestFileGetter: public Csi::HttpClient::RequestClient
         {
         private:
            /**
             * Specifies the source.
             */
            HttpSource *source;

            /**
             * Specifies the sink.
             */
            SinkBase *sink;

            /**
             * Specifies the data source uri.
             */
            StrUni source_uri;

            /**
             * Specifies the pattern.
             */
            StrUni pattern;

            StrUni file_name;

            /**
             * Reference to the HTTP connection.
             */
            HttpSource::http_connection_handle http_connection;

            /**
             * Specifies the HTTP request.
             */
            typedef Csi::HttpClient::Request request_type;
            Csi::SharedPtr<request_type> request;
            
         public:
            /**
             * Constructor
             */
            NewestFileGetter(
               HttpSource *source_,
               SinkBase *sink_,
               StrUni const &source_uri_,
               StrUni const &pattern_);
            
            /**
             * Destructor
             */
            virtual ~NewestFileGetter();

            /**
             * Overloads the on failure notification.
             */
            virtual bool on_failure(request_type *sender);

            /**
             * Overloads the notification of response data.
             */
            virtual void on_response_data(request_type *sender);

            /**
             * Overloads the notification of the response completion.
             */
            virtual bool on_response_complete(request_type *sender);

         private:
            /**
             * Extracts the file name from the specified disposition string.
             */
            void extract_file_name(StrUni const &disposition);
         };


         NewestFileGetter::NewestFileGetter(
            HttpSource *source_,
            SinkBase *sink_,
            StrUni const &source_uri_,
            StrUni const &pattern_):
            source(source_),
            sink(sink_),
            source_uri(source_uri_),
            pattern(pattern_)
         {
            // make sure that the connection is allocated.
            if(http_connection == 0)
            {
               http_connection.bind(new Csi::HttpClient::Connection);
               http_connection->set_watcher(source->get_watcher());
            }

            // we need to form the request
            Csi::Uri web_uri;
            web_uri.set_server_address(source->get_server_address());
            web_uri.set_server_port(source->get_server_port());
            if(source->get_use_https())
               web_uri.set_protocol("https");
            if(pattern.find(L"*") < pattern.length() || pattern.find(L"?") < pattern.length())
            {
               web_uri.insert("command", "NewestFile");
               web_uri.insert("expr", pattern.to_utf8());
            }
            else
            {
               StrAsc path(pattern.to_utf8());
               path.replace(":", "/");
               web_uri.set_path(path);
            }
            request.bind(new request_type(this, web_uri));
            request->set_authentication(source->get_user_name(), source->get_password());
            http_connection->add_request(request);
         } // constructor


         NewestFileGetter::~NewestFileGetter()
         {
            http_connection.clear();
            request.clear();
         } // destructor


         bool NewestFileGetter::on_failure(request_type *sender)
         {
            SinkBase::get_newest_file_status_type status(SinkBase::get_newest_status_connection_failed);
            if(sender->get_response_code() == 401)
               status = SinkBase::get_newest_status_logger_permission_denied;
            else if(sender->get_response_code() == 404)
               status = SinkBase::get_newest_status_no_file;
            if(SinkBase::is_valid_instance(sink))
               sink->on_get_newest_file_status(source->get_manager(), status, source_uri, pattern);
            source->end_get_newest_file(this);
            return false;
         } // on_failure


         void NewestFileGetter::on_response_data(request_type *sender)
         {
            if(SinkBase::is_valid_instance(sink))
            {
               // we need to check for the returned file name
               if(file_name.length() == 0)
                  extract_file_name(sender->get_content_disposition());

               // we'll use this opportunity to send all data received to the sink
               byte buff[1024];
               Csi::ByteQueue &queue(sender->get_receive_buff());
               while(sender->get_response_code() == 200 &&
                     queue.size() > 0 &&
                     SinkBase::is_valid_instance(sink))
               {
                  uint4 bytes_read(queue.pop(buff, sizeof(buff)));
                  sink->on_get_newest_file_status(
                     source->get_manager(),
                     SinkBase::get_newest_status_in_progress,
                     source_uri,
                     pattern,
                     file_name,
                     buff,
                     bytes_read);
               }
            }
         } // on_response_data


         bool NewestFileGetter::on_response_complete(request_type *sender)
         {
            if(SinkBase::is_valid_instance(sink))
            {
               SinkBase::get_newest_file_status_type status;
               switch(sender->get_response_code())
               {
               case 200:
                  if(file_name.length() == 0)
                     extract_file_name(sender->get_content_disposition());
                  status = SinkBase::get_newest_status_complete;
                  break;

               case 401:
               case 403:
                  status = SinkBase::get_newest_status_logger_permission_denied;
                  break;

               case 404:
                  status = SinkBase::get_newest_status_no_file;
                  break;

               default:
                  status = SinkBase::get_newest_status_connection_failed;
                  break;
               }
               sink->on_get_newest_file_status(
                  source->get_manager(),
                  status,
                  source_uri,
                  pattern,
                  file_name);
            }
            source->end_get_newest_file(this);
            return false;
         } // on_response_complete


         void NewestFileGetter::extract_file_name(StrUni const &disposition)
         {
            StrUni filename_header(L"filename=");
            size_t filename_pos(disposition.find(filename_header.c_str()) + filename_header.length());
            if(filename_pos < disposition.length())
            {
               size_t end_pos(disposition.find(L";", filename_pos));
               if(end_pos >= disposition.length())
                  end_pos = disposition.find(L"\r");
               disposition.sub(file_name, filename_pos, end_pos - filename_pos);
            }
         } // extract_file_name

         
         class ClockChecker: public Csi::HttpClient::RequestClient
         {
         private:
            /**
             * Specifies the source.
             */
            HttpSource *source;

            /**
             * Specifies the sink.
             */
            SinkBase *sink;

            /**
             * Specifies the source uri.
             */
            StrUni const source_uri;

            /**
             * Set to true if the clock is being set.
             */
            bool should_set;

            /**
             * Specifies the HTTP connection.
             */
            HttpSource::http_connection_handle http_connection;

            /**
             * Specifies the HTTP request.
             */
            typedef Csi::HttpClient::Request request_type;
            Csi::SharedPtr<request_type> request;

         public:
            /**
             * Constructor
             */
            ClockChecker(
               HttpSource *source_,
               SinkBase *sink_,
               StrUni const &source_uri_,
               bool should_set_,
               bool send_server_time,
               Csi::LgrDate const &server_time);

            /**
             * Destructor
             */
            virtual ~ClockChecker();

            /**
             * Overloads the failure notification.
             */
            virtual bool on_failure(request_type *sender);

            /**
             * Overloads the completion notification.
             */
            virtual bool on_response_complete(request_type *sender);
         };


         ClockChecker::ClockChecker(
            HttpSource *source_,
            SinkBase *sink_,
            StrUni const &source_uri_,
            bool should_set_,
            bool send_server_time,
            Csi::LgrDate const &server_time):
            source(source_),
            sink(sink_),
            source_uri(source_uri_),
            should_set(should_set_)
         {
            // we need to form the web request
            Csi::Uri web_uri;
            web_uri.set_server_address(source->get_server_address());
            web_uri.set_server_port(source->get_server_port());
            if(source->get_use_https())
               web_uri.set_protocol("https");
            if(should_set)
            {
               Csi::OStrAscStream temp;
               web_uri.insert("command", "ClockSet");
               if(send_server_time)
                  server_time.format(temp, "%Y-%m-%dT%H:%M:%S%x");
               else
                  Csi::LgrDate::system().format(temp, "%Y-%m-%dT%H:%M:%S%x");
               web_uri.insert("time", temp.str());
            }
            else
               web_uri.insert("command", "ClockCheck");
            web_uri.insert("format", "json");
            
            // we can now send the request
            http_connection.bind(new Csi::HttpClient::Connection);
            http_connection->set_watcher(source->get_watcher());
            request.bind(new request_type(this, web_uri));
            request->set_authentication(source->get_user_name(), source->get_password());
            http_connection->add_request(request);
         } // constructor


         ClockChecker::~ClockChecker()
         {
            request.clear();
            http_connection.clear();
         } // destructor


         bool ClockChecker::on_failure(request_type *sender)
         {
            SinkBase::clock_outcome_type outcome(SinkBase::clock_failure_connection);
            if(sender->get_response_code() == 401)
               outcome = SinkBase::clock_failure_logger_permission;
            if(SinkBase::is_valid_instance(sink))
               sink->on_clock_complete(source->get_manager(), source_uri, outcome);
            source->end_clock_checker(this);
            return false;
         } // on_failure


         bool ClockChecker::on_response_complete(request_type *sender)
         {
            // we need to try the parse the response
            SinkBase::clock_outcome_type outcome(SinkBase::clock_failure_unknown);
            Csi::LgrDate logger_time;

            try
            {
               Csi::Json::Object response;
               Csi::IByteQueueStream response_str(&request->get_receive_buff());
               int4 response_outcome;
               response.parse(response_str);
               response_outcome = static_cast<int4>(response.get_property_number("outcome"));
               if(!should_set)
               {
                  switch(response_outcome)
                  {
                  case 1:
                     outcome = SinkBase::clock_success_checked;
                     logger_time = response.get_property_date("time");
                     break;
                     
                  case 2:
                     outcome = SinkBase::clock_success_set;
                     logger_time = response.get_property_date("time");
                     break;
                     
                  case 3:
                     outcome = SinkBase::clock_failure_connection;
                     break;
                     
                  case 4:
                     outcome = SinkBase::clock_failure_invalid_logon;
                     break;
                     
                  case 5:
                     outcome = SinkBase::clock_failure_server_permission;
                     break;
                     
                  case 6:
                     outcome = SinkBase::clock_failure_communication;
                     break;
                     
                  case 7:
                     outcome = SinkBase::clock_failure_communication_disabled;
                     break;
                     
                  case 8:
                     outcome = SinkBase::clock_failure_logger_permission;
                     break;
                     
                  case 9:
                     outcome = SinkBase::clock_failure_invalid_uri;
                     break;
                     
                  case 10:
                     outcome = SinkBase::clock_failure_busy;
                     break;
                     
                  case 11:
                     outcome = SinkBase::clock_failure_unsupported;
                     break;
                  }
               }
               else
               {
                  switch(response_outcome)
                  {
                  case 1:
                     outcome = SinkBase::clock_success_set;
                     logger_time = response.get_property_date("time");
                     break;
                     
                  case 2:
                     outcome = SinkBase::clock_failure_connection;
                     break;
                     
                  case 3:
                     outcome = SinkBase::clock_failure_invalid_logon;
                     break;
                     
                  case 4:
                     outcome = SinkBase::clock_failure_server_permission;
                     break;
                     
                  case 5:
                     outcome = SinkBase::clock_failure_communication;
                     break;
                     
                  case 6:
                     outcome = SinkBase::clock_failure_communication_disabled;
                     break;
                     
                  case 7:
                     outcome = SinkBase::clock_failure_logger_permission;
                     break;
                     
                  case 8:
                     outcome = SinkBase::clock_failure_invalid_uri;
                     break;
                     
                  case 9:
                     outcome = SinkBase::clock_failure_busy;
                     break;
                  }
               }
            }
            catch(std::exception &)
            { outcome = SinkBase::clock_failure_communication; }

            // we can now dispatch the result
            if(SinkBase::is_valid_instance(sink))
            {
               sink->on_clock_complete(
                  source->get_manager(),
                  source_uri,
                  outcome,
                  logger_time);
               sink = 0;
            }
            source->end_clock_checker(this);
            return false;
         } // on_response_complete


         class FileControl: public Csi::HttpClient::RequestClient
         {
         private:
            /**
             * Specifies the source.
             */
            HttpSource *source;

            /**
             * Specifies the sink.
             */
            SinkBase *sink;

            /**
             * Specifies the station uri.
             */
            StrUni const source_uri;

            /**
             * Specifies the command code.
             */
            uint4 const command;

            /**
             * Specifies the first parameter.
             */
            StrAsc const p1;

            /**
             * Specifies the second parameter.
             */
            StrAsc const p2;

            /**
             * Specifies the HTTP connection.
             */
            HttpSource::http_connection_handle http_connection;

            /**
             * Specifies the HTTP request.
             */
            typedef Csi::HttpClient::Request request_type;
            Csi::SharedPtr<request_type> request;
            
         public:
            /**
             * Constructor
             */
            FileControl(
               HttpSource *source_,
               SinkBase *sink_,
               StrUni const &source_uri_,
               uint4 command_,
               StrAsc const &p1,
               StrAsc const &p2);

            /**
             * Destructor
             */
            virtual ~FileControl();

            /**
             * Overloads the failure handler.
             */
            virtual bool on_failure(request_type *request);

            /**
             * Overloads the response handler.
             */
            virtual bool on_response_complete(request_type *request);
         };


         FileControl::FileControl(
            HttpSource *source_,
            SinkBase *sink_,
            StrUni const &source_uri_,
            uint4 command_,
            StrAsc const &p1_,
            StrAsc const &p2_):
            source(source_),
            sink(sink_),
            source_uri(source_uri_),
            command(command_),
            p1(p1_),
            p2(p2_)
         {
            // we need to form the web URI.
            Csi::Uri web_uri;
            Csi::OStrAscStream temp;
            
            web_uri.set_server_address(source->get_server_address());
            web_uri.set_server_port(source->get_server_port());
            if(source->get_use_https())
               web_uri.set_protocol("https");
            web_uri.insert("command", "FileControl");
            web_uri.insert("format", "json");
            temp << command;
            web_uri.insert("action", temp.str());
            web_uri.insert("file", p1);
            web_uri.insert("file2", p2);
            
            // we can now sent the request
            http_connection.bind(new Csi::HttpClient::Connection);
            http_connection->set_watcher(source->get_watcher());
            request.bind(new request_type(this, web_uri));
            request->set_authentication(source->get_user_name(), source->get_password());
            http_connection->add_request(request);
         } // constructor


         FileControl::~FileControl()
         {
            request.clear();
            http_connection.clear();
         } // destructor


         bool FileControl::on_failure(request_type *sender)
         {
            SinkBase::file_control_outcome_type outcome(SinkBase::filecontrol_failure_connection);

            if(sender->get_response_code() == 401)
               outcome = SinkBase::filecontrol_failure_logger_permission;
            if(SinkBase::is_valid_instance(sink))
               sink->on_file_control_complete(
                  source->get_manager(), source_uri, command, p1, p2, outcome);
            source->end_file_control(this);
            return false;
         } // on_failure


         bool FileControl::on_response_complete(request_type *sender)
         {
            SinkBase::file_control_outcome_type outcome(SinkBase::filecontrol_failure_unknown);
            uint2 hold_off(0);

            try
            {
               Csi::Json::Object response;
               Csi::IByteQueueStream response_str(&request->get_receive_buff());
               int4 response_outcome;
               response.parse(response_str);
               response_outcome = static_cast<int4>(response.get_property_number("outcome"));
               switch(response_outcome)
               {
               case 0:
                  outcome = SinkBase::filecontrol_success;
                  hold_off = static_cast<uint2>(response.get_property_number("holdoff"));
                  break;

               case 1:
                  outcome = SinkBase::filecontrol_failure_logger_permission;
                  break;

               case 2:
                  outcome = SinkBase::filecontrol_failure_invalid_logon;
                  break;

               case 3:
                  outcome = SinkBase::filecontrol_failure_connection;
                  break;

               case 4:
                  outcome = SinkBase::filecontrol_failure_invalid_uri;
                  break;

               case 5:
                  outcome = SinkBase::filecontrol_failure_unsupported;
                  break;

               case 6:
                  outcome = SinkBase::filecontrol_failure_server_permission;
                  break;

               case 7:
                  outcome = SinkBase::filecontrol_failure_communication;
                  break;

               case 8:
                  outcome = SinkBase::filecontrol_failure_communication_disabled;
                  break;

               case 9:
                  outcome = SinkBase::filecontrol_failure_logger_resources;
                  break;

               case 10:
                  outcome = SinkBase::filecontrol_failure_logger_locked;
                  break;

               case 11:
                  outcome = SinkBase::filecontrol_failure_logger_root_dir_full;
                  break;

               case 12:
                  outcome = SinkBase::filecontrol_failure_file_busy;
                  break;

               case 13:
                  outcome = SinkBase::filecontrol_failure_invalid_file_name;
                  break;

               case 14:
                  outcome = SinkBase::filecontrol_failure_drive_busy;
                  break;

               case 19:
                  outcome = SinkBase::filecontrol_failure_unsupported_command;
                  break;

               case 20:
                  outcome = SinkBase::filecontrol_failure_logger_root_dir_full;
                  break;
               }
            }
            catch(std::exception &)
            { }
            if(SinkBase::is_valid_instance(sink))
            {
               sink->on_file_control_complete(
                  source->get_manager(),
                  source_uri,
                  command,
                  p1,
                  p2,
                  outcome,
                  hold_off);
            }
            source->end_file_control(this, hold_off);
            return false;
         } // on_response_complete


         class AccessChecker: public Csi::HttpClient::RequestClient
         {
         public:
            /**
             * Constructor
             *
             * @param source_ Specifies the source that owns this operation.
             *
             * @param sink_ Specifies the application object that will receive notice of completion.
             */
            AccessChecker(HttpSource *source_, SinkBase *sink_);

            /**
             * Destructor
             */
            virtual ~AccessChecker();

            /**
             * Overloads the base class version to handle a request failure.
             */
            typedef Csi::HttpClient::Request request_type;
            virtual bool on_failure(request_type *sender);

            /**
             * Overloads the base class version to handle a completion notification.
             */
            virtual bool on_response_complete(request_type *sender);

         private:
            /**
             * Handles the work of reporting the outcome to the sink and closing this operation.
             */
            void on_complete(SinkBase::check_access_level_outcome_type outcome, int access);

         private:
            /**
             * Specifies the source.
             */
            HttpSource *source;

            /**
             * Specifies the sink.
             */
            SinkBase *sink;

            /**
             * Specifies the HTTP connection used for the request.
             */
            typedef HttpSource::http_connection_handle http_connection_handle;
            http_connection_handle http_connection;

            /**
             * Specifies the web request.
             */
            Csi::SharedPtr<request_type> request;

            /**
             * Specifies the state of this operation.
             */
            enum state_type
            {
               state_check_no_anonymous,
               state_check_status,
               state_check_file_control
            } state;
         };


         AccessChecker::AccessChecker(HttpSource *source_, SinkBase *sink_):
            source(source_),
            sink(sink_),
            state(state_check_no_anonymous)
         {
            // make sure that the HTTP connection is allocated.
            if(http_connection == 0)
            {
               http_connection.bind(new Csi::HttpClient::Connection);
               http_connection->set_watcher(source->get_watcher());
            }

            // we need to form the URI
            Csi::Uri web_uri;
            web_uri.set_server_address(source->get_server_address());
            web_uri.set_server_port(source->get_server_port());
            if(source->get_use_https())
               web_uri.set_protocol("https");
            web_uri.insert("command", "CheckAuthorization");
            web_uri.insert("format", "json");
            web_uri.insert("anonymous", "false");

            // we can now allocate and send the request.
            request.bind(new request_type(this, web_uri));
            request->set_authentication(source->get_user_name(), source->get_password());
            http_connection->add_request(request);
         } // constructor


         AccessChecker::~AccessChecker()
         {
            http_connection.clear();
            request.clear();
            source = 0;
            sink = 0;
         } // destructor


         bool AccessChecker::on_failure(request_type *sender)
         {
            bool rtn(true);
            Csi::Uri web_uri;
            web_uri.set_server_address(source->get_server_address());
            web_uri.set_server_port(source->get_server_port());
            if(source->get_use_https())
               web_uri.set_protocol("https");
            if(state == state_check_no_anonymous)
            {
               if(request->get_response_code() == 404)
               {
                  web_uri.insert("command", "SetValueEx");
                  web_uri.insert("format", "json");
                  web_uri.insert("uri", "dl:Status.OSVersion");
                  web_uri.insert("value", "1");
                  state = state_check_status;
                  request.bind(new request_type(this, web_uri));
                  request->set_authentication(source->get_user_name(), source->get_password());
                  http_connection->add_request(request);
               }
               else
                  on_complete(SinkBase::check_access_level_outcome_success, Csi::PasswdHelpers::access_none);
            }
            else if(state == state_check_status)
            {
               StrAsc desc(request->get_response_description());
               if(desc == "Forbidden" || desc == "Authorization Required")
                  on_complete(SinkBase::check_access_level_outcome_success, Csi::PasswdHelpers::access_none);
               else if(desc == "Not Found")
                  on_complete(SinkBase::check_access_level_outcome_success, Csi::PasswdHelpers::access_read_write);
               else
                  on_complete(SinkBase::check_access_level_outcome_success, Csi::PasswdHelpers::access_none);
            }
            else if(state == state_check_file_control)
            {
               StrAsc desc(request->get_response_description());
               if(desc == "Forbidden" || desc == "Authorization Required")
                  on_complete(SinkBase::check_access_level_outcome_success, Csi::PasswdHelpers::access_read_write);
               else
                  on_complete(SinkBase::check_access_level_outcome_success, Csi::PasswdHelpers::access_none);
            }
            return rtn;
         } // on_failure


         bool AccessChecker::on_response_complete(request_type *sender)
         {
            bool rtn(true);
            try
            {
               if(state == state_check_no_anonymous)
               {
                  if(request->get_response_code() == 200)
                  {
                     Csi::Json::Object response;
                     Csi::IByteQueueStream response_str(&request->get_receive_buff());
                     response.parse(response_str);
                     on_complete(SinkBase::check_access_level_outcome_success, static_cast<int>(response.get_property_number("authorization")));
                  }
               }
               else if(state == state_check_status)
               {
                  if(request->get_response_code() == 200)
                  {
                     Csi::Json::Object response;
                     Csi::IByteQueueStream response_str(&request->get_receive_buff());
                     int4 outcome;
                     
                     response.parse(response_str);
                     outcome = (int4)(response.get_property_number("outcome"));
                     if(outcome == 12)
                        on_complete(SinkBase::check_access_level_outcome_success, Csi::PasswdHelpers::access_read_only);
                     else
                     {
                        Csi::Uri web_uri;
                        web_uri.set_server_address(source->get_server_address());
                        web_uri.set_server_port(source->get_server_port());
                        if(source->get_use_https())
                           web_uri.set_protocol("https");
                        web_uri.insert("command", "FileControl");
                        web_uri.insert("format", "json");
                        web_uri.insert("uri", "dl:");
                        web_uri.insert("action", "90");
                        web_uri.insert("p1", "");
                        web_uri.insert("p2", "");
                        state = state_check_file_control;
                        request.bind(new request_type(this, web_uri));
                        request->set_authentication(source->get_user_name(), source->get_password());
                        http_connection->add_request(request);
                     }
                  }
               }
               else if(state == state_check_file_control)
               {
                  if(request->get_response_code() == 200)
                  {
                     Csi::Json::Object response;
                     Csi::IByteQueueStream response_str(&request->get_receive_buff());
                     int4 outcome;
                     response.parse(response_str);
                     outcome = (int4)response.get_property_number("outcome");
                     if(outcome == 1)
                        on_complete(SinkBase::check_access_level_outcome_success, Csi::PasswdHelpers::access_read_write);
                     else
                        on_complete(SinkBase::check_access_level_outcome_success, Csi::PasswdHelpers::access_all);
                  }
               }
            }
            catch(std::exception &)
            { on_complete(SinkBase::check_access_level_outcome_failure_communication, Csi::PasswdHelpers::access_none); }
            return rtn;
         } // on_response_complete


         void AccessChecker::on_complete(SinkBase::check_access_level_outcome_type outcome, int access)
         {
            HttpSource *owner(source);
            if(SinkBase::is_valid_instance(sink))
               sink->on_check_access_level_complete(
                  source->get_manager(), outcome, static_cast<Csi::PasswdHelpers::access_level_type>(access));
            source = 0;
            sink = 0;
            owner->end_get_access_level(this);
         } // on_complete
      };

      
      namespace
      {
         uint4 const std_retry_interval(10000);
      };

      
      ////////////////////////////////////////////////////////////
      // class HttpSource definitions
      ////////////////////////////////////////////////////////////
      HttpSource::HttpSource(StrUni const &source_name):
         SourceBase(source_name),
         server_port(80),
         was_connected(false),
         connect_active(false),
         poll_schedule_interval(300000),
         retry_id(0),
         hold_off_id(0),
         poll_schedule_id(0),
         poll_schedule_enabled(true),
         use_websocket(true),
         last_websock_tran(0),
         terminal_sink(0),
         terminal_sink_token(0),
         log_enabled(false),
         log_interval(0)
      { }


      HttpSource::~HttpSource()
      {
         if(symbol != 0)
            symbol->set_source(0);
         symbol.clear();
         if(timer != 0 && retry_id != 0)
            timer->disarm(retry_id);
         if(timer != 0 && hold_off_id != 0)
            timer->disarm(hold_off_id);
         if(scheduler != 0 && poll_schedule_id)
            scheduler->cancel(poll_schedule_id);
         scheduler.clear();
         timer.clear();
         websocket.clear();
         watcher.clear();
      } // destructor


      void HttpSource::set_manager(Manager *manager)
      {
         SourceBase::set_manager(manager);
         if(manager)
         {
            timer = manager->get_timer();
            scheduler.bind(new Scheduler(timer));
         }
      } // set_manager
      

      void HttpSource::connect()
      {
         was_connected = true;
         if(!connect_active && connect_request == 0)
         {
            Csi::Uri uri;
            uri.set_server_address(server_address);
            uri.set_server_port(server_port);
            if(use_https)
               uri.set_protocol("https");
            uri.insert("command", "ClockCheck");
            uri.insert("format", "json");
            manager->report_source_connecting(this);
            connect_request.bind(new http_request(this, uri));
            connect_request->set_authentication(user_name, password);
            if(log_enabled)
               watcher.bind(new Csi::HttpClient::ConnectionWatcherLog(log_path, log_file, log_interval * 1000, timer));
            else
               watcher.clear();
            connection.bind(new http_connection_type);
            connection->set_watcher(watcher);
            connection->set_wait_interval(poll_schedule_interval * 2);
            connection->add_request(connect_request);
         }
      } // connect

      
      void HttpSource::disconnect()
      {
         bool report_disconnect(connect_active);
         connect_active = was_connected = false;
         connection.clear();
         watcher.clear();
         connect_request.clear();
         websocket.clear();
         if(report_disconnect)
            manager->report_source_disconnect(this, ManagerClient::disconnect_by_application);
      } // disconnect

      
      bool HttpSource::is_connected() const
      { return connect_active; }


      StrUni const HttpSource::server_address_name(L"server-address");
      StrUni const HttpSource::server_port_name(L"server-port");
      StrUni const HttpSource::user_name_name(L"user-name");
      StrUni const HttpSource::password_name(L"password");
      StrUni const HttpSource::poll_schedule_base_name(L"poll-base");
      StrUni const HttpSource::poll_schedule_interval_name(L"poll-interval");
      StrUni const HttpSource::use_https_name(L"use-https");

      
      void HttpSource::get_properties(Csi::Xml::Element &prop_xml)
      {
         prop_xml.set_attr_str(server_address, server_address_name);
         prop_xml.set_attr_uint2(server_port, server_port_name);
         prop_xml.set_attr_bool(use_https, use_https_name);
         prop_xml.set_attr_str(user_name, user_name_name);
         prop_xml.set_attr_str(password, password_name);
         prop_xml.set_attr_lgrdate(poll_schedule_base, poll_schedule_base_name);
         prop_xml.set_attr_uint4(poll_schedule_interval, poll_schedule_interval_name);
         if(log_enabled)
         {
            Csi::Xml::Element::value_type log_xml(prop_xml.add_element(log_name));
            log_xml->set_attr_str(log_path, log_path_name);
            log_xml->set_attr_str(log_file, log_file_name);
            log_xml->set_attr_int8(log_interval, log_interval_name);
         }
      } // get_properties

      
      void HttpSource::set_properties(Csi::Xml::Element &prop_xml)
      {
         // we will set the optional properties to their default values
         bool reconnect = was_connected;
         Csi::Xml::Element::iterator li(prop_xml.find(log_name));
         
         disconnect();
         server_port = 80;
         user_name = password = "";
         poll_schedule_base = 0;
         poll_schedule_interval = 300000;

         // we can now apply the attributes
         server_address = prop_xml.get_attr_str(server_address_name);
         if(prop_xml.has_attribute(server_port_name))
            server_port = prop_xml.get_attr_uint2(server_port_name);
         if(prop_xml.has_attribute(use_https_name))
            use_https = prop_xml.get_attr_bool(use_https_name);
         else
            use_https = (server_port == 443);
         if(prop_xml.has_attribute(user_name_name))
         {
            user_name = prop_xml.get_attr_str(user_name_name);
            password = prop_xml.get_attr_str(password_name);
         }
         if(prop_xml.has_attribute(poll_schedule_base_name))
            poll_schedule_base = prop_xml.get_attr_lgrdate(poll_schedule_base_name);
         if(prop_xml.has_attribute(poll_schedule_interval_name))
            poll_schedule_interval = prop_xml.get_attr_uint4(poll_schedule_interval_name);
         if(li != prop_xml.end())
         {
            Csi::Xml::Element::value_type &log_xml(*li);
            log_enabled = true;
            log_path = log_xml->get_attr_str(log_path_name);
            log_file = log_xml->get_attr_str(log_file_name);
            log_interval = log_xml->get_attr_int8(log_interval_name);
            if(log_interval == 0 || log_path.length() == 0 || log_file.length() == 0)
               log_enabled = false;
         }
         else
            log_enabled = false;
         if(reconnect)
            connect();
      } // set_properties


      void HttpSource::start()
      {
         uint4 interval(poll_schedule_interval);
         SourceBase::start();
         if(interval < 100)
            interval = 100;
         poll_schedule_id = scheduler->start(this, poll_schedule_base, interval);
      } // start


      void HttpSource::stop()
      {
         if(poll_schedule_id != 0)
         {
            scheduler->cancel(poll_schedule_id);
            poll_schedule_id = 0;
         }
         SourceBase::stop();
      } // stop

      
      void HttpSource::add_request(
         request_handle &request, bool more_to_follow)
      {
         if(SinkBase::is_valid_instance(request->get_sink()))
         {
            // we need to search for a cursor that is compatible with this request.
            cursor_handle cursor;
            request->set_wart(new TableFieldUri(request->get_uri()));
            for(cursors_type::iterator ci = cursors.begin();
                cursor == 0 && ci != cursors.end();
                ++ci)
            {
               cursor_handle &candidate(*ci);
               if(candidate->add_request(request))
                  cursor = candidate;
            }
            if(cursor == 0)
            {
               cursor.bind(new cursor_type(this, request));
               cursors.push_back(cursor);
               if(!more_to_follow && connect_active)
                  cursor->poll();
            }
         }
      } // add_request

      
      void HttpSource::remove_request(request_handle &request)
      {
         for(cursors_type::iterator ci = cursors.begin(); ci != cursors.end(); ++ci)
         {
            cursor_handle &cursor(*ci);
            if(cursor->remove_request(request))
            {
               request->set_wart(0);
               if(cursor->has_no_requests())
               {
                  websock_requests_type::iterator wi(websock_requests.find(cursor->get_websock_tran()));
                  cursors.erase(ci);
                  if(wi != websock_requests.end() && websocket != 0)
                  {
                     Csi::Json::Object remove_request;
                     Csi::Json::ArrayHandle transactions(new Csi::Json::Array);
                     Csi::Json::NumberHandle transaction(new Csi::Json::Number(static_cast<double>(wi->first)));
                     remove_request.set_property_str("message", "RemoveRequests");
                     remove_request.set_property("transactions", transactions.get_handle(), false);
                     transactions->push_back(transaction.get_handle());
                     request_stream.str("");
                     remove_request.format(request_stream, false);
                     try
                     {
                        websocket->send_message(request_stream.c_str(), (uint4)request_stream.length());
                     }
                     catch(std::exception &)
                     { }
                     websock_requests.erase(wi);
                  }
               }
               break;
            }
         }
      } // remove_request

      
      void HttpSource::remove_all_requests()
      {
         cursors.clear();
         if(!websock_requests.empty() && websocket != 0)
         {
            Csi::Json::Object remove_request;
            Csi::Json::ArrayHandle transactions(new Csi::Json::Array);
            remove_request.set_property_str("message", "RemoveRequests");
            remove_request.set_property("transactions", transactions.get_handle(), false);
            for(websock_requests_type::iterator wi = websock_requests.begin();
                wi != websock_requests.end();
                ++wi)
            {
               Csi::Json::NumberHandle transaction(new Csi::Json::Number(static_cast<double>(wi->first)));
               transactions->push_back(transaction.get_handle());
            }
            request_stream.str("");
            remove_request.format(request_stream, false);
            try
            {
               websocket->send_message(request_stream.c_str(), (uint4)request_stream.length());
            }
            catch(std::exception &)
            { }
            websock_requests.clear();
         }
      } // remove_all_requests

      
      void HttpSource::activate_requests()
      {
         for(cursors_type::iterator ci = cursors.begin(); ci != cursors.end(); ++ci)
         {
            cursor_handle &cursor(*ci);
            if(!cursor->get_previously_polled())
               cursor->poll();
         }
      } // activate_requests

      
      HttpSource::symbol_handle HttpSource::get_source_symbol()
      {
         if(symbol == 0)
         {
            symbol.bind(
               new HttpSymbol(
                  this, get_name(), 0, SymbolBase::type_http_source));
         }
         return symbol.get_handle();
      } // get_source_symbol

      
      void HttpSource::breakdown_uri(symbols_type &symbols, StrUni const &uri_)
      {
         TableFieldUri uri(uri_);
         symbols.clear();
         if(uri.get_source_name().length() > 0)
         {
            symbols.push_back(symbol_type(uri.get_source_name(), SymbolBase::type_http_source));
            if(uri.get_table_name().length() > 0)
            {
               symbols.push_back(symbol_type(uri.get_table_name(), SymbolBase::type_table));
               if(uri.get_column_name().length() > 0)
                  symbols.push_back(symbol_type(uri.get_column_name(), SymbolBase::type_scalar));
            }
         }
      } // breakdown_uri


      bool HttpSource::on_failure(http_request *request)
      {
         uint4 ref_count = connection.get_reference_count();
         connect_active = false;
         retry_id = timer->arm(this, std_retry_interval);
         connect_request.clear();
         connection.clear();
         manager->report_source_log(this, "connect request failed");
         manager->report_source_disconnect(this, ManagerClient::disconnect_connection_failed);
         return ref_count > 1;
      } // on_failure


      bool HttpSource::on_response_complete(http_request *request)
      {
         bool rtn = true;
         bool report_error(false);
         if(request->get_response_code() == 200)
         {
            try
            {
               // we need to verify the content type that was returned.  We'll start with the
               // mime type
               if(request->get_content_type().find("application/json") >= request->get_content_type().length())
                  throw std::invalid_argument("invalid content type returned");

               // we also need to verify the response is what we expected
               Csi::Json::Object response;
               Csi::IByteQueueStream response_str(&request->get_receive_buff());
               double outcome;
               
               response.parse(response_str);
               outcome = response.get_property_number("outcome");
               if(outcome != 1 && outcome != 2)
                  throw std::invalid_argument("invalid connect response");

               // if web sockets are enabled, we will need to establish a connection to the
               // websocket service before we can proceed.
               if(use_websocket)
               {
                  Csi::Uri uri;
                  websocket.bind(new websock_type(timer, poll_schedule_interval * 2));
                  websocket->set_watcher(watcher);
                  uri.set_server_address(server_address);
                  uri.set_server_port(server_port);
                  if(use_https)
                     uri.set_protocol("wss");
                  else
                     uri.set_protocol("ws");
                  websocket->connect(this, uri, "com.campbellsci.webdata", user_name, password);
               }
               else
               {
                  // if we made it this far, then we are happy with the connection
                  was_connected = true;
                  connect_active = true;
                  manager->report_source_connect(this);
                  if(is_started())
                     start();
               }
            }
            catch(std::exception &e)
            {
               manager->report_source_log(this, e.what());
               report_error = true;
            }
         }
         else if(request->get_response_code() == 301)
         {
            try
            {
               // we need to handle the redirect by updating the server address, port, and protocol.
               Csi::Uri redirect(request->get_location());
               if(redirect.get_protocol() == "https")
                  use_https = true;
               else
                  use_https = false;
               server_address = redirect.get_server_address();
               server_port = redirect.get_server_port();
               connect_active = false;
               connect_request.clear();
               connect();
            }
            catch(std::exception &e)
            {
               manager->report_source_log(this, e.what());
               report_error = true;
            } 
         }
         else
            report_error = true;

         // if there was a failure handling the response, we will need to report a failure.
         if(report_error)
         {
            ManagerClient::disconnect_reason_type failure(ManagerClient::disconnect_connection_failed);
            SinkBase::sink_failure_type sink_failure(SinkBase::sink_failure_connection_failed);
            
            cursors_type temp_cursors(cursors);
            if(request->get_response_code() == 401)
            {
               failure =  ManagerClient::disconnect_invalid_logon;
               sink_failure = SinkBase::sink_failure_invalid_logon;
               manager->report_source_log(this, "invalid logon");
            }
            was_connected = false;
            retry_id = timer->arm(this, std_retry_interval);
            cursors.clear();
            while(!temp_cursors.empty())
            {
               cursor_handle cursor(temp_cursors.front());
               temp_cursors.pop_front();
               cursor->on_failure(sink_failure);
            }
            manager->report_source_disconnect(this, failure);
         }
         return rtn;
      } // on_response_complete


      void HttpSource::on_connected(websock_type *sender)
      {
         was_connected = true;
         connect_active = true;
         connect_request.clear();
         manager->report_source_connect(this);
         if(is_started())
            start();
      } // on_connected


      void HttpSource::on_failure(
         websock_type *sender,
         WebSocketClient::failure_type failure,
         int http_response)
      {
         // if the server reports that the upgrade is unsupported, we will resume requests using
         // normal requests.
         websocket.clear();
         if(failure == WebSocketClient::failure_unsupported)
         {
            was_connected = true;
            connect_active = true;
            use_websocket = false;
            manager->report_source_connect(this);
            if(is_started())
               start();
         }
         else
         {
            ManagerClient::disconnect_reason_type disconnect_failure(ManagerClient::disconnect_connection_failed);
            SinkBase::sink_failure_type sink_failure(SinkBase::sink_failure_connection_failed);
            cursors_type temp_cursors(cursors);
            connect_active = was_connected = false;
            retry_id = timer->arm(this, std_retry_interval);
            cursors.clear();
            connection.clear();
            if(poll_schedule_id != 0)
            {
               scheduler->cancel(poll_schedule_id);
               poll_schedule_id = 0;
            }
            while(!temp_cursors.empty())
            {
               cursor_handle cursor(temp_cursors.front());
               temp_cursors.pop_front();
               cursor->on_failure(sink_failure);
            }
            if(TerminalSinkBase::is_valid_instance(terminal_sink) && terminal_sink_token)
            {
               terminal_sink->on_terminal_failed(manager, terminal_sink_token, TerminalSinkBase::terminal_failure_connection);
               terminal_sink = 0;
               terminal_sink_token = 0;
            }
            manager->report_source_disconnect(this, disconnect_failure);
         }
      } // on_failure


      void HttpSource::on_message(
         websock_type *sender,
         void const *content,
         uint4 content_len,
         Csi::HttpClient::websock_op_code op_code,
         bool fin)
      {
         if(op_code == Csi::HttpClient::websock_op_text || op_code == Csi::HttpClient::websock_op_continuation)
         {
            websock_buffer.append(content, content_len);
            if(fin)
            {
               try
               {
                  // we need to parse the message.
                  Csi::Json::Object response;
                  Csi::IBuffStream input(websock_buffer.getContents(), websock_buffer.length());
                  input.imbue(Csi::StringLoader::make_locale(0));
                  response.parse(input);
                  websock_buffer.cut(0);

                  // we now need to dispatch the message
                  StrAsc message_type(response.get_property_str("message"));
                  if(message_type == "RequestStarted")
                     on_request_started(response);
                  else if(message_type == "RequestRecords")
                     on_request_records(response);
                  else if(message_type == "RequestFailed")
                     on_request_failed(response);
                  else if(message_type == "TerminalData")
                     on_terminal_data(response);
               }
               catch(std::exception &e)
               {
                  websock_buffer.cut(0);
                  manager->report_source_log(this, e.what());
               }
            }
         }
      } // on_message


      void HttpSource::onOneShotFired(uint4 id)
      {
         if(id == retry_id)
         {
            retry_id = 0;
            connect();
         }
         if(id == hold_off_id)
         {
            hold_off_id = 0;
            connect();
            if(symbol != 0)
               symbol->refresh();
         }
      } // onOneShotFired


      void HttpSource::onScheduledEvent(uint4 id)
      {
         if(id == poll_schedule_id && poll_schedule_enabled && connect_active)
         {
            // we need to add any cursors for requests that have not been serviced.
            for(Manager::requests_iterator ri = manager->requests_begin();
                ri != manager->requests_end();
                ++ri)
            {
               request_handle &request(*ri);
               if(request->get_source() == this && request->get_wart() == 0)
                  add_request(request, false);
            }
            for(cursors_type::iterator ci = cursors.begin(); ci != cursors.end(); ++ci)
            {
               cursors_type::value_type &cursor(*ci);
               cursor->poll();
            }
         }
      } // onScheduledEvent


      void HttpSource::remove_cursor(cursor_type *cursor)
      {
         cursors_type::iterator ci(
            std::find_if(cursors.begin(), cursors.end(), Csi::HasSharedPtr<cursor_type>(cursor)));
         if(ci != cursors.end())
            cursors.erase(ci);
      } // remove_cursor


      bool HttpSource::start_set_value(
         SinkBase *sink, StrUni const &uri, ValueSetter const &value)
      {
         value_setters.push_back(new value_setter_type(sink, uri.to_utf8(), value, this));
         return true;
      } // start_set_value


      void HttpSource::end_set_value(value_setter_type *setter)
      {
         value_setters_type::iterator si(
            std::find_if(
               value_setters.begin(), value_setters.end(), Csi::HasSharedPtr<value_setter_type>(setter)));
         if(si != value_setters.end())
            value_setters.erase(si);
      } // end_set_value


      bool HttpSource::start_send_file(
         SinkBase *sink,
         StrUni const &uri,
         StrUni const &dest_file_name,
         StrUni const &file_name)
      {
         file_senders.push_back(new file_sender_type(sink, uri, dest_file_name, file_name, this));
         return true;
      }


      void HttpSource::end_send_file(file_sender_type *sender)
      {
         file_senders_type::iterator fi(
            std::find_if(
               file_senders.begin(), file_senders.end(), Csi::HasSharedPtr<file_sender_type>(sender)));
         if(fi != file_senders.end())
            file_senders.erase(fi);
      } // end_send_file


      bool HttpSource::start_get_newest_file(
         SinkBase *sink,
         StrUni const &uri,
         StrUni const &pattern)
      {
         newest_file_getters.push_back(new newest_file_getter_type(this, sink, uri, pattern));
         return true;
      } // start_get_newest_file


      void HttpSource::end_get_newest_file(newest_file_getter_type *getter)
      {
         newest_file_getters_type::iterator gi(
            std::find_if(
               newest_file_getters.begin(), newest_file_getters.end(), Csi::HasSharedPtr<newest_file_getter_type>(getter)));
         if(gi != newest_file_getters.end())
            newest_file_getters.erase(gi);
      } // end_get_newest_file


      bool HttpSource::start_clock_check(
         SinkBase *sink,
         StrUni const &uri,
         bool should_set,
         bool send_server_time,
         Csi::LgrDate const &server_time)
      {
         clock_checkers.push_back(
            new clock_checker_type(
               this, sink, uri, should_set, send_server_time, server_time));
         return true;
      } // start_clock_check


      void HttpSource::end_clock_checker(clock_checker_type *checker)
      {
         clock_checkers_type::iterator ci(
            std::find_if(
               clock_checkers.begin(), clock_checkers.end(), Csi::HasSharedPtr<clock_checker_type>(checker)));
         if(ci != clock_checkers.end())
            clock_checkers.erase(ci);
      } // end_clock_checker


      bool HttpSource::start_file_control(
         SinkBase *sink,
         StrUni const &uri,
         uint4 command,
         StrAsc const &p1,
         StrAsc const &p2)
      {
         file_controls.push_back(
            new file_control_type(this, sink, uri, command, p1, p2));
         return true;
      } // start_file_control


      void HttpSource::end_file_control(file_control_type *control, uint4 hold_off)
      {
         file_controls_type::iterator fi(
            std::find_if(
               file_controls.begin(),
               file_controls.end(),
               Csi::HasSharedPtr<file_control_type>(control)));
         if(fi != file_controls.end())
            file_controls.erase(fi);
         if(hold_off > 0)
         {
            hold_off_id = timer->arm(this, hold_off * Csi::LgrDate::msecPerSec);
            connect_active = was_connected = false;
            connection.clear();
            watcher.clear();
            connect_request.clear();
            websocket.clear();
            manager->report_source_disconnect(this, ManagerClient::disconnect_connection_failed);
         }
      } // end_file_control
      

      int4 HttpSource::start_websock_request(cursor_type *cursor_, Csi::Json::ObjectHandle &request)
      {
         // we need to find the cursor
         int4 rtn(-1);
         cursors_type::iterator ci(std::find_if(cursors.begin(), cursors.end(), Csi::HasSharedPtr<cursor_type>(cursor_)));
         if(ci != cursors.end() && websocket != 0)
         {
            // we need to allocate a new web socket transaction number.
            int4 new_tran(last_websock_tran + 1);
            cursor_handle &cursor(*ci);
            if(new_tran <= 0)
               new_tran = 1;
            if(websock_requests.find(new_tran) != websock_requests.end())
               new_tran = websock_requests.rbegin()->second->get_websock_tran() + 1;
            last_websock_tran = rtn = new_tran;

            // we need to add the cursor with the new transaction number
            Csi::Json::Object add_request;
            Csi::Json::ArrayHandle requests(new Csi::Json::Array);
            requests->push_back(request.get_handle());
            add_request.set_property_str("message", "AddRequests");
            add_request.set_property("requests", requests.get_handle(), false);
            request->set_property("transaction", new Csi::Json::Number(new_tran));
            websock_requests[new_tran] = cursor;
            request_stream.str("");
            add_request.format(request_stream, false);
            websocket->send_message(request_stream.c_str(), (uint4)request_stream.length());
         }
         return rtn;
      } // start_websock_request


      void HttpSource::on_request_started(Csi::Json::Object &message)
      {
         try
         {
            // we need to look up the transaction number from the message
            int4 transaction(static_cast<int4>(message.get_property_number("transaction")));
            websock_requests_type::iterator wi(websock_requests.find(transaction));
            if(wi != websock_requests.end())
               wi->second->on_request_started(message);
         }
         catch(std::exception &)
         { }
      } // on_request_started


      void HttpSource::on_request_records(Csi::Json::Object &message)
      {
         try
         {
            // we need to look up the transaction number from the message
            int4 transaction(static_cast<int4>(message.get_property_number("transaction")));
            websock_requests_type::iterator wi(websock_requests.find(transaction));
            if(wi != websock_requests.end())
               wi->second->on_request_records(message);
         }
         catch(std::exception &)
         { }
      } // on_request_records


      void HttpSource::on_request_failed(Csi::Json::Object &message)
      {
         try
         {
            // we need to look up the transaction number from the message
            int4 transaction(static_cast<int4>(message.get_property_number("transaction")));
            websock_requests_type::iterator wi(websock_requests.find(transaction));
            if(wi != websock_requests.end())
            {
               cursor_handle cursor(wi->second);
               websock_requests.erase(wi);
               cursor->on_request_failed(message);
            }
         }
         catch(std::exception &)
         { }
      } // on_request_failed


      bool HttpSource::start_terminal(
         TerminalSinkBase *sink, StrUni const &station_uri, int8 sink_token)
      {
         bool rtn(use_websocket && was_connected);
         if(rtn && was_connected)
         {
            if(terminal_sink == 0 && terminal_sink_token == 0)
            {
               Csi::Json::Object message;
               terminal_sink = sink;
               terminal_sink_token = sink_token;
               message.set_property_str("message", "StartTerminal", false);
               message.set_property_number("transaction", (double)sink_token, false);
               message.set_property_str("station_uri", station_uri.to_utf8(), false);
               request_stream.str("");
               message.format(request_stream, false);
               try
               {
                  websocket->send_message(request_stream.c_str(), (uint4)request_stream.length());
               }
               catch(std::exception &)
               { }
            }
            else
            {
            }
         }
         return rtn;
      } // start_terminal


      void HttpSource::send_terminal(
         TerminalSinkBase *sink, int8 sink_token, void const *buff_, size_t buff_len)
      {
         if(sink == terminal_sink && sink_token == terminal_sink_token)
         {
            Csi::Json::Object message;
            char const *buff(static_cast<char const *>(buff_));
            bool binary(false);
            StrAsc content;

            for(size_t i = 0; !binary && i < buff_len; ++i)
            {
               if(buff[i] == 0)
               {
                  Csi::Base64::encode(content, buff, buff_len);
                  binary = true;
               }
            }
            if(!binary)
               content.setContents(buff, buff_len);
            message.set_property_str("message", "TerminalData", false);
            message.set_property_number("transaction", (double)sink_token, false);
            message.set_property_number("status", 1, false);
            message.set_property_bool("binary", binary, false);
            message.set_property_str("content", content, false);
            request_stream.str("");
            message.format(request_stream, false);
            try
            {
               websocket->send_message(request_stream.c_str(), (uint4)request_stream.length());
            }
            catch(std::exception &)
            {
               terminal_sink = 0;
               terminal_sink_token = 0;
            }
         }
      } // send_terminal


      void HttpSource::close_terminal(
         TerminalSinkBase *sink, int8 sink_token)
      {
         if(sink == terminal_sink && sink_token == terminal_sink_token)
         {
            Csi::Json::Object message;
            message.set_property_str("message", "TerminalData", false);
            message.set_property_number("transaction", (double)sink_token);
            message.set_property_bool("binary", false);
            message.set_property_number("status", 6, false);
            message.set_property_str("content", "", false);
            request_stream.str("");
            message.format(request_stream, false);
            try
            {
               websocket->send_message(request_stream.c_str(), (uint4)request_stream.length());
            }
            catch(std::exception &)
            { }
            terminal_sink = 0;
            terminal_sink_token = 0;
         }
      } // close_terminal


      void HttpSource::on_terminal_data(Csi::Json::Object &message)
      {
         int8 transaction((int8)message.get_property_number("transaction"));
         if(TerminalSinkBase::is_valid_instance(terminal_sink) && transaction == terminal_sink_token)
         {
            StrAsc content(message.get_property_str("content"));
            uint4 status((uint4)message.get_property_number("status"));
            StrBin binary_content(content.c_str(), content.length());
            if(message.has_property("binary") && message.get_property_bool("binary"))
               Csi::Base64::decode(binary_content, content.c_str(), content.length());
            if(status == 1)
               terminal_sink->on_terminal_content(manager, terminal_sink_token, binary_content);
            else
            {
               TerminalSinkBase::terminal_failure_type failure(TerminalSinkBase::terminal_failure_unknown);
               switch(status)
               {
               case 2:
                  failure = TerminalSinkBase::terminal_failure_logger_security_blocked;
                  break;

               case 4:
               case 5:
                  failure = TerminalSinkBase::terminal_failure_invalid_device_name;
                  break;
               }
               terminal_sink->on_terminal_failed(manager, terminal_sink_token, failure);
               terminal_sink = 0;
               terminal_sink_token = 0;
            }
         }
      } // on_terminal_data


      bool HttpSource::start_get_access_level(SinkBase *sink)
      {
         bool rtn(true);
         access_checkers.push_back(new access_checker_type(this, sink));
         return rtn;
      } // start_get_access_level


      void HttpSource::end_get_access_level(access_checker_type *checker)
      {
         access_checkers_type::iterator ci(
            std::find_if(access_checkers.begin(), access_checkers.end(), Csi::HasSharedPtr<access_checker_type>(checker)));
         if(ci != access_checkers.end())
            access_checkers.erase(ci);
      } // end_get_access_level
      

      ////////////////////////////////////////////////////////////
      // class HttpSymbol definitions
      ////////////////////////////////////////////////////////////
      HttpSymbol::HttpSymbol(
         HttpSource *source,
         StrUni const &name,
         HttpSymbol *parent,
         symbol_type_code symbol_type_):
         SymbolBase(source, name, parent),
         symbol_type(symbol_type_),
         read_only(false),
         enabled(true)
      { }


      HttpSymbol::~HttpSymbol()
      {
         connection.clear();
      } // destructor


      bool HttpSymbol::is_connected() const
      {
         bool rtn(false);
         if(symbol_type == type_http_source)
            rtn = (connection != 0);
         else
            rtn = SymbolBase::is_connected();
         return rtn;
      } // is_connected


      bool HttpSymbol::can_expand() const
      {
         bool rtn(false);
         switch(symbol_type)
         {
         case type_http_source:
         case type_table:
         case type_array:
            rtn = true;
            break;
         }
         return rtn;
      } // can_expand


      void HttpSymbol::start_expansion()
      {
         if(http_request == 0)
         {
            // we need to form the HTTP request
            Csi::Uri uri;
            Csi::OStrAscStream symbol_uri;
            HttpSource *source(static_cast<HttpSource *>(this->source));

            uri.set_server_address(source->get_server_address());
            uri.set_server_port(source->get_server_port());
            if(source->get_use_https())
               uri.set_protocol("https");
            uri.insert("command", "BrowseSymbols");
            if(parent == 0)
               symbol_uri << name << ":";
            else
               format_uri(symbol_uri);
            uri.insert("uri", symbol_uri.str());
            uri.insert("format", "json");
            http_request.bind(new http_request_type(this, uri));
            http_request->set_authentication(source->get_user_name(), source->get_password());

            // we can now send the request
            if(connection == 0)
            {
               connection = source->get_connection();
               if(connection == 0)
               {
                  connection.bind(new connection_type);
                  connection->set_watcher(source->get_watcher());
               }
            }
            connection->add_request(http_request);
         }
      } // start_expansion


      void HttpSymbol::refresh()
      {
         children.clear();
         start_expansion();
      } // refresh            


      bool HttpSymbol::on_failure(http_request_type *request)
      {
         if(http_request == request)
            http_request.clear();
         return true;
      } // on_failure


      bool HttpSymbol::on_response_complete(http_request_type *request)
      {
         try
         {
            if(request->get_response_code() == 200)
            {
               // we will first parse the response as JSON
               Csi::IByteQueueStream input(&request->get_receive_buff());
               Csi::Json::Object response;
               response.parse(input);

               // before we begin, we need to put together a list of the current child names.  As we
               // parse the list, we will remove names from this list.  At the end of parsing, all
               // remaining names will be symbols that have been deleted.
               typedef std::set<StrUni> old_names_type;
               old_names_type old_names;

               for(iterator ci = begin(); ci != end(); ++ci)
               {
                  SymbolBase::value_type &child(*ci);
                  old_names.insert(child->get_name());
               }
               
               // we will now use the symbols information in the JSON structure to refresh our own
               // child information.  We must first look for any symbols that we don't already know
               // about
               Csi::Json::ArrayHandle symbols(response["symbols"]);
               for(Csi::Json::Array::iterator si = symbols->begin(); si != symbols->end(); ++si)
               {
                  // we need to look up the information for this symbol from the JSON structure
                  Csi::Json::ObjectHandle json_symbol(*si);
                  Csi::Json::StringHandle json_symbol_name((*json_symbol)["name"]);
                  Csi::Json::NumberHandle json_symbol_type((*json_symbol)["type"]);
                  Csi::Json::BooleanHandle json_symbol_enabled((*json_symbol)["is_enabled"]);
                  Csi::Json::BooleanHandle json_symbol_read_only((*json_symbol)["is_read_only"]);
                  
                  // we need to determine whether there is already a child with the given name
                  iterator ci(find(json_symbol_name->get_value()));
                  if(ci == end())
                  {
                     // this is a new symbol so we will add and announce it
                     Csi::PolySharedPtr<SymbolBase, HttpSymbol> new_symbol(
                        new HttpSymbol(
                           static_cast<HttpSource *>(source),
                           json_symbol_name->get_value(),
                           this,
                           static_cast<symbol_type_code>(json_symbol_type->get_value_int4())));
                     new_symbol->read_only = json_symbol_read_only->get_value();
                     new_symbol->enabled = json_symbol_enabled->get_value();
                     children.push_back(new_symbol.get_handle());
                     browser->send_symbol_added(new_symbol.get_handle());
                  }
                  else
                     old_names.erase(json_symbol_name->get_value());
               }

               // we now need to remove any symbols that were not in the list sent by the server
               for(old_names_type::iterator oi = old_names.begin(); oi != old_names.end(); ++oi)
               {
                  iterator ci(find(*oi));
                  if(ci != end())
                  {
                     SymbolBase::value_type child(*ci);
                     SymbolBrowserClient::remove_reason_type reason(SymbolBrowserClient::remove_unknown);

                     switch(child->get_symbol_type())
                     {
                     case type_table:
                        reason = SymbolBrowserClient::remove_table_deleted;
                        break;
                        
                     case type_array:
                     case type_scalar:
                        reason = SymbolBrowserClient::remove_column_deleted;
                        break;
                     }
                     children.erase(ci);
                     browser->send_symbol_removed(child, reason);
                  }
               }
               browser->send_expansion_complete(this);
            }
         }
         catch(std::exception &)
         { }
         http_request.clear();
         return true;
      } // on_response_complete


      void HttpSymbol::format_uri(std::ostream &out) const
      {
         // if the parent is an array, we will need to skip it so we can substitute our own
         // name.
         if(parent != 0 && parent->get_symbol_type() == type_array)
         {
            SymbolBase const *avo(parent->get_parent());
            avo->format_uri(out);
            out << "." << get_name();
         }
         else
            SymbolBase::format_uri(out);
      } // format_uri


      void HttpSymbol::format_uri(std::wostream &out) const
      {
         if(parent != 0 && parent->get_symbol_type() == type_array)
         {
            SymbolBase const *avo(parent->get_parent());
            avo->format_uri(out);
            out << L"." << get_name();
         }
         else
            SymbolBase::format_uri(out);
      } // format_uri
         


      namespace HttpSourceHelpers
      {
         ////////////////////////////////////////////////////////////
         // function json_type_to_csi
         //
         // Converts the type CsiJson type string specified to a CSI data type.
         ////////////////////////////////////////////////////////////
         CsiDbTypeCode json_type_to_csi(StrAsc const &json_type)
         {
            CsiDbTypeCode rtn(CsiUnknown);
            bool big_endian = Csi::is_big_endian();
            
            if(json_type == "xsd:string")
               rtn = CsiAscii;
            else if(json_type == "xsd:long")
               rtn = big_endian ? CsiInt8 : CsiInt8Lsf;
            else if(json_type == "xsd:int")
               rtn = big_endian ? CsiInt4 : CsiInt4Lsf;
            else if(json_type == "xsd:unsignedInt")
               rtn = big_endian ? CsiUInt4 : CsiUInt4Lsf;
            else if(json_type == "xsd:short")
               rtn = big_endian ? CsiInt2 : CsiInt2Lsf;
            else if(json_type == "xsd:unsignedShort")
               rtn = big_endian ? CsiUInt2 : CsiUInt2Lsf;
            else if(json_type == "xsd:byte")
               rtn = CsiInt1;
            else if(json_type == "xsd:unsignedByte")
               rtn = CsiUInt1;
            else if(json_type == "xsd:float")
               rtn = big_endian ? CsiIeee4 : CsiIeee4Lsf;
            else if(json_type == "xsd:double")
               rtn = big_endian ? CsiIeee8 : CsiIeee8Lsf;
            else if(json_type == "xsd:boolean")
               rtn = CsiBool;
            else if(json_type == "xsd:dateTime")
               rtn = big_endian ? CsiLgrDate : CsiLgrDateLsf;
            return rtn;
         } // json_type_to_csi


         ////////////////////////////////////////////////////////////
         // function parse_csijson_header
         //
         // Parses the header of a CsiJson structure and generates a record description based upon
         // its contents
         ////////////////////////////////////////////////////////////
         Csi::SharedPtr<Broker::RecordDesc>
         parse_csijson_header(Csi::Json::Object &csijson)
         {
            // we first need to parse the environment fields.
            Csi::Json::ObjectHandle json_head(csijson["head"]);
            Csi::Json::ObjectHandle json_env((*json_head)["environment"]);
            Csi::Json::StringHandle json_station_name((*json_env)["station_name"]);
            Csi::Json::StringHandle json_table_name((*json_env)["table_name"]);
            Csi::Json::StringHandle json_model_no((*json_env)["model"]);
            Csi::Json::StringHandle json_serial_no((*json_env)["serial_no"]);
            Csi::Json::StringHandle json_os_version((*json_env)["os_version"]);
            Csi::Json::StringHandle json_dld_name((*json_env)["dld_name"]);
            Csi::Json::StringHandle json_dld_sig((*json_env)["dld_sig"]);
            Csi::SharedPtr<Broker::RecordDesc> rtn(
               new Broker::RecordDesc(
                  json_station_name->get_value().c_str(),
                  json_table_name->get_value().c_str()));
            rtn->model_name = json_model_no->get_value();
            rtn->serial_number = json_serial_no->get_value();
            rtn->os_version = json_os_version->get_value();
            if(json_dld_name != 0)
               rtn->dld_name = json_dld_name->get_value();
            if(json_dld_sig != 0)
               rtn->dld_signature = json_dld_sig->get_value();

            // we are now ready to parse the field names
            Csi::Json::ArrayHandle json_fields((*json_head)["fields"]);
            for(Csi::Json::Array::iterator fi = json_fields->begin(); fi != json_fields->end(); ++fi)
            {
               // pick the associated fields from the current field definition
               Csi::Json::ObjectHandle json_field(*fi);
               Csi::Json::StringHandle json_field_name((*json_field)["name"]);
               Csi::Json::StringHandle json_field_type((*json_field)["type"]);
               Csi::Json::StringHandle json_field_units((*json_field)["units"]);
               Csi::Json::StringHandle json_field_process((*json_field)["process"]);
               Broker::ValueName value_name(json_field_name->get_value().c_str());
               Broker::RecordDesc::value_type value_desc(new Broker::ValueDesc);

               value_desc->name = value_name.get_column_name();
               if(json_field_process != 0)
                  value_desc->process = json_field_process->get_value();
               if(json_field_units != 0)
                  value_desc->units = json_field_units->get_value();
               std::copy(
                  value_name.begin(),
                  value_name.end(),
                  std::back_inserter(value_desc->array_address));
               value_desc->data_type = json_type_to_csi(json_field_type->get_value());
               if(json_field->get_property_bool("settable"))
                  value_desc->modifying_cmd = 276;
               else
                  value_desc->modifying_cmd = 0;
               rtn->values.push_back(value_desc);


               // if the field is declared as a string, we will assume a max length of 64 bytes and
               // append new values for these "extra" fields.
               if(value_desc->data_type == CsiAscii)
               {
                  uint4 string_len = 64;
                  if(json_field->find("string_len") != json_field->end())
                  {
                     double temp(
                        json_field->get_property_number("string_len"));
                     if(temp < 0)
                        temp = 64;
                     string_len = static_cast<uint4>(temp);
                  }
                  value_desc->array_address.push_back(1);
                  for(uint4 i = 0; i < string_len; ++i)
                  {
                     Csi::SharedPtr<Broker::ValueDesc> str_value(new Broker::ValueDesc);
                     str_value->name = value_desc->name;
                     str_value->data_type = value_desc->data_type;
                     str_value->modifying_cmd = value_desc->modifying_cmd;
                     str_value->units = value_desc->units;
                     str_value->process = value_desc->process;
                     str_value->array_address = value_desc->array_address;
                     str_value->array_address.back() = i + 2;
                     rtn->values.push_back(str_value);
                  } 
               }
            }
            return rtn;
         } // parse_csijson_header


         ////////////////////////////////////////////////////////////
         // format_logger_time
         //
         // Formats the time stamp so that it expresses resolution that can be picked up by
         // dataloggers.  If the time stamp has resolution below 1E-6 seconds, the usec position
         // will be set to one. 
         ////////////////////////////////////////////////////////////
         void format_logger_time(std::ostream &out, Csi::LgrDate const &time)
         {
            int8 nsec(time.nsec());
            
            time.format(out, "%Y-%m-%dT%H:%M:%S");
            if(nsec > 0)
            {
               int8 usec(nsec / Csi::LgrDate::nsecPerUSec);
               if(usec == 0)
                  time.format(out, ".%51");
               else
                  time.format(out, "%x");
            }
         } // format_logger_time
            

         
         ////////////////////////////////////////////////////////////
         // class Cursor definitions
         ////////////////////////////////////////////////////////////
         Cursor::Cursor(HttpSource *source_, request_handle &request):
            source(source_),
            table_def_sig(-1),
            last_record_no(0xFFFFFFFF),
            previously_polled(false),
            websock_tran(-1)
         {
            requests.push_back(request);
         } // constructor


         Cursor::~Cursor()
         {
            // we will clear the wart on all associated requests so that they will be marked
            // as needing new cursors when the poll interval hits.
            for(requests_type::iterator ri = requests.begin(); ri != requests.end(); ++ri)
            {
               request_handle &request(*ri);
               request->set_wart(0);
            }
            requests.clear();
         } // destructor


         bool Cursor::add_request(request_handle &request)
         {
            // we will allow the combination of like requests provided that this cursor has not yet
            // been polled.
            bool rtn(false);
            if(!requests.empty() && record_desc == 0)
            {
               try
               {
                  request_handle &first(requests.front());
                  Csi::PolySharedPtr<WartBase, TableFieldUri> first_wart(first->get_wart());
                  Csi::PolySharedPtr<WartBase, TableFieldUri> request_wart(request->get_wart());
                  if(first_wart->get_table_name() == request_wart->get_table_name() &&
                     first->is_compatible(*request))
                  {
                     requests.push_back(request);
                     rtn = true;
                  } 
               }
               catch(std::exception &)
               { rtn = false; }
            }
            return rtn;
         } // add_request


         bool Cursor::remove_request(request_handle &request)
         {
            bool rtn(false);
            requests_type::iterator ri(
               std::find(requests.begin(), requests.end(), request));
            if(ri != requests.end())
            {
               rtn = true;
               requests.erase(ri);
               if(requests.empty() && http_request != 0)
               {
                  source->get_connection()->remove_request(http_request.get_rep());
                  http_request.clear();
               } 
            }
            return rtn;
         } // remove_request


         bool Cursor::has_no_requests()
         { return requests.empty(); }
         

         void Cursor::poll()
         {
            // we need to determine whether a new websocket request should be added.
            if(source->get_use_websocket() && websock_tran <= 0 && !requests.empty())
            {
               try
               {
                  Csi::Json::ObjectHandle websock_request(new Csi::Json::Object);
                  request_handle first(requests.front());
                  Csi::PolySharedPtr<WartBase, TableFieldUri> wart(first->get_wart());
                  Csi::OStrAscStream temp;
                  temp << source->get_name() << ":" << wart->get_table_name();
                  websock_request->set_property_str("uri", temp.str());
                  temp.str("");
                  switch(first->get_start_option())
                  {
                  case Request::start_at_record:
                     websock_request->set_property_str("mode", "since-record");
                     temp << first->get_record_no();
                     websock_request->set_property_str("p1", temp.str());
                     websock_request->set_property_str("p2", "");
                     break;
                     
                  case Request::start_at_time:
                     websock_request->set_property_str("mode", "since-time");
                     websock_request->set_property_date("p1", first->get_start_time());
                     websock_request->set_property_str("p2", "");
                     break;
                     
                  case Request::start_after_newest:
                  case Request::start_at_newest:
                     websock_request->set_property_str("mode", "most-recent");
                     websock_request->set_property_str("p1", "1");
                     websock_request->set_property_str("p2", "");
                     break;
                     
                  case Request::start_relative_to_newest:
                     websock_request->set_property_str("mode", "backfill");
                     temp << (-first->get_backfill_interval() / Csi::LgrDate::nsecPerSec);
                     websock_request->set_property_str("p1", temp.str());
                     websock_request->set_property_str("p2", "");
                     break;
                     
                  case Request::start_at_offset_from_newest:
                     websock_request->set_property_str("mode", "most-recent");
                     temp << first->get_start_record_offset();
                     websock_request->set_property_str("p1", temp.str());
                     websock_request->set_property_str("p2", "");
                     break;
                     
                  case Request::start_date_query:
                     websock_request->set_property_str("mode", "date-range");
                     websock_request->set_property_date("p1", first->get_start_time());
                     websock_request->set_property_date("p2", first->get_end_time());
                     break;
                  }
                  switch(first->get_order_option())
                  {
                  case Request::order_real_time:
                     websock_request->set_property_str("order", "real-time");
                     break;
                     
                  case Request::order_collected:
                  case Request::order_logged_with_holes:
                  case Request::order_logged_without_holes:
                     websock_request->set_property_str("order", "collected");
                     break;
                  }
                  websock_tran = source->start_websock_request(this, websock_request);
               }
               catch(std::exception &)
               { websock_tran = -1; }
            }
            else if(!requests.empty() && http_request == 0 && source->get_connection() != 0 && !source->get_use_websocket())
            {
               // we need to generate the parameters for the web query 
               bool new_poll(!previously_polled || last_record_no == 0xFFFFFFFF);
               StrAsc query_mode;
               StrAsc query_p1;
               StrAsc query_p2;
               request_handle first(requests.front());
               Csi::OStrAscStream temp;

               temp.imbue(std::locale::classic());
               previously_polled = true;
               switch(first->get_start_option())
               {
               case Request::start_at_record:
                  query_mode = "since-record";
                  if(new_poll)
                     temp << first->get_record_no();
                  else
                     temp << last_record_no;
                  query_p1 = temp.str();
                  break;
                  
               case Request::start_at_time:
                  if(new_poll)
                  {
                     query_mode = "since-time";
                     format_logger_time(temp, first->get_start_time());
                     query_p1 = temp.str();
                  }
                  else
                  {
                     query_mode = "since-record";
                     temp << last_record_no;
                     query_p1 = temp.str();
                  }
                  break;
                  
               case Request::start_after_newest:
               case Request::start_at_newest:
                  if(first->get_order_option() == Request::order_real_time || new_poll)
                  {
                     query_mode = "most-recent";
                     query_p1 = "1"; 
                  }
                  else
                  {
                     query_mode = "since-record";
                     temp << last_record_no;
                     query_p1 = temp.str();
                  }
                  break;
                  
               case Request::start_relative_to_newest:
                  if(new_poll)
                  {
                     query_mode = "backfill";
                     temp << (-first->get_backfill_interval() / Csi::LgrDate::nsecPerSec);
                     query_p1 = temp.str();
                  }
                  else
                  {
                     query_mode = "since-record";
                     temp << last_record_no;
                     query_p1 = temp.str();
                  }
                  break;
                  
               case Request::start_at_offset_from_newest:
                  if(new_poll)
                  {
                     query_mode = "most-recent";
                     temp << first->get_start_record_offset();
                     query_p1 = temp.str();
                  }
                  else
                  {
                     query_mode = "since-record";
                     temp << last_record_no;
                     query_p1 = temp.str();
                  }
                  break;
                  
               case Request::start_date_query:
                  if(new_poll)
                  {
                     query_mode = "date-range";
                     format_logger_time(temp, first->get_start_time());
                     query_p1 = temp.str();
                     temp.str("");
                     format_logger_time(temp, first->get_end_time());
                     query_p2 = temp.str();
                  }
                  else
                  {
                     if(last_time + 1 < first->get_end_time())
                     {
                        query_mode = "date-range";
                        format_logger_time(temp, last_time + 1);
                        query_p1 = temp.str();
                        temp.str("");
                        format_logger_time(temp, first->get_end_time());
                        query_p2 = temp.str();
                     }
                     else
                        return;
                  }
                  break;
               }

               // now that we have determined the parameters and mode for the query, we can launch
               // the query.
               Csi::Uri uri;
               Csi::PolySharedPtr<WartBase, TableFieldUri> wart(first->get_wart());
               
               uri.set_server_address(source->get_server_address());
               uri.set_server_port(source->get_server_port());
               if(source->get_use_https())
                  uri.set_protocol("https");
               temp.str("");
               temp << source->get_name() << ":" << wart->get_table_name();
               uri.insert("command", "DataQuery");
               uri.insert("uri", temp.str());
               uri.insert("mode", query_mode);
               uri.insert("p1", query_p1);
               uri.insert("p2", query_p2);
               uri.insert("format", "json");
               if(table_def_sig >= 0)
               {
                  temp.str("");
                  temp << table_def_sig;
                  uri.insert("headsig", temp.str());
               }
               http_request.bind(new http_request_type(this, uri));
               http_request->set_authentication(
                  source->get_user_name(), source->get_password());
               source->get_connection()->add_request(http_request);
            }
         } // poll
         

         bool Cursor::on_failure(http_request_type *request)
         {
            on_failure(SinkBase::sink_failure_connection_failed);
            return false;
         } // on_failure


         bool Cursor::on_response_complete(http_request_type *request)
         {
            int failure(-1);
            bool more_data(false);
            try
            {
               if(request->get_response_code() == 200)
               {
                  // we now need to parse the response as a JSON structure
                  Csi::IByteQueueStream input(&request->get_receive_buff());
                  Csi::Json::Object response;
                  
                  response.parse(input);

                  // we need to determine whether the header signature has changed since our last
                  // query.
                  Csi::Json::ObjectHandle json_head(response["head"]);
                  Csi::Json::NumberHandle json_signature((*json_head)["signature"]);
                  Csi::SharedPtr<Broker::ValueFactory> value_factory(source->get_manager()->get_value_factory());
                  Csi::Json::Object::iterator more_data_it(response.find("more"));

                  if(more_data_it != response.end())
                  {
                     Csi::Json::BooleanHandle more_data_val(more_data_it->second);
                     more_data = more_data_val->get_value();
                  }
                  if(json_signature->get_value_int4() != table_def_sig)
                  {
                     // if we have already processed a response for this cursor, this will be
                     // considered an error since the table definitions have changed.  If we
                     // haven't, this represents an opportunity to parse the header and to report
                     // that the request is ready
                     if(record_desc == 0)
                     {
                        record_handle record;
                        record_desc = parse_csijson_header(response);
                        record.bind(new Broker::Record(record_desc, *value_factory));
                        cache.push_back(record);
                        for(requests_type::iterator ri = requests.begin(); ri != requests.end(); ++ri)
                        {
                           request_handle &request(*ri);
                           Csi::PolySharedPtr<WartBase, TableFieldUri> uri(request->get_wart());
                           request->set_value_indices(*record, uri->get_column_name());
                           request->set_state(source, Request::state_started);
                           request->get_sink()->on_sink_ready(source->get_manager(), request, record);
                           if(!Cursor::is_valid_instance(this))
                              return false;
                        }
                        table_def_sig = json_signature->get_value_int4();
                     }
                     else
                        throw SinkBase::sink_failure_table_deleted;
                  }
                  
                  // we can now parse the records that were returned in the response
                  Csi::Json::ArrayHandle json_data(response["data"]);
                  cache_type records;
                  
                  for(Csi::Json::Array::iterator jri = json_data->begin(); jri != json_data->end(); ++jri)
                  {
                     // we need to determine whether this record should be processed.  We will
                     // ignore it if its record number and time stamp are the same as the last one
                     // collected
                     Csi::Json::ObjectHandle json_record(*jri);
                     Csi::Json::StringHandle json_time((*json_record)["time"]);
                     Csi::Json::NumberHandle json_record_no((*json_record)["no"]);
                     uint4 record_no(json_record_no->get_value_uint4());
                     Csi::LgrDate time(Csi::LgrDate::fromStr(json_time->get_value().c_str()));

                     if(record_no != last_record_no || time != last_time)
                     {
                        // we need to either create a new record or pull one off of the cache
                        record_handle record;
                        
                        if(!cache.empty())
                        {
                           record = cache.front();
                           cache.pop_front();
                        }
                        else
                           record.bind(new Broker::Record(record_desc, *value_factory));
                        
                        // we can now process the json record data
                        Csi::Json::ArrayHandle json_vals((*json_record)["vals"]);
                        Broker::Record::iterator vi(record->begin());
                        Csi::Json::Array::iterator jvi(json_vals->begin());
                        
                        record->set_stamp(time);
                        record->set_record_no(record_no);
                        while(vi != record->end() && jvi != json_vals->end())
                        {
                           Broker::Record::value_type &value(*vi);
                           value->read_json(jvi->get_rep());
                           ++vi;
                           ++jvi;
                        }
                        if(vi != record->end() || jvi != json_vals->end())
                           throw SinkBase::sink_failure_invalid_column_name;
                        records.push_back(record);
                        last_record_no = record_no;
                        last_time = time;
                     }
                  }
                  
                  // we can now report the records that have been received to the request clients
                  if(!more_data)
                  {
                     for(requests_type::iterator ri = requests.begin(); ri != requests.end(); ++ri)
                     {
                        request_handle &request(*ri);
                        request->set_expect_more_data(false);
                     }
                  }
                  if(!records.empty())
                  {
                     std::copy(records.begin(), records.end(), std::back_inserter(cache));
                     SinkBase::report_sink_records(
                        source->get_manager(), requests, records);
                  } 
               }
               else
               {
                  switch(request->get_response_code())
                  {
                  case 401:
                     failure = SinkBase::sink_failure_invalid_logon;
                     break;

                  default:
                     failure = SinkBase::sink_failure_unknown;
                     break;
                  }
               }
            }
            catch(SinkBase::sink_failure_type code)
            { failure = code; }
            catch(std::exception &)
            { failure = SinkBase::sink_failure_unknown; }

            // we will need to report any failure that occurred
            if(Cursor::is_valid_instance(this))
            {
               // if the datalogger reported that there is more data that will satisfy the
               // conditions, we will immediately launch another query.
               http_request.clear();
               if(failure >= SinkBase::sink_failure_unknown)
                  on_failure(static_cast<SinkBase::sink_failure_type>(failure));
               else if(more_data)
                  poll();
            }
            return true;
         } // on_response_complete


         void Cursor::on_failure(SinkBase::sink_failure_type failure)
         {
            HttpSource *source(this->source);
            requests_type requests(this->requests);

            this->requests.clear();
            while(!requests.empty())
            {
               request_handle request(requests.front());
               requests.pop_front();
               request->set_wart(0);
               if(SinkBase::is_valid_instance(request->get_sink()))
                  request->get_sink()->on_sink_failure(source->get_manager(), request, failure);
            }
            source->remove_cursor(this);
         } // on_failure


         void Cursor::on_request_started(Csi::Json::Object &message)
         {
            Csi::Json::ObjectHandle json_head(message["head"]);
            record_handle record;
            Csi::SharedPtr<Broker::ValueFactory> value_factory(source->get_manager()->get_value_factory());
            
            table_def_sig = static_cast<int4>(json_head->get_property_number("signature"));
            record_desc = parse_csijson_header(message);
            record.bind(new Broker::Record(record_desc, *value_factory));
            cache.push_back(record);
            for(requests_type::iterator ri = requests.begin(); ri != requests.end(); ++ri)
            {
               request_handle &request(*ri);
               Csi::PolySharedPtr<WartBase, TableFieldUri> uri(request->get_wart());
               request->set_value_indices(*record, uri->get_column_name());
               request->set_state(source, Request::state_started);
               request->get_sink()->on_sink_ready(source->get_manager(), request, record);
            }
         } // on_request_started


         void Cursor::on_request_records(Csi::Json::Object &message)
         {
            int failure(-1);
            try
            {
               Csi::Json::ObjectHandle json_records(message["records"]);
               Csi::Json::ObjectHandle json_head(json_records->get_property("head"));
               int4 reported_sig(static_cast<int4>(json_head->get_property_number("signature")));
               Csi::SharedPtr<Broker::ValueFactory> value_factory(source->get_manager()->get_value_factory());
               
               if(reported_sig == table_def_sig)
               {
                  Csi::Json::ArrayHandle json_data(json_records->get_property("data"));
                  cache_type records;
                  bool more_data(false);

                  if(json_records->has_property("more"))
                     json_records->get_property_bool("more");
                  for(Csi::Json::Array::iterator jri = json_data->begin(); jri != json_data->end(); ++jri)
                  {
                     // we need to initialise a record to report.
                     Csi::Json::ObjectHandle json_record(*jri);
                     record_handle record;
                     if(!cache.empty())
                     {
                        record = cache.front();
                        cache.pop_front();
                     }
                     else
                        record.bind(new Broker::Record(record_desc, *value_factory));
                     record->set_stamp(json_record->get_property_date("time"));
                     record->set_record_no(static_cast<uint4>(json_record->get_property_number("no")));
                     last_record_no = record->get_record_no();
                     last_time = record->get_stamp();
                     
                     // we can now process the json record data.
                     Csi::Json::ArrayHandle json_vals(json_record->get_property("vals"));
                     Broker::Record::iterator vi(record->begin());
                     Csi::Json::Array::iterator jvi(json_vals->begin());
                     while(vi != record->end() && jvi != json_vals->end())
                     {
                        Broker::Record::value_type &value(*vi);
                        value->read_json(jvi->get_rep());
                        ++vi;
                        ++jvi;
                     }
                     if(vi != record->end() || jvi != json_vals->end())
                        throw SinkBase::sink_failure_invalid_column_name;
                     records.push_back(record);
                  }

                  // we need to set the expect more state of the requests.
                  for(requests_type::iterator ri = requests.begin(); !more_data && ri != requests.end(); ++ri)
                     (*ri)->set_expect_more_data(false);

                  // finally, we need to report the records to the client
                  if(!records.empty())
                  {
                     std::copy(records.begin(), records.end(), std::back_inserter(cache));
                     SinkBase::report_sink_records(source->get_manager(), requests, records);
                  }
               }
            }
            catch(SinkBase::sink_failure_type code)
            { failure = code; }
            catch(std::exception &)
            { failure = SinkBase::sink_failure_unknown; }

            if(failure >= SinkBase::sink_failure_unknown && Cursor::is_valid_instance(this))
               on_failure(static_cast<SinkBase::sink_failure_type>(failure));
         } // on_request_records


         void Cursor::on_request_failed(Csi::Json::Object &message)
         {
            SinkBase::sink_failure_type sink_failure(SinkBase::sink_failure_unknown);
            int4 json_failure(static_cast<int4>(message.get_property_number("failure")));
            switch(json_failure)
            {
            case 1:
               sink_failure = SinkBase::sink_failure_invalid_source;
               break;

            case 2:
               sink_failure = SinkBase::sink_failure_connection_failed;
               break;

            case 3:
               sink_failure = SinkBase::sink_failure_invalid_logon;
               break;

            case 4:
               sink_failure = SinkBase::sink_failure_invalid_station_name;
               break;

            case 5:
               sink_failure = SinkBase::sink_failure_invalid_table_name;
               break;

            case 6:
               sink_failure = SinkBase::sink_failure_server_security;
               break;

            case 7:
               sink_failure = SinkBase::sink_failure_invalid_start_option;
               break;

            case 8:
               sink_failure = SinkBase::sink_failure_invalid_order_option;
               break;

            case 9:
               sink_failure = SinkBase::sink_failure_table_deleted;
               break;

            case 10:
               sink_failure = SinkBase::sink_failure_station_shut_down;
               break;

            case 11:
               sink_failure = SinkBase::sink_failure_unsupported;
               break;

            case 12:
               sink_failure = SinkBase::sink_failure_invalid_column_name;
               break;

            case 13:
               sink_failure = SinkBase::sink_failure_invalid_array_address;
               break;
            }
            on_failure(sink_failure);
         } // on_request_failed
      };
   };
};


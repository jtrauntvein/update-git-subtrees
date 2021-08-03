/* Csi.HttpClient.h

   Copyright (C) 2010, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 07 October 2010
   Last Change: Thursday 10 September 2020
   Last Commit: $Date: 2020-11-16 17:23:52 -0600 (Mon, 16 Nov 2020) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Csi_HttpClient_h
#define Csi_HttpClient_h

#include "Csi.Uri.h"
#include "Csi.SocketTcpSock.h"
#include "OneShot.h"
#include "Csi.InstanceValidator.h"
#include "Csi.LgrDate.h"
#include "Csi.StrAscStream.h"
#include "Csi.Utils.h"
#include "Csi.LogByte.h"
#include <stdlib.h>


namespace Csi
{
   namespace HttpClient
   {
      // @group: class forward declarations
      class Connection;
      class Request;
      // @endgroup:


      /**
       * Defines an interface that must be implemented by an application in order to use the Request
       * component.
       */
      class RequestClient: public Csi::InstanceValidator
      {
      public:
         /**
          * Destructor
          */
         virtual ~RequestClient()
         { }

         /**
          * Called when the HTTP request has failed.  The request parameter can be queried to
          * determine the nature of the failure.
          *
          * @param request Specifies the request that has failed.
          *
          * @return Returns true to indicate whether the connection should continue with subsequent
          * requests.
          */
         virtual bool on_failure(Request *request) = 0;

         /**
          * Called when the connection for a request has been made to the server.
          *
          * @param request Specifies the request object for which the connection is being made.
          */
         virtual void on_connected(Request *request)
         { }

         /**
          * Called when the request header has been sent and the request body is ready to be sent.
          *
          * @param request Specifies the affected request.
          */
         virtual void on_header_sent(Request *request)
         { }

         /**
          * Called when the header portion of the HTTP response has been received for the specified
          * request.
          *
          * @param request Specifies the request for which the response header has been received.
          */
         virtual void on_response_header(Request *request)
         { }

         /**
          * Called when some (but not necessarily all) data has been received in response to the
          * specified request.
          *
          * @param request Specifies the request for which data has been received.
          */
         virtual void on_response_data(Request *request)
         { }

         /**
          * Called when the entire response, including the data, has been received for the specified
          * request.
          *
          * @param request Specifies the request for which the response has been received.
          */
         virtual bool on_response_complete(Request *request) = 0;

         /**
          * Called when data has been transmitted by the connection for this request.
          *
          * @param sender Specifies the request that is sending the event.
          */
         virtual void on_request_data_written(Request *sender)
         { }
      };


      /**
       * Defines a base class that implements authorisation for the request.
       */
      class AuthorisationBase
      {
      public:
         /**
          * Destructor
          */
         virtual ~AuthorisationBase()
         { }

         /**
          * Writes the type of authorisation and parameters to the specified stream.
          *
          * @param request Specifies the request for which authorisation is being written.
          *
          * @param out Specifies the stream to which the authorisation will be written.
          */
         virtual void write_authorisation(Request *request, std::ostream &out) = 0;
      };


      /**
       * Defines an authorisation module that performs basic authorisation.
       */
      class AuthorisationBasic: public AuthorisationBase
      {
      private:
         /**
          * Specifies the user name.
          */
         StrUni const user_name;

         /**
          * Specifies the password.
          */
         StrUni const password;

      public:
         /**
          * @param user_name_ Specifies the user name that will be used.
          *
          * @param password_ Specifies the password that will be used.
          */
         AuthorisationBasic(StrUni const &user_name_, StrUni const &password_):
            user_name(user_name_),
            password(password_)
         { }

         /**
          * Overloads the base class to write the authorisation part of the header.
          */
         virtual void write_authorisation(Request *request, std::ostream &out);
      };


      /**
       * Defines an authorisation object used for the KDAPI service.
       */
      class AuthorisationKdapi: public AuthorisationBase
      {
      private:
         /**
          * Specifies the unique device ID
          */
         StrAsc const device_id;

         /**
          * Specifies the Konect ID.
          */
         StrAsc const konect_id;

         /**
          * Specifies the konect secret (specific to a unique device and account).
          */
         StrAsc const konect_secret;

         /**
          * Specifies the message type string.
          */
         StrAsc const message_type;

         /**
          * Specifies the message string.
          */
         StrAsc const &message;

         /**
          * Specifies the nonce.
          */
         StrAsc nonce;

         /**
          * Specifies the time stamp.
          */
         StrAsc timestamp;

         /**
          * Specifies the signature of the authorisation header formatted as base64.
          */
         StrAsc signature;

      public:
         /**
          * @param device_id_ Specifies the unique device id (usually SN<serial_no>-<model>)
          *
          * @param konect_id_ Specifies the user's konect ID for the device.
          *
          * @param konect_secret_ Specifies the konect secret.
          *
          * @param message_type_ Specifies the kdapi message type.
          *
          * @param message_ Specifies the kdapi message string.
          */
         AuthorisationKdapi(
            StrAsc const &device_id_,
            StrAsc const &message_type_,
            StrAsc const &message_,
            StrAsc const &konect_id_ = "",
            StrAsc const &konect_secret_ = "");

         /**
          * Overloads the base class version to generate the authorisation header for the request.
          */
         virtual void write_authorisation(Request *request, std::ostream &out);
      };

      
      /**
       * Defines an authorisation model that uses a bearer token.
       */
      class AuthorisationBearer: public AuthorisationBase
      {
      private:
         /**
          * Specifies the token string used for authorisation.
          */
         StrAsc const token;

      public:
         /**
          * Constructor
          *
          * @param token_ Specifies the authorisation token.
          */
         AuthorisationBearer(StrAsc const &token_):
            token(token_)
         { }

         /**
          * Overloads the base class version to write the authorisation line.
          */
         virtual void write_authorisation(Request *request, std::ostream &out);
      };
      

      /**
       * Defines an object that maintains the state of an HTTP request.  This object will store the
       * fields in the request header and will also store fields received in the response header.
       * It will also provide buffers for the application to send or receive request data.
       */
      class Request
      {
      public:
         /**
          * Construct a request.
          *
          * @param client Specifies the application object that will be notified regarding request
          * status.
          *
          * @param uri Specifies the URI for the request.
          *
          * @param method_ Specifies the HTTP method that should be used by the request.
          *
          * @param send_complete_ Set to true if the request does not need any body.
          *
          * @param authorisation_ Specifies the authorisation that will be used.
          */
         typedef RequestClient client_type;
         enum method_type
         {
            method_get,
            method_post,
            method_put
         };
         typedef SharedPtr<AuthorisationBase> authorisation_handle;
         Request(
            client_type *client,
            Uri const &uri,
            method_type method_ = method_get,
            bool send_complete_ = true,
            authorisation_handle authorisation_ = 0);

         /**
          * Destructor
          */
         virtual ~Request();

         /**
          * @return Returns the URI used for the request.
          */
         Uri const &get_uri() const
         { return uri; }
         
         /**
          * @return Returns the time out for the request in units of milliseconds.
          */
         uint4 get_response_timeout() const
         { return response_timeout; }

         /**
          * @param timeout Specifies the time out for the request in units of milliseconds.
          */
         Request &set_response_timeout(uint4 timeout)
         {
            response_timeout = timeout;
            return *this;
         }

         /**
          * @return Returns the code that identifies the HTTP method that will be used for this
          * request.
          */
         method_type get_method() const
         {
            return method;
         }

         /**
          * @param method_ Specifies a code that identifies the HTTP method that will be used for
          * this request.
          */
         Request &set_method(method_type method_)
         {
            method = method_;
            return *this;
         }

         /**
          * Sets up the request to use basic authentication with the specified user name and password.
          */
         Request &set_authentication(StrAsc const &user_name_, StrAsc const &password_)
         {
            authorisation.bind(new AuthorisationBasic(user_name_, password_));
            return *this;
         }

         /**
          * @param authorisation_ Specifies the authorisation that will be used.
          */
         Request &set_authorisation(authorisation_handle authorisation_)
         {
            authorisation = authorisation_;
            return *this;
         }

         /**
          * @return Returns a reference to the authorisation module.
          */
         authorisation_handle &get_authorisation()
         { return authorisation; }

         ////////////////////////////////////////////////////////////
         // add_header_auth_param
         ////////////////////////////////////////////////////////////
         Request &add_header_auth_param(StrAsc const &key, StrAsc const &value);

         /**
          * Formats the request parameters to the specified stream.
          *
          * @param out Specifies the stream to which the request will be formatted.
          */
         void format_request_header(std::ostream &out);

         /**
          * @return Returns true if there is no more data to be sent for this request.
          */
         bool get_send_complete() const
         { return send_complete; }

         /**
          * Adds the specified data to the request send buffer.
          *
          * @param buff Specifies the start of the data to add.
          *
          * @param buff_len Specifies the number of bytes to add.
          *
          * @param send_complete_ Set to true if this is the last chunk of data to send.
          */
         void add_bytes(void const *buff, size_t buff_len, bool send_complete_);

         /**
          * @return Returns a reference to the transmit buffer for this request.
          */
         ByteQueue &get_send_buff()
         { return send_buff; }

         /**
          * @return Returns the number of bytes pending in the send buff plus the number of bytes
          * pending for the socket buffer.
          */
         size_t get_send_buff_pending();

         /**
          * @return Returns a reference to the receive buffer for this request.
          */
         ByteQueue &get_receive_buff()
         { return receive_buff; }
         
         /**
          * @return Returns a pointer to the client object for this request.
          */
         client_type *get_client()
         { return client; }

         /**
          * Attempts to parse a header response string that has been received from the server.
          *
          * @param header Specifies the response string that has been received.
          *
          * @return Returns true if the header was successfully parsed.
          */
         bool parse_response(StrAsc const &header);

         /**
          * @param code Specifies the HTTp response code for this request.
          */
         void set_response_code(int4 code)
         { response_code = code; }

         /**
          * @return Returns the HTTP response code that was returned in the response header.
          */
         int4 get_response_code() const
         { return response_code; }

         /**
          * @return Returns the description string that accompanied the response code.
          */
         StrAsc const &get_response_description() const
         { return response_description; }

         /**
          * @param desc Specifies the description string for the response code.
          */
         void set_response_description(StrAsc const &desc)
         { response_description = desc; }

         /**
          * @return Returns the content type value to be sent in the request.
          */
         StrAsc const &get_content_type() const
         { return content_type; }

         /**
          * @param value Specifies the value of content-type that will be sent with the request.
          */
         void set_content_type(StrAsc const &value)
         { content_type = value; }

         /**
          * @return Returns true if the request is using chunked encoding.
          */
         bool get_uses_chunked() const
         { return uses_chunked; }

         /**
          * @return Returns the total number of bytes of content that are sent with the request.
          */
         int8 get_content_len() const
         { return content_len; }

         /**
          * @param value Sets the content length that will be encoded when the request is sent.
          */
         Request &set_content_len(int8 value)
         {
            content_len = value;
            return *this;
         }

         /**
          * @return Returns true if all of the response data has been received.
          */
         bool get_receive_complete() const
         { return receive_complete; }

         /**
          * @param connection_ Specifies the connection with which this request will be associated.
          */
         void set_connection(Connection *connection_)
         { connection = connection_; }

         /**
          * @return Returns the content disposition response header value.
          */
         StrAsc const &get_content_disposition() const
         { return content_disposition; }

         /**
          * @return Returns the content encoding response header value.
          */
         StrAsc const &get_content_encoding() const
         { return content_encoding; }

         /**
          * @return Returns the last modified time reported in the request header before the
          * response was received and reports the last modified value in the response after the
          * response header was received.
          */
         LgrDate const &get_last_modified() const
         { return last_modified; }

         /**
          * @param last_modified_ Specifies the last modified time that will be reported in the
          * request header
          */
         Request &set_last_modified(LgrDate const &last_modified_)
         {
            last_modified = last_modified_;
            return *this;
         }

         /**
          * @return Returns the location field that was returned in the response header.  This value
          * is generally only returned on a redirect.
          */
         StrAsc const &get_location() const
         { return location; }

         /**
          * @return Returns true if the "Connection: close" field was reported in the response
          * header.
          */
         bool get_will_close() const
         { return will_close; }

         /**
          * @param value Specifies the value for the Upgrade header field.
          */
         Request &set_upgrade(StrAsc const &value);

         /**
          * @return Returns the value fo the Upgrade header field.
          */
         StrAsc const &get_upgrade() const
         { return upgrade; }

         /**
          * @return Returns the websocket key.
          */
         StrAsc const &get_websock_key() const
         { return websock_key; }

         /**
          * @return Returns the websocket subprotocol.
          */
         StrAsc const &get_websock_protocol() const
         { return websock_protocol; }

         /**
          * @param value Specifies the set of websocket protocols as a comma
          * separated list.
          */
         Request &set_websock_protocol(StrAsc const &value)
         {
            websock_protocol = value;
            return *this;
         }

         /**
          * @return Returns the websock accept key that was returned in the
          * upgrade header.
          */
         StrAsc const &get_websock_accept() const
         { return websock_accept; }

      private:
         /**
          * Specifies the application object that will receive notifications regarding the progress
          * of this request.
          */
         client_type *client;

         /**
          * Specifies the connection used by this request.
          */
         Connection *connection;

         /**
          * Specifies the URI used in the response header.
          */
         Uri uri;

         /**
          * Specifies the authorisation that will be used.
          */
         authorisation_handle authorisation;

         /**
          * Set to true if the entire request has been transmitted.
          */
         bool send_complete;


         /**
          * Specifeis the maximum interval in milliseconds that we should eait for a response from
          * the server.
          */
         uint4 response_timeout;

         /**
          * Specifies the HTTP method that should be used.
          */
         method_type method;

         /**
          * Specifies the buffer that will be used for data to be transmitted with the request.
          */
         ByteQueue send_buff;

         /**
          * Specifies the buffer that will be used for data that has been received from the
          * response.
          */
         ByteQueue receive_buff;

         /**
          * Specifies the HTTP version.
          */
         StrAsc http_version;
         
         /**
          * Specifies the response code that was received in the HTTP response.
          */
         int4 response_code;

         /**
          * Specifies the description of the response code that was received in the HTTP response.
          */
         StrAsc response_description;

         /**
          * Specifies the content type field of the request header and also reports the content type
          * field of the response header.
          */
         StrAsc content_type;

         /**
          * Specifies the character set that was reported in the response header.
          */
         StrAsc char_set;

         /**
          * Specifies the number of bytes received of the response payload.
          */
         int8 content_len;

         /**
          * Set to true if the entire response has been received.
          */
         bool receive_complete;

         /**
          * Set to true if the content encoding of the response is chunked.
          */
         bool uses_chunked;

         /**
          * Specifies the content disposition field reported in the response header.
          */
         StrAsc content_disposition;

         /**
          * Specifies the content encoding field reported in the response header.
          */
         StrAsc content_encoding;

         /**
          * Specifies the If-Modified-Since field of the request header and reports the last
          * modified field of the response header.
          */
         LgrDate last_modified;

         /**
          * Reports the location field of the response header.
          */
         StrAsc location;

         /**
          * Set to true if the response header had a Connection: close field.
          */
         bool will_close;

         /**
          * Specifies the value for the Upgrade header field.
          */
         StrAsc upgrade;

         /**
          * Specifies the value of the websocket key.
          */
         StrAsc websock_key;

         /**
          * Specifies the websocket protocol needed by the application.
          */
         StrAsc websock_protocol;

         /**
          * Specifies the value of the websock accept signature that was
          * returned in the header.
          */
         StrAsc websock_accept;
      };


      /**
       * Defines a stream buffer that can be used to read or write data received by Request
       * component.
       */
      class RequestBuff: public std::streambuf
      {
      private:
         /**
          * Specifies the HTTP client component.
          */
         Request *request;

         /**
          * Specifies a buffer used for receiving data.
          */
         char rx_buff[512];

         /**
          * Specifies the amount of data in the rx queue.
          */
         size_t rx_buff_len;
         
      public:
         /**
          * Constructor
          *
          * @param request_ Specifies the client that is read from or written to by this buffer.
          */
         RequestBuff(Request *request_):
            request(request_),
            rx_buff_len(0)
         { setg(rx_buff, rx_buff, rx_buff); }

         /**
          * Overloads the base class method to write the specified data to the client.
          */
         virtual int_type overflow(int_type ch)
         {
            request->add_bytes(&ch, 1, false);
            return ch;
         }

         /**
          * Overloads the base class method to write the specified data to the client.
          */
         virtual std::streamsize xsputn(char const *buff, std::streamsize buff_len)
         {
            request->add_bytes(buff, (size_t)buff_len, false);
            return buff_len;
         }

         /**
          * Overloads the base class version to read from the http client read buffer.
          */
         virtual int_type underflow()
         {
            byte rtn(EOF);
            ByteQueue &request_buff(request->get_receive_buff());
            if(request_buff.size() > 0)
            {
               rx_buff_len = request_buff.pop(rx_buff, sizeof(rx_buff));
               setg(rx_buff, rx_buff, rx_buff + rx_buff_len);
               rtn = *rx_buff;
            }
            return rtn;
         }
      };


      /**
       * Defines a stream that writes to the payload of a given HTTP request.
       */
      class ORequestStream: public std::ostream
      {
      protected:
         /**
          * Specifies the stream buffer.
          */
         RequestBuff buffer;

      public:
         /**
          * Constructor
          *
          * @param request Specifies the request on which this stream will operate.
          */
         ORequestStream(Request *request):
            buffer(request),
            std::ostream(&buffer)
         { }
      };


      /**
       * Defines a stream that reads from the payload of a given HTTP request.
       */
      class IRequestStream: public std::istream
      {
      protected:
         /**
          * Specifies the buffer used by this stream.
          */
         RequestBuff buffer;

      public:
         /**
          * Constructor
          *
          * @param request Specifies the request on which this stream will operate.
          */
         IRequestStream(Request *request):
            buffer(request),
            std::istream(&buffer)
         { }
      };
      

      /**
       * Defines an object that represents an "upgrade" to the connection.
       * When this type of object is installed in the connection, the
       * connection will divert received data and connection related events to
       * this upgrade.  It will also cancel any watch dog timer for the
       * connection lifetime.
       */
      class Upgrade
      {
      public:
         /**
          * Called when data has been received by the connection.
          *
          * @param sender Specifies the connection reporting this event.
          */
         virtual void on_read(Connection *sender) = 0;

         /**
          * Called when the connection has encountered a socket error.
          *
          * @param sender Specifies the connection reporting this event.
          *
          * @param error_code Specifies the socket error code that was
          * reported.
          */
         virtual void on_error(Connection *sender, int error_code) = 0;
      };


      /**
       * Defines a base class for an object that monitors the low level I/O for the HTTP connection
       * object.
       */
      class ConnectionWatcher
      {
      public:
         /**
          * Called when there is a comment to log.
          *
          * @param sender Specifies the connection that is responsible for this event.
          *
          * @param comment Specifies the text of the comment.
          */
         virtual void on_log_comment(Connection *sender, StrAsc const &comment)
         { }

         /**
          * Called when there is data that has been sent or received.
          *
          * @param sender Specifies the connection that is responsible for this event.
          *
          * @param buff Specifies the data that is being sent or received.
          *
          * @param buff_len Specifies the number of bytes of data that is being sent or received.
          *
          * @param received Set to true if the data has been received or false if the data has been
          * sent.
          */
         virtual void on_data(Connection *sender, void const *buff, size_t buff_len, bool received)
         { }
      };


      /**
       * Defines a connection watcher object that streams the data and comments to a baled low level
       * log file.
       */
      class ConnectionWatcherLog: public ConnectionWatcher
      {
      private:
         /**
          * Specifies the object that will bale the log file.
          */
         SharedPtr<LogByte> baler;
         
      public:
         /**
          * Constructor
          *
          * @param path Specifies the path for the low level log file.
          *
          * @param file_name Specifies the name pattern used for the low level file.
          *
          * @param bale_interval Specifies the time based baling interval in units of milliseconds
          * for the low level file.
          *
          * @param timer Specifies the one shot timer that will be used for time based baling.
          */
         ConnectionWatcherLog(
            StrAsc const &path, StrAsc const &file_name, int8 bale_interval, SharedPtr<OneShot> timer = 0)
         {
            baler.bind(new LogByte(path.c_str(), file_name.c_str()));
            baler->set_time_based_baling(true, bale_interval, timer);
            baler->setEnable(true);
         }

         /**
          * Destructor
          */
         virtual ~ConnectionWatcherLog()
         { baler.clear(); }

         /**
          * Overloads the base class version to write the comment as a break message.
          */
         virtual void on_log_comment(Connection *sender, StrAsc const &comment)
         {
            baler->force_break(comment.c_str());
         }

         /**
          * Overloads the base class version to write the i/o data to the log.
          */
         virtual void on_data(Connection *sender, void const *buff, size_t buff_len, bool received)
         {
            baler->wr(buff, (uint4)buff_len, received);
         }
      };


      /**
       * Represents the state of a connection to a web server.  This class will defined the main
       * object that an application will use to access web services.
       */
      class Connection:
         public Csi::SocketTcpSock,
         public OneShotClient
#ifdef _WIN32
         , public EventReceiver
#endif
      {
      public:
         /**
          * @param timer_ Specifies the one shot timer used by the connection.  If set to null (the
          * default), this connection will allocate its own timer.
          */
         typedef SharedPtr<OneShot> timer_handle;
         Connection(timer_handle timer_ = 0);

         /**
          * Destructor
          */
         virtual ~Connection();

         /**
          * Adds a request to be sent through this connection.
          *
          * @param request Specifeis the request that is to be sent.
          */
         typedef Request request_type;
         typedef SharedPtr<request_type> request_handle;
         void add_request(request_handle &request);

         /**
          * Removes the specified request from the queue managed by this connection.
          */
         void remove_request(request_type *request);

         /**
          * Called when a connection has been made to an HTTP server.
          *
          * @param connected_address Specifies the address of the server.
          */
         virtual void on_connected(SocketAddress const &connected_address);

         /**
          * Overloads the base class version to handle the case where the TLS connection has been
          * made. 
          */
         virtual void on_tls_client_ready();
         
         /**
          * Overloads the base class version to read data from the socket.
          */
         virtual void on_read();

         /**
          * Overloads the base class version to handle a connection failure.
          */
         virtual void on_socket_error(int error_code);

         /**
          * Overloads to handle connection and request timers.
          */
         virtual void onOneShotFired(uint4 id);

         /**
          * @return Returns the interval that this connection will use to wait for new requests
          * before closing its current connection.
          */
         uint4 get_wait_interval() const
         { return wait_interval; }

         /**
          * @param interval Specifis the interval, in milliseconds, for which the connection will
          * wait with an open connection for new requests to be added before that connection is shut
          * down.
          */
         void set_wait_interval(uint4 interval);
         
         /**
          * Transmits data for the specified request.
          */
         void send_request_data(Request *request);

         /**
          * @return Returns the number of requests that are pending.
          */
         typedef std::list<request_handle> requests_type;
         requests_type::size_type get_request_count() const
         { return requests.size(); }

         /**
          * @return Returns the current state of this connection.
          */
         enum state_type
         {
            state_idle,
            state_connecting,
            state_sending_request_body,
            state_reading_response_header,
            state_reading_response_body,
            state_reading_response_chunk_len,
            state_reading_response_chunk,
            state_waiting_for_next
         };
         state_type get_state() const
         { return state; }

         /**
          * Overloads the base class version to handle received events.
          */
         virtual void receive(SharedPtr<Event> &ev);

         /**
          * @param watcher_ Specifies the application object that will monitor this connection.
          */
         void set_watcher(SharedPtr<ConnectionWatcher> watcher_)
         {
            watcher = watcher_;
         }

         /**
          * @return Returns a reference to the assigned application object that watches this connection.
          */
         SharedPtr<ConnectionWatcher> get_watcher()
         { return watcher; }

         /**
          * Adds the specified comment to the associated low level log (if any).
          */
         void add_log_comment(StrAsc const &comment)
         {
            if(watcher != 0)
               watcher->on_log_comment(this, comment); 
         }

         /**
          * Overloads the base class version to send data to the log.
          */
         virtual void on_low_level_read(
            void const *buff, uint4 buff_len)
         {
            if(watcher != 0)
               watcher->on_data(this, buff, buff_len, true);
            SocketTcpSock::on_low_level_read(buff, buff_len);
         }

         /**
          * Overloads the base class version to send data to the log.
          */
         virtual void on_low_level_write(
            void const *buff, uint4 buff_len)
         {
            if(watcher != 0)
               watcher->on_data(this, buff, buff_len, false);
            if(current_request != 0)
               current_request->get_client()->on_request_data_written(current_request.get_rep());
         }

         /**
          * Sets the upgrade for this connection.  If the specified object is
          * valid (not null), the connection will pass data received events to
          * it rather than any pending requests.  The presence of this upgrade
          * will also cancel any timers used to control the lifetime of the
          * connection.
          *
          * @param upgrade_ Specifies the upgrade object for this connection.
          */
         virtual void set_upgrade(Upgrade *upgrade_);

      private:
         /**
          * Starts the next request in the queue.
          */
         void start_next_request();

         /**
          * Starts the process of transmitting the current request.
          */
         void start_send_request();

         /**
          * Starts the process of waiting for a request reponse.
          */
         void start_wait_response_header();

         /**
          * Handles error conditions.
          *
          * @param affects_current Set to true if the error will affect the current request.
          */
         void do_on_error(bool affects_current = true);

         /**
          * Handles the case where the current response has been completed.
          */
         void on_response_complete();

      private:
         /**
          * Specifies the requests that are pending.
          */
         requests_type requests;

         /**
          * Specifies the state of the connection.
          */
         state_type state;

         /**
          * Specifies the current request.
          */
         request_handle current_request;

         /**
          * Specifies the object that will be used for timeouts.
          */
         timer_handle timer;

         /**
          * Identifies the timer used for waiting for a response.
          */
         uint4 response_timeout_id;

         /**
          * Identifies the timer used for waiting for the next request to be added.
          */
         uint4 wait_id;

         /**
          * Specifies the maximum amount of time that the connection will wait in an inactive state
          * before a new request is added or the connection is dropped.
          */
         uint4 wait_interval;

         /**
          * Specifies the last address of the server for the last request that was added.  This will
          * be used to determine whether the existing connection can be reused.
          */
         StrAsc last_address;

         /**
          * Specifies the TCP port for the last request connection.
          */
         uint2 last_port;

         /**
          * Keeps track of the number of bytes that have been received for the response body or the
          * current chunk.  This will be used to determine whether the response is complete.
          */
         int8 chunk_bytes_received;

         /**
          * Specifies the length of the current chunk.
          */
         int8 chunk_len;

         /**
          * Specifies a delay that should be used to start the next request.
          */
         uint4 delay_start_next;

         /**
          * Specifies the object that will monitor the data and events associated with this
          * connection.
          */
         SharedPtr<ConnectionWatcher> watcher;

         /**
          * Specifies an object that "upgrades" this connection by diverting
          * received data.
          */
         Upgrade *upgrade;
      };
   };
};


#endif

/* Csi.HttpClient.cpp

   Copyright (C) 2010, 2021 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 07 October 2010
   Last Change: Friday 05 February 2021
   Last Commit: $Date: 2021-02-05 16:37:21 -0600 (Fri, 05 Feb 2021) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.HttpClient.h"
#include "Csi.StrAscStream.h"
#include "Csi.BuffStream.h"
#include "Csi.Base64.h"
#include "Csi.TlsContext.h"
#include "Csi.Hmac.h"
#include "Csi.Utils.h"
#include "Csi.StringLoader.h"
#include "Csi.Digest.h"
#include <algorithm>


namespace Csi
{
   namespace HttpClient
   {
      void AuthorisationBasic::write_authorisation(Request *request, std::ostream &out)
      {
         if(user_name.length() > 0 || password.length() > 0)
         {
            OStrAscStream tokens;
            tokens << user_name << ":" << password;
            out << "Authorization: Basic ";
            Base64::encode(out, tokens.c_str(), tokens.length());
            out << "\r\n";
         }
      } // write_authorisation


      namespace
      {
         StrAsc const kdapi_private1("269fb2c0205345d783aa48b2c88f84c29b47751faa2d4e4c93d434ff05637f5b99a57795517841a1acf0d1f6dde2bcddf07414a363264d75b3d3cba39ed4a6a1");
         StrAsc const kdapi_private2("f43d4180cadc4f77bc34e88234b753f09cdd47729e3d45268678c94818391786c7794cdfcc8c4c84921e083db4498d4a71c6c7f92bd545c78b173431aaebfb5f");
      };
      

      AuthorisationKdapi::AuthorisationKdapi(
         StrAsc const &device_id_,
         StrAsc const &message_type_,
         StrAsc const &message_,
         StrAsc const &konect_id_,
         StrAsc const &konect_secret_):
         device_id(device_id_),
         message_type(message_type_),
         message(message_),
         konect_id(konect_id_),
         konect_secret(konect_secret_),
         nonce(make_guid())
      {
         // we need to format the current utc time as seconds since 1970 and we also need to format
         // the device secret key.
         OStrAscStream temp;
         uint2 secret_key_sig;
         StrAsc device_secret_key;
         temp.imbue(StringLoader::make_locale(0));
         temp << LgrDate::local().to_time_t();
         nonce.replace("-", "");
         timestamp = temp.str();
         temp.str("");
         temp << kdapi_private1 << "&" << device_id << "&" << kdapi_private2;
         secret_key_sig = calcSigFor(temp.c_str(), temp.length());
         temp.str("");
         temp << kdapi_private2 << "&" << secret_key_sig << "&" << device_id << "&" << kdapi_private1 << "&" << konect_secret;
         device_secret_key = temp.str();
         

         // we are now ready to calculate the signature.  To do so, we first need to format the base
         // string.
         byte digest[Sha1Digest::digest_size];
         temp.str("");
         temp << device_id << "&"
              << konect_id << "&"
              << message_type << "&"
              << message << "&"
              << timestamp << "&"
              << nonce;
         csi_hmac_sha1(temp.str(), device_secret_key, digest);
         Base64::encode(signature, digest, sizeof(digest));
      } // constructor
         

      void AuthorisationKdapi::write_authorisation(Request *request, std::ostream &out)
      {
         out << "Authorization: KDAPI "
             << "deviceid=\"" << device_id << "\"";
         if(konect_id.length() > 0)
            out << ",konectid=\"" << konect_id << "\"";
         out << ",timestamp=\"" << timestamp << "\""
             << ",nonce=\"" << nonce << "\""
             << ",signature=\"" << signature << "\"\r\n";
      } // write_authorisation


      void AuthorisationBearer::write_authorisation(Request *request, std::ostream &out)
      {
         out << "Authorization: Bearer " << token << "\r\n";
      } // write_authorisation
      
      
      Request::Request(
         client_type *client_,
         Uri const &uri_,
         method_type method_,
         bool send_complete_,
         authorisation_handle authorisation_):
         client(client_),
         uri(uri_),
         send_complete(send_complete_),
         response_timeout(300000),
         connection(0),
         method(method_),
         will_close(false),
         authorisation(authorisation_),
         content_len(-1)
      {
         if(method == method_get)
            send_complete = true;
         if(!client_type::is_valid_instance(client))
            throw std::invalid_argument("invalid request client specified");
      } // constructor


      Request::~Request()
      { }


      Request &Request::set_upgrade(StrAsc const &value)
      {
         upgrade = value;
         if(upgrade == "websocket")
         {
            // we need to generate a random websocket key
            StrBin key;
            while(key.length() < 16)
            {
               uint4 random_value(rand());
               key.append(&random_value, sizeof(random_value));
            }
            websock_key.cut(0);
            Base64::encode(websock_key, key.getContents(), key.length());
         }
         return *this;
      } // set_upgrade


      void Request::format_request_header(std::ostream &out_)
      {
         OStrAscStream out;
         // format the method
         switch(method)
         {
         case method_get:
            out << "GET ";
            break;
            
         case method_post:
            out << "POST ";
            break;
            
         case method_put:
            out << "PUT ";
            break;
         }

         // format the path
         std::list<StrAsc> path_names;
         uri.get_path_names(path_names);
         if(path_names.empty())
            out << '/';
         while(!path_names.empty())
         {
            out << '/';
            Uri::encode(out, path_names.front());
            path_names.pop_front();
         }

         // format any parameters
         if(!uri.empty())
         {
            out << "?";
            for(Uri::const_iterator pi = uri.begin(); pi != uri.end(); ++pi)
            {
               if(pi != uri.begin())
                  out << "&";
               Uri::encode(out, pi->first);
               out << "=";
               Uri::encode(out, pi->second);
            }
         }
         out << " HTTP/1.1\r\n";
         out << "HOST: " << uri.get_server_address() << "\r\n";

         // allow the authorisation module to write its authorisation
         if(authorisation != 0)
            authorisation->write_authorisation(this, out);

         // add the upgrade header
         if(upgrade.length() > 0)
         {
            out << "Connection: Upgrade\r\n"
                << "Upgrade: " << upgrade << "\r\n";
            if(upgrade == "websocket")
            {
               out << "Sec-Websocket-Version: 13\r\n";
               out << "Sec-Websocket-Key: " << websock_key << "\r\n";
               if(websock_protocol.length() > 0)
                  out << "Sec-Websocket-Protocol: " << websock_protocol << "\r\n";
            }
         }

         // we may need to ssend the content-type
         if(content_type.length() > 0)
            out << "Content-Type: " << content_type << "\r\n";
         
         // we now need to determine the content encoding.  This is going to depend upon whether the
         // body is complete
         if(send_complete)
            out << "Content-Length: " << std::dec << send_buff.size() << "\r\n";
         else if(content_len > 0)
            out << "Content-Length: " << std::dec << content_len << "\r\n";
         else
         {
            out << "Transfer-Encoding: chunked\r\n";
            uses_chunked = true;
         }

         // we need to determine whether the to send the last modified value to the server. 
         if(last_modified != 0)
         {
            out << "If-Modified-Since: ";
            last_modified.format_http(out);
            out << "\r\n";
         }

         // we are now done and will output a blank line to signal the end of the header
         out << "\r\n";
         out_ << out.str().c_str();
      } // format_request_header


      void Request::add_bytes(void const *buff, size_t buff_len, bool send_complete_)
      {
         send_complete = send_complete_;
         if(buff_len > 0)
            send_buff.push(buff, (uint4)buff_len);
         if(connection)
            connection->send_request_data(this);
      } // add_bytes

      size_t Request::get_send_buff_pending()
      {
         size_t rtn(send_buff.size());
         if(connection)
            rtn += connection->get_tx_buff_size();
         return rtn;
      } // get_send_buff_pending

      bool Request::parse_response(StrAsc const &header)
      {
         bool rtn(false);
         size_t status_line_end(header.find("\r\n"));

         response_code = 0;
         response_description.cut(0);
         content_len = -1;
         receive_complete = true;
         uses_chunked = false;
         if(status_line_end < header.length())
         {
            size_t http_version_end(header.find(" "));
            if(http_version_end < status_line_end)
            {
               size_t response_code_end(header.find(" ", http_version_end + 1));
               header.sub(http_version, 0, http_version_end);
               if(response_code_end < status_line_end)
               {
                  // the status line is the only required field in the response so we will mark
                  // success after parsing it
                  IBuffStream response_code_str(
                     header.c_str() + http_version_end, response_code_end - http_version_end);
                  rtn = true;
                  response_code_str.imbue(std::locale::classic());
                  response_code_str >> response_code;
                  header.sub(response_description, response_code_end + 1, status_line_end - response_code_end - 1);

                  // parse the content type
                  StrAsc const content_type_name("Content-Type: ");
                  size_t content_type_pos(header.find(content_type_name.c_str(), status_line_end));
                  if(content_type_pos < header.length())
                  {
                     size_t content_type_end(header.find("\r\n", content_type_pos));
                     if(content_type_end < header.length())
                     {
                        size_t param_pos(header.find(";", content_type_pos));
                        size_t content_type_start(content_type_pos + content_type_name.length());
                        if(param_pos <= content_type_end)
                        {
                           // we may need to parse the charset from the string
                           StrAsc const charset_name("charset=");
                           size_t charset_pos(header.find(charset_name.c_str(), param_pos));
                           if(charset_pos < content_type_end)
                           {
                              size_t charset_start(charset_pos + charset_name.length());
                              header.sub(char_set, charset_start, content_type_end - charset_start);
                           }
                           param_pos = content_type_end;
                        }
                        else
                           param_pos = content_type_end;
                        header.sub(content_type, content_type_start, param_pos - content_type_start);
                     }
                  }

                  // we need to determine whether the content length was specified
                  StrAsc const content_len_name("Content-Length:");
                  size_t content_len_pos(header.find(content_len_name.c_str(), status_line_end));
                  if(content_len_pos < header.length())
                  {
                     size_t content_len_end(header.find("\r\n", content_len_pos));
                     size_t content_len_start(content_len_pos + content_len_name.length() + 1);
                     IBuffStream temp(header.c_str() + content_len_start, content_len_end - content_len_start);

                     temp.imbue(std::locale::classic());
                     temp >> content_len;

                     if(content_len > 0)
                        receive_complete = false;
                  }

                  // we need to determine whether transfer encoding was specified
                  StrAsc const transfer_encoding_name("Transfer-Encoding:");
                  size_t transfer_encoding_pos(header.find(transfer_encoding_name.c_str(), status_line_end));
                  if(transfer_encoding_pos < header.length())
                  {
                     size_t chunked_pos(header.find("chunked", transfer_encoding_pos));
                     if(chunked_pos < header.length())
                     {
                        uses_chunked = true;
                        receive_complete = false;
                     }
                  }

                  // we need to determine whether content encoding was specified
                  StrAsc const content_encoding_name("Content-Encoding:");
                  size_t content_encoding_pos(
                     header.find(content_encoding_name.c_str(), status_line_end));
                  content_encoding.cut(0);
                  if(content_encoding_pos < header.length())
                  {
                     size_t content_encoding_start(content_encoding_pos + content_encoding_name.length());
                     size_t content_encoding_end(header.find("\r\n", content_encoding_start));
                     header.sub(
                        content_encoding,
                        content_encoding_start + 1,
                        content_encoding_end - content_encoding_start);
                  }

                  // we need to see if the last-modified field was specified
                  try
                  {
                     StrAsc const last_modified_name("Last-Modified:");
                     size_t last_modified_pos(header.find(last_modified_name.c_str(), status_line_end));
                     
                     last_modified = 0;
                     if(last_modified_pos < header.length())
                     {
                        size_t last_modified_start(last_modified_pos + last_modified_name.length());
                        size_t last_modified_end(header.find("\r\n", last_modified_start));
                        StrAsc temp;
                        
                        header.sub(temp, last_modified_start + 1, last_modified_end - last_modified_start);
                        last_modified = LgrDate::from_http(temp.c_str());
                     }
                  }
                  catch(std::exception &)
                  { last_modified = 0; }

                  // we need to check to see if the content disposition was specified
                  StrAsc const content_disposition_name("Content-Disposition:");
                  size_t content_disposition_pos(header.find(content_disposition_name.c_str(), status_line_end));
                  content_disposition.cut(0);
                  if(content_disposition_pos < header.length())
                  {
                     size_t content_disposition_start(content_disposition_pos + content_disposition_name.length());
                     size_t content_disposition_end(header.find("\r\n", content_disposition_start));
                     header.sub(
                        content_disposition,
                        content_disposition_start + 1,
                        content_disposition_end - content_disposition_start - 1);
                  }

                  // we need to see if the location is specified in the response header
                  StrAsc const location_name("Location:");
                  size_t location_pos(header.find(location_name.c_str(), status_line_end));
                  if(location_pos < header.length())
                  {
                     size_t location_start(location_pos + location_name.length());
                     size_t location_end(header.find("\r\n", location_start));
                     header.sub(
                        location, location_start + 1, location_end - location_start - 1);
                  }

                  // we need to see of the Connection directive is in the header
                  StrAsc const will_close_name("Connection: close");
                  size_t will_close_pos(header.find(will_close_name.c_str(), status_line_end));
                  if(will_close_pos < header.length())
                     will_close = true;
                  else
                     will_close = false;

                  // we need to see if there are upgrade related responses in the header
                  StrAsc const upgrade_name("Connection: Upgrade");
                  size_t upgrade_pos(header.find(upgrade_name.c_str(), status_line_end));
                  if(upgrade_pos < header.length())
                  {
                     // is this associated ewith web sockets?
                     StrAsc websock_upgrade_name("Upgrade: websocket");
                     size_t websock_upgrade_pos(header.find(websock_upgrade_name.c_str(), status_line_end));
                     if(websock_upgrade_pos < header.length())
                     {
                        StrAsc websock_accept_name("Sec-Websocket-Accept:");
                        size_t websock_accept_pos(header.find(websock_accept_name.c_str(), status_line_end));
                        if(websock_accept_pos < header.length())
                        {
                           size_t accept_start(websock_accept_pos + websock_accept_name.length());
                           size_t accept_end(header.find("\r\n", accept_start));
                           header.sub(websock_accept, accept_start + 1, accept_end - accept_start - 1);
                        }
                     }
                  }
               }
            }
         }
         return rtn;
      } // parse_response


      Connection::Connection(timer_handle timer_):
         state(state_idle),
         timer(timer_),
         response_timeout_id(0),
         wait_id(0),
         last_port(0),
         wait_interval(40000),
         delay_start_next(0),
         upgrade(0)
      {
         if(timer == 0)
            timer.bind(new OneShot);
      } // constructor


      Connection::~Connection()
      {
         current_request.clear();
         requests.clear();
         if(timer != 0)
         {
            if(wait_id != 0)
               timer->disarm(wait_id);
            if(response_timeout_id != 0)
               timer->disarm(response_timeout_id);
            timer.clear();
         }
      } // destructor


      void Connection::add_request(request_handle &request)
      {
         request->set_connection(this);
         requests.push_back(request);
         if(current_request == 0)
            start_next_request();
      } // add_request


      void Connection::remove_request(request_type *request)
      {
         if(current_request == request)
         {
            current_request.clear();
            close();
            state = state_idle;
            if(response_timeout_id != 0)
               timer->disarm(response_timeout_id);
            if(!requests.empty())
            {
               if(delay_start_next == 0)
                  delay_start_next = timer->arm(this, 500);
            }
         }
         else
         {
            requests_type::iterator ri = std::find_if(
               requests.begin(), requests.end(), HasSharedPtr<request_type>(request));
            if(ri != requests.end())
               requests.erase(ri);
         }
      } // remove_request


      void Connection::on_connected(SocketAddress const &connected_address)
      {
         if(state == state_connecting && current_request != 0)
         {
            // before we proceed with the current request, we will need to determine whether TLS
            // needs to be started for this request.  This can be determined by looking at the
            // request's URI
            Csi::Uri const &uri(current_request->get_uri());
            if((uri.get_protocol() == "https" || uri.get_protocol() == "wss") && !get_using_tls())
            {
               try
               {
                  if(get_tls_context() == 0)
                     set_tls_context(new Csi::TlsContext);
                  start_tls_client();
                  return;
               }
               catch(std::exception &)
               {
                  on_socket_error(0);
                  return;
               }
            }
            
            RequestClient *client(current_request->get_client());
            state = state_waiting_for_next;
            if(RequestClient::is_valid_instance(client))
               start_send_request();
            else
            {
               current_request.clear();
               start_next_request();
            }
         }
      } // on_connected


      void Connection::on_tls_client_ready()
      {
         // we will now behave as if the connection is complete.
         on_connected(SocketAddress());
      } // on_tls_client_ready
      

      void Connection::on_read()
      {
         // if the upgrade is set, we will divert notification to that.
         SocketTcpSock::on_read();
         if(upgrade != 0)
            upgrade->on_read(this);
         else
         {
            char temp_rx[1024];
            RequestClient *client(0);
            if(current_request != 0)
            {
               client = current_request->get_client();
               if(!RequestClient::is_valid_instance(client))
                  client = 0;
            }
            if(state == state_reading_response_header)
            {
               // look for the end of the header
               uint4 header_end_pos(read_buffer.find("\r\n\r\n", 4));
               if(header_end_pos < read_buffer.size())
               {
                  StrAsc header;
                  read_buffer.pop(header, header_end_pos + 4);
                  add_log_comment(header);
                  if(current_request != 0 && current_request->parse_response(header))
                  {
                     if(client)
                     {
                        if(current_request->get_receive_complete())
                           on_response_complete();
                        else
                        {
                           timer->reset(response_timeout_id);
                           client->on_response_header(current_request.get_rep());
                           chunk_bytes_received = 0;
                           if(current_request->get_uses_chunked())
                           {
                              state = state_reading_response_chunk_len;
                              if(read_buffer.size() > 0)
                                 on_read();
                           }
                           else
                           {
                              state = state_reading_response_body;
                              if(read_buffer.size() > 0)
                                 on_read();
                           }
                        }
                     }
                     else
                        do_on_error(true);
                  }
                  else
                     do_on_error(true);
               }
            }
            else if(state == state_reading_response_body)
            {
               // we will read the available data and pass it on to the client
               timer->reset(response_timeout_id);
               while(client != 0 && read_buffer.size() > 0)
               {
                  uint4 bytes_read(read_buffer.pop(temp_rx, sizeof(temp_rx)));
                  current_request->get_receive_buff().push(temp_rx, bytes_read);
                  chunk_bytes_received += bytes_read;
                  client->on_response_data(current_request.get_rep());
               }
               
               // we need to determine whether the number of bytes received matches the content length
               // sent by the server. If so, we can go on to the next request
               if(chunk_bytes_received >= current_request->get_content_len())
                  on_response_complete();
            }
            else if(state == state_reading_response_chunk_len)
            {
               // we are looking for the end of line to indicate the end of the chunk header
               uint4 chunk_header_end_pos(read_buffer.find("\r\n", 2));
               if(chunk_header_end_pos < read_buffer.size())
               {
                  StrAsc header;
                  read_buffer.pop(header, chunk_header_end_pos + 2);
                  if(header.length() > 2)
                  {
                     IBuffStream chunk_size_str(header.c_str(), header.length());
                     chunk_size_str >> std::hex >> chunk_len;
                     if(chunk_len == 0)
                        on_response_complete();
                     else
                     {
                        chunk_bytes_received = 0;
                        state = state_reading_response_chunk;
                        if(read_buffer.size() > 0)
                           on_read();
                     }
                  }
                  else if(read_buffer.size() > 0)
                     on_read();
               } 
            }
            else if(state == state_reading_response_chunk)
            {
               // read the available data
               timer->reset(response_timeout_id);
               while(client != 0 && read_buffer.size() > 0 && chunk_bytes_received < chunk_len)
               {
                  // we need to determine how many bytes need to be read in order to fill the current chunk
                  uint4 bytes_to_read(sizeof(temp_rx));
                  if(bytes_to_read + chunk_bytes_received > chunk_len)
                     bytes_to_read = static_cast<uint4>(chunk_len - chunk_bytes_received);
                  
                  // now read what we can 
                  uint4 bytes_read(read_buffer.pop(temp_rx, bytes_to_read));
                  current_request->get_receive_buff().push(temp_rx, bytes_read);
                  chunk_bytes_received += bytes_read;
                  client->on_response_data(current_request.get_rep());
               }
               
               // we need to determine if this chunk is complete
               if(chunk_bytes_received >= chunk_len)
               {
                  state = state_reading_response_chunk_len;
                  if(read_buffer.size() > 0)
                     on_read();
               }
            }
         }
      } // on_read


      void Connection::on_socket_error(int error_code)
      {
         if(upgrade)
         {
            Upgrade *report(upgrade);
            upgrade = 0;
            report->on_error(this, error_code);
         }
         do_on_error(true);
      } // on_socket_error


      void Connection::onOneShotFired(uint4 id)
      {
         if(id == wait_id)
         {
            wait_id = 0;
            if(state == state_waiting_for_next && upgrade == 0)
            {
               close();
               state = state_idle; 
            }
         }
         else if(id == response_timeout_id)
         {
            response_timeout_id = 0;
            do_on_error(true);
         }
         else if(id == delay_start_next)
         {
            delay_start_next = 0;
            start_next_request();
         }
      } // onOneShotFired


      void Connection::set_wait_interval(uint4 interval)
      {
         wait_interval = interval;
         if(state == state_waiting_for_next)
         {
            timer->disarm(wait_id);
            if(wait_interval != 0)
               wait_id = timer->arm(this, wait_interval);
         }
      } // set_wait_interval


      namespace
      {
         uint4 const event_response_complete(
            Event::registerType("Csi::HttpClient::Connection::event_response_complete"));
      };

      
      void Connection::receive(SharedPtr<Event> &ev)
      {
         if(ev->getType() == event_response_complete)
         {
            current_request.clear();
            if(state != state_idle)
            {
               state = state_waiting_for_next;
               wait_id = timer->arm(this, wait_interval);
            }
            start_next_request();
         }
#ifndef _WIN32
         else
            SocketTcpSock::receive(ev);
#endif
      } // receive


      void Connection::set_upgrade(Upgrade *upgrade_)
      {
         upgrade = upgrade_;
         if(upgrade != 0)
         {
            if(wait_id != 0)
               timer->disarm(wait_id);
            if(response_timeout_id != 0)
               timer->disarm(response_timeout_id);
            if(delay_start_next != 0)
               timer->disarm(delay_start_next);
         }
         else
         {
            close();
            start_next_request();
         }
      } // set_upgrade
      
      
      void Connection::start_next_request()
      {
         if(delay_start_next != 0)
            timer->disarm(delay_start_next);
         if(current_request == 0 && !requests.empty())
         {
            // we will get the next request off of the queue.  if the request client is not valid,
            // we will move on to the next request.
            current_request.clear();
            while(current_request == 0 && !requests.empty())
            {
               current_request = requests.front();
               requests.pop_front();
               if(!RequestClient::is_valid_instance(current_request->get_client()))
                  current_request.clear();
            }
            if(current_request != 0)
            {
               // the first thing that we need to check (if already connected), is whether the current
               // request uses the same address and port as the previous request.
               Csi::Uri const &uri(current_request->get_uri());
               if(state == state_waiting_for_next)
               {
                  if(uri.get_server_address() == last_address && uri.get_server_port() == last_port)
                     start_send_request();
                  else
                  {
                     // we will close the current connection and proceed as if we were idle
                     close();
                     state = state_idle;
                  }
               }
               
               // if we are idle, we will need to start the connection first
               if(state == state_idle)
               {
                  try
                  {
                     last_address = uri.get_server_address();
                     last_port = uri.get_server_port();
                     state = state_connecting;
                     open(last_address.c_str(), last_port);
                  }
                  catch(std::exception &)
                  { on_socket_error(0); }
               }
            }
            else if(state != state_idle)
            {
               // there are no more requests and the state indicates that we have a valid
               // connection.  We will enter the waiting state and set the timer to control this.
               timer->disarm(wait_id);
               if(wait_interval != 0 && upgrade == 0)
               {
                  state = state_waiting_for_next;
                  wait_id = timer->arm(this, wait_interval);
               }
               else
                  state = state_idle;
            }
         }
      } // start_next_request


      void Connection::start_send_request()
      {
         // sanity checking
         assert(current_request != 0);
         assert(RequestClient::is_valid_instance(current_request->get_client()));

         // we will first set up and transmit the header
         OStrAscStream header;
         RequestClient *client(current_request->get_client());

         client->on_connected(current_request.get_rep());
         current_request->format_request_header(header);
         add_log_comment(header.str());
         write(header.str().c_str(), (uint4)header.str().length());

         // if the request is complete, we can send all of the data in its buffer,  otherwise, we
         // will need to transmit the available data.
         if(current_request->get_send_complete())
         {
            ByteQueue &buff(current_request->get_send_buff());
            char temp_buff[1024];
            uint4 temp_size;
            current_request->get_client()->on_header_sent(current_request.get_rep());
            while((temp_size = buff.pop(temp_buff, sizeof(temp_buff))) != 0)
               write(temp_buff, temp_size);
            start_wait_response_header();
         }
         else
         {
            state = state_sending_request_body;
            send_request_data(current_request.get_rep());
            current_request->get_client()->on_header_sent(current_request.get_rep());
         }
      } // start_send_request


      void Connection::send_request_data(Request *request)
      {
         if(current_request == request && request != 0 && state == state_sending_request_body)
         {
            // we will accumulate enough in the buffer to make it worthwhile to send a chunk
            ByteQueue &buff(request->get_send_buff());
            char tx_buff[1024];
            uint4 tx_len;
            if(request->get_content_len() <= 0)
            {
               if(buff.size() >= 1024 || request->get_send_complete())
               {
                  // we need to send a chunk for the entire contents
                  OStrAscStream chunk_buff;
                  
                  chunk_buff << std::hex << buff.size() << "\r\n";
                  write(chunk_buff.str().c_str(), (uint4)chunk_buff.str().length());
                  while((tx_len = buff.pop(tx_buff, sizeof(tx_buff))) > 0)
                     write(tx_buff, tx_len);
                  write("\r\n", 2);
               }
            }
            else
            {
               while((tx_len = buff.pop(tx_buff, sizeof(tx_buff))) > 0)
                  write(tx_buff, tx_len);
            }

            // if the request is complete, we will send an empty chunk to trigger the server to send
            // a response
            if(request->get_send_complete())
            {
               if(request->get_content_len() <= 0)
                  write("0\r\n\r\n", 5);
               start_wait_response_header();
            }
         }
      } // send_request_data


      void Connection::start_wait_response_header()
      {
         // we need to set up the timer for waiting for the response.  We will also process any
         // bytes that may have come in from the server.
         uint4 response_timeout(current_request->get_response_timeout()); 
         state = state_reading_response_header;
         if(response_timeout < 20000)
            response_timeout = 20000;
         response_timeout_id = timer->arm(this, response_timeout);
         on_read();
      } // start_wait_response_header


      void Connection::do_on_error(bool effects_current)
      {
         requests_type requests(this->requests);
         if(state != state_waiting_for_next)
         {
            if(effects_current && current_request != 0)
               requests.push_front(current_request);
            current_request.clear();
            this->requests.clear();
         }
         close();
         timer->disarm(wait_id);
         timer->disarm(response_timeout_id);
         state = state_idle;
         while(!requests.empty() && state != state_waiting_for_next)
         {
            request_handle request(requests.front());
            RequestClient *client(request->get_client());

            requests.pop_front();
            request->set_response_code(503);
            request->set_response_description("service unavailable");
            if(RequestClient::is_valid_instance(client))
               client->on_failure(request.get_rep());
         }
      } // do_on_error


      void Connection::on_response_complete()
      {
         if(current_request != 0)
         {
            // we will only go on to the next request if the client return true.  If the client were
            // to return false, it would be an indication that the client is through with this
            // connection and may have deleted it.  Because of that, we will not do anything that
            // would access or modify this.
            RequestClient *client(current_request->get_client());

            timer->disarm(response_timeout_id);
            read_buffer.pop(read_buffer.size());
            if(RequestClient::is_valid_instance(client))
            {
               Csi::Event *event = Csi::Event::create(event_response_complete, this); 
               int response_code(current_request->get_response_code());
               event->post();
               state = state_waiting_for_next;
               if(current_request->get_will_close())
               {
                  close();
                  wait_id = 0;
                  state = state_idle;
               }
               if(response_code == 101 || (response_code >= 200 && response_code < 400))
                  client->on_response_complete(current_request.get_rep());
               else
                  client->on_failure(current_request.get_rep()); 
            }
            else
               start_next_request();
         }
      } // on_response_complete
   };
};


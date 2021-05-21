/* Csi.SmtpSender.cpp

   Copyright (C) 2012, 2021 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Thursday 27 September 2012
   Last Change: Friday 21 May 2021
   Last Commit: $Date: 2019-10-30 09:50:27 -0600 (Wed, 30 Oct 2019) $
   Last Changed by: $Author: tmecham $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.SmtpSender.h"
#include "Csi.Utils.h"
#include "Csi.Base64.h"
#include "Csi.TlsContext.h"
#include "Csi.SocketTcpStream.h"
#include "Csi.SocketException.h"
#include "Csi.SocketBase.h"
#include "Csi.ReadFileMapping.h"
#include "Csi.Digest.h"
#include "Csi.FileSystemObject.h"
#include "Csi.RegistryManager.h"
#include "LoggerNetBuild.h"
#include <iostream>


namespace Csi
{
   namespace SmtpHelpers
   {
      ////////////////////////////////////////////////////////////
      // parse_addresses
      ////////////////////////////////////////////////////////////
      void parse_addresses(addresses_type &addresses, StrUni const &s)
      {
         StrUni temp;
         bool in_quote(false);
         addresses.clear();
         for(size_t i = 0; i < s.length(); ++i)
         {
            wchar_t ch(s[i]);
            switch(ch)
            {
            case ';':
            case ' ':
            case ',':
               if(!in_quote)
               {
                  if(temp.length() > 0)
                     addresses.push_back(temp);
                  temp.cut(0);
               }
               break;

            default:
               if(ch == '\"')
                  in_quote = !in_quote;
               temp.append(ch);
               break;
            }
         }
         if(temp.length() > 0)
            addresses.push_back(temp);
      } // parse_addresses


      void format_smtp_date(std::ostream &out, LgrDate &date)
      {
         switch(date.dayOfWeek())
         {
         case 1:
            out << "Sun, ";
            break;
         case 2:
            out << "Mon, ";
            break;
         case 3:
            out << "Tue, ";
            break;
         case 4:
            out << "Wed, ";
            break;
         case 5:
            out << "Thu, ";
            break;
         case 6:
            out << "Fri, ";
            break;
         default:
            out << "Sat, ";
            break;
         }
         
         out << date.day();
         
         switch(date.month())
         {
         case 1:
            out << " Jan ";
            break;
         case 2:
            out << " Feb ";
            break;
         case 3:
            out << " Mar ";
            break;
         case 4:
            out << " Apr ";
            break;
         case 5:
            out << " May ";
            break;
         case 6:
            out << " Jun ";
            break;
         case 7:
            out << " Jul ";
            break;
         case 8:
            out << " Aug ";
            break;
         case 9:
            out << " Sep ";
            break;
         case 10:
            out << " Oct ";
            break;
         case 11:
            out << " Nov ";
            break;
         default:
            out << " Dec ";
            break;
         }
         date.format(out, "%Y %H:%M:%S GMT");
      } // format_smtp_date


      StrAsc generate_cram_digest(
         StrAsc const &challenge,
         StrAsc const &user_name,
         StrAsc const &password)
      {
         // We need to decode this base64 response to find the "Challenge"
         // EXAMPLE: 334 PDI0NjA5LjEwNDc5MTQwNDZAcG9wbWFpbC5TcGFjZS5OZXQ+
         // This decodes to a challenge of <24609.1047914046@popmail.Space.Net>
         StrBin challenge_decoded;
         byte ipad_conv[64];
         byte opad_conv[64];
         byte result1[16];
         byte result2[16];
         Csi::Md5Digest md5;
         
         Base64::decode(challenge_decoded, challenge.c_str(), challenge.length());
         memset(result1, 0, sizeof(result1));
         memset(result2, 0, sizeof(result2));

         // initialise the password that will be used.  If the password specified is longer than 64
         // characters, the MD5 checksum will be used to mangle it.
         byte use_password[64];
         memset(use_password, 0, sizeof(use_password));
         memset(ipad_conv, 0, sizeof(ipad_conv));
         memset(opad_conv, 0, sizeof(opad_conv));
         if(password.length() > 64)
         {
            md5.add(password.c_str(), password.length());
            memcpy(use_password, md5.final(), md5.digest_size);
         }
         else
            memcpy(use_password, password.c_str(), password.length());
         for(int i = 0; i < sizeof(use_password); ++i)
         {
            ipad_conv[i] = use_password[i] ^ 0x36;
            opad_conv[i] = use_password[i] ^ 0x5c;
         }

         // we will now generate the results by calculating various checksums
         StrAsc hexed_digest;
         md5.reset();
         md5.add(ipad_conv, sizeof(ipad_conv));
         md5.add(challenge_decoded.getContents(), challenge_decoded.length());
         memcpy(result1, md5.final(), md5.digest_size);
         md5.reset();
         md5.add(opad_conv, sizeof(opad_conv));
         md5.add(result1, sizeof(result1));
         memcpy(result2, md5.final(), md5.digest_size);
         hexed_digest.encodeHex(result2, sizeof(result2), false);

         // we can now generate the response
         OStrAscStream cram_response;
         StrAsc rtn;

         cram_response << user_name << " " << hexed_digest;
         Base64::encode(rtn, cram_response.c_str(), cram_response.length());
         return rtn;
      } // generate_cram_digest
   };


   namespace
   {
      uint4 const smtp_timeout(30000);
      StrUni product_name(L"Campbell Scientific Mail Agent");
      StrUni product_version(CORASTRINGS_VERSION);
      StrUni gateway_model;
      StrUni gateway_serial_no;
      StrAsc const kdapi_smtp_uri("https://kdapiemail.konectgds.com/smtp");
      

      ////////////////////////////////////////////////////////////
      // class event_complete
      ////////////////////////////////////////////////////////////
      class event_complete: public Event
      {
      public:
         ////////////////////////////////////////////////////////////
         // event_id
         ////////////////////////////////////////////////////////////
         static uint4 const event_id;

         ////////////////////////////////////////////////////////////
         // outcome
         ////////////////////////////////////////////////////////////
         typedef SmtpSenderClient::outcome_type outcome_type;
         outcome_type outcome;

         ////////////////////////////////////////////////////////////
         // cpost
         ////////////////////////////////////////////////////////////
         static void cpost(SmtpSender *sender, outcome_type outcome)
         {
            event_complete *event(new event_complete(sender, outcome));
            event->post();
         }

      private:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         event_complete(SmtpSender *sender, outcome_type outcome_):
            Event(event_id, sender),
            outcome(outcome_)
         { }
      };


      uint4 const event_complete::event_id(
         Event::registerType("Csi::SmtpSender::on_complete"));


      /**
       * Defines an HTTP client connection watcher that will redirect its output to an SmtpSender
       * client object.
       */
      class MyHttpWatcher: public HttpClient::ConnectionWatcher
      {
      private:
         /**
          * Specifies the client that will receive notifications.
          */
         SmtpSenderClient *client;

         /**
          * Specifies the sender.
          */
         SmtpSender *component;

      public:
         /**
          * Constructor
          *
          * #param component_ Specifies the sender component.
          *
          * @param client_ Specifies the client object that will receive watcher notifications.
          */
         MyHttpWatcher(SmtpSender *component_, SmtpSenderClient *client_):
            client(client_)
         { }

         /**
          * Overloads the base class version to report the comment to the client.
          */
         virtual void on_log_command(HttpClient::Connection *sender, StrAsc const &comment)
         {
            if(SmtpSenderClient::is_valid_instance(client))
               client->on_log(component, comment);
         }

         /**
          * Overloads the base class version to report the data.
          */
         virtual void on_data(HttpClient::Connection *sender, void const *buff, size_t buff_len, bool received)
         {
            if(SmtpSenderClient::is_valid_instance(client))
            {
               if(received)
                  client->on_bytes_received(component, buff, (uint4)buff_len);
               else
                  client->on_bytes_sent(component, buff, (uint4)buff_len);
            }
         }
      };
   };


   void SmtpSenderClient::format_outcome(
      std::ostream &out, outcome_type outcome)
   {
      switch(outcome)
      {
      case outcome_unknown_failure:
         out << "an unrecognised failure occurred";
         break;
         
      case outcome_success:
         out << "succeeded";
         break;
         
      case outcome_connection_failed:
         out << "SMTP server connection failed";
         break;
         
      case outcome_authorisation_failed:
         out << "SMTP authorisation failed";
         break;
         
      case outcome_tls_initialise_failed:
         out << "TLS initialisation failed";
         break;
         
      case outcome_data_ack_invalid:
         out << "invalid acknowledgement of data";
         break;
         
      case outcome_receipt_to_ack_invalid:
         out << "invalid acknowledgement of send to address";
         break;
         
      case outcome_password_ack_invalid:
         out << "invalid acknowledgement of password";
         break;

      case outcome_user_name_ack_invalid:
         out << "invalid acknowledgement of user name";
         break;
         
      case outcome_auth_ack_invalid:
         out << "invalid authorisation acknowledgement";
         break;
         
      case outcome_starttls_ack_invalid:
         out << "invalid acknowledgement of STARTTLS";
         break;
         
      case outcome_helo_ack_invalid:
         out << "invalid acknowledgement of HELO or EHLO";
         break;
         
      case outcome_timed_out:
         out << "timed out while waiting for the SMTP server";
         break;
      }
   } // format_outcome


   StrUni const &SmtpSender::get_product_name()
   { return product_name; }


   void SmtpSender::set_product_name(StrUni const &name)
   { product_name = name; }


   StrUni const &SmtpSender::get_product_version()
   { return product_version; }


   void SmtpSender::set_product_version(StrUni const &version)
   { product_version  = version; }


   void SmtpSender::set_gateway_model(StrUni const &value)
   {
      gateway_model = value;
      gateway_model.to_upper();
   }


   StrUni const &SmtpSender::get_gateway_model()
   { return gateway_model; }


   void SmtpSender::set_gateway_serial_no(StrUni const &value)
   {
      gateway_serial_no = value;
      gateway_serial_no.to_upper();
      if(gateway_serial_no.length() == 0)
      {
         StrAsc temp;
         Csi::RegistryManager::read_anywhere_string(
            temp,
            "MachineGuid",
            "SOFTWARE\\Microsoft\\Cryptography",
            HKEY_LOCAL_MACHINE,
            true);
         temp.to_upper();
         gateway_serial_no = temp;
      }
   }


   StrUni const &SmtpSender::get_gateway_serial_no()
   { return gateway_serial_no; }


   void SmtpSender::add_attachment(
      StrAsc const &file_name,
      StrAsc const &content_type,
      StrAsc const &content,
      StrAsc const &content_disposition)
   {
      attachment_type attachment;
      attachment.file_name = file_name;
      if(content_type.length())
         attachment.content_type = content_type;
      else
      {
         Csi::content_encoding_type encoding(Csi::resolve_content_type(file_name));
         attachment.content_type = encoding.first;
      }
      attachment.content = content;
      attachment.content_disposition = content_disposition;
      attachments.push_back(attachment);
   } // add_attachment
   
   
   void SmtpSender::start(client_type *client_)
   {
      StrAsc address;
      uint2 port(25);
      OStrAscStream log;
      
      if(state != state_standby)
         throw Csi::MsgExcept("email send already started");
      if(!client_type::is_valid_instance(client_))
         throw std::invalid_argument("invalid client pointer");
      client = client_;
      if(!use_gateway)
      {
         parse_uri_address(address, port, server_address.to_utf8());
         if(port == 0)
            port = 25;
         log << "connecting to " << address << ":" << port << "\n";
         using_tls = false;
         client->on_log(this, log.str());
         state = state_connecting;
         open(address.c_str(), port);
      }
      else
      {
         // we need to generate the JSON content that will be sent with the request.
         Json::ObjectHandle data(new Json::Object);
         Json::ArrayHandle to_array(new Json::Array);
         
         gateway_request_json.bind(new Json::Object);
         gateway_request_json->set_property_str("messagetype", "smtp");
         gateway_request_json->set_property_str("message", "send-email");
         data->set_property_str("subject", subject.to_utf8());
         data->set_property_str("message", message.to_utf8());
         for(addresses_type::iterator ai = to_addresses.begin(); ai != to_addresses.end(); ++ai)
            to_array->push_back(ai->to_utf8());
         data->set_property("to", to_array.get_handle());
         if(!cc_addresses.empty())
         {
            Json::ArrayHandle cc_array(new Json::Array);
            for(addresses_type::iterator ai = cc_addresses.begin(); ai != cc_addresses.end(); ++ai)
               cc_array->push_back(ai->to_utf8());
            data->set_property("cc", cc_array.get_handle());
         }
         if(!bcc_addresses.empty())
         {
            Json::ArrayHandle bcc_array(new Json::Array);
            for(addresses_type::iterator ai = bcc_addresses.begin(); ai != bcc_addresses.end(); ++ai)
               bcc_array->push_back(ai->to_utf8());
            data->set_property("bcc", bcc_array.get_handle());
         }
         if(!attachments.empty())
         {
            Json::ArrayHandle attachments_array(new Json::Array);
            for(attachments_type::iterator ai = attachments.begin(); ai != attachments.end(); ++ai)
            {
               attachment_type const &attachment(*ai);
               Json::ObjectHandle attachment_json(new Json::Object);
               StrAsc file_name;

               split_path(0, &file_name, attachment.file_name);
               attachment_json->set_property_str("filename", file_name);
               attachment_json->set_property_str("type", attachment.content_type);
               attachment_json->set_property_str("content_disposition", attachment.content_disposition);
               if(attachment.content.length() == 0)
               {
                  FileSystemObject attach_info(attachment.file_name.c_str());
                  if(attach_info.get_is_valid())
                  {
                     attachment_json->set_property("content", new Json::BlobFile(attachment.file_name));
                     attachments_array->push_back(attachment_json.get_handle());
                  }
               }
               else
               {
                  Csi::OStrAscStream encoded_content;
                  Base64::encode(encoded_content, attachment.content.c_str(), attachment.content.length(), false);
                  attachment_json->set_property_str("content", encoded_content.str());
                  attachments_array->push_back(attachment_json.get_handle());
               }
            }
            data->set_property("attachments", attachments_array.get_handle());
         }
         gateway_request_json->set_property("data", data.get_handle());

         // we can now create the connection if needed and start the HTTP request
         Csi::OStrAscStream device_id;
         if(http_connection == 0)
            http_connection.bind(new HttpClient::Connection(timer));
         device_id << "SN" << gateway_serial_no << "-" << gateway_model;
         http_request.bind(
            new HttpClient::Request(
               this, kdapi_smtp_uri, HttpClient::Request::method_post, false));
         http_request->set_authorisation(
            new HttpClient::AuthorisationKdapi(device_id.str(), "smtp", "send-email"));
         http_request->set_content_type("application/json");
         http_connection->set_watcher(new MyHttpWatcher(this, client));
         http_connection->add_request(http_request);
         state = state_send_http;
      }
   } // start


   void SmtpSender::on_connected(SocketAddress const &address)
   {
      OStrAscStream log;
      state = state_wait_220;
      clear_last_response = true;
      log << "connected to " << address << "\n";
      if(client_type::is_valid_instance(client))
         client->on_log(this, log.str());
      timeout_id = timer->arm(this, smtp_timeout);
   } // on_connected


   void SmtpSender::on_read()
   {
      ByteQueue &read_buffer(get_read_buffer());
      uint4 eol_pos;
      StrAsc response;
      OSocketTcpStream output(this);
      byte log_byte[1024];
      uint4 logged(0);
      
      SocketTcpSock::on_read();
      eol_pos = read_buffer.find("\r\n", 2);
      timer->reset(timeout_id);
      while(logged < read_buffer.size() && client_type::is_valid_instance(client))
      {
         uint4 count(read_buffer.copy(log_byte, sizeof(log_byte), logged));
         client->on_bytes_received(this, log_byte, count);
         logged += count;
      }
      while(eol_pos < read_buffer.size())
      {
         read_buffer.pop(last_response, eol_pos + 2, clear_last_response);
         eol_pos = read_buffer.find("\r\n", 2);
         clear_last_response = true;
         if(state == state_wait_220)
         {
            if(last_response.find("220-") < last_response.length())
               continue;
            else if(last_response.find("220 ") < last_response.length())
            {
               output <<  "EHLO " << get_host_name() << "\r\n";
               state = state_ehlo_wait_250;
            }
            else
               on_complete(client_type::outcome_helo_ack_invalid);
         }
         else if(state == state_ehlo_wait_250 || state == state_helo_wait_250)
         {
            if(last_response.find("250") == 0) // EHLO supported check
            {
               size_t end_multi_line(last_response.find("250 "));
               size_t end_line_multi_line(last_response.find("\r\n", end_multi_line));
               if(end_line_multi_line < last_response.length())
               {
                  // check for the secure auth first
                  if(!using_tls &&
                     (last_response.find("250-STARTTLS") < last_response.length() ||
                      last_response.find("250 STARTTLS") < last_response.length()))
                  {
                     output << "STARTTLS\r\n";
                     state = state_starttls_wait_220;
                  }
                  else if(last_response.find("250-AUTH") < last_response.length() ||
                          last_response.find("250 AUTH") < last_response.length())
                  {
                     if(user_name.length() == 0 && password.length() == 0)
                     {
                        // no authentication is needed
                        format_mail_from(output);
                        state = state_rcpt_to_wait_250;
                        to_index = cc_index = bcc_index = 0;
                     }
                     else
                     {
                        if(last_response.find("CRAM-MD5") < last_response.length())
                        {
                           output << "AUTH CRAM-MD5\r\n" ;
                           state = state_cram_wait_334;
                        }
                        else if(last_response.find("LOGIN") < last_response.length())
                        {
                           output << "AUTH LOGIN\r\n";
                           state = state_login_name_wait_334;
                        }
                        else
                        {
                           StrAsc temp;
                           OStrAscStream auth;
                           auth << '\0' << user_name.to_utf8() << '\0' << password.to_utf8();
                           Base64::encode(temp, auth.c_str(), auth.length());
                           output << "AUTH PLAIN " << temp << "\r\n";
                           state = state_auth_wait_235;
                        }
                     }
                  }
                  else
                  {
                     // no authentication needed
                     format_mail_from(output);
                     state = state_rcpt_to_wait_250;
                     to_index = cc_index = bcc_index = 0;
                  }
               }
               else
                  clear_last_response = false;
            }
            else if(state == state_ehlo_wait_250)
            {
               // The EHLO failed, so fall back to the older style HELO. See
               // http://rfc-ref.org/RFC-TEXTS/2821/chapter4.html
               output << "HELO " << get_host_name() << "\r\n";
               state = state_helo_wait_250;
            }
         }
         else if(state == state_starttls_wait_220)
         {
            if(last_response.find("220") < last_response.length())
            {
               try
               {
                  if(SocketTcpSock::get_tls_context() == 0)
                     SocketTcpSock::set_tls_context(new TlsContext);
                  using_tls = true;
                  start_tls_client();
               }
               catch(std::exception &e)
               {
                  OStrAscStream log;
                  log << "TLS initialisation failed: \"" << e.what() << "\"\n";
                  if(client_type::is_valid_instance(client))
                     client->on_log(this, log.str());
                  on_complete(client_type::outcome_tls_initialise_failed);
               }
            }
            else
               on_complete(client_type::outcome_starttls_ack_invalid);
         }
         else if(state == state_auth_wait_235)
         {
            if(last_response.find("235") == 0)
            {
               format_mail_from(output);
               to_index = cc_index = bcc_index = 0;
               state = state_rcpt_to_wait_250;
            }
            else
               on_complete(client_type::outcome_auth_ack_invalid);
         }
         else if(state == state_login_name_wait_334)
         {
            if(last_response.find("334 VXNlcm5hbWU6") < last_response.length())
            {
               StrAsc auth(user_name.to_utf8());
               StrAsc temp;
               Base64::encode(temp, auth.c_str(), auth.length());
               output << temp << "\r\n";
               state = state_login_password_wait_334;
            }
            else
               on_complete(client_type::outcome_user_name_ack_invalid);
         }
         else if(state == state_login_password_wait_334)
         {
            if(last_response.find("334 UGFzc3dvcmQ6") == 0)
            {
               StrAsc auth(password.to_utf8());
               StrAsc temp;
               Base64::encode(temp, auth.c_str(), auth.length());
               output << temp << "\r\n";
               state = state_auth_wait_235;
            }
            else
               on_complete(client_type::outcome_password_ack_invalid);
         }
         else if(state == state_cram_wait_334)
         {
            if(last_response.find("334") == 0)
            {
               StrAsc cram_digest;
               last_response.cut(0, 4);
               cram_digest = SmtpHelpers::generate_cram_digest(
                  last_response, user_name.to_utf8(), password.to_utf8());
               output << cram_digest << "\r\n";
               state = state_auth_wait_235;
            }
            else
               on_complete(client_type::outcome_password_ack_invalid);
         }
         else if(state == state_rcpt_to_wait_250)
         {
            if(last_response.find("250") == 0)
            {
               if(to_index < to_addresses.size())
                  output << "RCPT TO:<" << to_addresses[to_index++].to_utf8() << ">\r\n";
               else if(cc_index < cc_addresses.size())
                  output << "RCPT TO:<" << cc_addresses[cc_index++].to_utf8() << ">\r\n";
               else if(bcc_index < bcc_addresses.size())
                  output << "RCPT TO:<" << bcc_addresses[bcc_index++].to_utf8() << ">\r\n";
               else
               {
                  output << "DATA\r\n";
                  state = state_data_wait_354;
               }
            }
            else
               on_complete(client_type::outcome_receipt_to_ack_invalid);
         }
         else if(state == state_data_wait_354)
         {
            if(last_response.find("354") == 0)
            {
               // Output the formatted message and the terminating sequence. 
               if(message_out.str().length() == 0)
                  format_message(message_out);
               output << message_out.str();
               output << "\r\n.\r\n";
               state = state_data_wait_250;
            }
            else
               on_complete(client_type::outcome_data_ack_invalid);
         }
         else if(state == state_data_wait_250)
         {
            if(last_response.find("250") == 0)
            {
               output << "QUIT\r\n";
               state = state_quit_wait_221;
            }
            else
               on_complete(client_type::outcome_data_ack_invalid);
         }
         else if(state == state_quit_wait_221)
            on_complete(client_type::outcome_success);
      }
   } // on_read


   void SmtpSender::write(void const *buff, uint4 buff_len)
   {
      if(client_type::is_valid_instance(client))
         client->on_bytes_sent(this, buff, buff_len);
      SocketTcpSock::write(buff, buff_len);
   } // write


   void SmtpSender::on_tls_client_ready()
   {
      OSocketTcpStream output(this);
      SocketTcpSock::on_tls_client_ready();
      output << "EHLO " << get_host_name() << "\r\n";
      state = state_ehlo_wait_250;
   } // on_tls_client_ready


   void SmtpSender::on_socket_error(int err)
   {
      OStrAscStream log;
      log << "connection failed";
      if(err != 0)
      {
         SocketException e("", err);
         log << ": \"" << e.what() << "\"";
      }
      if(client_type::is_valid_instance(client))
         client->on_log(this, log.str());
      if(state == state_quit_wait_221)
         on_complete(client_type::outcome_success);
      else
         on_complete(client_type::outcome_connection_failed);
   } // on_socket_error


   void SmtpSender::onOneShotFired(uint4 id)
   {
      if(id == timeout_id)
      {
         timeout_id = 0;
         on_complete(client_type::outcome_timed_out);
      }
   } // onOneShotFired


   void SmtpSender::receive(SharedPtr<Event> &ev)
   {
      if(ev->getType() == event_complete::event_id)
      {
         event_complete *event(static_cast<event_complete *>(ev.get_rep()));
         client_type *client(this->client);
         this->client = 0;
         state = state_standby;
         if(client_type::is_valid_instance(client))
            client->on_complete(this, event->outcome);
      }
#ifndef WIN32
      else
         SocketTcpSock::receive(ev);
#endif
   } // receive


   void SmtpSender::format_message(std::ostream &out)
   {
      // format the message ID
      LgrDate now(LgrDate::gmt());
      StrAsc from_domain(from_address.to_utf8());
      size_t at_pos(from_domain.find("@"));
      OStrAscStream temp;
      
      from_domain.cut(0, at_pos);
      now.format(temp, "Message-ID: <%Y%m%d%H%M%S%x");
      temp << from_domain << ">\r\n";
      out << temp.str();
      
      // format the date for the message
      temp.str("");
      temp << "Date: ";
      SmtpHelpers::format_smtp_date(temp, now);
      temp << "\r\n";
      out << temp.str();
      
      // format the from address
      out << "From: \"" << from_address.to_utf8() << "\" <"
          << from_address.to_utf8() << ">\r\n";
      
      // format the user agent
      out << "User-Agent: " << product_name << " "
          << product_version << "\r\n";
      
      // format the mime version
      out << "MIME-Version: 1.0\r\n";
      
      // format the to addresses
      if(!to_addresses.empty())
      {
         out << "TO: ";
         for(addresses_type::iterator ai = to_addresses.begin();
             ai != to_addresses.end();
             ++ai)
         {
            if(ai != to_addresses.begin())
               out << ", ";
            out << ai->to_utf8();
         }
         out << "\r\n";
      }
      
      // format the CC addresses
      if(!cc_addresses.empty())
      {
         out << "CC: ";
         for(addresses_type::iterator ai = cc_addresses.begin();
             ai != cc_addresses.end();
             ++ai)
         {
            if(ai != cc_addresses.begin())
               out << ", ";
            out << ai->to_utf8();
         }
         out << "\r\n"; 
      }
      
      // encode the subject line as UTF-8 and BASE64
      StrAsc subject_utf8(subject.to_utf8());
      StrAsc subject_base64;
      Base64::encode(subject_base64, subject_utf8.c_str(), subject_utf8.length());
      out << "Subject: =?UTF-8?b?" << subject_base64 << "?=\r\n";
      
      // If the attachments are present, we need to out a multi-part format
      if(!attachments.empty())
      {
         // format the multi-part header
         static char const boundary[] = "CampbellSci_Boundary";
         out << "Content-Type: multipart/mixed;\r\n"
             << " boundary=\"" << boundary << "\"\r\n"
             << "This is a multi-part message in MIME format.\r\n";
         
         // format the message
         StrAsc message_utf8(message.to_utf8());
         OStrAscStream temp;
         temp << "--" << boundary << "\r\n"
              << "Content-Type: text/plain; charset=UTF-8\r\n"
              << "Content-Transfer-Encoding: base64\r\n\r\n";
         Base64::encode(temp, message_utf8.c_str(), message_utf8.length());
         out << temp.str() << "\r\n\r\n";
         
         // format the attachments
         for(attachments_type::iterator ai = attachments.begin();
             ai != attachments.end();
             ++ai)
         {
            attachment_type const &attachment(*ai);
            try
            {
               // we need to convert the attachment name into a multi-byte path which can
               // then be split.
               StrAsc file_name;
               split_path(0, &file_name, attachment.file_name);
               
               // we will now open the attachment as a memory mapped file and format the
               // multi-part header.
               out << "\r\n--" << boundary << "\r\n"
                   << "Content-Type: " << attachment.content_type << ";\r\n"
                   << " name=\"" << file_name << "\"\r\n"
                   << "Content-Transfer-Encoding: base64\r\n";
               if(attachment.content_disposition.length() == 0)
                  out << "Content-Disposition: attachment;\r\n  filename=" << file_name << "\r\n";
               else
                  out << attachment.content_disposition << "\r\n";
               out << "\r\n";
               if(attachment.content.length())
               {
                  ReadFileMapping file(attachment.file_name.c_str());
                  Base64::encode(
                     out, static_cast<char const *>(file.open_view()), static_cast<uint4>(file.file_size()));
               }
               else
                  Base64::encode(out, attachment.content.c_str(), attachment.content.length());
               out << "\r\n";
            }
            catch(std::exception &e)
            {
               OStrAscStream log;
               log << "open attachment failed: \"" << e.what() << "\"";
               if(client_type::is_valid_instance(client))
                  client->on_log(this, log.str());
            }
         }
         out << "\r\n--" << boundary << "--\r\n";
      }
      else
      {
         StrAsc message_utf8(message.to_utf8());
         OStrAscStream temp;
         
         temp << "Content-Type: text/plain; charset=UTF-8\r\n"
              << "Content-Transfer-Encoding: base64\r\n\r\n";
         Base64::encode(temp, message_utf8.c_str(), message_utf8.length());
         out << temp.str();
      }
   } // format_message


   void SmtpSender::format_mail_from(std::ostream &out)
   {
      OStrAscStream temp;
      message_out.str("");
      format_message(message_out);
      temp << "MAIL FROM:<" << from_address.to_utf8() << "> SIZE="
           << message_out.str().length() << "\r\n";
      out << temp.str();
   } // format_mail_from


   void SmtpSender::on_connected(HttpClient::Request *sender)
   {
      if(http_request == sender && client_type::is_valid_instance(client))
         client->on_log(this, "connected to the gateway service");
   } // on_connected


   void SmtpSender::on_header_sent(HttpClient::Request *sender)
   {
      if(http_request == sender && client_type::is_valid_instance(client))
      {
         HttpClient::ORequestStream output(sender);
         gateway_request_json->format(output);
         output.flush();
         sender->add_bytes("", 0, true);
         client->on_log(this, "gateway service request sent");
      }
   } // on_header_sent
   

   void SmtpSender::on_response_header(HttpClient::Request *sender)
   {
      if(http_request == sender && client_type::is_valid_instance(client))
      {
         Csi::OStrAscStream message;
         message << "gateway service response header received: "
                 << sender->get_response_code() << " " << sender->get_response_description();
         client->on_log(this, message.str());
      }
   } // on_response_header


   void SmtpSender::on_response_data(HttpClient::Request *sender)
   {
   } // on_response_data


   bool SmtpSender::on_failure(HttpClient::Request *sender)
   {
      bool rtn(http_request == sender && client_type::is_valid_instance(client));
      if(rtn)
      {
         switch(sender->get_response_code())
         {
         default:
            on_complete(client_type::outcome_data_ack_invalid);
            break;

         case 401:
         case 403:
            on_complete(client_type::outcome_authorisation_failed);
            break;

         case 408:
            on_complete(client_type::outcome_timed_out);
            break;
         }
      }
      return rtn;
   } // on_failure


   bool SmtpSender::on_response_complete(HttpClient::Request *sender)
   {
      bool rtn(http_request == sender && client_type::is_valid_instance(client));
      if(rtn)
      {
         if(sender->get_response_code() == 200)
         {
            try
            {
               HttpClient::IRequestStream in(sender);
               Csi::Json::Object response;
               response.parse(in);
               if(response.get_property_str("messagetype") == "acknowledgement" &&
                  response.get_property_str("message") == "email-sent")
               {
                  on_complete(client_type::outcome_success);
               }
               else
                  on_complete(client_type::outcome_data_ack_invalid);
            }
            catch(std::exception &)
            {
               on_complete(client_type::outcome_data_ack_invalid);
            }
         }
         else
         {
            // @todo: we need to determine what to do with failed HTTP requests.
         }
      }
      return rtn;
   } // on_response_complete

   
   void SmtpSender::on_complete(client_type::outcome_type outcome)
   {
      close();
      if(timeout_id != 0)
         timer->disarm(timeout_id);
      if(http_connection != 0)
         http_connection->set_watcher(0);
      http_request.clear();
      event_complete::cpost(this, outcome);
   } // on_complete
};


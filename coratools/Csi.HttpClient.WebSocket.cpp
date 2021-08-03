/* Csi.HttpClient.WebSocket.cpp

   Copyright (C) 2015, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Wednesday 21 October 2015
   Last Change: Thursday 30 June 2016
   Last Commit: $Date: 2017-04-03 16:25:43 -0600 (Mon, 03 Apr 2017) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.HttpClient.WebSocket.h"
#include "Csi.Digest.h"
#include "Csi.Base64.h"
#include "Csi.ByteOrder.h"
#include "Csi.Utils.h"


namespace Csi
{
   namespace HttpClient
   {
      namespace
      {
         /**
          * Defines an event object that reports an error.
          */
         class event_error: public Event
         {
         public:
            /**
             * Specifies the event identifier for this class.
             */
            static uint4 const event_id;

            /**
             * Specifies the failure that has occurred.
             */
            WebSocketClient::failure_type const failure;

            /**
             * Specifies the HTTP response code.
             */
            int const http_resp;

            /**
             * Creates and posts this event type.
             */
            static void cpost(WebSocket *socket, WebSocketClient::failure_type failure, int http_resp)
            {
               event_error *event(new event_error(socket, failure, http_resp));
               event->post();
            }

         private:
            /**
             * Constructor
             */
            event_error(
               WebSocket *socket, WebSocketClient::failure_type failure_, int http_resp_):
               Event(event_id, socket),
               failure(failure_),
               http_resp(http_resp_)
            { }
         };


         uint4 const event_error::event_id(
            Event::registerType("Csi::HttpClient::WebSocket::event_error"));
      };

      
      WebSocket::WebSocket(timer_handle timer_, uint4 ping_interval_):
         state(state_standby),
         client(0),
         timer(timer_),
         watchdog_id(0),
         pings_count(0),
         ping_interval(csimax(ping_interval_, (uint4)15000))
      {
         if(timer == 0)
            timer.bind(new OneShot);
      } // constructor


      WebSocket::~WebSocket()
      {
         request.clear();
         connection.clear();
         client = 0;
         if(timer != 0 && watchdog_id)
            timer->disarm(watchdog_id);
         timer.clear();
      } // destructor


      void WebSocket::connect(
         WebSocketClient *client_,
         Uri const &uri_,
         StrAsc const &protocol, 
         StrAsc const &auth_name,
         StrAsc const &auth_password)
      {
         // We need to be in a standby state in order to connect.
         if(state != state_standby)
            throw std::invalid_argument("WebSocket already started");
         if(!WebSocketClient::is_valid_instance(client_))
            throw std::invalid_argument("Invalid WebSocket client pointer");
         
         // we need to create the connection and send a request to start the handshape
         client = client_;
         uri = uri_;
         connection.bind(new Connection(timer));
         request.bind(new Request(this, uri));
         request->set_authentication(auth_name, auth_password);
         request->set_upgrade("websocket");
         request->set_websock_protocol(protocol);
         state = state_connecting;
         if(watcher != 0)
            connection->set_watcher(watcher);
         connection->add_request(request);
      } // connect


      void WebSocket::close()
      {
         request.clear();
         connection.clear();
         state = state_standby;
      } // close


      void WebSocket::send_message(
         void const *content,
         uint4 content_len,
         websock_op_code op_code,
         bool fin)
      {
         // we need to ensure that we can transmit
         if(state < state_before_frame)
            throw std::invalid_argument("invalid state to transmit");
         if(watcher != 0 && op_code == websock_op_text)
         {
            // since the message is being sent, it will be masked which makes it hard to read.  We
            // will send any text message to the log as annotations.
            StrAsc text(static_cast<char const *>(content), content_len);
            watcher->on_log_comment(connection.get_rep(), text);
         }
         
         // format the header
         uint2 header(0);
         uint2 header_len;
         uint2 op_code_val(op_code);
         uint4 mask_val(rand());
         byte mask[4];
         mask_val <<= 16;
         mask_val |= rand();
         memcpy(mask, &mask_val, sizeof(mask));
         if(content_len <= 125)
            header_len = static_cast<uint2>(content_len);
         else if(content_len <= 65535)
            header_len = 126;
         else
            header_len = 127;
         header_len |= 0x80;    // indicate masking
         if(fin)
            header |= 0x8000;
         header |= header_len;
         header |= (op_code_val << 8);
         if(!is_big_endian())
            reverse_byte_order(&header, sizeof(header));
         tx_payload.cut(0);
         tx_payload.append(&header, sizeof(header));

         // we may need to append an extended size.
         if(content_len > 65535)
         {
            int8 extended_len(content_len);
            if(!is_big_endian())
               reverse_byte_order(&extended_len, sizeof(extended_len));
            tx_payload.append(&extended_len, sizeof(extended_len));
         }
         else if(content_len > 125)
         {
            uint2 extended_len(static_cast<uint2>(content_len));
            if(!is_big_endian())
               reverse_byte_order(&extended_len, sizeof(extended_len));
            tx_payload.append(&extended_len, sizeof(extended_len));
         }

         // we need to add the mask to the frame
         byte const *content_bytes(static_cast<byte const *>(content));
         tx_payload.append(mask, sizeof(mask));
         tx_payload.reserve(content_len);
         for(uint4 i = 0; i < content_len; ++i)
            tx_payload.append(content_bytes[i] ^ mask[i % 4]);
         connection->write(tx_payload.getContents(), (uint4)tx_payload.length());
         connection->add_log_comment("message transmitted");
      } // send_message


      bool WebSocket::on_failure(Request *sender)
      {
         do_report_error(WebSocketClient::failure_connect, 503);
         return false;
      } // on_failure


      bool WebSocket::on_response_complete(Request *sender)
      {
         bool rtn(false);
         if(sender->get_response_code() == 101)
         {
            // we need to confirm the server's acceptance
            StrAsc const suffix("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
            StrAsc const key(sender->get_websock_key());
            StrAsc const server_accept(sender->get_websock_accept());
            StrAsc my_accept;
            Csi::Sha1Digest digest;
            digest.add(key.c_str(), key.length());
            digest.add(suffix.c_str(), suffix.length());
            Base64::encode(my_accept, digest.final(), Csi::Sha1Digest::digest_size);
            if(server_accept == my_accept)
            {
               connection->set_upgrade(this);
               rtn = true;
               state = state_before_frame;
               pings_count = 0;
               watchdog_id = timer->arm(this, ping_interval);
               client->on_connected(this);
            }
            else
               do_report_error(WebSocketClient::failure_unsupported, sender->get_response_code());
         }
         else if(sender->get_response_code() == 200 || sender->get_response_code() == 301)
            do_report_error(WebSocketClient::failure_unsupported, sender->get_response_code());
         else
            do_report_error(WebSocketClient::failure_connect, sender->get_response_code());
         return rtn;
      } // on_response_complete
      

      void WebSocket::receive(SharedPtr<Event> &ev)
      {
         if(ev->getType() == event_error::event_id)
         {
            event_error *event(static_cast<event_error *>(ev.get_rep()));
            WebSocketClient *report(client);
            close();
            if(WebSocketClient::is_valid_instance(report))
               report->on_failure(this, event->failure, event->http_resp);
         }
      } // receive

      void WebSocket::set_watcher(watcher_handle watcher_)
      {
         watcher = watcher_;
         if(connection != 0)
            connection->set_watcher(watcher);
      } // set_watcher


      void WebSocket::onOneShotFired(uint4 id)
      {
         if(id == watchdog_id)
         {
            StrAsc const ping_data("ping");
            watchdog_id = 0;
            if(pings_count++ < 2)
            {
               watchdog_id = timer->arm(this, ping_interval);
               send_message(ping_data.c_str(), (uint4)ping_data.length(), websock_op_ping);
            }
            else
               do_report_error(WebSocketClient::failure_connect, 503);
         }
      } // onOneShotFired


      void WebSocket::on_read(Connection *sender)
      {
         ByteQueue &buff(sender->get_read_buffer());
         bool state_changed(false);
         if(state >= state_before_frame && watchdog_id != 0)
         {
            pings_count = 0;
            timer->reset(watchdog_id);
         }
         if(state == state_before_frame)
         {
            if(buff.size() >= 2)
            {
               // read the header from the buffer
               buff.pop(&last_header, sizeof(last_header));
               if(!is_big_endian())
                  reverse_byte_order(&last_header, sizeof(last_header));

               // we need to decode the header to determine our next state
               payload_len = last_header & 0x7f;
               state_changed = true;
               if(payload_len > 125)
                  state = state_read_len;
               else
               {
                  if((last_header & 0x0080) != 0)
                     state = state_read_mask;
                  else
                     state = state_read_payload;
               }
            }
         }
         else if(state == state_read_len)
         {
            if(payload_len == 126 && buff.size() >= 2)
            {
               uint2 extended_len;
               buff.pop(&extended_len, sizeof(extended_len));
               if(!is_big_endian())
                  reverse_byte_order(&extended_len, sizeof(extended_len));
               payload_len = extended_len;
               if((last_header & 0x0080) != 0)
                  state = state_read_mask;
               else
                  state = state_read_payload;
               state_changed = true;
            }
            else if(payload_len == 127 && buff.size() >= 8)
            {
               buff.pop(&payload_len, sizeof(payload_len));
               if(!is_big_endian())
                  reverse_byte_order(&payload_len, sizeof(payload_len));
               if((last_header & 0x0080) != 0)
                  state = state_read_mask;
               else
                  state = state_read_payload;
               state_changed = true;
            }
         }
         else if(state == state_read_mask)
         {
            if(buff.size() >= sizeof(rx_mask))
            {
               buff.pop(&rx_mask, sizeof(rx_mask));
               state = state_read_payload;
               state_changed = true;
            }
         }
         else if(state == state_read_payload)
         {
            if(buff.size() >= payload_len)
            {
               // we need to extract the payload from the buffer.  If the header indicates a mask, we need to apply that
               // mask.
               payload.cut(0);
               buff.pop(payload, static_cast<uint4>(payload_len));
               if((last_header & 0x0080) != 0)
               {
                  for(size_t i = 0; i < payload.length(); ++i)
                     payload[i] = rx_mask[i % 4] ^ payload[i];
               }

               // we can now report this frame
               bool finished((last_header & 0x8000) != 0);
               uint2 op_code((last_header & 0x0f00) >> 8);
               state = state_before_frame;
               state_changed = true;
               do_on_message(op_code, finished);
            }
         }
         if(state_changed && buff.size() > 0)
            on_read(sender);
      } // on_read


      void WebSocket::on_error(Connection *sender, int error_code)
      {
         do_report_error(WebSocketClient::failure_connect, 503);
      } // on_error


      void WebSocket::do_report_error(
         WebSocketClient::failure_type failure,
         int http_resp)
      {
         if(watchdog_id != 0)
            timer->disarm(watchdog_id);
         event_error::cpost(this, failure, http_resp);
      } // do_report_error


      void WebSocket::do_on_message(uint2 op_code, bool finished)
      {
         if(watcher != 0)
         {
            OStrAscStream message;
            message << "message received: op=" << op_code << ", fin=" << finished
                    << ", len=" << payload.length() << ", accum=" << complete_buffer.length()
                    << ", last_head=" << std::hex << last_header;
            watcher->on_log_comment(connection.get_rep(), message.str());
         }
         if(op_code == websock_op_continuation)
         {
            complete_buffer.append(payload.getContents(), payload.length());
            if(finished)
            {
               client->on_message(
                  this,
                  complete_buffer.getContents(),
                  (uint4)complete_buffer.length(),
                  fragmented_op_code,
                  finished);
               complete_buffer.cut(0);
            }
         }
         if(op_code == websock_op_text || op_code == websock_op_binary)
         {
            complete_buffer.append(payload.getContents(), payload.length());
            fragmented_op_code = static_cast<websock_op_code>(op_code);
            if(finished)
            {
               client->on_message(
                  this,
                  complete_buffer.getContents(),
                  (uint4)complete_buffer.length(),
                  fragmented_op_code,
                  finished);
               complete_buffer.cut(0);
            }
         }
         else if(op_code == websock_op_close)
            do_report_error(WebSocketClient::failure_server_closed, 0);
         else if(op_code == websock_op_ping)
         {
            send_message(
               payload.getContents(), (uint4)payload.length(), websock_op_pong, true);
         }
      } // do_on_message
   };
};


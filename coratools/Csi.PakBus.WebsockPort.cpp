/* Csi.PakBus.WebsockPort.cpp

   Copyright (C) 2020, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Saturday 29 February 2020
   Last Change: Tuesday 03 March 2020
   Last Commit: $Date: 2020-03-05 09:53:24 -0600 (Thu, 05 Mar 2020) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.PakBus.WebsockPort.h"
#include "Csi.PakBus.Router.h"
#include "Csi.PakBus.LowLevelDecoder.h"
#include "Csi.PakBus.SerialPacket.h"
#include "Csi.Utils.h"


namespace Csi
{
   namespace PakBus
   {
      namespace
      {
         /**
          * @return Returns the combined session identifier for the source and dest addresses.
          *
          * @param source Specifies the source address.
          *
          * @param dest Specifies the dest address.
          */
         uint4 get_session_key(uint4 source, uint4 dest)
         {
            return (source << 16) | dest;
         }
      };

      
      WebsockPort::WebsockPort(StrAsc const &network_id_, StrAsc const &ws_url_):
         network_id(network_id_),
         beacon_id(0),
         ws_url(ws_url_),
         server_connected(false)
      { }

      
      WebsockPort::~WebsockPort()
      {
         pending_messages.clear();
         pending_broadcasts.clear();
         if(timer != nullptr)
         {
            if(retry_id)
               timer->disarm(retry_id);
            if(beacon_id)
               timer->disarm(beacon_id);
            timer.clear();
         }
         router.clear();
         server.clear();
      } // destructor

      
      void WebsockPort::on_message_ready(uint2 phys_dest, priority_type priority)
      {
         message_handle message;
         while(router->get_next_port_message(this, phys_dest, message))
         {
            if(server != nullptr && server_connected)
               send_message(message);
            else
            {
               pending_messages.push_back(message);
               start_connect();
            }
         }
      } // on_message_ready

      
      void WebsockPort::broadcast_message(message_handle &message)
      {
         if(server != nullptr && server_connected)
            send_message(message);
         else
            pending_broadcasts.push_back(message);
      } // broadcast_message

      
      void WebsockPort::on_message_aborted(uint2 phys_dest)
      {
         pending_messages_type::iterator ri(
            std::remove_if(
               pending_messages.begin(),
               pending_messages.end(),
               [phys_dest](message_handle &message) {
                  return message->get_physical_destination() == phys_dest;
               }));
         if(ri != pending_messages.end())
            pending_messages.erase(ri, pending_messages.end());
      } // on_message_aborted

      
      uint2 WebsockPort::get_beacon_interval()
      {
         return 60;
      } // get_beacon_interval

      
      uint4 WebsockPort::get_worst_case_response()
      {
         return 10000;
      } // get_worst_case_response

      
      bool WebsockPort::has_session(uint2 source, uint2 dest)
      {
         auto si(sessions.find(get_session_key(source, dest)));
         bool rtn(false);
         if(si != sessions.end())
         {
            rtn = Csi::counter(si->second) < 40000;
            if(!rtn)
               sessions.erase(si);
         }
         return rtn;
      } // has_session

      
      bool WebsockPort::link_is_active()
      {
         return server != nullptr && server_connected;
      } // link_is_active

      
      bool WebsockPort::must_close_link()
      {
         return false;
      } // must_close_link

      
      void WebsockPort::on_neighbour_lost(uint2 phys_address)
      {
      } // on_neighbour_lost

      
      void WebsockPort::reset_session_timer(uint2 source, uint2 dest)
      {
         auto si(sessions.find(get_session_key(source, dest)));
         if(si != sessions.end())
            si->second = Csi::counter(0);
      } // reset_session_timer

      
      StrAsc WebsockPort::get_port_name() const
      {
         return network_id;
      } // get_port_name


      void WebsockPort::set_pakbus_router(router_handle router_)
      {
         router = router_;
         if(router != nullptr)
         {
            timer = router->get_timer();
            retry_id = timer->arm(this, 10);
         }
      } // set_pakbus_router


      WebsockPort::router_handle &WebsockPort::get_pakbus_router()
      { return router; }

      
      void WebsockPort::on_connected(server_type *sender)
      {
         server_connected = true;
         while(!pending_broadcasts.empty())
         {
            send_message(pending_broadcasts.front());
            pending_broadcasts.pop_front();
         }
         while(!pending_messages.empty())
         {
            send_message(pending_messages.front());
            pending_messages.pop_front();
         }
         send_beacon();
      } // on_connected

      
      void WebsockPort::on_failure(server_type *sender, failure_type failure, int http_response)
      {
         server_connected = false;
         server.clear();
         if(router != nullptr)
            router->on_port_delivery_failure(this);
         retry_id = timer->arm(this, 10000);
      } // on_failure

      
      void WebsockPort::on_message(
         server_type *sender,
         void const *content,
         uint4 content_len,
         HttpClient::websock_op_code op_code,
         bool fin)
      {
         if(op_code == HttpClient::websock_op_binary)
         {
            LowLevelDecoder decoder;
            uint4 begins_at(0), processed(0);
            auto rcd(decoder.decode(content, content_len, begins_at, processed));
            if(rcd == LowLevelDecoder::decode_found_serial_packet ||
               rcd == LowLevelDecoder::decode_found_unquoted_packet)
            {
               SerialPacket frame(decoder.get_storage(), decoder.get_storage_len(), false);
               bool is_broadcast(frame.get_destination_physical_address() == Router::broadcast_address);
               if(should_process_message(
                     frame.get_source_physical_address(),
                     frame.get_destination_physical_address(),
                     router.get_rep()))
               {
                  message_handle message(frame.make_pakbus_message());
                  router->on_beacon(this, frame.get_source_physical_address(), is_broadcast);
                  if(is_broadcast)
                     message->set_destination(router->get_this_node_address());
                  else
                     update_expect_more(message);
                  router->on_port_message(this, message);
               }
            }
         }
      } // on_message


      void WebsockPort::onOneShotFired(uint4 id)
      {
         if(id == retry_id)
         {
            retry_id = 0;
            start_connect();
         }
         else if(id == beacon_id)
         {
            beacon_id = 0;
            send_beacon();
         }
      } // onOneShotFired


      void WebsockPort::send_message(message_handle &message)
      {
         message->set_source(router->get_this_node_address());
         SerialPacket frame(*message);
         uint2 frame_len(static_cast<uint2>(frame.length()));
         frame.set_source_physical_address(router->get_this_node_address());
         frame_buffer.cut(0);
         frame_buffer.append(&LowLevelDecoder::synch_byte, 1);
         frame_buffer.append("\xf0", 1);
         if(!Csi::is_big_endian())
            Csi::reverse_byte_order(&frame_len, sizeof(frame_len));
         frame_buffer.append(&frame_len, sizeof(frame_len));
         frame_buffer.append(frame.getMsg(), frame.length());
         frame_buffer.append(&LowLevelDecoder::synch_byte, 1);
         update_expect_more(message);
         server->send_message(frame_buffer.getContents(), frame_buffer.length(), HttpClient::websock_op_binary);
      } // send_message


      void WebsockPort::start_connect()
      {
         if(retry_id)
            timer->disarm(retry_id);
         server_connected = false;
         server.bind(new server_type(timer));
         server->connect(this, ws_url, StrAsc("com.campbellsci.pbws.") + network_id);
      } // start_connect


      void WebsockPort::update_expect_more(message_handle &message)
      {
         uint2 source(message->get_source());
         uint2 dest(message->get_destination());
         uint4 key(get_session_key(source, dest));
         switch(message->get_expect_more())
         {
         case ExpectMoreCodes::expect_more:
            sessions[key] = Csi::counter(0);
            break;
            
         case ExpectMoreCodes::last:
            sessions.erase(key);
            break;
            
         case ExpectMoreCodes::expect_more_opposite:
            sessions[get_session_key(dest, source)] = Csi::counter(0);
            break;
         }
      } // update_expect_more


      void WebsockPort::send_beacon()
      {
         uint4 interval(get_beacon_interval());
         if(interval != 0 && interval != 0xffff)
         {
            message_handle message(new Message);
            message->set_destination(Router::broadcast_address);
            message->set_physical_destination(Router::broadcast_address);
            message->set_expect_more(ExpectMoreCodes::neutral);
            send_message(message);
            beacon_id = timer->arm(this, interval * 1000);
         }
      } // send_beacon
   };
};


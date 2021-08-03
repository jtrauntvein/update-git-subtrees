/* Csi.Posix.SocketConnection.cpp

   Copyright (C) 2005, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 20 September 2005
   Last Change: Thursday 04 February 2016
   Last Commit: $Date: 2016-02-04 11:02:04 -0600 (Thu, 04 Feb 2016) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Posix.SocketConnection.h"
#include "Csi.Messaging.Defs.h"
#include "Csi.Messaging.Router.h"
#include "Csi.Messaging.Message.h"
#include "Csi.MaxMin.h"
#include "Csi.Utils.h"
#include "Csi.StrAscStream.h"
#include <netinet/in.h>


namespace Csi
{
   namespace Posix
   {
      ////////////////////////////////////////////////////////////
      // class SocketConnection definitions
      ////////////////////////////////////////////////////////////
      SocketConnection::SocketConnection(
         int socket_handle):
         SocketTcpSock(socket_handle),
         is_server(true),
         between_messages(true),
         message_len(0),
         server_port(0)
      { }


      SocketConnection::SocketConnection(
         char const *server_name_,
         uint2 server_port_):
         SocketTcpSock(-1),
         is_server(false),
         between_messages(true),
         message_len(0),
         server_name(server_name_),
         server_port(server_port_)
      { }


      SocketConnection::~SocketConnection()
      { }


      void SocketConnection::sendMessage(
         Csi::Messaging::Message *msg)
      {
         int4 message_len = htonl(msg->getLen());
         
         assert(msg->getLen() >= 4);
         write(&message_len,sizeof(message_len));
         write(msg->getMsg(),msg->getLen());
      } // sendMessage


      void SocketConnection::attach()
      {
         if(!is_server)
         {
            SocketTcpSock::close();
            SocketTcpSock::open(server_name.c_str(),server_port);
         }
      } // attach


      void SocketConnection::detach()
      {
         SocketTcpSock::close(); 
      } // detach


      StrAsc SocketConnection::get_remote_address()
      {
         Csi::OStrAscStream rtn;
         StrAsc peer_address;
         uint2 peer_port;
         if(get_peer_address(peer_address, peer_port))
            rtn << peer_address << ":" << peer_port;
         return rtn.str();
      } // get_remote_address


      void SocketConnection::on_read()
      {
         try
         {
            bool more_messages = true;
            while(more_messages)
            {
               // determine if the message length is available
               if(between_messages && read_buffer.size() >= 4)
               {
                  uint4 len;
                  read_buffer.pop(&len,sizeof(len));
                  message_len = ntohl(len);
                  if(message_len < 4)
                     continue;  // treat too short messages as heartbeats
                  if(message_len > get_available_virtual_memory())
                     throw MsgExcept("Not enough memory to buffer the message");
                  between_messages = false;
               }

               // now determine if the reset of the message can be read
               if(!between_messages && message_len <= read_buffer.size())
               {
                  // all messages except the heartbeat will be sent through the router
                  Messaging::Message msg(read_buffer,message_len);

                  between_messages = true;
                  if(msg.getMsgType() != Messaging::Messages::type_heart_beat)
                     router->rcvMessage(&msg);
               }

               // now determine if there is enough content to continue
               if((!between_messages && message_len > read_buffer.size()) ||
                  (between_messages && read_buffer.size() < 4))
                  more_messages = false;
            }
         }
         catch(std::exception &e)
         {
            trace("Csi::Posix::SocketConnection::on_read -- read failure: %s",e.what());
            router->onConnClosed(Csi::Messaging::Router::closed_unknown_failure);
         }
      } // on_read


      void SocketConnection::on_close()
      {
         router->onConnClosed(Csi::Messaging::Router::closed_remote_disconnect);
      } // on_close

      
      uint4 SocketConnection::pop_length()
      {
         uint4 len;
         uint4 rtn;
         read_buffer.pop(&len,sizeof(len));
         rtn = ntohl(len);
         if(rtn < 4)
            throw MsgExcept("Invalid message length received");
         if(rtn > get_available_virtual_memory())
            throw MsgExcept("Not enough memory available to buffer the message");
         return rtn;
      } // pop_length
   };
};



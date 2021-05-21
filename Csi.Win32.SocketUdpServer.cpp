/* Csi.Win32.SocketUdpServer.cpp

   Copyright (C) 2004, 2017 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 26 February 2004
   Last Change: Tuesday 19 December 2017
   Last Commit: $Date: 2017-12-19 16:29:39 -0600 (Tue, 19 Dec 2017) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "Csi.Win32.SocketUdpServer.h"
#include "Csi.Win32.WinSockException.h"
#include "Csi.OsException.h"
#include "Csi.StrAscStream.h"
#include "trace.h"


namespace Csi
{
   namespace Win32
   {
      ////////////////////////////////////////////////////////////
      // class SocketUdpServer definitions
      ////////////////////////////////////////////////////////////
      SocketUdpServer::SocketUdpServer(
         SocketAddress const &bind_address_,
         uint4 max_datagram_size_):
         bind_address(bind_address_),
         max_datagram_size(max_datagram_size_),
         read_buffer(0)
      {
         // set up the socket handle
         OStrAscStream temp;
         temp << "listening on " << bind_address;
         trace(temp.c_str());
         socket_handle = socket(bind_address.get_family(), SOCK_DGRAM, 0);
         if(socket_handle == INVALID_SOCKET)
            throw WinsockException("Socket allocation failure");

         // bind the new socket to the specified port
         int rcd;
         rcd = bind(socket_handle, bind_address.get_storage(), bind_address.get_address_len());
         if(rcd == SOCKET_ERROR)
         {
            WinsockException e("Failure to bind a listening socket");
            closesocket(socket_handle);
            socket_handle = INVALID_SOCKET;
            throw e;
         }

         // we need to enable the socket to transmit broadcasts
         DWORD broadcast_enable(TRUE);
         ::setsockopt(
            socket_handle,
            SOL_SOCKET,
            SO_BROADCAST,
            reinterpret_cast<char const *>(&broadcast_enable),
            sizeof(broadcast_enable));
         
         // we need to set up the asynchronous event notification for the socket
         rcd = WSAAsyncSelect(
            socket_handle,
            get_window_handle(),
            socket_event,
            FD_READ);
         if(rcd == SOCKET_ERROR)
         {
            WinsockException e("Failure to set select() options");
            closesocket(socket_handle);
            socket_handle = INVALID_SOCKET;
            throw e;
         }

         // allocate the read buffer
         read_buffer = new char[max_datagram_size];
         if(read_buffer == 0)
         {
            OsException e("Read buffer allocation failure");
            closesocket(socket_handle);
            socket_handle = INVALID_SOCKET;
            throw e;
         }
      } // constructor

      
      SocketUdpServer::~SocketUdpServer()
      {
         if(read_buffer)
            delete[] read_buffer;
         if(socket_handle != INVALID_SOCKET)
         {
            closesocket(socket_handle);
            socket_handle = INVALID_SOCKET;
         }
      } // destructor

      
      void SocketUdpServer::send_datagram(
         Packet const &packet,
         SocketAddress const &address)
      {
         int rcd;
         Csi::OStrAscStream temp;
         temp << "sending UDP to " << address;
         trace(temp.c_str());
         rcd = sendto(
            socket_handle,
            packet.getMsg(),
            packet.length(),
            0,                  // flags
            address.get_storage(),
            address.get_address_len());
         if(rcd == SOCKET_ERROR)
            throw WinsockException("send_datagram failure");
      } // send_datagram

      
      void SocketUdpServer::on_socket_event(
         int error_code,
         int event_id)
      {
         if(event_id == FD_READ && error_code == 0)
         {
            // call recvfrom() to get the next datagram.  According to the documentation, this
            // should set up the sockets layer so that another notification will be posted when new
            // data is available
            struct sockaddr_storage address;
            int address_size = sizeof(address);
            int rcd;

            rcd = recvfrom(
               socket_handle,
               read_buffer,
               max_datagram_size,
               0,
               reinterpret_cast<struct sockaddr *>(&address),
               &address_size);
            if(rcd > 0)
            {
               try
               {
                  Packet packet(read_buffer, rcd, 0);
                  on_datagram(
                     packet,
                     SocketAddress(
                        reinterpret_cast<struct sockaddr *>(&address),
                        address_size));
               }
               catch(std::exception &)
               { } 
            }
         }
      } // on_socket_event
   };
};

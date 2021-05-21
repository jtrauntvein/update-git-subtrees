/* Csi.Win32.SocketUdpServer.h

   Copyright (C) 2004, 2013 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 26 February 2004
   Last Change: Friday 15 March 2013
   Last Commit: $Date: 2013-03-19 10:46:27 -0600 (Tue, 19 Mar 2013) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Csi_Win32_SocketUdpServer_h
#define Csi_Win32_SocketUdpServer_h

#include "Csi.Win32.WinSockBase.h"
#include "Csi.SocketAddress.h"
#include "Packet.h"


namespace Csi
{
   namespace Win32
   {
      ////////////////////////////////////////////////////////////
      // class SocketUdpServer
      //
      // Defines an object that is able to wait for UDP packets to arrive on a
      // specified UDP port and, when they arrive, invokes a pure virtual
      // method to inform the application that a datagram has arrived.  
      ////////////////////////////////////////////////////////////
      class SocketUdpServer: public WinSockBase
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         //
         // This constructor will create the socket and bind it to the
         // speciifed port.  If the socket initialisation fails or if the bind
         // operation fails, an object derived from std::exception will be
         // thrown.  Once the constructor has completed, the object will be in
         // a state where it is waiting for incoming datagrams.
         ////////////////////////////////////////////////////////////
         SocketUdpServer(
            SocketAddress const &bind_address_, uint4 max_datagram_size = 2048);

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~SocketUdpServer();

         ////////////////////////////////////////////////////////////
         // get_peer_address
         ////////////////////////////////////////////////////////////
         bool get_peer_address(
            StrAsc &peer_address,
            uint2 &peer_port)
         { return false; }

         ////////////////////////////////////////////////////////////
         // send_datagram
         ////////////////////////////////////////////////////////////
         void send_datagram(
            Packet const &packet,
            SocketAddress const &address);
         
      protected:
         ////////////////////////////////////////////////////////////
         // on_datagram
         ////////////////////////////////////////////////////////////
         virtual void on_datagram(
            Packet &packet,
            SocketAddress const &address) = 0;

         ////////////////////////////////////////////////////////////
         // on_socket_event
         ////////////////////////////////////////////////////////////
         virtual void on_socket_event(
            int error_code,
            int event_id);

      protected:
         ////////////////////////////////////////////////////////////
         // max_datagram_size
         ////////////////////////////////////////////////////////////
         uint4 max_datagram_size;

         ////////////////////////////////////////////////////////////
         // read_buffer
         //
         // This buffer is allocated in the constructor to be the same size as
         // max_datagram_size.  It is used to read the datagram from the
         // socket.  It will be deleted by the destructor.
         ////////////////////////////////////////////////////////////
         char *read_buffer;

         ////////////////////////////////////////////////////////////
         // bind_address
         //
         // Specifies the interface address to which this socket will be bound.
         ////////////////////////////////////////////////////////////
         SocketAddress bind_address;
      };


      typedef SocketUdpServer WinSockUdpServer;
   };
};


#endif

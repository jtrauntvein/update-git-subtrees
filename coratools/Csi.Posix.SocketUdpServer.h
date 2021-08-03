/* Csi.Posix.SocketUdpServer.h

   Copyright (C) 2005, 2013 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 21 September 2005
   Last Change: Thursday 04 April 2013
   Last Commit: $Date: 2013-04-04 16:56:04 -0600 (Thu, 04 Apr 2013) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Csi_Posix_SocketUdpServer_h
#define Csi_Posix_SocketUdpServer_h

#include "Csi.Posix.SocketBase.h"
#include "Csi.SocketAddress.h"
#include "Csi.Protector.h"
#include "Csi.Thread.h"
#include "Packet.h"
#include <list>


namespace Csi
{
   namespace Posix
   {
      namespace SocketUdpServerHelpers
      {
         ////////////////////////////////////////////////////////////
         // class datagram_type
         ////////////////////////////////////////////////////////////
         class datagram_type
         {
         public:
            ////////////////////////////////////////////////////////////
            // content
            ////////////////////////////////////////////////////////////
            Packet content;

            ////////////////////////////////////////////////////////////
            // address
            ////////////////////////////////////////////////////////////
            SocketAddress address;

            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            datagram_type(
               Packet const &content_, SocketAddress const &address_):
               content(content_),
               address(address_)
            { }

            ////////////////////////////////////////////////////////////
            // default constructor
            ////////////////////////////////////////////////////////////
            datagram_type():
               content(0)
            { }

            ////////////////////////////////////////////////////////////
            // copy constructor
            ////////////////////////////////////////////////////////////
            datagram_type(datagram_type const &other):
               content(other.content),
               address(other.address)
            { }

            ////////////////////////////////////////////////////////////
            // copy operator
            ////////////////////////////////////////////////////////////
            datagram_type &operator =(datagram_type const &other)
            {
               content = other.content;
               address = other.address;
               return *this;
            }
         };
      };

      
      ////////////////////////////////////////////////////////////
      // class SocketUdpServer
      //
      // Defines an object that is able to wait for UDP packets to arrive on a
      // specified UDP port and, when they arrive, invokes a pure virtual
      // method to inform the application that a datagram has arrived.  Also
      // provides a method that allows the application to transmit datagrams as
      // well. 
      ////////////////////////////////////////////////////////////
      class SocketUdpServer:
         public SocketBase,
         private Thread
      {
      public:
         ////////////////////////////////////////////////////////////
         // SocketUdpServer
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
            Packet &packet, SocketAddress const &address);

      protected:
         ////////////////////////////////////////////////////////////
         // on_datagram
         ////////////////////////////////////////////////////////////
         virtual void on_datagram(Packet &packet, SocketAddress const &address) = 0;


         ////////////////////////////////////////////////////////////
         // execute
         ////////////////////////////////////////////////////////////
         virtual void execute();

         ////////////////////////////////////////////////////////////
         // wait_for_end
         ////////////////////////////////////////////////////////////
         virtual void wait_for_end();

         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(SharedPtr<Event> &ev);

      protected:
         ////////////////////////////////////////////////////////////
         // udp_port
         ////////////////////////////////////////////////////////////
         uint2 udp_port;

         ////////////////////////////////////////////////////////////
         // max_datagram_size
         ////////////////////////////////////////////////////////////
         uint4 max_datagram_size;

         ////////////////////////////////////////////////////////////
         // should_quit
         ////////////////////////////////////////////////////////////
         bool should_quit;

         ////////////////////////////////////////////////////////////
         // tx_queue
         //
         // Holds pending datagrams that the thread can send. 
         ////////////////////////////////////////////////////////////
         typedef SocketUdpServerHelpers::datagram_type datagram_type;
         typedef std::list<datagram_type> datagram_list;
         typedef Protector<datagram_list> tx_queue_type;
         tx_queue_type tx_queue;

         ////////////////////////////////////////////////////////////
         // bind_address
         //
         // Specifies the address of the interface to which this socket will be bound. 
         ////////////////////////////////////////////////////////////
         SocketAddress bind_address;
      };
   };
};


#endif

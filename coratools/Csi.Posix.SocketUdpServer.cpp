/* Csi.Posix.SocketUdpServer.cpp

   Copyright (C) 2005, 2017 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 21 September 2005
   Last Change: Thursday 02 November 2017
   Last Commit: $Date: 2017-11-02 16:48:23 -0600 (Thu, 02 Nov 2017) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Posix.SocketUdpServer.h"
#include "Csi.Posix.SocketException.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>


namespace Csi
{
   namespace Posix
   {
      namespace
      {
         ////////////////////////////////////////////////////////////
         // class event_datagram_received
         ////////////////////////////////////////////////////////////
         class event_datagram_received: public Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // datagram
            ////////////////////////////////////////////////////////////
            SocketUdpServerHelpers::datagram_type datagram;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               SocketUdpServer *receiver,
               void const *buff,
               size_t buff_len,
               SocketAddress const &address)
            {
               event_datagram_received *event = new event_datagram_received(receiver, buff, buff_len, address);
               event->post();
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_datagram_received(
               SocketUdpServer *receiver,
               void const *buff,
               size_t buff_len,
               SocketAddress const &address):
               Event(event_id,receiver),
               datagram(Packet(buff, buff_len, 0), address)
            { } 
         };


         uint4 const event_datagram_received::event_id =
            Event::registerType("Csi::Posix::SocketUdpServer::event_datagram_received");
      };

      
      ////////////////////////////////////////////////////////////
      // class SocketUdpServer definitions
      ////////////////////////////////////////////////////////////
      SocketUdpServer::SocketUdpServer(
         SocketAddress const &bind_address_, uint4 max_datagram_size_):
         max_datagram_size(max_datagram_size_),
         should_quit(false),
         bind_address(bind_address_)
      {
         // ordinarily, this work would be done in the thread.  However, in order to maintain
         // compatible semantics with the win32 implementation, we will do the work of opening and
         // binding in the constructor before the thread is started.  This was, a failure can be
         // reported by throwing an exception.
         int rcd;
         
         socket_handle = ::socket(bind_address.get_family(), SOCK_DGRAM, 0);
         if(socket_handle == -1)
            throw SocketException("socket alloc failure");
         rcd = ::bind(socket_handle, bind_address.get_storage(), bind_address.get_address_len());
         if(rcd != 0)
         {
            ::close(socket_handle);
            socket_handle = -1;
            throw SocketException("failure to bind address");
         }

         // we may want to broadcast on this socket so we will enable that option
         int broadcast_enable(1);
         ::setsockopt(socket_handle, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable));

         // having made it this far, we can now start the thread to handle socket events.
         if(max_datagram_size < 1024)
            max_datagram_size = 1024;
         Thread::start();
      } // constructor

      
      SocketUdpServer::~SocketUdpServer()
      {
         wait_for_end();
      } // destructor

      
      void SocketUdpServer::send_datagram(Packet &packet, SocketAddress const &address)
      {
         tx_queue_type::key_type key(tx_queue);
         key->push_back(datagram_type(packet, address));
      } // send_datagram

      
      void SocketUdpServer::execute()
      {
         // before the thread begins, we will allocate the variables that we need to do the work.
         // This will include the creation of a buffer that can be used for the reception of
         // datagrams. 
         byte *rx_buff = new byte[max_datagram_size];
         fd_set wait_list;
         struct timeval timeout;
         int rcd;
         socklen_t len;
         
         try
         {
            while(!should_quit)
            {
               // if there are any pending datagrams to send, no is the time to send them
               tx_queue_type::key_type tx_key(tx_queue);
               while(!tx_key->empty())
               {
                  datagram_type &datagram = tx_key->front();
                  len = ::sendto(
                     socket_handle,
                     datagram.content.getMsg(),
                     datagram.content.length(),
                     0,         // flags
                     datagram.address.get_storage(),
                     datagram.address.get_address_len());
                  if(len == -1)
                     throw SocketException("tx datagram error");
                  tx_key->pop_front();
               }
               tx_key.release();
               
               // most of our time in this thread will be spent waiting for incoming datagrams.  
               FD_ZERO(&wait_list);
               FD_SET(socket_handle,&wait_list);
               timeout.tv_sec = 0;
               timeout.tv_usec = 100000; // 100 milli-seconds delay
               rcd = ::select(
                  socket_handle + 1,
                  &wait_list,
                  0,            // no write
                  0,            // no error
                  &timeout);
               if(rcd > 0)
               {
                  if(FD_ISSET(socket_handle,&wait_list))
                  {
                     struct sockaddr_storage address;
                     socklen_t address_size = sizeof(address);
                     len = ::recvfrom(
                        socket_handle,
                        rx_buff,
                        max_datagram_size,
                        0,      // no flags
                        reinterpret_cast<struct sockaddr *>(&address),
                        &address_size);
                     if(len > 0)
                     {
                        event_datagram_received::cpost(
                           this,
                           rx_buff,
                           len,
                           SocketAddress(
                              reinterpret_cast<struct sockaddr *>(&address), address_size));
                     }
                  }
               }
               else if(rcd == -1 && errno == EINTR)
                  throw SocketException("select failure");
            }
         }
         catch(SocketException &e)
         {
            post_socket_error(e.get_socket_error());
         }
         delete[] rx_buff;
      } // execute


      void SocketUdpServer::wait_for_end()
      {
         should_quit = true;
         Thread::wait_for_end();
      } // wait_for_end


      void SocketUdpServer::receive(SharedPtr<Event> &ev)
      {
         if(ev->getType() == event_datagram_received::event_id)
         {
            event_datagram_received *event = static_cast<event_datagram_received *>(ev.get_rep());
            on_datagram(event->datagram.content, event->datagram.address);
         }
         else
            SocketBase::receive(ev);
      } // receive
   };
};


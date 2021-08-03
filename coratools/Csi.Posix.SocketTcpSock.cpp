/* Csi.Posix.SocketTcpSock.cpp

   Copyright (C) 2005, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 30 August 2005
   Last Change: Saturday 28 March 2020
   Last Commit: $Date: 2020-11-16 17:23:52 -0600 (Mon, 16 Nov 2020) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Posix.SocketTcpSock.h"
#include "Csi.Posix.SocketException.h"
#include "Csi.SocketBase.h"
#include "Csi.TlsContext.h"
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <unistd.h>


namespace Csi
{
   namespace Posix
   {
      namespace
      {
         ////////////////////////////////////////////////////////////
         // class read_event
         ////////////////////////////////////////////////////////////
         class read_event: public Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(SocketTcpSock *receiver)
            {
               read_event *event = new read_event(receiver);
               try { event->post(); }
               catch(Event::BadPost &) { delete event; }
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            read_event(SocketTcpSock *receiver):
               Event(event_id,receiver)
            { }
         };


         uint4 const read_event::event_id =
            Event::registerType("Csi::Posix::SocketTcpSock::read_event");


         ////////////////////////////////////////////////////////////
         // class connected_event
         ////////////////////////////////////////////////////////////
         class connected_event: public Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // connected_address
            ////////////////////////////////////////////////////////////
            SocketAddress connected_address;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               SocketTcpSock *receiver,
               SocketAddress const &connected_address)
            {
               connected_event *event = new connected_event(
                  receiver, connected_address);
               event->post();
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            connected_event(
               SocketTcpSock *receiver,
               SocketAddress const &connected_address_):
               Event(event_id,receiver),
               connected_address(connected_address_)
            { }
         };


         uint4 const connected_event::event_id =
            Event::registerType("Csi::Posix::SocketTcpSock::connected_event");


         ////////////////////////////////////////////////////////////
         // class closed_event
         ////////////////////////////////////////////////////////////
         class closed_event: public Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(SocketTcpSock *receiver)
            {
               closed_event *event = new closed_event(receiver);
               try { event->post(); }
               catch(Event::BadPost &) { }
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            closed_event(SocketTcpSock *receiver):
               Event(event_id,receiver)
            { }
         };


         uint4 const closed_event::event_id =
            Csi::Event::registerType("Csi::Posix::SocketTcpSock::closed_event");


         ////////////////////////////////////////////////////////////
         // tls_context
         ////////////////////////////////////////////////////////////
         TlsContext *tls_context(0);
      };

      
      ////////////////////////////////////////////////////////////
      // class SocketTcpSock definitions
      ////////////////////////////////////////////////////////////
      SocketTcpSock::SocketTcpSock(
         int socket_handle,
         bool using_tls_,
         bool is_server_):
         SocketBase(socket_handle),
         should_quit(false),
         client_port(0),
         is_connected(false),
         read_buffer(100, true),
         write_buffer(100, true),
         using_tls(using_tls_),
         is_server(is_server_),
         can_send(false)
      {
         if(socket_handle != -1)
         {
            start();
            if(using_tls)
            {
               if(tls_context)
                  tls_context->add_connection(this, !is_server);
               else
                  throw std::invalid_argument("cannot start TLS without a valid context");
            }
         }
      } // constructor


      SocketTcpSock::~SocketTcpSock()
      {
         close();
      } // destructor


      void SocketTcpSock::open(
         char const *address,
         uint2 port)
      {
         close();
         using_tls = false;
         should_quit = false;
         client_address = address;
         client_port = port;
         resolved_address.clear();
         is_server = false;
         can_send = false;
         start();
      } // open


      void SocketTcpSock::open(
         uint4 resolved_address_,
         uint2 port)
      {
         StrAsc address(address_to_str(resolved_address_));
         open(address.c_str(), port);
      } // open


      void SocketTcpSock::open(int new_connection)
      {
         close();
         socket_handle = new_connection;
         should_quit = false;
         using_tls = false;
         is_server = true;
         can_send = false;
         start();
      } // open


      void SocketTcpSock::close()
      {
         if(using_tls)
            tls_context->remove_connection(this);
         wait_for_end();
         is_connected = false;
         SocketBase::close();
         is_connected = false;
         can_send = false;
         read_buffer.pop(read_buffer.size());
         write_buffer.pop(write_buffer.size());
      } // close

      
      void SocketTcpSock::write(
         void const *buff,
         uint4 buff_len)
      {
         write_buffer.push(buff, buff_len);
         signal_urgent();
         if(using_tls)
            tls_context->on_socket_data(this, 0, 0);
      } // write


      void SocketTcpSock::on_low_level_read(
         void const *buff,
         uint4 buff_len)
      { read_buffer.push(buff, buff_len); }

      
      void SocketTcpSock::receive(SharedPtr<Event> &ev)
      {
         if(ev->getType() == read_event::event_id)
            on_read();
         else if(ev->getType() == connected_event::event_id)
         {
            connected_event *event = static_cast<connected_event *>(ev.get_rep()); 
            on_connected(event->connected_address);
         }
         else if(ev->getType() == closed_event::event_id)
            on_close();
         else
            SocketBase::receive(ev);
      } // receive


      void SocketTcpSock::set_tls_context(TlsContext *context)
      {
         if(tls_context != 0)
            delete tls_context;
         tls_context = context;
      } // set_tls_context


      TlsContext *SocketTcpSock::get_tls_context()
      { return tls_context; }


      void SocketTcpSock::start_tls_client()
      {
         if(!is_connected)
            throw std::invalid_argument("attempt to start TLS without a connection");
         if(!using_tls)
         {
            if(tls_context == 0)
               throw std::invalid_argument("the TLS context is not initialised");
            using_tls = true;
            tls_context->add_connection(this, true);
         }
         else
            throw std::invalid_argument("TLS has already been started");
      } // start_tls_client


      void SocketTcpSock::start_tls_server(
         void const *transmit_before, uint4 before_len)
      {
         if(!is_connected)
            throw std::invalid_argument("attempt to start TLS without a connection");
         if(!using_tls)
         {
            if(tls_context == 0)
               throw std::invalid_argument("the TLS context is not initialised");
            using_tls = true;
            tls_context->add_connection(this, false, transmit_before, before_len);
         }
         else
            throw std::invalid_argument("TLS already started");
      } // start_tls_server


      void SocketTcpSock::on_closing_tls()
      {
         using_tls = false;
      } // on_closing_tls 


      uint4 SocketTcpSock::get_tx_buff_size()
      {
         uint4 rtn(0);
         ByteQueue *buffer(&write_buffer);
         if(using_tls)
            buffer = tls_context->get_socket_write_buffer(this);
         if(buffer)
            rtn = buffer->size();
         return rtn;
      } // get_tx_buff_size
      
      void SocketTcpSock::execute()
      {
         try
         {
            // if the socket handle has not been opened (the handle was passed to the constructor),
            // we will need to allocate, bind, and connect the socket.
            if(socket_handle == -1)
            {
               // we will need to resolve the client address and try each address that was
               // resolved.
               SocketAddress::addresses_type addresses;
               SocketAddress::resolve(addresses, client_address.c_str(), client_port, false);
               if(addresses.empty())
                  throw SocketException("address resolution failure");

               // we now need to attempt to open each of the resolved addresses.
               while(!addresses.empty() && !is_connected)
               {
                  try
                  {
                     // create the socket
                     SocketAddress address(addresses.front());
                     int rcd(0);
                     
                     addresses.pop_front();
                     socket_handle = ::socket(address.get_family(), SOCK_STREAM, 0);
                     if(socket_handle == -1)
                     {
                        if(addresses.empty())
                           throw SocketException("socket create failure");
                        continue;
                     }
                     
                     // we now need to attempt to connect the socket
                     rcd = ::connect(socket_handle, address.get_storage(), address.get_address_len());
                     resolved_address = address;
                     if(rcd == 0)
                        is_connected = true;
                     else
                     {
                        // if we were unblocked by an interrupt, we will wait until the socket is
                        // connected or a failure occurs
                        if(errno != EINTR)
                           throw SocketException("socket connect failure");
                        while(errno == EINTR && !should_quit && !is_connected)
                        {
                           fd_set read_set, error_set;
                           FD_ZERO(&read_set);
                           FD_ZERO(&error_set);
                           FD_SET(socket_handle,&read_set);
                           FD_SET(socket_handle,&error_set);
                           rcd = select(
                              socket_handle + 1,
                              &read_set,
                              0,      // no write set
                              &error_set,
                              0);     // no time out
                           if(rcd > 0)
                           {
                              if(FD_ISSET(socket_handle,&read_set))
                                 is_connected = true;
                              else if(addresses.empty())
                                 throw SocketException("connect error");
                              else
                                 break;
                           }
                           else if(errno != EINTR)
                              throw SocketException("connect error");
                        }
                     }
                  }
                  catch(SocketException &)
                  {
                     if(socket_handle != -1)
                        ::close(socket_handle);
                     socket_handle = -1;
                     if(addresses.empty())
                        throw;
                  }
                  catch(OsException &)
                  {
                     if(socket_handle != -1)
                        ::close(socket_handle);
                     socket_handle = -1;
                     if(addresses.empty())
                        throw;
                  }
                  catch(std::exception &)
                  {
                     if(socket_handle != -1)
                        ::close(socket_handle);
                     socket_handle = -1;
                     if(addresses.empty())
                        throw;
                  }
               }
            }
            else
            {
               // we'll get the peer name for the connection to report as the resolved address. 
               struct sockaddr_storage storage;
               struct sockaddr *address(reinterpret_cast<struct sockaddr *>(&storage));
               socklen_t address_len(sizeof(storage));
               
               int rcd(::getpeername(socket_handle, address, &address_len));
               if(rcd == 0)
                  resolved_address = SocketAddress(address, address_len);
               is_connected = true;
            }
            connected_event::cpost(this, resolved_address);

            // now a connection has been established, we will enter a loop where we will wait for
            // read and write events for the socket.
            char tx_buff[1024];
            char rx_buff[1024];
            struct pollfd poll_descriptor;
            int rcd;

            can_send = true;
            poll_descriptor.fd = socket_handle;
            poll_descriptor.events = POLLIN | POLLOUT;
            poll_descriptor.revents = 0;
            while(!should_quit && is_connected)
            {
               // if we know that data can be written, we need to do this before we poll the OS for
               // events.  This will prevent the 100 msec latency that would otherwise occur
               ByteQueue *buffer = &write_buffer;
               fill_write_buffer(write_buffer);
               if(using_tls)
               {
                  buffer = tls_context->get_socket_write_buffer(this);
                  if(buffer == 0)
                     throw std::invalid_argument("TLS write buffer no longer valid");
               }
               while(can_send && !should_quit && !buffer->empty())
               {
                  uint4 tx_len = buffer->copy(tx_buff, sizeof(tx_buff));
                  rcd = ::send(socket_handle, tx_buff, tx_len, MSG_NOSIGNAL);
                  if(rcd == -1 && errno != EINTR)
                     throw SocketException("socket write failure");
                  buffer->pop(rcd);
                  if(rcd > 0 && !using_tls)
                     on_low_level_write(tx_buff, rcd);
                  if(rcd < tx_len)
                     can_send = false;
               }
               
               // we will use poll for up to 100 msec to determine whether the socket can be read or
               // written
               if(!can_send)
                  poll_descriptor.events = POLLIN | POLLOUT;
               else
                  poll_descriptor.events = POLLIN;
               poll(&poll_descriptor, 1, 100);
               
               // check to see if an error has occurred
               if((poll_descriptor.revents & POLLERR) != 0 ||
                  (poll_descriptor.revents & POLLHUP) != 0 ||
                  (poll_descriptor.revents & POLLNVAL) != 0)
                  throw SocketException("socket hung up or socket error");

               // check to see if anything can be written
               if((poll_descriptor.revents & POLLOUT) != 0)
                  can_send = true;

               // check to see if anything can be read
               if((poll_descriptor.revents & POLLIN) != 0)
               {
                  ssize_t bytes_read;
                  ssize_t total_bytes_read = 0;
                  int bytes_remaining = 0;
                  do
                  {
                     bytes_read = ::recv(socket_handle, rx_buff, sizeof(rx_buff), 0);
                     if(bytes_read > 0)
                     {
                        total_bytes_read += bytes_read;
                        if(!using_tls)
                           on_low_level_read(rx_buff,bytes_read);
                        else
                           tls_context->on_socket_data(this, rx_buff, bytes_read);
                     }
                     else if(bytes_read == -1)
                        throw SocketException("read failure");
                     ioctl(
                        socket_handle,
                        FIONREAD,
                        &bytes_remaining);
                  }
                  while(bytes_remaining != 0);
                  
                  // recv() will return 0 if the socket has been closed
                  if(total_bytes_read > 0)
                     read_event::cpost(this);
                  else
                  {
                     is_connected = false;
                     closed_event::cpost(this);
                  }
               }
               
            }
         }
         catch(SocketException &e)
         {
            ::close(socket_handle);
            socket_handle = -1;
            is_connected = false;
            post_socket_error(e.get_socket_error());
         }
         catch(Csi::OsException &e)
         {
            ::close(socket_handle);
            socket_handle = -1;
            is_connected = false;
            post_socket_error(e.get_osError());
         }
         catch(std::exception &e)
         {
            ::close(socket_handle);
            socket_handle = -1;
            is_connected = false;
            post_socket_error(0);
         }
      } // execute


      void SocketTcpSock::wait_for_end()
      {
         should_quit = true;
         Thread::wait_for_end();
      } // wait_for_end
   };
};


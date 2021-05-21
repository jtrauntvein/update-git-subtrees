/* Csi.Posix.SocketTcpService.cpp

   Copyright (C) 2005, 2014 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 24 August 2005
   Last Change: Friday 29 August 2014
   Last Commit: $Date: 2014-09-02 08:44:49 -0600 (Tue, 02 Sep 2014) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.SocketTcpService.h"
#include "Csi.SocketException.h"
#include "Csi.SocketAddress.h"
#include "Csi.StrAscStream.h"
#include "trace.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>


namespace Csi
{
   uint4 const SocketServiceLogEvent::event_id(
      Event::registerType("Cora::Win32::WinsockService::LoggedEvent"));

   
   namespace Posix
   {
      namespace
      {
         ////////////////////////////////////////////////////////////
         // class accept_event
         ////////////////////////////////////////////////////////////
         class accept_event: public Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // socket_handle
            ////////////////////////////////////////////////////////////
            int socket_handle;

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            accept_event(
               SocketTcpService *service,
               int socket_handle_):
               socket_handle(socket_handle_),
               Event(event_id,service)
            { }

         public:
            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               SocketTcpService *service,
               int socket_handle)
            {
               accept_event *event = new accept_event(service,socket_handle);
               try { event->post(); }
               catch(Event::BadPost &) { delete event; }
            }
         };


         uint4 const accept_event::event_id =
         Event::registerType("Csi::Posix::SocketTcpService::accept_event");
      };


      ////////////////////////////////////////////////////////////
      // class SocketTcpService definitions
      ////////////////////////////////////////////////////////////
      SocketTcpService::SocketTcpService():
         should_stop(0),
         service_port(0),
         logger(0),
         allow_ipv6(true)
      { }

      
      void SocketTcpService::start_service(
         uint2 service_port_, bool local_only_)
      {
         // if the service is currently running, we will stop it before attempting to restart it
         // using the new parameters.
         wait_for_end();

         // now we can set the new parameters and start the service.
         service_port = service_port_;
         local_only = local_only_;
         should_stop = false;
         started_condition.reset();
         Thread::start();
         started_condition.wait();
      } // start_service

      
      void SocketTcpService::stop_service()
      { wait_for_end(); }


      void SocketTcpService::set_logger(EventReceiver *logger_)
      { logger = logger_; }


      void SocketTcpService::set_allow_ipv6(bool enabled)
      { allow_ipv6 = enabled; }

      
      void SocketTcpService::receive(SharedPtr<Event> &ev)
      {
         if(ev->getType() == accept_event::event_id)
         {
            accept_event *event = static_cast<accept_event *>(ev.get_rep());
            on_accept(event->socket_handle);
         }
         else if(ev->getType() == SocketHelpers::error_event::event_id)
            wait_for_end();
         SocketBase::receive(ev);
      } // receive

      
      void SocketTcpService::wait_for_end()
      {
         if(is_started)
         {
            should_stop = true;
            Thread::wait_for_end();
         }
      } // wait_for_end

      
      void SocketTcpService::execute()
      {
         typedef std::list<int> sockets_type;
         typedef SocketServiceLogEvent log_event_type;
         sockets_type sockets;
         OStrAscStream log_str;
         try
         {
            // we are going to listen on IPv4 as well as IPv6 sockets.  We will create a socket for
            // each protocol.
            SocketAddress::addresses_type addresses;
            SocketAddress::addresses_type temp;
            last_error.cut(0);
            listening_addresses.clear();
            if(local_only)
            {
               if(allow_ipv6)
               {
                  try
                  {
                     SocketAddress::resolve(temp, "::1", service_port, true);
                     while(!temp.empty())
                     {
                        SocketAddress &address(temp.front());
                        if(address.get_family() == SocketAddress::family_ipv6)
                           addresses.push_back(address);
                        temp.pop_front();
                     }
                  }
                  catch(std::exception &)
                  { }
               }
               try
               {
                  SocketAddress::resolve(temp, "127.0.0.1", service_port, true);
                  while(!temp.empty())
                  {
                     SocketAddress address(temp.front());
                     if(address.get_family() == SocketAddress::family_ipv4)
                        addresses.push_back(address);
                     temp.pop_front();
                  }
               }
               catch(std::exception &)
               { }
            }
            else
            {
               if(allow_ipv6)
               {
                  try
                  {
                     SocketAddress::resolve(temp, "::0", service_port, true);
                     while(!temp.empty())
                     {
                        SocketAddress address(temp.front());
                        if(address.get_family() == SocketAddress::family_ipv6)
                           addresses.push_back(address);
                        temp.pop_front();
                     }
                  }
                  catch(std::exception &)
                  { }
               }
               try
               {
                  SocketAddress::resolve(temp, "0.0.0.0", service_port, true);
                  while(!temp.empty())
                  {
                     SocketAddress address(temp.front());
                     if(address.get_family() == SocketAddress::family_ipv4)
                        addresses.push_back(address);
                     temp.pop_front();
                  }
               }
               catch(std::exception &)
               { }
            }

            // we now need to create sockets for each of these addresses
            int rcd;
            while(!addresses.empty())
            {
               SocketAddress address(addresses.front());
               int socket_handle;

               if(logger)
               {
                  log_str.str("");
                  log_str << "listening on address\",\"" << address;
                  log_event_type::cpost(logger, log_str.str());
               }
               addresses.pop_front();
               socket_handle = ::socket(address.get_family(), SOCK_STREAM, 0);
               if(socket_handle >= 0)
               {
                  // we need to ensure that IPv6 sockets only accept IPv6 connections
                  sockets.push_back(socket_handle);
                  if(address.get_family() == SocketAddress::family_ipv6)
                  {
                     int option_value(1);
                     setsockopt(
                        socket_handle,
                        IPPROTO_IPV6,
                        IPV6_V6ONLY,
                        (char *)&option_value,
                        sizeof(option_value));
                  }
                  if(logger)
                  {
                     log_str.str("");
                     log_str << "socket opened\",\"" << address << "\",\"" << socket_handle;
                     log_event_type::cpost(logger, log_str.str());
                  }

                  // If this service was offered previously and there is a client connection in a
                  // time_wait state (or the client connection is still valid) the attempt to bind
                  // the socket may well fail.  We can overcome this by using the SO_REUSEADDR
                  // socket option.
                  int reuseaddr_option_val = 1;
                  rcd = ::setsockopt(
                     socket_handle,
                     SOL_SOCKET,
                     SO_REUSEADDR,
                     &reuseaddr_option_val,
                     sizeof(reuseaddr_option_val));
                  if(rcd != 0)
                     throw SocketException("failed to set SO_REUSEADDR");

                  // we need to bind the socket to the address
                  rcd = ::bind(socket_handle, address.get_storage(), address.get_address_len());
                  if(rcd != 0)
                     throw SocketException("Failure to bind address");

                  // if no service port was specified, we will need to get the socket address to
                  // determine what port was used.
                  if(service_port == 0)
                  {
                     struct sockaddr_storage storage;
                     struct sockaddr *address(reinterpret_cast<struct sockaddr *>(&storage));
                     socklen_t address_len(sizeof(storage));
                     rcd = ::getsockname(socket_handle, address, &address_len);
                     if(rcd == 0)
                     {
                        SocketAddress temp(address, address_len);
                        service_port = temp.get_port();
                        for(SocketAddress::addresses_type::iterator ai = addresses.begin();
                            ai != addresses.end();
                            ++ai)
                           ai->set_port(service_port);
                     }
                  }

                  // we need to listen for incoming connections
                  rcd = ::listen(socket_handle, 5);
                  if(rcd != 0)
                     throw SocketException("Failure to start listen on service");
                  if(logger)
                  {
                     log_str.str("");
                     log_str << "listen succeeded\",\"" << address << "\",\"" << socket_handle;
                     log_event_type::cpost(logger, log_str.str());
                  }
                  listening_addresses.push_back(address);
               }
            }

            // if there were no sockets created, we will need to throw an exception
            if(sockets.empty())
               throw SocketException("no interfaces");

            // we are now ready to start waiting for connection notifications
            started_condition.set();
            while(!should_stop)
            {
               fd_set wait_list;
               struct timeval timeout = { 0, 100000 };
               int largest_socket = -1;
               int rcd(0);
               
               FD_ZERO(&wait_list);
               for(sockets_type::iterator si = sockets.begin(); si != sockets.end(); ++si)
               {
                  FD_SET(*si, &wait_list);
                  if(*si > largest_socket)
                     largest_socket = *si;
               }
               rcd = ::select(largest_socket + 1, &wait_list, 0, 0, &timeout);
               if(rcd > 0)
               {
                  for(sockets_type::iterator si = sockets.begin(); si != sockets.end(); ++si)
                  {
                     if(FD_ISSET(*si, &wait_list))
                     {
                        struct sockaddr_storage storage;
                        struct sockaddr *address(reinterpret_cast<struct sockaddr *>(&storage));
                        socklen_t address_len(sizeof(storage));
                        int new_socket(::accept(*si, address, &address_len));
                        if(new_socket != -1)
                           accept_event::cpost(this, new_socket);
                     }
                  }
               }
               else if(rcd == -1 && errno != EINTR)
                  throw SocketException("select failure");
            }
            
         }
         catch(SocketException &e)
         {
            log_str.str("");
            log_str << "error listening on socket\",\"" << e.what();
            if(logger)
               log_event_type::cpost(logger, log_str.str());
            last_error = log_str.str();
            post_socket_error(e.get_socket_error());
         }
         catch(std::exception &e)
         {
            OStrAscStream temp;
            temp << "error listening on socket: " << e.what();
            last_error = temp.str();
            post_socket_error(0);
         }

         // we need to ensure that all sockets are closed
         while(!sockets.empty())
         {
            ::close(sockets.front());
            sockets.pop_front();
         }
         started_condition.set();
      } // execute
   };
};


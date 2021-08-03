/* Csi.Win32.WinSockService.cpp

   Copyright (C) 2001, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 22 February 2001
   Last Change: Tuesday 11 February 2020
   Last Commit: $Date: 2020-02-11 13:43:47 -0600 (Tue, 11 Feb 2020) $ 
   Commited by: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.SocketTcpService.h"
#include "Csi.Win32.WinsockException.h"
#include "Csi.StrAscStream.h"
#include <ws2tcpip.h>


namespace Csi
{
   uint4 const SocketServiceLogEvent::event_id(
      Event::registerType("Cora::Win32::WinsockService::LoggedEvent"));

   
   namespace Win32
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
            // new_connection
            ////////////////////////////////////////////////////////////
            SOCKET new_connection;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(WinSockService *service, SOCKET new_connection)
            {
               accept_event *event(new accept_event(service, new_connection));
               event->post();
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            accept_event(WinSockService *service, SOCKET new_connection_):
               Event(event_id, service),
               new_connection(new_connection_)
            { }
         };


         uint4 const accept_event::event_id(
            Event::registerType("Csi::Win32::WinsockService::accept_event"));


         ////////////////////////////////////////////////////////////
         // class error_event
         ////////////////////////////////////////////////////////////
         class error_event: public Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // error
            ////////////////////////////////////////////////////////////
            WinsockException error;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(WinSockService *service, WinsockException const &error)
            {
               error_event *event(new error_event(service, error));
               event->post();
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            error_event(WinSockService *service, WinsockException const &error_):
               Event(event_id, service),
               error(error_)
            { }
         };


         uint4 const error_event::event_id(
            Event::registerType("Csi::Win32:::WinsockService::error_event"));
      };

      
      ////////////////////////////////////////////////////////////
      // class WinSockService definitions
      ////////////////////////////////////////////////////////////
      WinSockService::WinSockService():
         service_port(0),
         local_only(false),
         should_close(false),
         logger(0)
      { }


      void WinSockService::start_service(uint2 service_port_, bool local_only_)
      {
         // make sure that the current socket is closed
         stop_service();
         service_port = service_port_;
         local_only = local_only_;
         should_close = false;
         start_condition.reset();
         Thread::start();
         start_condition.wait();
      } // start_service


      void WinSockService::stop_service()
      {
         should_close = true;
         Thread::wait_for_end();
      } // stop_service
   

      void WinSockService::receive(SharedPtr<Event> &ev)
      {
         if(ev->getType() == accept_event::event_id)
         {
            accept_event *event(static_cast<accept_event *>(ev.get_rep()));
            on_accept(event->new_connection);
         }
         else if(ev->getType() == error_event::event_id)
         {
            error_event *event(static_cast<error_event *>(ev.get_rep()));
            stop_service();
            on_socket_error(event->error.get_socket_error());
         }
      } // receive


      void WinSockService::execute()
      {
         typedef std::list<SOCKET> sockets_type;
         typedef SocketServiceLogEvent log_event_type;
         sockets_type sockets;
         OStrAscStream log_str;
         try
         {
            // we are going to listen on IPv6 and IPv4 sockets.  We can't rely on the dual stack socket
            // because this is not supported under windows server 2003 or XP.  Depending upon whether
            // the local_only flag is set, the addresses to which we will bind will either be the
            // localhost addresses or the wildcard addresses.
            SocketAddress::addresses_type addresses;
            SocketAddress::addresses_type temp;
            listening_addresses.clear();
            last_error.cut(0);
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
                     SocketAddress &address(temp.front());
                     if(address.get_family() == SocketAddress::family_ipv4)
                        addresses.push_back(temp.front());
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
                        SocketAddress &address = temp.front();
                        if(address.get_family() == SocketAddress::family_ipv6)
                           addresses.push_back(address);
                        temp.pop_front();
                     }
                  }
                  catch(std::exception &)
                  { }
               }
               SocketAddress::resolve(temp, "0.0.0.0", service_port, true);
               while(!temp.empty())
               {
                  SocketAddress &address = temp.front();
                  if(address.get_family() == SocketAddress::family_ipv4)
                     addresses.push_back(temp.front());
                  temp.pop_front();
               }
            }
            
            // we now need to create sockets for each of these addresses
            int rcd;
            while(!addresses.empty())
            {
               SocketAddress address(addresses.front());
               SOCKET sock_handle;

               if(logger)
               {
                  log_str.str("");
                  log_str << "listening on address\",\"" << address;
                  log_event_type::cpost(logger, log_str.str());
               }
               addresses.pop_front();
               sock_handle = ::socket(address.get_family(), SOCK_STREAM, 0);
               if(sock_handle != INVALID_SOCKET)
               {
                  // we need to ensure that the socket for IPv6 addresses is set to accept only IPv6
                  // connections only.  If the option is not supported, we will ignore the failure.
                  sockets.push_back(sock_handle);
                  if(address.get_family() == SocketAddress::family_ipv6)
                  {
                     int option_value(1);
                     setsockopt(
                        sock_handle,
                        IPPROTO_IPV6,
                        IPV6_V6ONLY,
                        (char *)&option_value,
                        sizeof(option_value));
                  }
                  if(logger)
                  {
                     log_str.str("");
                     log_str << "socket opened\",\"" << address << "\",\"" << sock_handle;
                     log_event_type::cpost(logger, log_str.str());
                  }

                  // we need to set the socket as non-blocking so that calls to listen() will not
                  // block
                  uint4 imode(1);
                  rcd = ::ioctlsocket(sock_handle, FIONBIO, &imode);
                  if(rcd != NO_ERROR)
                     throw WinsockException("Failure to set non-blocking");
                  
                  // we now need to bind the socket to the specified port.
                  rcd = ::bind(sock_handle, address.get_storage(), address.get_address_len());
                  if(rcd == SOCKET_ERROR)
                     throw WinsockException("Failure to bind to a listening socket");

                  // if a port of zero was specified, we will need to get the ephemeral port that
                  // was allocated.
                  if(service_port == 0)
                  {
                     struct sockaddr_storage storage;
                     struct sockaddr *addr(reinterpret_cast<struct sockaddr *>(&storage));
                     int addr_len(sizeof(storage));

                     rcd = ::getsockname(sock_handle, addr, &addr_len);
                     if(rcd != SOCKET_ERROR)
                     {
                        SocketAddress temp(addr, addr_len);
                        service_port = temp.get_port();
                        address.set_port(service_port);
                     }
                  }

                  // the last thing that we need to do is to start listening on the socket
                  rcd = ::listen(sock_handle, 5);
                  if(rcd == SOCKET_ERROR)
                     throw WinsockException("Listen failure");
                  if(logger)
                  {
                     log_str.str("");
                     log_str << "listen succeeded\",\"" << address;
                     log_event_type::cpost(logger, log_str.str());
                  }
               }
               else 
                  throw WinsockException("socket allocate failure");
               listening_addresses.push_back(address);
            }

            // if no sockets were allocated, there is no point in going on
            if(sockets.empty())
               throw WinsockException("no interfaces");
            
            // finally, we are ready to wait for events from the sockets
            start_condition.set();
            while(!should_close)
            {
               struct fd_set read_set, error_set;
               struct timeval timeout = { 0, 100000 };
               FD_ZERO(&read_set);
               FD_ZERO(&error_set);
               for(auto si = sockets.begin(); si != sockets.end(); ++si)
               {
                  FD_SET(*si, &read_set);
                  FD_SET(*si, &error_set);
               }
               rcd = ::select(
                  0,         // this is ignored in win32
                  &read_set,
                  0,         // we won't worry about write events
                  &error_set,
                  &timeout);
               if(rcd > 0)
               {
                  for(auto si = sockets.begin(); si != sockets.end(); ++si)
                  {
                     if(FD_ISSET(*si, &read_set))
                     {
                        struct sockaddr_storage storage;
                        struct sockaddr *client_addr(reinterpret_cast<struct sockaddr *>(&storage));
                        int client_addr_len(sizeof(storage));
                        SOCKET new_socket(::accept(*si, client_addr, &client_addr_len));
                        if(new_socket != INVALID_SOCKET)
                        {
                           if(logger)
                              log_event_type::cpost(logger, "accepted new connection");
                           accept_event::cpost(this, new_socket);
                        }
                     }
                     if(FD_ISSET(*si, &error_set))
                        throw WinsockException("listen error");
                  }
               }
               else if(rcd < 0)
                  throw WinsockException("select failure");
            }
         }
         catch(WinsockException &e)
         {
            log_str.str("");
            log_str << "error listening on socket\",\"" << e.what();
            if(logger)
               log_event_type::cpost(logger, log_str.c_str());
            last_error = log_str.str();
            error_event::cpost(this, e);
         }
         catch(std::exception &e)
         {
            log_str << "error listening on socket\",\"" << e.what();
            if(logger)
               log_event_type::cpost(logger, log_str.str());
            last_error = log_str.str();
            error_event::cpost(this, WinsockException("unrecognised error"));
         }
         
         // we need to close all of the sockets
         while(!sockets.empty())
         {
            ::closesocket(sockets.front());
            sockets.pop_front();
         }
         start_condition.set();
      } // execute
   };
};

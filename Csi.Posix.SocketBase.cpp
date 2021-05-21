/* Csi.Posix.SocketBase.cpp

   Copyright (C) 2005, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 24 August 2005
   Last Change: Thursday 28 June 2012
   Last Commit: $Date: 2015-01-13 13:40:06 -0600 (Tue, 13 Jan 2015) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Posix.SocketBase.h"
#include "Csi.StrAscStream.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>


namespace Csi
{
   namespace Posix
   {
      namespace SocketHelpers
      {
         ////////////////////////////////////////////////////////////
         // class error_event definitions
         ////////////////////////////////////////////////////////////
         uint4 const error_event::event_id =
            Event::registerType("Csi::Posix::SocketBase::error_event");
      };


      ////////////////////////////////////////////////////////////
      // class SocketBase definitions
      ////////////////////////////////////////////////////////////
      void SocketBase::close()
      {
         if(socket_handle != -1)
         {
            ::close(socket_handle);
            socket_handle = -1;
         }
      } // close


      bool SocketBase::get_peer_address(
         StrAsc &address,
         uint2 &port)
      {
         bool rtn = false;
         address.cut(0);
         port = 0;
         if(socket_handle != -1)
         {
            SocketAddress temp(get_peer_address());
            address = temp.get_address();
            port = temp.get_port();
            rtn = true;
         }
         return rtn;
      } // get_peer_address


      SocketAddress SocketBase::get_peer_address()
      {
         SocketAddress rtn;
         if(socket_handle != -1)
         {
            struct sockaddr_storage storage;
            struct sockaddr *address(reinterpret_cast<struct sockaddr *>(&storage));
            socklen_t address_len(sizeof(storage));
            int rcd(::getpeername(socket_handle, address, &address_len));
            if(rcd == 0)
               rtn = SocketAddress(address, address_len);
         }
         return rtn;
      } // get_peer_address


      void SocketBase::post_socket_error(int error_code)
      { SocketHelpers::error_event::cpost(this,error_code); }

      
      void SocketBase::receive(SharedPtr<Event> &ev)
      {
         using namespace SocketHelpers;
         if(ev->getType() == error_event::event_id)
         {
            error_event *event = static_cast<error_event *>(ev.get_rep());
            on_socket_error(event->error_code);
         }
      } // receive
   };
};

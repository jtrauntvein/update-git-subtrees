/* Csi.Win32.WinSockBase.cpp

   Copyright (C) 2001, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 22 February 2001
   Last Change: Tuesday 10 July 2012
   Last Commit: $Date: 2012-07-10 10:40:39 -0600 (Tue, 10 Jul 2012) $ 
   Committed by: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Win32.WinSockBase.h"


namespace Csi
{
   namespace Win32
   {
      ////////////////////////////////////////////////////////////
      // class WinSockBase definitions
      ////////////////////////////////////////////////////////////
      uint4 const WinSockBase::socket_event =
      RegisterWindowMessageW(L"Csi::Win32::WinSockbase::socket_event");


      WinSockBase::WinSockBase(SOCKET socket_handle_):
         socket_handle(socket_handle_)
      { }


      WinSockBase::~WinSockBase()
      { close(); }


      void WinSockBase::close()
      {
         if(socket_handle != INVALID_SOCKET)
         {
            closesocket(socket_handle);
            socket_handle = INVALID_SOCKET;
         }
      } // close


      bool WinSockBase::get_peer_address(
         StrAsc &peer_address,
         uint2 &peer_port)
      {
         bool rtn = false;
         peer_address.cut(0);
         peer_port = 0;
         if(socket_handle != INVALID_SOCKET)
         {
            SocketAddress address(get_peer_address());
            peer_address = address.get_address();
            peer_port = address.get_port();
         }
         return rtn;
      } // get_peer_address


      SocketAddress WinSockBase::get_peer_address()
      {
         SocketAddress rtn;
         if(socket_handle != INVALID_SOCKET)
         {
            struct sockaddr_storage storage;
            struct sockaddr *address(reinterpret_cast<struct sockaddr *>(&storage));
            int address_len = sizeof(storage);
            int rcd = getpeername(socket_handle, address, &address_len);
            if(rcd == 0)
               rtn = SocketAddress(address, address_len);
         }
         return rtn;
      } // get_peer_address


      LRESULT WinSockBase::on_message(
         uint4 message_id,
         WPARAM wparam,
         LPARAM lparam)
      {
         LRESULT rtn = 0;
         if(message_id == socket_event && socket_handle != INVALID_SOCKET)
         {
            int4 error_code = WSAGETSELECTERROR(lparam);
            int4 event = WSAGETSELECTEVENT(lparam);
            on_socket_event(error_code,event);
         }
         else
            rtn = MessageWindow::on_message(message_id,wparam,lparam);
         return rtn;
      } // on_message
   };
};

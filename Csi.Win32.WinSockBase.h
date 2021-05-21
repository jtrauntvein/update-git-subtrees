/* Csi.Win32.WinSockBase.h

   Copyright (C) 2001, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 22 February 2001
   Last Change: Tuesday 26 June 2012
   Last Commit: $Date: 2012-06-26 16:34:54 -0600 (Tue, 26 Jun 2012) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Csi_Win32_WinSockBase_h
#define Csi_Win32_WinSockBase_h

#include "Csi.MessageWindow.h"
#include "Csi.SocketAddress.h"
#include "StrAsc.h"
#include <winsock2.h>


namespace Csi
{
   namespace Win32
   {
      ////////////////////////////////////////////////////////////
      // class WinSockBase
      //
      // Defines a base class for accessing windows socket
      // services. Encapsulates the windows socket handle and implements the
      // base windows message handling mechanism for receiving events from the
      // windows sockets API. Defines abstract base functions for handling
      // specific event types such as socket errors.
      ////////////////////////////////////////////////////////////
      class WinSockBase: public MessageWindow
      {
      protected:
         ////////////////////////////////////////////////////////////
         // socket_handle
         //
         // The handle to the windows socket
         //////////////////////////////////////////////////////////// 
         SOCKET socket_handle;

         ////////////////////////////////////////////////////////////
         // socket_event
         //
         // Refers to the windows event type that will report events from the
         // winsock layer
         //////////////////////////////////////////////////////////// 
         static uint4 const socket_event;
         
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         //////////////////////////////////////////////////////////// 
         WinSockBase(SOCKET socket_handle_ = INVALID_SOCKET);

         ////////////////////////////////////////////////////////////
         // destructor
         //////////////////////////////////////////////////////////// 
         virtual ~WinSockBase();

         ////////////////////////////////////////////////////////////
         // close
         //
         // Releases any resources associated with the handle. The derived
         // class can overload this method if it has its own resources to
         // release. It should, however, delegate to this version so that the
         // socket handle will be properly released.
         //////////////////////////////////////////////////////////// 
         virtual void close();

         ////////////////////////////////////////////////////////////
         // get_peer_address
         //
         // Returns the address of the remote host to which the socket handle
         // may be connected and the peer port for the same in the program
         // arguments. Will return false is the socket is not connected.
         //////////////////////////////////////////////////////////// 
         virtual bool get_peer_address(
            StrAsc &peer_address,
            uint2 &peer_port);

         ////////////////////////////////////////////////////////////
         // get_peer_address
         //
         // Returns the peer address of the remote host as an integer in network order. 
         ////////////////////////////////////////////////////////////
         virtual SocketAddress get_peer_address();
         
         ////////////////////////////////////////////////////////////
         // get_socket_handle
         ////////////////////////////////////////////////////////////
         virtual SOCKET get_socket_handle() const
         { return socket_handle; }
         
         ////////////////////////////////////////////////////////////
         // on_socket_error
         //
         // Called when a socket error has been detected. 
         //////////////////////////////////////////////////////////// 
         virtual void on_socket_error(int error_code)
         { close(); } 

      protected:
         ////////////////////////////////////////////////////////////
         // on_socket_event
         //
         // Encapsulates a windows socket events with the parameters. This
         // method is invoked on on_message and has the winsock event
         // parameters cracked from the windows message parameters. This method
         // should be overloaded by the derived class to provide the specific
         // behaviour for the event.
         //////////////////////////////////////////////////////////// 
         virtual void on_socket_event(
            int error_code,
            int event) = 0;
         
         ////////////////////////////////////////////////////////////
         // on_message
         //
         // Handles an incoming windows message. A derived class can choose to
         // overload this method but it must be delegated to for all messages
         // associated with the socket_event message class.
         //////////////////////////////////////////////////////////// 
         virtual LRESULT on_message(
            uint4 message_id,
            WPARAM wparam,
            LPARAM lparam);
      };
   };
};

#endif

/* Csi.Posix.SocketBase.h

   Copyright (C) 2005, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 24 August 2005
   Last Change: Thursday 28 June 2012
   Last Commit: $Date: 2012-06-28 10:51:50 -0600 (Thu, 28 Jun 2012) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Csi_Posix_SocketBase_h
#define Csi_Posix_SocketBase_h

#include "Csi.Events.h"
#include "StrAsc.h"
#include "CsiTypeDefs.h"
#include "Csi.SocketAddress.h"


typedef int SOCKET;


namespace Csi
{
   namespace Posix
   {
      ////////////////////////////////////////////////////////////
      // class SocketBase
      //
      // Defines base services for socket components of all types.  This class
      // encapsulates a socket handle and provides a common error handling
      // mechanism for the derived classes.  Because of the handle
      // encapsulation, automatic closure of the socket can be guaranteed by
      // the virtual destructor.  
      ////////////////////////////////////////////////////////////
      class SocketBase: public EventReceiver
      {
      protected:
         ////////////////////////////////////////////////////////////
         // socket_handle
         ////////////////////////////////////////////////////////////
         SOCKET socket_handle;

      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         SocketBase(int socket_handle_ = -1):
            socket_handle(socket_handle_)
         { }

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~SocketBase()
         { close(); }

         ////////////////////////////////////////////////////////////
         // close
         //
         // Closes the socket handle (if open and sets the internal member to
         // -1 (invalid socket value). 
         ////////////////////////////////////////////////////////////
         virtual void close();

         ////////////////////////////////////////////////////////////
         // get_peer_address
         //
         // Returns the address of the remote host to which the socket handle
         // may be connected and the peer port.  The return value will return
         // the connected state of the socket. 
         ////////////////////////////////////////////////////////////
         virtual bool get_peer_address(
            StrAsc &peer_address,
            uint2 &peer_port);

         ////////////////////////////////////////////////////////////
         // get_peer_address
         //
         // Returns the peer address of the remote host as an integer in
         // network order.
         ////////////////////////////////////////////////////////////
         virtual SocketAddress get_peer_address();

         ////////////////////////////////////////////////////////////
         // get_socket_handle
         ////////////////////////////////////////////////////////////
         virtual SOCKET get_socket_handle() const
         { return socket_handle; }
         
         ////////////////////////////////////////////////////////////
         // on_socket_error
         ////////////////////////////////////////////////////////////
         virtual void on_socket_error(int error_code)
         { close(); }

      protected:
         ////////////////////////////////////////////////////////////
         // post_socket_error
         ////////////////////////////////////////////////////////////
         virtual void post_socket_error(int error_code);

         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(SharedPtr<Event> &ev);
      };


      namespace SocketHelpers
      {
         ////////////////////////////////////////////////////////////
         // class error_event
         ////////////////////////////////////////////////////////////
         class error_event: public Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // error_code
            ////////////////////////////////////////////////////////////
            int error_code;

            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(SocketBase *receiver, int error_code)
            {
               error_event *event = new error_event(receiver,error_code);
               try { event->post(); }
               catch(Event::BadPost &) { delete event; }
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            error_event(
               SocketBase *receiver,
               int error_code_):
               Event(event_id,receiver),
               error_code(error_code_)
            { }
         };
      }; 
   };
};


#endif

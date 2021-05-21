/* Csi.Posix.SocketConnection.h

   Copyright (C) 2005, 2005 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 20 September 2005
   Last Change: Tuesday 20 December 2005
   Last Commit: $Date: 2016-06-23 10:14:34 -0600 (Thu, 23 Jun 2016) $ (UTC)
   Last Changed by: $Author: jon $

*/

#ifndef Csi_Posix_SocketConnection_h
#define Csi_Posix_SocketConnection_h

#include "Csi.Messaging.Connection.h"
#include "Csi.Posix.SocketTcpSock.h"


namespace Csi
{
   namespace Posix
   {
      ////////////////////////////////////////////////////////////
      // class SocketConnection
      //
      // Uses the posix sockets interface to provide a message delivery service
      // using TCP/IP.  This class can be used either in a server application
      // or in a client application.  
      ////////////////////////////////////////////////////////////
      class SocketConnection:
         public Messaging::Connection,
         public SocketTcpSock
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor (for server)
         ////////////////////////////////////////////////////////////
         SocketConnection(int socket_handle);

         ////////////////////////////////////////////////////////////
         // constructor (for client)
         ////////////////////////////////////////////////////////////
         SocketConnection(
            char const *server_name,
            uint2 server_port);

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~SocketConnection();

         //@group methods overloaded from Connection that implement the
         //connection interface.

         ////////////////////////////////////////////////////////////
         // sendMessage
         ////////////////////////////////////////////////////////////
         virtual void sendMessage(Messaging::Message *msg);

         ////////////////////////////////////////////////////////////
         // attach
         //
         // Starts the connection process only if the object is created as a
         // client connection.  If created as a server, this method will do
         // nothing more than set the is_connected flag. 
         ////////////////////////////////////////////////////////////
         virtual void attach();

         ////////////////////////////////////////////////////////////
         // detach
         //
         // Closes the socket handle and clears the is_connected flag. 
         ////////////////////////////////////////////////////////////
         virtual void detach();

         ////////////////////////////////////////////////////////////
         // get_remote_address
         ////////////////////////////////////////////////////////////
         StrAsc get_remote_address();
         
         //@endgroup

      protected:
         //@group methods overloaded from class Posix::SocketTcpSock
         ////////////////////////////////////////////////////////////
         // on_read
         ////////////////////////////////////////////////////////////
         virtual void on_read();

         ////////////////////////////////////////////////////////////
         // on_close
         ////////////////////////////////////////////////////////////
         virtual void on_close();

         /**
          * Overloads the base class to handle a socket error.
          */
         virtual void on_socket_error(int error_code)
         { on_close(); }
         
         //@endgroup

      private:
         ////////////////////////////////////////////////////////////
         // pop_length
         //
         // Retrieves the length of the next message from the receive buffer. 
         ////////////////////////////////////////////////////////////
         uint4 pop_length();
         
      private:
         ////////////////////////////////////////////////////////////
         // is_server
         //
         // Set to true if this object was created to be a server. 
         ////////////////////////////////////////////////////////////
         bool is_server;

         ////////////////////////////////////////////////////////////
         // between_messages
         //
         // Set to true if we are waiting for a message to be finished. 
         ////////////////////////////////////////////////////////////
         bool between_messages;

         ////////////////////////////////////////////////////////////
         // message_len
         //
         // Specifies the length of the current message (valid only when
         // between_messages is set). 
         ////////////////////////////////////////////////////////////
         uint4 message_len;

         ////////////////////////////////////////////////////////////
         // server_name
         ////////////////////////////////////////////////////////////
         StrAsc server_name;

         ////////////////////////////////////////////////////////////
         // server_port
         ////////////////////////////////////////////////////////////
         uint2 server_port;
      };
   };
};


#endif

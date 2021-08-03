/* Csi.Win32.SocketListener.h

   Copyright (C) 2000, 2005 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 9 August 1996
   Last Change: Thursday 12 May 2005
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)

*/

#ifndef Csi_Win32_SocketListener_h
#define Csi_Win32_SocketListener_h

#include "Csi.Win32.WinSockService.h"


namespace Csi
{
   //@group class forward declarations
   namespace Messaging { class Server; };
   //@endgroup

   namespace Win32
   {
      ////////////////////////////////////////////////////////////
      // class SocketListener
      //
      // Instances of this class create server sockets on using the specified port number and accept
      // incoming connections. Once a connection is accepted, an instance of NetStub and an
      // associated NetSockConn instance will be created and the default NetServer object will be
      // notified that the connection has been made. This class is designed to work with the WinSock
      // library.
      ////////////////////////////////////////////////////////////
      class SocketListener: public WinSockService
      {
      public:
         ////////// constructor
         // Responsible for performing the windows initialisation (creates an object
         // window) and for creating a server socket using the specified <i>port</i>
         // number.
         SocketListener(
            Messaging::Server *default_server,
            uint2 port = 6789,
            char const *bind_address = " ");

         ////////////////////////////////////////////////////////////
         // destructor
         //
         // Closes the server socket and destroys the object window.
         ////////////////////////////////////////////////////////////
         virtual ~SocketListener();

      protected:
         ////////////////////////////////////////////////////////////
         // on_accept
         ////////////////////////////////////////////////////////////
         virtual void on_accept(SOCKET new_connection);

         ////////////////////////////////////////////////////////////
         // on_socket_error
         ////////////////////////////////////////////////////////////
         virtual void on_socket_error(int error_code);
   
      private:
         ////////// default_server
         // The object that will be notified of a new connection
         Messaging::Server *default_server;
      };

   };
};

#endif

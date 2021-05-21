/* Csi.SocketListener.h

   Copyright (C) 2000, 2013 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 21 September 2005
   Last Change: Tuesday 08 October 2013
   Last Commit: $Date: 2013-10-08 11:10:20 -0600 (Tue, 08 Oct 2013) $ 
   Commited by: $Author: jon $
*/

#ifndef Csi_SocketListener_h
#define Csi_SocketListener_h

#include "Csi.SocketTcpService.h"


namespace Csi
{
   //@group class forward declarations
   namespace Messaging { class Server; };
   //@endgroup

   ////////////////////////////////////////////////////////////
   // class SocketListener
   //
   // Instances of this class create server sockets on using the specified port
   // number and accept incoming connections. Once a connection is accepted, an
   // instance of NetStub and an associated NetSockConn instance will be
   // created and the default NetServer object will be notified that the
   // connection has been made. This class is designed to work with the WinSock
   // library.
   ////////////////////////////////////////////////////////////
   class SocketListener: public SocketTcpService
   {
   public:
      ////////////////////////////////////////////////////////////
      // constructor
      //
      // Responsible for performing the windows initialisation (creates an object
      // window) and for creating a server socket using the specified <i>port</i>
      // number.
      ////////////////////////////////////////////////////////////
      SocketListener(
         Messaging::Server *default_server,
         uint2 port = 6789,
         char const *bind_address = " ",
         EventReceiver *logger = 0,
         bool allow_ipv6 = true);

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

#endif

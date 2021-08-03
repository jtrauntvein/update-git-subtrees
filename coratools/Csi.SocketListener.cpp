/* Csi.Win32.SocketListener.cpp

   Copyright (C) 2000, 2014 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 21 September 2005
   Last Change: Friday 29 August 2014
   Last Commit: $Date: 2014-09-02 08:44:49 -0600 (Tue, 02 Sep 2014) $ 
   Committed by: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.SocketListener.h"
#include "Csi.Messaging.Defs.h"
#include "Csi.Messaging.Server.h"
#include "Csi.Messaging.Stub.h"
#include "Csi.Utils.h"
#include "trace.h"
#ifdef _WIN32
#include "Csi.Win32.SocketConnection.h"
#else
#include "Csi.Posix.SocketConnection.h"
#include <unistd.h>
#endif


namespace Csi
{
#ifdef _WIN32
   using Win32::SocketConnection;
#else
   using Posix::SocketConnection;
#endif

   
   ////////////////////////////////////////////////////////////
   // class SocketListener definitions
   ////////////////////////////////////////////////////////////
   SocketListener::SocketListener(
      Messaging::Server *default_server_,
      uint2 port,
      char const *bind_address,
      EventReceiver *logger,
      bool allow_ipv6):
      default_server(default_server_)
   {
      bool local_only(true);
      if(bind_address[0] == 0 || bind_address[0] == ' ')
         local_only = false;
      set_logger(logger);
      set_allow_ipv6(allow_ipv6);
      start_service(port, local_only);
   }


   SocketListener::~SocketListener()
   { stop_service(); }


   void SocketListener::on_accept(SOCKET new_connection)
   {
      try
      {
         SocketConnection *conn = new SocketConnection(new_connection);
         Messaging::Stub *stub = new Messaging::Stub(default_server,conn);
      }
      catch(std::exception &e)
      {
         trace("Csi::SocketListener::on_accept: connection allocation failed: %s", e.what());
#ifdef _WIN32
         ::closesocket(new_connection);
#else
         ::close(new_connection);
#endif
      }
   } // on_accept
   
   
   void SocketListener::on_socket_error(int error_code)
   { default_server->onListenerFail(this); }
};

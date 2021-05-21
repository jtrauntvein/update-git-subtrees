/* Csi.Win32.SocketListener.cpp

   Copyright (C) 2000, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 21 June 2000
   Last Change: Wednesday 27 June 2012
   Last Commit: $Date: 2012-06-27 15:07:44 -0600 (Wed, 27 Jun 2012) $ 
   Committed by: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Win32.SocketListener.h"
#include "Csi.Win32.SocketConnection.h"
#include "Csi.Messaging.Defs.h"
#include "Csi.Messaging.Server.h"
#include "Csi.Messaging.Stub.h"
#include "Csi.Utils.h"
#include "trace.h"


namespace Csi
{
   namespace Win32
   {
      ////////////////////////////////////////////////////////////
      // class SocketListener definitions
      ////////////////////////////////////////////////////////////
      SocketListener::SocketListener(
         Messaging::Server *default_server_,
         uint2 port,
         char const *bind_address):
         default_server(default_server_)
      {
         bool local_only(true);
         if(bind_address[0] == 0 || bind_address[0] == ' ')
            local_only = false;
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
            trace("Csi::Win32::SocketListener::on_accept: socket connection failed: %s", e.what());
            ::closesocket(new_connection);
         }
      } // on_accept


      void SocketListener::on_socket_error(int error_code)
      { default_server->onListenerFail(this); }
   };
};

/* Cora.Win32.DllNetConnManager.h

   Copyright (C) 2000, 2011 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 02 June 2000
   Last Change: Wednesday 16 March 2011
   Last Commit: $Date: 2011-03-17 09:05:06 -0600 (Thu, 17 Mar 2011) $ 
   Committed by: $Author: jon $
   
*/
#ifndef Cora_Win32_DllNetConnManager_h
#define Cora_Win32_DllNetConnManager_h

#include "CoraDllMessagingDefs.h"
#define WIN32_LEAN_AND_MEAN
#define _WINSOCKAPI_    // stops windows.h including winsock.h
#include <windows.h>

//@group class forward declarations
namespace Csi { namespace Messaging { class Message; }; };
namespace Cora { namespace Win32 { class DllNetConn; }; };
//@endgroup

namespace Cora
{
   namespace Win32
   {
      ////////////////////////////////////////////////////////////
      // namespace DllNetConnManager
      //
      // Defines a collection of functions that manage the global state of DLL connections. This
      // namespace forms a boundary layer between the connection objects and the DLL interface declared
      // in CoraDllMessagingDefs.h
      ////////////////////////////////////////////////////////////
      namespace DllNetConnManager
      {
         ////////////////////////////////////////////////////////////
         // add_connection
         //
         // Adds a connection object to the set managed by this class. Will register that connection
         // object through the DLL interface. Will throw a std::exception based object if the addition
         // fails.
         ////////////////////////////////////////////////////////////
         void add_connection(
            DllNetConn *connection,
            HMODULE coralib_module_);

         ////////////////////////////////////////////////////////////
         // remove_connection
         //
         // Removes a connection object from the list managed by this object
         ////////////////////////////////////////////////////////////
         void remove_connection(DllNetConn *connection);

         ////////////////////////////////////////////////////////////
         // receive_event
         //
         // Handles an event received through the DLL interface. This function can be passed as a
         // callback to the cora_start_connection() function.
         ////////////////////////////////////////////////////////////
         int __stdcall receive_event(
            uint4 connection_id,
            uint4 event_id,
            uint4 message_body_len,
            void const *message_body);

         ////////////////////////////////////////////////////////////
         // add_message
         //
         // Adds the specified message to the queue that can be deleted later by delete_message.
         ////////////////////////////////////////////////////////////
         void add_message(Csi::Messaging::Message &message);

         ////////////////////////////////////////////////////////////
         // delete_message
         //
         // Deletes the specified message from the queue.
         ////////////////////////////////////////////////////////////
         void delete_message(void const *message_body);
      };
   };
};




#endif

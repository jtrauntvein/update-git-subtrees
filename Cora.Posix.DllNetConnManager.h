/* Cora.Posix.DllNetConnManager.h

   Copyright (C) 2020 Campbell Scientific, Inc.

   Written by: Andrew Mortenson
   Date Begun: Monday 20 May 2020
   Last Change: Monday 20 May 2020
   Last Commit: $Date: $
   Committed by: $Author: $
   
*/

#ifndef Cora_Posix_DllNetConnManager_h
#define Cora_Posix_DllNetConnManager_h

#include "CoraDllMessagingDefs.h"

//@group class forward declarations
namespace Csi { namespace Messaging { class Message; }; };
namespace Cora { namespace Posix { class DllNetConn; }; };
//@endgroup

namespace Cora
{
   namespace Posix
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
            void* coralib_module_);

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
            uint8 connection_id,
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

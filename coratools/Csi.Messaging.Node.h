/* Csi.Messaging.Node.h

   Copyright (C) 2000, Campbell Scientific, Inc

   Written by: Jon Trauntvein
   Date Begun: 2 August 1996
   Last Change: Saturday 21 October 2000
   Last Commit: $Date: 2007-11-13 14:47:16 -0600 (Tue, 13 Nov 2007) $ (UTC)

*/

#ifndef Csi_Messaging_Node_h
#define Csi_Messaging_Node_h

#include "CsiTypeDefs.h"
#include "Csi.InstanceValidator.h"

namespace Csi
{
   namespace Messaging
   {
      //@group class forward declarations
      class Message;
      class Router;
      //@endgroup

      ////////// class Node
      // Declares the interface that any object should implement that needs to be able to receive
      // Csi::Messaging messages.
      class Node: public InstanceValidator
      {
      public:
         ////////// onNetSesBroken
         // Called when a session that had been previously opened is no longer viable
         virtual void onNetSesBroken(Router *router,
                                     uint4 session_no,
                                     uint4 error_code,
                                     char const *error_message) = 0;

         ////////// onNetMessage
         // Called by the router when a message on the session associated with this node has been
         // received
         virtual void onNetMessage(Router *router,
                                   Message *message) = 0;
      };
   };
};

#endif

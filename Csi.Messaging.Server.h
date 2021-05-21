/* Csi.Messaging.Server.h

   Copyright (C) 2000, 2006 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 21 June 2000
   Last Change: Tuesday 14 March 2006
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)
   Committed by: $Author: tmecham $
   
*/

#ifndef Csi_Messaging_Server_h
#define Csi_Messaging_Server_h

#include "Csi.Messaging.Node.h"
#include <set>


namespace Csi
{
   namespace Messaging
   {
      //@group class forward declarations
      class Stub;
      //@endgroup
      

      ////////////////////////////////////////////////////////////
      // class Server
      //
      // Defines a class that acts as a server of the CSI messaging protocol. 
      ////////////////////////////////////////////////////////////
      class Server: public Node
      {
      public:
         ////////////////////////////////////////////////////////////
         // destructor
         //
         // Ensures that the list of stubs is deleted
         //////////////////////////////////////////////////////////// 
         virtual ~Server();

         ////////////////////////////////////////////////////////////
         // onSessionOpen
         //
         // Called by a Stub object on the server when a client has
         // opened a session
         //////////////////////////////////////////////////////////// 
         virtual void onSessionOpen(Stub *stub, uint4 session_no);

         ////////////////////////////////////////////////////////////
         // onListenerFail
         //
         // Called by a listener object when it can no longer perform its function.
         // Since there is no abstract listener class defined, a simple number is
         // used to identify the listener that failed.  This will probably be a 
         // pointer to the failing listener.
         //////////////////////////////////////////////////////////// 
         virtual void onListenerFail(void *listenerID) { }

         ////////////////////////////////////////////////////////////
         // addStub
         //
         // Called by a stub when it is first created. This will create an entry in
         // the list of stubs for this instance.
         //////////////////////////////////////////////////////////// 
         void addStub(Stub *stub);

         ////////////////////////////////////////////////////////////
         // removeStub
         //
         // Called to remove this stub from the list of all.
         ////////////////////////////////////////////////////////////
         void removeStub(Stub *stub);

      protected:
         ////////////////////////////////////////////////////////////
         // stubs
         //
         // Represents those stubs that have partially completed sessions (the first
         // message has not yet passed through)
         //////////////////////////////////////////////////////////// 
         typedef std::set<Stub *> stubs_type;
         stubs_type stubs;

         ////////////////////////////////////////////////////////////
         // deleteStub
         //
         // Called by a subclass to delete a stub from the list
         //////////////////////////////////////////////////////////// 
         void deleteStub(Stub *stub);
      };

   };
};

#endif

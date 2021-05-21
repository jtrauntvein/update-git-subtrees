/* Csi.Messaging.Server.cpp

   Copyright (C) 2000, Campbell Scientific, Inc

   Written by: Jon Trauntvein
   Date Begun: Friday 9 August 1996
   Last Change: Wednesday 21 June 2000
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)

*/

#pragma warning(disable:4786) // disable overlong symbols warning
#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Messaging.Server.h"
#include "Csi.Messaging.Stub.h"
#include "Csi.Messaging.Defs.h"


namespace Csi
{
   namespace Messaging
   {
      ////////////////////////////////////////////////////////////
      // class Server definitions
      ////////////////////////////////////////////////////////////

      Server::~Server()
      {
         while(!stubs.empty())
         {
            stubs_type::iterator si = stubs.begin();
            delete *si;
            stubs.erase(si);
         }
      } // destructor


      void Server::onSessionOpen(Stub *stub, uint4 sesNo)
      {
         // remove the stub from the list since the session is now complete
         removeStub(stub);
      } // onSessionOpen


      void Server::addStub(Stub *stub)
      { stubs.insert(stub); }


      void Server::removeStub(Stub *stub)
      {
         stubs_type::iterator si = stubs.find(stub);
         if(si != stubs.end())
            stubs.erase(si);
      } // removeStub


      void Server::deleteStub(Stub *stub)
      {
         stubs_type::iterator si = stubs.find(stub);
         if(si != stubs.end())
         {
            stubs.erase(si);
            delete stub;
         }
      } // deleteStub

   };
};

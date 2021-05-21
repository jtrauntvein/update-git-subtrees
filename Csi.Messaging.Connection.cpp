/* Csi.Messaging.Connection.cpp

   Copyright (C) 2000, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 24 February 1997
   Last Change: Tuesday 05 January 2016
   Last Commit: $Date: 2016-01-05 18:09:55 -0600 (Tue, 05 Jan 2016) $ 
   Committed by: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Messaging.Connection.h"
#include "Csi.Messaging.Message.h"
#include "Csi.Messaging.Router.h"
#include "Csi.Messaging.Defs.h"
#include "OneShot.h"
#include "Csi.CriticalSection.h"


namespace Csi
{
   namespace Messaging
   {
      ////////////////////////////////////////////////////////////
      // class Connection definitions
      ////////////////////////////////////////////////////////////
      const uint4 Connection::txTimeOut = 60000;
      int Connection::instanceCnt = 0;
      CriticalSection ConnectionProtector;
      OneShot *Connection::watchDog = 0;


      Connection::Connection()
      {
         ConnectionProtector.lock();
         if(instanceCnt++ == 0)
            watchDog = new OneShot;
         txWdId = watchDog->arm(this,txTimeOut);
         ConnectionProtector.unlock();
      } // constructor


      Connection::~Connection()
      {
         ConnectionProtector.lock();
         watchDog->disarm(txWdId);
         if(--instanceCnt == 0)
         {
            delete watchDog;
            watchDog = 0;
         }
         ConnectionProtector.unlock();
      } // destructor


      void Connection::onOneShotFired(uint4 id)
      {
         ConnectionProtector.lock();
         if(id == txWdId)
         {
            Message msg(0,Messages::type_heart_beat);
            sendMessage(&msg);
            txWdId = watchDog->arm(this,txTimeOut);
         } // response to transmit watch dog
         ConnectionProtector.unlock();
      } // onOneShotFired


      bool Connection::peer_is_remote()
      {
         StrAsc address(get_remote_address());
         address.cut(address.rfind(":"));
         return address != "127.0.0.1" && address != "[::1]";
      } // peer_is_remote
      

      void Connection::resetTxWd()
      {
         ConnectionProtector.lock();
         watchDog->reset(txWdId);
         ConnectionProtector.unlock();
      } // resetTxWd
   };
};

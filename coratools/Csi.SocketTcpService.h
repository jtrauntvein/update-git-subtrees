/* Csi.SocketTcpService.h

   Copyright (C) 2005, 2013 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 12 September 2005
   Last Change: Thursday 26 September 2013
   Last Commit: $Date: 2013-09-26 09:08:29 -0600 (Thu, 26 Sep 2013) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Csi_SocketTcpService_h
#define Csi_SocketTcpService_h

#ifdef _WIN32
#include "Csi.Win32.WinsockService.h"
#else
#include "Csi.Posix.SocketTcpService.h"
#endif
#include "Csi.Events.h"


namespace Csi
{
#ifdef _WIN32
   typedef Win32::WinSockService SocketTcpService;
#else
   using Posix::SocketTcpService;
#endif


   ////////////////////////////////////////////////////////////
   // class SocketServiceLogEvent
   ////////////////////////////////////////////////////////////
   class SocketServiceLogEvent: public Event
   {
   public:
      ////////////////////////////////////////////////////////////
      // event_id
      ////////////////////////////////////////////////////////////
      static uint4 const event_id;

      ////////////////////////////////////////////////////////////
      // message
      ////////////////////////////////////////////////////////////
      StrAsc const message;

      ////////////////////////////////////////////////////////////
      // cpost
      ////////////////////////////////////////////////////////////
      static void cpost(EventReceiver *receiver, StrAsc const &message)
      {
         SocketServiceLogEvent *event(new SocketServiceLogEvent(receiver, message));
         event->post();
      }

   private:
      ////////////////////////////////////////////////////////////
      // constructor
      ////////////////////////////////////////////////////////////
      SocketServiceLogEvent(EventReceiver *receiver, StrAsc const &message_):
         Event(event_id, receiver),
         message(message_)
      { }
   };
};


#endif

/* Csi.Messaging.Connection.h

   Copyright (C) 2000, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 21 June 2000
   Last Change: Tuesday 05 January 2016
   Last Commit: $Date: 2016-01-05 18:09:55 -0600 (Tue, 05 Jan 2016) $ 
   Committed By: $Author: jon $
   
*/

#ifndef Csi_Messaging_Connection_h
#define Csi_Messaging_Connection_h

#include "OneShot.h"
#include "Csi.InstanceValidator.h" 
#include "StrAsc.h"


namespace Csi
{
   namespace Messaging
   {
      //@group class forward declarations
      class Message;
      class Router;
      //@endgroup


      /**
       * Declares the interface for a network connection object.  There is a
       * one to one relationship between connections and routers.  Once a
       * connection is assigned to a router, that connection comes strictly
       * under the router's control.
       */
      class Connection: public OneShotClient
      {
      public:
         /**
          * Default constructor.  This has no parameters because, typically, a
          * client will allocate a new connection object before it allocates a
          * router.
          */
         Connection();

         /**
          * Destructor that must be overloaded to clean up any resources allocated for this
          * connection.
          */
         virtual ~Connection();

         /**
          * Must be overloaded to initiate the network connection under router control.  A
          * connection object should have all of the information that it needs to make that
          * connection before it is assigned to a router.  The actual connection should not be made
          * until this method is invoked by the router.
          */
         virtual void attach()
         { }

         /**
          * Invoked by the router when its session count for this connection has dropped to zero.
          */
         virtual void detach() { }

         /**
          * Must be overloaded to transmit the specified message to the peer.
          *
          * @param msg Specifies the content of the message to be sent.
          */
         virtual void sendMessage(Message *msg) = 0;

         /**
          * @return Returns the router associated with this connection.
          */
         Router *getRouter()
         { return router; }

         /**
          * @param router_ Specifies the router that will be associated with this connection.
          */
         void setRouter(Router *router_)
         { router = router_; }

         /**
          * @return Returns a string representation of the peer network address.  This will likely
          * only be overloaded for TCP based connection objects.
          */
         virtual StrAsc get_remote_address()
         { return ""; }

         /**
          * @return Returns true if the remote connection originates from a different machine.
          */
         virtual bool peer_is_remote();
         
      protected:
         /**
          * Reference to the router that owns this connection.
          */
         Router *router;

         /**
          * Resets the transmit watch dog timer.  This method should be called whenever the derived
          * class has transmitted characters across the link.  If this watch dog fires, the
          * connection will send a heart beat message in order to prevent the peer's receive wathc
          * dog from being fired.
          */
         void resetTxWd();

         /**
          * Overloads the base class version to handle transmit and receive watch dog timers.
          */
         virtual void onOneShotFired(uint4 id);

      private:
         /**
          * Specifies the identifier for the transmit watch dog timer.
          */
         uint4 txWdId;

         /**
          * Specifies the time in milli-seconds for the transmit wathc dog timer.
          */
         static const uint4 txTimeOut;

         /**
          * Reference to the watch dog timer shared by this and other connections.
          */
         static OneShot *watchDog;

         /**
          * Specifies the total number of connection instances that have been created in the
          * application.
          */
         static int instanceCnt;
      };

   };
};

#endif

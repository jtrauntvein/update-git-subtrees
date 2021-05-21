/* Cora.LgrNet.PakbusRoutersEnumerator.h

   Copyright (C) 2002, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 11 June 2002
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2019-11-21 14:57:26 -0600 (Thu, 21 Nov 2019) $ 
   Last Changed by: $Author: amortenson $

*/

#ifndef Cora_LgrNet_PakbusRoutersEnumerator_h
#define Cora_LgrNet_PakbusRoutersEnumerator_h

#include "Cora.ClientBase.h"
#include "Csi.InstanceValidator.h"
#include "Csi.Events.h"


namespace Cora
{
   namespace LgrNet
   {
      //@group class forward declarations
      class PakbusRoutersEnumerator;
      //@endgroup


      ////////////////////////////////////////////////////////////
      // class PakbusRoutersEnumeratorClient
      ////////////////////////////////////////////////////////////
      class PakbusRoutersEnumeratorClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_router_added
         //
         // Called when a PakBus router object has been added.
         ////////////////////////////////////////////////////////////
         virtual void on_router_added(
            PakbusRoutersEnumerator *enumerator,
            StrUni const &router_name)
         { }

         ////////////////////////////////////////////////////////////
         // on_router_deleted
         //
         // Called when a PakBus router object has been deleted.
         ////////////////////////////////////////////////////////////
         virtual void on_router_deleted(
            PakbusRoutersEnumerator *enumerator,
            StrUni const &router_name)
         { }

         ////////////////////////////////////////////////////////////
         // on_started
         ////////////////////////////////////////////////////////////
         virtual void on_started(
            PakbusRoutersEnumerator *enumerator)
         { }

         ////////////////////////////////////////////////////////////
         // on_failure
         //
         // Called when the enumerator has failed for the reason indicated by the failure
         // parameter. 
         ////////////////////////////////////////////////////////////
         enum failure_type
         {
            failure_unknown = 0,
            failure_session_failed = 1,
            failure_invalid_logon = 2,
            failure_unsupported = 3,
            failure_server_security_blocked = 4,
         };
         virtual void on_failure(
            PakbusRoutersEnumerator *enumerator,
            failure_type failure) = 0;
      };


      ////////////////////////////////////////////////////////////
      // class PakbusRoutersEnumerator
      //
      // This component allows the client to keep track of the pakbus routers that exist in a
      // LoggerNet server instance.
      //
      // An application can use this component by creating an instance of class
      // PakbusRoutersEnumerator, setting the appropriate attributes, and calling start() with a
      // reference to an object derived from class PakbusRoutersEnumeratorClient which will receive
      // the notifications from the component.  Once start() has been called, the component will
      // start the enumeration transaction with the server.  After the initial list of routers is
      // received, the component will invoke the client's on_router_added() for each router recieved
      // and then invoke the client's on_started() method.  Thereafter, as routers are added or
      // removed, the component will invoke the client's on_router_added() and on_router_deleted()
      // methods as these notifications come from the server.  The component will continue to
      // operate in this fashion until the component is deleted, finish() is called, or the
      // connection to the server is lost. 
      ////////////////////////////////////////////////////////////
      class PakbusRoutersEnumerator:
         public ClientBase,
         public Csi::EventReceiver
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         PakbusRoutersEnumerator();

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~PakbusRoutersEnumerator();

         ////////////////////////////////////////////////////////////
         // start (new connection)
         ////////////////////////////////////////////////////////////
         typedef PakbusRoutersEnumeratorClient client_type;
         void start(client_type *client_, router_handle &router);

         ////////////////////////////////////////////////////////////
         // start (share existing connection)
         ////////////////////////////////////////////////////////////
         void start(client_type *client_, ClientBase *other_component);

         ////////////////////////////////////////////////////////////
         // finish
         ////////////////////////////////////////////////////////////
         void finish();

         ////////////////////////////////////////////////////////////
         // format_failure
         ////////////////////////////////////////////////////////////
         static void format_failure(std::ostream &out, client_type::failure_type failure);

      protected:
         //@group ClientBase derived methods
         ////////////////////////////////////////////////////////////
         // onNetMessage
         ////////////////////////////////////////////////////////////
         virtual void onNetMessage(
            Csi::Messaging::Router *rtr,
            Csi::Messaging::Message *msg);

         ////////////////////////////////////////////////////////////
         // on_corabase_ready
         ////////////////////////////////////////////////////////////
         virtual void on_corabase_ready();

         ////////////////////////////////////////////////////////////
         // on_corabase_failure
         ////////////////////////////////////////////////////////////
         virtual void on_corabase_failure(corabase_failure_type failure);

         ////////////////////////////////////////////////////////////
         // on_corabase_session_failure
         ////////////////////////////////////////////////////////////
         virtual void on_corabase_session_failure(); 
         //@endgroup

         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

      private:
         //@group handlers for the new transaction
         ////////////////////////////////////////////////////////////
         // on_start_ack
         ////////////////////////////////////////////////////////////
         void on_start_ack(Csi::Messaging::Message *message);

         ////////////////////////////////////////////////////////////
         // on_notification
         ////////////////////////////////////////////////////////////
         void on_notification(Csi::Messaging::Message *message);
         //@endgroup
         
         //@group handlers for the old transaction
         ////////////////////////////////////////////////////////////
         // on_old_start_ack
         ////////////////////////////////////////////////////////////
         void on_old_start_ack(Csi::Messaging::Message *message);

         ////////////////////////////////////////////////////////////
         // on_notification
         ////////////////////////////////////////////////////////////
         void on_old_notification(Csi::Messaging::Message *message);
         //@endgroup
         
      private:
         ////////////////////////////////////////////////////////////
         // client
         ////////////////////////////////////////////////////////////
         client_type *client;

         ////////////////////////////////////////////////////////////
         // state
         ////////////////////////////////////////////////////////////
         enum state_type
         {
            state_standby,
            state_delegate,
            state_active,
         } state; 
      };
   };
};


#endif

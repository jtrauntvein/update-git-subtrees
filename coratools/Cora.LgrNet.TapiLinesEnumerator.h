/* Cora.LgrNet.TapiLinesEnumerator.h

   Copyright (C) 2001, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 12 June 2001
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Committed by: $Author: jon $
   
*/


#ifndef Cora_LgrNet_TapiLinesEnumerator_h
#define Cora_LgrNet_TapiLinesEnumerator_h


#include "Cora.ClientBase.h"
#include "Csi.InstanceValidator.h"
#include "CsiEvents.h"
#include "Cora.LgrNet.Defs.h"


namespace Cora
{
   namespace LgrNet
   {
      //@group class forward declarations
      class TapiLinesEnumerator;
      //@endgroup


      ////////////////////////////////////////////////////////////
      // class TapiLinesEnumeratorClient
      //
      // Defines the client class' interface for a tapi lines enumerator object.
      ////////////////////////////////////////////////////////////
      class TapiLinesEnumeratorClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_started
         //
         // Called when the enumeration transaction has been started with the server and all of the
         // initial notifications have been received.
         ////////////////////////////////////////////////////////////
         virtual void on_started(
            TapiLinesEnumerator *enumerator)
         { }

         ////////////////////////////////////////////////////////////
         // on_failure
         //
         // Called when  a failure has occurred that will force the enumerator object into a standby
         // state. 
         ////////////////////////////////////////////////////////////
         enum failure_type
         {
            failure_unknown = 0,
            failure_invalid_logon = 1, // logon parameters are invalid
            failure_session_broken = 2, // the messaging session was lost
            failure_unsupported = 3, // the transaction is not supported
            failure_server_security = 4, // server security disallowed the transaction
            failure_server_aborting = 5, // the server shut down the transaction
         };
         virtual void on_failure(
            TapiLinesEnumerator *enumerator,
            failure_type failure) = 0;

         ////////////////////////////////////////////////////////////
         // on_line_added
         //
         // Called when a new line has been detected by the server and also when the enumerator is
         // initialising. The initial list of line names will be reported through this method.
         ////////////////////////////////////////////////////////////
         virtual void on_line_added(
            TapiLinesEnumerator *enumerator,
            StrAsc const &line_name)
         { }

         ////////////////////////////////////////////////////////////
         // on_line_removed
         //
         // Called when the server has detected that an existing line has been removed. 
         ////////////////////////////////////////////////////////////
         virtual void on_line_removed(
            TapiLinesEnumerator *enumerator,
            StrAsc const &line_name)
         { }
      };


      ////////////////////////////////////////////////////////////
      // class TapiLinesEnumerator
      ////////////////////////////////////////////////////////////
      class TapiLinesEnumerator:
         public ClientBase,
         public Csi::EvReceiver
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         TapiLinesEnumerator();

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~TapiLinesEnumerator();

         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         typedef TapiLinesEnumeratorClient client_type;
         void start(
            client_type *client_,
            router_handle &router);
         void start(
            client_type *client_,
            ClientBase *other_component);

         ////////////////////////////////////////////////////////////
         // finish
         ////////////////////////////////////////////////////////////
         void finish();

      protected:
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

         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

         ////////////////////////////////////////////////////////////
         // onNetMessage
         ////////////////////////////////////////////////////////////
         virtual void onNetMessage(
            Csi::Messaging::Router *rtr,
            Csi::Messaging::Message *msg);

      private:
         ////////////////////////////////////////////////////////////
         // on_started_notification
         ////////////////////////////////////////////////////////////
         void on_started_notification(Csi::Messaging::Message *message);

         ////////////////////////////////////////////////////////////
         // on_line_added_notification
         ////////////////////////////////////////////////////////////
         void on_line_added_notification(Csi::Messaging::Message *message);

         ////////////////////////////////////////////////////////////
         // on_line_removed_notification
         ////////////////////////////////////////////////////////////
         void on_line_removed_notification(Csi::Messaging::Message *message);

         ////////////////////////////////////////////////////////////
         // on_stopped_notification
         ////////////////////////////////////////////////////////////
         void on_stopped_notification(Csi::Messaging::Message *message);

      private:
         ////////////////////////////////////////////////////////////
         // state
         ////////////////////////////////////////////////////////////
         enum state_type
         {
            state_standby,
            state_delegate,
            state_active
         } state;

         ////////////////////////////////////////////////////////////
         // client
         ////////////////////////////////////////////////////////////
         client_type *client;
      };
   };
};


#endif

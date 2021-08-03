/* Cora.LgrNet.BatchModeMaintainer.h

   Copyright (C) 2001, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 14 December 2001
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Cora_LgrNet_BatchModeMaintainer_h
#define Cora_LgrNet_BatchModeMaintainer_h

#include "Cora.ClientBase.h"
#include "Csi.Events.h"
#include "Csi.InstanceValidator.h"


namespace Cora
{
   namespace LgrNet
   {
      //@group class forward declarations
      class BatchModeMaintainer;
      //@endgroup


      ////////////////////////////////////////////////////////////
      // class BatchModeMaintainerClient
      ////////////////////////////////////////////////////////////
      class BatchModeMaintainerClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_started
         ////////////////////////////////////////////////////////////
         virtual void on_started(
            BatchModeMaintainer *maintainer)
         { }

         ////////////////////////////////////////////////////////////
         // on_failure
         ////////////////////////////////////////////////////////////
         enum failure_type
         {
            failure_unknown = 0,
            failure_invalid_logon = 1,
            failure_session_broken = 2,
            failure_unsupported = 3,
            failure_server_security_blocked = 4,
            failure_duplicate = 5,
         };
         virtual void on_failure(
            BatchModeMaintainer *maintainer,
            failure_type failure) = 0;
      };


      ////////////////////////////////////////////////////////////
      // class BatchModeMaintainer
      //
      // Defines a component that can be used to maintain the LgrNet network controller in batch
      // mode. This will cause the server to suppress automatic network map notifications until
      // batch mode is ended by closing the transaction (or its session).
      //
      // In order to use this component, an application must provide an object derived from class
      // BatchModeMaintainerClient. It can then create an instance of the class, set the appropriate
      // properties and call start(). The batch mode transaction will be maintained until finish()
      // is called, the component is deleted, or the server session is lost.
      ////////////////////////////////////////////////////////////
      class BatchModeMaintainer:
         public ClientBase,
         public Csi::EventReceiver
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         BatchModeMaintainer();

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~BatchModeMaintainer();

         ////////////////////////////////////////////////////////////
         // start
         //
         // Two versions of start() are provided. One expects a newly created router and will
         // execute all of the logon protocol specified by the component properties. The other
         // version will clone the information provided by the other_component parameter. 
         ////////////////////////////////////////////////////////////
         typedef BatchModeMaintainerClient client_type;
         void start(
            client_type *client_,
            router_handle &router);
         void start(
            client_type *client_,
            ClientBase *other_component);

         ////////////////////////////////////////////////////////////
         // finish
         ////////////////////////////////////////////////////////////
         virtual void finish();

      protected:
         ////////////////////////////////////////////////////////////
         // method on_corabase_ready
         ////////////////////////////////////////////////////////////
         virtual void on_corabase_ready();

         ////////////////////////////////////////////////////////////
         // method on_corabase_failure
         ////////////////////////////////////////////////////////////
         virtual void on_corabase_failure(corabase_failure_type failure);

         ////////////////////////////////////////////////////////////
         // method on_corabase_session_failure
         ////////////////////////////////////////////////////////////
         virtual void on_corabase_session_failure();

         ////////////////////////////////////////////////////////////
         // method receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

         ////////////////////////////////////////////////////////////
         // method onNetMessage
         ////////////////////////////////////////////////////////////
         virtual void onNetMessage(
            Csi::Messaging::Router *rtr,
            Csi::Messaging::Message *msg);

      private:
         ////////////////////////////////////////////////////////////
         // on_start_ack
         ////////////////////////////////////////////////////////////
         void on_start_ack(Csi::Messaging::Message *message);

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
            state_active
         } state;
      };
   };
};


#endif

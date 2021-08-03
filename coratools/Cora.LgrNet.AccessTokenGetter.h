/* Cora.LgrNet.AccessTokenGetter.h

   Copyright (C) 2020, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Tuesday 15 December 2020
   Last Change: Thursday 17 December 2020
   Last Commit: $Date: 2020-12-17 12:40:37 -0600 (Thu, 17 Dec 2020) $
   Last Changed by: $Author: jon $

*/

#ifndef Cora_LgrNet_AccessTokenGetter_h
#define Cora_LgrNet_AccessTokenGetter_h
#include "Cora.ClientBase.h"
#include "Csi.Events.h"

namespace Cora
{
   namespace LgrNet
   {
      class AccessTokenGetter;


      /**
       * Defines a class that specifies the interface that must be implemented by an application
       * object in order to use the AccessTokenGetter component type.
       */
      class AccessTokenGetterClient: public Csi::InstanceValidator
      {
      public:
         /**
          * Called by the component to report that the LgrNet transaction is complete.
          *
          * @param sender Specifies the component calling this method.
          *
          * @param outcome Specifies the outcome of the transaction.
          *
          * @param access_token Specifies the access token that was returned by the server.  Empty
          * if the transaction did not succeed.
          *
          * @param refresh_token Specifies the refresh token returned by the server.  Empty if the
          * transaction did not succeed.
          */
         enum outcome_type
         {
            outcome_failure_unknown = 0,
            outcome_success = 1,
            outcome_failure_no_account = 2,
            outcome_failure_session = 3,
            outcome_failure_unsupported = 4,
            outcome_failure_security = 5,
            outcome_failure_invalid_access = 6,
            outcome_failure_access_expired = 7,
            outcome_failure_invalid_refresh = 8,
            outcome_failure_refresh_expired = 9
         };
         virtual void on_complete(
            AccessTokenGetter *sender,
            outcome_type outcome,
            StrAsc const &access_token,
            StrAsc const &refresh_token) = 0;
      };


      /**
       * Defines an object that will request an access and refresh token from the LoggerNet
       * server.  In order to use this component, the application must provide an object that
       * extends class AccessTokenGetterClient.  It should then create an instance of this class.
       * If the application has a an access token that needs to be renewed, this should be set by
       * calling set_refresh_token().  The application should then call one of the two versions of
       * start().  When the server transaction has completed, the component will call the client's
       * on_complete() method.
       */
      class AccessTokenGetter:
         public Cora::ClientBase,
         public Csi::EventReceiver
      {
      private:
         /**
          * Specifies the client for this component.
          */
         AccessTokenGetterClient *client;

         /**
          * Specifies the state of this component.
          */
         enum state_type
         {
            state_standby,
            state_delegate,
            state_active
         } state;

         /**
          * Specifies the optional refresh token.
          */
         StrAsc refresh_token;

      public:
         /**
          * Constructor
          */
         AccessTokenGetter():
            client(0),
            state(state_standby)
         { }

         /**
          * Destructor
          */
         virtual ~AccessTokenGetter()
         { finish(); }

         /**
          * @return Returns the refresh token that will be passed to the server.
          */
         StrAsc const &get_refrssh_token() const
         { return refresh_token; }

         /**
          * @param value Specifies the refresh token that will be passed to the server.
          */
         void set_refresh_token(StrAsc const &value)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            refresh_token = value;
         }

         /**
          * Starts the transaction with the server.
          *
          * @param client_ Specifies the client to which the outcome will be reported.
          *
          * @param other_client Specifies a component that already has a server connection that can
          * be shared with this component.
          *
          * @param router Specifies the newly constructed and not connected messaging router to be
          * used by this component.
          */
         typedef AccessTokenGetterClient client_type;
         void start(client_type *client_, ClientBase *other_client)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("invalid client pointer");
            client = client_;
            state = state_delegate;
            ClientBase::start(other_client);
         }
         void start(client_type *client_, router_handle router)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("invalid client pointer");
            client = client_;
            state = state_delegate;
            ClientBase::start(router);
         }

         /**
          * Overrides the base class version to release resources and reset state.
          */
         virtual void finish() override
         {
            client = 0;
            state = state_standby;
            ClientBase::finish();
         }

         /**
          * Overrides the base class version to handle asynch events.
          */
         virtual void receive(event_handle &ev) override;

         /**
          * Writes a text description of the outcome to the stream.
          *
          * @param out Specifies the output stream.
          *
          * @param outcome Specifies the outcome code to format.
          */
         static void describe_outcome(std::ostream &out, client_type::outcome_type outcome);

      protected:
         /**
          * Overloads the base class version to start the transaction
          */
         virtual void on_corabase_ready() override;

         /**
          * Overloads the base class version to handle a failure report.
          */
         virtual void on_corabase_failure(corabase_failure_type failure) override;

         /**
          * Overrides the base class version to start the login.  If we are given a refresh token,
          * we should be able to get the access token without having to log in.
          */
         virtual void start_logon() override;

         /**
          * Overrides the base class version to handle incoming messages.
          */
         virtual void onNetMessage(
            Csi::Messaging::Router *router, Csi::Messaging::Message *message) override;
      };
   };
};

#endif

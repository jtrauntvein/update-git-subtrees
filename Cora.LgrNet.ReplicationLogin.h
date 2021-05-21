/* Cora.LgrNet.ReplicationLogin.h

   Copyright (C) 2020, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Thursday 15 October 2020
   Last Change: Tuesday 03 November 2020
   Last Commit: $Date: 2020-11-03 11:29:38 -0600 (Tue, 03 Nov 2020) $
   Last Changed by: $Author: jon $

*/

#ifndef Cora_LgrNet_ReplicationLogin_h
#define Cora_LgrNet_ReplicationLogin_h

#include "Cora.ClientBase.h"
#include "Csi.Events.h"


namespace Cora
{
   namespace LgrNet
   {
      /**
       * Defines the interface that must be implemented by an application object in order to use
       * the ReplicationLogin component.
       */
      class ReplicationLogin;
      class ReplicationLoginClient: public Csi::InstanceValidator
      {
      public:
         /**
          * Called by the component when the server tranaction has completed.
          *
          * @param sender Specifies the component sending this event.
          *
          * @param outcome Specifies the outcome of the transaction.
          */
         enum outcome_type
         {
            outcome_failure_unknown = 0,
            outcome_success = 1,
            outcome_failure_logon = 2,
            outcome_failure_session = 3,
            outcome_failure_unsupported = 4,
            outcome_failure_security = 5,
            outcome_failure_cloud_comms = 6,
            outcome_failure_cloud = 7,
            outcome_failure_disabled = 8,
            outcome_failure_busy = 9
         };
         virtual void on_complete(ReplicationLogin *sender, outcome_type outcome) = 0;
      };


      /**
       * Defines a component that can be used to register the LoggerNet server instance with the
       * campbell cloud by requesting an off-line access token which can be used by the server to
       * replicate collected data to the cloud ingest services.  In order to use this component, and
       * application must provide an object that extends class ReplicationLoginClient and should
       * then create an instance of this class.  The application should then set the properties of
       * this class including user_name and password and then invoke one of the two versions of
       * start().  When the LoggerNet transaction is complete, this component will invoke the
       * client object's on_complete() method.
       */
      class ReplicationLogin: public ClientBase, public Csi::EventReceiver
      {
      private:
         /**
          * Specifies the client object.
          */
         ReplicationLoginClient *client;

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
          * Specifies the user name of the campbell cloud account.
          */
         StrUni user_name;

         /**
          * Specifies the password for the campbell cloud account.
          */
         StrUni password;

         /**
          * Specifies the base URL for the campbell cloud server.
          */
         StrAsc base_uri;
         
      public:
         /**
          * Constructor
          */
         ReplicationLogin():
            client(0),
            state(state_standby),
            base_uri("https://api.campbellcloud.io")
         { }

         /**
          * Destructor
          */
         virtual ~ReplicationLogin()
         {
            finish();
            client = 0;
         }

         /**
          * @return Returns the user_name property.
          */
         StrUni const &get_user_name() const
         { return user_name; }

         /**
          * @param value Specifies the value of the user_name property.
          */
         void set_user_name(StrUni const &value)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            user_name = value;
         }

         /**
          * @return Returns the value of the password property.
          */
         StrUni const &get_password() const
         { return password; }

         /**
          * @param value Specifies the value of the login password.
          */
         void set_password(StrUni const &value)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            password = value;
         }

         /**
          * @return Returns the value of the campbell cloud base URI.
          */
         StrAsc const &get_base_uri() const
         { return base_uri; }

         /**
          * @param value Specifies the value of the campbell cloud base URI.
          */
         void set_base_uri(StrAsc const &value)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            base_uri = value;
         }

         /**
          * Starts the server transaction after connecting if needed.
          *
          * @param client_ Specifies the application object that will receive the completion event.
          *
          * @param router Specifies a messaging router that has not yet been connected to a server.
          *
          * @param other_client Specifies another component that already has an active connection to
          * the server that this component can borrow.
          */
         typedef ReplicationLoginClient client_type;
         void start(client_type *client_, router_handle &router)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("invalid client pointer");
            client = client_;
            state = state_delegate;
            ClientBase::start(router);
         }
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

         /**
          * Overloads the base class version to set all properties to standby state.
          */
         virtual void finish() override
         {
            client = 0;
            state = state_standby;
            ClientBase::finish();
         }

         /**
          * Overloads the base class version to handle asynch events.
          */
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev) override;

         /**
          * Describes the specified outcome to the specified stream.
          */
         static void describe_outcome(std::ostream &out, client_type::outcome_type outcome);

      protected:
         /**
          * Overloads the base class version to start the server transaction.
          */
         virtual void on_corabase_ready() override;

         /**
          * Overloads the base class version to handle a failure report.
          */
         virtual void on_corabase_failure(corabase_failure_type failure) override;

         /**
          * Overloads the base class version to handle incoming server messages.
          */
         virtual void onNetMessage(Csi::Messaging::Router *router, Csi::Messaging::Message *message);
      };
   };
};


#endif

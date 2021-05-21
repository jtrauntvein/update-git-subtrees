/* Cora.Sec2.Enabler.h

   Copyright (C) 2002, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 24 December 2002
   Last Change: Saturday 19 October 2019
   Last Commit: $Date: 2019-10-19 10:35:54 -0600 (Sat, 19 Oct 2019) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Cora_Sec2_Enabler_h
#define Cora_Sec2_Enabler_h

#include "Cora.Sec2.Sec2Base.h"
#include "Csi.InstanceValidator.h"
#include "Csi.Events.h"


namespace Cora
{
   namespace Sec2
   {
      class Enabler;


      /**
       * Defines the interface that the application must implement in order to use the Enabler
       * component type.
       */
      class EnablerClient: public Csi::InstanceValidator
      {
      public:
         /**
          * Called to report that the LoggerNet transaction has been completed.
          *
          * @param sender Specifies the component that called this method,
          *
          * @param outcome Specifies the outcome of the LoggerNet transaction.
          */
         enum outcome_type
         {
            outcome_unknown = 0,
            outcome_success = 1,
            outcome_connection_failed = 2,
            outcome_invalid_logon = 3,
            outcome_insufficient_access = 4,
            outcome_unsupported = 5,
            outcome_no_root_account = 6,
            outcome_locked = 7,
            outcome_not_admin = 8
         };
         virtual void on_complete(Enabler *sender, outcome_type outcome) = 0;
      };


      /**
       * Defines a component that can be used to enable or to disable LoggerNet security.  In order
       * to use this component, the application must provide a client object that extends class
       * EnablerClient.  It should then create an instance of this class, set its enabled property,
       * and call one of the two versions of start().  When the LoggerNet transaction is complete,
       * the component will call the client's on_complete() method.
       */
      class Enabler:
         public Sec2Base,
         public Csi::EventReceiver
      {
      public:
         /**
          * Constructor
          */
         Enabler();

         /**
          * Destructor
          */
         virtual ~Enabler();

         /**
          * @param security_enabled_ Set to true if LoggerNet security is to be enabled.
          */
         void set_security_enabled(bool security_enabled_);

         /**
          * @return Returns true if LoggerNet security is to be enabled.
          */
         bool get_security_enabled() const
         { return security_enabled; }

         /**
          * Called to connect to the server and to execute LoggerNet transaction.
          *
          * @param client_ Specifies the application object that will receive a completion
          * notification.
          *
          * @param router Specifies a messaging router that has not been previously connected to the
          * server.
          *
          * @param other_component Specifies a connected component that can share its connection
          * with this component.
          */
         typedef EnablerClient client_type;
         void start(client_type *client_, router_handle &router);
         void start(client_type *client, ClientBase *other_component);

         /**
          * Releases any resources and returns this component to a standby state.
          */
         virtual void finish();

         /**
          * Formats the specified outcome code to the specified stream.
          *
          * @param out Specifies the stream to which the outcome description will be written.
          *
          * @param outcome Specifies the outcome code to describe.
          */
         static void describe_outcome(std::ostream &out, client_type::outcome_type outcome);

         /**
          * Overloads the base class to handle asynchronous events.
          */
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);
         
      protected:
         /**
          * Overloads the base class to start the transaction.
          */
         virtual void on_sec2base_ready();

         /**
          * Overloads the base class to handle a failure.
          */
         virtual void on_sec2base_failure(sec2base_failure_type failure);

         /**
          * Overloads the base class version to handle incoming messages.
          */
         virtual void onNetMessage(Csi::Messaging::Router *rtr, Csi::Messaging::Message *msg);

      private:
         /**
          * Specifies the client objkect.
          */
         client_type *client;

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
          * Set to true if security is to be enabled.
          */
         bool security_enabled;
      };
   };
};


#endif

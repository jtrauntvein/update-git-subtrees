/* Cora.Sec2.AccountDeleter.h

   Copyright (C) 2002, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 30 December 2002
   Last Change: Saturday 19 October 2019
   Last Commit: $Date: 2019-10-19 11:15:49 -0600 (Sat, 19 Oct 2019) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Cora_Sec2_AccountDeleter_h
#define Cora_Sec2_AccountDeleter_h

#include "Cora.Sec2.Sec2Base.h"
#include "Csi.InstanceValidator.h"
#include "Csi.Events.h"
#include <list>
#include <algorithm>


namespace Cora
{
   namespace Sec2
   {
      class AccountDeleter;


      /**
       * Defines the interface that an application object must implement in order to use the
       * AccountDeleter component type.
       */
      class AccountDeleterClient: public Csi::InstanceValidator
      {
      public:
         /**
          * Called by the component when the LoggerNet transaction has been completed.
          *
          * @param sender Specifies the component that called this method.
          *
          * @param outcome Specifies the outcome of the transaction.
          */
         enum outcome_type
         {
            outcome_unknown = 0,
            outcome_success = 1,
            outcome_connection_failed = 2,
            outcome_invalid_logon = 3,
            outcome_insufficient_access = 4,
            outcome_unsupported = 5,
            outcome_locked = 6,
            outcome_invalid_account_name = 7,
            outcome_account_in_use = 8,
         };
         virtual void on_complete(AccountDeleter *sender, outcome_type outcome) = 0;
      };

      
      /**
       * Defines a component that can be used to delete a security account.  In order to use this
       * component type, the application must provide an object that extends class
       * AccountDeleterClient and must use an account that has root level privileges.  It should
       * then create an instance of this class, set its account_name property, and call one of the
       * two versions of start().  When the LoggerNet transaction is complete, the component will
       * call the client's on_complete() method.
       */
      class AccountDeleter:
         public Sec2Base,
         public Csi::EventReceiver
      {
      private:
         /**
          * Specifies the name of the account to be deleted.
          */
         StrUni account_name;

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
          * Specifies the client for this component.
          */
      public:
         typedef AccountDeleterClient client_type;
      private:
         client_type *client;
         
      public:
         /**
          * Constructor
          */
         AccountDeleter();

         /**
          * Destructor
          */
         virtual ~AccountDeleter();

         /**
          * @param account_name_ Specifies the name of the account to be deleted.
          */
         void set_account_name(StrUni const &account_name_)
         {
            if(state == state_standby)
               account_name = account_name_;
            else
               throw exc_invalid_state();
         }

         /**
          * @return Returns the account name to be deleted.
          */
         StrUni const &get_account_name() const
         { return account_name; }

         /**
          * Called to start the LoggerNet transaction.
          *
          * @param client_ Specifies the client object.
          *
          * @param router Specifies a messaging router that has not yet been connected.
          *
          * @param other_client Specifies a component that has a connection that can be shared with
          * this component.
          */
         void start(client_type *client_, router_handle &router);
         void start(client_type *client_, ClientBase *other_client);

         /**
          * Called to return this component to a standby state.
          */
         virtual void finish();

         /**
          * Formats the specified outcome code to the specified stream.
          *
          * @param out Specifies the stream to which the description will be written.
          *
          * @param outcome Specifies the outcome code to format.
          */
         static void describe_outcome(std::ostream &out, client_type::outcome_type outcome);

         /**
          * Overloads the base class version to handle asynch events.
          */
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);
         
      protected:
         /**
          * Overloads the base class version to start the transaction.
          */
         virtual void on_sec2base_ready();

         /**
          * Overloads the base class version to handle a failure.
          */
         virtual void on_sec2base_failure(sec2base_failure_type failure);

         /**
          * Overloads the base class version to handle incoming messages.
          */
         virtual void onNetMessage(Csi::Messaging::Router *rtr, Csi::Messaging::Message *msg);
      };
   };
};


#endif

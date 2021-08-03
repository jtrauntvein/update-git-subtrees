/* Cora.Sec2.AccountsEnumerator.h

   Copyright (C) 2002, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 30 December 2002
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2019-10-29 14:52:52 -0600 (Tue, 29 Oct 2019) $ 
   Last Changed by: $Author: amortenson $

*/

#ifndef Cora_Sec2_AccountsEnumerator_h
#define Cora_Sec2_AccountsEnumerator_h

#include "Cora.Sec2.Sec2Base.h"
#include "Csi.InstanceValidator.h"
#include "Csi.Events.h"
#include <list>


namespace Cora
{
   namespace Sec2
   {
      //@group class forward declarations
      class AccountsEnumerator;
      //@endgroup


      ////////////////////////////////////////////////////////////
      // class AccountsEnumeratorClient
      ////////////////////////////////////////////////////////////
      class AccountsEnumeratorClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_started
         //
         // Called when all of the initial accounts have been read 
         //////////////////////////////////////////////////////////// 
         virtual void on_started(
            AccountsEnumerator *enumerator)
         { }

         ////////////////////////////////////////////////////////////
         // on_account_added
         ////////////////////////////////////////////////////////////
         typedef std::list<StrUni> device_additions_type;
         virtual void on_account_added(
            AccountsEnumerator *enumerator,
            StrUni const &account_name,
            StrUni const &account_password,
            AccessLevels::AccessLevelType access_level,
            device_additions_type const &device_additions)
         { }

         ////////////////////////////////////////////////////////////
         // on_account_changed
         ////////////////////////////////////////////////////////////
         virtual void on_account_changed(
            AccountsEnumerator *enumerator,
            StrUni const &account_name,
            StrUni const &account_password,
            AccessLevels::AccessLevelType access_level,
            device_additions_type const &device_additions)
         { }

         ////////////////////////////////////////////////////////////
         // on_account_deleted
         ////////////////////////////////////////////////////////////
         virtual void on_account_deleted(
            AccountsEnumerator *enumerator,
            StrUni const &account_name)
         { }

         ////////////////////////////////////////////////////////////
         // on_failure
         ////////////////////////////////////////////////////////////
         enum failure_type
         {
            failure_unknown = 0,
            failure_connection_failed = 1,
            failure_logon = 2,
            failure_insufficient_access = 3,
            failure_unsupported = 4,
         };
         virtual void on_failure(
            AccountsEnumerator *enumerator,
            failure_type failure) = 0; 
      };


      ////////////////////////////////////////////////////////////
      // class AccountsEnumerator
      //
      // Defines a component that can be used by an application to maintain
      // a copy of all of the account information supported by a LoggerNet
      // server that supports the Security2 interface.  In order to use
      // this component, an application must provide an object derived from
      // class AccountsEnumeratorClient (this class is typedefed as
      // client_type in this class definition).  It should then create an
      // instance of this class, optionally call set_logon_name() and
      // set_logon_password(), and then invoke one of the two versions of
      // start().  Assuming the server enumerate accounts transaction could
      // be started, the component will invoke the client's
      // on_account_added() methods for each existing account and then
      // invoke the client's on_started() method.  As the transaction
      // continues, the component will invoke on_account_added(),
      // on_account_changed(), or on_account_deleted() each time one of
      // these changes occurs on the server.  If the component becomes
      // unable to carry on the transaction, the client's on_failure()
      // notification will be invoked.
      //
      // The client can stop the component at any time by calling the
      // component's finish() method or by deleting the component. 
      ////////////////////////////////////////////////////////////
      class AccountsEnumerator:
         public Sec2Base,
         public Csi::EventReceiver
      {
         ////////////////////////////////////////////////////////////
         // client
         ////////////////////////////////////////////////////////////
      public:
         typedef AccountsEnumeratorClient client_type;
      private:
         client_type *client;

         ////////////////////////////////////////////////////////////
         // state
         ////////////////////////////////////////////////////////////
         enum state_type
         {
            state_standby,
            state_delegate,
            state_before_active,
            state_active
         } state;

      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         AccountsEnumerator();

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~AccountsEnumerator();

         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
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

         ////////////////////////////////////////////////////////////
         // format_failure
         ////////////////////////////////////////////////////////////
         static void format_failure(std::ostream &out, client_type::failure_type failure);

      protected:
         ////////////////////////////////////////////////////////////
         // on_sec2base_ready
         ////////////////////////////////////////////////////////////
         virtual void on_sec2base_ready();

         ////////////////////////////////////////////////////////////
         // on_sec2base_failure
         ////////////////////////////////////////////////////////////
         virtual void on_sec2base_failure(sec2base_failure_type failure);

         ////////////////////////////////////////////////////////////
         // onNetMessage
         ////////////////////////////////////////////////////////////
         virtual void onNetMessage(
            Csi::Messaging::Router *rtr,
            Csi::Messaging::Message *msg);

         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev); 
      };
   };
};


#endif

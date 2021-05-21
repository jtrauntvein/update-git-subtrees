/* Cora.LgrNet.CountriesEnumerator.h

   Copyright (C) 2001, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 13 June 2001
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Committed by: $Author: jon $
   
*/


#ifndef Cora_LgrNet_CountriesEnumerator_h
#define Cora_LgrNet_CountriesEnumerator_h


#include "Cora.ClientBase.h"
#include "Csi.InstanceValidator.h"
#include "CsiEvents.h"
#include "Cora.LgrNet.Defs.h"
#include <list>


namespace Cora
{
   namespace LgrNet
   {
      //@group class forward declarations
      class CountriesEnumerator;
      //@endgroup


      ////////////////////////////////////////////////////////////
      // class CountriesEnumeratorClient
      ////////////////////////////////////////////////////////////
      class CountriesEnumeratorClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_complete
         //
         // Called when the server transaction has been completed. The outcome parameter will
         // identify whether the transaction succeeded. The countries parameter will contain a list
         // of country_record_type structures if the outcome indicates success. Otherwise, the list
         // will be empty. 
         ////////////////////////////////////////////////////////////
         struct country_record_type
         {
            uint4 country_code;
            StrAsc country_name;
         };
         typedef std::list<country_record_type> countries_type;
         enum outcome_type
         {
            outcome_unknown = 0,
            outcome_success = 1,
            outcome_invalid_logon = 2,
            outcome_session_broken = 3,
            outcome_unsupported = 4,
            outcome_server_security = 5,
         };
         virtual void on_complete(
            CountriesEnumerator *enumerator,
            outcome_type outcome,
            countries_type &countries) = 0;
      };


      ////////////////////////////////////////////////////////////
      // class CountriesEnumerator
      //
      // Defines a component that exercises the LgrNet Enumerate Countries transaction and provides
      // the supplied information to a client object through a notification method when the
      // transaction is complete.
      ////////////////////////////////////////////////////////////
      class CountriesEnumerator:
         public ClientBase,
         public Csi::EvReceiver
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         CountriesEnumerator();

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~CountriesEnumerator();

         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         typedef CountriesEnumeratorClient client_type;
         void start(
            client_type *client_,
            router_handle &router);
         void start(
            client_type *client,
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

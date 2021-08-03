/* Cora.LgrNet.CommPortsLister.h

   Copyright (C) 2003, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 15 April 2003
   Last Change: Thursday 15 September 2016
   Last Commit: $Date: 2016-09-15 13:41:05 -0600 (Thu, 15 Sep 2016) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Cora_LgrNet_CommPortsLister_h
#define Cora_LgrNet_CommPortsLister_h

#include "Cora.ClientBase.h"
#include <deque>


namespace Cora
{
   namespace LgrNet
   {
      //@group class forward dedclarations
      class CommPortsLister;
      //@endgroup


      ////////////////////////////////////////////////////////////
      // class CommPortsListerClient
      ////////////////////////////////////////////////////////////
      class CommPortsListerClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_complete
         ////////////////////////////////////////////////////////////
         enum outcome_type
         {
            outcome_unknown = 0,
            outcome_success = 1,
            outcome_invalid_logon = 2,
            outcome_session_broken = 3,
            outcome_unsupported = 4,
            outcome_server_security_blocked = 5, 
         };
         typedef std::pair<StrAsc, StrAsc> name_type;
         typedef std::deque<name_type> names_type;
         virtual void on_complete(
            CommPortsLister *lister,
            outcome_type outcome,
            names_type const &names) = 0;
      };


      ////////////////////////////////////////////////////////////
      // class CommPortsLister
      //
      // Defines a component that can be used to obtain the list of comm ports that can be used on
      // the server's host computer.  In order to use this component, an application must provide a
      // client object derived from class CommPortsListerClient (also typedefed as
      // CommPortsLister::client_type).  It should then create an instance of this class, set
      // appropriate properties as needed, and invoke one of the two versions of start().  When the
      // server transaction is complete, the client's on_complete() method will be invoked and will
      // be passed the outcome of the transaction as well as a list of port names. 
      ////////////////////////////////////////////////////////////
      class CommPortsLister:
         public ClientBase,
         public Csi::EventReceiver
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         CommPortsLister();

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~CommPortsLister();

         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         typedef CommPortsListerClient client_type;
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
         virtual void receive(Csi::SharedPtr<Csi::Event> &event);

         ////////////////////////////////////////////////////////////
         // onNetMessage
         ////////////////////////////////////////////////////////////
         virtual void onNetMessage(
            Csi::Messaging::Router *rtr,
            Csi::Messaging::Message *msg);

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

         ////////////////////////////////////////////////////////////
         // requested_friendly_names
         ////////////////////////////////////////////////////////////
         bool requested_friendly_names;
      };
   };
};


#endif

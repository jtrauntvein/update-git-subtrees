/* Cora.LgrNet.LogsClearer.h

   Copyright (C) 2008, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 03 April 2008
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Cora_LgrNet_LogsClearer_h
#define Cora_LgrNet_LogsClearer_h

#include "Cora.ClientBase.h"
#include "Csi.Events.h"


namespace Cora
{
   namespace LgrNet
   {
      ////////////////////////////////////////////////////////////
      // class LogsClearerClient
      ////////////////////////////////////////////////////////////
      class LogsClearer;
      class LogsClearerClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_complete
         ////////////////////////////////////////////////////////////
         enum outcome_type
         {
            outcome_unknown = 0,
            outcome_success = 1,
            outcome_failure_logon = 2,
            outcome_failure_session = 3,
            outcome_failure_unsupported = 4,
            outcome_failure_security = 5,
            outcome_failure_clear_failed = 6
         };
         virtual void on_complete(
            LogsClearer *clearer,
            outcome_type outcome) = 0;
      };
      
      
      ////////////////////////////////////////////////////////////
      // class LogsClearer
      //
      // Defines a component that can be used to clear the log files in the
      // LoggerNet server.  In order to use this component, an application must
      // provide an object that inherits from class LogsClearerClient.  It should
      // then create an instance if LogsClearer, optionally invoke methods to
      // configure parameters such as user name and password, and invoke one of
      // the two start() methods.  Once the component has completed the server
      // transaction, the application will  be notified through the client's
      // overload of the on_complete() method. 
      ////////////////////////////////////////////////////////////
      class LogsClearer: public ClientBase, public Csi::EventReceiver
      {
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
         LogsClearerClient *client; 
         
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         LogsClearer():
            client(0),
            state(state_standby)
         { }
         
         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~LogsClearer()
         { finish(); }
         
         ////////////////////////////////////////////////////////////
         // start (new router)
         ////////////////////////////////////////////////////////////
         typedef LogsClearerClient client_type;
         void start(
            client_type *client, router_handle &router);
         
         ////////////////////////////////////////////////////////////
         // start (use other component connection)
         ////////////////////////////////////////////////////////////
         void start(
            client_type *client, ClientBase *other_component);
         
         ////////////////////////////////////////////////////////////
         // finish
         ////////////////////////////////////////////////////////////
         virtual void finish();

         ////////////////////////////////////////////////////////////
         // describe_outcome
         ////////////////////////////////////////////////////////////
         static void describe_outcome(std::ostream &out, client_type::outcome_type outcome);
         
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
      };
   };
};


#endif

/* Cora.LgrNet.LogsZipper.h

   Copyright (C) 2008, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 07 April 2008
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Cora_LgrNet_LogsZipper_h
#define Cora_LgrNet_LogsZipper_h

#include "Cora.ClientBase.h"
#include "Csi.Events.h"


namespace Cora
{
   namespace LgrNet
   {
      ////////////////////////////////////////////////////////////
      // class LogsZipperClient
      ////////////////////////////////////////////////////////////
      class LogsZipper;
      class LogsZipperClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_complete
         //
         // Called when the server transaction has been completed.  The
         // zip_file_name parameter will only have significance if the outcome
         // is equal to outcome_success.
         ////////////////////////////////////////////////////////////
         enum outcome_type
         {
            outcome_unknown = 0,
            outcome_success = 1,
            outcome_failure_logon = 2,
            outcome_failure_session = 3,
            outcome_failure_unsupported = 4,
            outcome_failure_security = 5,
            outcome_failure_create = 6
         };
         virtual void on_complete(
            LogsZipper *zipper,
            outcome_type outcome,
            StrAsc const &zip_file_name) = 0;
      };


      ////////////////////////////////////////////////////////////
      // class LogsZipper
      //
      // Defines a component that can be used to create a zip archive of the
      // server log files on the server host file system.  In order to use this
      // component, an application must provide an object that inherits from
      // class LogsZipperClient (also given the alias of
      // LogsZipper::client_type).  It must then create an instance of class
      // LogsZipper, optionally set login information and the zip file name,
      // and invoke one of the two overloads of start().  When the server
      // transaction is complete, the application will receive notification
      // through its client object's on_complete() method. 
      ////////////////////////////////////////////////////////////
      class LogsZipper:
         public ClientBase,
         public Csi::EventReceiver
      {
      private:
         ////////////////////////////////////////////////////////////
         // client
         ////////////////////////////////////////////////////////////
         LogsZipperClient *client;

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
         // create_file_name
         ////////////////////////////////////////////////////////////
         StrAsc create_file_name;
         
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         LogsZipper():
            client(0),
            state(state_standby)
         { }

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~LogsZipper()
         { finish(); }

         ////////////////////////////////////////////////////////////
         // get_create_file_name
         ////////////////////////////////////////////////////////////
         StrAsc const &get_create_file_name() const
         { return create_file_name; }

         ////////////////////////////////////////////////////////////
         // set_create_file_name
         ////////////////////////////////////////////////////////////
         void set_create_file_name(StrAsc const &name)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            create_file_name = name;
         }

         ////////////////////////////////////////////////////////////
         // start (new router)
         ////////////////////////////////////////////////////////////
         typedef LogsZipperClient client_type;
         void start(client_type *client, router_handle &router)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            if(!client_type::is_valid_instance(client))
               throw std::invalid_argument("invalid client parameter");
            state = state_delegate;
            this->client = client;
            ClientBase::start(router);
         }

         ////////////////////////////////////////////////////////////
         // start (borrow other component's connection)
         ////////////////////////////////////////////////////////////
         void start(client_type *client, ClientBase *other_component)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            if(!client_type::is_valid_instance(client))
               throw std::invalid_argument("invalid client parameter");
            state = state_delegate;
            this->client = client;
            ClientBase::start(other_component);
         }

         ////////////////////////////////////////////////////////////
         // finish
         ////////////////////////////////////////////////////////////
         virtual void finish()
         {
            state = state_standby;
            client = 0;
            ClientBase::finish();
         }

         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

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
         // onNetMessage
         ////////////////////////////////////////////////////////////
         virtual void onNetMessage(
            Csi::Messaging::Router *rtr,
            Csi::Messaging::Message *msg);
      };
   };
};


#endif

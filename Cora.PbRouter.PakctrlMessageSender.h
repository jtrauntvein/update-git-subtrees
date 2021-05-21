/* Cora.PbRouter.PakctrlMessageSender.h

   Copyright (C) 2004, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 09 January 2004
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Cora_PbRouter_PakctrlMessageSender_h
#define Cora_PbRouter_PakctrlMessageSender_h

#include "Cora.PbRouter.PbRouterBase.h"
#include "Csi.PakBus.PakCtrlMessage.h"


namespace Cora
{
   namespace PbRouter
   {
      //@group class forward declarations
      class PakctrlMessageSender;
      //@endgroup


      ////////////////////////////////////////////////////////////
      // class PakctrlMessageSenderClient
      ////////////////////////////////////////////////////////////
      class PakctrlMessageSenderClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_complete
         //
         // Defines the callback that will be invoked when the transaction is
         // complete.  the response and round trip parameters should be ignored
         // if the value of outcome is not equal to 1 (response received).
         ////////////////////////////////////////////////////////////
         typedef Csi::SharedPtr<Csi::PakBus::PakCtrlMessage> response_type;
         enum outcome_type
         {
            outcome_unknown = 0,
            outcome_response_received = 1,
            outcome_no_response_expected = 2,
            outcome_invalid_logon = 3,
            outcome_invalid_router_id = 4,
            outcome_server_permission_denied = 5,
            outcome_server_session_failed = 6,
            outcome_address_unreachable = 7,
            outcome_timed_out = 8,
            outcome_command_malformed = 9,
            outcome_command_unsupported = 10,
            outcome_unsupported = 11,
         };
         virtual void on_complete(
            PakctrlMessageSender *sender,
            outcome_type outcome,
            response_type &response,
            uint4 round_trip_time) = 0;
      };


      ////////////////////////////////////////////////////////////
      // class PakctrlMessageSender
      //
      // Defines a component that allows an application to use the server's
      // PakBus Router Send PakCtrl Message transaction.  In order to use this
      // component, an application must provide a client object that is derived
      // from class PakctrlMessageSenderClient.  It should then create an
      // instance of this class and call the appropriate methods to set
      // properties including set_pakbus_router_name() and
      // set_command_message().  It should then invoke start().  When the
      // server transaction has been completed, the client objects
      // on_complete() method will be invoked with parameters that describe the
      // outcome of the transaction including, if applicable, the response
      // message from the pakbus device.
      ////////////////////////////////////////////////////////////
      class PakctrlMessageSender:
         public PbRouterBase,
         public Csi::EventReceiver
      {
      private:
         ////////////////////////////////////////////////////////////
         // command
         ////////////////////////////////////////////////////////////
         Csi::SharedPtr<Csi::PakBus::PakCtrlMessage> command;

         ////////////////////////////////////////////////////////////
         // response_expected
         ////////////////////////////////////////////////////////////
         bool response_expected;

         ////////////////////////////////////////////////////////////
         // extra_timeout
         ////////////////////////////////////////////////////////////
         uint4 extra_timeout;

         ////////////////////////////////////////////////////////////
         // max_retries
         ////////////////////////////////////////////////////////////
         uint4 max_retries;

         ////////////////////////////////////////////////////////////
         // client
         ////////////////////////////////////////////////////////////
         PakctrlMessageSenderClient *client;

         ////////////////////////////////////////////////////////////
         // state
         ////////////////////////////////////////////////////////////
         enum state_type
         {
            state_standby,
            state_delegate,
            state_active
         } state;

      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         PakctrlMessageSender():
            client(0),
            state(state_standby),
            response_expected(true),
            max_retries(3),
            extra_timeout(1000)
         { }

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~PakctrlMessageSender()
         { finish(); }

         ////////////////////////////////////////////////////////////
         // get_command
         ////////////////////////////////////////////////////////////
         typedef Csi::SharedPtr<Csi::PakBus::PakCtrlMessage> command_type;
         command_type &get_command()
         { return command; }

         ////////////////////////////////////////////////////////////
         // set_command
         ////////////////////////////////////////////////////////////
         void set_command(command_type &command_)
         {
            if(state == state_standby)
               command = command_;
            else
               throw exc_invalid_state();
         }

         ////////////////////////////////////////////////////////////
         // get_response_expected
         ////////////////////////////////////////////////////////////
         bool get_response_expected() const
         { return response_expected; }

         ////////////////////////////////////////////////////////////
         // set_response_expected
         ////////////////////////////////////////////////////////////
         void set_response_expected(bool response_expected_)
         {
            if(state == state_standby)
               response_expected = response_expected_;
            else
               throw exc_invalid_state();
         }

         ////////////////////////////////////////////////////////////
         // get_extra_timeout
         ////////////////////////////////////////////////////////////
         uint4 get_extra_timeout() const
         { return extra_timeout; }

         ////////////////////////////////////////////////////////////
         // set_extra_timeout
         ////////////////////////////////////////////////////////////
         void set_extra_timeout(uint4 extra_timeout_)
         {
            if(state == state_standby)
               extra_timeout = extra_timeout;
            else
               throw exc_invalid_state();
         }

         ////////////////////////////////////////////////////////////
         // get_max_retries
         ////////////////////////////////////////////////////////////
         uint4 get_max_retries() const
         { return max_retries; }

         ////////////////////////////////////////////////////////////
         // set_max_retries
         ////////////////////////////////////////////////////////////
         void set_max_retries(uint4 max_retries_)
         {
            if(state == state_standby)
               max_retries = max_retries_;
            else
               throw exc_invalid_state();
         }

         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         typedef PakctrlMessageSenderClient client_type;
         void start(
            client_type *client_,
            router_handle &router);

         ////////////////////////////////////////////////////////////
         // start (other component)
         ////////////////////////////////////////////////////////////
         void start(
            client_type *client_,
            ClientBase *other_component);

         ////////////////////////////////////////////////////////////
         // finish
         ////////////////////////////////////////////////////////////
         virtual void finish();

      protected:
         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

         ////////////////////////////////////////////////////////////
         // on_pbrouterbase_ready
         ////////////////////////////////////////////////////////////
         virtual void on_pbrouterbase_ready();

         ////////////////////////////////////////////////////////////
         // on_pbrouterbase_failure
         ////////////////////////////////////////////////////////////
         virtual void on_pbrouterbase_failure(pbrouterbase_failure_type);

         ////////////////////////////////////////////////////////////
         // onNetMessage
         ////////////////////////////////////////////////////////////
         virtual void onNetMessage(
            Csi::Messaging::Router *router,
            Csi::Messaging::Message *message); 
      };
   };
};


#endif

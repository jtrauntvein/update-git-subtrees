/* Cora.LgrNet.ModemTypesAdder.h

   Copyright (C) 2009, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 14 September 2009
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Cora_LgrNet_ModemTypesAdder_h
#define Cora_LgrNet_ModemTypesAdder_h

#include "Cora.ClientBase.h"
#include "Csi.Events.h"


namespace Cora
{
   namespace LgrNet
   {
      ////////////////////////////////////////////////////////////
      // class ModemTypeAdderClient
      ////////////////////////////////////////////////////////////
      class ModemTypeAdder;
      class ModemTypeAdderClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_complete
         ////////////////////////////////////////////////////////////
         enum outcome_type
         {
            outcome_unknown = 0,
            outcome_success = 1,
            outcome_failure_invalid_logon = 2,
            outcome_failure_session = 3,
            outcome_failure_unsupported = 4,
            outcome_failure_security = 5,
            outcome_failure_invalid_modem_name = 6,
            outcome_failure_modem_already_exists = 7,
            outcome_failure_network_locked = 8
         };
         virtual void on_complete(
            ModemTypeAdder *adder, outcome_type outcome) = 0;
      };


      ////////////////////////////////////////////////////////////
      // class ModemTypeAdder
      ////////////////////////////////////////////////////////////
      class ModemTypeAdder:
         public ClientBase,
         public Csi::EventReceiver
      {
      private:
         ////////////////////////////////////////////////////////////
         // client
         ////////////////////////////////////////////////////////////
         ModemTypeAdderClient *client;

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
         // type_name
         ////////////////////////////////////////////////////////////
         StrUni type_name;

         ////////////////////////////////////////////////////////////
         // init
         ////////////////////////////////////////////////////////////
         StrAsc init;

         ////////////////////////////////////////////////////////////
         // reset
         ////////////////////////////////////////////////////////////
         StrAsc reset;

      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         ModemTypeAdder():
            client(0),
            state(state_standby)
         { }


         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~ModemTypeAdder()
         { finish(); }

         ////////////////////////////////////////////////////////////
         // start (new router)
         ////////////////////////////////////////////////////////////
         typedef ModemTypeAdderClient client_type;
         void start(
            client_type *client_, ClientBase::router_handle &router)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("invalid client pointer");
            state = state_delegate;
            client = client_;
            ClientBase::start(router);
         }

         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         void start(
            client_type *client_, ClientBase *other_component)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("invalid client pointer");
            state = state_delegate;
            client = client_;
            ClientBase::start(other_component);
         }

         ////////////////////////////////////////////////////////////
         // finish
         ////////////////////////////////////////////////////////////
         virtual void finish()
         {
            client = 0;
            state = state_standby;
            ClientBase::finish();
         }

         ////////////////////////////////////////////////////////////
         // get_type_name
         ////////////////////////////////////////////////////////////
         StrUni const &get_type_name() const
         { return type_name; }

         ////////////////////////////////////////////////////////////
         // set_type_name
         ////////////////////////////////////////////////////////////
         void set_type_name(StrUni const &type_name_)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            type_name = type_name_;
         }

         ////////////////////////////////////////////////////////////
         // get_reset
         ////////////////////////////////////////////////////////////
         StrAsc const &get_reset() const
         { return reset; }

         ////////////////////////////////////////////////////////////
         // set_reset
         ////////////////////////////////////////////////////////////
         void set_reset(StrAsc const &reset_)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            reset = reset_;
         }

         ////////////////////////////////////////////////////////////
         // get_init
         ////////////////////////////////////////////////////////////
         StrAsc const &get_init() const
         { return init; }

         ////////////////////////////////////////////////////////////
         // set_init
         ////////////////////////////////////////////////////////////
         void set_init(StrAsc const &init_)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            init = init_;
         }

         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

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
         virtual void on_corabase_session_failure()
         { on_corabase_failure(corabase_failure_session); }

         ////////////////////////////////////////////////////////////
         // onNetMessage
         ////////////////////////////////////////////////////////////
         virtual void onNetMessage(
            Csi::Messaging::Router *router, Csi::Messaging::Message *message); 
      };
   };
};


#endif

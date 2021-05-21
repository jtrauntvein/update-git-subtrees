/* Cora.LgrNet.LogMessageAdder.h

   Copyright (C) 2000, 2021 Campbell Scientific, Inc.

   Written by: tmecham
   Last Modified by: $Author: jon $
   Date Begun: Thursday 2 August 2001
   Last Change: Monday 01 March 2021
   Last Commit: $Date: 2021-03-01 17:41:26 -0600 (Mon, 01 Mar 2021) $ 

*/


#ifndef Cora_LgrNet_LogMessageAdder_h
#define Cora_LgrNet_LogMessageAdder_h


#include "Cora.ClientBase.h"
#include "CsiEvents.h"
#include "Csi.InstanceValidator.h"
#include "Csi.LgrDate.h"


namespace Cora
{
   namespace LgrNet
   {
      class LogMessageAdder;

      /**
       * Defines the interface that an application object must implement to use the LogMessageAdder
       * component type.
       */
      class LogMessageAdderClient: public Csi::InstanceValidator
      {
      public:
         /**
          * Called by the component when the transaction has been completed.
          *
          * @param sender Specifies the component reporting this event.
          *
          * @param outcome Reports the outcome of the transaction.
          */
         enum outcome_type
         {
            outcome_unknown = 0,
            outcome_success = 1,
            outcome_session_failed = 2,
            outcome_invalid_logon = 3,
            outcome_server_security_blocked = 4,
            outcome_unsupported_transaction = 5,
            outcome_invalid_log_identifier = 6,
            outcome_invalid_tran_log_id = 7
         };
         virtual void on_complete(LogMessageAdder *adder, outcome_type outcome) = 0;
      };


      /**
       * Defines a component type that can be used to add messages to rhe LoggerNet server's
       * transaction or object state logs.  In order to use this component, the application must
       * provide an object that extends class LogMessageAdderClient.  It must then create an
       * instance of this class, set its properties including log_type, log_message, and,
       * optionally, tran_log_message_type and tran_log_message_desc.  It must then call one of the
       * two versions of start().  When the LoggerNet transaction is complete, the component will
       * call the client's on_complete() method.
       */
      class LogMessageAdder:
         public ClientBase,
         public Csi::EvReceiver
      {
      public:
         enum log_type
         {
            transaction_log = 1,
            object_state_log = 3
         };
         
      private:
         /**
          * Specifies the log message content.
          */
         StrAsc log_message;

         /**
          * Specifies the log type to which the message should be posted.
          */
         log_type log_identifier;

         /**
          * Specifies the time stamp to be assigned to the log message.
          */
         Csi::LgrDate log_date;

         /**
          * Specifies the message type code for the transaction log.
          */
         uint4 tran_log_id;

         /**
          * Specifies the decription string for the transaction log message type.
          */
         StrAsc tran_log_desc;

         /**
          * Specifies the client for this component.
          */
         LogMessageAdderClient *client;

         /**
          * Specifies the state for this component.
          */
         enum state_type
         {
            state_standby,
            state_delegate,
            state_active
         } state;
         
      public:
         /**
          * Constructor
          */
         LogMessageAdder():
            client(0),
            state(state_standby),
            log_identifier(object_state_log),
            tran_log_id(0)
         { }

         /**
          * Destructor
          */
         virtual ~LogMessageAdder()
         { finish(); }

         /**
          * @param value Specifies the value for the log message content.
          */
         void set_log_message(StrAsc const &value)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            log_message  = value;
         }

         /**
          * @return Returns the log message.
          */
         StrAsc const &get_log_message() const
         { return log_message; }

         /**
          * @param value Specifies the log identifier.
          */
         void set_log_identifier(log_type value)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            log_identifier = value;
         }

         /**
          * @return Returns the log identifier.
          */
         log_type get_log_identifier() const
         { return log_identifier; }

         /**
          * @param value Specifies the time stamp for the log message.
          */
         void set_log_date(Csi::LgrDate const &value)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            log_date = value;
         }

         /**
          * @return Returns the intended time stamp for the message.
          */
         Csi::LgrDate const &get_log_date() const
         { return log_date; }
         
         /**
          * @param value Specifies the transaction log message type identifier.
          */
         void set_tran_log_id(uint4 value)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            tran_log_id = value;
         }

         /**
          * @return Returns the transaction log message type identifier.
          */
         uint4 get_tran_log_id() const
         { return tran_log_id; }

         /**
          * @param valuse Specifies the transaction log message type description.
          */
         void set_tran_log_desc(StrAsc const &value)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            tran_log_desc = value;
         }

         /**
          * @return Returns the transaction log message type description.
          */
         StrAsc const &get_tran_log_desc() const
         { return tran_log_desc; }
         
         /**
          * Starts the transaction with the LoggerNet server.
          *
          * @param client_ Specifies the client object reference.
          *
          * @param router Specifies a messaging router that has not been previously connected.
          *
          * @param other_component Specifies a connected component from which this component can borrow
          * the connection
          */
         typedef LogMessageAdderClient client_type;
         void start(client_type *client_, router_handle &router)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("invalid client pointer");
            state = state_delegate;
            client = client_;
            ClientBase::start(router);
         }
         void start(client_type *client_, ClientBase *other_component)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("invalid client pointer");
            state = state_delegate;
            client = client_;
            ClientBase::start(other_component);
         }

         /**
          * Releases resources.
          */
         virtual void finish() override
         {
            client = 0;
            state = state_standby;
            ClientBase::finish();
         }

         /**
          * Overrides the base class version to handle completion.
          */
         virtual void receive(event_handle &ev) override;

         /**
          * Describes the given outcome to the specified stream.
          */
         static void describe_outcome(std::ostream &out, client_type::outcome_type outcome);

      private:
         /**
          * Overrides the base class version to start the transaction.
          */
         virtual void on_corabase_ready() override;

         /**
          * Overrides the base class version to handle a failure.
          */
         virtual void on_corabase_failure(corabase_failure_type failure) override;

         /**
          * Overrides the base class version to handle a session failure.
          */
         virtual void on_corabase_session_failure() override
         { on_corabase_failure(corabase_failure_session); }

         /**
          * Override the base class  version to handle incoming messages.
          */
         virtual void onNetMessage(
            Csi::Messaging::Router *router, Csi::Messaging::Message *message);
      };
   };
};

#endif















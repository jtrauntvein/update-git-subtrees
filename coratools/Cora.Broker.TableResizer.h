/* Cora.Broker.TableResizer.h

   Copyright (C) 2005, 2021 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 20 January 2005
   Last Change: Monday 12 April 2021
   Last Commit: $Date: 2021-04-12 15:37:10 -0600 (Mon, 12 Apr 2021) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Cora_Broker_TableResizer_h
#define Cora_Broker_TableResizer_h

#include "Cora.Broker.BrokerBase.h"


namespace Cora
{
   namespace Broker
   {
      class TableResizer;


      /**
       * Defines the interface that the application must implement in order to use the TableResizer
       * component type.
       */
      class TableResizerClient: public Csi::InstanceValidator
      {
      public:
         /**
          * Called when the server tranaxtion has been completed.
          *
          * @param sender Specifies the component reporting this event.
          * @param outcome Specifies the outcome of the server transactiuon.
          */
         enum outcome_type
         {
            outcome_unknown = 0,
            outcome_success = 1,
            outcome_invalid_logon = 2,
            outcome_connection_failed = 3,
            outcome_invalid_station_name = 4,
            outcome_unsupported = 5,
            outcome_server_security_blocked = 6,
            outcome_invalid_table_name = 7,
            outcome_invalid_size = 8,
            outcome_insufficient_resources = 9
         };
         virtual void on_complete(TableResizer *sender, outcome_type outcome) = 0; 
      };


      /**
       * Defines a class that can be used as a component to change the number of records that can be
       * stored in a cache table.  This will not effect the original datalogger table definition.
       * In order to use this component, an application must provide a client object that is derived
       * from class TableResizerClient.  It must then create an instance of class TableResizer,
       * invoke methods to set class properties including set_table_size(), and invoke one of the
       * two versions of start().  When the server transaction is complete, the client's
       * on_complete() method will be invoked.
       */
      class TableResizer:
         public BrokerBase, public Csi::EventReceiver
      {
      private:
         /**
          * Specifies the name of the table on which to operate.
          */
         StrUni table_name;

         /**
          * Specifies the number of records to allocate for the table.
          */
         uint4 table_size;

         /**
          * Specifies the application object that will receive the completion notification  event.
          */
         TableResizerClient *client;

         /**
          * Specifies the state of this componnet.
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
         TableResizer():
            client(0),
            state(state_standby),
            table_size(0)
         { }

         /**
          * Destructor
          */
         virtual ~TableResizer()
         { finish(); }

         /**
          * @param table_name_ Specifies the name of the table.
          */
         void set_table_name(StrUni const &table_name_)
         {
            if(state == state_standby)
               table_name = table_name_;
            else
               throw exc_invalid_state();
         }

         /**
          * @return Returns the name of the table.
          */
         StrUni const &get_table_name() const
         { return table_name; }

         /**
          * @param value Specifies the number of records to allocate for the table.
          */
         void set_table_size(uint4 value)
         {
            if(state == state_standby)
               table_size = value;
            else
               throw exc_invalid_state();
         }

         /**
          * Connects to LoggerNet and starts the transaction.
          *
          * @param client_ Specifies the application object that will recieve the completion
          * notification.
          * @param router Specifies a router that has been created but not yet connected to
          * LoggerNet.
          * @param other_component Specifies a component that already has a connection to LoggerNet
          * that this component can borrow.
          */
         typedef TableResizerClient client_type;
         void start(
            client_type *client_,
            router_handle &router)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("Invalid client pointer");
            if(table_size == 0)
               throw std::invalid_argument("invalid table size");
            client = client_;
            state = state_delegate;
            BrokerBase::start(router);
         }
         void start(
            client_type *client_,
            ClientBase *other_component)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("Invalid client pointer");
            if(table_size == 0)
               throw std::invalid_argument("invalid table size");
            client = client_;
            state = state_delegate;
            BrokerBase::start(other_component);
         }

         /**
          * Overrides the base class version in order to reset state and release resources.
          */
         virtual void finish() override
         {
            state = state_standby;
            client = 0;
            BrokerBase::finish();
         }

         /**
          * Overrides the base class version to handle asynch events.
          */
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev) override;

         /**
          * Writes a description of the specified outcome to the given output stream,
          */
         static void describe_outcome(std::ostream &out, client_type::outcome_type outcome);

      protected:
         /**
          * Overrides the base class version to handle the notification that the session with the
          * broker is ready.
          */
         virtual void on_brokerbase_ready() override;

         /**
          * Overrides the base class version to handle LoggerNet messages.
          */
         virtual void onNetMessage(
            Csi::Messaging::Router *rtr, Csi::Messaging::Message *msg) override;

         /**
          * Overrides the base class version to handle notification of failure.
          */
         virtual void on_brokerbase_failure(brokerbase_failure_type failure) override;
         virtual void on_brokerbase_session_failure() override
         { on_brokerbase_failure(brokerbase_failure_session); } 
      };
   };
};


#endif

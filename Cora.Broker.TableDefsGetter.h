/* Cora.Broker.TableDefsGetter.h

   Copyright (C) 2004, 2021 Campbell Scientific, Inc.

   Written by: Tyler Mecham
   Date Begun: Thursday 22 January 2004
   Last Change: Monday 12 April 2021
   Last Commit: $Date: 2021-04-12 09:23:10 -0600 (Mon, 12 Apr 2021) $ 
   Committed by: $Author: jon $
 
*/

#ifndef Cora_Broker_TableDefsGetter_h
#define Cora_Broker_TableDefsGetter_h

#include "Cora.Broker.BrokerBase.h"
#include "Cora.Broker.TableDesc.h"
#include "Csi.Events.h"


namespace Cora
{
   namespace Broker
   {
      class TableDefsGetter;

      /**
       * Defines the interface that must be implemented by an application object in order to use the
       * TableDefsGetter component.
       */
      class TableDefsGetterClient: public Csi::InstanceValidator
      {
      public:
         /**
          * Called when the LoggerNet transaction has been completed.
          *
          * @param sender Specifies the component that is sending this event.
          * @param outcome Specifies the outcome of the server transaction.
          * @param table_defs Specifies the table description.  This will only be a valid object if
          * the value of outcome is outcome_success.
          */
         typedef Csi::SharedPtr<Cora::Broker::TableDesc> table_defs_type;
         enum outcome_type
         {
            outcome_unknown = 0,
            outcome_success = 1,
            outcome_invalid_table = 2,
            outcome_invalid_logon = 3,
            outcome_connection_failed = 4,
            outcome_invalid_station_name = 5,
            outcome_unsupported = 6,
            outcome_server_security_blocked = 7
         };
         virtual void on_complete(
            TableDefsGetter *getter, outcome_type outcome, table_defs_type &table_defs) = 0;
      };


      /**
       * Defines a component that can be used to retrieve the table definitions from a data broker.
       * In order to use this component, the application must provide a client object that
       * implements the TableDefsGetterClient interface.  It should then create an instance of this
       * class, set its properties including data broker ID and table name, and then call one of the
       * two instances of start().  When the transaction has completed, the component will call the
       * client's on_complete() method.
       */
      class TableDefsGetter: public BrokerBase, public Csi::EventReceiver
      {
      private:
         /**
          * Specifies the state of this component.
          */
         enum state_type
         {
            state_standby,
            state_delegate,
            state_active,
         } state;

         /**
          * Specifies the application object that will receive completion notification.
          */
         TableDefsGetterClient *client;

         /**
          * Specifies the name of the table.
          */
         StrUni table_name;
         
      public:
         /**
          * Constructor
          */
         TableDefsGetter():
            state(state_standby),
            client(0)
         { }

         /**
          * Destructor
          */
         virtual ~TableDefsGetter()
         { finish(); }

         /**
          * @param value Specifies the name of the table.
          */
         void set_table_name(StrUni const &value)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            table_name = value;
         }

         /**
          * @return Returns the name of the table.
          */
         StrUni const &get_table_name() const
         { return table_name; }

         /**
          * Called to connect to the server and start the transaction.
          *
          * @param client_ Specifies the application object that will receive completion
          * notifications.
          * @param router Specifies a router that has been created but not yet connected to
          * LoggerNet.
          * @param other_component Specifies a component that already has a connection to LogegrNet
          * that can be borrowed by this component.
          */
         typedef TableDefsGetterClient client_type;
         void start(client_type *client_, router_handle &router)
         {
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("invalid client pointer");
            if(state != state_standby)
               throw exc_invalid_state();
            client = client_;
            state = state_standby;
            BrokerBase::start(router);
         }
         void start(client_type *client_, ClientBase *other_component)
         {
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("invalid client pointer");
            if(state != state_standby)
               throw exc_invalid_state();
            client = client_;
            state = state_standby;
            BrokerBase::start(other_component);
         }

         /**
          * Overloads the base class version to return this component to a standby state.
          */
         virtual void finish() override
         {
            client = 0;
            state = state_standby;
            BrokerBase::finish();
         }

         /**
          * Overrides the base class version to handle asynch events.
          */
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev) override;

         /**
          * Formats a string to the stream to describe the specified outcome code.
          */
         static void describe_outcome(std::ostream &out, client_type::outcome_type outcome);
        
      protected:
         /**
          * Handles the notification that the session with the broker is ready.
          */
         virtual void on_brokerbase_ready() override;

         /**
          * Handles the notification that something has gone wrong with the data broker session.
          */
         virtual void on_brokerbase_failure(brokerbase_failure_type failure) override;

         /**
          * Handles the notification that the session with the data broker has failed.
          */
         virtual void on_brokerbase_session_failure() override;
         
         /**
          * Handles incoming messages from the LoggerNet server.
          */
         virtual void onNetMessage(Csi::Messaging::Router *rtr, Csi::Messaging::Message *msg) override;
      };
   };
};

#endif

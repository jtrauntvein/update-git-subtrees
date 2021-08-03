/* Cora.Broker.TableLister.h

   Copyright (C) 2000, 2008 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 08 August 2000
   Last Change: Thursday 18 September 2008
   Last Commit: $Date: 2019-10-29 15:49:38 -0600 (Tue, 29 Oct 2019) $ 
   Committed by: $Author: amortenson $
   
*/

#ifndef Cora_Broker_TableLister_h
#define Cora_Broker_TableLister_h

#include "Cora.Broker.BrokerBase.h"
#include "Csi.InstanceValidator.h"
#include "Csi.Events.h"


namespace Cora
{
   namespace Broker
   {
      //@group class forward declarations
      class TableLister;
      //@endgroup

      ////////////////////////////////////////////////////////////
      // class TableListerClient
      //
      // Defines the interface that should be implemented by a client object to the TableLister
      // class.
      ////////////////////////////////////////////////////////////
      class TableListerClient: public Csi::InstanceValidator
      {
      public:
         ////////// on_started
         // Called when all existing table names have been sent.
         ////////////////////////////////////////////////////////////
         // on_started
         //
         // Called when all of the existing table names have been sent.
         ////////////////////////////////////////////////////////////
         virtual void on_started(TableLister *lister)
         { }

         ////////////////////////////////////////////////////////////
         // on_failure
         //
         // Called when an error has occurred that would prevent the server transaction from
         // continuing. 
         ////////////////////////////////////////////////////////////
         enum failure_type
         {
            failure_unknown = 0,
            failure_invalid_broker_id = 1,
            failure_invalid_logon = 2,
            failure_session_failed = 3,
            failure_unsupported = 4,
            failure_server_security_blocked = 5, 
         };
         virtual void on_failure(
            TableLister *lister,
            failure_type failure) = 0;

         ////////////////////////////////////////////////////////////
         // on_table_added
         //
         // called when a new table name has been added to the broker.
         ////////////////////////////////////////////////////////////
         virtual void on_table_added(
            TableLister *lister,
            StrUni const &table_name)
         { }

         ////////////////////////////////////////////////////////////
         // on_table_deleted
         //
         // Called when a table has been deleted from the broker.
         ////////////////////////////////////////////////////////////
         virtual void on_table_deleted(
            TableLister *lister,
            StrUni const &table_name)
         { }
      };


      ////////////////////////////////////////////////////////////
      // class TableLister
      //
      // Implements a class that lists the tables associated with a data broker. This class is used
      // by creating an instance of it. The broker can be specified by calling
      // set_open_active_broker_name() or by calling set_opeb_broker_id(). The start() method should
      // then be invoked. If the server transaction can continue after start() has been invoked, the
      // client's on_table_added() method will be invoked for each table defined by the broker until
      // all existing tables have been listed. The client's on_started() method will then be
      // invoked. Thereafter, the client will receive on_table_added() or on_table_deleted()
      // whenever such an event occurs on the data broker.
      //
      // The client's on_failure() method can be invoked at any time following an invocation of
      // start() to report a failure. Following this, the TableLister object will return to a
      // standby state.
      ////////////////////////////////////////////////////////////
      class TableLister: public BrokerBase, public Csi::EventReceiver
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         TableLister();

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~TableLister();

         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         typedef TableListerClient client_type;
         void start(
            client_type *client, router_handle &router);
         void start(
            client_type *client, ClientBase *other_component);

         ////////////////////////////////////////////////////////////
         // finish
         ////////////////////////////////////////////////////////////
         void finish();

         ////////////////////////////////////////////////////////////
         // format_failure
         ////////////////////////////////////////////////////////////
         static void format_failure(std::ostream &out, client_type::failure_type failure);

         ////////////////////////////////////////////////////////////
         // set_send_temporaries
         ////////////////////////////////////////////////////////////
         void set_send_temporaries(bool send_temporaries);

         ////////////////////////////////////////////////////////////
         // get_send_temporaries
         ////////////////////////////////////////////////////////////
         bool get_send_temporaries()
         { return send_temporaries; }

      protected:
         //@group class BrokerBase overloaded methods
         ////////////////////////////////////////////////////////////
         // onNetMessage
         ////////////////////////////////////////////////////////////
         virtual void onNetMessage(
            Csi::Messaging::Router *rtr, Csi::Messaging::Message *msg);

         ////////////////////////////////////////////////////////////
         // on_brokerbase_ready
         ////////////////////////////////////////////////////////////
         virtual void on_brokerbase_ready();

         ////////////////////////////////////////////////////////////
         // on_brokerbase_failure
         ////////////////////////////////////////////////////////////
         virtual void on_brokerbase_failure(brokerbase_failure_type failure);

         ////////////////////////////////////////////////////////////
         // on_brokerbase_session_failure
         ////////////////////////////////////////////////////////////
         virtual void on_brokerbase_session_failure(); 
         //@endgroup

         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

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
            state_before_active,
            state_active
         } state;

         ////////////////////////////////////////////////////////////
         // send_temporaries
         ////////////////////////////////////////////////////////////
         bool send_temporaries;
      };
   };
};

#endif

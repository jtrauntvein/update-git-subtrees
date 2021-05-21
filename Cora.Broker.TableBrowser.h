/* Cora.Broker.TableBrowser.h

   Copyright (C) 2000, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 30 May 2000
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Cora_Broker_TableBrowser_h
#define Cora_Broker_TableBrowser_h

#include "Cora.Broker.BrokerBase.h"
#include "Csi.InstanceValidator.h"
#include "Cora.Broker.TableDesc.h"
#include "CsiEvents.h"
#include <map>


namespace Cora
{
   namespace Broker
   {
      //@group class forward declarations
      class TableBrowser;
      //@endgroup


      ////////////////////////////////////////////////////////////
      // class TableBrowserClient
      //
      // Defines the interface that must be implemented by an object that is to be a client of a
      // TableBrowser object
      ////////////////////////////////////////////////////////////
      class TableBrowserClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_all_started
         //
         // Called by the browser after the table definitions enumerate transaction has been started
         // and the definitions for all known tables have been loaded.
         ////////////////////////////////////////////////////////////
         virtual void on_all_started(TableBrowser *tran) { }

         ////////////////////////////////////////////////////////////
         // on_table_added
         //
         // Called when the browser has detected that a new table has been added 
         ////////////////////////////////////////////////////////////
         virtual void on_table_added(
            TableBrowser *tran,
            Csi::SharedPtr<TableDesc> &table)
         { }

         ////////////////////////////////////////////////////////////
         // on_table_deleted
         //
         // Called when the browser has detected that a table has been deleted.
         ////////////////////////////////////////////////////////////
         virtual void on_table_deleted(
            TableBrowser *tran,
            Csi::SharedPtr<TableDesc> &table)
         { }

         ////////////////////////////////////////////////////////////
         // on_failure
         //
         // Called when a failure has occurred that will place the browser back into a standby state
         ////////////////////////////////////////////////////////////
         enum failure_type
         {
            failure_unknown = 1,
            failure_connection_failed = 2,
            failure_invalid_logon = 3,
            failure_invalid_broker_id = 4,
            failure_broker_deleted = 5, 
         };
         virtual void on_failure(TableBrowser *tran, failure_type failure)
         { }
      };


      ////////////////////////////////////////////////////////////
      // class TableBrowser
      //
      // Defines a class that, while in a started state, maintains a list of all table definitions
      // associated with a specified data broker. In order to receive notifications of events
      // associated with this class, a client object must be derived from class
      // Cora::Broker::TableBrowserClient.
      ////////////////////////////////////////////////////////////
      class TableBrowser: public BrokerBase, public Csi::EvReceiver
      {
      private:
         ////////////////////////////////////////////////////////////
         // send_temporaries
         //
         // Stores the property value that controls whether temporary tables are reported from the
         // server. The default value for this property is false but can be changed by calling
         // set_send_temporaries().
         ////////////////////////////////////////////////////////////
         bool send_temporaries;
         
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         TableBrowser();

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~TableBrowser();

         ////////////////////////////////////////////////////////////
         // send_temporaries access methods
         ////////////////////////////////////////////////////////////
         bool get_send_temporaries() const { return send_temporaries; }
         void set_send_temporaries(bool send_temporaries_);
         
         ////////////////////////////////////////////////////////////
         // start
         //
         // Engages in the neccesary transactions to logon and attach to the appropriate data broker
         // (specified through the base class' open_broker_active_name or open_broker_id
         // properties. When the initial table definitions have been received, the client's
         // on_all_started() method will be invoked. As table definition additions or deletions take
         // place, the client's on_table_added() or on_table_deleted() method will be invoked.
         ////////////////////////////////////////////////////////////
         virtual void start(
            TableBrowserClient *client_,
            router_handle &router);
         virtual void start(
            TableBrowserClient *client_,
            ClientBase *other_component);

         ////////////////////////////////////////////////////////////
         // finish
         ////////////////////////////////////////////////////////////
         virtual void finish();

         //@group Table definition access methods
         typedef std::list<Csi::SharedPtr<TableDesc> > tables_type;
         typedef tables_type::const_iterator const_iterator;
         typedef tables_type::const_reverse_iterator const_reverse_iterator;
         typedef tables_type::size_type size_type;
         const_iterator begin() const { return tables.begin(); }
         const_iterator end() const { return tables.end(); }
         const_reverse_iterator rbegin() const { return tables.rbegin(); }
         const_reverse_iterator rend() const { return tables.rend(); }
         size_type size() const { return tables.size(); }
         bool empty() const { return tables.empty(); }
         //@endgroup

      protected:
         //@group Methods overloaded from class BrokerBase
         ////////////////////////////////////////////////////////////
         // onNetMessage
         ////////////////////////////////////////////////////////////
         virtual void onNetMessage(
            Csi::Messaging::Router *rtr,
            Csi::Messaging::Message *msg);

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
         // on_table_defs_enum_not
         ////////////////////////////////////////////////////////////
         void on_table_defs_enum_not(Csi::Messaging::Message *msg);

         ////////////////////////////////////////////////////////////
         // on_table_def_get_ack
         ////////////////////////////////////////////////////////////
         void on_table_def_get_ack(Csi::Messaging::Message *msg);

         ////////////////////////////////////////////////////////////
         // process_table_change
         //
         // Does the work of deciding how to react to a notification
         ////////////////////////////////////////////////////////////
         void process_table_change(StrUni const &table_name, uint4 change_code);

      private:
         ////////////////////////////////////////////////////////////
         // state
         //
         // Records the present state of this object (associated with this class)
         ////////////////////////////////////////////////////////////
         enum state_type
         {
            state_standby,      // ready to be started
            state_attach,       // attaching to the broker
            state_first_set,    // receiving the first set of table definitions
            state_steady,       // waiting for further notifications
         } state;

         ////////////////////////////////////////////////////////////
         // client
         //
         // Pointer to the object that will receive event notifications. This member is set by
         // start().
         ////////////////////////////////////////////////////////////
         TableBrowserClient *client;

         ////////////////////////////////////////////////////////////
         // tables
         //
         // List of all tables currently known for this broker
         ////////////////////////////////////////////////////////////
         tables_type tables;

         ////////////////////////////////////////////////////////////
         // waiting_trans
         //
         // Contains all descriptions that still need to be filled in by a table definition get
         // transaction. This container is keyed by the transaction number.
         ////////////////////////////////////////////////////////////
         typedef std::map<uint4, Csi::SharedPtr<TableDesc> > waiting_trans_type;
         waiting_trans_type waiting_trans;
      };
   };
};

#endif

/* Cora.LgrNet.BrokerBrowser2.h

   Copyright (C) 2004, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 19 January 2004
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Cora_LgrNet_BrokerBrowser2_h
#define Cora_LgrNet_BrokerBrowser2_h

#include "Cora.LgrNet.BrokerLister.h"
#include "Cora.Device.ScheduledTablesMonitor.h"
#include "Cora.Broker.TableLister.h"
#include "Cora.Broker.TableDefsGetter.h"


namespace Cora
{
   namespace LgrNet
   {
      //@group forward declarations
      class BrokerBrowser2;
      class BrokerInfo;
      class TableInfo;
      typedef Csi::SharedPtr<BrokerInfo> broker_info_type;
      typedef Csi::SharedPtr<TableInfo> table_info_type;
      //@endgroup

      ////////////////////////////////////////////////////////////
      // class BrokerBrowser2Client
      ////////////////////////////////////////////////////////////
      class BrokerBrowser2Client :
         public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_server_connect_started
         ////////////////////////////////////////////////////////////
         virtual void on_server_connect_started(BrokerBrowser2 *manager) = 0;

         ////////////////////////////////////////////////////////////
         // on_server_connect_failed
         ////////////////////////////////////////////////////////////
         enum failure_type
         {
            failure_unknown = 0,
            failure_connection_failed = 1,
            failure_invalid_logon = 2,
            failure_server_security = 3,
         };
         virtual void on_server_connect_failed(
            BrokerBrowser2 *manager, 
            failure_type failure) = 0;

         ////////////////////////////////////////////////////////////
         // on_brokers_changed
         ////////////////////////////////////////////////////////////
         virtual void on_brokers_changed() = 0;

         ////////////////////////////////////////////////////////////
         // on_tables_changed
         ////////////////////////////////////////////////////////////
         virtual void on_tables_changed(StrUni const &broker_name) = 0;

         ////////////////////////////////////////////////////////////
         // on_table_defs_updated
         ////////////////////////////////////////////////////////////
         virtual void on_table_defs_updated(
            StrUni const &broker_name, 
            StrUni const &table_name) = 0;

         ////////////////////////////////////////////////////////////
         // get_notify_specifics
         //
         // Allows the client to say they want updates for every item
         // changed instead of general change notifications.
         ////////////////////////////////////////////////////////////
         virtual bool get_notify_specifics()
         { return false; }

         ////////////////////////////////////////////////////////////
         // on_broker_added
         //
         // only sent if get_notify_specifics() returns true
         ////////////////////////////////////////////////////////////
         virtual void on_broker_added(
            StrUni const &broker_name)
         { }

         ////////////////////////////////////////////////////////////
         // on_broker_started
         //
         // only sent if get_notify_specifics() returns true
         ////////////////////////////////////////////////////////////
         virtual void on_broker_started(
            StrUni const &broker_name)
         { }

         ////////////////////////////////////////////////////////////
         // on_broker_deleted
         //
         // only sent if get_notify_specifics() returns true
         ////////////////////////////////////////////////////////////
         virtual void on_broker_deleted(
            StrUni const &broker_name)
         { }

         ////////////////////////////////////////////////////////////
         // on_table_added
         //
         // only sent if get_notify_specifics() returns true
         ////////////////////////////////////////////////////////////
         virtual void on_table_added(
            StrUni const &broker_name,
            StrUni const &table_name)
         { }

         ////////////////////////////////////////////////////////////
         // on_table_deleted
         //
         // only sent if get_notify_specifics() returns true
         ////////////////////////////////////////////////////////////
         virtual void on_table_deleted(
            StrUni const &broker_name,
            StrUni const &table_name)
         { }
      };


      ////////////////////////////////////////////////////////////
      // BrokerBrowser2
      ////////////////////////////////////////////////////////////
      class BrokerBrowser2 :
         public Cora::LgrNet::BrokerListerClient
      {
      public:
         ////////////////////////////////////////////////////////////
         // Constructor
         ////////////////////////////////////////////////////////////
         BrokerBrowser2();

         ////////////////////////////////////////////////////////////
         // Destructor
         ////////////////////////////////////////////////////////////
         ~BrokerBrowser2();

         ////////////////////////////////////////////////////////////
         // set_logon_name
         ////////////////////////////////////////////////////////////
         void set_logon_name(StrUni const &logon_name_)
         { logon_name = logon_name_; }

         ////////////////////////////////////////////////////////////
         // set_logon_password
         ////////////////////////////////////////////////////////////
         void set_logon_password(StrUni const &logon_password_)
         { logon_password = logon_password_; }
         
         ////////////////////////////////////////////////////////////
         // set_application_name
         ////////////////////////////////////////////////////////////
         void set_application_name(StrUni const &application_name_)
         { application_name = application_name_; }
         
         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         typedef Csi::SharedPtr<Csi::Messaging::Router> router_handle;
         typedef BrokerBrowser2Client client_type;
         void start(
            client_type *client_,
            router_handle &router);
         void start(
            client_type *client_,
            ClientBase *other_component);

         ////////////////////////////////////////////////////////////
         // get_connection
         ////////////////////////////////////////////////////////////
         ClientBase *get_connection(){return broker_lister.get_rep();}

         //@group Cora::LgrNet::BrokerListerClient
         ////////////////////////////////////////////////////////////
         // on_started
         //
         // Called after the initial set of broker names have been received and the transaction is
         // in a steady state.
         //////////////////////////////////////////////////////////// 
         virtual void on_started(BrokerLister *lister);

         ////////////////////////////////////////////////////////////
         // on_failure
         //
         // Called at any time after start() has been invoked and returned to report an
         // unrecoverable failure
         //////////////////////////////////////////////////////////// 
         virtual void on_failure(BrokerLister *lister, failure_type failure);

         ////////////////////////////////////////////////////////////
         // on_broker_added
         //
         // called when the lister has become aware of a new broker object that matches the mask
         //////////////////////////////////////////////////////////// 
         virtual void on_broker_added(
            BrokerLister *lister,
            StrUni const &broker_name,
            uint4 broker_id,
            broker_type_code type);

         ////////////////////////////////////////////////////////////
         // on_broker_deleted
         //
         // Called when the lister has become aware that a databroker object has been deleted
         //////////////////////////////////////////////////////////// 
         virtual void on_broker_deleted(
            BrokerLister *lister,
            StrUni const &broker_name,
            uint4 broker_id,
            broker_type_code type);

         ////////////////////////////////////////////////////////////
         // on_broker_renamed
         ////////////////////////////////////////////////////////////
         virtual void on_broker_renamed(
            BrokerLister *lister,
            StrUni const &old_broker_name,
            StrUni const &new_broker_name,
            uint4 broker_id,
            broker_type_code type);
         //@endgroup

         ////////////////////////////////////////////////////////////
         // brokers
         ////////////////////////////////////////////////////////////
         typedef std::map<StrUni,broker_info_type> brokers_type;
         brokers_type brokers;

         ////////////////////////////////////////////////////////////
         // classic_inlocs_table
         ////////////////////////////////////////////////////////////
         static const StrUni classic_inlocs_table;

         //@group brokers list access methods
         ////////////////////////////////////////////////////////////
         // begin
         ////////////////////////////////////////////////////////////
         typedef brokers_type::iterator iterator;
         typedef brokers_type::const_iterator const_iterator;
         iterator begin() { return brokers.begin(); }
         const_iterator begin() const { return brokers.end(); }

         ////////////////////////////////////////////////////////////
         // end
         ////////////////////////////////////////////////////////////
         iterator end() { return brokers.end(); }
         const_iterator end() const { return brokers.end(); }

         ////////////////////////////////////////////////////////////
         // find
         ////////////////////////////////////////////////////////////
         iterator find(StrUni const &broker_name)
         { return brokers.find(broker_name); }

         ////////////////////////////////////////////////////////////
         // empty
         ////////////////////////////////////////////////////////////
         bool empty() const
         { return brokers.empty(); }

         ////////////////////////////////////////////////////////////
         // size
         ////////////////////////////////////////////////////////////
         brokers_type::size_type size() const
         { return brokers.size(); }
         //@endgroup

         ////////////////////////////////////////////////////////////
         // get_all_started
         ////////////////////////////////////////////////////////////
         bool get_all_started()
         { return all_started; }

      private:
         ////////////////////////////////////////////////////////////
         // client
         ////////////////////////////////////////////////////////////
         client_type *client;

         ////////////////////////////////////////////////////////////
         // broker_lister
         ////////////////////////////////////////////////////////////
         typedef Csi::SharedPtr<Cora::LgrNet::BrokerLister> broker_lister_type;
         broker_lister_type broker_lister;

         ////////////////////////////////////////////////////////////
         // all_started
         ////////////////////////////////////////////////////////////
         bool all_started;

         ////////////////////////////////////////////////////////////
         // application_name
         ////////////////////////////////////////////////////////////
         StrUni application_name;

         ////////////////////////////////////////////////////////////
         // logon_name
         ////////////////////////////////////////////////////////////
         StrUni logon_name;

         ////////////////////////////////////////////////////////////
         // logon_password
         ////////////////////////////////////////////////////////////
         StrUni logon_password;
      };


      ////////////////////////////////////////////////////////////
      // class BrokerInfo
      ////////////////////////////////////////////////////////////
      class BrokerInfo :
         public Cora::Device::ScheduledTablesMonitorClient,
         public Cora::Broker::TableListerClient
      {
      public:
         ////////////////////////////////////////////////////////////
         // Constructor
         ////////////////////////////////////////////////////////////
         BrokerInfo();

         ////////////////////////////////////////////////////////////
         // Destructor
         ////////////////////////////////////////////////////////////
         ~BrokerInfo();

         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         typedef Csi::SharedPtr<Csi::Messaging::Router> router_handle;
         typedef BrokerBrowser2Client client_type;
         void start(
            client_type *client_,
            router_handle &router);
         void start(
            client_type *client_,
            ClientBase *other_component);

         //@group Cora::Broker::TableListerClient
         ////////////////////////////////////////////////////////////
         // on_started
         //
         // Called when all existing table names have been sent.
         ////////////////////////////////////////////////////////////
         virtual void on_started(Cora::Broker::TableLister *lister);

         ////////////////////////////////////////////////////////////
         // on_failure
         //
         // Called when an error has occurred that would prevent the transaction from continuing
         ////////////////////////////////////////////////////////////
         virtual void on_failure(
            Cora::Broker::TableLister *lister,
            Cora::Broker::TableListerClient::failure_type failure);

         ////////////////////////////////////////////////////////////
         // on_table_added
         //
         // Called when a table name has been added to the broker
         ////////////////////////////////////////////////////////////
         virtual void on_table_added(
            Cora::Broker::TableLister *lister,
            StrUni const &table_name);

         ////////////////////////////////////////////////////////////
         // on_table_deleted
         //
         // Called when a table has been deleted by the broker
         ////////////////////////////////////////////////////////////
         virtual void on_table_deleted(
            Cora::Broker::TableLister *lister,
            StrUni const &table_name);
         //@endgroup

         //@group Cora::Device::ScheduledTablesMonitorClient
         ////////////////////////////////////////////////////////////
         // on_started
         //
         // Called when all existing table names have been sent.
         ////////////////////////////////////////////////////////////
         virtual void on_started(Cora::Device::ScheduledTablesMonitor *monitor);

         ////////////////////////////////////////////////////////////
         // on_failure
         //
         // Called when an error has occurred that would prevent the transaction from continuing
         ////////////////////////////////////////////////////////////
         virtual void on_failure(
            Cora::Device::ScheduledTablesMonitor *monitor,
            Cora::Device::ScheduledTablesMonitorClient::failure_type failure);

         ////////////////////////////////////////////////////////////
         // on_table_added
         //
         // Called when a table name has been added to the broker
         ////////////////////////////////////////////////////////////
         virtual void on_table_added(
            Cora::Device::ScheduledTablesMonitor *monitor,
            StrUni const &table_name,
            bool scheduled);

         ////////////////////////////////////////////////////////////
         // on_table_deleted
         //
         // Called when a table has been deleted by the broker
         ////////////////////////////////////////////////////////////
         virtual void on_table_deleted(
            Cora::Device::ScheduledTablesMonitor *monitor,
            StrUni const &table_name);
         
         ////////////////////////////////////////////////////////////
         // on_table_scheduled
         //
         // Called when a table has been enabled for scheduled collection
         ////////////////////////////////////////////////////////////
         virtual void on_table_scheduled(
            Cora::Device::ScheduledTablesMonitor *monitor,
            StrUni const &table_name,
            bool scheduled);
         //@endgroup

         ////////////////////////////////////////////////////////////
         // broker_name
         ////////////////////////////////////////////////////////////
         StrUni broker_name;

         ////////////////////////////////////////////////////////////
         // broker_id
         ////////////////////////////////////////////////////////////
         uint4 broker_id;

         ////////////////////////////////////////////////////////////
         // broker_type
         ////////////////////////////////////////////////////////////
         typedef Cora::Broker::Type::Code broker_type_code;
         broker_type_code broker_type;

         ////////////////////////////////////////////////////////////
         // tables
         ////////////////////////////////////////////////////////////
         typedef std::map<StrUni,table_info_type> tables_type;
         tables_type tables;

         //@group tables container access methods
         ////////////////////////////////////////////////////////////
         // begin
         ////////////////////////////////////////////////////////////
         typedef tables_type::iterator iterator;
         typedef tables_type::const_iterator const_iterator;
         iterator begin() { return tables.begin(); }
         const_iterator begin() const { return tables.begin(); }

         ////////////////////////////////////////////////////////////
         // end
         ////////////////////////////////////////////////////////////
         iterator end() { return tables.end(); }
         const_iterator end() const { return tables.end(); }

         ////////////////////////////////////////////////////////////
         // find
         ////////////////////////////////////////////////////////////
         iterator find(StrUni const &table_name)
         { return tables.find(table_name); }

         ////////////////////////////////////////////////////////////
         // empty
         ////////////////////////////////////////////////////////////
         bool empty() const
         { return tables.empty(); }

         ////////////////////////////////////////////////////////////
         // size
         ////////////////////////////////////////////////////////////
         tables_type::size_type size() const
         { return tables.size(); }
         //@endgroup

         ////////////////////////////////////////////////////////////
         // get_all_started
         ////////////////////////////////////////////////////////////
         bool get_all_started()
         { return all_started; }

      private:
         ////////////////////////////////////////////////////////////
         // device_table_lister
         ////////////////////////////////////////////////////////////
         typedef Csi::SharedPtr<Cora::Device::ScheduledTablesMonitor> device_table_lister_type;
         device_table_lister_type device_table_lister;

         ////////////////////////////////////////////////////////////
         // other_table_lister
         ////////////////////////////////////////////////////////////
         typedef Csi::SharedPtr<Cora::Broker::TableLister> other_table_lister_type;
         other_table_lister_type other_table_lister;

         ////////////////////////////////////////////////////////////
         // client
         ////////////////////////////////////////////////////////////
         client_type *client;

         ////////////////////////////////////////////////////////////
         // all_started
         ////////////////////////////////////////////////////////////
         bool all_started;
      };


      ////////////////////////////////////////////////////////////
      // class TableInfo
      ////////////////////////////////////////////////////////////
      class TableInfo:
         public Cora::Broker::TableDefsGetterClient,
         public Csi::EventReceiver
      {
      public:
         ////////////////////////////////////////////////////////////
         // Constructor
         ////////////////////////////////////////////////////////////
         typedef Cora::Device::ScheduledTablesMonitor::input_location_identifiers_type input_location_identifiers_type;
         TableInfo(
            StrUni const &broker_name_,
            StrUni const &table_name_,
            input_location_identifiers_type input_location_identifiers_ = 0);

         ////////////////////////////////////////////////////////////
         // Destructor
         ////////////////////////////////////////////////////////////
         ~TableInfo();

         //@group Csi::EventReceiver
         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &event);
         //@endgroup

         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         typedef Csi::SharedPtr<Csi::Messaging::Router> router_handle;
         void start(
            BrokerBrowser2Client *client_,
            router_handle &router);
         void start(
            BrokerBrowser2Client *client_,
            ClientBase *other_component);

         //@group Cora::Broker::TableDefsGetterClient
         ////////////////////////////////////////////////////////////
         // on_complete
         ////////////////////////////////////////////////////////////
         virtual void on_complete(
            Cora::Broker::TableDefsGetter *getter,
            Cora::Broker::TableDefsGetterClient::outcome_type outcome,
            Cora::Broker::TableDefsGetterClient::table_defs_type &table_defs);
         //@endgroup

         ////////////////////////////////////////////////////////////
         // broker_name
         ////////////////////////////////////////////////////////////
         StrUni broker_name;

         ////////////////////////////////////////////////////////////
         // table_name
         ////////////////////////////////////////////////////////////
         StrUni table_name;

         ////////////////////////////////////////////////////////////
         // table_scheduled
         ////////////////////////////////////////////////////////////
         bool table_scheduled;

         ////////////////////////////////////////////////////////////
         // used_generic_inlocs
         ////////////////////////////////////////////////////////////
         bool used_generic_inlocs;

         ////////////////////////////////////////////////////////////
         // table_defs
         ////////////////////////////////////////////////////////////
         typedef Cora::Broker::TableDefsGetterClient::table_defs_type table_defs_type;
         table_defs_type table_defs;

         ////////////////////////////////////////////////////////////
         // input_location_identifiers
         ////////////////////////////////////////////////////////////
         input_location_identifiers_type input_location_identifiers;

         ////////////////////////////////////////////////////////////
         // get_all_started
         ////////////////////////////////////////////////////////////
         bool get_all_started()
         { return all_started; }

      private:
         ////////////////////////////////////////////////////////////
         // table_defs_getter
         ////////////////////////////////////////////////////////////
         typedef Csi::SharedPtr<Cora::Broker::TableDefsGetter> table_defs_getter_type;
         table_defs_getter_type table_defs_getter;

         ////////////////////////////////////////////////////////////
         // client
         ////////////////////////////////////////////////////////////
         BrokerBrowser2Client *client;

         ////////////////////////////////////////////////////////////
         // all_started
         ////////////////////////////////////////////////////////////
         bool all_started;
      };
   };
};

#endif //Cora_LgrNet_BrokerBrowser2_h

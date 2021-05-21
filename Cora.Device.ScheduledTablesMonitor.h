/* Cora.Device.ScheduledTablesMonitor.h

   Copyright (C) 2000, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 09 August 2000
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Cora_Device_ScheduledTablesMonitor_h
#define Cora_Device_ScheduledTablesMonitor_h

#include "Cora.Device.DeviceBase.h"
#include "Csi.InstanceValidator.h"
#include "CsiEvents.h"
#include "Cora.Device.TablesEnumerator.h"
#include "Cora.Device.SettingsEnumerator.h"
#include "Cora.Broker.TableLister.h"
#include "Csi.PolySharedPtr.h"
#include "Cora.Device.DeviceSettingTypes.h"
#include "Cora.CommonSettingTypes.h"


namespace Cora
{
   namespace Device
   {
      //@group class forward declarations
      class ScheduledTablesMonitor;
      //@endgroup

      ////////////////////////////////////////////////////////////
      // class ScheduledTablesMonitorClient
      //
      // Defines the interface expected from a client to a ScheduledTablesMonitor object.
      ////////////////////////////////////////////////////////////
      class ScheduledTablesMonitorClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_started
         //
         // Called when the initial list of tables has been collected and the transaction has
         // entered a steady state
         //////////////////////////////////////////////////////////// 
         virtual void on_started(
            ScheduledTablesMonitor *monitor)
         { }

         ////////////////////////////////////////////////////////////
         // on_failure
         //
         // Called when a failure has occurred that prevents the monitor object from continuing.
         //////////////////////////////////////////////////////////// 
         enum failure_type
         {
            failure_unknown = 0,
            failure_invalid_logon = 1,
            failure_invalid_device_name = 2,
            failure_server_security = 3,
            failure_unsupported = 4,
            failure_session = 5,
         };
         virtual void on_failure(
            ScheduledTablesMonitor *monitor,
            failure_type failure) = 0;

         ////////////////////////////////////////////////////////////
         // on_table_added
         //////////////////////////////////////////////////////////// 
         virtual void on_table_added(
            ScheduledTablesMonitor *monitor,
            StrUni const &table_name,
            bool scheduled)
         { }

         ////////////////////////////////////////////////////////////
         // on_table_deleted
         //////////////////////////////////////////////////////////// 
         virtual void on_table_deleted(
            ScheduledTablesMonitor *monitor,
            StrUni const &table_name)
         { }

         ////////////////////////////////////////////////////////////
         // on_table_scheduled
         //
         // Called when a table has been enabled for scheduled collection
         //////////////////////////////////////////////////////////// 
         virtual void on_table_scheduled(
            ScheduledTablesMonitor *monitor,
            StrUni const &table_name,
            bool scheduled)
         { }
      };


      ////////////////////////////////////////////////////////////
      // class ScheduledTablesMonitor
      //
      // Defines an object that provides notifications to its client regarding when tables are
      // enabled or disabled for scheduled collection. This class will work for server versions
      // 1.1.3 and greater.
      ////////////////////////////////////////////////////////////
      class ScheduledTablesMonitor:
         public DeviceBase,
         public Cora::Device::TablesEnumeratorClient,
         public Cora::Device::SettingsEnumeratorClient,
         public Cora::Broker::TableListerClient,
         public Csi::EvReceiver
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         //////////////////////////////////////////////////////////// 
         ScheduledTablesMonitor();

         ////////////////////////////////////////////////////////////
         // destructor
         //////////////////////////////////////////////////////////// 
         virtual ~ScheduledTablesMonitor();

         ////////////////////////////////////////////////////////////
         // start
         //////////////////////////////////////////////////////////// 
         virtual void start(
            ScheduledTablesMonitorClient *client,
            router_handle &router);
         virtual void start(
            ScheduledTablesMonitorClient *client,
            ClientBase *other_component);

         ////////////////////////////////////////////////////////////
         // finish
         //////////////////////////////////////////////////////////// 
         void finish();

         ////////////////////////////////////////////////////////////
         // get_is_classic_logger
         ////////////////////////////////////////////////////////////
         bool get_is_classic_logger() const
         { return is_classic_logger; }

         ////////////////////////////////////////////////////////////
         // get_input_location_identifiers
         ////////////////////////////////////////////////////////////
         typedef Csi::PolySharedPtr<Setting, SettingInlocIds> input_location_identifiers_type;
         input_location_identifiers_type &get_input_location_identifiers()
         { return input_location_identifiers; }

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
         //@group class DeviceBase overloaded methods
         ////////////////////////////////////////////////////////////
         // on_devicebase_ready
         //////////////////////////////////////////////////////////// 
         virtual void on_devicebase_ready();

         ////////////////////////////////////////////////////////////
         // on_devicebase_failure
         //////////////////////////////////////////////////////////// 
         virtual void on_devicebase_failure(devicebase_failure_type failure);

         ////////////////////////////////////////////////////////////
         // on_devicebase_session_failure
         //////////////////////////////////////////////////////////// 
         virtual void on_devicebase_session_failure();
         //@endgroup

         //@group class SettingsEnumeratorClient overloaded methods
         ////////////////////////////////////////////////////////////
         // on_failure
         //////////////////////////////////////////////////////////// 
         virtual void on_failure(SettingsEnumerator *settings,
                                 SettingsEnumeratorClient::failure_type failure);

         ////////////////////////////////////////////////////////////
         // on_setting_changed
         //////////////////////////////////////////////////////////// 
         virtual void on_setting_changed(SettingsEnumerator *settings,
                                         Csi::SharedPtr<Setting> &setting);
         //@endgroup

         //@group class TablesEnumeratorClient overloaded methods
         ////////////////////////////////////////////////////////////
         // on_started
         //////////////////////////////////////////////////////////// 
         virtual void on_started(TablesEnumerator *tran);

         ////////////////////////////////////////////////////////////
         // on_failure
         //////////////////////////////////////////////////////////// 
         virtual void on_failure(TablesEnumerator *tran,
                                 TablesEnumeratorClient::failure_type failure);

         ////////////////////////////////////////////////////////////
         // on_table_added
         //////////////////////////////////////////////////////////// 
         virtual void on_table_added(TablesEnumerator *tran,
                                     StrUni const &table_name,
                                     bool enabled,
                                     area_names_type const &area_names);

         ////////////////////////////////////////////////////////////
         // on_table_deleted
         //
         // Called after a table deleted notification is received
         //////////////////////////////////////////////////////////// 
         virtual void on_table_deleted(TablesEnumerator *tran,
                                       StrUni const &table_name);

         ////////////////////////////////////////////////////////////
         // on_table_enabled
         //
         // Called after a table has been enabled (or disabled) for scheduled collection
         //////////////////////////////////////////////////////////// 
         virtual void on_table_enabled(TablesEnumerator *tran,
                                       StrUni const &table_name,
                                       bool enabled);
         //@endgroup

         //@group class TableListerClient overloaded methods
         ////////////////////////////////////////////////////////////
         // on_started
         //////////////////////////////////////////////////////////// 
         virtual void on_started(Broker::TableLister *lister);

         ////////////////////////////////////////////////////////////
         // on_failure
         //////////////////////////////////////////////////////////// 
         virtual void on_failure(Broker::TableLister *lister,
                                 Broker::TableListerClient::failure_type failure);

         ////////////////////////////////////////////////////////////
         // on_table_added
         //////////////////////////////////////////////////////////// 
         virtual void on_table_added(Broker::TableLister *lister,
                                     StrUni const &table_name);

         ////////////////////////////////////////////////////////////
         // on_table_deleted
         //////////////////////////////////////////////////////////// 
         virtual void on_table_deleted(Broker::TableLister *lister,
                                       StrUni const &table_name);
         //@endgroup

         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

      private:
         ////////////////////////////////////////////////////////////
         // client
         //////////////////////////////////////////////////////////// 
         typedef ScheduledTablesMonitorClient client_type;
         client_type *client;

         ////////////////////////////////////////////////////////////
         // state
         //////////////////////////////////////////////////////////// 
         enum state_type
         {
            state_standby,
            state_before_active,
            state_active,
         } state;

         ////////////////////////////////////////////////////////////
         // device_tables_lister
         //
         // Used to enumerate device defined tabels. This object is used when the server version is
         // 1.2.1 or greater. It is otherwise inactive.
         //////////////////////////////////////////////////////////// 
         Csi::SharedPtr<TablesEnumerator> device_tables_lister;

         ////////////////////////////////////////////////////////////
         // broker_tables_lister
         //
         // Used to enumerate tables defined by the active data broker. This is only used when the
         // server version is 1.1.3.
         //////////////////////////////////////////////////////////// 
         Csi::SharedPtr<Cora::Broker::TableLister> broker_tables_lister;

         ////////////////////////////////////////////////////////////
         // settings
         //
         // This is used to monitor the the tablesToExclude setting when the server interface
         // version is 1.1.3
         //////////////////////////////////////////////////////////// 
         Csi::SharedPtr<SettingsEnumerator> settings;

         ////////////////////////////////////////////////////////////
         // set_tables_to_exclude
         //////////////////////////////////////////////////////////// 
         Csi::PolySharedPtr<Setting, SettingNameSet> set_tables_to_exclude;

         ////////////////////////////////////////////////////////////
         // tables
         //
         // The set of tables associated with the device. The associated value indicates whether the
         // table is enabled for scheduled collection
         //////////////////////////////////////////////////////////// 
         typedef std::map<StrUni, bool> tables_type;
         tables_type tables;

         ////////////////////////////////////////////////////////////
         // is_classic_logger
         ////////////////////////////////////////////////////////////
         bool is_classic_logger;

         ////////////////////////////////////////////////////////////
         // set_input_location_identifiers
         //
         // This will store off the input location labels if applicable
         // for classic loggers.  This component doesn't need this
         // but since it is already reading settings, it is exposing them.
         ////////////////////////////////////////////////////////////
         input_location_identifiers_type input_location_identifiers;

         ////////////////////////////////////////////////////////////
         // send_temporaries
         ////////////////////////////////////////////////////////////
         bool send_temporaries;
      };
   };
};

#endif

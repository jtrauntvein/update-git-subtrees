/* Cora.Device.DataMonitor.h

   Copyright (C) 2004, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 03 November 2004
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Cora_Device_DataMonitor_h
#define Cora_Device_DataMonitor_h

#include "Cora.Device.DeviceBase.h"
#include "Cora.Broker.Record.h"
#include "Csi.Events.h"
#include <list>


namespace Cora
{
   namespace Device
   {
      //@group class forward declarations
      class DataMonitor;
      //@endgroup


      ////////////////////////////////////////////////////////////
      // class DataMonitorClient
      ////////////////////////////////////////////////////////////
      class DataMonitorClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_started
         //
         // Called when the advise transaction has been started successfully.  The record object
         // will not hold data but will allow the application to access the meta-data for the
         // record. 
         ////////////////////////////////////////////////////////////
         typedef Csi::SharedPtr<Cora::Broker::Record> record_handle;
         virtual void on_started(
            DataMonitor *monitor,
            record_handle &record)
         { }

         ////////////////////////////////////////////////////////////
         // on_failure
         ////////////////////////////////////////////////////////////
         enum failure_type
         {
            failure_unknown = 0,
            failure_connection_failed = 1,
            failure_invalid_logon = 2,
            failure_invalid_station_name = 3,
            failure_invalid_table_name = 4,
            failure_server_security = 5,
            failure_table_deleted = 6,
            failure_unsupported = 7,
            failure_station_shut_down = 8
         };
         virtual void on_failure(
            DataMonitor *monitor,
            failure_type failure) = 0;

         ////////////////////////////////////////////////////////////
         // on_records
         //
         // Called when records have been received for this monitor. The
         // records will be in the provided list.  These records will be placed
         // back in the component's recycling queue after this notification has
         // been processed.  If the application needs the records to last
         // beyond that, the record objects should be cloned. 
         ////////////////////////////////////////////////////////////
         typedef std::list<record_handle> records_type;
         virtual void on_records(
            DataMonitor *monitor,
            records_type const &records)
         { }
      };


      ////////////////////////////////////////////////////////////
      // class DataMonitor
      ////////////////////////////////////////////////////////////
      class DataMonitor:
         public DeviceBase,
         public Csi::EventReceiver
      {
      private:
         //@group properties
         ////////////////////////////////////////////////////////////
         // table_name
         //
         // Specifies the name of the table that should be monitored
         ////////////////////////////////////////////////////////////
         StrUni table_name;

         ////////////////////////////////////////////////////////////
         // value_factory
         //
         // Specifies the object that will create values for the record.  If
         // not specified by the application, this will default to an object of
         // class Cora::Broker::ValueFactory.
         ////////////////////////////////////////////////////////////
         Csi::SharedPtr<Cora::Broker::ValueFactory> value_factory;
         //@endgroup

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
         // client
         ////////////////////////////////////////////////////////////
         DataMonitorClient *client;

         ////////////////////////////////////////////////////////////
         // monitor_tran
         ////////////////////////////////////////////////////////////
         uint4 monitor_tran;

         ////////////////////////////////////////////////////////////
         // recycled_records
         ////////////////////////////////////////////////////////////
         typedef Csi::SharedPtr<Cora::Broker::Record> record_handle;
         typedef std::list<record_handle> recycled_records_type;
         recycled_records_type recycled_records;

         ////////////////////////////////////////////////////////////
         // description
         ////////////////////////////////////////////////////////////
         Cora::Broker::Record::desc_handle description;

      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         DataMonitor();

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~DataMonitor();

         //@group properties access
         ////////////////////////////////////////////////////////////
         // set_table_name
         ////////////////////////////////////////////////////////////
         void set_table_name(StrUni const &table_name_)
         {
            if(state == state_standby)
               table_name = table_name_;
         }

         ////////////////////////////////////////////////////////////
         // get_table_name
         ////////////////////////////////////////////////////////////
         StrUni const &get_table_name() const
         { return table_name; }

         ////////////////////////////////////////////////////////////
         // set_value_factory
         //////////////////////////////////////////////////////////// 
         void set_value_factory(Csi::SharedPtr<Cora::Broker::ValueFactory> &factory)
         {
            if(state == state_standby && factory != 0)
               value_factory = factory;
         }

         ////////////////////////////////////////////////////////////
         // get_value_factory
         ////////////////////////////////////////////////////////////
         Csi::SharedPtr<Cora::Broker::ValueFactory> get_value_factory() const
         { return value_factory; }
         //@endgroup

         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         typedef DataMonitorClient client_type;
         void start(
            client_type *client_,
            router_handle &router);
         void start(
            client_type *client_,
            ClientBase *other_component);

         ////////////////////////////////////////////////////////////
         // finish
         ////////////////////////////////////////////////////////////
         virtual void finish();

         ////////////////////////////////////////////////////////////
         // add_recycled_records
         ////////////////////////////////////////////////////////////
         void add_recycled_records(recycled_records_type &source);

      protected:
         //@group methods overloaded from class BrokerBase
         ////////////////////////////////////////////////////////////
         // onNetMessage
         ////////////////////////////////////////////////////////////
         virtual void onNetMessage(
            Csi::Messaging::Router *rtr,
            Csi::Messaging::Message *msg);

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

         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

      private:
         ////////////////////////////////////////////////////////////
         // on_start_ack
         ////////////////////////////////////////////////////////////
         void on_start_ack(Csi::Messaging::Message *message);

         ////////////////////////////////////////////////////////////
         // on_notification
         ////////////////////////////////////////////////////////////
         void on_notification(Csi::Messaging::Message *message);

         ////////////////////////////////////////////////////////////
         // on_stopped_notification
         ////////////////////////////////////////////////////////////
         void on_stopped_notification(Csi::Messaging::Message *message);
      };
   };
};


#endif

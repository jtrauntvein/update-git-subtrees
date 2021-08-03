/* Cora.Device.LoggerQueryEx.h

   Copyright (C) 2019, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Monday 16 September 2019
   Last Change: Tuesday 24 September 2019
   Last Commit: $Date: 2019-09-24 15:24:59 -0600 (Tue, 24 Sep 2019) $
   Last Changed by: $Author: jon $

*/

#ifndef Cora_Device_LoggerQueryEx_h
#define Cora_Device_LoggerQueryEx_h
#include "Cora.Device.DeviceBase.h"
#include "Cora.Broker.Toa5Options.h"
#include "Cora.Broker.CustomCsvOptions.h"
#include "Cora.Broker.XmlOptions.h"
#include "Cora.Broker.Tob1Options.h"
#include "Cora.Broker.NohOptions.h"
#include "Csi.Events.h"
#include <deque>


namespace Cora
{
   namespace Device
   {
      class LoggerQueryEx;


      namespace LoggerQueryExHelpers
      {
         struct record_type
         {
            uint4 record_no;
            Csi::LgrDate timestamp;
            StrBin record;
         };
      };

      
      /**
       * Defines the interface that must be implemented by an application object that uses the
       * LoggerQueryEx component type.
       */
      class LoggerQueryExClient: public Csi::InstanceValidator
      {
      public:
         /**
          * Called by the component when it has received notification that the transaction has
          * started.
          *
          * @param sender Specifies the component that called this method.
          *
          * @param outcome Specifies the initial outcome of the transaction.
          *
          * @param formatted_header Specifies the header for the file.
          *
          * @param formatted_footer Specifies the footer for the file.
          */
         enum start_outcome_type
         {
            start_outcome_success = 1,
            start_outcome_failure_logon = 2,
            start_outcome_failure_invalid_device_name = 3,
            start_outcome_failure_server_security_blocked = 4,
            start_outcome_failure_session = 5,
            start_outcome_failure_invalid_table_name = 6,
            start_outcome_failure_comms_disabled = 7,
            start_outcome_failure_unsupported_query_mode = 8,
            start_outcome_failure_unsupported_file_format = 9,
            start_outcome_failure_unknown = 10,
            start_outcome_failure_unsupported = 11
         };
         virtual void on_started(
            LoggerQueryEx *sender,
            start_outcome_type outcome,
            StrBin const &formatted_header,
            StrBin const &formatted_footer) = 0;

         /**
          * Called by the component when records have been received.
          *
          * @param sender Specifies the component that called this method.
          *
          * @param records Specifies the collection of record objects.
          */
         typedef LoggerQueryExHelpers::record_type record_type;
         typedef Csi::SharedPtr<record_type> record_handle;
         typedef std::deque<record_handle> records_type;
         virtual void on_records(LoggerQueryEx *sender, records_type const &records) = 0;

         /**
          * Called by the component when the stopped notification has been received.
          *
          * @param sender Specifies the component that called this method.
          *
          * @param outcome Specifies the reported outcome
          */
         enum stopped_outcome_type
         {
            stopped_outcome_success = 1,
            stopped_outcome_success_client_request = 2,
            stopped_outcome_failure_table_defs = 3,
            stopped_outcome_failure_comms = 4,
            stopped_outcome_failure_comms_disabled = 5,
            stopped_outcome_failure_blocked_by_logger = 6,
            stopped_outcome_failure_session = 7,
            stopped_outcome_failure_ack_timed_out = 8,
            stopped_outcome_failure_unknown = 9
         };
         virtual void on_stopped(LoggerQueryEx *sender, stopped_outcome_type outcome) = 0;
      };


      /**
       * Defines a component that can be used to drive the LoggerNet Device Logger Query EX
       * transaction.  This component works similarly to the LoggerQuery or LoggerQueryFile
       * transactions but differs from them in that the server directly formats the records and
       * sends them to the client rather than relying on the client to use the broker formatted
       * data advise transaction.
       *
       * In order to use this transaction, the application must provide an object that inherits from
       * class LoggerQueryExClient.  It should then create an instance of this class, set its
       * properties including device name, table name, collect mode, and output format and then call
       * one of the two versions of start().  When the component has been notified that the server
       * has started the transaction or failed to start the transaction, the client's on_start()
       * method will be called.  Thereafter, the component will call the client's on_records()
       * method when formatted data has been received from the server.  Finally, the client's
       * on_stopped() method will be called when the transaction has either successfully concluded
       * or been stopped by an error.
       */
      class LoggerQueryEx: public DeviceBase, public Csi::EventReceiver
      {
      private:
         /**
          * Specifies the client object.
          */
         LoggerQueryExClient *client;

         /**
          * Specifies the state of this transaction.
          */
         enum state_type
         {
            state_standby,
            state_connecting,
            state_starting,
            state_started
         } state;

         /**
          * Specifies the records that have been received and are waiting to be processed.
          */
         typedef LoggerQueryExClient::records_type records_type;
         typedef LoggerQueryExClient::record_handle record_handle;
         typedef LoggerQueryExClient::record_type record_type;
         records_type records;

         /**
          * Specifies record objects that have already been processed and that can be reused.
          */
         records_type records_cache;

         /**
          * Specifies the table name.
          */
         StrUni table_name;

         /**
          * Specifies the query mode.
          */
      public:
         enum query_mode_type
         {
            query_date_range = 1,
            query_most_recent = 2,
            query_record_range = 3,
            query_since_last_poll = 4,
            query_all_data = 5,
            query_record_to_newest = 6,
            query_backfill = 7
         };
      private:
         query_mode_type query_mode;

         /**
          * Specifies the bounds of the date range.
          */
         Csi::LgrDate begin_date, end_date;

         /**
          * Specifies the maximum number of records to poll.
          */
         uint4 max_records;

         /**
          * Specifies the range of record numbers.
          */
         uint4 begin_record_no, end_record_no;

         /**
          * Specifies the backfill interval in milliseconds.
          */
         int8 backfill_interval;

         /**
          * Specifies the record format.
          */
      public:
         enum record_format_type
         {
            record_format_toaci1 = 1,
            record_format_toa5 = 2,
            record_format_tob1 = 3,
            record_format_custom_csv = 4,
            record_format_csixml = 5,
            record_format_csijson = 6,
            record_format_csv = 7
         };
      private:
         record_format_type record_format;

         /**
          * Specifies the record format options.
          */
         uint4 record_format_options;

         /**
          * Specifies the reported station name.
          */
         StrUni reported_station_name;

         /**
          * Specifies the formatted header.
          */
         StrBin formatted_header;

         /**
          * Specifies the formatted footer.
          */
         StrBin formatted_footer;

         /**
          * Specifies the transaction number.
          */
         uint4 tran_no;

      public:
         /**
          * Constructor
          */
         LoggerQueryEx():
            state(state_standby),
            client(0),
            query_mode(query_all_data),
            max_records(1),
            begin_record_no(0),
            end_record_no(0xffffffff),
            backfill_interval(Csi::LgrDate::msecPerDay),
            record_format(record_format_toa5),
            record_format_options(
               CollectArea::table_file_include_record_no |
               CollectArea::table_file_include_time),
            tran_no(0)
         { }

         /**
          * Destructor
          */
         virtual ~LoggerQueryEx()
         { finish(); }

         /**
          * @return Returns the selected table name.
          */
         StrUni const &get_table_name() const
         { return table_name; }

         /**
          * @param value Specifies the table name.
          */
         void set_table_name(StrUni const &value)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            table_name = value;
         }

         /**
          * @return Returns the selected query mode.
          */
         query_mode_type get_query_mode() const
         { return query_mode; }

         /**
          * Sets the query mode to query by date range.
          *
          * @param start Specifies the start of the date range.
          *
          * @param end Specifies the end of the date range.
          */
         void set_query_date_range(Csi::LgrDate const &start, Csi::LgrDate const &end)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            if(start > end)
               throw std::invalid_argument("invalid date range");
            begin_date = start;
            end_date = end;
            query_mode = query_date_range;
         }

         /**
          * @return Returns the begin date.
          */
         Csi::LgrDate const &get_begin_date() const
         { return begin_date; }

         /**
          * @return Returns the end date.
          */
         Csi::LgrDate const &get_end_date() const
         { return end_date; }

         /**
          * Sets the query up to query by record nunber range.
          *
          * @param begin_record Specifies the start record number.
          *
          * @para, end_record Specifies the end record number.
          */
         void set_query_by_record(uint4 begin_record, uint4 end_record)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            if(end_record <= begin_record)
               throw std::invalid_argument("invalid record number range");
            begin_record_no = begin_record;
            end_record_no = end_record;
            query_mode = query_record_range;
         }

         /**
          * @return Returns the begin record number.
          */
         uint4 get_begin_record_no() const
         { return begin_record_no; }

         /**
          * @return Returns the end record number.
          */
         uint4 get_end_record_no() const
         { return end_record_no; }
            
         /**
          * Sets the query mode to get most recent.
          *
          * @param count Specifies the maximum number of records to get.
          */
         void set_query_most_recent(uint4 count)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            if(count == 0)
               throw std::invalid_argument("invalid max records");
            query_mode = query_most_recent;
            max_records = count;
         }

         /**
          * @return Returns the maximum number of records to retrieve.
          */
         uint4 get_max_records() const
         { return max_records; }

         /**
          * Sets tge query mode to query a;ll records stored since the last poll.
          */
         void set_query_all_since_last()
         {
            if(state != state_standby)
               throw exc_invalid_state();
            query_mode = query_since_last_poll;
         }

         /**
          * Sets the query mode to collect all records.
          */
         void set_query_all()
         {
            if(state != state_standby)
               throw exc_invalid_state();
            query_mode = query_all_data;
         }

         /**
          * Sets the query to start at a given record number.
          *
          * @param start Specifies the starting record number.
          */
         void set_query_record_to_newest(uint4 start)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            begin_record_no = start;
            query_mode = query_record_to_newest;
         }
         
         /**
          * Sets the query mode to backfill.
          *
          * @param interval Specifies the backfill interval in milliseconds.
          */
         void set_query_backfill(int8 interval)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            if(interval <= 0)
               throw std::invalid_argument("invalid backfill interval");
            backfill_interval = interval;
            query_mode = query_backfill;
         }

         /**
          * @return Returns the backfill interval.
          */
         int8 get_backfill_interval() const
         { return backfill_interval; }

         /**
          * Sets the data format to TOACI1.
          */
         void set_record_format_toaci1()
         {
            if(state != state_standby)
               throw exc_invalid_state();
            record_format = record_format_toaci1;
         }

         /**
          * Sets the record format to TOA5.
          *
          * @param options Specifies the TOA5 options.
          */
         void set_record_format_toa5(Broker::Toa5Options const &options = Broker::Toa5Options())
         {
            if(state != state_standby)
               throw exc_invalid_state();
            record_format = record_format_toa5;
            record_format_options = options.get_options();
         }

         /**
          * Sets the record format to TOB1.
          *
          * @param options Specifies the TOB1 options.
          */
         void set_record_format_tob1(Broker::Tob1Options const &options = Broker::Tob1Options())
         {
            if(state != state_standby)
               throw exc_invalid_state();
            record_format = record_format_tob1;
            record_format_options = options.get_options();
         }

         /**
          * Sets the record format to custom CSV.
          *
          * @param options Specifies the options for custom CSV.
          */
         void set_record_format_custom_csv(Broker::CustomCsvOptions const &options = Broker::CustomCsvOptions())
         {
            if(state != state_standby)
               throw exc_invalid_state();
            record_format = record_format_custom_csv;
            record_format_options = options.get_options();
         }

         /**
          * Sets the record format to CSIXML
          *
          * @param options Specifies the options for XML.
          */
         void set_record_format_csixml(Broker::XmlOptions options = Broker::XmlOptions())
         {
            if(state != state_standby)
               throw exc_invalid_state();
            record_format = record_format_csixml;
            record_format_options = options.get_options();
         }
         
         /**
          * Sets the record format to CSIJson.
          */
         void set_record_format_json()
         {
            if(state != state_standby)
               throw exc_invalid_state();
            record_format = record_format_csijson;
         }

         /**
          * Sets the record format CSV with no header.
          *
          * @param options Specifies the options.
          */
         void set_record_format_csv(Broker::NohOptions const &options = Broker::NohOptions())
         {
            if(state != state_standby)
               throw exc_invalid_state();
            record_format = record_format_csv;
            record_format_options = options.get_options();
         }

         /**
          * Sets the reported station name.
          *
          * @param value Specifies the name.
          */
         void set_reported_station_name(StrUni const &value)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            reported_station_name = value;
         }

         /**
          * @return Returns the reported station name.
          */
         StrUni const get_reported_station_name() const
         { return reported_station_name; }

         /**
          * Starts the transaction with the server.
          *
          * @param router Specifies a newly created router that is not yet connected.
          *
          * @param other_client Specifies a component that is already connected that can share it
          * connection with this component.
          *
          * @param client_ Specifies the application object that will receive notifications.
          */
         typedef LoggerQueryExClient client_type;
         void start(client_type *client_, router_handle &router)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("invalid client pointer");
            client = client_;
            state = state_connecting;
            DeviceBase::start(router);
         }
         void start(client_type *client_, ClientBase *other_client)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("invalid client pointer");
            client = client_;
            state = state_connecting;
            DeviceBase::start(other_client);
         }

         /**
          * Overloads the base class version to release the server transaction.
          */
         virtual void finish()
         {
            client = 0;
            state = state_standby;
            records.clear();
            records_cache.clear();
            DeviceBase::finish();
         }

         /**
          * Formats the specified start outcome code to the specified stream.
          *
          * @param out Specifies the output stream.
          *
          * @param outcome Specifies the outcome code to format.
          */
         static void format_start_outcome(std::ostream &out, client_type::start_outcome_type outcome);

         /**
          * Formats the specified stopped outcome code to the specified stream.
          *
          * @param out Specifies the output stream.
          *
          * @param outcome Specifies the outcome code to format.
          */
         static void format_stopped_outcome(std::ostream &out, client_type::stopped_outcome_type outcome);
         
         /**
          * Overloads the asynchronous event handler.
          */
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);
         
         /**
          * Overloads the base class version to start the server transaction.
          */
         virtual void on_devicebase_ready();

         /**
          * Overloads the base class version to handle a failure.
          */
         virtual void on_devicebase_failure(devicebase_failure_type failure);

         /**
          * Overloads the handler for connection loss.
          */
         virtual void on_devicebase_session_failure()
         { on_devicebase_failure(devicebase_failure_session); }

         /**
          * Overloads the base class message handler.
          */
         virtual void onNetMessage(Csi::Messaging::Router *router, Csi::Messaging::Message *message);

      private:
         /**
          * Handles the start acknowledgement message.
          */
         void on_start_ack(Csi::Messaging::Message *message);

         /**
          * Handles the records notification.
          */
         void on_records_not(Csi::Messaging::Message *message);

         /**
          * Handles the stopped notification.
          */
         void on_stopped_not(Csi::Messaging::Message *message);
      };
   };
};


#endif


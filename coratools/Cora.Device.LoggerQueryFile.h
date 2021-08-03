/* Cora.Device.LoggerQueryFile.h

   Copyright (C) 2008, 2017 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 18 December 2008
   Last Change: Thursday 11 May 2017
   Last Commit: $Date: 2017-05-11 10:32:26 -0600 (Thu, 11 May 2017) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Cora_Device_LoggerQueryFile_h
#define Cora_Device_LoggerQueryFile_h

#include "Cora.Device.LoggerQuery.h"
#include "Cora.Broker.FormattedDataAdvisor.h"


namespace Cora
{
   namespace Device
   {
      // @group: class forward declarations
      class LoggerQueryFile;
      // @endgroup

      ////////////////////////////////////////////////////////////
      // class LoggerQueryFileClient
      ////////////////////////////////////////////////////////////
      class LoggerQueryFileClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_query_status
         //
         // Called when progress has been made on the query.
         ////////////////////////////////////////////////////////////
         enum status_code_type
         {
            status_temporary_table_created = 1,
            status_some_data_collected = 2,
            status_all_data_collected = 3,
            status_file_mark_created = 5
         };
         virtual void on_query_status(
            LoggerQueryFile *query,
            status_code_type status_code,
            uint4 record_count,
            uint4 begin_record_no,
            uint4 end_record_no)
         { }

         ////////////////////////////////////////////////////////////
         // on_append_warnings
         //
         // Called synchronously when by the query when it is attempting to
         // append to an existing data file and there are warnings.  The query
         // will be aborted if this method returns a value of false. 
         ////////////////////////////////////////////////////////////
         virtual bool on_append_warnings(
            LoggerQueryFile *query,
            int error_code,
            StrAsc const &warnings)
         { return false; }

         ////////////////////////////////////////////////////////////
         // on_complete
         //
         // Called when the query is complete and all resulting data has been
         // written to the file.
         ////////////////////////////////////////////////////////////
         enum outcome_type
         {
            outcome_unknown = 0,
            outcome_success = 1,
            outcome_server_session_failed = 2,
            outcome_invalid_device_name = 3,
            outcome_unsupported = 4,
            outcome_server_security_blocked = 5,
            outcome_logger_security_blocked = 6,
            outcome_communication_failure = 7,
            outcome_communication_disabled = 8,
            outcome_invalid_table_name = 9,
            outcome_invalid_table_definitions = 10,
            outcome_insufficient_resources = 11,
            outcome_file_open_failed = 12,
            outcome_query_interrupted = 13,
            outcome_invalid_logon = 14,
            outcome_invalid_format_option = 15,
            outcome_cannot_append = 16,
            outcome_file_io_failed = 17
         };
         virtual void on_complete(
            LoggerQueryFile *query, outcome_type outcome) = 0;
      };

      
      ////////////////////////////////////////////////////////////
      // class LoggerQueryFile
      //
      // Defines a component that encapsulates the device Logger Query
      // transaction and writes the resulting data, if any, to a file.  It
      // provides properties that allow the application to specify the query
      // properties as well as the name and format of the data file.
      //
      // In order to use this component, an application must provide an object
      // that is derived from class LoggerQueryFileClient (also typedefed as
      // "client_type" within the class.  The application should then
      // instantiate an object of this type and set properties for the
      // component including device name, table name, output file name, &
      // etc. and then invoke one of the two start() methods.  As the
      // transaction progresses notifications will be sent to the client
      // regarding that progress.  If the data file is to be appended but there
      // are warnings in doing so, the client will also be notified via a
      // special method and given the option of abandoning the query.  When the
      // transaction is complete, the client's on_complete() method will be
      // invoked. 
      ////////////////////////////////////////////////////////////
      class LoggerQueryFile:
         public LoggerQueryClient,
         public Broker::FormattedDataAdvisorClient,
         public Csi::EventReceiver
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         LoggerQueryFile();

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~LoggerQueryFile();

         ////////////////////////////////////////////////////////////
         // get_device_name
         ////////////////////////////////////////////////////////////
         typedef LoggerQuery query_type;
         StrUni const get_device_name() const
         { return query->get_device_name(); }

         ////////////////////////////////////////////////////////////
         // set_device_name
         ////////////////////////////////////////////////////////////
         void set_device_name(StrUni const &device_name)
         { query->set_device_name(device_name); }

         ////////////////////////////////////////////////////////////
         // get_table_name
         ////////////////////////////////////////////////////////////
         StrUni const &get_table_name() const
         { return  query->get_table_name(); }

         ////////////////////////////////////////////////////////////
         // set_table_name
         ////////////////////////////////////////////////////////////
         void set_table_name(StrUni const &table_name)
         { query->set_table_name(table_name); }

         ////////////////////////////////////////////////////////////
         // set_query_dates
         ////////////////////////////////////////////////////////////
         void set_query_date_range(Csi::LgrDate const &begin_date, Csi::LgrDate const &end_date)
         {
            query->set_begin_date(begin_date);
            query->set_end_date(end_date);
            query->set_query_mode(query_type::query_date_range);
         }

         ////////////////////////////////////////////////////////////
         // set_query_recent_records
         ////////////////////////////////////////////////////////////
         void set_query_recent_records(uint4 number_of_records)
         {
            query->set_number_of_records(number_of_records);
            query->set_query_mode(query_type::query_recent_records);
         }

         ////////////////////////////////////////////////////////////
         // set_query_record_number_range
         ////////////////////////////////////////////////////////////
         void set_query_record_number_range(uint4 begin_record_no, uint4 end_record_no)
         {
            query->set_begin_record_no(begin_record_no);
            query->set_end_record_no(end_record_no);
            query->set_query_mode(query_type::query_record_number_range);
         }

         ////////////////////////////////////////////////////////////
         // set_all_since_last_poll
         ////////////////////////////////////////////////////////////
         void set_query_all_since_last_poll()
         { query->set_query_mode(query_type::query_all_since_last_poll); }

         ////////////////////////////////////////////////////////////
         // set_query_all
         ////////////////////////////////////////////////////////////
         void set_query_all()
         { query->set_query_mode(query_type::query_all); }

         ////////////////////////////////////////////////////////////
         // set_query_start_at_record
         ////////////////////////////////////////////////////////////
         void set_query_start_at_record(uint4 begin_record_no)
         {
            query->set_begin_record_no(begin_record_no);
            query->set_query_mode(query_type::query_start_at_record);
         }

         /**
          * Called to set up the query to perform a backfill.
          */
         void set_query_backfill(int8 backfill_interval)
         {
            query->set_backfill_interval(backfill_interval);
         }

         ////////////////////////////////////////////////////////////
         // get_query
         ////////////////////////////////////////////////////////////
         Csi::SharedPtr<query_type> &get_query()
         { return query; }

         ////////////////////////////////////////////////////////////
         // set_output_file_name
         ////////////////////////////////////////////////////////////
         void set_output_file_name(StrAsc const &output_file_name_);

         ////////////////////////////////////////////////////////////
         // get_output_file_name
         ////////////////////////////////////////////////////////////
         StrAsc const &get_output_file_name() const
         { return output_file_name; }

         ////////////////////////////////////////////////////////////
         // set_output_file_format
         ////////////////////////////////////////////////////////////
         typedef Broker::FormattedDataAdvisor advisor_type;
         void set_output_file_format(advisor_type::format_option_type format)
         { advisor->set_format_option(format); }

         ////////////////////////////////////////////////////////////
         // set_format_option_flags
         ////////////////////////////////////////////////////////////
         void set_format_option_flags(uint4 flags)
         { advisor->set_format_option_flags(flags); }

         ////////////////////////////////////////////////////////////
         // get_append_data
         ////////////////////////////////////////////////////////////
         bool get_append_data() const
         { return append_data; }

         ////////////////////////////////////////////////////////////
         // set_append_data
         ////////////////////////////////////////////////////////////
         void set_append_data(bool append_data);

         ////////////////////////////////////////////////////////////
         // get_reported_station_name
         ////////////////////////////////////////////////////////////
         StrUni const &get_reported_station_name() const
         { return advisor->get_reported_station_name(); }

         ////////////////////////////////////////////////////////////
         // set_reported_station_name
         ////////////////////////////////////////////////////////////
         void set_reported_station_name(StrUni const &name)
         { advisor->set_reported_station_name(name); }

         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         typedef LoggerQueryFileClient client_type;
         typedef ClientBase::router_handle router_handle;
         void start(client_type *client_, router_handle router);
         void start(client_type *client_, ClientBase *other_component);

         ////////////////////////////////////////////////////////////
         // finish
         ////////////////////////////////////////////////////////////
         virtual void finish();

         ////////////////////////////////////////////////////////////
         // format_outcome
         ////////////////////////////////////////////////////////////
         static void format_outcome(std::ostream &out, client_type::outcome_type outcome);

         ////////////////////////////////////////////////////////////
         // format_status
         ////////////////////////////////////////////////////////////
         static void format_status(std::ostream &out, client_type::status_code_type);

         ////////////////////////////////////////////////////////////
         // on_query_status
         ////////////////////////////////////////////////////////////
         virtual void on_query_status(
            query_type *query,
            query_type::client_type::status_code_type status_code,
            StrUni const &temp_table_name,
            uint4 record_count,
            uint4 begin_record_no,
            uint4 end_record_no);

         ////////////////////////////////////////////////////////////
         // on_failure
         ////////////////////////////////////////////////////////////
         virtual void on_failure(
            query_type *query, query_type::client_type::failure_type failure);

         ////////////////////////////////////////////////////////////
         // on_started
         ////////////////////////////////////////////////////////////
         virtual void on_started(
            advisor_type *advisor,
            StrAsc const &data_header,
            StrAsc const &data_footer);

         ////////////////////////////////////////////////////////////
         // on_failure
         ////////////////////////////////////////////////////////////
         virtual void on_failure(
            advisor_type *advisor,
            advisor_type::client_type::failure_type failure);

         ////////////////////////////////////////////////////////////
         // on_records
         ////////////////////////////////////////////////////////////
         virtual void on_records(
            advisor_type *advisor);

         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

         ////////////////////////////////////////////////////////////
         // get_records_expected
         ////////////////////////////////////////////////////////////
         uint4 get_records_expected() const
         { return records_expected; }

         ////////////////////////////////////////////////////////////
         // get_records_collected
         ////////////////////////////////////////////////////////////
         uint4 get_records_collected() const
         { return records_collected; }

      private:
         ////////////////////////////////////////////////////////////
         // post_complete
         ////////////////////////////////////////////////////////////
         void post_complete(client_type::outcome_type outcome);
         
      private:
         ////////////////////////////////////////////////////////////
         // query
         ////////////////////////////////////////////////////////////
         Csi::SharedPtr<query_type> query;

         ////////////////////////////////////////////////////////////
         // advisor
         ////////////////////////////////////////////////////////////
         Csi::SharedPtr<advisor_type> advisor;

         ////////////////////////////////////////////////////////////
         // header
         ////////////////////////////////////////////////////////////
         StrAsc header;

         ////////////////////////////////////////////////////////////
         // footer
         ////////////////////////////////////////////////////////////
         StrAsc footer;

         ////////////////////////////////////////////////////////////
         // output
         ////////////////////////////////////////////////////////////
         FILE *output;

         ////////////////////////////////////////////////////////////
         // state
         ////////////////////////////////////////////////////////////
         enum state_type
         {
            state_standby,
            state_query_started,
            state_query_complete
         } state;

         ////////////////////////////////////////////////////////////
         // output_file_name
         ////////////////////////////////////////////////////////////
         StrAsc output_file_name;

         ////////////////////////////////////////////////////////////
         // append_data
         ////////////////////////////////////////////////////////////
         bool append_data;

         ////////////////////////////////////////////////////////////
         // client
         ////////////////////////////////////////////////////////////
         client_type *client;

         ////////////////////////////////////////////////////////////
         // records_expected
         ////////////////////////////////////////////////////////////
         uint4 records_expected;

         ////////////////////////////////////////////////////////////
         // records_collected
         ////////////////////////////////////////////////////////////
         uint4 records_collected;

         ////////////////////////////////////////////////////////////
         // advisor_started
         ////////////////////////////////////////////////////////////
         bool advisor_started;
      };
   };
};


#endif

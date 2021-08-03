/* Cora.DataSources.SinkBase.h

   Copyright (C) 2008, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 04 August 2008
   Last Change: Wednesday 13 May 2020
   Last Commit: $Date: 2020-05-13 13:50:49 -0600 (Wed, 13 May 2020) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Cora_DataSources_SinkBase_h
#define Cora_DataSources_SinkBase_h

#include "Cora.DataSources.Request.h"
#include "Csi.InstanceValidator.h"
#include "Cora.Broker.Record.h"
#include "Cora.Device.FileLister.h"
#include "Csi.Passwd.h"
#include <list>
#include <deque>


namespace Cora
{
   namespace DataSources
   {
      using Cora::Broker::Record;
      using Cora::Broker::RecordDesc;

      class Manager;
      class Request;

      
      /**
       * Defines the application's interface to the data sources.  Classes that start requests will
       * need to override that virtual methods in this interface to handle data and other source
       * related events.
       */
      class SinkBase: public Csi::InstanceValidator
      {
      public:
         /**
          * Called when the source is ready to read data for the request.
          *
          * @param manager Specifies the data source manager.
          *
          * @param request Specifies the data source request.
          *
          * @param record Specifies a template record that was created using the request record
          * description.  All record reported in on_sink_records() will be created using this same
          * description.
          */
         typedef Csi::SharedPtr<Request> request_handle;
         typedef Csi::SharedPtr<Record> record_handle;
         virtual void on_sink_ready(
            Manager *manager,
            request_handle &request,
            record_handle &record)
         { }

         /**
          * @return Returns the source associated with the specified name.
          *
          * @param name Specifies the name of the source.
          */
         virtual SourceBase *get_source(StrUni const &name)
         {
            return nullptr;
         }

         /**
          * Reports that a failure has occurred for the specified request.
          *
          * @param manager Specifies the data source manager.
          *
          * @param request Specifies the request that has failed.
          *
          * @param failure Specifies a code that describes the failure.
          */
         enum sink_failure_type
         {
            sink_failure_unknown = 0,
            sink_failure_invalid_source = 1,
            sink_failure_connection_failed = 2,
            sink_failure_invalid_logon = 3,
            sink_failure_invalid_station_name = 4,
            sink_failure_invalid_table_name = 5,
            sink_failure_server_security = 6,
            sink_failure_invalid_start_option = 7,
            sink_failure_invalid_order_option = 8,
            sink_failure_table_deleted = 9,
            sink_failure_station_shut_down = 10,
            sink_failure_unsupported = 11,
            sink_failure_invalid_column_name = 12,
            sink_failure_invalid_array_address = 13
         };
         virtual void on_sink_failure(
            Manager *manager,
            request_handle &request,
            sink_failure_type failure) = 0;

         /**
          * Called to report that a collection of records have been received from the source.
          *
          * @param manager Specifies the data source manager.
          *
          * @param requests Specifies the collection of requests for which these records are being
          * sent.
          *
          * @param records Specifies a collection of records that have been received.  The
          * application must either process these records while blocking this call or it must make
          * copies of the records.  Once this call returns, the records may be returned to a queue
          * used to handle future incoming records.
          */
         typedef std::deque<record_handle> records_type;
         typedef std::list<request_handle> requests_type;
         virtual void on_sink_records(
            Manager *manager,
            requests_type &requests,
            records_type const &records) = 0;

         /**
          * Called to report that the state of a data request has changed.
          *
          * @param manager Specifies the data source manager.
          *
          * @param request Specifies the request whose state has changed.
          */
         virtual void on_request_state_change(
            Manager *manager, Request *request)
         { }

         /**
          * Used by the data source manager to report a collection of related records to the same
          * sink.
          */
         static void report_sink_records(
            Manager *manager,
            requests_type &requests,
            records_type const &records);

         /**
          * Writes a text description of a sink failure code to the specified stream.
          */
         static void format_sink_failure(
            std::ostream &out, sink_failure_type failure);

         /**
          * Called when a set variable attempt has been completed.
          *
          * @param manager Specifies the data source manager.
          *
          * @param uri Specifies the variable URI.
          *
          * @param outcome Specifies a code that describes the outcome of the operation.
          */
         enum set_outcome_type
         {
            set_outcome_unknown = 0,
            set_outcome_succeeded = 1,
            set_outcome_connection_failed = 2,
            set_outcome_invalid_logon = 3,
            set_outcome_server_security_blocked = 4,
            set_outcome_column_read_only = 5,
            set_outcome_invalid_table_name = 6,
            set_outcome_invalid_column_name = 7,
            set_outcome_invalid_subscript = 8,
            set_outcome_invalid_data_type = 9,
            set_outcome_communication_failed = 10,
            set_outcome_communication_disabled = 11,
            set_outcome_logger_security_blocked = 12,
            set_outcome_unmatched_logger_table_definition = 13,
            set_outcome_invalid_device_name = 14
         };
         virtual void on_set_complete(
            Manager *manager,
            StrUni const &uri,
            set_outcome_type outcome)
         { }

         /**
          * Writes a text description of the specified set outcome code to the specified stream.
          */
         static void format_set_outcome(
            std::ostream &out, set_outcome_type outcome);

         /**
          * Called when a send file attempt has been completed.
          *
          * @param manager Specifies the data source manager.
          *
          * @param station_uri Specifies the uri that identifies the station.
          *
          * @param file_name Specifies the name of the file that was being sent.
          *
          * @param outcome Specifies a code that describes the outcome.
          */
         enum send_file_outcome_type
         {
            send_file_unknown = 0,
            send_file_success = 1,
            send_file_connection_failed = 2,
            send_file_invalid_logon = 3,
            send_file_invalid_station_uri = 4,
            send_file_server_security_blocked = 5,
            send_file_communication_failed = 6,
            send_file_communication_disabled = 7,
            send_file_invalid_file_name = 8,
            send_file_logger_resource_error = 9,
            send_file_logger_security_blocked = 10,
            send_file_logger_root_full = 11,
            send_file_invalid_uri = 12
         };
         virtual void on_send_file_complete(
            Manager *manager,
            StrUni const &station_uri,
            StrUni const &file_name,
            send_file_outcome_type outcome)
         { }

         /**
          * Writes a text description of the specified file send outcome code to the specified
          * stream.
          */
         static void format_send_file_outcome(std::ostream &out, send_file_outcome_type outcome);

         /**
          * Called when a fragment of the newest file requested by the data source's
          * start_get_newest_file() has been received.
          *
          * @return Returns true if the operation should continue.  If false, the retrieval
          * operation will be abandoned.
          *
          * @param manager Specifies the data source manager.
          *
          * @param status Specifies the status of the operation.
          *
          * @param uri Specifies the uri that was specified when the operation was started.
          *
          * @param pattern Specifies the pattern that was requested.
          *
          * @param file_name Specifies the name of the file on the datalogger being returned.
          *
          * @param buff Specifies the file fragment being returned.
          *
          * @param buff_len Specifies the length of the fragment being returned.
          */
         enum get_newest_file_status_type
         {
            get_newest_status_unknown_failure = 0,
            get_newest_status_complete = 1,
            get_newest_status_in_progress = 2,
            get_newest_status_invalid_logon  = 3,
            get_newest_status_invalid_uri = 4,
            get_newest_status_connection_failed = 5,
            get_newest_status_server_permission_denied = 6,
            get_newest_status_communication_failed = 7,
            get_newest_status_communication_disabled = 8,
            get_newest_status_logger_permission_denied = 9,
            get_newest_status_invalid_file_name = 10,
            get_newest_status_unsupported = 11,
            get_newest_status_no_file = 12
         };
         virtual bool on_get_newest_file_status(
            Manager *manager,
            get_newest_file_status_type status,
            StrUni const &uri,
            StrUni const &pattern,
            StrUni const &file_name = "",
            void const *buff = 0,
            uint4 buff_len = 0)
         { return false; }

         /**
          * Writes a text description of the specified get newest file status to the specified
          * stream.
          *
          * @param out Specifies the stream to which the description will be written.
          *
          * @param status Specifies the status to describe.
          */
         static void format_get_newest_file_status(std::ostream &out, get_newest_file_status_type status);

         /**
          * Called when an operation to check or set the datalogger clock has been completed.
          *
          * @param manager Specifies the data source manager.
          *
          * @param uri Specifies the station URI that was specified at the start.
          *
          * @param outcome Specifies the status of the attempt.
          *
          * @param logger_time Specifies the datalogger time.  If the logger clock was changed, this
          * value will be the value BEFORE the logger clock was changed.
          */
         enum clock_outcome_type
         {
            clock_failure_unknown = 0,
            clock_success_checked = 1,
            clock_success_set = 2,
            clock_failure_invalid_logon = 3,
            clock_failure_invalid_uri = 4,
            clock_failure_connection = 5,
            clock_failure_server_permission = 6,
            clock_failure_communication = 7,
            clock_failure_communication_disabled = 8,
            clock_failure_logger_permission = 9,
            clock_failure_unsupported = 10,
            clock_failure_busy = 11
         };
         virtual void on_clock_complete(
            Manager *manager,
            StrUni const &uri,
            clock_outcome_type outcome,
            Csi::LgrDate const &logger_time = 0)
         { }

         /**
          * Formats the specified clock outcome to the provided stream.
          */
         static void format_clock_outcome(std::ostream &out, clock_outcome_type outcome);


         /**
          * Called to report the completion of a file control operation that was started by a call
          * to the source's start_file_control() method.
          *
          * @param manager Specifies the manager that owns the file source.
          *
          * @param uri Specifies the data source URI that was used to identify the station.
          *
          * @param command Specifies the file control command code that was used.
          *
          * @param p1 Specifies the p1 parameter that was used.
          *
          * @param p2 specifies the p2 parameter that was used.
          *
          * @param outcome Specifies a code that identifies the outcome of the operation.
          *
          * @param hold_off Specifies the estimated time, in milli-seconds, the datalogger will
          * require to reboot.
          */
         enum file_control_outcome_type
         {
            filecontrol_failure_unknown = 0,
            filecontrol_success = 1,
            filecontrol_failure_invalid_logon = 2,
            filecontrol_failure_connection = 3,
            filecontrol_failure_invalid_uri = 4,
            filecontrol_failure_unsupported = 5,
            filecontrol_failure_server_permission = 6,
            filecontrol_failure_communication = 7,
            filecontrol_failure_communication_disabled = 8,
            filecontrol_failure_logger_permission = 9,
            filecontrol_failure_logger_resources = 10,
            filecontrol_failure_invalid_file_name = 11,
            filecontrol_failure_unsupported_command = 12,
            filecontrol_failure_logger_locked = 13,
            filecontrol_failure_logger_root_dir_full = 14,
            filecontrol_failure_file_busy = 15,
            filecontrol_failure_drive_busy = 16
         };
         virtual void on_file_control_complete(
            Manager *manager,
            StrUni const &uri,
            uint4 command,
            StrAsc const &p1,
            StrAsc const &p2,
            file_control_outcome_type outcome,
            uint2 hold_off = 0)
         { }

         /**
          * Formats the specified file control outcome code to the provided stream.
          */
         static void format_file_control_outcome(std::ostream &out, file_control_outcome_type outcome);

         /**
          * Called to report that the source has determined its access level.
          *
          * @param sender Specifies the manager that owns the source.
          *
          * @param outcome Specifies the outcome of the operation.
          *
          * @param access_level Specifies the access level that was determined.  Ignore unless
          * outcome is set to check_auth_outcome_success.
          */
         enum check_access_level_outcome_type
         {
            check_access_level_outcome_failure_unknown = -1,
            check_access_level_outcome_success = 0,
            check_access_level_outcome_failure_unsupported = 1,
            check_access_level_outcome_failure_connection = 2,
            check_access_level_outcome_failure_invalid_uri = 3,
            check_access_level_outcome_failure_communication = 4,
            check_access_level_outcome_failure_communication_disabled = 5,
            check_access_level_outcome_failure_invalid_logon = 6
         };
         virtual void on_check_access_level_complete(
            Manager *sender,
            check_access_level_outcome_type outcome,
            Csi::PasswdHelpers::access_level_type access_level)
         { }

         /**
          * Called to format the outcome code for the check access level operation.
          *
          * @param out Specifies the stream to which the description will be written.
          *
          * @param outome Specifies the outcome to describe.
          */
         static void format_check_access_level_outcome(std::ostream &out, check_access_level_outcome_type outcome);

         /**
          * Called to report that the source has completed a requested list files operation.
          *
          * @param outcome Specifies the outcome of the operation.
          *
          * @param station_uri Specifies the station URI for the operation.
          *
          * @param transaction Specifies the transaction for the operation.
          *
          * @param filter Specifies the filter that was used.
          *
          * @param files Specifies the collection of file descriptors that were returned.
          */
         enum list_files_outcome_type
         {
            list_files_outcome_failure_unknown = 0,
            list_files_outcome_success = 1,
            list_files_outcome_failure_invalid_station_uri = 2,
            list_files_outcome_failure_comms = 3,
            list_files_outcome_failure_comms_disabled = 4,
            list_files_outcome_failure_logger_security = 5,
            list_files_outcome_failure_server_security = 6,
            list_files_outcome_failure_session = 7,
            list_files_outcome_failure_logon = 8,
            list_files_outcome_failure_unsupported = 9
         };
         typedef Cora::Device::FileListerHelpers::file_type list_file_type;
         typedef Cora::Device::FileListerClient::file_list_type files_type;
         virtual void on_list_files_complete(
            Manager *sender,
            list_files_outcome_type outcome,
            StrUni const &station_uri,
            int8 transaction,
            StrAsc const &filter,
            files_type const &files)
         { }

         /**
          * Formats the specified list files outcome type to the given stream.
          *
          * @param out Specifies the stream.
          *
          * @param outcome Specifies the outcome to format
          */
         static void format_list_files_outcome(std::ostream &out, list_files_outcome_type outcome);
      };
   };
};


#endif

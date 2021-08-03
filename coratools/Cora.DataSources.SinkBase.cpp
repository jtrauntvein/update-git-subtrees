/* Cora.DataSources.SinkBase.cpp

   Copyright (C) 2009, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 18 March 2009
   Last Change: Saturday 21 April 2018
   Last Commit: $Date: 2018-04-21 13:02:22 -0600 (Sat, 21 Apr 2018) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.DataSources.SinkBase.h"
#include "Cora.DataSources.Manager.h"
#include "Cora.Device.DeviceBase.h"
#include "coratools.strings.h"


namespace Cora
{
   namespace DataSources
   {
      void SinkBase::report_sink_records(
         Manager *manager,
         requests_type &requests,
         records_type const &records)
      {
         requests_type temp(requests);
         while(!temp.empty())
         {
            // we need to pick out all requests that have the same sink
            requests_type batch;
            requests_type::iterator rqi = temp.begin();
            while(rqi != temp.end())
            {
               request_handle &request = *rqi;
               if(batch.empty() || batch.front()->get_sink() == request->get_sink())
               {
                  requests_type::iterator dqi = rqi++;
                  if(request->get_state() != Request::state_remove_pending)
                     batch.push_back(request);
                  temp.erase(dqi);
               }
               else
                  ++rqi;
            }

            // we can now report the records for this batch
            if(!batch.empty())
            {
               Manager::supervisor_handle supervisor(manager->get_supervisor());
               records_type temp_records(records);
               SinkBase *sink = batch.front()->get_sink();
               
               if(supervisor != 0)
                  supervisor->on_sink_data(manager, requests, temp_records);
               if(!temp_records.empty() && SinkBase::is_valid_instance(sink))
                  sink->on_sink_records(manager, batch, temp_records);
            }
         }
      } // report_sink_records


      void SinkBase::format_sink_failure(
         std::ostream &out, sink_failure_type failure)
      {
         using namespace SinkBaseStrings;
         switch(failure)
         {
         case sink_failure_invalid_source:
            out << my_strings[strid_failure_invalid_source];
            break;
            
         case sink_failure_connection_failed:
            out << my_strings[strid_failure_connection_failed];
            break;
            
         case sink_failure_invalid_logon:
            out << my_strings[strid_failure_invalid_logon];
            break;
            
         case sink_failure_invalid_station_name:
            out << my_strings[strid_failure_invalid_station_name];
            break;
            
         case sink_failure_invalid_table_name:
            out << my_strings[strid_failure_invalid_table_name];
            break;
            
         case sink_failure_server_security:
            out << my_strings[strid_failure_server_security];
            break;
            
         case sink_failure_invalid_start_option:
            out << my_strings[strid_failure_invalid_start_option];
            break;
            
         case sink_failure_invalid_order_option:
            out << my_strings[strid_failure_invalid_order_option];
            break;
            
         case sink_failure_table_deleted:
            out << my_strings[strid_failure_table_deleted];
            break;
            
         case sink_failure_station_shut_down:
            out << my_strings[strid_failure_station_shut_down];
            break;
            
         case sink_failure_unsupported:
            out << my_strings[strid_failure_unsupported];
            break;
            
         case sink_failure_invalid_column_name:
            out << my_strings[strid_failure_invalid_column_name];
            break;
            
         case sink_failure_invalid_array_address:
            out << my_strings[strid_failure_invalid_array_address];
            break;
            
         default:
            out << my_strings[strid_failure_unknown];
            break;
         }
      } // format_sink_failure


      void SinkBase::format_set_outcome(
         std::ostream &out, set_outcome_type outcome)
      {
         using namespace Cora::Device::VariableSetterStrings;
         using namespace Cora::Device;
         switch(outcome)
         {
         case set_outcome_succeeded:
            out << my_strings[strid_outcome_succeeded];
            break;
            
         case set_outcome_connection_failed:
            DeviceBase::format_devicebase_failure(out, DeviceBase::devicebase_failure_session);
            break;
            
         case set_outcome_invalid_logon:
            DeviceBase::format_devicebase_failure(out, DeviceBase::devicebase_failure_logon);
            break;
            
         case set_outcome_server_security_blocked:
            DeviceBase::format_devicebase_failure(out, DeviceBase::devicebase_failure_security);
            break;
            
         case set_outcome_column_read_only:
            out << my_strings[strid_outcome_column_read_only];
            break;
            
         case set_outcome_invalid_table_name:
            out << my_strings[strid_outcome_invalid_table_name];
            break;
            
         case set_outcome_invalid_column_name:
            out << my_strings[strid_outcome_invalid_column_name];
            break;
            
         case set_outcome_invalid_subscript:
            out << my_strings[strid_outcome_invalid_subscript];
            break;
            
         case set_outcome_invalid_data_type:
            out << my_strings[strid_outcome_invalid_data_type];
            break;
            
         case set_outcome_communication_failed:
            out << my_strings[strid_outcome_communication_failed];
            break;
            
         case set_outcome_communication_disabled:
            out << my_strings[strid_outcome_communication_disabled];
            break;
            
         case set_outcome_logger_security_blocked:
            out << my_strings[strid_outcome_logger_security_blocked];
            break;
            
         case set_outcome_unmatched_logger_table_definition:
            out << my_strings[strid_outcome_invalid_table_defs];
            break;
            
         case set_outcome_invalid_device_name:
            DeviceBase::format_devicebase_failure(out, DeviceBase::devicebase_failure_invalid_device_name);
            break;
            
         default:
            DeviceBase::format_devicebase_failure(out, DeviceBase::devicebase_failure_unknown);
            break;
         }
      } // format_set_outcome


      void SinkBase::format_send_file_outcome(std::ostream &out, send_file_outcome_type outcome)
      {
         using namespace Cora::Device::FileSenderStrings;
         using namespace Cora::Device;
         switch(outcome)
         {
         case send_file_success:
            out << my_strings[strid_outcome_success];
            break;
            
         case send_file_connection_failed:
            DeviceBase::format_devicebase_failure(out, DeviceBase::devicebase_failure_session);
            break;
            
         case send_file_invalid_logon:
            DeviceBase::format_devicebase_failure(out, DeviceBase::devicebase_failure_logon);
            break;
            
         case send_file_invalid_station_uri:
         case send_file_invalid_uri:
            DeviceBase::format_devicebase_failure(out, DeviceBase::devicebase_failure_invalid_device_name);
            break;
            
         case send_file_server_security_blocked:
            DeviceBase::format_devicebase_failure(out, DeviceBase::devicebase_failure_security);
            break;
            
         case send_file_communication_failed:
            out << my_strings[strid_outcome_communication_failed];
            break;
            
         case send_file_communication_disabled:
            out << my_strings[strid_outcome_communication_disabled];
            break;
            
         case send_file_invalid_file_name:
            out << my_strings[strid_outcome_invalid_file_name];
            break;
            
         case send_file_logger_resource_error:
            out << my_strings[strid_outcome_logger_resource_error];
            break;
            
         case send_file_logger_security_blocked:
            out << my_strings[strid_outcome_logger_permission_denied];
            break;
            
         case send_file_logger_root_full:
            out << my_strings[strid_outcome_logger_root_dir_full];
            break;
            
         default:
            DeviceBase::format_devicebase_failure(out, DeviceBase::devicebase_failure_unknown);
            break;
         }
      } // format_send_file_outcome


      void SinkBase::format_get_newest_file_status(std::ostream &out, get_newest_file_status_type status)
      {
         using namespace Cora::Device;
         switch(status)
         {
         default:
         case get_newest_status_unknown_failure:
            DeviceBase::format_devicebase_failure(out, DeviceBase::devicebase_failure_unknown);
            break;
            
         case get_newest_status_complete:
         case get_newest_status_in_progress:
            out << common_strings[common_success];
            break;
            
         case get_newest_status_invalid_logon:
            DeviceBase::format_devicebase_failure(out, DeviceBase::devicebase_failure_logon);
            break;
            
         case get_newest_status_invalid_uri:
            DeviceBase::format_devicebase_failure(out, DeviceBase::devicebase_failure_invalid_device_name);
            break;
            
         case get_newest_status_connection_failed:
            DeviceBase::format_devicebase_failure(out, DeviceBase::devicebase_failure_session);
            break;
            
         case get_newest_status_server_permission_denied:
            DeviceBase::format_devicebase_failure(out, DeviceBase::devicebase_failure_security);
            break;
            
         case get_newest_status_communication_failed:
            out << common_strings[common_comm_failed];
            break;
            
         case get_newest_status_communication_disabled:
            out << common_strings[common_comm_disabled];
            break;
            
         case get_newest_status_logger_permission_denied:
            out << common_strings[common_logger_security_blocked];
            break;
            
         case get_newest_status_invalid_file_name:
            out << common_strings[common_invalid_file_name];
            break;
            
         case get_newest_status_unsupported:
            DeviceBase::format_devicebase_failure(out, DeviceBase::devicebase_failure_unsupported);
            break;
            
         case get_newest_status_no_file:
            out << "no matching file found";
            break;
         }
      } // format_get_newest_file_status
      

      void SinkBase::format_clock_outcome(std::ostream &out, clock_outcome_type outcome)
      {
         using namespace Cora::Device::ClockSetterStrings;
         using namespace Cora::Device;
         switch(outcome)
         {
         default:
         case clock_failure_unknown:
            DeviceBase::format_devicebase_failure(out, DeviceBase::devicebase_failure_unknown);
            break;
            
         case clock_success_checked:
            out << my_strings[strid_clock_checked];
            break;
            
         case clock_success_set:
            out << my_strings[strid_clock_set];
            break;
            
         case clock_failure_invalid_logon:
            DeviceBase::format_devicebase_failure(out, DeviceBase::devicebase_failure_logon);
            break;
            
         case clock_failure_invalid_uri:
            DeviceBase::format_devicebase_failure(out, DeviceBase::devicebase_failure_invalid_device_name);
            break;
            
         case clock_failure_connection:
            DeviceBase::format_devicebase_failure(out, DeviceBase::devicebase_failure_session);
            break;
            
         case clock_failure_server_permission:
            DeviceBase::format_devicebase_failure(out, DeviceBase::devicebase_failure_security);
            break;
            
         case clock_failure_communication:
            out << my_strings[strid_communication_failed];
            break;
            
         case clock_failure_communication_disabled:
            out << my_strings[strid_communication_disabled];
            break;
            
         case clock_failure_logger_permission:
            out << my_strings[strid_logger_security_blocked];
            break;
            
         case clock_failure_unsupported:
            break;
            
         case clock_failure_busy:
            break;
         }
      } // format_clock_outcome


      void SinkBase::format_file_control_outcome(
         std::ostream &out, file_control_outcome_type outcome)
      {
         using namespace Cora::Device::FileControllerStrings;
         using Cora::Device::DeviceBase;
         switch(outcome)
         {
         default:
         case filecontrol_failure_unknown:
            DeviceBase::format_devicebase_failure(out, DeviceBase::devicebase_failure_unknown);
            break;
            
         case filecontrol_success:
            out << my_strings[strid_outcome_success];
            break;
            
         case filecontrol_failure_invalid_logon:
            DeviceBase::format_devicebase_failure(out, DeviceBase::devicebase_failure_logon);
            break;
            
         case filecontrol_failure_connection:
            DeviceBase::format_devicebase_failure(out, DeviceBase::devicebase_failure_session);
            break;
            
         case filecontrol_failure_invalid_uri:
            DeviceBase::format_devicebase_failure(out, DeviceBase::devicebase_failure_invalid_device_name);
            break;
            
         case filecontrol_failure_unsupported:
            DeviceBase::format_devicebase_failure(out, DeviceBase::devicebase_failure_unsupported);
            break;
            
         case filecontrol_failure_server_permission:
            DeviceBase::format_devicebase_failure(out, DeviceBase::devicebase_failure_security);
            break;
            
         case filecontrol_failure_communication:
            out << my_strings[strid_outcome_logger_communication_failed];
            break;
            
         case filecontrol_failure_communication_disabled:
            out << my_strings[strid_outcome_logger_communication_disabled];
            break;
            
         case filecontrol_failure_logger_permission:
            out << my_strings[strid_outcome_logger_security_blocked];
            break;
            
         case filecontrol_failure_logger_resources:
            out << my_strings[strid_outcome_insufficient_logger_resources];
            break;
            
         case filecontrol_failure_invalid_file_name:
            out << my_strings[strid_outcome_invalid_file_name];
            break;
            
         case filecontrol_failure_unsupported_command:
            out << my_strings[strid_outcome_unsupported_command];
            break;
            
         case filecontrol_failure_logger_locked:
            out << my_strings[strid_outcome_logger_locked];
            break;
            
         case filecontrol_failure_logger_root_dir_full:
            out << my_strings[strid_outcome_logger_root_dir_full];
            break;
            
         case filecontrol_failure_file_busy:
            out << my_strings[strid_outcome_logger_file_busy];
            break;
            
         case filecontrol_failure_drive_busy:
            out << my_strings[strid_outcome_logger_drive_busy];
            break;
         }
      } // format_file_control_outcome


      void SinkBase::format_check_access_level_outcome(
         std::ostream &out, check_access_level_outcome_type outcome)
      {
         using namespace Cora::Device;
         switch(outcome)
         {
         case check_access_level_outcome_failure_unknown:
         default:
            DeviceBase::format_devicebase_failure(out, DeviceBase::devicebase_failure_unknown);
            break;

         case check_access_level_outcome_success:
            out << common_strings[common_success];
            break;

         case check_access_level_outcome_failure_unsupported:
            DeviceBase::format_devicebase_failure(out, DeviceBase::devicebase_failure_unsupported);
            break;
            
         case check_access_level_outcome_failure_connection:
            DeviceBase::format_devicebase_failure(out, DeviceBase::devicebase_failure_session);
            break;

         case check_access_level_outcome_failure_invalid_uri:
            DeviceBase::format_devicebase_failure(out, DeviceBase::devicebase_failure_invalid_device_name);
            break;
            
         case check_access_level_outcome_failure_communication:
            out << common_strings[common_comm_failed];
            break;
            
         case check_access_level_outcome_failure_communication_disabled:
            out << common_strings[common_comm_disabled];
            break;
            
         case check_access_level_outcome_failure_invalid_logon:
            DeviceBase::format_devicebase_failure(out, DeviceBase::devicebase_failure_logon);
            break;
         }
      } // format_check_access_level_outcome


      void SinkBase::format_list_files_outcome(std::ostream &out, list_files_outcome_type outcome)
      {
         using namespace Cora::Device;
         switch(outcome)
         {
         case list_files_outcome_failure_unknown:
         default:
            DeviceBase::format_devicebase_failure(out, DeviceBase::devicebase_failure_unknown);
            break;
            
         case list_files_outcome_success:
            out << common_strings[common_success];
            break;
            
         case list_files_outcome_failure_invalid_station_uri:
            DeviceBase::format_devicebase_failure(out, DeviceBase::devicebase_failure_invalid_device_name);
            break;
            
         case list_files_outcome_failure_comms:
            out << common_strings[common_comm_failed];
            break;
            
         case list_files_outcome_failure_comms_disabled:
            out << common_strings[common_comm_disabled];
            break;
            
         case list_files_outcome_failure_logger_security:
            out << common_strings[common_logger_security_blocked];
            break;
            
         case list_files_outcome_failure_server_security:
            DeviceBase::format_devicebase_failure(out, DeviceBase::devicebase_failure_security);
            break;
            
         case list_files_outcome_failure_session:
            DeviceBase::format_devicebase_failure(out, DeviceBase::devicebase_failure_session);
            break;
            
         case list_files_outcome_failure_logon:
            DeviceBase::format_devicebase_failure(out, DeviceBase::devicebase_failure_logon);
            break;
            
         case list_files_outcome_failure_unsupported:
            DeviceBase::format_devicebase_failure(out, DeviceBase::devicebase_failure_unsupported);
            break;
         }
      } // format_list_files_outcome
   };
};


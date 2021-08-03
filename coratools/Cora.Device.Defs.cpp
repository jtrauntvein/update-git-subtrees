/* Cora.Device.Defs.cpp

   Copyright (C) 2007, 2021 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 29 June 2007
   Last Change: Thursday 18 March 2021
   Last Commit: $Date: 2021-03-18 08:38:22 -0600 (Thu, 18 Mar 2021) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.Defs.h"
#include "Csi.MsgExcept.h"
#include "StrAsc.h"
#include <stdlib.h>


namespace Cora
{
   namespace Device
   {
      namespace CollectArea
      {
         namespace Persistence
         {
            char const *persistence_type_to_str(uint4 type_code)
            {
               char const *rtn = "";
               switch(type_code)
               {
               case logger_feature:
                  rtn = "logger-feature";
                  break;

               case table_def_feature:
                  rtn = "table-def-feature";
                  break;

               case client_defined:
                  rtn = "client-defined";
                  break;

               case auto_delete:
                  rtn = "auto-delete";
                  break;
               }
               return rtn;
            } // persistence_type_to_str
         };


         namespace Types
         {
            char const *collect_area_type_to_str(uint4 type_code)
            {
               char const *rtn = "";
               switch(type_code)
               {
               case unknown:
                  rtn = "unknown";
                  break;

               case classic_ports_flags:
                  rtn = "classic-ports-flags";
                  break;

               case classic_inlocs:
                  rtn = "classic-inlocs";
                  break;

               case classic_final_storage:
                  rtn = "classic-final-storage";
                  break;

               case classic_status:
                  rtn = "classic-status";
                  break;

               case bmp1_table:
                  rtn = "bmp1-table";
                  break;

               case bmp3_table:
                  rtn = "bmp3-table";
                  break;

               case bmp5_table:
                  rtn = "bmp5-table";
                  break;

               case sm_last_file_mark:
                  rtn = "sm-last-file-mark";
                  break;

               case alert2_sensor_report:
                  rtn = "alert2-sensor-report";
                  break;

               case data_file_import:
                  rtn = "data-file-import";
                  break;

               case aloha_station_report:
                  rtn = "aloha-station-report";
                  break;

               case gps_position_report:
                  rtn = "gps-position-report";
                  break;

               case gps_satellites_report:
                  rtn = "gps-satellites-report";
                  break;
               }
               return rtn;
            } // collect_area_type_to_str
         };
         
         
         namespace Settings
         {
            char const *setting_id_to_str(uint4 setting_id)
            {
               char const *rtn = "";
               switch(setting_id)
               {
               case tables_written:
                  rtn = "tables-written";
                  break;
                  
               case schedule_enabled:
                  rtn = "schedule-enabled";
                  break;
                  
               case fs_area:
                  rtn = "fs-area";
                  break;
                  
               case fs_cache_data:
                  rtn = "fsCacheData";
                  break;
                  
               case fs_output_option:
                  rtn = "fsOutputOption";
                  break;
                  
               case fs_output_format:
                  rtn = "fs-output-format";
                  break;
                  
               case fs_output_name:
                  rtn = "fsOutputName";
                  break;
                  
               case fs_collect_mode:
                  rtn = "fs-collect-mode";
                  break;
                  
               case fs_collect_all_on_first_poll:
                  rtn = "fs-collect-all-on-first-poll";
                  break;
                  
               case fs_arrays_to_collect_on_first_poll:
                  rtn = "fs-arrays-to-collect-on-first-poll";
                  break;
                  
               case fs_max_arrays_to_poll:
                  rtn = "fs-max-arrays-to-poll";
                  break;
                  
               case fs_current_loc:
                  rtn = "fs-current-loc";
                  break;
                  
               case table_last_record_no:
                  rtn = "table-last-record-no";
                  break;
                  
               case inloc_ids:
                  rtn = "inloc-ids";
                  break;
                  
               case cache_data:
                  rtn = "cache-data";
                  break;
                  
               case data_file_output_option:
                  rtn = "data-file-output-option";
                  break;
                  
               case data_file_output_name:
                  rtn = "data-file-output-name";
                  break;
                  
               case data_file_timestamp_resolution:
                  rtn = "data-file-timestamp-resolution";
                  break;
                  
               case data_file_output_format:
                  rtn = "dataFileOutputFormat";
                  break;
                  
               case data_file_toa_header_format:
                  rtn = "dataFileToaHeaderFormat";
                  break;
                  
               case expanded_data_file_output_name:
                  rtn = "expanded-data-file-output-name";
                  break;
                  
               case use_default_data_file_output_name:
                  rtn = "use-default-data-file-output-name";
                  break;
                  
               case fs_values_to_poll:
                  rtn = "fs-values-to-poll";
                  break;
                  
               case table_collect_mode:
                  rtn = "table-collect-mode";
                  break;
                  
               case table_collect_all_on_first_poll:
                  rtn = "table-collect-all-on-first-poll";
                  break;
                  
               case table_records_to_collect_on_first_poll:
                  rtn = "table-records-to-collect-on-first-poll";
                  break;
                  
               case table_max_records_to_poll:
                  rtn = "table-max-records-to-poll";
                  break;
                  
               case table_file_format:
                  rtn = "table-file-format";
                  break;
                  
               case logger_table_no:
                  rtn = "logger-table-no";
                  break;
                  
               case custom_csv_format_options:
                  rtn = "custom-csv-format-options";
                  break;
                  
               case toa5_format_options:
                  rtn = "toa5-format-options";
                  break;
                  
               case tob1_format_options:
                  rtn = "tob1-format-options";
                  break;
                  
               case noh_format_options:
                  rtn = "noh-format-options";
                  break;
                  
               case csixml_format_options:
                  rtn = "csixml-format-options";
                  break;
                  
               case last_data_file_output_name:
                  rtn = "last-data-file-output-name";
                  break;

               case table_max_interval_to_poll:
                  rtn = "table-max-interval-to-poll";
                  break;

               case alert2_sensor_id:
                  rtn = "alert2-sensor-id";
                  break;

               case alert2_convert_expression:
                  rtn = "alert2-convert-expression";
                  break;

               case alert2_value_data_type:
                  rtn = "alert2-value-date-type";
                  break;

               case import_file_watch_dir:
                  rtn = "import-file-watch-dir";
                  break;

               case import_file_watch_pattern:
                  rtn = "import-file-watch-pattern";
                  break;

               case aloha_station_name:
                  rtn = "aloha-station-name";
                  break;
               }
               return rtn;
            } // setting_id_to_str


            setting_id_type str_to_setting_id(char const *s)
            {
               StrAsc name(s);
               struct names_map_type
               {
                  char const *name;
                  uint4 const id;
               } names_map[] = {
                  { "tables-written", tables_written },
                  { "tablesWritten", tables_written },
                  { "schedule-enabled", schedule_enabled },
                  { "scheduleEnabled", schedule_enabled },
                  { "fs-area", fs_area },
                  { "fsArea", fs_area },
                  { "fs-cache-data", fs_cache_data },
                  { "fsCacheData", fs_cache_data },
                  { "fs-output-option", fs_output_option },
                  { "fsOutputOption", fs_output_option },
                  { "fs-output-format", fs_output_format },
                  { "fsOutputFormat", fs_output_format },
                  { "fs-output-name", fs_output_name },
                  { "fsOutputName", fs_output_name },
                  { "fs-collect-mode", fs_collect_mode },
                  { "fsCollectMode", fs_collect_mode },
                  { "fs-collect-all-on-first-poll", fs_collect_all_on_first_poll },
                  { "fsCollectAllOnFirstPoll", fs_collect_all_on_first_poll },
                  { "fs-arrays-to-collect-on-first-poll", fs_arrays_to_collect_on_first_poll },
                  { "fsArraysToCollectOnFirstPoll", fs_arrays_to_collect_on_first_poll },
                  { "fs-max-arrays-to-poll", fs_max_arrays_to_poll },
                  { "fsMaxArraysToPoll", fs_max_arrays_to_poll },
                  { "fs-current-loc", fs_current_loc },
                  { "fsCurrentLoc", fs_current_loc },
                  { "table-last-record-no", table_last_record_no },
                  { "tableLastRecordNo", table_last_record_no },
                  { "inloc-ids", inloc_ids },
                  { "inlocIds", inloc_ids },
                  { "cache-data", cache_data },
                  { "cacheData", cache_data },
                  { "data-file-output-option", data_file_output_option },
                  { "dataFileOutputOption", data_file_output_option },
                  { "data-file-output-name", data_file_output_name },
                  { "dataFileOutputName", data_file_output_name },
                  { "data-file-timestamp-resolution", data_file_timestamp_resolution },
                  { "dataFileTimestampResolution", data_file_timestamp_resolution },
                  { "data-file-output-format", data_file_output_format },
                  { "dataFileOutputFormat", data_file_output_format },
                  { "data-file-toa-header-format", data_file_toa_header_format },
                  { "dataFileToaHeaderFormat", data_file_toa_header_format },
                  { "expanded-data-file-output-name", expanded_data_file_output_name },
                  { "expandedDataFileOutputName", expanded_data_file_output_name },
                  { "use-default-data-file-output-name", use_default_data_file_output_name },
                  { "useDefaultDataFileOutputName", use_default_data_file_output_name },
                  { "fs-values-to-poll", fs_values_to_poll },
                  { "fsValuesToPoll", fs_values_to_poll },
                  { "table-collect-mode", table_collect_mode },
                  { "tableCollectMode", table_collect_mode },
                  { "table-collect-all-on-first-poll", table_collect_all_on_first_poll },
                  { "tableCollectAllOnFirstPoll", table_collect_all_on_first_poll },
                  { "table-records-to-collect-on-first-poll", table_records_to_collect_on_first_poll },
                  { "tableRecordsToCollectOnFirstPoll", table_records_to_collect_on_first_poll },
                  { "table-max-records-to-poll", table_max_records_to_poll },
                  { "tableMaxRecordsToPoll", table_max_records_to_poll },
                  { "table-file-format", table_file_format },
                  { "tableFileFormat", table_file_format },
                  { "logger-table-no", logger_table_no },
                  { "loggerTableNo", logger_table_no },
                  { "custom-csv-format-options", custom_csv_format_options },
                  { "customCsvFormatOptions", custom_csv_format_options },
                  { "toa5-format-options", toa5_format_options },
                  { "toa5FormatOptions", toa5_format_options },
                  { "tob1-format-options", tob1_format_options },
                  { "tob1FormatOptions", tob1_format_options },
                  { "noh-format-options", noh_format_options },
                  { "nohFormatOptions", noh_format_options },
                  { "csixml-format-options", csixml_format_options },
                  { "csixmlFormatOptions", csixml_format_options },
                  { "last-data-file-output-name", last_data_file_output_name },
                  { "lastDataFileOutputName", last_data_file_output_name },
                  { "table-max-interval-to-poll", table_max_interval_to_poll },
                  { "tableMaxIntervalToPoll", table_max_interval_to_poll },
                  { "alert2-sensor-id", alert2_sensor_id },
                  { "alert2SensorId", alert2_sensor_id },
                  { "alert2ConvertExpression", alert2_convert_expression },
                  { "alert2-convert-expression", alert2_convert_expression },
                  { "alert2ValueDataType", alert2_value_data_type },
                  { "alert2-value-data-type", alert2_value_data_type },
                  { "import-file-watch-dir", import_file_watch_dir },
                  { "importFileWatchDir", import_file_watch_dir },
                  { "import-file-watch-pattern", import_file_watch_pattern },
                  { "importFileWatchPattern", import_file_watch_pattern },
                  { "aloha-station-name", aloha_station_name },
                  { "alohaStationName", aloha_station_name },
                  { 0, 0 }
               };
               uint4 rtn = 0;

               for(int i = 0; names_map[i].name != 0 && rtn == 0; ++i)
                  if(name == names_map[i].name)
                     rtn = names_map[i].id;
               if(rtn == 0)
               {
                  // the identifier may be numeric.  We will convert it to a number and test it
                  rtn = strtoul(s,0,10);
                  if(setting_id_to_str(rtn)[0] == 0)
                     throw Csi::MsgExcept("Invalid setting idenfier");
               }
               return static_cast<setting_id_type>(rtn);
            } // str_to_setting_id
         };
      };


      namespace Settings
      {
         char const *setting_id_to_str(uint4 setting_id)
         {
            char const *rtn = "";
            switch(setting_id)
            {
            case clock_check_sched:
               rtn = "clock-check-schedule";
               break;
               
            case max_time_on_line:
               rtn = "max-time-on-line";
               break;
               
            case max_packet_size:
               rtn = "max-packet-size";
               break;
               
            case extra_response_time:
               rtn = "extra-response-time";
               break;
               
            case collect_schedule:
               rtn = "collect-schedule";
               break;
               
            case security_code:
               rtn = "security-code";
               break;
               
            case do_hole_collect:
               rtn = "do-hole-collect";
               break;
               
            case baud_rate:
               rtn = "baudRate";
               break;
               
            case switch_id:
               rtn = "switch-id";
               break;
               
            case logger_program_info:
               rtn = "logger-program-info";
               break;
               
            case low_level_poll_schedule:
               rtn = "low-level-poll-schedule";
               break;
               
            case com_port_id:
               rtn = "com-port-id";
               break;
               
            case bmp1_station_id:
               rtn = "bmp1-station-id";
               break;
               
            case collect_via_advise:
               rtn = "collect-via-advise";
               break;
               
            case tables_to_exclude:
               rtn = "tablesToExclude";
               break;
               
            case time_zone_difference:
               rtn = "time-zone-difference";
               break;
               
            case table_size_factor:
               rtn = "table-size-factor";
               break;
               
            case comm_enabled:
               rtn = "comm-enabled";
               break;
               
            case dial_string_list:
               rtn = "dial-string-list";
               break;
               
            case phone_modem_type:
               rtn = "phone-modem-type";
               break;
               
            case settings_overriden:
               rtn = "settingsOverriden";
               break;
               
            case data_collection_enabled:
               rtn = "data-collection-enabled";
               break;
               
            case qtracs_eal_dir:
               rtn = "qtracsEalDir";
               break;
               
            case qtracs_poll_interval:
               rtn = "qtracsPollInterval";
               break;
               
            case qtracs_mct_no:
               rtn = "qtracsMctNo";
               break;
               
            case qtracs_dial_now_dir:
               rtn = "qtracsDialNowDir";
               break;
               
            case data_broker_id:
               rtn = "data-broker-id";
               break;
               
            case max_inlocs_per_request:
               rtn = "max-inlocs-per-request";
               break;
               
            case callback_enabled:
               rtn = "callback-enabled";
               break;
               
            case callback_id:
               rtn = "callback-id";
               break;
               
            case input_location_labels:
               rtn = "input-location-labels";
               break;
               
            case rf_use_f:
               rtn = "rf-use-f";
               break;
               
            case rf_use_u:
               rtn = "rf-use-u";
               break;
               
            case rf_use_w:
               rtn = "rf-use-w";
               break;
               
            case hole_addition_enabled:
               rtn = "holeAdditionEnabled";
               break;
               
            case udp_address:
               rtn = "udpAddress";
               break;
               
            case udp_port:
               rtn = "udpPort";
               break;
               
            case udp_first_packet_delay:
               rtn = "udpFirstPacketDelay";
               break;
               
            case udp_send_null_attention:
               rtn = "udpSendNullAttention";
               break;
               
            case bmp1_mutex_name:
               rtn = "bmp1MutexName";
               break;
               
            case generic_dial_script:
               rtn = "generic-dialing-script";
               break;
               
            case generic_end_script:
               rtn = "generic-end-script";
               break;
               
            case generic_half_duplex:
               rtn = "generic-half-duplex";
               break;
               
            case generic_raise_dtr:
               rtn = "generic-raise-dtr";
               break;
               
            case generic_rts_cts_use:
               rtn = "generic-rts-cts-use";
               break;
               
            case bmp1_low_level_delay:
               rtn = "bmp1-low-level-delay";
               break;
               
            case pakbus_route_broadcast_interval:
               rtn = "pakbus-route-broadcast-interval";
               break;
               
            case pakbus_node_identifier:
               rtn = "pakbus-node-identifier";
               break;
               
            case default_schedule_enabled:
               rtn = "defaultScheduleEnabled";
               break;
               
            case default_cache_data:
               rtn = "defaultCacheData";
               break;
               
            case default_data_file_output_option:
               rtn = "defaultDataFileOutputOption";
               break;
               
            case default_data_file_output_name:
               rtn = "defaultDataFileOutputName";
               break;
               
            case default_data_file_time_stamp_resolution:
               rtn = "defaultDataFileTimeStampResolution";
               break;
               
            case default_data_file_output_format:
               rtn = "defaultDataFileOutputFormat";
               break;
               
            case default_data_file_toa_header_format:
               rtn = "defaultDataFileToaHeaderFormat";
               break;
               
            case use_tapi_dialing_properties:
               rtn = "useTapiDialingProperties";
               break;
               
            case tapi_country_code:
               rtn = "tapiCountryCode";
               break;
               
            case tapi_area_code:
               rtn = "tapiAreaCode";
               break;
               
            case tapi_dial_string:
               rtn = "tapiDialString";
               break;
               
            case secondary_collect_schedule_enabled:
               rtn = "secondary-collect-schedule-enabled";
               break;
               
            case stay_on_collect_schedule:
               rtn = "stay-on-collect-schedule";
               break;
               
            case root_delay_before_reopen:
               rtn = "root-delay-before-reopen";
               break;
               
            case max_baud_rate:
               rtn = "max-baud-rate";
               break;
               
            case pakbus_beacon_interval:
               rtn = "pakbus-beacon-interval";
               break;
               
            case pakbus_is_dialed_link:
               rtn = "pakbus-is-dialed-link";
               break;
               
            case rf400_network_id:
               rtn = "rf400NetworkId";
               break;
               
            case rf400_radio_id:
               rtn = "rf400RadioId";
               break;
               
            case rf400_attention_char:
               rtn = "rf400AttentionChar";
               break;
               
            case rf95_dial_retries:
               rtn = "rf95DialRetries";
               break;
               
            case rf95_custom_dial_string:
               rtn = "rf95CustomDialString";
               break;
               
            case table_defs_policy:
               rtn = "table-defs-policy";
               break;
               
            case pakbus_computer_id:
               rtn = "pakbus-computer-id";
               break;
               
            case bmp5_callback_enabled:
               rtn = "bmp5-callback-enabled";
               break;
               
            case default_table_file_format:
               rtn = "defaultTableFileFormat";
               break;
               
            case tcp_callback_port:
               rtn = "tcp-callback-port";
               break;
               
            case hangup_delay:
               rtn = "hangup-delay";
               break;
               
            case collect_ports_and_flags:
               rtn = "collectPortsAndFlags";
               break;
               
            case delay_comms_after_open:
               rtn = "delay-comms-after-open";
               break;
               
            case pakbus_router_name:
               rtn = "pakbus-router-name";
               break;
               
            case airlink_device_id:
               rtn = "airlinkDeviceId";
               break;
               
            case cache_ip_address:
               rtn = "cacheIpAddress";
               break;
               
            case current_program_name:
               rtn = "current-program-name";
               break;
               
            case user_description:
               rtn = "user-description";
               break;
               
            case max_cache_table_size:
               rtn = "max-cache-table-size";
               break;
               
            case create_cache_tables_only_in_memory:
               rtn = "create-cache-tables-only-in-memory";
               break;
               
            case allowed_pakbus_neighbours:
               rtn = "allowed-pakbus-neighbours";
               break;
               
            case default_custom_csv_format_options:
               rtn = "defaultCustomCsvFormatOptions";
               break;
               
            case pakbus_leaf_node:
               rtn = "pakbus-leaf-node";
               break;
               
            case file_synch_control:
               rtn = "fileSynchControl";
               break;
               
            case default_toa5_format_options:
               rtn = "defaultToa5FormatOptions";
               break;
               
            case default_tob1_format_options:
               rtn = "defaultTob1FormatOptions";
               break;
               
            case default_noh_format_options:
               rtn = "defaultNohFormatOptions";
               break;
               
            case default_csixml_format_options:
               rtn = "defaultCsixmlFormatOptions";
               break;
               
            case low_level_poll_enabled:
               rtn = "lowLevelPollEnabled";
               break;
               
            case prevent_tcp_open:
               rtn = "prevent-tcp-open";
               break;
               
            case tcp_callback_verify_time:
               rtn = "tcp-callback-verify-time";
               break;

            case pakbus_verify_interval:
               rtn = "pakbus-verify-interval";
               break;

            case pakbus_tcp_maintained_nodes:
               rtn = "pakbus-tcp-maintained-nodes";
               break;

            case file_synch_mode:
               rtn = "file-synch-mode";
               break;
               
            case file_synch_schedule_base:
               rtn = "file-synch-schedule-base";
               break;
               
            case file_synch_schedule_interval:
               rtn = "file-synch-schedule-interval";
               break;
               
            case file_synch_control_ex:
               rtn = "file-synch-control-ex";
               break;

            case tcp_password:
               rtn = "tcp-password";
               break;

            case pakbus_tcp_out_addresses:
               rtn = "pakbus-tcp-out-addresses";
               break;

            case reschedule_on_data:
               rtn = "reschedule-on-data";
               break;

            case delete_files_after_synch:
               rtn = "delete-files-after-synch";
               break;

            case rftd_poll_interval:
               rtn = "rfTdPollInterval";
               break;

            case serial_use_simplified_io:
               rtn = "serial-use-simplified-io";
               break;

            case pooled_serial_ports:
               rtn = "pooled-serial-ports";
               break;

            case pooled_terminal_servers:
               rtn = "pooled-terminal-servers";
               break;

            case tls_client_enabled:
               rtn = "tlsClientEnabled";
               break;

            case table_file_station_name_selector:
               rtn = "table-file-station-name-selector";
               break;

            case socket_pre_open_script:
               rtn = "socket-pre-open-script";
               break;

            case socket_post_close_script:
               rtn = "socket-post-close-script";
               break;

            case poll_for_statistics:
               rtn = "poll-for-statistics";
               break;

            case pakbus_encryption_key:
               rtn = "pakbus-encryption-key";
               break;

            case alert2_station_id:
               rtn = "alert2-station-id";
               break;

            case alert2_message_log_size:
               rtn = "alert2-message-log-size";
               break;

            case default_schedule_enabled_expr:
               rtn = "default-schedule-enabled-expr";
               break;

            case cc_proxy_conn_info:
               rtn = "cc-proxy-conn-info";
               break;

            case station_meta_json:
               rtn = "station-meta-json";
               break;

            case pakbus_ws_server_url:
               rtn = "pakbus-ws-server-url";
               break;

            case pakbus_ws_network_id:
               rtn = "pakbus-ws-network-id";
               break;

            case alert2_station_ports:
               rtn = "alert2-station-ports";
               break;

            case replication_enabled:
               rtn = "replication-enabled";
               break;

            case aloha_export_station_id:
               rtn = "aloha-export-station-id";
               break;

            case aloha_export_type:
               rtn = "aloha-export-type";
               break;

            case aloha_export_resource:
               rtn = "aloha-export-resource";
               break;

            case gps_update_system_clock:
               rtn = "gps-update-system-clock";
               break;

            case aloha_log_enabled:
               rtn = "aloha-log-enabled";
               break;

            case aloha_log_interval:
               rtn = "aloha-log-interval";
               break;

            case aloha_log_max_count:
               rtn = "aloha-log-max-count";
               break;

            case aloha_export_status:
               rtn = "aloha-export-status";
               break;

            case pipeline_window_len:
               rtn = "pipeline-window-len";
               break;
            }
            return rtn;
         } // setting_id_to_str


         setting_id_type str_to_setting_id(char const *s)
         {
            StrAsc name(s);
            struct names_map_type
            {
               char const *name;
               uint4 const id;
            } names_map[] = {
               { "clock-check-schedule", clock_check_sched },
               { "clock-check-sched", clock_check_sched },
               { "clockCheckSched", clock_check_sched },
               { "clock-check-schedule", clock_check_sched },
               { "clockCheckSchedule", clock_check_sched },
               { "clk-chk-sched", clock_check_sched },
               { "clkChkSched", clock_check_sched },
               { "max-time-on-line", max_time_on_line },
               { "maxTimeOnLine", max_time_on_line },
               { "max-packet-size", max_packet_size },
               { "maxPacketSize", max_packet_size },
               { "extra-response-time", extra_response_time },
               { "extraResponseTime", extra_response_time },
               { "collect-schedule", collect_schedule },
               { "collectSchedule", collect_schedule },
               { "security-code", security_code },
               { "securityCode", security_code },
               { "do-hole-collect", do_hole_collect },
               { "doHoleCollect", do_hole_collect },
               { "baud-rate", baud_rate },
               { "baudRate", baud_rate },
               { "switch-id", switch_id },
               { "switchId", switch_id },
               { "logger-program-info", logger_program_info },
               { "loggerProgramInfo", logger_program_info },
               { "low-level-poll-schedule", low_level_poll_schedule },
               { "lowLevelPollSchedule", low_level_poll_schedule },
               { "com-port-id", com_port_id },
               { "comPortId", com_port_id },
               { "bmp1-station-id", bmp1_station_id },
               { "bmp1StationId", bmp1_station_id },
               { "collect-via-advise", collect_via_advise },
               { "collectViaAdvise", collect_via_advise },
               { "tables-to-exclude", tables_to_exclude },
               { "tablesToExclude", tables_to_exclude },
               { "time-zone-difference", time_zone_difference },
               { "timeZoneDifference", time_zone_difference },
               { "table-size-factor", table_size_factor },
               { "tableSizeFactor", table_size_factor },
               { "comm-enabled", comm_enabled },
               { "commEnabled", comm_enabled },
               { "dial-string-list", dial_string_list },
               { "dialStringList", dial_string_list },
               { "phone-modem-type", phone_modem_type },
               { "phoneModemType", phone_modem_type },
               { "settings-overriden", settings_overriden },
               { "settingsOverriden", settings_overriden },
               { "data-collection-enabled", data_collection_enabled },
               { "dataCollectionEnabled", data_collection_enabled },
               { "qtracs-eal-dir", qtracs_eal_dir },
               { "qtracsEalDir", qtracs_eal_dir },
               { "qtracs-poll-interval", qtracs_poll_interval },
               { "qtracsPollInterval", qtracs_poll_interval },
               { "qtracs-mct-no", qtracs_mct_no },
               { "qtracsMctNo", qtracs_mct_no },
               { "qtracs-dial-now-dir", qtracs_dial_now_dir },
               { "qtracsDialNowDir", qtracs_dial_now_dir },
               { "data-broker-id", data_broker_id },
               { "dataBrokerId", data_broker_id },
               { "max-inlocs-per-request", max_inlocs_per_request },
               { "maxInlocsPerRequest", max_inlocs_per_request },
               { "callback-enabled", callback_enabled },
               { "callbackEnabled", callback_enabled },
               { "callback-id", callback_id },
               { "callbackId", callback_id },
               { "input-location-labels", input_location_labels },
               { "inputLocationLabels", input_location_labels },
               { "rf-use-f", rf_use_f },
               { "rfUseF", rf_use_f },
               { "rf-use-u", rf_use_u },
               { "rfUseU", rf_use_u },
               { "rf-use-w", rf_use_w },
               { "rfUseW", rf_use_w },
               { "hole-addition-enabled", hole_addition_enabled },
               { "holeAdditionEnabled", hole_addition_enabled },
               { "udp-address", udp_address },
               { "udpAddress", udp_address },
               { "udp-port", udp_port },
               { "udpPort", udp_port },
               { "udp-first-packet-delay", udp_first_packet_delay },
               { "udpFirstPacketDelay", udp_first_packet_delay },
               { "udp-send-null-attention", udp_send_null_attention },
               { "udpSendNullAttention", udp_send_null_attention },
               { "bmp1-mutex-name", bmp1_mutex_name },
               { "bmp1MutexName", bmp1_mutex_name },
               { "generic-dial-script", generic_dial_script },
               { "genericDialScript", generic_dial_script },
               { "generic-end-script", generic_end_script },
               { "genericEndScript", generic_end_script },
               { "generic-half-duplex", generic_half_duplex },
               { "genericHalfDuplex", generic_half_duplex },
               { "generic-raise-dtr", generic_raise_dtr },
               { "genericRaiseDtr", generic_raise_dtr },
               { "generic-rts-cts-use", generic_rts_cts_use },
               { "genericRtsCtsUse", generic_rts_cts_use },
               { "bmp1-low-level-delay", bmp1_low_level_delay },
               { "bmp1LowLevelDelay", bmp1_low_level_delay },
               { "pakbus-route-broadcast-interval", pakbus_route_broadcast_interval },
               { "pakbusRouteBroadcastInterval", pakbus_route_broadcast_interval },
               { "pakbus-node-identifier", pakbus_node_identifier },
               { "pakbusNodeIdentifier", pakbus_node_identifier },
               { "default-schedule-enabled", default_schedule_enabled },
               { "defaultScheduleEnabled", default_schedule_enabled },
               { "default-cache-data", default_cache_data },
               { "defaultCacheData", default_cache_data },
               { "default-data-file-output-option", default_data_file_output_option },
               { "defaultDataFileOutputOption", default_data_file_output_option },
               { "default-data-file-output-name", default_data_file_output_name },
               { "defaultDataFileOutputName", default_data_file_output_name },
               { "default-datafile-time-stamp-resolution", default_data_file_time_stamp_resolution },
               { "defaultDataFileTimeStampResolution", default_data_file_time_stamp_resolution },
               { "default-data-file-output-format", default_data_file_output_format },
               { "defaultDataFileOutputFormat", default_data_file_output_format },
               { "default-data-file-toa-header-format", default_data_file_toa_header_format },
               { "defaultDataFileToaHeaderFormat", default_data_file_toa_header_format },
               { "use-tapi-dialing-properties", use_tapi_dialing_properties },
               { "useTapiDialingProperties", use_tapi_dialing_properties },
               { "tapi-country-code", tapi_country_code },
               { "tapiCountryCode", tapi_country_code },
               { "tapi-area-code", tapi_area_code },
               { "tapiAreaCode", tapi_area_code },
               { "tapi-dial-string", tapi_dial_string },
               { "tapiDialString", tapi_dial_string },
               { "secondary-collect-schedule-enabled", secondary_collect_schedule_enabled },
               { "secondaryCollectScheduleEnabled", secondary_collect_schedule_enabled },
               { "stay-on-collect-schedule", stay_on_collect_schedule },
               { "stayOnCollectSchedule", stay_on_collect_schedule },
               { "root-delay-before-reopen", root_delay_before_reopen },
               { "rootDelayBeforeReopen", root_delay_before_reopen },
               { "max-baud-rate", max_baud_rate },
               { "maxBaudRate", max_baud_rate },
               { "pakbus-beacon-interval", pakbus_beacon_interval },
               { "pakbusBeaconInterval", pakbus_beacon_interval },
               { "pakbus-is-dialed-link", pakbus_is_dialed_link },
               { "pakbusIsDialedLink", pakbus_is_dialed_link },
               { "rf400-network-id", rf400_network_id },
               { "rf400NetworkId", rf400_network_id },
               { "rf400-radio-id", rf400_radio_id },
               { "rf400RadioId", rf400_radio_id },
               { "rf400-attention-char", rf400_attention_char },
               { "rf400AttentionChar", rf400_attention_char },
               { "rf95-dial-retries", rf95_dial_retries },
               { "rf95DialRetries", rf95_dial_retries },
               { "rf95-custom-dial-string", rf95_custom_dial_string },
               { "rf95CustomDialString", rf95_custom_dial_string },
               { "table-defs-policy", table_defs_policy },
               { "tableDefsPolicy", table_defs_policy },
               { "pakbus-computer-id", pakbus_computer_id },
               { "pakbusComputerId", pakbus_computer_id },
               { "bmp5-callback-enabled", bmp5_callback_enabled },
               { "bmp5CallbackEnabled", bmp5_callback_enabled },
               { "default-table-file-format", default_table_file_format },
               { "defaultTableFileFormat", default_table_file_format },
               { "tcp-callback-port", tcp_callback_port },
               { "tcpCallbackPort", tcp_callback_port },
               { "hangup-delay", hangup_delay },
               { "hangupDelay", hangup_delay },
               { "collect-ports-and-flags", collect_ports_and_flags },
               { "collectPortsAndFlags", collect_ports_and_flags },
               { "delay-comms-after-open", delay_comms_after_open },
               { "delayCommsAfterOpen", delay_comms_after_open },
               { "pakbus-router-name", pakbus_router_name },
               { "pakbusRouterName", pakbus_router_name },
               { "airlink-device-id", airlink_device_id },
               { "airlinkDeviceId", airlink_device_id },
               { "cache-ip-address", cache_ip_address },
               { "cacheIpAddress", cache_ip_address },
               { "current-program-name", current_program_name },
               { "currentProgramName", current_program_name },
               { "user-description", user_description },
               { "userDescription", user_description },
               { "max-cache-table-size", max_cache_table_size },
               { "maxCacheTableSize", max_cache_table_size },
               { "create-cache-tables-only-in-memory", create_cache_tables_only_in_memory },
               { "createCacheTablesOnlyInMemory", create_cache_tables_only_in_memory },
               { "allowed-pakbus-neighbours", allowed_pakbus_neighbours },
               { "allowedPakbusNeighbours", allowed_pakbus_neighbours },
               { "default-custom-csv-format-options", default_custom_csv_format_options },
               { "defaultCustomCsvFormatOptions", default_custom_csv_format_options },
               { "pakbus-leaf-node", pakbus_leaf_node },
               { "pakbusLeafNode", pakbus_leaf_node },
               { "file-synch-control", file_synch_control },
               { "fileSynchControl", file_synch_control },
               { "default-toa5-format-options", default_toa5_format_options },
               { "defaultToa5FormatOptions", default_toa5_format_options },
               { "default-tob1-format-options", default_tob1_format_options },
               { "defaultTob1FormatOptions", default_tob1_format_options },
               { "default-noh-format-options", default_noh_format_options },
               { "defaultNohFormatOptions", default_noh_format_options },
               { "default-csixml-format-options", default_csixml_format_options },
               { "defaultCsixmlFormatOptions", default_csixml_format_options },
               { "low-level-poll-enabled", low_level_poll_enabled },
               { "lowLevelPollEnabled", low_level_poll_enabled },
               { "prevent-tcp-open", prevent_tcp_open },
               { "preventTcpOpen", prevent_tcp_open },
               { "tcp-callback-verify-time", tcp_callback_verify_time },
               { "tcpCallbackVerifyTime", tcp_callback_verify_time },
               { "pakbus-verify-interval", pakbus_verify_interval },
               { "pakbusVerifyInterval", pakbus_verify_interval },
               { "pakbus-tcp-maintained-nodes", pakbus_tcp_maintained_nodes },
               { "pakbusTcpMaintainedNodes", pakbus_tcp_maintained_nodes },
               { "file-synch-mode", file_synch_mode },
               { "fileSynchMode", file_synch_mode },
               { "file-synch-schedule-base", file_synch_schedule_base },
               { "fileSynchScheduleBase", file_synch_schedule_base },
               { "file-synch-schedule-interval", file_synch_schedule_interval },
               { "fileSynchScheduleInterval", file_synch_schedule_interval },
               { "file-synch-control-ex", file_synch_control_ex },
               { "fileSynchControlEx", file_synch_control_ex },
               { "tcp-password", tcp_password },
               { "tcpPassword", tcp_password },
               { "pakbus-tcp-out-addresses", pakbus_tcp_out_addresses },
               { "pakbusTcpOutAddresses", pakbus_tcp_out_addresses },
               { "reschedule-on-data", reschedule_on_data },
               { "rescheduleOnData", reschedule_on_data },
               { "delete-files-after-synch", delete_files_after_synch },
               { "deleteFilesAfterSynch", delete_files_after_synch },
               { "rfTdPollInterval", rftd_poll_interval },
               { "rftd-poll-interval", rftd_poll_interval },
               { "serialUseSimplifiedIo", serial_use_simplified_io },
               { "serial-use-simplified-io", serial_use_simplified_io },
               { "pooledSerialPorts", pooled_serial_ports },
               { "pooled-serial-ports", pooled_serial_ports },
               { "pooledTerminalServers", pooled_terminal_servers },
               { "pooled-terminal-servers", pooled_terminal_servers },
               { "tls-client-enabled", tls_client_enabled },
               { "tlsClientEnabled", tls_client_enabled },
               { "table-file-station-name-selector", table_file_station_name_selector },
               { "tableFileStationNameSelector", table_file_station_name_selector },
               { "socket-pre-open-script", socket_pre_open_script },
               { "socketPreOpenScript", socket_pre_open_script },
               { "socket-post-close-script", socket_post_close_script },
               { "socketPostCloseScript", socket_post_close_script },
               { "pollForStatistics", poll_for_statistics },
               { "poll-for-statistics", poll_for_statistics },
               { "pakbusEncryptionKey", pakbus_encryption_key },
               { "pakbus-encryption-key", pakbus_encryption_key },
               { "alert2StationId", alert2_station_id },
               { "alert2-station-id", alert2_station_id },
               { "alert2-message-log-size", alert2_message_log_size },
               { "alert2MessageLogSize", alert2_message_log_size },
               { "default-schedule-enabled-expr", default_schedule_enabled_expr },
               { "defaultScheduleEnabledExpr", default_schedule_enabled_expr },
               { "ccProxyConnInfo", cc_proxy_conn_info },
               { "cc-proxy-conn-info", cc_proxy_conn_info },
               { "stationMetaJson", station_meta_json },
               { "station-meta-json", station_meta_json },
               { "pakbus-ws-server-url", pakbus_ws_server_url },
               { "pakbusWsServerUrl", pakbus_ws_server_url },
               { "pakbus-ws-network-id", pakbus_ws_network_id },
               { "pakbusWsNetworkId", pakbus_ws_network_id },
               { "alert2-station-ports", alert2_station_ports },
               { "alert2StationPorts", alert2_station_ports },
               { "replication-enabled", replication_enabled },
               { "replicationEnabled", replication_enabled },
               { "aloha-export-station-id", aloha_export_station_id },
               { "alohaExportStationId", aloha_export_station_id },
               { "aloha-export-type", aloha_export_type },
               { "alohaExportType", aloha_export_type },
               { "aloha-export-resource", aloha_export_resource },
               { "alohaExportResource", aloha_export_resource },
               { "gps-update-system-clock", gps_update_system_clock },
               { "gpsUpdateSystemClock", gps_update_system_clock },
               { "alohaLogEnabled", aloha_log_enabled },
               { "aloha-log-enabled", aloha_log_enabled },
               { "alohaLogInterval", aloha_log_interval },
               { "aloha-log-interval", aloha_log_interval },
               { "alohaLogMaxCount", aloha_log_max_count },
               { "aloha-log-max-count", aloha_log_max_count },
               { "alohaExportStatus", aloha_export_status },
               { "aloha-export-status", aloha_export_status },
               { "pipelineWindowLen", pipeline_window_len },
               { "pipeline-window-len", pipeline_window_len },
               { 0, 0 }
            };
            uint4 rtn = 0;
            for(int i = 0; names_map[i].name != 0 && rtn == 0;  ++i)
               if(name == names_map[i].name)
                  rtn = names_map[i].id;
            if(rtn == 0)
            {
               rtn = strtoul(s,0,10);
               if(setting_id_to_str(rtn)[0] == 0)
                  throw Csi::MsgExcept("invalid device setting ID");
            }
            return static_cast<setting_id_type>(rtn);
         } // str_to_setting_id
      };
   };
};


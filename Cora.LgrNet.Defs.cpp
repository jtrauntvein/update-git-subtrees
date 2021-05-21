/* Cora.LgrNet.Defs.cpp

   Copyright (C) 2007, 2021 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 28 June 2007
   Last Change: Monday 11 January 2021
   Last Commit: $Date: 2021-01-11 17:11:24 -0600 (Mon, 11 Jan 2021) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.LgrNet.Defs.h"
#include "StrAsc.h"
#include "Csi.MsgExcept.h"
#include <stdlib.h>


namespace Cora
{
   namespace LgrNet
   {
      namespace
      {
         struct anchor_converter
         {
            StrAsc const name;
            anchor_code_type const id;
         } const anchor_converters[] =
         {
            { "before", anchor_before },
            { "0", anchor_before },
            { "as-child", anchor_as_child },
            { "1", anchor_as_child },
            { "asChild", anchor_as_child },
            { "after", anchor_after },
            { "2", anchor_after },
            { "", anchor_as_child }
         };
      };


      char const *anchor_type_to_string(uint4 id)
      {
         char const *rtn = 0;
         for(int i = 0; rtn == 0 && anchor_converters[i].name.length() != 0; ++i)
         {
            if(anchor_converters[i].id == id)
               rtn = anchor_converters[i].name.c_str();
         }
         return rtn;
      }


      anchor_code_type string_to_anchor_type(char const *id)
      {
         int rtn(-1);
         for(int i = 0; rtn == -1 && anchor_converters[i].name.length() != 0; ++i)
         {
            if(anchor_converters[i].name == id)
               rtn = anchor_converters[i].id;
         }
         if(rtn == -1)
            throw std::invalid_argument("invalid anchor type name");
         return (anchor_code_type)rtn;
      } // string_to_anchor_type

         
      namespace Settings
      {
         char const *setting_id_to_str(uint4 setting_id)
         {
            char const *rtn = "";
            switch(setting_id)
            {
            case network_schedule_enabled:
               rtn = "network-schedule-enabled";
               break;
               
            case transaction_log_settings:
               rtn = "transaction-log-settings";
               break;
               
            case communications_log_settings:
               rtn = "communications-log-settings";
               break;
               
            case object_state_log_settings:
               rtn = "object-state-log-settings";
               break;
               
            case low_level_logs_settings:
               rtn = "low-level-logs-settings";
               break;
               
            case cqr_log_settings:
               rtn = "cqr-log-settings";
               break;
               
            case bmp1_computer_identifier:
               rtn = "bmp1-computer-identifier";
               break;
               
            case network_communications_enabled:
               rtn = "network-communications-enabled";
               break;
               
            case check_security_password:
               rtn = "check-security-password";
               break;
               
            case system_clock_specifier:
               rtn = "system-clock-specifier";
               break;
               
            case bmp3_computer_identifier:
               rtn = "bmp3-computer-identifier";
               break;
               
            case pakbus_computer_identifier:
               rtn = "pakbus-computer-identifier";
               break;
               
            case use_global_pakbus_router:
               rtn = "use-global-pakbus-router";
               break;
               
            case ip_manager_port:
               rtn = "ip-manager-port";
               break;
               
            case ip_manager_key:
               rtn = "ip-manager-key";
               break;

            case auto_backup_enabled:
               rtn = "auto-backup-enabled";
               break;
               
            case auto_backup_base:
               rtn = "auto-backup-base";
               break;
               
            case auto_backup_interval:
               rtn = "auto-backup-interval";
               break;
               
            case auto_backup_include_cache:
               rtn = "auto-backup-include-cache";
               break;
               
            case auto_backup_extra_paths:
               rtn = "auto-backup-extra-paths";
               break;
               
            case auto_backup_path:
               rtn = "auto-backup-path";
               break;
               
            case auto_backup_bale_count:
               rtn = "auto-backup-bale-count";
               break;

            case min_config_rewrite_interval:
               rtn = "min-config-rewrite-interval";
               break;

            case working_dir:
               rtn = "working-dir";
               break;
               
            case application_dir:
               rtn = "application-dir";
               break;
               
            case dir_separator:
               rtn = "dir-separator";
               break;

            case user_notes:
               rtn = "user-notes";
               break;

            case allow_remote_tasks_admin:
               rtn = "allow-remote-task-admin";
               break;
               
            case default_clock_schedule:
               rtn = "default-clock-schedule";
               break;
               
            case default_collect_schedule:
               rtn = "default-collect-schedule";
               break;
               
            case default_secondary_collect_schedule_enabled:
               rtn = "default-secondary-collect-schedule-enabled";
               break;
               
            case default_stay_on_collect_schedule:
               rtn = "default-stay-on-collect-schedule";
               break;
               
            case default_do_hole_collect:
               rtn = "default-do-hole-collect";
               break;
               
            case default_hole_addition_enabled:
               rtn = "default-hole-addition-enabled";
               break;
               
            case default_collect_via_advise:
               rtn = "default-collect-via-advise";
               break;
               
            case default_reschedule_on_data:
               rtn = "default-reschedule-on-data";
               break;
               
            case default_table_defs_policy:
               rtn = "default-table-defs-policy";
               break;
               
            case default_max_cache_table_size:
               rtn = "default-max-cache-table-size";
               break;
               
            case default_table_size_factor:
               rtn = "default-table-size-factor";
               break;
               
            case default_file_synch_mode:
               rtn = "default-file-synch-mode";
               break;
               
            case default_file_synch_schedule_base:
               rtn = "default-file-synch-schedule-base";
               break;
               
            case default_file_synch_schedule_interval:
               rtn = "default-file-synch-schedule-interval";
               break;
               
            case default_file_synch_control_ex:
               rtn = "default-file-synch-control-ex";
               break;
               
            case default_delete_files_after_synch:
               rtn = "default-delete-files-after-synch";
               break;
               
            case default_collect_ports_and_flags:
               rtn = "default-collect-ports-and-flags";
               break;
               
            case default_fs_output_format:
               rtn = "default-fs-output-format";
               break;
               
            case default_fs_collect_mode:
               rtn = "default-fs-collect-mode";
               break;
               
            case default_fs_collect_all_on_first_poll:
               rtn = "default-fs-collect-all-on-first-poll";
               break;
               
            case default_fs_arrays_to_collect_on_first_poll:
               rtn = "default-fs-arrays-to-collect-on-first-poll";
               break;
               
            case default_fs_max_arrays_to_poll:
               rtn = "default-fs-max-arrays-to-poll";
               break;
               
            case default_data_file_output_option:
               rtn = "default-data-file-output-option";
               break;
               
            case default_data_file_output_name:
               rtn = "default-data-file-output-name";
               break;
               
            case default_table_collect_mode:
               rtn = "default-table-collect-mode";
               break;
               
            case default_table_collect_all_on_first_poll:
               rtn = "default-table-collect-all-on-first-poll";
               break;
               
            case default_table_records_to_collect_on_first_poll:
               rtn = "default-table-records-to-collect-on-first-poll";
               break;
               
            case default_table_max_records_to_poll:
               rtn = "default-table-max-records-to-poll";
               break;
               
            case default_table_file_format:
               rtn = "default-table-file-format";
               break;
               
            case default_custom_csv_format_options:
               rtn = "default-custom-csv-format-options";
               break;
               
            case default_toa5_format_options:
               rtn = "default-toa5-format-options";
               break;
               
            case default_tob1_format_options:
               rtn = "default-tob1-format-options";
               break;
               
            case default_noh_format_options:
               rtn = "default-noh-format-options";
               break;

            case default_csixml_format_options:
               rtn = "default-csixml-format-options";
               break;

            case default_table_file_station_name_selector:
               rtn = "default-table-file-station-name-selector";
               break;

            case max_data_file_size:
               rtn = "max-data-file-size";
               break;
               
            case default_poll_for_statistics:
               rtn = "default-poll-for-statistics";
               break;

            case proxy_account:
               rtn = "proxy-account";
               break;

            case proxy_address:
               rtn = "proxy-address";
               break;

            case proxy_password:
               rtn = "proxy-password";
               break;

            case default_table_max_interval_to_poll:
               rtn = "default-table-max-interval-to-poll";
               break;

            case replication_type:
               rtn = "replication-type";
               break;
            }
            return rtn;
         } // setting_id_to_str


         setting_identifier_type str_to_setting_id(char const *s)
         {
            StrAsc name(s);
            struct names_map_type
            {
               char const *name;
                  uint4 const id;
            } names_map[] = {
               { "network-schedule-enabled", network_schedule_enabled },
               { "networkScheduleEnabled", network_schedule_enabled },
               { "transaction-log-settings", transaction_log_settings },
               { "transactionLogSettings", transaction_log_settings },
               { "communications-log-settings", communications_log_settings },
               { "communicationsLogSettings", communications_log_settings },
               { "object-state-log-settings", object_state_log_settings },
               { "objectStateLogSettings", object_state_log_settings },
               { "low-level-logs-settings", low_level_logs_settings },
               { "lowLevelLogsSettings", low_level_logs_settings },
               { "cqr-log-settings", cqr_log_settings },
               { "cqrLogSettings", cqr_log_settings },
               { "bmp1-computer-identifier", bmp1_computer_identifier },
               { "bmp1ComputerIdentifier", bmp1_computer_identifier },
               { "network-communications-enabled", network_communications_enabled },
               { "networkCommunicationsEnabled", network_communications_enabled },
               { "check-security-password", check_security_password },
               { "checkSecurityPassword", check_security_password },
               { "system-clock-specifier", system_clock_specifier },
               { "systemClockSpecifier", system_clock_specifier },
               { "bmp3-computer-identifier", bmp3_computer_identifier },
               { "bmp3ComputerIdentifier", bmp3_computer_identifier },
               { "pakbus-computer-identifier", pakbus_computer_identifier },
               { "pakbusComputerIdentifier", pakbus_computer_identifier },
               { "use-global-pakbus-router", use_global_pakbus_router },
               { "useGlobalPakbusRouter", use_global_pakbus_router },
               { "ip-manager-port", ip_manager_port },
               { "ipManagerPort", ip_manager_port },
               { "ip-manager-key", ip_manager_key },
               { "ipManagerKey", ip_manager_key },
               { "autoBackupEnabled", auto_backup_enabled },
               { "auto-backup-enabled", auto_backup_enabled },
               { "autoBackupBase", auto_backup_base },
               { "auto-backup-base", auto_backup_base },
               { "autoBackupInterval", auto_backup_interval },
               { "auto-backup-interval", auto_backup_interval },
               { "autobackupIncludeCache", auto_backup_include_cache },
               { "auto-backup-include-cache", auto_backup_include_cache },
               { "autoBackupExtraPaths", auto_backup_extra_paths },
               { "auto-backup-extra-paths", auto_backup_extra_paths },
               { "autoBackupPath", auto_backup_path },
               { "auto-backup-path", auto_backup_path },
               { "autoBackupBaleCount", auto_backup_bale_count },
               { "auto-backup-bale-count", auto_backup_bale_count },
               { "minConfigRwriteInterval", min_config_rewrite_interval },
               { "min-config-rewrite-interval", min_config_rewrite_interval },
               { "workingDir", working_dir },
               { "working-dir", working_dir },
               { "applicationDir", application_dir },
               { "application-dir", application_dir },
               { "dirSeparator", dir_separator },
               { "dir-separator", dir_separator },
               { "user-notes", user_notes },
               { "userNotes", user_notes },
               { "allow-remote-tasks-admin", allow_remote_tasks_admin },
               { "allowRemoteTasksAdmin", allow_remote_tasks_admin },
               { "default-clock-schedule", default_clock_schedule},
               { "default-collect-schedule", default_collect_schedule},
               { "default-secondary-collect-schedule-enabled", default_secondary_collect_schedule_enabled},
               { "default-stay-on-collect-schedule", default_stay_on_collect_schedule},
               { "default-do-hole-collect", default_do_hole_collect},
               { "default-hole-addition-enabled", default_hole_addition_enabled},
               { "default-collect-via-advise", default_collect_via_advise},
               { "default-reschedule-on-data", default_reschedule_on_data},
               { "default-table-defs-policy", default_table_defs_policy},
               { "default-max-cache-table-size", default_max_cache_table_size},
               { "default-table-size-factor", default_table_size_factor},
               { "default-file-synch-mode", default_file_synch_mode},
               { "default-file-synch-schedule-base", default_file_synch_schedule_base},
               { "default-file-synch-schedule-interval", default_file_synch_schedule_interval},
               { "default-file-synch-control-ex", default_file_synch_control_ex},
               { "default-delete-files-after-synch", default_delete_files_after_synch},
               { "default-collect-ports-and-flags", default_collect_ports_and_flags},
               { "default-fs-output-format", default_fs_output_format},
               { "default-fs-collect-mode", default_fs_collect_mode},
               { "default-fs-collect-all-on-first-poll", default_fs_collect_all_on_first_poll},
               { "default-fs-arrays-to-collect-on-first-poll", default_fs_arrays_to_collect_on_first_poll},
               { "default-fs-max-arrays-to-poll", default_fs_max_arrays_to_poll},
               { "default-data-file-output-option", default_data_file_output_option},
               { "default-data-file-output-name", default_data_file_output_name},
               { "default-table-collect-mode", default_table_collect_mode},
               { "default-table-collect-all-on-first-poll", default_table_collect_all_on_first_poll},
               { "default-table-records-to-collect-on-first-poll", default_table_records_to_collect_on_first_poll},
               { "default-table-max-records-to-poll", default_table_max_records_to_poll},
               { "default-table-file-format", default_table_file_format},
               { "default-custom-csv-format-options", default_custom_csv_format_options},
               { "default-toa5-format-options", default_toa5_format_options},
               { "default-tob1-format-options", default_tob1_format_options},
               { "default-noh-format-options", default_noh_format_options},
               { "default-csixml-format-options", default_csixml_format_options },
               { "default-table-file-station-name-selector", default_table_file_station_name_selector },
               { "maxDataFileSize", max_data_file_size },
               { "max-data-file-size", max_data_file_size },
               { "defaultPollForStatics", default_poll_for_statistics },
               { "default-poll-for-statistics", default_poll_for_statistics },
               { "proxy-address", proxy_address },
               { "proxyAddress", proxy_address },
               { "proxy-account", proxy_account },
               { "proxyAccount", proxy_account },
               { "proxy-password", proxy_password },
               { "proxyPassword", proxy_password },
               { "defaultTableMaxIntervalToPoll", default_table_max_interval_to_poll },
               { "default-table-max-interval-to-poll", default_table_max_interval_to_poll },
               { "replication-type", replication_type },
               { "replicationType", replication_type },
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
            return static_cast<setting_identifier_type>(rtn);
         } // str_to_setting_id
      };

      
      namespace DeviceTypes
      {
         char const *device_type_to_str(uint4 device_type)
         {
            char const *rtn = "";
            switch(device_type)
            {
            case cr10:
               rtn = "CR10";
               break;
               
            case _21x:
               rtn = "21X";
               break;
               
            case cr7x:
               rtn = "CR7X";
               break;
               
            case cr10x:
               return "CR10X";
               break;
               
            case cr500:
               return "CR500";
               break;
               
            case cr10t:
               return "CR10T";
               break;
               
            case cr9000:
               return "CR9000";
               break;
               
            case cr5000:
               return "CR5000";
               break;
               
            case storage_module:
               return "SM";
               break;
               
            case com_port:
               return "com-port";
               break;
               
            case phone_modem_base:
               return "phone-modem";
               break;
               
            case rf95_base:
               return "RF95";
               break;
               
            case md9_base:
               return "MD9";
               break;
               
            case generic_modem:
               return "generic";
               break;
               
            case rf95t_base:
               return "RF95T";
               break;
               
            case cr510:
               return "CR510";
               break;
               
            case cr510t:
               return "CR510-TD";
               break;
               
            case cr23x:
               return "CR23X";
               break;
               
            case cr23xt:
               return "CR23X-TD";
               break;
               
            case cr10xt:
               return "CR10X-TD";
               break;
               
            case tcp_com_port:
               return "tcp-com-port";
               break;
               
            case phone_modem_remote:
               return "phone-modem-remote";
               break;
               
            case rf95_remote:
               return "RF95-remote";
               break;
               
            case md9_remote:
               return "MD9-remote";
               break;
               
            case rf95t_remote:
               return "RF95T-remote";
               break;
               
            case pakbus_port:
               return "pakbus-port";
               break;
               
            case tapi_port:
               return "tapi-port";
               break;
               
            case tapi_remote:
               return "tapi-remote";
               break;
               
            case cr10xpb:
               return "CR10X-PB";
               break;
               
            case cr510pb:
               return "CR510-PB";
               break;
               
            case cr23xpb:
               return "CR23X-PB";
               break;
               
            case cr2xx:
               return "CR200-series";
               break;
               
            case cr1000:
               return "CR1000";
               break;
               
            case other_pb_router:
               return "other-pb-router";
               break;
               
            case rf400_base:
               return "RF400";
               break;
               
            case rf400_remote:
               return "RF400-remote";
               break;
               
            case rf95t_pb:
               return "RF95T-PB";
               break;
               
            case pakbus_port_hd:
               return "pakbus-port-hd";
               break;
               
            case cr3000:
               return "CR3000";
               break;
               
            case cr9000x:
               return "CR9000X";
               break;
               
            case cr800:
               return "CR800";
               break;
               
            case cr5000_pb:
               return "CR5000-pb";
               break;
               
            case cr9000x_pb:
               return "CR9000x-pb";
               break;

            case pakbus_tcp_server:
               rtn = "pakbus-tcp-server";
               break;

            case crs450:
               rtn = "CRS450";
               break;

            case serial_port_pool:
               rtn = "serial-port-pool";
               break;
               
            case terminal_server_pool:
               rtn = "terminal-server-pool";
               break;

            case cr6:
               rtn = "CR6";
               break;
               
            case view_group:
               rtn = "group";

            case crvw:
               rtn = "CRVW";
               break;

            case crs500:
               rtn = "CRS500";
               break;

            case cr300:
               rtn = "CR300";
               break;

            case cr1000x:
               rtn = "CR1000X";
               break;

            case alert2_base:
               rtn = "ALERT2-base";
               break;

            case alert2_station:
               rtn = "ALERT2-station";
               break;
               
            case alert2_concentration_station:
               rtn = "ALERT2-concentration-station";
               break;

            case granite_9:
               rtn = "GRANITE-9";
               break;

            case granite_10:
               rtn = "GRANITE-10";
               break;

            case granite_6:
               rtn = "GRANITE-6";
               break;

            case cc_proxy_conn:
               rtn = "cc-proxy-conn";
               break;

            case pakbus_ws_port:
               rtn = "pakbus-ws-port";
               break;

            case aloha_receiver:
               rtn = "aloha-receiver";
               break;

            case gps_receiver:
               rtn = "gps-receiver";
               break;
            }
            return rtn;
         } // device_type_to_str


         device_type_code str_to_device_type(char const *device_type_name)
         {
            StrAsc name(device_type_name);
            struct
            {
               char const *name;
               device_type_code const type; 
            } name_map[] = {
               { "CR10", cr10 },
               { "21X", _21x },
               { "CR7X", cr7x },
               { "CR10X", cr10x },
               { "CR500", cr500 },
               { "CR10T", cr10t },
               { "CR9000", cr9000 },
               { "CR5000", cr5000 },
               { "SM", storage_module },
               { "com-port", com_port },
               { "tapi-port", tapi_port },
               { "tapi-remote", tapi_remote },
               { "phone-modem", phone_modem_base },
               { "phone-modem-remote", phone_modem_remote },
               { "rf95", rf95_base },
               { "rf95-remote", rf95_remote },
               { "md9", md9_base },
               { "md9-remote", md9_remote },
               { "generic", generic_modem },
               { "rf95t", rf95t_base },
               { "rf95t-remote", rf95t_remote },
               { "cr510", cr510 },
               { "cr510-td", cr510t },
               { "cr23x", cr23x },
               { "cr23x-td", cr23xt },
               { "cr10x-td", cr10xt },
               { "tcp-com-port", tcp_com_port },
               { "tcp_com_port", tcp_com_port },
               { "ip-com-port", tcp_com_port },
               { "pakbus-port", pakbus_port },
               { "cr10x-pb", cr10xpb },
               { "cr10x-td-pb", cr10xpb },
               { "cr510-pb", cr510pb },
               { "cr510-td-pb", cr510pb },
               { "cr23x-pb", cr23xpb },
               { "cr23x-td-pb", cr23xpb },
               { "storage-module", storage_module },
               { "sm", storage_module },
               { "cr200", cr2xx },
               { "cr205", cr2xx },
               { "cr210", cr2xx },
               { "cr215", cr2xx },
               { "cr200-series", cr2xx },
               { "cr1000", cr1000 },
               { "other-pb-router", other_pb_router },
               { "rf400", rf400_base },
               { "rf400-remote", rf400_remote },
               { "rf95t-pb", rf95t_pb },
               { "pakbus-port-hd", pakbus_port_hd },
               { "cr3000", cr3000 },
               { "cr9000x", cr9000x },
               { "cr800", cr800 },
               { "cr850", cr800 },
               { "cr8xx", cr800 },
               { "cr800", cr800 },
               { "cr800-series", cr800 },
               { "cr800 series", cr800 },
               { "cr5000-pb", cr5000_pb },
               { "cr9000x-pb", cr9000x_pb },
               { "pakbus-tcp-server", pakbus_tcp_server },
               { "pakbusTcpServer", pakbus_tcp_server },
               { "crs450", crs450 },
               { "serial-port-pool", serial_port_pool },
               { "terminal-server-pool", terminal_server_pool },
               { "cr6", cr6 },
               { "group", view_group },
               { "crvw", crvw },
               { "crvwx", crvw },
               { "crs500", crs500 },
               { "cr300", cr300 },
               { "cr1000x", cr1000x },
               { "granite-9", granite_9 },
               { "granite9", granite_9 },
               { "granite-6", granite_6 },
               { "granite6", granite_6 },
               { "granite-10", granite_10 },
               { "granite10", granite_10 },
               { "alert2-base", alert2_base },
               { "alert2Base", alert2_base },
               { "alert2-station", alert2_station },
               { "alert2Station", alert2_station },
               { "alert2-concentration-station", alert2_concentration_station },
               { "alert2ConcentrationStation", alert2_concentration_station },
               { "alert-station", alert2_concentration_station },
               { "cc-proxy-conn", cc_proxy_conn },
               { "ccProxyConn", cc_proxy_conn },
               { "pakbus-ws-port", pakbus_ws_port },
               { "pakbusWsPort", pakbus_ws_port },
               { "aloha-receiver", aloha_receiver },
               { "alohaReceiver", aloha_receiver },
               { "gps-receiver", gps_receiver },
               { "gpsReceiver", gps_receiver },
               { 0, unknown }
            };
            uint4 rtn = 0;
            for(int i = 0; rtn == 0 && name_map[i].type != 0; ++i)
               if(name == name_map[i].name)
                  rtn = name_map[i].type;
            return static_cast<device_type_code>(rtn);
         } // str_to_device_type


         bool is_logger_type(uint4 device_type)
         {
            bool rtn = false;
            switch(device_type)
            {
            case cr10:
            case _21x:
            case cr7x:
            case cr10x:
            case cr500:
            case cr10t:
            case cr9000:
            case cr5000:
            case storage_module:
            case cr510:
            case cr510t:
            case cr23x:
            case cr23xt:
            case cr10xt:
            case cr10xpb:
            case cr510pb:
            case cr23xpb:
            case cr2xx:
            case cr1000:
            case cr3000:
            case cr9000x:
            case cr800:
            case cr5000_pb:
            case cr9000x_pb:
            case crs450:
            case cr6:
            case crvw:
            case crs500:
            case cr300:
            case cr1000x:
            case granite_9:
            case granite_6:
            case granite_10:
            case alert2_station:
            case alert2_concentration_station:
            case aloha_receiver:
            case gps_receiver:
               rtn = true;
               break;
            }
            return rtn;
         } // is_logger_type
      };
   };
};

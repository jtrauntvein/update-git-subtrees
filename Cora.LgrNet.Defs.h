/* Cora.LgrNet.Defs.h

   Copyright (C) 2000, 2021 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 07 September 2000
   Last Change: Monday 11 January 2021
   Last Commit: $Date: 2021-01-11 12:29:02 -0600 (Mon, 11 Jan 2021) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Cora_LgrNet_Defs_h
#define Cora_LgrNet_Defs_h
#include "CsiTypes.h"


namespace Cora
{
   namespace LgrNet
   {
      /**
       * Specifies the codes that describe how devices can be added or moved within the network
       * map.
       */
      enum anchor_code_type
      {
         anchor_before = 0,
         anchor_as_child = 1,
         anchor_after = 2
      };


      /**
       * @return Returns the string that uniquely identifies an anchor type code.
       *
       * @param specifies the anchor type code.
       */
      char const *anchor_type_to_string(uint4 id);


      /**
       * @return Returns the anchor type code associated with the string.
       *
       * @param id Specifies the string to convert.
       *
       * @throw Throws std::invalid_argument if there is no match.
       */
      anchor_code_type string_to_anchor_type(char const *id);
      
      
      /**
       * Contains an enumeration that lists the message identifiers use for the LgrNet interface.
       */
      namespace Messages
      {
         enum message_identifier_type
         {
            get_settings_cmd = 101,
            settings_advise_not = 102,

            get_default_settings_cmd = 963,
            get_default_settings_ack = 964, 
            
            set_settings_cmd = 103,
            set_settings_ack = 123,
            
            network_map_enum_cmd = 104,
            network_map_advise_not = 105,
            
            add_device_cmd = 106,
            add_device_ack = 107,
            
            rename_device_cmd = 110,
            rename_device_ack = 111,
            
            delete_branch_cmd = 112,
            delete_branch_ack = 113,
            
            open_device_session_cmd = 114,
            
            log_advise_start_cmd = 115,
            log_advise_not = 116,
            log_advise_proceed_cmd = 117,
            log_advise_stop_cmd = 118,
            
            move_branch_cmd = 119,
            move_branch_ack = 120,
            
            logon_cmd = 124,
            logon_ack = 125,
            
            open_security_session_cmd = 126,
            
            clone_session_cmd = 127,
            
            modems_enum_cmd = 128,
            modems_enum_ack = 129,
            
            modem_get_cmd = 130,
            modem_get_ack = 131,

            modem_change_cmd = 132,
            modem_change_not = 133,
            modem_change_ack = 134,

            modem_add_cmd = 135,
            modem_add_ack = 136,

            modem_delete_cmd = 137,
            modem_delete_ack = 138,

            add_log_message_cmd = 140,
            add_log_message_ack = 141,

            get_server_clock_cmd = 144,
            get_server_clock_ack = 145,
            
            data_brokers_enum_cmd = 146,
            data_brokers_enum_not = 147,
            data_brokers_enum_stop_cmd = 148,
            
            open_data_broker_session_cmd = 149,
            open_data_broker_session_ack = 150,
            
            open_active_data_broker_session_cmd = 151,
            open_active_data_broker_session_ack = 152,

            describe_device_relations_cmd = 153,
            describe_device_relations_ack = 154,

            enum_tapi_lines_start_cmd = 155,
            enum_tapi_lines_started_not = 156,
            enum_tapi_lines_line_added_not = 157,
            enum_tapi_lines_line_removed_not = 158,
            enum_tapi_lines_stop_cmd = 159,
            enum_tapi_lines_stopped_not = 160,
            
            enum_countries_cmd = 161,
            enum_countries_ack = 162,

            enum_pakbus_routes_start_cmd = 163,
            enum_pakbus_routes_start_ack = 164,
            enum_pakbus_routes_added_not = 165,
            enum_pakbus_routes_lost_not = 166,
            enum_pakbus_routes_stop_cmd = 167,
            enum_pakbus_routes_stopped_not = 168,

            batch_mode_start_cmd = 169,
            batch_mode_start_ack = 170,
            batch_mode_stop_cmd = 171,
            batch_mode_stop_ack = 172,

            get_current_locale_cmd = 173,
            get_current_locale_ack = 174,

            translate_dialing_string_cmd = 175,
            translate_dialing_string_ack = 176,

            enum_pakbus_routers_start_cmd = 177,
            enum_pakbus_routers_start_ack = 178,
            enum_pakbus_routers_not = 179,
            enum_pakbus_routers_stop_cmd = 180,
            enum_pakbus_routers_stopped_not = 181,

            open_pakbus_router_session_cmd = 182,
            open_pakbus_router_session_ack = 183,

            open_security2_session_cmd = 187,
            open_security2_session_ack = 188,

            announce_access_level = 189,

            list_comm_ports_cmd = 190,
            list_comm_ports_ack = 191,

            enum_pakbus_router_names_start_cmd = 192,
            enum_pakbus_router_names_start_ack = 193,
            enum_pakbus_router_names_not = 194,
            enum_pakbus_router_names_stop_cmd = 195,
            enum_pakbus_router_names_stopped_not = 196,

            open_named_pakbus_router_session_cmd = 197,
            open_named_pakbus_router_session_ack = 198,

            list_device_default_settings_cmd = 185,
            list_device_default_settings_ack = 186,

            lock_network_start_cmd = 901,
            lock_network_start_ack = 902,
            lock_network_stop_cmd = 903,
            lock_network_stopped_not = 904,

            open_dev_ses_by_id_cmd = 905,
            open_dev_ses_by_id_ack = 906,

            list_discs_cmd = 907,
            list_discs_ack = 908,
            
            list_directory_files_cmd = 909,
            list_directory_files_ack = 910,
            
            create_directory_cmd = 911,
            create_directory_ack = 912,

            describe_device_relations_ex_cmd = 913,
            describe_device_relations_ex_ack = 914,

            create_backup_file_cmd = 915,
            create_backup_file_ack = 916,

            restore_snapshot_cmd = 917,
            restore_snapshot_ack = 918,
            snapshot_restored_not = 969,

            clear_logs_cmd = 919,
            clear_logs_ack = 920,

            zip_logs_cmd = 921,
            zip_logs_ack = 922,

            retrieve_file_cmd = 923,
            retrieve_file_ack = 924,
            retrieve_file_cont_cmd = 925,
            retrieve_file_frag_ack = 926,

            logon_ex_start_cmd = 927,
            logon_ex_challenge = 928,
            logon_ex_response = 929,
            logon_ex_ack = 930,

            operation_enum_start_cmd = 931,
            operation_enum_start_ack = 932,
            operation_enum_op_added_not = 933,
            operation_enum_op_changed_not = 934,
            operation_enum_op_deleted_not = 935,
            operation_enum_stop_cmd = 937,

            monitor_pooled_resources_start_cmd =  938,
            monitor_pooled_resources_start_ack = 939,
            monitor_pooled_resources_not = 940,
            monitor_pooled_resources_stop_cmd = 941,
            monitor_pooled_resources_stopped_not = 942,

            open_tasks_session_cmd = 943,
            open_tasks_session_ack = 944,

            enum_views_start_cmd = 945,
            enum_views_not = 946,
            enum_views_stop_cmd = 947,
            enum_views_stopped_not = 948,
            
            add_view_cmd = 953,
            add_view_ack = 954,
            
            change_view_cmd = 955,
            change_view_ack = 956,
            
            remove_view_cmd = 957,
            remove_view_ack = 958,
            
            monitor_view_start_cmd = 949,
            monitor_view_not = 950,
            monitor_view_stop_cmd = 951,
            monitor_view_stopped_not = 952,

            enum_view_map_start_cmd = 959,
            enum_view_map_not = 960,
            enum_view_map_stop_cmd = 961,
            enum_view_map_stopped_not = 962,

            udp_discover_start_cmd = 965,
            udp_discover_not = 966,
            udp_discover_stop_cmd = 967,
            udp_discover_stopped_not = 968,

            log_query_start_cmd = 970,
            log_query_not = 971,
            log_query_cont = 972,
            log_query_stopped_not = 973,

            replication_login_cmd = 974,
            replication_login_ack = 975,

            login_access_token_cmd = 976,
            login_access_token_ack = 977,

            get_access_token_cmd = 978,
            get_access_token_ack = 979,
            
            // next message id is 980
         };
      };


      /**
       * Contains an enumeration that lists all of the settings in the LgrNet interface.
       */
      namespace Settings
      {
         enum setting_identifier_type
         {
            network_schedule_enabled = 1,
            transaction_log_settings = 2,
            communications_log_settings = 3,
            object_state_log_settings = 4,
            low_level_logs_settings = 5,
            cqr_log_settings = 15,
            bmp1_computer_identifier = 6,
            network_communications_enabled = 7,
            check_security_password = 8,
            system_clock_specifier = 9,
            bmp3_computer_identifier = 10,
            pakbus_computer_identifier = 11,
            use_global_pakbus_router = 12,
            ip_manager_port = 13,
            ip_manager_key = 14,
            auto_backup_enabled = 16,
            auto_backup_base = 17,
            auto_backup_interval = 18,
            auto_backup_include_cache = 19,
            auto_backup_extra_paths = 20,
            auto_backup_path = 21,
            auto_backup_bale_count = 22,
            min_config_rewrite_interval = 23,
            working_dir = 24,
            application_dir = 25,
            dir_separator = 26,
            user_notes = 27,
            allow_remote_tasks_admin = 28,
            default_clock_schedule = 29,
            default_collect_schedule = 30,
            default_secondary_collect_schedule_enabled = 31,
            default_stay_on_collect_schedule = 32,
            default_do_hole_collect = 33,
            default_hole_addition_enabled = 34,
            default_collect_via_advise = 35,
            default_reschedule_on_data = 36,
            default_table_defs_policy = 37,
            default_max_cache_table_size = 38,
            default_table_size_factor = 39,
            default_file_synch_mode = 40,
            default_file_synch_schedule_base = 41,
            default_file_synch_schedule_interval = 42,
            default_file_synch_control_ex = 43,
            default_delete_files_after_synch = 44,
            default_collect_ports_and_flags = 45,
            default_fs_output_format = 46,
            default_fs_collect_mode = 47,
            default_fs_collect_all_on_first_poll = 48,
            default_fs_arrays_to_collect_on_first_poll = 49,
            default_fs_max_arrays_to_poll = 50,
            default_data_file_output_option = 51,
            default_data_file_output_name = 52,
            default_table_collect_mode = 53,
            default_table_collect_all_on_first_poll = 54,
            default_table_records_to_collect_on_first_poll = 55,
            default_table_max_records_to_poll = 56,
            default_table_max_interval_to_poll = 69,
            default_table_file_format = 57,
            default_custom_csv_format_options = 58,
            default_toa5_format_options = 59,
            default_tob1_format_options = 60,
            default_noh_format_options = 61,
            default_csixml_format_options = 62,
            default_table_file_station_name_selector = 63,
            max_data_file_size = 64,
            default_poll_for_statistics = 65,
            proxy_address = 66,
            proxy_account = 67,
            proxy_password = 68,
            replication_type = 70
         };


         /**
          * @return Returns a string that uniquely identifies the specified setting identifier.
          *
          * @param setting_id Specifies the setting identifier.
          */
         char const *setting_id_to_str(uint4 setting_id);

         /**
          * @return Returns the setting identifier associated wqith the specified string.
          *
          * @param s Specifies the setting identifier string.  This must match one of the
          * identifiers returned by settingid_to_str().  If no match is found, the value will be
          * converted to a number as a setting identifier.
          */
         setting_identifier_type str_to_setting_id(char const *s);
      };


      /**
       * Contains an enumeration of all possible types of devices that can be added to the network
       * map.
       */
      namespace DeviceTypes
      {
         enum device_type_code
         {
            unknown = 0,
            cr10 = 1,
            _21x = 2,
            cr7x = 3,
            cr10x = 4,
            cr500 = 5,
            cr10t = 6,
            cr9000 = 7,
            cr5000 = 8,
            storage_module = 10,
            com_port = 11,
            phone_modem_base = 12,
            rf95_base = 13,
            md9_base = 14,
            generic_modem = 15,
            rf95t_base = 16,
            cr510 = 18,
            cr510t = 19,
            cr23x = 20,
            cr23xt = 21,
            cr10xt = 22,
            tcp_com_port = 23,
            phone_modem_remote = 25,
            rf95_remote = 26,
            md9_remote = 27,
            rf95t_remote = 29,
            pakbus_port = 36,
            tapi_port = 38,
            tapi_remote = 39,
            cr10xpb = 40,
            cr510pb = 41,
            cr23xpb = 42,
            cr2xx = 43,
            cr1000 = 44,
            other_pb_router = 45,
            rf400_base = 46,
            rf400_remote = 47,
            black_box = 48,
            rf95t_pb = 49,
            pakbus_port_hd = 50,
            test_tunnel_id = 51,
            cr3000 = 52,
            cr9000x = 53,
            cr800 = 54,
            cr5000_pb = 55,
            cr9000x_pb = 56,
            pakbus_tcp_server = 59,
            crs450 = 60,
            serial_port_pool = 61,
            terminal_server_pool = 62,
            cr6 = 63,
            crvw = 64,
            crs500 = 65,
            cr300 = 66,
            cr1000x = 67,
            alert2_base = 68,
            alert2_station = 69,
            alert2_concentration_station = 70,
            granite_9 = 71,
            granite_6 = 72,
            granite_10 = 73,
            cc_proxy_conn = 74,
            pakbus_ws_port = 75,
            aloha_receiver = 76,
            gps_receiver = 77,
            
            // the next type should be 78
            max_device_type,
            view_group = 0xFFFFFFFF
         };
         typedef device_type_code device_type;


         /**
          * @return Returns a string that uniquely decribes the specified device type identifier.
          *
          * @param device_type Specifies the device type identifier.
          */
         char const *device_type_to_str(uint4 device_type);


         /**
          * @return Returns a device type identifier associated with the specified device type
          * string.
          *
          * @param device_type_name Specifies the string to map.  If no mapping is found, the
          * function will attempt to convert this string to an integer.
          */
         device_type_code str_to_device_type(char const *device_type_name);

         /**
          * @return  Returns true if the given device type identifier describes a datalogger type
          * device.
          */
         bool is_logger_type(uint4 device_type);
      };
   };
};

#endif

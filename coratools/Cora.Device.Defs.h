/* Cora.Device.Defs.h

   Copyright (C) 2000, 2021 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 05 January 2000
   Last Change: Thursday 18 March 2021
   Last Commit: $Date: 2021-03-18 08:38:22 -0600 (Thu, 18 Mar 2021) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Cora_Device_Defs_h
#define Cora_Device_Defs_h

#include "CsiTypeDefs.h"


namespace Cora
{
   namespace Device
   {
      namespace CollectArea
      {
         /**
          * Lists the types of persistence codes that can be assigned and reported for a collect
          * area.
          */
         namespace Persistence
         {
            enum Type
            {
               logger_feature = 1,
               table_def_feature = 2,
               client_defined = 3,
               auto_delete = 4,
            };


            /**
             * @return Returns a string that describes the specified persistence type.
             *
             * @param type_code Specifies the persistence type to describe.
             */
            char const *persistence_type_to_str(uint4 type_code);
         };


         /**
          * Lists the codes that can be assigned or reported for various types of collect areas.
          */
         namespace Types
         {
            enum Type
            {
               unknown = 0,
               
               classic_ports_flags = 1,
               classic_inlocs = 2,
               classic_final_storage = 3,
               classic_status = 4,

               bmp1_table = 5,

               bmp3_table = 6,

               bmp5_table = 7,

               sm_last_file_mark = 8,
               ntcip_table = 9,
               ntcip_images = 10,
               alert2_sensor_report = 11,
               data_file_import = 12,
               aloha_station_report = 13,
               gps_position_report = 14,
               gps_satellites_report = 15
            };


            /**
             * @return Returns a string that describes the specified collect area type.
             *
             * @param type_code Specifies the collect area type code.
             */
            char const *collect_area_type_to_str(uint4 type_code);
         };

         /**
          * Lists the setting identifiers for collect area settings.
          */
         namespace Settings
         {
            enum setting_id_type
            {
               tables_written = 1,
               schedule_enabled = 2,
               fs_area = 3,
               fs_cache_data = 4,
               fs_output_option = 5,
               fs_output_format = 6,
               fs_output_name = 7,
               fs_collect_mode = 8,
               fs_collect_all_on_first_poll = 9,
               fs_arrays_to_collect_on_first_poll = 10,
               fs_max_arrays_to_poll = 11,
               fs_current_loc = 12,
               table_last_record_no = 13,
               inloc_ids = 14,
               cache_data = 15,
               data_file_output_option = 16,
               data_file_output_name = 17,
               data_file_timestamp_resolution = 18,
               data_file_output_format = 19,
               data_file_toa_header_format = 20,
               expanded_data_file_output_name = 21,
               use_default_data_file_output_name = 22,
               fs_values_to_poll = 23,
               table_collect_mode = 24,
               table_collect_all_on_first_poll = 25,
               table_records_to_collect_on_first_poll = 26,
               table_max_records_to_poll = 27,
               table_file_format = 28,
               logger_table_no = 29,
               custom_csv_format_options = 30,
               toa5_format_options = 31,
               tob1_format_options = 32,
               noh_format_options = 33,
               csixml_format_options = 34,
               last_data_file_output_name = 36,
               table_max_interval_to_poll = 37,
               alert2_sensor_id = 38,
               alert2_convert_expression = 39,
               alert2_value_data_type = 40,
               import_file_watch_dir = 41,
               import_file_watch_pattern = 42,
               aloha_station_name = 43
               
               // the next identifier must be 44
            };


            /**
             * @return Returns a string description for the specified setting identifier.
             *
             * @param setting_id Specifies the setting identifier to describe.
             */
            char const *setting_id_to_str(uint4 setting_id);

            /**
             * @return Returns a collect area setting identifier that matches the specified string.
             *
             * @param setting_name Specifies the name of the setting to match.
             */
            setting_id_type str_to_setting_id(char const *setting_name);
         };

         /**
          * Defines the bit position of the option to specify that record time stamps should be
          * formatted so that midnight is reported as 24:00 on the previous day.
          */
         uint4 const table_file_midnight_is_2400 = 0x00000004;

         /**
          * Defines the bit position of the include record number option in all format options
          * fields except custom csv.
          */
         uint4 const table_file_include_record_no = 0x00000002;

         /**
          * Defines the bit position of the include time stamp flag in all of the format options
          * except custom csv.
          */
         uint4 const table_file_include_time = 0x00000001;

         /**
          * Defines the bit position of the option in custom csv to prevent quoting of string
          * content.
          */
         uint4 const noh_dont_quote_strings = 0x00000004;

         /**
          * Defines the bit position in custom csv formatting options that controls whether midnight
          * will be formatted as 24:00 on the previous day.
          */
         uint4 const noh_midnight_is_2400 = 0x00000008;
      };


      /**
       * Defines codes for all message types in the device interface.
       */
      namespace Messages
      {
         enum Type
         {
            settings_get_cmd = 201,
            settings_get_not = 202,
            
            settings_enum_start_cmd = 304,
            settings_enum_status_not = 1007,
            settings_enum_not = 305,
            settings_enum_override_not = 306,
            settings_enum_override_stop_not = 307,
            settings_enum_stop_cmd = 308,
            settings_enum_stopped_not = 309,

            settings_set_cmd = 203,
            settings_set_ack = 231,

            settings_override_start_cmd = 287,
            settings_override_start_ack = 288,
            settings_override_stop_cmd = 289,

            connection_management_start_cmd = 273,
            connection_management_start_ack = 278,
            connection_management_status_not = 274,
            connection_management_stop_cmd = 275,

            clock_check_cmd = 255,
            clock_check_stop_cmd = 360,
            clock_check_ack = 256,

            clock_set_cmd = 206,
            clock_set_stop_cmd = 361,
            clock_set_stop_ack = 362,
            clock_set_ack = 208,

            program_file_send_cmd = 232,
            program_file_send_ack = 233,
            program_file_send_abort_cmd = 1029,
            program_file_send_status_not = 234,
            program_file_send_extended_status_not = 385,

            program_file_receive_cmd = 235,
            program_file_receive_ack = 236,
            program_file_receive_cont_cmd = 237,

            logger_table_defs_get_cmd = 252,
            logger_table_defs_get_ack = 253,

            manual_poll_cmd = 230,
            manual_poll_stop_cmd = 367,
            manual_poll_ack = 254,
            manual_poll_status_not = 1009,

            logger_query_start_cmd = 310,
            logger_query_status_not = 311,
            logger_query_stop_cmd = 312,
            logger_query_stopped_not = 313,

            manual_query_cmd = 246,
            manual_query_ack = 247,

            variable_set_cmd = 276,
            variable_set_ack = 277,

            low_level_log_advise_start_cmd = 248,
            low_level_log_advise_not = 249,
            low_level_log_advise_cont_cmd = 250,
            low_level_log_advise_stop_cmd = 251,

            hole_advise_start_cmd = 257,
            hole_advise_start_ack = 349,
            hole_advise_not = 258,
            hole_advise_cont_cmd = 259,
            hole_advise_stop_cmd = 260,
            hole_advise_stopped_not = 279,

            terminal_emu_start_cmd = 268,
            terminal_emu_start_ack = 269,
            terminal_emu_send_cmd = 270,
            terminal_emu_send_ack = 284,
            terminal_emu_receive_not = 271,
            terminal_emu_stop_cmd = 272,

            rf_quality_test_cmd = 238,
            rf_quality_test_ack = 239,

            files_enum_cmd = 290,
            files_enum_ack = 291,

            file_send_cmd = 292,
            file_send_ack = 293,
            file_send_status_not = 294,

            file_receive_cmd = 295,
            file_receive_ack = 296,
            file_receive_cont_cmd = 297,

            file_delete_cmd = 298,
            file_delete_ack = 299,

            file_attributes_change_cmd = 300,
            file_attributes_change_ack = 301,

            disc_format_cmd = 302,
            disc_format_ack = 303,

            collect_areas_enum_start_cmd = 314,
            collect_areas_enum_added_not = 315,
            collect_areas_enum_deleted_not = 316,
            collect_areas_enum_stop_cmd = 317,
            collect_areas_enum_stopped_not = 318,

            collect_area_settings_enum_start_cmd = 319,
            collect_area_settings_enum_not = 320,
            collect_area_settings_override_not = 1038,
            collect_area_settings_override_stop_not = 1039,
            collect_area_settings_enum_stop_cmd = 321,
            collect_area_settings_enum_stopped_not = 322,

            collect_area_settings_set_cmd = 324,
            collect_area_settings_set_ack = 325,

            input_locations_area_create_cmd = 326,
            input_locations_area_create_ack = 327,

            final_storage_area_create_cmd = 328,
            final_storage_area_create_ack = 329,

            collect_area_delete_cmd = 330,
            collect_area_delete_ack = 331,

            selective_manual_poll_cmd = 332,
            selective_manual_poll_stop_cmd = 333,
            selective_manual_poll_ack = 334,
            selective_manual_poll_status_not = 1008,

            program_file_associate_start_cmd = 335,
            program_file_associate_start_ack = 336,
            program_file_associate_send_cmd = 337,
            program_file_associate_send_ack = 338,

            toggle_port_or_flag_cmd = 339,
            toggle_port_or_flag_ack = 340,

            tables_enum_start_cmd = 341,
            tables_enum_start_ack = 342,
            tables_enum_stopped_not = 343,
            tables_enum_table_added_not = 344,
            tables_enum_table_deleted_not = 345,
            tables_enum_table_enabled_not = 346,
            tables_enum_table_areas_not = 347,
            tables_enum_stop_cmd = 348,

            holes_delete_cmd = 350,
            holes_delete_ack = 351,

            collect_schedule_reset_cmd = 352,
            collect_schedule_reset_ack = 353,

            table_reset_cmd = 354,
            table_reset_ack = 355,

            manage_comm_resource_start_cmd = 356,
            manage_comm_resource_start_ack = 357,
            manage_comm_resource_stop_cmd = 358,
            manage_comm_resource_stopped_not = 359,

            file_control_cmd = 386,
            file_control_ack = 387,

            collect_area_reset_cmd = 388,
            collect_area_reset_ack = 389,

            table_area_clone_cmd = 390,
            table_area_clone_ack = 391,

            announce_access_level = 392,

            memory_send_cmd = 393,
            memory_send_ack = 394,

            memory_receive_start_cmd = 395,
            memory_receive_not = 396,
            memory_receive_stop_cmd = 397,
            memory_receive_stopped_not = 398,

            identify_cmd = 1001,
            identify_ack = 1002,

            identify_logger_protocol_start_cmd = 1003,
            identify_logger_protocol_status_not = 1004,
            identify_logger_protocol_stop_cmd = 1005,
            identify_logger_protocol_stopped_not = 1006,

            get_variable_cmd = 1010,
            get_variable_ack = 1011,

            find_pakbus_neighbours_start_cmd = 1012,
            find_pakbus_neighbours_start_ack = 1013,
            find_pakbus_neighbours_stop_cmd = 1014,
            find_pakbus_neighbours_stopped_not = 1015,

            data_monitor_start_cmd = 1016,
            data_monitor_start_ack = 1017,
            data_monitor_not = 1018,
            data_monitor_stop_cmd = 1019,
            data_monitor_stopped_not  = 1020,

            table_area_clone_ex_cmd = 1021,
            table_area_clone_ex_ack = 1022,

            create_table_area_cmd = 1023,
            create_table_area_ack = 1024,

            export_area_collect_state_cmd = 1025,
            export_area_collect_state_ack = 1026,

            get_link_wf_messages_cmd = 1027,
            get_link_wf_messages_ack = 1028,

            get_program_stats_cmd = 1030,
            get_program_stats_ack = 1031,

            override_collect_area_settings_cmd = 1034,
            override_collect_area_settings_ack = 1035,
            override_collect_area_settings_stop_cmd = 1036,
            override_collect_area_settings_stopped_not = 1037,

            find_security_code_cmd = 1040,
            find_security_code_ack = 1041,

            dump_classic_logger_stats_cmd = 1042,
            dump_classic_logger_stats_ack = 1043,

            change_classic_logger_stat_cmd = 1044,
            change_classic_logger_stat_ack = 1045,

            synch_files_cmd = 1046,
            synch_files_status_not = 1047,
            synch_files_ack = 1048,

            reset_link_time_remain_cmd = 1049,
            reset_link_time_remain_ack = 1050,

            classic_send_a_cmd = 1051,
            classic_send_a_ack = 1052,

            monitor_pool_start_cmd = 1053,
            monitor_pool_start_ack = 1054,
            monitor_pool_status_not = 1055,
            monitor_pool_decision_not = 1056,
            monitor_pool_stop_cmd = 1057,
            monitor_pool_stopped_not = 1058,

            force_link_offline_cmd = 1059,
            force_link_offline_ack = 1060,

            create_alert2_sensor_report_cmd = 1061,
            create_alert2_sensor_report_ack = 1062,

            send_alert2_data_cmd = 1063,
            send_alert2_data_ack = 1064,

            collect_area_rename_cmd = 1065,
            collect_area_rename_ack = 1066,

            create_import_area_cmd = 1067,
            create_import_area_ack = 1068,

            logger_query_ex_start_cmd = 1071,
            logger_query_ex_start_ack = 1072,
            logger_query_ex_records_not = 1073,
            logger_query_ex_records_ack_cmd = 1074,
            logger_query_ex_stop_cmd = 1075,
            logger_query_ex_stopped_not = 1076,

            get_collect_areas_config_cmd = 1077,
            get_collect_areas_config_ack = 1078,

            set_collect_areas_config_cmd = 1079,
            set_collect_areas_config_ack = 1080,

            get_final_storage_labels_cmd = 1081,
            get_final_storage_labels_ack = 1082,

            create_aloha_station_area_cmd = 1083,
            create_aloha_station_area_ack = 1084,

            monitor_aloha_messages_start_cmd = 1085,
            monitor_aloha_messages_start_ack = 1086,
            monitor_aloha_messages_not = 1087,
            monitor_aloha_messages_ack = 1088,
            monitor_aloha_messages_stop_cmd = 1089,

            query_aloha_messages_start_cmd = 1090,
            query_aloha_messages_start_ack = 1091,
            query_aloha_messages_not = 1092,
            query_aloha_messages_ack = 1093,
            
            ingest_aloha_messages_cmd = 1094,
            ingest_aloha_messages_ack = 1095,
            
            // the next message number should be 1096
         };
      };


      /**
       * Defines enumerations and functions associated with the device interface.
       */
      namespace Settings
      {
         /**
          * Lists all settings that are defined for the device interface.
          */
         enum setting_id_type
         {
            clock_check_sched = 1,
            max_time_on_line = 2,
            max_packet_size = 3,
            extra_response_time = 4,
            collect_schedule = 5,
            security_code = 6,
            do_hole_collect = 10,
            baud_rate = 11,
            switch_id = 12,
            logger_program_info = 13,
            low_level_poll_schedule = 14,
            com_port_id = 15,
            bmp1_station_id = 16,
            collect_via_advise = 18,
            tables_to_exclude = 19,
            time_zone_difference = 20,
            table_size_factor = 21,
            comm_enabled = 22,
            dial_string_list = 24,
            phone_modem_type = 25,
            settings_overriden = 27,
            data_collection_enabled = 28,
            qtracs_eal_dir = 30,
            qtracs_poll_interval = 31,
            qtracs_mct_no = 32,
            qtracs_dial_now_dir = 33,
            data_broker_id = 34,
            max_inlocs_per_request = 35,
            callback_enabled = 36,
            callback_id = 37,
            input_location_labels = 38,
            rf_use_f = 39,
            rf_use_u = 40,
            rf_use_w = 41,
            hole_addition_enabled = 42,
            udp_address = 43,
            udp_port = 44,
            udp_first_packet_delay = 45,
            udp_send_null_attention = 46,
            bmp1_mutex_name = 47,
            generic_dial_script = 48,
            generic_end_script = 49,
            generic_half_duplex = 50,
            generic_raise_dtr = 51,
            generic_rts_cts_use = 52,
            bmp1_low_level_delay = 53,
            pakbus_route_broadcast_interval = 54,
            pakbus_node_identifier = 55,
            default_schedule_enabled = 56,
            default_cache_data = 57,
            default_data_file_output_option = 58,
            default_data_file_output_name = 59,
            default_data_file_time_stamp_resolution = 60,
            default_data_file_output_format = 61,
            default_data_file_toa_header_format = 62,
            use_tapi_dialing_properties = 63,
            tapi_country_code = 64,
            tapi_area_code = 65,
            tapi_dial_string = 66,
            secondary_collect_schedule_enabled = 67,
            stay_on_collect_schedule = 68,
            root_delay_before_reopen = 69,
            max_baud_rate = 70,
            pakbus_beacon_interval = 71,
            pakbus_is_dialed_link = 72,
            rf400_network_id = 73,
            rf400_radio_id = 74,
            rf400_attention_char = 75,
            rf95_dial_retries = 76,
            rf95_custom_dial_string = 77,
            table_defs_policy = 78,
            pakbus_computer_id = 79,
            bmp5_callback_enabled = 80,
            default_table_file_format = 81,
            tcp_callback_port = 82,
            hangup_delay = 83,
            collect_ports_and_flags = 84,
            delay_comms_after_open = 85,
            pakbus_router_name = 86,
            airlink_device_id = 87,
            cache_ip_address = 88,
            current_program_name = 89,
            user_description = 90,
            max_cache_table_size = 91,
            create_cache_tables_only_in_memory = 92,
            allowed_pakbus_neighbours = 93,
            default_custom_csv_format_options = 94,
            pakbus_leaf_node = 95,
            file_synch_control = 96,
            default_toa5_format_options = 97,
            default_tob1_format_options = 98,
            default_noh_format_options = 99,
            default_csixml_format_options = 100,
            low_level_poll_enabled = 101,
            prevent_tcp_open = 102,
            tcp_callback_verify_time = 103,
            pakbus_verify_interval = 109,
            pakbus_tcp_maintained_nodes = 110,
            file_synch_mode = 111,
            file_synch_schedule_base = 112,
            file_synch_schedule_interval = 113,
            file_synch_control_ex = 114,
            pakbus_tcp_out_addresses = 115,
            tcp_password = 116,
            reschedule_on_data = 117,
            delete_files_after_synch = 118,
            rftd_poll_interval = 121,
            serial_use_simplified_io = 122,
            pooled_serial_ports = 123,
            pooled_terminal_servers = 124,
            tls_client_enabled = 125,
            table_file_station_name_selector = 126,
            socket_pre_open_script = 127,
            socket_post_close_script = 128,
            poll_for_statistics = 129,
            pakbus_encryption_key = 130,
            alert2_station_id = 131,
            alert2_message_log_size = 132,
            default_schedule_enabled_expr = 133,
            cc_proxy_conn_info = 134,
            station_meta_json = 135,
            pakbus_ws_server_url = 136,
            pakbus_ws_network_id = 137,
            alert2_station_ports = 138,
            replication_enabled = 139,
            aloha_export_station_id = 140,
            aloha_export_type = 141,
            aloha_export_resource = 142,
            gps_update_system_clock = 143,
            aloha_log_enabled = 144,
            aloha_log_interval = 145,
            aloha_log_max_count = 146,
            aloha_export_status = 147,
            pipeline_window_len = 148
         };


         /**
          * @return Returns a string description for the specified device setting ID.
          *
          * @param setting_id Specifies the setting identifier to describe.
          */
         char const *setting_id_to_str(uint4 setting_id);

         /**
          * @return Returns the device ID associated with the specified string token.
          *
          * @param s Specifies the setting name to match.
          */
         setting_id_type str_to_setting_id(char const *s);
      };
   };
};
         
#endif

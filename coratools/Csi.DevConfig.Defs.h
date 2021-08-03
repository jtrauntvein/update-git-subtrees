/* Csi.DevConfig.Defs.h

   Copyright (C) 2003, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 20 November 2003
   Last Change: Thursday 07 May 2020
   Last Commit: $Date: 2020-05-07 08:27:09 -0600 (Thu, 07 May 2020) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Csi_DevConfig_Defs_h
#define Csi_DevConfig_Defs_h

namespace Csi
{
   namespace DevConfig
   {
      namespace DeviceTypes
      {
         enum device_type_id
         {
            type_md485 = 1,
            type_rf400 = 2,
            type_sc105 = 3,
            type_cr10xpb = 4,
            type_cr510pb = 5,
            type_cr23xpb = 6,
            type_cr200 = 7,
            type_nl100 = 9,
            type_cc640 = 11,
            type_cr1000 = 12,
            type_cd295 = 13,
            type_rf401 = 14,
            type_cr3000 = 15,
            type_tga100a = 16,
            type_cr9032 = 17,
            type_cr800 = 18,
            type_cr5000pb = 19,
            type_rf450 = 20,
            type_ec100 = 21,
            type_avw200 = 22,
            type_rf500m = 23,
            type_ch200 = 24,
            type_pakbus_broker = 25,
            type_com320 = 26,
            type_cr200x = 27,
            type_cs450 = 28,
            type_cs650 = 29,
            type_rf430 = 30,
            type_sc115 = 31,
            type_csat3a = 32,
            type_cwb100 = 33,
            type_cws220 = 34,
            type_cws650 = 35,
            type_cws900 = 36,
            type_fsdaemon = 37,
            type_cs120 = 38,
            type_obs500 = 39,
            type_cs710 = 40,
            type_cs715 = 41,
            type_crs450 = 42,
            type_cc5mpx = 43,
            type_nl200 = 44,
            type_nl300 = 45,
            type_sc_cpi = 46,
            type_al200 = 47,
            type_cdm_vw300 = 48,
            type_cdm_a100 = 49,
            type_tx3xx = 50,
            type_nl240 = 51,
            type_cr6 = 55,
            type_ravenxt = 59,
            type_ls300 = 68,
            type_vsc10 = 61,
            type_rf401a = 62,
            type_crvw = 64,
            type_cr300 = 69,
            type_rf451 = 70,
            type_rf407 = 67,
            type_cr1000x = 78,
            type_nl241 = 80,
            type_granite9 = 76,
            type_granite10 = 94,
            type_granite6 = 92,
            type_vwire305 = 93,
            type_cr350 = 96
         };
      };

      
      namespace Messages
      {
         enum message_id_type
         {
            get_settings_cmd = 0x0f,
            get_settings_ack = 0x8f,
            set_settings_cmd = 0x10,
            set_settings_ack = 0x90,
            get_setting_fragment_cmd = 0x11,
            get_setting_fragment_ack = 0x91,
            set_setting_fragment_cmd = 0x12,
            set_setting_fragment_ack = 0x92,
            control_cmd = 0x13,
            control_ack = 0x93,
            send_file_cmd = 0x14,
            send_file_ack = 0x94,
            send_file_continue_cmd = 0x95,
            discovery_cmd = 0x17,
            discovery_ack = 0x97
         };
      };


      namespace ControlCodes
      {
         enum action_type
         {
            action_commit_changes = 1,
            action_cancel_without_reboot = 2,
            action_revert_to_defaults = 3,
            action_refresh_timer = 4,
            action_cancel_with_reboot = 5,
            action_scan_wifi = 6
         };


         enum control_outcome_type
         {
            outcome_committed = 1,
            outcome_invalid_security_code = 2,
            outcome_session_ended = 3,
            outcome_discarded_with_reboot = 4,
            outcome_reverted_to_defaults = 5,
            outcome_session_timer_reset = 6,
            outcome_locked_by_other = 7,
            outcome_system_error = 8,
            outcome_invalid_action = 9,
            outcome_wifi_scan_started = 10
         };
      };


      namespace Components
      {
         enum component_code_type
         {
            comp_uint1 = 1,
            comp_uint2,
            comp_uint4,
            comp_int1,
            comp_int2,
            comp_int4,
            comp_bool,
            comp_float,
            comp_double,
            comp_enum,
            comp_enumi4,
            comp_enumi2,
            comp_string,
            comp_choice,
            comp_bitfield,
            comp_ipaddr,
            comp_tls_key,
            comp_tls_cert,
            comp_md_float_array,
            comp_ssid,
            comp_ipaddr6,
            comp_serial_no,
            comp_url_address,
            comp_time_stamp,
            comp_ipaddr_any
         };
      };


      namespace Files
      {
         ////////////////////////////////////////////////////////////
         // enum file_send_outcome_type
         ////////////////////////////////////////////////////////////
         enum file_send_outcome_type
         {
            send_outcome_ready_for_next = 1,
            send_outcome_processed_last = 2,
            send_outcome_reset_after_last = 3,
            send_outcome_invalid_offset = 4,
            send_outcome_invalid_name = 5
         };
      };


      namespace Discovery
      {
         ////////////////////////////////////////////////////////////
         // enum param_id_type
         ////////////////////////////////////////////////////////////
         enum param_id_type
         {
            param_mac_address = 1,
            param_station_name = 2,
            param_serial_no = 3,
            param_os_version = 4,
            param_encrypted = 5,
            param_pakbus_address = 6,
            param_pakbus_tcp_port = 7
         };
      };


      namespace SetSettings
      {
         ////////////////////////////////////////////////////////////
         // enum outcome_type
         ////////////////////////////////////////////////////////////
         enum outcome_type
         {
            outcome_success = 1,
            outcome_invalid_security = 2,
            outcome_other_client = 3
         };

         
         ////////////////////////////////////////////////////////////
         // enum setting_outcome_type
         ////////////////////////////////////////////////////////////
         enum setting_outcome_type
         {
            setting_outcome_changed = 1,
            setting_outcome_invalid_setting_id = 2,
            setting_outcome_malformed = 3,
            setting_outcome_read_only = 4,
            setting_outcome_no_memory = 5
         };
      };
   };
};


#endif

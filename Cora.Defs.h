/* Defs.h

   Copyright (C) 1998, 2021 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: 7 February 1996
   Last Change: Monday 11 January 2021
   Last Commit: $Date: 2021-01-11 16:59:50 -0600 (Mon, 11 Jan 2021) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Cora_Defs_h
#define Cora_Defs_h

#include "NetDefs.h"
#include "Cora.Broker.Defs.h"


namespace Cora
{
   ////////// enum LogCategory
   // Enumerates the possible categories for logged events
   enum LogCategory
   {
      LogIdLowLevel = 0,           // low level log
      LogIdTran = 1,               // transaction log
      LogIdComms = 2,              // communications log
      LogIdState = 3,              // object state log
   };
   
   ////////// enum SwfCode
   // An enumeration of codes for logging status warning and fault events
   enum SwfCode
   {
      swf_status = 0,                  // successful operation
      swf_warning = 1,
      swf_fault = 2,
      swf_neutral_status = 3,
   };
   
   
   ////////// enum AnchorCode
   // Enumeration of values that control the placement of devices when they are created
   enum AnchorCode
   {
      Anch_AddBefore = 0,          // add as a peer before the selected device
      Anch_AttachAsChild = 1,      // attach as a child to the selected device
      Anch_AddAfter = 2,           // attach as a peer after the selected device
   };
   
   
   ////////// enum DevTypeCode
   // Enumeration of all device class type codes
   enum DevTypeCode
   {
      LgrNetId = 0,
      DevCr10Id = 1,
      Dev21XId = 2,
      DevCr7XId = 3,
      DevCr10XId = 4,
      DevCr500Id = 5,
      DevCr10TId = 6,
      DevCr9000Id = 7,
      DevCr5000Id = 8,
      DevSmId = 10,
      DevComPortId = 11,
      DevPhModemId = 12,
      DevPhModemPsvId = 25,
      DevRf95Id = 13,
      DevRf95PsvId = 26,
      DevMd9Id = 14,
      DevMd9PsvId = 27,
      DevGenericId = 15,
      DevRf95TId = 16,
      DevRf95TPsvId = 29,
      DevRf95TPbId = 49,
      DevCr510Id = 18,
      DevCr510TId = 19,
      DevCr23XId = 20,
      DevCr23XTId = 21,
      DevCr10XTId = 22,
      DevIpComPortId = 23,
      DevStatisticsId = 24,
      DevPakbusPortId = 36,
      DevPakbusPortHDId = 50,
      DevBmp5LoggerId = 37,
      DevTapiPortId = 38,
      DevTapiRemoteId = 39,
      DevCr10XPbId = 40,
      DevCr510PbId = 41,
      DevCr23XPbId = 42,
      DevCr20xId = 43,
      DevCr1000Id = 44,
      DevOtherPbRouterId = 45,
      DevRf400BaseId = 46,
      DevRf400RemoteId = 47,
      DevTestTunnelId = 51,
      DevCr3000Id = 52,
      DevCr9000xId = 53,
      DevCr800Id = 54,
      DevCr5000PbId = 55,
      DevCr9000xPbId = 56,
      DevPakbusTcpServerId = 59,
      DevCrs450Id = 60,
      DevSerialPortPoolId = 61,
      DevTerminalServerPoolId = 62,
      DevCr6Id = 63,
      DevCrvwId = 64,
      DevCrs500Id = 65,
      DevCr300Id = 66,
      DevCr1000xId = 67,
      DevAlert2Base = 68,
      DevAlert2Station = 69,
      DevAlert2ConcentrationStation = 70,
      DevGranite9Id = 71,
      DevGranite6Id = 72,
      DevGranite10Id = 73,
      DevCcProxyConnId = 74,
      DevPakbusWsPortId = 75,
      DevAlohaReceiverId = 76,
      DevGpsReceiverId = 77,
      // the next device ID should be 78
      
      
      /* base class identifiers */
      DevId = 1001,
      DevLoggerId = 1002,
      DevRouterId = 1004,
      DevBmp1Id = 1005,
      DevPassiveId = 1006,
      DevRootId = 1008,
      DevBmp3Id = 1009,
      DevClassicId = 1010,
      DevPakbusNodeId = 1012,
      DevCrsBaseId = 1013,
      DevPoolBaseId = 1014
   };
   
   ////////// enum LgrNetSettingId
   // List of all possible LgrNet settings identifiers
   enum LgrNetSettingId
   {
      LgrNet_ScheduledOn = 1,
      LgrNet_TranSettings = 2,
      LgrNet_CommsSettings = 3,
      LgrNet_StateSettings = 4,
      LgrNet_LowSettings = 5,
      LgrNet_CqrSettings = 15,
      LgrNet_Bmp1ComputerId = 6,
      LgrNet_CommEnabled = 7,
      LgrNet_CheckPassWd = 8,
      LgrNet_SystemClk = 9,
      LgrNet_PakBusComputerId = 11,
      LgrNet_use_global_pakbus_router = 12,
      LgrNet_ip_manager_port = 13,
      LgrNet_ip_manager_key = 14,
      LgrNet_auto_backup_enabled = 16,
      LgrNet_auto_backup_base = 17,
      LgrNet_auto_backup_interval = 18,
      LgrNet_auto_backup_include_cache = 19,
      LgrNet_auto_backup_extra_paths = 20,
      LgrNet_auto_backup_path = 21,
      LgrNet_auto_backup_bale_count = 22,
      LgrNet_min_config_rewrite_interval = 23,
      LgrNet_working_dir = 24,
      LgrNet_application_dir = 25,
      LgrNet_dir_separator = 26,
      LgrNet_user_notes = 27,
      LgrNet_allow_remote_tasks_admin = 28,
      LgrNet_default_clock_schedule = 29,
      LgrNet_default_collect_schedule = 30,
      LgrNet_default_secondary_collect_schedule_enabled = 31,
      LgrNet_default_stay_on_collect_schedule = 32,
      LgrNet_default_do_hole_collect = 33,
      LgrNet_default_hole_addition_enabled = 34,
      LgrNet_default_collect_via_advise = 35,
      LgrNet_default_reschedule_on_data = 36,
      LgrNet_default_table_defs_policy = 37,
      LgrNet_default_max_cache_table_size = 38,
      LgrNet_default_table_size_factor = 39,
      LgrNet_default_file_synch_mode = 40,
      LgrNet_default_file_synch_schedule_base = 41,
      LgrNet_default_file_synch_schedule_interval = 42,
      LgrNet_default_file_synch_control_ex = 43,
      LgrNet_default_delete_files_after_synch = 44,
      LgrNet_default_collect_ports_and_flags = 45,
      LgrNet_default_fs_output_format = 46,
      LgrNet_default_fs_collect_mode = 47,
      LgrNet_default_fs_collect_all_on_first_poll = 48,
      LgrNet_default_fs_arrays_to_collect_on_first_poll = 49,
      LgrNet_default_fs_max_arrays_to_poll = 50,
      LgrNet_default_data_file_output_option = 51,
      LgrNet_default_data_file_output_name = 52,
      LgrNet_default_table_collect_mode = 53,
      LgrNet_default_table_collect_all_on_first_poll = 54,
      LgrNet_default_table_records_to_collect_on_first_poll = 55,
      LgrNet_default_table_max_records_to_poll = 56,
      LgrNet_default_table_max_interval_to_poll = 69,
      LgrNet_default_table_file_format = 57,
      LgrNet_default_custom_csv_format_options = 58,
      LgrNet_default_toa5_format_options = 59,
      LgrNet_default_tob1_format_options = 60,
      LgrNet_default_noh_format_options = 61,
      LgrNet_default_csixml_format_options = 62,
      LgrNet_default_table_file_station_name_selector = 63,
      LgrNet_max_data_file_size = 64,
      LgrNet_default_poll_for_statistics = 65,
      LgrNet_proxy_address = 66,
      LgrNet_proxy_account = 67,
      LgrNet_proxy_password = 68,
      LgrNet_replication_type = 70
   };
   
   ////////// enum DevSettingId
   // List of all possible device setting identifiers
   enum DevSettingId
   {
      Dev_ClkChkSched = 1,
      Dev_MaxTimeOnline = 2,
      Dev_MaxPktSize = 3,
      Dev_ExtraRespTime = 4,
      Dev_CollectSched = 5,
      Dev_SecurityCode = 6,
      Dev_ClassicCtrl = 8,
      Dev_ErrorRate = 9,
      Dev_DoHoleCollect = 10,
      Dev_BaudRate = 11,
      Dev_SwitchId = 12,
      Dev_LgrProg = 13,
      Dev_LowLevelPoll = 14,
      Dev_ComPortId = 15,
      Dev_Bmp1StatId = 16,
      Dev_Bmp3StatId = 17,
      Dev_CollectViaAdvise = 18,
      Dev_TablesToExclude = 19,
      Dev_TzDiff = 20,
      Dev_TableSizeFactor = 21,
      Dev_CommEnabled = 22,
      Dev_StatUpdateInt = 23,
      Dev_DialStrList = 24,
      Dev_PhoneModemType = 25,
      Dev_SettingsOverridden = 27,
      Dev_DataCollectionEnabled = 28,
      Dev_BmpFrameType = 29,
      Dev_QtracsEalDir = 30,
      Dev_QtracsPollInt = 31,
      Dev_QtracsMctNo = 32,
      Dev_QtracsDialNowDir = 33,
      Dev_DataBrokerId = 34,
   };
   
   ////////// enum LgrNetMessageId
   // List of all message identifiers that are either sent or received by the network controller object 
   enum LgrNetMessageId
   {
      LgrNet_GetSettingsCmd = 101,
      LgrNet_SettingsAdvise = 102,
      LgrNet_get_default_settings_cmd = 963,
      LgrNet_get_default_settings_ack = 964,
      LgrNet_SetSettingsCmd = 103,
      LgrNet_SetSettingsAck = 123,
      LgrNet_EnumNetMapCmd = 104,
      LgrNet_NetMapAdvise = 105,
      LgrNet_AddDevCmd = 106,
      LgrNet_AddDevAck = 107,
      LgrNet_RenDevCmd = 110,
      LgrNet_RenDevAck = 111,
      LgrNet_DelBranchCmd = 112,
      LgrNet_DelBranchAck = 113,
      LgrNet_OpenDevSesCmd = 114,
      LgrNet_LogAdviseStartCmd = 115,
      LgrNet_LogAdvise = 116,
      LgrNet_LogAdviseProceed = 117,
      LgrNet_LogAdviseStopCmd = 118,
      LgrNet_MoveBranchCmd = 119,
      LgrNet_MoveBranchAck = 120,
      LgrNet_CopyBranchCmd = 121,
      LgrNet_CopyBranchAck = 122,
      LgrNet_LogonCmd = 124,
      LgrNet_LogonAck = 125,
      LgrNet_OpenSecSesCmd = 126,
      LgrNet_CloneSesCmd = 127,
      LgrNet_ModemEnumCmd = 128,
      LgrNet_ModemEnumAck = 129,
      LgrNet_ModemGetCmd = 130,
      LgrNet_ModemGetAck = 131,
      LgrNet_ModemChgCmd = 132,
      LgrNet_ModemChgdNot = 133,
      LgrNet_ModemChgAck = 134,
      LgrNet_ModemAddCmd = 135,
      LgrNet_ModemAddAck = 136,
      LgrNet_ModemDelCmd = 137,
      LgrNet_ModemDelAck = 138,
      LgrNet_AddLogMsgCmd = 140,
      LgrNet_AddLogMsgAck = 141,
      LgrNet_OpenDaemonSesCmd = 142,
      LgrNet_OpenDaemonSesAck = 143,
      LgrNet_GetSvrClkCmd = 144,
      LgrNet_GetSvrClkAck = 145,
   
      LgrNet_DataBrokersEnumCmd = 146,
      LgrNet_DataBrokersEnumNot = 147,
      LgrNet_DataBrokersEnumStopCmd = 148,
   
      LgrNet_OpenDataBrokerSesCmd = 149,
      LgrNet_OpenDataBrokerSesAck = 150,
   
      LgrNet_OpenActiveDataBrokerSesCmd = 151,
      LgrNet_OpenActiveDataBrokerSesAck = 152,

      LgrNet_describe_device_relations_cmd = 153,
      LgrNet_describe_device_relations_ack = 154,

      LgrNet_enum_tapi_lines_start_cmd = 155,
      LgrNet_enum_tapi_lines_started_not = 156,
      LgrNet_enum_tapi_lines_line_added_not = 157,
      LgrNet_enum_tapi_lines_line_removed_not = 158,
      LgrNet_enum_tapi_lines_stop_cmd = 159,
      LgrNet_enum_tapi_lines_stopped_not = 160,

      LgrNet_enum_countries_cmd = 161,
      LgrNet_enum_countries_ack = 162,

      LgrNet_enum_pakbus_routes_start_cmd = 163,
      LgrNet_enum_pakbus_routes_start_ack = 164,
      LgrNet_enum_pakbus_routes_added_not = 165,
      LgrNet_enum_pakbus_routes_lost_not = 166,
      LgrNet_enum_pakbus_routes_stop_cmd = 167,
      LgrNet_enum_pakbus_routes_stopped_not = 168,

      LgrNet_batch_mode_start_cmd = 169,
      LgrNet_batch_mode_start_ack = 170,
      LgrNet_batch_mode_stop_cmd = 171,
      LgrNet_batch_mode_stop_ack = 172,

      LgrNet_get_current_locale_cmd = 173,
      LgrNet_get_current_locale_ack = 174,

      LgrNet_translate_dialing_string_cmd = 175,
      LgrNet_translate_dialing_string_ack = 176,

      LgrNet_enum_pakbus_routers_start_cmd = 177,
      LgrNet_enum_pakbus_routers_start_ack = 178,
      LgrNet_enum_pakbus_routers_not = 179,
      LgrNet_enum_pakbus_routers_stop_cmd = 180,
      LgrNet_enum_pakbus_routers_stopped_not = 181,

      LgrNet_open_pakbus_router_session_cmd = 182,
      LgrNet_open_pakbus_router_session_ack = 183,

      LgrNet_open_security2_session_cmd = 187,
      LgrNet_open_security2_session_ack = 188,

      LgrNet_announce_access_level = 189,

      LgrNet_list_comm_ports_cmd = 190,
      LgrNet_list_comm_ports_ack = 191,

      LgrNet_enum_pakbus_router_names_start_cmd = 192,
      LgrNet_enum_pakbus_router_names_start_ack = 193,
      LgrNet_enum_pakbus_router_names_not = 194,
      LgrNet_enum_pakbus_router_names_stop_cmd = 195,
      LgrNet_enum_pakbus_router_names_stopped_not = 196,

      LgrNet_open_named_pakbus_router_session_cmd = 197,
      LgrNet_open_named_pakbus_router_session_ack = 198,

      LgrNet_list_device_default_settings_cmd = 185,
      LgrNet_list_device_default_settings_ack = 186,

      LgrNet_lock_network_start_cmd = 901,
      LgrNet_lock_network_start_ack = 902,
      LgrNet_lock_network_stop_cmd = 903,
      LgrNet_lock_network_stopped_not = 904,

      LgrNet_open_dev_ses_by_id_cmd = 905,
      LgrNet_open_dev_ses_by_id_ack = 906,

      LgrNet_list_discs_cmd = 907,
      LgrNet_list_discs_ack = 908,

      LgrNet_list_directory_files_cmd = 909,
      LgrNet_list_directory_files_ack = 910,

      LgrNet_create_directory_cmd = 911,
      LgrNet_create_directory_ack = 912,

      LgrNet_describe_device_relations_ex_cmd = 913,
      LgrNet_describe_device_relations_ex_ack = 914,

      LgrNet_create_backup_file_cmd = 915,
      LgrNet_create_backup_file_ack = 916,

      LgrNet_restore_snapshot_cmd = 917,
      LgrNet_restore_snapshot_ack = 918,
      LgrNet_snapshot_restored_not = 969,

      LgrNet_clear_logs_cmd = 919,
      LgrNet_clear_logs_ack = 920,

      LgrNet_zip_logs_cmd = 921,
      LgrNet_zip_logs_ack = 922,

      LgrNet_retrieve_file_cmd = 923,
      LgrNet_retrieve_file_ack = 924,
      LgrNet_retrieve_file_cont_cmd = 925,
      LgrNet_retrieve_file_frag_ack = 926,

      LgrNet_logon_ex_start_cmd = 927,
      LgrNet_logon_ex_challenge = 928,
      LgrNet_logon_ex_response = 929,
      LgrNet_logon_ex_ack = 930,

      LgrNet_operation_enum_start_cmd = 931,
      LgrNet_operation_enum_start_ack = 932,
      LgrNet_operation_enum_op_added_not = 933,
      LgrNet_operation_enum_op_changed_not = 934,
      LgrNet_operation_enum_op_deleted_not = 935,
      LgrNet_operation_enum_stop_cmd = 937,

      LgrNet_monitor_pooled_resources_start_cmd = 938,
      LgrNet_monitor_pooled_resources_start_ack = 939,
      LgrNet_monitor_pooled_resources_not = 940,
      LgrNet_monitor_pooled_resources_stop_cmd = 941,
      LgrNet_monitor_pooled_resources_stopped_not = 942,

      LgrNet_open_tasks_session_cmd = 943,
      LgrNet_open_tasks_session_ack = 944,

      LgrNet_enum_views_start_cmd = 945,
      Lgrnet_enum_views_not = 946,
      LgrNet_enum_views_stop_cmd = 947,
      LgrNet_enum_views_stopped_not = 948,

      LgrNet_add_view_cmd = 953,
      LgrNet_add_view_ack = 954,

      LgrNet_change_view_cmd = 955,
      LgrNet_change_view_ack = 956,

      LgrNet_remove_view_cmd = 957,
      LgrNet_remove_view_ack = 958,

      LgrNet_monitor_view_start_cmd = 949,
      LgrNet_monitor_view_not = 950,
      LgrNet_monitor_view_stop_cmd = 951,
      LgrNet_monitor_view_stopped_not = 952,

      LgrNet_enum_view_map_start_cmd = 959,
      LgrNet_enum_view_map_not = 960,
      LgrNet_enum_view_map_stop_cmd = 961,
      LgrNet_enum_view_map_stopped_not = 962,

      LgrNet_udp_discover_start_cmd = 965,
      LgrNet_udp_discover_not = 966,
      LgrNet_udp_discover_stop_cmd = 967,
      LgrNet_udp_discover_stopped_not = 968,

      LgrNet_log_query_start_cmd = 970,
      LgrNet_log_query_not = 971,
      LgrNet_log_query_cont = 972,
      LgrNet_log_query_stopped_not = 973,

      LgrNet_replication_login_cmd = 974,
      LgrNet_replication_login_ack = 975,

      LgrNet_login_access_token_cmd = 976,
      LgrNet_login_access_token_ack = 977,

      LgrNet_get_access_token_cmd = 978,
      LgrNet_get_access_token_ack = 979
      
      // next message is 980
   };
   
   
   ////////// enum DevMessageId
   // List of all message identifiers that can either be sent or recieved in the device interface
   enum DevMessageId
   {
      Dev_GetSettingsCmd = 201,
      Dev_SettingsAdvise = 202,
      Dev_SetSettingsCmd = 203,
      Dev_SetSettingsAck = 231,
      Dev_ClockSetCmd = 206,
      Dev_ClockSetAck = 208,
      Dev_ClockChkCmd = 255,
      Dev_ClockChkAck = 256,
      Dev_DataAdviseStartCmd = 224,
      Dev_DataAdviseNot = 225,
      Dev_DataAdviseContCmd = 226,
      Dev_DataAdviseStopCmd = 227,
      Dev_DataAdviseStopNot = 228,
      Dev_FileAssociateCmd = 229,
      Dev_ManPollCmd = 230,
      Dev_ProgFileSendCmd = 232,
      Dev_ProgFileSendAck = 233,
      Dev_ProgFileSendStatNot = 234,
      Dev_ProgFileRcvCmd = 235,
      Dev_ProgFileRcvAck = 236,
      Dev_ProgFileRcvContCmd = 237,
      Dev_RFTestCmd = 238,
      Dev_RFTestAck = 239,
      Dev_DataQueryStartCmd = 240,
      Dev_DataQueryAck = 241,
      Dev_DataQueryContCmd = 242,
      Dev_DataQueryStopCmd = 245,
      Dev_GetTableIdxCmd = 243,
      Dev_GetTableIdxAck = 244,
      Dev_ManQueryCmd = 246,
      Dev_ManQueryAck = 247,
      Dev_LowLogAdviseStartCmd = 248,
      Dev_LowLogAdviseNot = 249,
      Dev_LowLogAdviseContCmd = 250,
      Dev_LowLogAdviseStopCmd = 251,
      Dev_GetLgrTableDefsCmd = 252,
      Dev_GetLgrTableDefsAck = 253,
      Dev_ManPollAck = 254,
      
      Dev_TableDefsEnumCmd = 261,
      Dev_TableDefsEnumAck = 262,
      Dev_TableDefsChgdNot = 263,
      Dev_TableDefGetCmd = 264,
      Dev_TableDefGetAck = 265,
   
      Dev_ConnMngStartCmd = 273,
      Dev_ConnMngStartAck = 278,
      Dev_ConnMngStatNot = 274,
      Dev_ConnMngStopCmd = 275,
   
      Dev_TermEmuStartCmd = 268,
      Dev_TermEmuStartAck = 269,
      Dev_TermEmuSendCmd = 270,
      Dev_TermEmuSendAck = 284,
      Dev_TermEmuRcvNot = 271,
      Dev_TermEmuStopCmd = 272,
   
      Dev_HoleAdviseStartCmd = 257,
      Dev_HoleAdviseStartAck = 279,
      Dev_HoleAdviseNot = 258,
      Dev_HoleAdviseContCmd = 259,
      Dev_HoleAdviseContAck = 280,
      Dev_HoleAdviseStopCmd = 260,
      Dev_HoleAdviseStopAck = 281,
   
      Dev_ResizeTblCmd = 266,
      Dev_ResizeTblAck = 267,
   
      Dev_SetVarCmd = 276,
      Dev_SetVarAck = 277,
   
      Dev_TogglePortFlagCmd = 285,
      Dev_TogglePortFlagAck = 286,
   
      Dev_OverrideSettingsStartCmd = 287,
      Dev_OverrideSettingsStartAck = 288,
      Dev_OverrideSettingsStopCmd = 289,
   
      Dev_FilesEnumCmd = 290,
      Dev_FilesEnumAck = 291,
   
      Dev_FileSendCmd = 292,
      Dev_FileSendAck = 293,
      Dev_FileSendStatNot = 294,
   
      Dev_FileRcvCmd = 295,
      Dev_FileRcvAck = 296,
      Dev_FileRcvContCmd = 297,
   
      Dev_FileDelCmd = 298,
      Dev_FileDelAck = 299,
   
      Dev_FileChgCmd = 300,
      Dev_FileChgAck = 301,
   
      Dev_DiscFormatCmd = 302,
      Dev_DiscFormatAck = 303,
   };
   
   
   ////////// enum SecMessageId
   // Messages used to administer security
   enum SecMessageId
   {
      Sec_EnumGrpsCmd = 601,
      Sec_EnumGrpsAck = 602,
      Sec_GetGrpCmd = 621,
      Sec_GetGrpAck = 622,
      Sec_AddGrpCmd = 603,
      Sec_AddGrpAck = 604,
      Sec_ChgGrpCmd = 605,
      Sec_ChgGrpAck = 606,
      Sec_ChgGrpNot = 627,
      Sec_DelGrpCmd = 607,
      Sec_DelGrpAck = 608,
      Sec_EnumAcctsCmd = 609,
      Sec_EnumAcctsAck = 610,
      Sec_GetAcctCmd = 611,
      Sec_GetAcctAck = 612,
      Sec_GetOwnAcctCmd = 623,
      Sec_GetOwnAcctAck = 624,
      Sec_AddAcctCmd = 613,
      Sec_AddAcctAck = 614,
      Sec_ChgAcctPassWdCmd = 615,
      Sec_ChgAcctPassWdAck = 616,
      Sec_ChgOwnAcctPassWdCmd = 625,
      Sec_ChgOwnAcctPassWdAck = 626,
      Sec_ChgAcctGrpCmd = 617,
      Sec_ChgAcctGrpAck = 618,
      Sec_AcctChgdNot = 628,
      Sec_DelAcctCmd = 619,
      Sec_DelAcctAck = 620,
      Sec_EnumTransCmd = 629,
      Sec_EnumTransAck = 630
   };
};   
      
#endif

/* Cora.Device.DeviceSettingFactory.cpp

   Copyright (C) 2000, 2021 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 20 July 2000
   Last Change: Friday 05 February 2021
   Last Commit: $Date: 2021-03-18 08:38:22 -0600 (Thu, 18 Mar 2021) $ 
   Committed by: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.DeviceSettingFactory.h"
#include "Cora.CommonSettingTypes.h"
#include "Cora.Device.DeviceSettingTypes.h"
#include "Cora.Device.Defs.h"
#include "Cora.Device.CollectArea.CollectAreaSettingTypes.h"


namespace Cora
{
   namespace Device
   {
      Setting *DeviceSettingFactory::make_setting(uint4 setting_id)
      {
         Setting *rtn;
         switch(setting_id)
         {
         case Cora::Device::Settings::max_time_on_line:
         case Cora::Device::Settings::max_packet_size:
         case Cora::Device::Settings::extra_response_time:
         case Cora::Device::Settings::baud_rate:
         case Cora::Device::Settings::table_size_factor:
         case Cora::Device::Settings::data_broker_id:
         case Cora::Device::Settings::max_inlocs_per_request:
            rtn = new SettingUInt4(setting_id);
            break;

         case Cora::Device::Settings::security_code:
            rtn = new SettingStrAsc(setting_id);
            break;

         case Cora::Device::Settings::com_port_id:
            rtn = new ComPortIdSetting(setting_id);
            break;

         case Cora::Device::Settings::clock_check_sched:
            rtn = new ClockCheckSchedule(setting_id);
            break;

         case Cora::Device::Settings::collect_schedule:
            rtn = new CollectSchedule(setting_id);
            break;

         case Cora::Device::Settings::logger_program_info:
            rtn = new LoggerProgramInfo(setting_id);
            break;
            
         case Cora::Device::Settings::low_level_poll_schedule:
            rtn = new LowLevelPollSchedule(setting_id);
            break;

         case Cora::Device::Settings::dial_string_list:
            rtn = new DialStringList(setting_id);
            break;

         case Cora::Device::Settings::do_hole_collect:
         case Cora::Device::Settings::data_collection_enabled:
         case Cora::Device::Settings::collect_via_advise:
         case Cora::Device::Settings::comm_enabled:
         case Cora::Device::Settings::hole_addition_enabled:
         case Cora::Device::Settings::settings_overriden:
            rtn = new SettingBool(setting_id);
            break;
         
         case Cora::Device::Settings::switch_id:
            rtn = new SettingByte(setting_id);
            break;

         case Cora::Device::Settings::bmp1_station_id:
            rtn = new SettingUInt2(setting_id);
            break;

         case Cora::Device::Settings::time_zone_difference:
            rtn = new SettingInt4(setting_id);
            break;

         case Cora::Device::Settings::phone_modem_type:
            rtn = new SettingStrUni(setting_id);
            break;

         case Cora::Device::Settings::callback_enabled:
            rtn = new SettingBool(setting_id);
            break;

         case Cora::Device::Settings::callback_id:
            rtn = new SettingUInt4(setting_id);
            break;

         case Cora::Device::Settings::input_location_labels:
            rtn = new SettingInlocIds(setting_id);
            break;

         case Cora::Device::Settings::rf_use_f:
         case Cora::Device::Settings::rf_use_u:
         case Cora::Device::Settings::rf_use_w:
            rtn = new SettingBool(setting_id);
            break;

         case Cora::Device::Settings::tables_to_exclude:
            rtn = new TablesToExclude;
            break;

         case Cora::Device::Settings::udp_address:
            rtn = new SettingStrAsc(setting_id);
            break;

         case Cora::Device::Settings::udp_port:
            rtn = new SettingUInt2(setting_id);
            break;

         case Settings::udp_first_packet_delay:
            rtn = new SettingUInt4(setting_id);
            break;

         case Settings::udp_send_null_attention:
            rtn = new SettingBool(setting_id);
            break;

         case Settings::bmp1_mutex_name:
            rtn = new SettingStrUni(setting_id);
            break;

         case Settings::generic_dial_script:
         case Settings::generic_end_script:
            rtn = new SettingStrAsc(setting_id);
            break;
            
         case Settings::generic_half_duplex:
         case Settings::generic_raise_dtr:
            rtn = new SettingBool(setting_id);
            break;
            
         case Settings::generic_rts_cts_use:
            rtn = new SettingGenericRtsCtsUse;
            break;

         case Settings::bmp1_low_level_delay:
            rtn = new SettingUInt4(setting_id);
            break;

         case Settings::pakbus_route_broadcast_interval:
            rtn = new SettingUInt4(setting_id);
            break;

         case Settings::pakbus_node_identifier:
            rtn = new SettingUInt2(setting_id);
            break;

         case Settings::default_cache_data:
            rtn = new SettingBool(setting_id);
            break;

         case Settings::default_schedule_enabled:
            rtn = new SettingBool(setting_id);
            break;
            
         case Settings::default_data_file_output_option:
            rtn = new CollectArea::SettingDataFileOutputOption(setting_id);
            break;

         case Settings::default_data_file_output_name:
            rtn = new SettingStrAsc(setting_id);
            break;

         case Settings::default_data_file_time_stamp_resolution:
            rtn = new CollectArea::SettingDataFileTimestampResolution(setting_id);
            break;

         case Settings::default_data_file_output_format:
            rtn = new CollectArea::SettingDataFileOutputFormat(setting_id);
            break;

         case Settings::default_data_file_toa_header_format:
            rtn = new CollectArea::SettingDataFileToaHeaderFormat(setting_id);
            break;

         case Settings::use_tapi_dialing_properties:
            rtn = new SettingBool(setting_id);
            break;

         case Settings::tapi_country_code:
            rtn = new TapiCountryCode(setting_id);
            break;

         case Settings::tapi_area_code:
            rtn = new SettingStrAsc(setting_id);
            break;

         case Settings::tapi_dial_string:
            rtn = new SettingStrAsc(setting_id);
            break;

         case Settings::secondary_collect_schedule_enabled:
            rtn = new SettingBool(setting_id);
            break;

         case Settings::stay_on_collect_schedule:
            rtn = new SettingBool(setting_id);
            break;

         case Settings::root_delay_before_reopen:
            rtn = new SettingUInt4(setting_id);
            break;

         case Settings::max_baud_rate:
            rtn = new SettingUInt4(setting_id);
            break;

         case Settings::pakbus_beacon_interval:
            rtn = new SettingUInt2(setting_id);
            break;

         case Settings::pakbus_is_dialed_link:
            rtn = new SettingBool(setting_id);
            break;

         case Settings::rf400_network_id:
            rtn = new SettingByte(setting_id);
            break;

         case Settings::rf400_radio_id:
            rtn = new SettingUInt2(setting_id);
            break;

         case Settings::rf400_attention_char:
            rtn = new SettingByte(setting_id);
            break;

         case Settings::rf95_dial_retries:
            rtn = new SettingUInt4(setting_id);
            break;

         case Settings::rf95_custom_dial_string:
            rtn = new SettingStrAsc(setting_id);
            break;

         case Settings::table_defs_policy:
            rtn = new SettingTableDefsPolicy;
            break;

         case Settings::pakbus_computer_id:
            rtn = new SettingUInt2(setting_id);
            break;

         case Settings::bmp5_callback_enabled:
            rtn = new SettingBool(setting_id);
            break;

         case Settings::default_table_file_format:
            rtn = new CollectArea::SettingTableFileFormat(setting_id);
            break;

         case Settings::tcp_callback_port:
            rtn = new SettingUInt2(setting_id);
            break;

         case Settings::hangup_delay:
            rtn = new SettingUInt4(setting_id);
            break;

         case Settings::collect_ports_and_flags:
            rtn = new SettingBool(setting_id);
            break;

         case Settings::delay_comms_after_open:
            rtn = new SettingUInt4(setting_id);
            break;

         case Settings::pakbus_router_name:
            rtn = new SettingStrUni(setting_id);
            break;

         case Settings::airlink_device_id:
            rtn = new SettingStrAsc(setting_id);
            break;

         case Settings::cache_ip_address:
            rtn = new SettingBool(setting_id);
            break;

         case Settings::current_program_name:
            rtn = new SettingStrAsc(setting_id);
            break;

         case Settings::user_description:
            rtn = new SettingStrUni(setting_id);
            break;

         case Settings::max_cache_table_size:
            rtn = new SettingUInt4(setting_id);
            break;

         case Settings::create_cache_tables_only_in_memory:
            rtn = new SettingBool(setting_id);
            break;

         case Settings::allowed_pakbus_neighbours:
            rtn = new AllowedPakbusNeighbours;
            break;

         case Settings::pakbus_leaf_node:
            rtn = new SettingBool(setting_id);
            break;

         case Settings::file_synch_control:
            rtn = new SettingFileSynchControl;
            break;

         case Settings::default_toa5_format_options:
         case Settings::default_tob1_format_options:
         case Settings::default_noh_format_options:
         case Settings::default_custom_csv_format_options:
         case Settings::default_csixml_format_options:
            rtn = new SettingUInt4(setting_id);
            break;

         case Settings::low_level_poll_enabled:
         case Settings::prevent_tcp_open:
            rtn = new SettingBool(setting_id);
            break;

         case Settings::tcp_callback_verify_time:
            rtn = new SettingUInt4(setting_id);
            break;

         case Settings::tcp_password:
         case Settings::pakbus_encryption_key:
            rtn = new SettingAscPassword(setting_id);
            break;
            
         case Settings::socket_pre_open_script:
         case Settings::socket_post_close_script:
            rtn = new SettingStrAsc(setting_id);
            break;

         case Settings::pakbus_verify_interval:
            rtn = new SettingUInt2(setting_id);
            break;

         case Settings::pakbus_tcp_maintained_nodes:
            rtn = new PakbusAddressesList(setting_id);
            break;

         case Settings::file_synch_mode:
            rtn = new FileSynchMode;
            break;
            
         case Settings::file_synch_schedule_base:
            rtn = new TimeSetting(setting_id);
            break;
            
         case Settings::file_synch_schedule_interval:
            rtn = new SettingUInt4(setting_id);
            break;
            
         case Settings::file_synch_control_ex:
            rtn = new FileSynchControlEx;
            break;

         case Settings::pakbus_tcp_out_addresses:
            rtn = new PakbusTcpOutAddresses;
            break;

         case Settings::reschedule_on_data:
         case Settings::delete_files_after_synch:
            rtn = new SettingBool(setting_id);
            break;


         case Settings::rftd_poll_interval:
            rtn = new SettingUInt4(setting_id);
            break;

         case Settings::serial_use_simplified_io:
            rtn = new SettingBool(setting_id);
            break;

         case Settings::pooled_serial_ports:
            rtn = new SettingStrAscList(setting_id);
            break;

         case Settings::pooled_terminal_servers:
            rtn = new PooledTerminalServersSetting;
            break;

         case Settings::tls_client_enabled:
            rtn = new SettingBool(setting_id);
            break;

         case Settings::table_file_station_name_selector:
            rtn = new SettingTableFileStationNameSelector;
            break;

         case Settings::poll_for_statistics:
            rtn = new SettingBool(setting_id);
            break;

         case Settings::alert2_station_id:
            rtn = new SettingUInt2(setting_id);
            break;

         case Settings::alert2_message_log_size:
            rtn = new SettingUInt4(setting_id);
            break;

         case Settings::default_schedule_enabled_expr:
            rtn = new SettingStrAsc(setting_id);
            break;

         case Settings::cc_proxy_conn_info:
         case Settings::station_meta_json:
            rtn = new JsonString(setting_id);
            break;

         case Settings::pakbus_ws_server_url:
            rtn = new PakbusWsServerUrl;
            break;

         case Settings::pakbus_ws_network_id:
            rtn = new SettingStrAsc(setting_id);
            break;

         case Settings::alert2_station_ports:
            rtn = new SettingNameSet(setting_id);
            break;

         case Settings::replication_enabled:
            rtn = new SettingBool(setting_id);
            break;

         case Settings::aloha_export_station_id:
            rtn = new SettingUInt4(setting_id);
            break;

         case Settings::aloha_export_type:
            rtn = new AlohaExportType(setting_id);
            break;

         case Settings::aloha_export_resource:
            rtn = new JsonString(setting_id);
            break;

         case Settings::gps_update_system_clock:
            rtn = new SettingBool(setting_id);
            break;

         case Settings::aloha_log_enabled:
            rtn = new SettingBool(setting_id);
            break;

         case Settings::aloha_log_interval:
            rtn = new SettingInt8(setting_id);
            break;

         case Settings::aloha_log_max_count:
            rtn = new SettingUInt4(setting_id);
            break;

         case Settings::aloha_export_status:
            rtn = new AlohaExportStatusType;
            break;

         case Settings::pipeline_window_len:
            rtn = new SettingUInt2(setting_id);
            break;
            
         default:
            rtn = 0;
            break;
         }
         return rtn;
      } // make_setting
   }
};

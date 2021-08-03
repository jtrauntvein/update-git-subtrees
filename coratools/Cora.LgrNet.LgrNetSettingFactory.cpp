/* Cora.LgrNet.LgrNetSettingFactory.cpp

   Copyright (C) 2000, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 07 September 2000
   Last Change: Tuesday 10 December 2019
   Last Commit: $Date: 2020-10-31 10:08:12 -0600 (Sat, 31 Oct 2020) $ 
   Committed by: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.LgrNet.LgrNetSettingFactory.h"
#include "Cora.LgrNet.LgrNetSettingTypes.h"
#include "Cora.CommonSettingTypes.h"
#include "Cora.LgrNet.Defs.h"
#include "Cora.Device.DeviceSettingTypes.h"
#include "Cora.Device.CollectArea.CollectAreaSettingTypes.h"


namespace Cora
{
   namespace LgrNet
   {
      ////////////////////////////////////////////////////////////
      // class LgrNetSettingFactory definitions
      ////////////////////////////////////////////////////////////
      
      Setting *LgrNetSettingFactory::make_setting(uint4 setting_id)
      {
         Setting *rtn = 0;
         switch(setting_id)
         {
         case Settings::network_schedule_enabled:
         case Settings::network_communications_enabled:
         case Settings::check_security_password:
         case Settings::allow_remote_tasks_admin:
            rtn = new SettingBool(setting_id);
            break;
            
         case Settings::transaction_log_settings:
         case Settings::communications_log_settings:
         case Settings::object_state_log_settings:
         case Settings::low_level_logs_settings:
         case Settings::cqr_log_settings:
            rtn = new LogControl(setting_id);
            break;

         case Settings::bmp1_computer_identifier:
         case Settings::pakbus_computer_identifier:
            rtn = new SettingUInt2(setting_id);
            break;
            
         case Settings::system_clock_specifier:
            rtn = new SettingSystemClock();
            break;
            
         case Settings::bmp3_computer_identifier:
            rtn = new SettingUInt4(setting_id);
            break;

         case Settings::use_global_pakbus_router:
            rtn = new SettingBool(setting_id);
            break;

         case Settings::ip_manager_port:
            rtn = new SettingUInt2(setting_id);
            break;

         case Settings::ip_manager_key:
            rtn = new SettingIpManagerKey;
            break;

         case Settings::auto_backup_enabled:
            rtn = new SettingBool(setting_id);
            break;
            
         case Settings::auto_backup_base:
            rtn = new TimeSetting(setting_id);
            break;
            
         case Settings::auto_backup_interval:
            rtn = new SettingUInt4(setting_id);
            break;
            
         case Settings::auto_backup_include_cache:
            rtn = new SettingBool(setting_id);
            break;
            
         case Settings::auto_backup_extra_paths:
            rtn = new SettingStrAscList(setting_id);
            break;
            
         case Settings::auto_backup_path:
            rtn = new SettingStrAsc(setting_id);
            break;
            
         case Settings::auto_backup_bale_count:
            rtn = new SettingUInt4(setting_id);
            break;

         case Settings::min_config_rewrite_interval:
            rtn = new SettingUInt4(setting_id);
            break;

         case Settings::working_dir: 
         case Settings::application_dir:
            rtn = new SettingStrAsc(setting_id);
            break;
            
         case Settings::dir_separator:
            rtn = new SettingByte(setting_id);
            break;

         case Settings::user_notes:
            rtn = new SettingStrAsc(setting_id);
            break;
            
         case Settings::default_clock_schedule:
            rtn = new Device::ClockCheckSchedule(setting_id);
            break;
            
         case Settings::default_collect_schedule:
            rtn = new Device::CollectSchedule(setting_id);
            break;
            
         case Settings::default_secondary_collect_schedule_enabled:
            rtn = new SettingBool(setting_id);
            break;
            
         case Settings::default_stay_on_collect_schedule:
            rtn = new SettingBool(setting_id);
            break;
            
         case Settings::default_do_hole_collect:
            rtn = new SettingBool(setting_id);
            break;
            
         case Settings::default_hole_addition_enabled:
            rtn = new SettingBool(setting_id);
            break;
            
         case Settings::default_collect_via_advise:
            rtn = new SettingBool(setting_id);
            break;
            
         case Settings::default_reschedule_on_data:
            rtn = new SettingBool(setting_id);
            break;
            
         case Settings::default_table_defs_policy:
            rtn = new Device::SettingTableDefsPolicy(setting_id);
            break;
            
         case Settings::default_max_cache_table_size:
            rtn = new SettingUInt4(setting_id);
            break;
            
         case Settings::default_table_size_factor:
            rtn = new SettingUInt4(setting_id);
            break;
            
         case Settings::default_file_synch_mode:
            rtn = new Device::FileSynchMode(setting_id);
            break;
            
         case Settings::default_file_synch_schedule_base:
            rtn = new TimeSetting(setting_id);
            break;
            
         case Settings::default_file_synch_schedule_interval:
            rtn = new SettingUInt4(setting_id);
            break;
            
         case Settings::default_file_synch_control_ex:
            rtn = new Device::FileSynchControlEx(setting_id);
            break;
            
         case Settings::default_delete_files_after_synch:
            rtn = new SettingBool(setting_id);
            break;
            
         case Settings::default_collect_ports_and_flags:
            rtn = new SettingBool(setting_id);
            break;
            
         case Settings::default_fs_output_format:
            rtn = new Device::CollectArea::SettingFsOutputFormat(setting_id);
            break;
            
         case Settings::default_fs_collect_mode:
            rtn = new Device::CollectArea::SettingFsCollectMode(setting_id);
            break;
            
         case Settings::default_fs_collect_all_on_first_poll:
            rtn = new SettingBool(setting_id);
            break;
            
         case Settings::default_fs_arrays_to_collect_on_first_poll:
            rtn = new SettingUInt4(setting_id);
            break;
            
         case Settings::default_fs_max_arrays_to_poll:
            rtn = new SettingUInt4(setting_id);
            break;
            
         case Settings::default_data_file_output_option:
            rtn = new Device::CollectArea::SettingDataFileOutputOption(setting_id);
            break;
            
         case Settings::default_data_file_output_name:
            rtn = new SettingStrAsc(setting_id);
            break;
            
         case Settings::default_table_collect_mode:
            rtn = new Device::CollectArea::SettingTableCollectMode(setting_id);
            break;
            
         case Settings::default_table_collect_all_on_first_poll:
            rtn = new SettingBool(setting_id);
            break;
            
         case Settings::default_table_records_to_collect_on_first_poll:
            rtn = new SettingUInt4(setting_id);
            break;
            
         case Settings::default_table_max_records_to_poll:
            rtn = new SettingUInt4(setting_id);
            break;
            
         case Settings::default_table_file_format:
            rtn = new Device::CollectArea::SettingTableFileFormat(setting_id);
            break;
            
         case Settings::default_custom_csv_format_options:
            rtn = new SettingUInt4(setting_id);
            break;
            
         case Settings::default_toa5_format_options:
            rtn = new SettingUInt4(setting_id);
            break;
            
         case Settings::default_tob1_format_options:
            rtn = new SettingUInt4(setting_id);
            break;
            
         case Settings::default_noh_format_options:
            rtn = new SettingUInt4(setting_id);
            break;

         case Settings::default_csixml_format_options:
            rtn = new SettingUInt4(setting_id);
            break;

         case Settings::default_table_file_station_name_selector:
            rtn = new Device::SettingTableFileStationNameSelector(setting_id);
            break;

         case Settings::max_data_file_size:
            rtn = new SettingInt8(setting_id);
            break;

         case Settings::default_poll_for_statistics:
            rtn = new SettingBool(setting_id);
            break;

         case Settings::proxy_address:
         case Settings::proxy_account:
            rtn = new SettingStrAsc(setting_id);
            break;

         case Settings::proxy_password:
            rtn = new SettingStrUni(setting_id);
            break;

         case Settings::default_table_max_interval_to_poll:
            rtn = new SettingUInt4(setting_id);
            break;

         case Settings::replication_type:
            rtn = new SettingReplicationType();
            break;
         }
         return rtn;
      } // make_setting
   };
};

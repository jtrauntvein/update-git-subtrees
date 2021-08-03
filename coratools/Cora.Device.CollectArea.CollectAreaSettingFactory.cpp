/* Cora.Device.CollectArea.CollectAreaSettingFactory.cpp

   Copyright (C) 2000, 2021 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 31 October 2000
   Last Change: Wednesday 13 January 2021
   Last Commit: $Date: 2021-01-13 12:59:28 -0600 (Wed, 13 Jan 2021) $ 
   Committed by: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.CollectArea.CollectAreaSettingFactory.h"
#include "Cora.Device.CollectArea.CollectAreaSettingTypes.h"
#include "Cora.Device.DeviceSettingTypes.h"
#include "Cora.Device.Defs.h"


namespace Cora
{
   namespace Device
   {
      namespace CollectArea
      {
         ////////////////////////////////////////////////////////////
         // class CollectAreaSettingFactory definitions
         ////////////////////////////////////////////////////////////
         Setting *CollectAreaSettingFactory::make_setting(uint4 setting_id)
         {
            Setting *rtn = 0;
            switch(setting_id)
            {
            case Settings::tables_written:
               rtn = new SettingNameSet(setting_id);
               break;

            case Settings::schedule_enabled:
               rtn = new SettingBool(setting_id);
               break;

            case Settings::fs_area:
               rtn = new SettingUInt4(setting_id);
               break;

            case Settings::fs_output_format:
               rtn = new SettingFsOutputFormat(setting_id); 
               break;

            case Settings::fs_collect_mode:
               rtn = new SettingFsCollectMode(setting_id);
               break;

            case Settings::fs_collect_all_on_first_poll:
               rtn = new SettingBool(setting_id);
               break;

            case Settings::fs_arrays_to_collect_on_first_poll:
               rtn = new SettingUInt4(setting_id);
               break;

            case Settings::fs_max_arrays_to_poll:
               rtn = new SettingUInt4(setting_id);
               break;

            case Settings::fs_current_loc:
               rtn = new SettingUInt4(setting_id);
               break;

            case Settings::table_last_record_no:
               rtn = new SettingUInt4(setting_id);
               break;

            case Settings::inloc_ids:
               rtn = new SettingInlocIds(setting_id);
               break;
               
            case Settings::cache_data:
               rtn = new SettingBool(setting_id);
               break;

            case Settings::data_file_output_option:
               rtn = new SettingDataFileOutputOption(setting_id);
               break;

            case Settings::data_file_output_name:
               rtn = new SettingStrAsc(setting_id);
               break;

            case Settings::data_file_timestamp_resolution:
               rtn = new SettingDataFileTimestampResolution(setting_id);
               break;

            case Settings::data_file_output_format:
               rtn = new SettingDataFileOutputFormat(setting_id);
               break;

            case Settings::data_file_toa_header_format:
               rtn = new SettingDataFileToaHeaderFormat(setting_id);
               break;

            case Settings::expanded_data_file_output_name:
               rtn = new SettingStrAsc(setting_id);
               break;

            case Settings::use_default_data_file_output_name:
               rtn = new SettingBool(setting_id);
               break;

            case Settings::fs_values_to_poll:
               rtn = new SettingUInt4(setting_id);
               break;

            case Settings::table_collect_mode:
               rtn = new SettingTableCollectMode;
               break;
               
            case Settings::table_collect_all_on_first_poll:
               rtn = new SettingBool(setting_id);
               break;
               
            case Settings::table_records_to_collect_on_first_poll:
            case Settings::table_max_records_to_poll:
               rtn = new SettingUInt4(setting_id);
               break;

            case Settings::table_file_format:
               rtn = new SettingTableFileFormat(setting_id);
               break;

            case Settings::logger_table_no:
               rtn = new SettingUInt4(setting_id);
               break;

            case Settings::custom_csv_format_options:
            case Settings::toa5_format_options:
            case Settings::tob1_format_options:
            case Settings::noh_format_options:
            case Settings::csixml_format_options:
               rtn = new SettingUInt4(setting_id);
               break;

            case Settings::last_data_file_output_name:
               rtn = new SettingStrAsc(setting_id);
               break;

            case Settings::table_max_interval_to_poll:
               rtn = new SettingUInt4(setting_id, 1000);
               break;

            case Settings::alert2_sensor_id:
               rtn = new SettingUInt2(setting_id);
               break;

            case Settings::alert2_convert_expression:
               rtn = new SettingStrAsc(setting_id);
               break;

            case Settings::alert2_value_data_type:
               rtn = new SettingAlert2ValueDataType(setting_id);
               break;

            case Settings::import_file_watch_dir:
            case Settings::import_file_watch_pattern:
               rtn = new SettingStrAsc(setting_id);
               break;

            case Settings::aloha_station_name:
               rtn = new SettingStrUni(setting_id);
               break;
            }
            return rtn;
         }
      };
   };
};

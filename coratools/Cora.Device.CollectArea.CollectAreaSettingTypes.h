/* Cora.Device.CollectArea.CollectAreaSettingTypes.h

   Copyright (C) 2000, 2017 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 31 October 2000
   Last Change: Wednesday 18 October 2017
   Last Commit: $Date: 2017-10-18 09:11:43 -0600 (Wed, 18 Oct 2017) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Cora_Device_CollectArea_CollectAreaSettingTypes_h
#define Cora_Device_CollectArea_CollectAreaSettingTypes_h


#include "Cora.CommonSettingTypes.h"
#include "Cora.Device.Defs.h"


namespace Cora
{
   namespace Device
   {
      namespace CollectArea
      {
         ////////////////////////////////////////////////////////////
         // class SettingFsOutputFormat
         ////////////////////////////////////////////////////////////
         class SettingScheduleEnabled: public SettingBool
         {
         public:
            SettingScheduleEnabled(uint4 setting_id):
               SettingBool(setting_id)
            { }
         };


         ////////////////////////////////////////////////////////////
         // class SettingFsOutputFormat
         ////////////////////////////////////////////////////////////
         class SettingFsOutputFormat: public SettingEnumeration
         {
         public:
            SettingFsOutputFormat(uint4 setting_id):
               SettingEnumeration(setting_id)
            {
               supported_values[1] = "binary";
               supported_values[2] = "comma-delimited-ascii";
               supported_values[3] = "printable-ascii";
            }
         };


         ////////////////////////////////////////////////////////////
         // class SettingFsCollectMode
         ////////////////////////////////////////////////////////////
         class SettingFsCollectMode: public SettingEnumeration
         {
         public:
            SettingFsCollectMode(uint4 setting_id):
               SettingEnumeration(setting_id)
            {
               supported_values[1] = "logged-since-last";
               supported_values[2] = "most-recently-logged";
               supported_values[3] = "all-values";
            }
         };


         ////////////////////////////////////////////////////////////
         // class SettingDataFileOutputOption
         ////////////////////////////////////////////////////////////
         class SettingDataFileOutputOption: public SettingEnumeration
         {
         public:
            SettingDataFileOutputOption(uint4 setting_id):
               SettingEnumeration(setting_id)
            {
               supported_values[1] = "do-not-create";
               supported_values[2] = "append";
               supported_values[3] = "overwrite";
               supported_values[4] = "unique-name";
            }
         };


         ////////////////////////////////////////////////////////////
         // class SettingDataFileTimestampResolution
         ////////////////////////////////////////////////////////////
         class SettingDataFileTimestampResolution: public SettingEnumeration
         {
         public:
            SettingDataFileTimestampResolution(uint4 setting_id):
               SettingEnumeration(setting_id)
            {
               supported_values[0] = "none";
               supported_values[1] = "1-10";
               supported_values[2] = "1-100";
               supported_values[3] = "1-1000";
               supported_values[4] = "1-10000";
               supported_values[5] = "1-100000";
               supported_values[6] = "1-1000000";
               supported_values[7] = "1-10000000";
               supported_values[9] = "1-100000000";
               supported_values[10] = "available";
            }
         };


         ////////////////////////////////////////////////////////////
         // class SettingDataFileOutputFormat
         ////////////////////////////////////////////////////////////
         class SettingDataFileOutputFormat: public SettingEnumeration
         {
         public:
            SettingDataFileOutputFormat(uint4 setting_id):
               SettingEnumeration(setting_id)
            {
               supported_values[1] = "table-ascii";
               supported_values[2] = "ldxp";
               supported_values[3] = "tob1";
            }
         };


         ////////////////////////////////////////////////////////////
         // class SettingDataFileToaHeaderFormat
         ////////////////////////////////////////////////////////////
         class SettingDataFileToaHeaderFormat: public SettingEnumeration
         {
         public:
            SettingDataFileToaHeaderFormat(uint4 setting_id):
               SettingEnumeration(setting_id)
            {
               supported_values[1] = "no-header";
               supported_values[2] = "toa1";
               supported_values[3] = "toa5";
               supported_values[4] = "tob1";
            }
         };


         ////////////////////////////////////////////////////////////
         // class SettingTableCollectMode
         ////////////////////////////////////////////////////////////
         class SettingTableCollectMode: public SettingEnumeration
         {
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            SettingTableCollectMode(uint4 setting_id = Settings::table_collect_mode):
               SettingEnumeration(setting_id)
            {
               supported_values[1] = "logged-since-last";
               supported_values[2] = "most-recently-logged";
               supported_values[3] = "at-most";
               supported_values[4] = "at-most-interval";
            }
         };


         ////////////////////////////////////////////////////////////
         // class SettingTableFileFormat
         ////////////////////////////////////////////////////////////
         class SettingTableFileFormat: public SettingEnumeration
         {
         public:
            SettingTableFileFormat(uint4 setting_id = Settings::table_file_format):
               SettingEnumeration(setting_id)
            {
               supported_values[1] = "ascii-without-header";
               supported_values[2] = "TOACI1";
               supported_values[3] = "TOA5";
               supported_values[4] = "TOB1";
               supported_values[5] = "LDxP";
               supported_values[6] = "custom-csv";
               supported_values[7] = "csixml";
            }
         };


         /**
          * Defines the collect area setting that controls the data type for values stored in an
          * ALERT2 cache table.
          */
         class SettingAlert2ValueDataType: public SettingEnumeration
         {
         public:
            SettingAlert2ValueDataType(uint4 setting_id = Settings::alert2_value_data_type):
               SettingEnumeration(setting_id)
            {
               supported_values[1] = "ieee4";
               supported_values[2] = "ieee8";
               supported_values[3] = "int";
            }
         };
      };
   };
};

#endif

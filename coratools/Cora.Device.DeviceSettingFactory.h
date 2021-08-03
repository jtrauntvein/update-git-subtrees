/* Cora.Device.DeviceSettingFactory.h

   Copyright (C) 2000, Campbell Scientific, Inc

   Written by: Jon Trauntvein
   Date Begun: Thursday 20 July 2000
   Last Change: Thursday 20 July 2000
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)

*/

#ifndef Cora_Device_DeviceSettingFactory_h
#define Cora_Device_DeviceSettingFactory_h

#include "Cora.SettingFactory.h"

namespace Cora
{
   namespace Device
   {
      ////////// class DeviceSettingFactory
      // Defines a setting factory that recognises all device settings
      class DeviceSettingFactory: public SettingFactory
      {
      public:
         virtual Setting *make_setting(uint4 setting_id);
      };
   };
};

#endif

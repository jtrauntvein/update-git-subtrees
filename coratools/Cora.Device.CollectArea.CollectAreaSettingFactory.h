/* Cora.Device.CollectArea.CollectAreaSettingFactory.h

   Copyright (C) 2000, Campbell Scientific, Inc

   Written by: Jon Trauntvein
   Date Begun: Tuesday 31 October 2000
   Last Change: Tuesday 31 October 2000
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)

*/

#ifndef Cora_Device_CollectArea_CollectAreaSettingFactory_h
#define Cora_Device_CollectArea_CollectAreaSettingFactory_h

#include "Cora.SettingFactory.h"

namespace Cora
{
   namespace Device
   {
      namespace CollectArea
      {
         ////////////////////////////////////////////////////////////
         // class CollectAreaSettingFactory
         ////////////////////////////////////////////////////////////
         class CollectAreaSettingFactory: public SettingFactory
         {
         public:
            virtual Setting *make_setting(uint4 setting_id);
         };
      };
   };
};

#endif

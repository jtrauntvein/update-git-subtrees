/* Cora.LgrNet.LgrNetSettingFactory.h

   Copyright (C) 2000, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 07 September 2000
   Last Change: Thursday 30 August 2012
   Last Commit: $Date: 2012-08-30 08:55:24 -0600 (Thu, 30 Aug 2012) $ 

*/

#ifndef Cora_LgrNet_LgrNetSettingFactory_h
#define Cora_LgrNet_LgrNetSettingFactory_h

#include "Cora.SettingFactory.h"


namespace Cora
{
   namespace LgrNet
   {
      ////////////////////////////////////////////////////////////
      // class LgrNetSettingFactory
      //
      // Creates all settings associated with the LgrNet interface
      ////////////////////////////////////////////////////////////
      class LgrNetSettingFactory: public SettingFactory
      {
      public:
         virtual Setting *make_setting(uint4 setting_id);
      };
   };
};

#endif

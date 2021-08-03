/* Cora.SettingFactory.h

   Copyright (C) 2000, Campbell Scientific, Inc

   Written by: Jon Trauntvein
   Date Begun: Thursday 20 July 2000
   Last Change: Thursday 20 July 2000
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)

*/

#ifndef Cora_SettingFactory_h
#define Cora_SettingFactory_h

#include "Cora.Setting.h"

namespace Cora
{
   ////////// class SettingFactory
   // Defines an abstract base class that defines an interface for a setting factory. A setting
   // factory implements a method that, given a setting id, it returns a pointer to a new instance
   // of class Cora::Setting or a null pointer if the setting identifier is not recognised or
   // supported.
   class SettingFactory
   {
   public:
      ////////// make_setting
      // Should be overloaded to create a new Cora::Setting based object. The type of the setting
      // object should depend on the setting_id parameter. If the setting_id parameter is not
      // recognised, the method should return a null pointer.
      virtual Setting *make_setting(uint4 setting_id) = 0;
   };
};

#endif

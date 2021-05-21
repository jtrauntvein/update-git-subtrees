/* Cora.Tasks.TasksSettingFactory.h

   Copyright (C) 2012, Campbell Scientific, Inc

   Written by: Jon Trauntvein 
   Date Begun: Thursday 17 May 2012
   Last Change: Thursday 17 May 2012
   Last Commit: $Date: 2012-05-30 13:17:20 -0600 (Wed, 30 May 2012) $
   Last Changed by: $Author: jon $

*/

#ifndef Cora_Tasks_TasksSettingFactory_h
#define Cora_Tasks_TasksSettingFactory_h

#include "Cora.SettingFactory.h"


namespace Cora
{
   namespace Tasks
   {
      /**
       * Defines an object that is able to create any setting for the tasks interface.
       */
      class TasksSettingFactory: public SettingFactory
      {
      public:
         /**
          * Creates a setting object for the specified setting id.
          *
          * @param setting_id  The identifier for the setting.
          * @return A new setting object or null for an unrecognised setting.
          */
         virtual Setting *make_setting(uint4 setting_id);
      };
   };
};


#endif


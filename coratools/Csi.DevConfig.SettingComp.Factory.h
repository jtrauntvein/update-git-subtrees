/* Csi.DevConfig.SettingComp.Factory.h

   Copyright (C) 2003, 2006 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 17 December 2003
   Last Change: Thursday 19 October 2006
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ 
   Last Changed by: $Author: tmecham $

*/

#ifndef Csi_DevConfig_SettingComp_Factory_h
#define Csi_DevConfig_SettingComp_Factory_h

#include "Csi.DevConfig.SettingComp.DescBase.h"


namespace Csi
{
   namespace DevConfig
   {
      namespace SettingComp
      {
         ////////////////////////////////////////////////////////////
         // class Factory
         ////////////////////////////////////////////////////////////
         class Factory
         {
         public:
            ////////////////////////////////////////////////////////////
            // destructor
            ////////////////////////////////////////////////////////////
            virtual ~Factory()
            { }

            ////////////////////////////////////////////////////////////
            // make_component_desc
            ////////////////////////////////////////////////////////////
            virtual DescBase *make_component_desc(
               StrUni const &type_name);
         };
      };
   };
};


#endif

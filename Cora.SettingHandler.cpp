/* Cora.SettingHandler.cpp

   Copyright (C) 2000, Campbell Scientific, Inc

   Written by: Jon Trauntvein
   Date Begun: Thursday 20 July 2000
   Last Change: Thursday 20 July 2000
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)

*/

#pragma warning(disable:4786) // disable overlong symbols warning
#pragma hdrstop               // stop creation of precompiled header
#include "Cora.SettingHandler.h"
#include "Csi.Messaging.Message.h"
#include <assert.h>

namespace Cora
{
   ////////////////////////////////////////////////////////////
   // SettingHandler definitions
   ////////////////////////////////////////////////////////////

   void SettingHandler::read_settings(Csi::Messaging::Message *message,
                                      SettingFactory *factory,
                                      uint4 context_token)
   {
      uint4 count;
      uint4 setting_id; 
      uint4 setting_len;
      StrBin setting_bytes;
      
      if(message->readUInt4(count))
      {
         for(uint4 i = 0; i < count; ++i)
         {
            if(message->readUInt4(setting_id) &&
               message->readUInt4(setting_len))
            {
               // see if the factory recognises this setting identifier
               Csi::SharedPtr<Setting> setting(factory->make_setting(setting_id));
               if(setting.get_rep())
               {
                  if(setting->read(message))
                     on_setting_read(setting,context_token);
                  else
                     assert(false);
               }
               else
                  message->movePast(setting_len);
            }
            else
               assert(false);
         }
      }
      else
         assert(false);
   } // read_settings 
};

/* Cora.SettingHandler.h

   Copyright (C) 2000, Campbell Scientific, Inc

   Written by: Jon Trauntvein
   Date Begun: Thursday 20 July 2000
   Last Change: Thursday 20 July 2000
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)

*/

#ifndef Cora_SettingHandler_h
#define Cora_SettingHandler_h

#include "Cora.SettingFactory.h"
#include "Csi.SharedPtr.h"

namespace Cora
{
   ////////// class SettingHandler
   // The CsiLgrNet interface reference defines four different types of settings enumerate
   // transactions: LgrNet settings enumerate, Device Settings Enumerate, Device Get Settings, and
   // Device Collect Area Settings Enumerate. All of these transactions purposefully have a common
   // way of describing settings in their notification messages. This class encapsulates that
   // commonality.
   //
   // This class is designed to be used through extension. It provides a protected method,
   // read_settings(), that reads the settings from a notification message. For each supported
   // setting read, the on_setting_read() virtual method will be invoked.
   class SettingHandler
   {
   protected:
      ////////// on_setting_read
      // This method will be invoked for each setting that is read by read_settings() and recognised
      // by the setting factory. The context_token parameter is the same as was passed into
      // read_settings() and can be used by the derived class to identify the context (change versus
      // override, for instance) in which the notification is taking place.
      virtual void on_setting_read(Csi::SharedPtr<Setting> &setting,
                                   uint4 context_token) = 0;

      ////////// read_settings
      // This method is invoked to process the common part of all settings notifications
      // messages. It reads the number of settings from the message and then, for each setting, it
      // reads the setting identifier and length and the amount of bytes specified by the length. It
      // then asks the factory to create a setting. If this setting is created and read
      // successfully, on_setting_read() will be invoked to pass the setting back out to the derived
      // object.
      //
      // The context_token parameter can mean anything that the onvoker wants it to. Its is intended
      // to make it possible for a derived class to distinguish between setting changes and
      // overrides but it could also be used for other purposes. 
      void read_settings(Csi::Messaging::Message *message,
                         SettingFactory *factory,
                         uint4 context_token = 0);
   };
};

#endif

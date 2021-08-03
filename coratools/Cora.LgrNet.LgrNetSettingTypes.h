/* Cora.LgrNet.LgrNetSettingTypes.h

   Copyright (C) 2000, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 07 September 2000
   Last Change: Saturday 31 October 2020
   Last Commit: $Date: 2020-10-31 10:08:12 -0600 (Sat, 31 Oct 2020) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Cora_LgrNet_LgrNetSettingTypes_h
#define Cora_LgrNet_LgrNetSettingTypes_h

#include "Cora.Setting.h"
#include "Cora.LgrNet.Defs.h"
#include "Cora.CommonSettingTypes.h"
#include "StrAsc.h"


namespace Cora
{
   namespace LgrNet
   {
      class LogControl: public Setting
      {
      public:
         LogControl(
            uint4 setting_id,
            bool to_disc_ = true,
            uint4 bale_count_ = 5,
            uint4 bale_size_ = 1200000):
            Setting(setting_id),
            to_disc(to_disc_),
            bale_count(bale_count_),
            bale_size(bale_size_)
         { }

         virtual bool read(Csi::Messaging::Message *msg);
         virtual bool read(char const *s);
         virtual void write(Csi::Messaging::Message *out) const;
         virtual void format(std::ostream &out) const;

         virtual json_value_handle write_json() const
         {
            Csi::Json::ObjectHandle rtn(new Csi::Json::Object);
            rtn->set_property_bool("to_disc", to_disc);
            rtn->set_property_number("bale_count", bale_count);
            rtn->set_property_number("bale_size", bale_size);
            return rtn.get_handle();
         }

         virtual bool read_json(json_value_handle &val_)
         {
            bool rtn(true);
            try
            {
               Csi::Json::ObjectHandle val(val_);
               to_disc = val->get_property_bool("to_disc");
               bale_count = (uint4)val->get_property_number("bale_count");
               bale_size = (uint4)val->get_property_number("bale_size");
            }
            catch(std::exception &)
            { rtn = false; }
            return rtn;
         }
         
      protected:
         bool to_disc;
         uint4 bale_count;
         uint4 bale_size;
      };


      class SettingIpManagerKey: public Setting
      {
      protected:
         StrAsc value;
         
      public:
         SettingIpManagerKey():
            value("FCA7DAA6A5054B288C9C57DFB01BDD99"),
            Setting(Settings::ip_manager_key)
         { }

         virtual bool read(Csi::Messaging::Message *msg);
         virtual bool read(char const *s);
         virtual void write(Csi::Messaging::Message *out) const;
         virtual void format(std::ostream  &out) const;

         virtual json_value_handle write_json() const
         {
            Csi::Json::StringHandle rtn(new Csi::Json::String(value));
            return rtn.get_handle();
         }

         virtual bool read_json(json_value_handle &val_)
         {
            bool rtn = true;
            try
            {
               Csi::Json::StringHandle val(val_);
               value = val->get_value();
            }
            catch(std::exception &)
            { rtn = false; }
            return rtn;
         }
      };


      class SettingSystemClock: public SettingEnumeration
      {
      public:
         SettingSystemClock():
            SettingEnumeration(Settings::system_clock_specifier)
         {
            supported_values[1] = "local-not-corrected";
            supported_values[2] = "local-corrected";
            supported_values[3] = "gmt";
         }
      };


      class SettingReplicationType: public SettingEnumeration
      {
      public:
         SettingReplicationType():
            SettingEnumeration(Settings::replication_type)
         {
            supported_values[1] = "disabled";
            supported_values[2] = "campbell-cloud";
         }
      };
   };
};

#endif

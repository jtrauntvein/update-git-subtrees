
/* Csi.DevConfig.SummaryConverterBase.cpp

   Copyright (C) 2007, 2014 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 19 April 2007
   Last Change: Thursday 20 February 2014
   Last Commit: $Date: 2014-02-20 11:39:48 -0600 (Thu, 20 Feb 2014) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.DevConfig.SummaryConverterBase.h"
#include "Csi.DevConfig.Defs.h"
#include "Csi.StrAscStream.h"
#include "Csi.BuffStream.h"
#include "coratools.strings.h"
#include "boost/format.hpp"
#include <list>
#include <algorithm>



namespace Csi
{
   namespace DevConfig
   {
      namespace
      {
         using namespace SummaryConverterBaseStrings;

         
         ////////////////////////////////////////////////////////////
         // class crx000_four_to_five
         //
         // Converts summaries from CR1000 major version four to CR1000 major version five. 
         ////////////////////////////////////////////////////////////
         class crx000_four_to_five: public SummaryConverterBase
         {
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            crx000_four_to_five():
               SummaryConverterBase(5)
            { }

            ////////////////////////////////////////////////////////////
            // convert_setting
            ////////////////////////////////////////////////////////////
            virtual void convert_setting(
               ConfigSummary::value_type &dest,
               ConfigSummary::value_type &source)
            {
               if(source->get_identifier() == 56 || source->get_identifier() == 57)
               {
                  // we need to copy each component of the settings.  The encoding on the first has
                  // changed
                  Setting::iterator si = source->begin();
                  Setting::iterator di = dest->begin();
                  OStrAscStream otemp;

                  while(si != source->end() && di != dest->end())
                  {
                     Setting::value_type &source_comp = *si;
                     Setting::value_type &dest_comp = *di;
                     otemp.str("");
                     source_comp->output(otemp,false);
                     if(si == source->begin())
                     {
                        // we need to conditionally modify the port number
                        int port_no;
                        IBuffStream itemp(otemp.str().c_str(), otemp.str().length());

                        itemp >> port_no;
                        if(port_no > 5)
                           port_no += 3;
                        otemp.str("");
                        otemp << port_no;

                        // we now need to write this to the destination
                        IBuffStream itemp2(otemp.str().c_str(), otemp.str().length());
                        dest_comp->input(itemp2,false);
                     }
                     else
                     {
                        IBuffStream itemp(otemp.str().c_str(), otemp.str().length());
                        dest_comp->input(itemp,false);
                     }
                     ++si;
                     ++di;
                  }
               }
               else
                  SummaryConverterBase::convert_setting(dest,source);
            } // convert_setting
         };


         ////////////////////////////////////////////////////////////
         // class md485_two_to_three
         ////////////////////////////////////////////////////////////
         class md485_two_to_three: public SummaryConverterBase
         {
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            md485_two_to_three():
               SummaryConverterBase(3)
            { }

            ////////////////////////////////////////////////////////////
            // convert_setting
            ////////////////////////////////////////////////////////////
            virtual void convert_setting(
               ConfigSummary::value_type &dest,
               ConfigSummary::value_type &source)
            {
               if(source->get_identifier() == 3)
               {
                  OStrAscStream otemp;
                  source->write_formatted(otemp,false);
                  IBuffStream itemp(otemp.str().c_str(), otemp.str().length());
                  int csio_mode;
                  itemp >> csio_mode;
                  if(csio_mode > 2)
                     csio_mode += 3;
                  otemp.str("");
                  otemp << csio_mode;
                  IBuffStream itemp2(otemp.str().c_str(), otemp.str().length());
                  dest->read_formatted(itemp2,false);
               }
               else
                  SummaryConverterBase::convert_setting(dest,source);
            }
         };


         ////////////////////////////////////////////////////////////
         // class sc105_two_to_three
         ////////////////////////////////////////////////////////////
         class sc105_two_to_three: public SummaryConverterBase
         {
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            sc105_two_to_three():
               SummaryConverterBase(3)
            { }

            ////////////////////////////////////////////////////////////
            // convert_setting
            ////////////////////////////////////////////////////////////
            virtual void convert_setting(
               ConfigSummary::value_type &dest,
               ConfigSummary::value_type &source)
            {
               if(source->get_identifier() == 2)
               {
                  OStrAscStream otemp;
                  source->write_formatted(otemp,false);
                  IBuffStream itemp(otemp.str().c_str(), otemp.str().length());
                  int csio_mode;
                  itemp >> csio_mode;
                  if(csio_mode > 3)
                     csio_mode += 2;
                  otemp.str("");
                  otemp << csio_mode;
                  IBuffStream itemp2(otemp.str().c_str(), otemp.str().length());
                  dest->read_formatted(itemp2,false);
               }
               else
                  SummaryConverterBase::convert_setting(dest,source);
            }
         };


         ////////////////////////////////////////////////////////////
         // class rf401_eight_to_seven
         ////////////////////////////////////////////////////////////
         class rf401_eight_to_seven: public SummaryConverterBase
         {
         private:
            ////////////////////////////////////////////////////////////
            // active_interface
            ////////////////////////////////////////////////////////////
            ConfigSummary::value_type source_active_interface;
            ConfigSummary::value_type dest_active_interface;
            
            ////////////////////////////////////////////////////////////
            // protocol
            ////////////////////////////////////////////////////////////
            ConfigSummary::value_type source_protocol;
            ConfigSummary::value_type dest_protocol;
            
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            rf401_eight_to_seven():
               SummaryConverterBase(7)
            { }

            ////////////////////////////////////////////////////////////
            // convert_setting
            ////////////////////////////////////////////////////////////
            virtual void convert_setting(
               ConfigSummary::value_type &dest, ConfigSummary::value_type &source)
            {
               if(source->get_identifier() == 1)
               {
                  int4 sdc_address(source->get_comp_int4());
                  if(sdc_address == 7 || sdc_address == 8)
                     dest->set_comp_int4(sdc_address);
                  else
                     throw MsgExcept(my_strings[strid_rf401_incompatible_sdc_address].c_str());
               }
               else
               {
                  if(source->get_identifier() == 6)
                  {
                     source_protocol = source;
                     dest_protocol = dest;
                  }
                  else if(source->get_identifier() == 0)
                  {
                     source_active_interface = source;
                     dest_active_interface = dest;
                  }
                  SummaryConverterBase::convert_setting(dest, source);
               }
            } // convert_setting

            ////////////////////////////////////////////////////////////
            // after_convert
            ////////////////////////////////////////////////////////////
            virtual void after_convert()
            {
               // we may need to adjust the active interface if a PakBus protocol was selected in
               // the source.  The reason for this is that version 3 of the OS needs a value of CSDC
               // rather than SDC
               if(source_protocol != 0 &&
                  dest_protocol != 0 &&
                  source_active_interface != 0 &&
                  dest_active_interface != 0)
               {
                  int4 protocol(source_protocol->get_comp_int4());
                  int4 interface(source_active_interface->get_comp_int4());
                  if(protocol >= 1 && interface == 3)
                     dest_active_interface->set_comp_int4(4);
               }
            }
         };
      };

      
      ////////////////////////////////////////////////////////////
      // class SummaryConverterBase definitions
      ////////////////////////////////////////////////////////////
      ConfigSummary *SummaryConverterBase::do_conversion(ConfigSummary *source)
      {
         // we need to create a new summary based upon the device type.  We will expect the summary
         // to be initially empty and add settings as the conversion progesses.  We will need to
         // get the device description and a catalog for the destination version in order to create
         // the appropriate settings. 
         LibraryManager::iterator device_it = source->get_library()->get_device(
            source->get_device_type());
         if(device_it == source->get_library()->end())
            throw MsgExcept(my_strings[strid_no_source_device_type].c_str());
         LibraryManager::value_type &device = *device_it;
         ConfigSummary *rtn = new ConfigSummary(
            source->get_library(),
            source->get_device_type(),
            dest_version);
         SharedPtr<SettingCatalog> &catalog = rtn->get_catalog();

         // we can now try to create all of the settings for the new summary that existed in the old summary
         try
         {
            for(ConfigSummary::iterator si = source->begin(); si != source->end(); ++si)
            {
               ConfigSummary::value_type &setting = *si;
               SettingCatalog::iterator setting_desc = catalog->get_setting(setting->get_identifier());
               if(setting_desc != catalog->end())
               {
                  SharedPtr<Setting> dest_setting(new Setting(*setting_desc));
                  convert_setting(dest_setting,setting);
                  rtn->push_back(dest_setting);
               }
            }
            after_convert();
         }
         catch(std::exception &)
         {
            delete rtn;
            throw;
         }
         return rtn;
      } // do_conversion


      void SummaryConverterBase::convert_setting(
         ConfigSummary::value_type &dest_setting,
         ConfigSummary::value_type &source_setting)
      {
         OStrAscStream otemp;
         source_setting->write_formatted(otemp,false);
         IBuffStream itemp(otemp.str().c_str(), otemp.str().length());
         dest_setting->read_formatted(itemp,false);
      } // convert_setting

      
      ConfigSummary *SummaryConverterBase::convert(
         ConfigSummary *source,
         byte dest_version)
      {
         SharedPtr<SummaryConverterBase> converter;
         switch(source->get_device_type())
         {
         case DeviceTypes::type_cr1000:
         case DeviceTypes::type_cr800:
         case DeviceTypes::type_cr3000:
            if(source->get_major_version() == 4 && dest_version == 5)
               converter.bind(new crx000_four_to_five);
            break;

         case DeviceTypes::type_sc105:
            if(source->get_major_version() == 2 && dest_version == 3)
               converter.bind(new sc105_two_to_three);
            break;
            
         case DeviceTypes::type_md485:
            if(source->get_major_version() == 2 && dest_version == 3)
               converter.bind(new md485_two_to_three);
            break;

         case DeviceTypes::type_rf401:
            if(source->get_major_version() == 7 && dest_version == 8)
               converter.bind(new SummaryConverterBase(dest_version));
            else if(source->get_major_version() == 8 && dest_version == 7)
               converter.bind(new rf401_eight_to_seven); 
            break;
            
         case DeviceTypes::type_avw200:
            if(source->get_major_version() == 0 && dest_version == 1)
               converter.bind(new SummaryConverterBase(dest_version));
            break;

         case DeviceTypes::type_nl240:
         case DeviceTypes::type_nl200:
            converter.bind(new SummaryConverterBase(dest_version));
            break;
         }
         if(converter == 0)
         {
            OStrAscStream temp;
            temp << boost::format(my_strings[strid_no_conversion_supported].c_str()) %
               static_cast<uint4>(source->get_major_version()) %
               static_cast<uint4>(dest_version);
            throw Csi::MsgExcept(temp.str().c_str());
         }
         return converter->do_conversion(source);
      } // convert
   };
};


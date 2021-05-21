/* Csi.DevConfig.SettingCatalog.cpp

   Copyright (C) 2003, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 16 December 2003
   Last Change: Thursday 21 March 2019
   Last Commit: $Date: 2019-03-21 12:23:57 -0600 (Thu, 21 Mar 2019) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include <algorithm>
#include "Csi.DevConfig.SettingCatalog.h"
#include "Csi.OsException.h"
#include "Csi.fstream.h"
#include "Csi.FileSystemObject.h"


namespace Csi
{
   namespace DevConfig
   {
      namespace
      {
         class setting_has_name
         {
         public:
            StrAsc name;
            setting_has_name(StrAsc const &name_):
               name(name_)
            { }

            bool operator ()(SharedPtr<SettingDesc> const &setting) const
            { return setting->get_name() == name; }
         };


         class setting_has_id
         {
         public:
            uint2 id;

            setting_has_id(uint2 id_):
               id(id_)
            { }

            bool operator ()(SharedPtr<SettingDesc> const &setting) const
            { return setting->get_identifier() == id; }
         };
      };

      
      SettingCatalog::iterator SettingCatalog::get_setting(
         StrAsc const &name,
         bool add_if_not_present)
      {
         iterator rtn = std::find_if(
            settings.begin(),
            settings.end(),
            setting_has_name(name));
         if(rtn == settings.end() && add_if_not_present)
         {
            // we will create a string based setting object to represent this name in order to do
            // so, we need to iterate the list of settings and extract the highest ID number (we'll
            // add one to that)
            uint2 max_id = 0;
            for(iterator si = settings.begin();
                si != settings.end();
                ++si)
            {
               value_type &setting = *si;
               if(setting->get_identifier() > max_id)
                  max_id = setting->get_identifier();
            }
            ++max_id;

            // we will now create an XML node to represent the setting info
            Xml::Element setting_info("setting");
            Xml::Element::value_type comp = setting_info.add_element(L"string");
            
            setting_info.set_attr_str(name,L"name");
            setting_info.set_attr_uint2(max_id,L"id");
            comp->set_attr_str(name,L"name");
            comp->set_attr_wstr(L"100",L"length");

            // we can now create a setting description and add it to the list maintained for this
            // catalog
            value_type desc(new SettingDesc);
            desc->init_from_xml(setting_info,library_dir);
            settings.push_back(desc);
            rtn = settings.end();
            --rtn;
         }
         return rtn;
      } // get_setting


      SettingCatalog::iterator SettingCatalog::get_setting(uint2 id)
      {
         return std::find_if(
            settings.begin(),
            settings.end(),
            setting_has_id(id));
      } // get_setting
      
      
      void SettingCatalog::init_from_xml(
         Csi::Xml::Element &xml_catalog,
         StrAsc const &library_dir_)
      {
         using namespace Csi::Xml;
         std::deque<StrAsc> include_paths;
         library_dir = library_dir_;
         major_version = xml_catalog.get_attr_uint1(L"major-version");
         for(Element::iterator ei = xml_catalog.begin(); ei != xml_catalog.end(); ++ei)
         {
            Element::value_type &child = *ei;
            if(child->get_name() == L"setting")
            {
               value_type setting(new SettingDesc);
               setting->init_from_xml(*child,library_dir);
               settings.push_back(setting);
            }
            else if(child->get_name() == L"include")
            {
               // we will assume that the include path is relative to the library directory.
               std::deque<StrAsc> include_path;
               include_path.push_back(library_dir);
               include_path.push_back(child->get_attr_str(L"href"));
               include_paths.push_back(join_path(include_path.begin(), include_path.end()));
            }
         }

         // we need to parse all of the include files that were referenced.  This process may wind
         // up adding new include files.
         while(!include_paths.empty())
         {
            StrAsc include_file_name(include_paths.front());
            Csi::ifstream include_file(include_file_name);
            Element include_root(L"settings-include");

            include_paths.pop_front();
            if(!include_file)
               throw Csi::OsException("Failed to open include file");
            include_root.input(include_file);
            for(Element::iterator ei = include_root.begin(); ei != include_root.end(); ++ei)
            {
               Element::value_type &child(*ei);
               if(child->get_name() == L"setting")
               {
                  // if the setting or its components cannot be read, we will simply ignore that
                  // setting rather than prevent the whole catalogue from being supported. 
                  try
                  {
                     value_type setting(new SettingDesc);
                     setting->init_from_xml(*child, library_dir);
                     settings.push_back(setting);
                  }
                  catch(std::exception &)
                  { }
               }
               else if(child->get_name() == L"include")
               {
                  std::deque<StrAsc> include_path;
                  include_path.push_back(library_dir);
                  include_path.push_back(child->get_attr_str(L"href"));
                  include_paths.push_back(join_path(include_path.begin(), include_path.end()));
               }
            }
         }
      } // init_from_xml
   };
};

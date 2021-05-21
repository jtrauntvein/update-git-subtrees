/* Csi.DevConfig.SettingDesc.cpp

   Copyright (C) 2003, 2017 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 16 December 2003
   Last Change: Friday 06 October 2017
   Last Commit: $Date: 2017-10-06 17:09:16 -0600 (Fri, 06 Oct 2017) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.DevConfig.SettingDesc.h"
#include "Csi.MsgExcept.h"
#include <algorithm>


namespace Csi
{
   namespace DevConfig
   {
      namespace
      {
         SharedPtr<SettingComp::Factory> the_factory;
      };

      
      void SettingDesc::init_from_xml(
         Csi::Xml::Element &xml_data,
         StrAsc const &library_dir)
      {
         // read the setting attributes
         StrAsc temp = xml_data.get_attr_str(L"read-only");

         if(temp == "1" || temp == "true")
            read_only = true;
         else
            read_only = false;
         name = xml_data.get_attr_str(L"name");
         identifier = xml_data.get_attr_uint2(L"id");
         temp = xml_data.get_attr_str(L"repeat-count");
         if(temp.length())
            repeat_count = strtoul(temp.c_str(),0,10);
         else
            repeat_count = 1;
         if(repeat_count == 0)
            throw MsgExcept("Invalid repeat count specified"); 
         default_value = xml_data.get_attr_str(L"default");
         temp = xml_data.get_attr_str(L"ignore-not-present");
         if(temp == "true" || temp == "1")
            ignore_not_present = true;
         else
            ignore_not_present = false;
         if(xml_data.has_attribute(L"displayable"))
            displayable = xml_data.get_attr_bool(L"displayable");
         if(xml_data.has_attribute("auto-refresh"))
            auto_refresh = xml_data.get_attr_bool(L"auto-refresh");
         if(xml_data.has_attribute(L"include-after-commit"))
            include_after_commit = xml_data.get_attr_bool(L"include-after-commit");
         if(xml_data.has_attribute(L"tab-name"))
            tab_name = xml_data.get_attr_str(L"tab-name");
         if(xml_data.has_attribute(L"sort-order"))
            sort_order = xml_data.get_attr_uint4(L"sort-order");
         if(xml_data.has_attribute(L"min-repeat-count"))
            min_repeat_count = xml_data.get_attr_uint4(L"min-repeat-count");
         if(xml_data.has_attribute(L"ignore-summary"))
            ignore_summary = xml_data.get_attr_bool(L"ignore-summary");
         else
            ignore_summary = false;
         if(xml_data.has_attribute(L"disruptive"))
            disruptive = xml_data.get_attr_bool(L"disruptive");
         else
            disruptive = true;
         if(xml_data.has_attribute(L"common-name"))
            common_name = xml_data.get_attr_str(L"common-name");
         if(xml_data.has_attribute(L"multi-line-height"))
            multi_line_height = xml_data.get_attr_int4(L"multi-line-height");
         else
            multi_line_height = -1;
         if(xml_data.has_attribute(L"factory-write"))
            factory_write = xml_data.get_attr_bool(L"factory-write");
         else
            factory_write = false;
         if(xml_data.has_attribute(L"fixed-repeat"))
            fixed_repeat = xml_data.get_attr_bool(L"fixed-repeat");
         else
            fixed_repeat = false;
         if(xml_data.has_attribute(L"repeat-presentation"))
         {
            StrUni temp = xml_data.get_attr_wstr(L"repeat-presentation");
            if(temp == L"grid")
               repeat_presentation = repeat_present_grid;
            else
               repeat_presentation = repeat_present_normal;
         }
         else
            repeat_presentation = repeat_present_normal;
         if(xml_data.has_attribute(L"factory-only"))
            factory_only = xml_data.get_attr_bool(L"factory-only");
         else
            factory_only = false;
            
         // we need to iterate through the setting components and create a
         // description of each component
         if(the_factory == 0)
            the_factory.bind(new SettingComp::Factory);
         for(Xml::Element::iterator ei = xml_data.begin();
             ei != xml_data.end();
             ++ei)
         {
            Xml::Element::value_type &xml_comp = *ei;
            if(xml_comp->get_name() == L"enable-expr")
               enable_expression = xml_comp->get_cdata_str();
            else
            {
               SharedPtr<SettingComp::DescBase> component(
                  the_factory->make_component_desc(
                     xml_comp->get_name().c_str()));
               if(component != 0)
               {
                  component->init_from_xml(*xml_comp,library_dir);
                  components.push_back(component);
               }
               else
               {
#ifdef _DEBUG
                  StrAsc temp("Unsupported setting component type: ");
                  StrAsc temp2;
                  xml_comp->get_name().toMulti(temp2);
                  temp += temp2;
                  throw MsgExcept(temp.c_str());
#endif
               }
            }
         }
      } // init_from_xml


      void SettingDesc::set_factory(
         SharedPtr<SettingComp::Factory> the_factory_)
      { the_factory = the_factory_; }


      SharedPtr<SettingComp::Factory> SettingDesc::get_factory()
      { return the_factory; }


      namespace
      {
         ////////////////////////////////////////////////////////////
         // class comp_has_name
         ////////////////////////////////////////////////////////////
         class comp_has_name
         {
         public:
            ////////////////////////////////////////////////////////////
            // name
            ////////////////////////////////////////////////////////////
            StrAsc name;

            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            comp_has_name(StrAsc const &name_):
               name(name_)
            { }

            ////////////////////////////////////////////////////////////
            // evaluation operator
            ////////////////////////////////////////////////////////////
            bool operator() (SettingDesc::value_type const &comp)
            { return comp->get_name() == name; }
         };
      };
      

      StrAsc const &SettingDesc::get_description_text(
         StrAsc const &component_name)
      {
         iterator ci = components.begin();
         if(component_name.length())
         {
            ci = std::find_if(
               components.begin(),
               components.end(),
               comp_has_name(component_name));
         }
         if(ci == components.end())
            throw std::invalid_argument("Invalid component name");
         return (*ci)->get_description();
      } // get_description_text


      StrAsc const &SettingDesc::get_description_text(uint4 comp_no)
      {
         return components.at(comp_no)->get_description();
      }


      bool SettingDesc::description_text_is_file(StrAsc const &component_name)
      {
         iterator ci = components.begin();
         if(component_name.length())
         {
            ci = std::find_if(
               components.begin(),
               components.end(),
               comp_has_name(component_name));
         }
         if(ci == components.end())
            throw std::invalid_argument("Invalid component name");
         return (*ci)->get_description_is_file();
      } // description_text_is_file


      bool SettingDesc::description_text_is_file(uint4 comp_no)
      {
         return components.at(comp_no)->get_description_is_file();
      }
   };
};


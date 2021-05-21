/* Csi.DevConfig.ConfigSummary.cpp

   Copyright (C) 2004, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 26 March 2004
   Last Change: Monday 10 June 2019
   Last Commit: $Date: 2019-06-10 14:16:19 -0600 (Mon, 10 Jun 2019) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.DevConfig.ConfigSummary.h"
#include "Csi.DevConfig.SummaryConverterBase.h"
#include "Csi.DevConfig.SettingComp.StringComp.h"
#include "Csi.Html.StringStreamTag.h"
#include "Csi.Html.Table.h"
#include "Csi.Html.List.h"
#include "Csi.Html.Empty.h"
#include "Csi.Utils.h"
#include "Csi.StringLoader.h"
#include "Csi.OsException.h"
#include "Csi.Xml.Element.h"
#include "Csi.BuffStream.h"
#include "coratools.strings.h"
#include "Csi.fstream.h"
#include "boost/format.hpp"
#include <sstream>
#include <algorithm>
#include <iterator>


namespace Csi
{
   namespace DevConfig
   {
      using namespace ConfigSummaryStrings;

      
      ////////////////////////////////////////////////////////////
      // class ConfigSummary definitions
      ////////////////////////////////////////////////////////////
      ConfigSummary::ConfigSummary(
         SettingsManager &manager,
         bool after_commit):
         model_no(manager.get_model_no()),
         present_model_no(manager.get_device_desc()->get_model_no(true)),
         major_version(manager.get_major_version()),
         minor_version(manager.get_minor_version()),
         timestamp(LgrDate::system()),
         library(manager.get_library()),
         catalog(manager.get_catalog()),
         device_type(manager.get_device_type())
      {
         // we need to find the serial number setting (if there is one)
         SettingsManager::iterator si(manager.find_setting_common("serial-no"));
         if(si != manager.end())
         {
            Csi::OStrAscStream temp;
            temp.imbue(std::locale::classic());
            (*si)->write_formatted(temp, true);
            serial_no = temp.str();
         }
         for(SettingsManager::iterator si = manager.begin(); si != manager.end(); ++si)
         {
            SettingsManager::value_type &setting(*si);
            if(!after_commit || setting->get_include_after_commit())
               settings.push_back(setting);
         }
         load_errors = manager.get_commit_warnings();
      } // constructor (from manager)

      
      ConfigSummary::ConfigSummary(
         Xml::Element &xml_data,
         library_handle &library_):
         library(library_),
         major_version(0xFF),
         minor_version(0xFF)
      { read_xml_data(xml_data); }


      ConfigSummary::ConfigSummary(
         char const *xml_file_name,
         library_handle &library_):
         library(library_),
         major_version(0xFF),
         minor_version(0xFF)
      {
         // open a file stream to read the data
         Csi::ifstream file(xml_file_name);
         Xml::Element root(L"");
         
         if(!file)
            throw OsException("Failed to open summary file name");
         root.input(file);
         read_xml_data(root);
      } // file name constructor


      ConfigSummary::ConfigSummary(
         library_handle &library_,
         uint2 device_type_,
         byte major_version_):
         library(library_),
         device_type(device_type_),
         major_version(major_version_),
         minor_version(0xFF),
         timestamp(LgrDate::system())
      {
         LibraryManager::iterator device_it = library->get_device(device_type);
         if(device_it == library->end())
            throw std::invalid_argument("Invalid device type");
         device_desc = *device_it;
         DeviceDesc::iterator catalog_it = device_desc->find_catalog(major_version);
         if(catalog_it == device_desc->end())
            throw std::invalid_argument("Unsupported major version");
         catalog = *catalog_it;
         model_no = device_desc->get_model_no();
         present_model_no = device_desc->get_model_no(true);
      } // constructor

      
      ConfigSummary::~ConfigSummary()
      { }

      
      void ConfigSummary::update_manager(SettingsManager &manager, bool throw_on_error)
      {
         // we first need to validate the manager.  It must be loaded with the appropriate device
         // type and also specify an appropriate major version (it may not be equal but must be
         // convertible). 
         ConfigSummary *summary = this;
         try
         {
            if(manager.get_device_type() != device_type)
               throw std::invalid_argument("Invalid model number");
            if(manager.get_major_version() != major_version)
               summary = SummaryConverterBase::convert(this,manager.get_major_version());
            for(iterator si = summary->begin(); si != summary->end(); ++si)
            {
               value_type &setting(*si);
               try
               {
                  if(!setting->get_read_only())
                     manager.copy_setting(si->get_rep());
               }
               catch(std::exception &)
               {
                  if(throw_on_error)
                  {
                     if(summary != this)
                        delete summary;
                     throw;
                  }
               }
            }
         }
         catch(std::exception &)
         {
            if(summary != this)
               delete summary;
            throw;
         }
         if(summary != this)
            delete summary;
      } // update_manager

      
      void ConfigSummary::update_xml(Xml::Element &xml_data)
      {
         // set the attributes
         xml_data.set_name(L"device-config");
         xml_data.set_attr_str(model_no, L"model-no");
         xml_data.set_attr_uint4(major_version, L"major-version");
         xml_data.set_attr_uint4(minor_version, L"minor-version");
         xml_data.set_attr_str(serial_no, L"serial-no");
         xml_data.set_attr_lgrdate(timestamp, L"timestamp");
         xml_data.set_attr_uint2(device_type, L"device-type");
         
         // generate the settings
         for(settings_type::iterator si = settings.begin();
             si != settings.end();
             ++si)
         {
            // create the setting element
            value_type &setting = *si;
            SharedPtr<Xml::Element> xml_setting = xml_data.add_element(L"setting");
            Xml::OAttributeStream str_id(xml_setting.get_rep(),L"id");
            Xml::OAttributeStream str_name(xml_setting.get_rep(),L"name");

            str_id << setting->get_identifier();
            if(setting->size() == 1)
               str_name << (*setting->begin())->get_name();
            else
               str_name << setting->get_name();

            // generate the component elements
            for(Setting::iterator ci = setting->begin();
                ci != setting->end();
                ++ci)
            {
               SharedPtr<Xml::Element> xml_comp(
                  xml_setting->add_element(L"component"));
               Setting::value_type &component = *ci;
               Xml::OElementStream comp_value(xml_comp.get_rep());
               Xml::OAttributeStream str_name(xml_comp.get_rep(),L"name");
               Xml::OAttributeStream str_repeat_count(xml_comp.get_rep(),L"repeat-count");

               if(component->get_desc()->get_has_summary_value())
                  comp_value << component->get_desc()->get_summary_value();
               else
                  component->output(comp_value,false);
               str_name << component->get_name();
               str_repeat_count << component->get_repeat_count();
            }
         }
      } // update_xml


      void ConfigSummary::update_xml(char const *xml_file_name)
      {
         // load the summary into an XML structure
         Xml::Element root(L"");
         update_xml(root);

         // now open a file and stream the structure to it.
         Csi::ofstream out(xml_file_name, std::ios::binary);
         if(!out)
            throw OsException("Could not overwrite the file");
         out.imbue(std::locale::classic());
         out << "<?xml version=\"1.0\"?>\n"
             << "<?xml-stylesheet type=\"text/xsl\" href=\"file:///"
             << library->get_library_dir() << "/summary.xsl\"?>"
             << std::endl;
         root.output(out, true, 0, false);
         out << "\n\n";
      } // update_xml

      
      void ConfigSummary::report_html(Html::Document &document)
      {
         // format the heading
         typedef PolySharedPtr<Html::Tag, Html::StringStreamTag> stream_tag;
         typedef PolySharedPtr<Html::Tag, Html::Table> table_tag;
         typedef PolySharedPtr<Html::Tag, Html::Empty> empty_tag;
         stream_tag heading1(new Html::StringStreamTag("h2"));
         stream_tag heading2(new Html::StringStreamTag("p"));
         table_tag table(new Html::Table);
         Html::Tag::value_type errors_tag;

         heading1->imbue(Csi::StringLoader::make_locale());
         heading2->imbue(Csi::StringLoader::make_locale());
         (*heading1) << "Configuration of " << present_model_no;
         if(serial_no.length() > 0)
            (*heading1)  << ", " << serial_no;
         (*heading2) << "Configured on: ";
         timestamp.format(*heading2,"%#c<br>");

         // we need to format any errors that were encountered
         if(!load_errors.empty())
         {
            PolySharedPtr<Html::Tag, Html::List> errors_list(new Html::List);
            errors_tag.bind(new Html::Tag("font"));
            errors_tag->add_attribute("color", "red");
            errors_tag->add_tag(errors_list.get_handle());
            for(errors_type::iterator ei = load_errors.begin();
                ei != load_errors.end();
                ++ei)
               errors_list->add_tag(new Html::Text(*ei));
         }

         // now we can format the table
         StrAsc temp;
         table->add_attribute("border","1");
         table->add_tag(new Html::Text("<b>Setting Name</b>"));
         table->add_tag(new Html::Text("<b>Setting Value</b>"));
         for(settings_type::iterator si = settings.begin();
             si != settings.end();
             ++si)
         {
            value_type &setting = *si;
            empty_tag value_tag(new Html::Empty);
            setting->format_html(*value_tag);
            table->add_row();
            table->add_tag(new Html::Text(setting->get_name(), "b"));
            table->add_tag(value_tag.get_handle());
         }
         document.add_tag(heading1.get_handle());
         document.add_tag(heading2.get_handle());
         document.add_tag(new Html::Text("<hr><br>"));
         if(errors_tag != 0)
         {
            document.add_tag(errors_tag);
            document.add_tag(new Html::Text("<hr><br>"));
         }
         document.add_tag(table.get_handle());
      } // report_html


      namespace
      {
         ////////////////////////////////////////////////////////////
         // predicate setting_has_id
         ////////////////////////////////////////////////////////////
         struct setting_has_id
         {
            uint4 id;
            setting_has_id(uint4 id_): id(id_) { }
            bool operator() (ConfigSummary::value_type &setting)
            { return setting->get_identifier() == id; }
         };
      };
      

      ConfigSummary::value_type ConfigSummary::add_setting(uint2 id)
      {
         iterator si = std::find_if(begin(),end(),setting_has_id(id));
         value_type rtn;

         if(si != end())
            rtn = *si;
         else
         {
            SettingCatalog::iterator ci = catalog->get_setting(id);
            if(ci != catalog->end())
            {
               rtn.bind(new Setting(*ci));
               push_back(rtn);
            }
            else
               throw std::invalid_argument("ConfigSummary::add_setting: unsupported ID");
         }
         return rtn;
      } // add_setting


      void ConfigSummary::on_commit_complete(SettingsManager &manager)
      {
         std::copy(
            manager.get_commit_warnings().begin(),
            manager.get_commit_warnings().end(),
            std::back_inserter(load_errors));
      } // on_commit_complete

      
      void ConfigSummary::read_xml_data(Xml::Element &xml_data)
      {
         // initialise from the element attributes
         StrAsc temp;
            
         if(xml_data.get_name() != L"device-config")
            throw std::invalid_argument("Invalid tag type");
         model_no = xml_data.get_attr_str(L"model-no");
         if(xml_data.has_attribute("serial-no"))
            serial_no = xml_data.get_attr_str("serial-no");
         major_version = static_cast<byte>(xml_data.get_attr_uint2(L"major-version"));
         minor_version = static_cast<byte>(xml_data.get_attr_uint2(L"minor-version"));
         timestamp = xml_data.get_attr_lgrdate(L"timestamp");
            
         // we need to locate the device description in the library
         LibraryManager::iterator device_it;
         if(xml_data.has_attribute(L"device-type"))
            device_it = library->get_device(xml_data.get_attr_uint2(L"device-type"));
         else
            device_it = library->get_device(model_no);
         if(device_it == library->end())
            throw std::invalid_argument("The model number is not defined");
         device_desc = *device_it;
         present_model_no = device_desc->get_model_no(true);
         
         // we need to locate the appropriate catalog based upon the major version
         DeviceDesc::iterator catalog_it = device_desc->find_catalog(major_version);
         device_type = device_desc->get_device_type();
         if(catalog_it != device_desc->end())
            catalog = *catalog_it;
         else
            throw std::invalid_argument("Unsupported major version");
            
         // we now need to iterate through the list of settings for the device
         load_errors.clear();
         for(Xml::Element::iterator ei = xml_data.begin();
             ei != xml_data.end();
             ++ei)
         {
            SharedPtr<Xml::Element> &xml_setting = *ei;
            if(xml_setting->get_name() == L"setting")
            {
               // we now need to look up the setting description from the device catalog
               uint2 setting_id = xml_setting->get_attr_uint2(L"id");
               SettingCatalog::iterator setting_it = catalog->get_setting(setting_id);
               if(setting_it != catalog->end() && !(*setting_it)->get_ignore_summary())
               {
                  // we can now create the setting and add it to our list
                  SharedPtr<Setting> setting(new Setting(*setting_it));
                  settings.push_back(setting);

                  try
                  {
                     // we can now read the set of components for this setting
                     uint4 comp_id = 0;
                     uint4 last_repeat_count = 0;
                     for(Xml::Element::iterator xci = xml_setting->begin();
                         xci != xml_setting->end();
                         ++xci)
                     {
                        SharedPtr<Xml::Element> &xml_comp = *xci;
                        uint4 repeat_count = 0;
                        temp = xml_comp->get_cdata_str();
                        IBuffStream value(temp.c_str(),temp.length());
                        
                        if(xml_comp->has_attribute(L"repeat-count"))
                           repeat_count = xml_comp->get_attr_uint4(L"repeat-count");
                        if(repeat_count != last_repeat_count)
                        {
                           comp_id = 0;
                           last_repeat_count = repeat_count;
                        }
                        setting->read_comp_by_id(
                           value,
                           comp_id,
                           false,
                           repeat_count);
                        ++comp_id;
                     }
                  }
                  catch(std::out_of_range &e)
                  {
                     OStrAscStream error;
                     error << boost::format(my_strings[strid_load_out_of_range].c_str()) % e.what();
                     load_errors.push_back(error.str());
                     settings.pop_back();
                  }
                  catch(std::exception &e) 
                  {
                     OStrAscStream error;
                     error << boost::format(my_strings[strid_load_failed].c_str()) % e.what();
                     load_errors.push_back(error.str());
                     settings.pop_back();
                  }
               }
            } 
         }
      } // read_xml_data
   };
};

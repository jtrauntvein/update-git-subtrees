/* Csi.DevConfig.DeviceDesc.cpp

   Copyright (C) 2003, 2017 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 10 December 2003
   Last Change: Friday 06 October 2017
   Last Commit: $Date: 2018-03-15 16:43:21 -0600 (Thu, 15 Mar 2018) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.MsgExcept.h"
#include "Csi.DevConfig.DeviceDesc.h"
#include "Csi.Xml.Element.h"
#include "Csi.Xml.EventParser.h"
#include "Csi.OsException.h"
#include "Csi.BuffStream.h"
#include "Csi.FileSystemObject.h"
#include "Csi.Utils.h"
#include "Csi.fstream.h"
#include "Csi.ProgramRunner.h"
#include "Csi.RegistryManager.h"
#include "coratools.strings.h"
#include <algorithm>


namespace Csi
{
   namespace DevConfig
   {
      ////////////////////////////////////////////////////////////
      // class DeviceDesc definitions
      ////////////////////////////////////////////////////////////
      DeviceDesc::DeviceDesc(
         StrAsc const &file_name,
         StrAsc const &library_directory_):
         library_directory(library_directory_),
         config_baud_rate(9600),
         config_protocol(config_devconfig),
         exclude(false),
         force_baud_rate(false),
         os_protocol(os_protocol_none),
         os_baud_rate(0),
         refresh_interval(0),
         srecord_should_confirm(false),
         srecord_model_address(0),
         srecord_os_sig_address(0),
         has_srecord_config(false),
         session_start_retries(375),
         session_start_timeout(40),
         supports_defaults(true),
         network_config(false),
         discovery_udp_port(6785),
         network_os(false),
         supports_encryption(false),
         supports_pakbus_encryption(false),
         offline_defaults(true),
         supports_settings_editor(true),
         supports_terminal(true),
         device_description_file_name(file_name),
         init_state(init_read_nothing),
         factory_only(false)
      {
         // open the file and parse its contents
         Csi::ifstream file(file_name.c_str(), std::ios::binary);
         if(!file)
            throw OsException("Unable to read a device description file");
         read_header(file);
      } // constructor


      DeviceDesc::DeviceDesc(
         void const *dd_image,
         uint4 dd_image_len,
         StrAsc const &library_directory_):
         library_directory(library_directory_),
         config_baud_rate(9600),
         config_protocol(config_devconfig),
         exclude(false),
         force_baud_rate(false),
         os_protocol(os_protocol_none),
         os_baud_rate(0),
         refresh_interval(0),
         session_start_retries(375),
         session_start_timeout(40),
         supports_defaults(true),
         network_config(false),
         discovery_udp_port(6785),
         network_os(false),
         supports_pakbus_encryption(false),
         offline_defaults(true),
         supports_settings_editor(true),
         supports_terminal(true),
         init_state(init_read_everything),
         factory_only(false)
      {
         IBuffStream file(reinterpret_cast<char const *>(dd_image), dd_image_len);
         Xml::Element root(L"device-description");
         read_header(file);
         file.seekg(0, std::ios_base::beg);
         root.input(file);
         read_xml(root);
      } // constructor


      namespace
      {
         ////////////////////////////////////////////////////////////
         // class catalog_has_version
         ////////////////////////////////////////////////////////////
         class catalog_has_version
         {
         private:
            ////////////////////////////////////////////////////////////
            // major_version
            ////////////////////////////////////////////////////////////
            byte major_version;
            
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            catalog_has_version(byte major_version_):
               major_version(major_version_)
            { }

            ////////////////////////////////////////////////////////////
            // evaluate operator
            ////////////////////////////////////////////////////////////
            bool operator ()(SharedPtr<SettingCatalog> &catalog)
            { return catalog->get_major_version() == major_version; }
         };
      };

      
      DeviceDesc::iterator DeviceDesc::find_catalog(byte major_version)
      {
         check_loaded();
         return std::find_if(
            catalogs.begin(),
            catalogs.end(),
            catalog_has_version(major_version));
      } // find_catalog


      void DeviceDesc::install_usb_drivers()
      {
#ifdef WIN32
         StrAsc program, command_line;
         get_install_usb_items(program, command_line);
         if(program.length() != 0 || command_line.length() != 0)
         {
            trace("Installing USB: %s %s", program.c_str(), command_line.c_str());
            Csi::ProgramRunner runner(
               0,
               program.c_str(),
               command_line.c_str(),
               0xFFFFFFFF,
               SW_SHOW,
               false,
               true);
            runner.start();
         }
#endif
      } // install_usb_drivers


      void DeviceDesc::get_install_usb_items(StrAsc &program_name, StrAsc &command_line)
      {
         program_name.cut(0);
         command_line.cut(0);
#ifdef WIN32
         // we need to put together the path to the install programs
         StrAsc common_dir;
         
         RegistryManager::read_shared_value(common_dir, "CommonDir");
         if(common_dir.length() == 0)
         {
            StrAsc app_parent;
            StrAsc install_name;
            get_app_dir(install_name);
            get_path_from_file_name(common_dir, install_name.c_str());
         }
         if(common_dir.last() != FileSystemObject::dir_separator())
            common_dir.append(FileSystemObject::dir_separator());
         common_dir.append("dpinst");
         if(is_wow64())
            common_dir.append("_x64.exe");
         else
            common_dir.append("_x86.exe");
         program_name = common_dir;

         // we need to create a list of unique directories that contain drivers to be installed.
         usb_drivers_type directories;
         for(usb_drivers_type::iterator di = usb_drivers.begin(); di != usb_drivers.end(); ++di)
         {
            StrAsc path(library_directory);
            StrAsc directory;
            if(path.last() != '\\')
               path.append('\\');
            path.append(*di);
            path.replace("/", "\\");
            get_path_from_file_name(directory, path.c_str());
            if(directory.last() == '\\')
               directory.cut(directory.length() - 1);
            if(std::find(directories.begin(), directories.end(), directory) == directories.end())
               directories.push_back(directory);
         }

         // we can now line out the command line.
         OStrAscStream temp;
         for(usb_drivers_type::iterator di = directories.begin(); di != directories.end(); ++di)
         {
            if(di != directories.begin())
               temp << " ";
            temp << "/PATH \"" << *di << "\"";
         }
         command_line = temp.str();
#endif
      } // get_install_usb_items


      void DeviceDesc::check_loaded()
      {
         if(init_state != init_read_everything)
         {
            Xml::Element root(L"device-description");
            Csi::ifstream input(device_description_file_name);
            if(!input)
               throw OsException("failed to load the device description file");
            root.input(input);
            read_xml(root);
         }
      } // check_loaded


      void DeviceDesc::read_xml(Xml::Element &root)
      {
         // we can now iterate the setting catalogues in the file
         for(Xml::Element::iterator ei = root.begin(); ei != root.end(); ++ei)
         {
            Csi::Xml::Element::value_type &child = *ei;
            if(child->get_name() == L"setting-catalog")
            {
               value_type catalog(new SettingCatalog);
               catalog->init_from_xml(*child,library_directory);
               catalogs.push_back(catalog);
            }
            else if(child->get_name() == L"apply-message")
            {
               apply_message = child->get_cdata_wstr();
            }
         }
         init_state = init_read_everything;
      } // read_xml


      void DeviceDesc::read_header(std::istream &input)
      {
         // we can now parse the input
         typedef Xml::EventParser parser_type;
         parser_type parser;
         enum state_type
         {
            state_before_desc,
            state_parse_desc_attribs,
            state_parse_other,
            state_parse_os_config,
            state_parse_os_srecord,
            state_parse_os_admonition,
            state_parse_usb_driver,
            state_parse_alternative_summaries,
            state_parse_alternative_summary,
            state_parse_apply_message,
            state_complete
         } state = state_before_desc;
         bool found_apply_message(false);
         parser_type::parse_outcome_type rcd(parser.parse(input));
         StrAsc alternative_file;
         StrAsc alternative_description;
         bool skip_next(false);

         while(state != state_complete && rcd != parser_type::parse_end_of_document)
         {
            skip_next = false;
            if(state == state_before_desc)
            {
               if(rcd == parser_type::parse_start_of_element && parser.get_elem_name() == L"device-description")
                  state = state_parse_desc_attribs;
               else
                  throw std::invalid_argument("invalid device description tag name");
            }
            else if(state == state_parse_desc_attribs)
            {
               if(rcd == parser_type::parse_attribute_read)
               {
                  if(parser.get_attr_name() == L"model-no")
                     model_no = parser.get_value().to_utf8();
                  else if(parser.get_attr_name() == L"present-model-no")
                     present_model_no = parser.get_value().to_utf8();
                  else if(parser.get_attr_name() == L"device-type")
                     device_type_code = parser.get_value_uint2();
                  else if(parser.get_attr_name() == L"connect-instructions")
                  {
                     connect_instructions_file_name = library_directory;
                     if(connect_instructions_file_name.last() != Csi::FileSystemObject::dir_separator())
                        connect_instructions_file_name.append(Csi::FileSystemObject::dir_separator());
                     connect_instructions_file_name.append(parser.get_value().to_utf8());
                  }
                  else if(parser.get_attr_name() == L"connect-instructions-noip")
                  {
                     connect_instructions_noip_file_name = library_directory;
                     if(connect_instructions_noip_file_name.last() != Csi::FileSystemObject::dir_separator())
                        connect_instructions_noip_file_name.append(Csi::FileSystemObject::dir_separator());
                     connect_instructions_noip_file_name.append(parser.get_value().to_utf8());
                  }
                  else if(parser.get_attr_name() == L"config-baud-rate")
                     config_baud_rate = parser.get_value_int4();
                  else if(parser.get_attr_name() == L"force-baud-rate")
                     force_baud_rate = parser.get_value_bool();
                  else if(parser.get_attr_name() == L"config-protocol")
                  {
                     if(parser.get_value() == L"pakbus")
                        config_protocol = config_pakbus;
                     else if(parser.get_value() == L"devconfig")
                        config_protocol = config_devconfig;
                     else if(parser.get_value() == L"tx3xx")
                        config_protocol = config_tx3xx;
                     else if(parser.get_value() == L"at-commands")
                        config_protocol = config_at_commands;
                     else if(parser.get_value() == L"none")
                        config_protocol = config_none;
                  }
                  else if(parser.get_attr_name() == L"at-command-map")
                  {
                     at_command_map = library_directory;
                     if(at_command_map.last() != FileSystemObject::dir_separator())
                        at_command_map.append(FileSystemObject::dir_separator());
                     at_command_map.append(parser.get_value().to_utf8());
                  }
                  else if(parser.get_attr_name() == L"exclude")
                     exclude = parser.get_value_bool();
                  else if(parser.get_attr_name() == L"refresh-interval")
                     refresh_interval = parser.get_value_uint4();
                  else if(parser.get_attr_name() == L"session-start-retries")
                     session_start_retries = parser.get_value_uint4();
                  else if(parser.get_attr_name() == L"session-start-timeout")
                     session_start_timeout = parser.get_value_uint4();
                  else if(parser.get_attr_name() == L"default-summary")
                  {
                     StrAsc temp(library_directory);
                     if(temp.last() != FileSystemObject::dir_separator())
                        temp.append(FileSystemObject::dir_separator());
                     temp.append(parser.get_value().to_utf8());
                     if(file_exists(temp.c_str()))
                        default_summaries.push_back(default_summary_type(temp, ""));
                  }
                  else if(parser.get_attr_name() == L"supports-defaults")
                     supports_defaults = parser.get_value_bool();
                  else if(parser.get_attr_name() == L"factory-defaults-button")
                     defaults_button_name = parser.get_value();
                  else if(parser.get_attr_name() == L"network-config")
                     network_config = parser.get_value_bool();
                  else if(parser.get_attr_name() == L"network-config-password-title")
                     network_config_password_title = parser.get_value().to_utf8();
                  else if(parser.get_attr_name() == L"discovery-udp-port")
                     discovery_udp_port = parser.get_value_uint2();
                  else if(parser.get_attr_name() == L"supports-encryption")
                     supports_encryption = parser.get_value_bool();
                  else if(parser.get_attr_name() == L"supports-pakbus-encryption")
                     supports_pakbus_encryption = parser.get_value_bool();
                  else if(parser.get_attr_name() == L"category")
                     category = parser.get_value().to_utf8();
                  else if(parser.get_attr_name() == L"icon")
                  {
                     icon_file_name = library_directory;
                     if(icon_file_name.last() != FileSystemObject::dir_separator())
                        icon_file_name.append(FileSystemObject::dir_separator());
                     icon_file_name.append(parser.get_value().to_utf8());
                  }
                  else if(parser.get_attr_name() == L"offline-defaults")
                     offline_defaults = parser.get_value_bool();
                  else if(parser.get_attr_name() == L"description")
                     description = parser.get_value().to_utf8();
                  else if(parser.get_attr_name() == L"supports-settings-editor")
                     supports_settings_editor = parser.get_value_bool();
                  else if(parser.get_attr_name() == L"supports-terminal")
                     supports_terminal = parser.get_value_bool();
                  else if(parser.get_attr_name() == L"factory-only")
                     factory_only = parser.get_value_bool();
               }
               else if(rcd == parser_type::parse_start_of_element)
               {
                  skip_next = true;
                  state = state_parse_other;
               }
               else
                  state = state_complete;
            }
            else if(state == state_parse_other)
            {
               if(rcd == parser_type::parse_start_of_element)
               {
                  if(parser.get_elem_name() == L"alternative-summaries")
                     state = state_parse_alternative_summaries;
                  else if(parser.get_elem_name() == L"os-config")
                     state = state_parse_os_config;
                  else if(parser.get_elem_name() == L"usb-driver")
                     state = state_parse_usb_driver;
                  else if(parser.get_elem_name() == L"apply-message")
                     state = state_parse_apply_message;
                  else if(parser.get_elem_name() == L"setting-catalog")
                     state = state_complete;
               }
            }
            else if(state == state_parse_os_config)
            {
               if(rcd == parser_type::parse_attribute_read)
               {
                  if(parser.get_attr_name() == L"protocol")
                  {
                     if(parser.get_value() == L"csos")
                        os_protocol = os_protocol_csos;
                     else if(parser.get_value() == L"srecord")
                        os_protocol = os_protocol_srecord;
                     else if(parser.get_value() == L"srecord+usb")
                        os_protocol = os_protocol_srecord_usb;
                     else if(parser.get_value() == L"simplified-srecord")
                        os_protocol = os_protocol_simplified_srecord;
                     else if(parser.get_value() == L"devconfig")
                        os_protocol = os_protocol_devconfig;
                     else if(parser.get_value() == L"csterm")
                        os_protocol = os_protocol_csterm;
                     else if(parser.get_value() == L"sr50a")
                        os_protocol = os_protocol_sr50a;
                  }
                  else if(parser.get_attr_name() == L"baud-rate")
                     os_baud_rate = parser.get_value_uint4();
                  else if(parser.get_attr_name() == L"instructions-url")
                  {
                     os_instructions_url = library_directory;
                     if(os_instructions_url.last() != FileSystemObject::dir_separator())
                        os_instructions_url.append(FileSystemObject::dir_separator());
                     os_instructions_url.append(parser.get_value().to_utf8());
                  }
                  else if(parser.get_attr_name() == L"file-name-image")
                     os_file_extension = parser.get_value().to_utf8();
                  else if(parser.get_attr_name() == L"allow-network")
                     network_os = parser.get_value_bool();
               }
               else if(rcd == parser_type::parse_start_of_element)
               {
                  if(parser.get_elem_name() == L"srecord")
                  {
                     has_srecord_config = true;
                     srecord_os_sig_address = 0;
                     srecord_model_address = 0;
                     srecord_should_confirm = true;
                     srecord_model_no.cut(0);
                     state = state_parse_os_srecord;
                  }
                  else if(parser.get_elem_name() == L"os-admonition")
                     state = state_parse_os_admonition;
               }
               else if(rcd == parser_type::parse_end_of_element)
                  state = state_parse_other;
            }
            else if(state == state_parse_os_admonition)
            {
               if(rcd == parser_type::parse_end_of_element)
               {
                  state = state_parse_os_config;
                  os_admonishment = parser.get_value().to_utf8();
               }
            }
            else if(state == state_parse_os_srecord)
            {
               if(rcd == parser_type::parse_attribute_read)
               {
                  if(parser.get_attr_name() == L"model-no")
                     srecord_model_no = parser.get_value().to_utf8();
                  else if(parser.get_attr_name() == L"model-address")
                     srecord_model_address = parser.get_value_uint4();
                  else if(parser.get_attr_name() == L"os-sig-address")
                     srecord_os_sig_address = parser.get_value_uint4();
                  else if(parser.get_attr_name() == L"should-confirm")
                     srecord_should_confirm = parser.get_value_bool();
               }
               else if(rcd == parser_type::parse_end_of_element)
                  state = state_parse_os_config;
            }
            else if(state == state_parse_usb_driver)
            {
               if(rcd == parser_type::parse_attribute_read)
               {
                  if(parser.get_attr_name() == L"inf")
                     usb_drivers.push_back(parser.get_value().to_utf8());
               }
               else if(rcd == parser_type::parse_end_of_element)
                  state = state_parse_other;
            }
            else if(state == state_parse_alternative_summaries)
            {
               if(rcd == parser_type::parse_start_of_element && parser.get_elem_name() == L"alternative")
                  state = state_parse_alternative_summary;
               else if(rcd == parser_type::parse_end_of_element)
                  state = state_parse_other;
            }
            else if(state == state_parse_alternative_summary)
            {
               if(rcd == parser_type::parse_attribute_read)
               {
                  if(parser.get_attr_name() == L"file")
                  {
                     alternative_file = library_directory;
                     if(alternative_file.last() != FileSystemObject::dir_separator())
                        alternative_file.append(FileSystemObject::dir_separator());
                     alternative_file.append(parser.get_value().to_utf8());
                  }
                  else if(parser.get_attr_name() == L"description")
                     alternative_description = parser.get_value().to_utf8();
               }
               else if(rcd == parser_type::parse_end_of_element)
               {
                  if(file_exists(alternative_file.c_str()))
                  {
                     default_summaries.push_back(
                        default_summary_type(alternative_file, alternative_description));
                  }
                  state = state_parse_alternative_summaries;
               }
            }
            else if(state == state_parse_apply_message)
            {
               if(rcd == parser_type::parse_end_of_element)
               {
                  apply_message = parser.get_value();
                  state = state_parse_other;
               }
            }
            if(!skip_next)
               rcd = parser.parse(input);
         }
         if(!found_apply_message)
            apply_message = SettingsManagerStrings::my_strings[SettingsManagerStrings::strid_default_apply_message];
         if(connect_instructions_noip_file_name.length() == 0)
            connect_instructions_noip_file_name = connect_instructions_file_name;
         if(category.length() == 0)
            category = model_no;
         init_state = init_read_header;
      } // read_xml
   };
};


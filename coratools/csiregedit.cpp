/* csiregedit.cpp

   Copyright (C) 2011, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 11 January 2011
   Last Change: Wednesday 05 February 2020
   Last Commit: $Date: 2020-02-05 09:47:07 -0600 (Wed, 05 Feb 2020) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.CommandLine.h"
#include "Csi.Xml.Element.h"
#include "Csi.Utils.h"
#include "Csi.OsException.h"
#include <iostream>
#include <fstream>


namespace
{
   StrAsc const registry_file_name("/etc/opt/CampbellSci/csi_registry.xml");
   StrUni const registry_hive_name(L"hive");
   StrUni const registry_hive_value(L"HKEY_LOCAL_MACHINE");
   StrUni const registry_key_name(L"key");
   StrUni const registry_name_name(L"name");
   StrUni const registry_value_name(L"value");


   Csi::Xml::Element::value_type resolve_key(
      Csi::Xml::Element::value_type &root, StrAsc const &path)
   {
      using namespace Csi::Xml;
      Element::value_type parent(root);
      StrAsc key_name;
      Element::value_type rtn;

      for(int i = 0; i < path.length(); ++i)
      {
         if(path[i] == '\\' || i + 1 == path.length())
         {
            if(path[i] != '\\')
               key_name += path[i];
            rtn.clear();
            for(Element::iterator ei = parent->begin(); ei != parent->end(); ++ei)
            {
               Element::value_type &child(*ei);
               if(child->get_name() == registry_key_name && child->get_attr_str(registry_name_name) == key_name)
               {
                  rtn = child;
                  break;
               }
            }
            if(rtn != 0)
               parent = rtn;
            else 
            {
               rtn = parent->add_element(registry_key_name);
               rtn->set_attr_str(key_name, registry_name_name);
               parent = rtn;
            }
            key_name.cut(0);
         }
         else
            key_name.append(path[i]);
      }
      return rtn;
   } // resolve_key


   struct value_has_name
   {
      StrAsc const name;
      value_has_name(StrAsc const &name_):
         name(name_)
      { }

      bool operator()(Csi::Xml::Element::value_type &elem)
      {
         bool rtn(false);
         if(elem->get_name() == registry_value_name &&
            elem->get_attr_str(registry_name_name) == name)
            rtn = true;
         return rtn;
      }
   };
};


int main(int argc, char const *argv[])
{
   int rtn(0);
   try
   {
      // we need to parse the command line
      Csi::CommandLine parser;
      StrAsc file_name(registry_file_name);
      
      Csi::set_command_line(argc, argv);
      parser.add_expected_option("file");
      parser.parse_command_line(Csi::get_command_line());
      parser.get_option_value("file", file_name);
      Csi::check_file_name_path(file_name.c_str());
      
      // we need to attempt to read in the registry file
      using namespace Csi::Xml;
      Element::value_type registry(new Element(L"registry"));
      std::ifstream input(file_name.c_str(), std::ios::binary);
      if(input)
      {
         registry->input(input);
         input.close();
      }
      else
         registry->set_attr_wstr(registry_hive_value, registry_hive_name);

      // we need to set a machine GUID for this machine if it has not already been set.
      Element::value_type crypto_key(resolve_key(registry, "SOFTWARE\\Microsoft\\Cryptography"));
      Element::value_type guid_val(crypto_key->find_elem_if(value_has_name("MachineGuid"), false));
      if(guid_val == 0)
      {
         guid_val = crypto_key->add_element(registry_value_name);
         guid_val->set_attr_wstr(L"MachineGuid", registry_name_name);
         guid_val->set_cdata_str(Csi::make_guid());
      }
      
      // we are now ready to fill the requests on the command line
      size_t arg(1);
      StrAsc op;
      while(arg < parser.args_size())
      {
         parser.get_argument(op, arg);
         if(op == "add")
         {
            // process the arguments
            StrAsc path;
            StrAsc value_name;
            StrAsc value;
            if(!parser.get_argument(path, arg + 1) ||
               !parser.get_argument(value_name, arg + 2) ||
               !parser.get_argument(value, arg + 3))
               throw std::invalid_argument("expect the path, value name, and value for the add operation");
            arg += 4;

            // look for the appropriate key
            Element::value_type key(resolve_key(registry, path));
            Element::value_type value_xml(
               key->find_elem_if(value_has_name(value_name), false));

            if(value_xml == 0)
            {
               value_xml = key->add_element(registry_value_name);
               value_xml->set_attr_str(value_name, registry_name_name);
            }
            value_xml->set_cdata_str(value); 
         }
         else
            throw std::invalid_argument("invalid registry operation");
      }

      // we now need to rewrite the registry file
      std::ofstream output(file_name.c_str(), std::ios::binary);
      if(output)
         registry->output(output, true);
      else
         throw Csi::OsException("failed to write the registry file");
   }
   catch(std::exception &e)
   {
      std::cout << "regedit error: " << e.what() << std::endl;
      rtn = 1;
   }
   return rtn;
} // main

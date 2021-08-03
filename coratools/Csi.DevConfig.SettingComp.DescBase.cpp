/* Csi.DevConfig.SettingComp.DescBase.cpp

   Copyright (C) 2004, 2021 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 26 January 2004
   Last Change: Monday 19 April 2021
   Last Commit: $Date: 2021-04-20 12:42:24 -0600 (Tue, 20 Apr 2021) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.DevConfig.SettingComp.DescBase.h"


namespace Csi
{
   namespace DevConfig
   {
      namespace SettingComp
      {
         void DescBase::init_from_xml(
            Csi::Xml::Element &xml_data,
            StrAsc const &library_dir)
         {
            name = xml_data.get_attr_str(L"name");
            if(xml_data.has_attribute("common-name"))
               common_name = xml_data.get_attr_str("common-name");
            format_prefix = xml_data.get_attr_str(L"format-prefix");
            format_postfix = xml_data.get_attr_str(L"format-postfix");
            read_only = false;
            if(xml_data.has_attribute(L"read-only"))
               read_only = xml_data.get_attr_bool(L"read-only");
            default_value = xml_data.get_attr_str(L"default");
            if(xml_data.has_attribute(L"summary-value"))
            {
               has_summary_value = true;
               summary_value = xml_data.get_attr_str(L"summary-value");
            }
            for(Xml::Element::iterator ei = xml_data.begin();
                ei != xml_data.end();
                ++ei)
            {
               Xml::Element::value_type &xml_value = *ei;
               if(xml_value->get_name() == L"description")
               {
                  description =
                     "<html><head>\n"
                     "<link href=\"style.css\" rel=\"stylesheet\" type=\"text/css\">\n"
                     "</head>\n"
                     "<body>\n";
                  description.append("<b>" + name + "</b>\n<p>");
                  description.append(xml_value->get_cdata_str());
                  description.append("</p>\n</body></html>");
               }
               else if(xml_value->get_name() == L"description-url")
               {
                  description = library_dir;
#ifdef _WIN32
                  if(description.last() != '\\')
                     description += '\\';
#else
                  if(description.last() != '/')
                     description.append('/');
#endif
                  description_url = xml_value->get_attr_str(L"href");
                  description += description_url;
                  description_is_file = true;
               }
               else if(xml_value->get_name() == L"enable-expr")
                  enable_expr = xml_value->get_cdata_str();
            }

            // we need to read the set of validation rules from the XML structure.
            Xml::Element::iterator vi(xml_data.find(L"validate"));
            rules.clear();
            if(vi != xml_data.end())
            {
               Xml::Element::value_type &validate(*vi);
               for(Xml::Element::iterator ri = validate->begin(); ri != validate->end(); ++ri)
               {
                  Xml::Element::value_type &rule(*ri);
                  if(rule->get_name() == L"rule")
                  {
                     Xml::Element::value_type predicate(rule->find_elem(L"predicate", 0, true));
                     Xml::Element::value_type message(rule->find_elem(L"message", 0, true));
                     rules.push_back(rule_type(predicate->get_cdata_str(), message->get_cdata_str()));
                  }
               }
            }
         } // init_from_xml

         void DescBase::describe_json(Csi::Json::Object &desc_json)
         {
            Csi::Json::ArrayHandle rules_json(new Csi::Json::Array);
            desc_json.set_property_str("name", name);
            desc_json.set_property_str("common_name", common_name);
            desc_json.set_property_str("format_prefix", format_prefix);
            desc_json.set_property_str("format_postfix", format_postfix);
            desc_json.set_property_str("description", description);
            desc_json.set_property_str("description_url", description_url);
            desc_json.set_property_bool("description_is_file", description_is_file);
            desc_json.set_property_number("component_type", component_type);
            desc_json.set_property_str("enable_expr", enable_expr);
            desc_json.set_property_bool("read_only", read_only);
            for(rules_type::iterator ri = rules.begin(); ri != rules.end(); ++ri)
            {
               Csi::Json::ObjectHandle rule(new Csi::Json::Object);
               rule->set_property_str("rule", ri->first);
               rule->set_property_str("message", ri->second);
               rules_json->push_back(rule.get_handle());
            }
            desc_json.set_property("rules", rules_json.get_handle());
         } // describe_json
      };
   };
};


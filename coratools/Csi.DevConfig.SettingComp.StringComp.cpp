/* Csi.DevConfig.SettingComp.StringComp.cpp

   Copyright (C) 2004, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 13 January 2004
   Last Change: Thursday 17 December 2020
   Last Commit: $Date: 2020-12-17 09:15:18 -0600 (Thu, 17 Dec 2020) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.DevConfig.SettingComp.StringComp.h"
#include "Csi.Utils.h"
#include "CsiTypes.h"
#include "Csi.StrAscStream.h"
#include <regex>


namespace Csi
{
   namespace DevConfig
   {
      namespace SettingComp
      {
         void StringComp::input(std::istream &in, bool translate)
         {
            StrAsc temp;
            if(!desc->get_multi_line())
               temp.readLine(in);
            else
            {
               char ch;
               while(in.get(ch))
                  temp.append(ch);
               while(isspace(temp.last()))
                  temp.cut(temp.length() - 1);
            }
            if(desc->is_valid_input(temp.c_str()))
               value = temp, has_changed = true;
            else
               throw std::invalid_argument(desc->get_name().c_str());
         } // input

         byte StringComp::get_val_byte()
         {
            IBuffStream temp(value.c_str(), value.length());
            int rtn;
            temp.imbue(std::locale::classic());
            temp >> rtn;
            if(!temp)
               throw InvalidComponentConversion();
            if(rtn < 0 || rtn > 0xFF)
               throw InvalidComponentConversion();
            return static_cast<byte>(rtn);
         } // get_val_byte
         
         int2 StringComp::get_val_int2()
         {
            IBuffStream temp(value.c_str(), value.length());
            int2 rtn;
            temp.imbue(std::locale::classic());
            temp >> rtn;
            if(!temp)
               throw InvalidComponentConversion();
            return rtn;
         } // get_val_int2

         uint2 StringComp::get_val_uint2()
         {
            IBuffStream temp(value.c_str(), value.length());
            uint2 rtn;
            temp.imbue(std::locale::classic());
            temp >> rtn;
            if(!temp)
               throw InvalidComponentConversion();
            return rtn;
         } // get_val_uint2
         
         int4 StringComp::get_val_int4()
         {
            IBuffStream temp(value.c_str(), value.length());
            int4 rtn;
            temp.imbue(std::locale::classic());
            temp >> rtn;
            if(!temp)
               throw InvalidComponentConversion();
            return rtn;
         } // get_val_int4
         
         float StringComp::get_val_float()
         {
            return static_cast<float>(
               csiStringToFloat(value.c_str(), std::locale::classic(), true));
         } // get_val_float
         
         double StringComp::get_val_double()
         {
            return csiStringToFloat(value.c_str(), std::locale::classic(), true); 
         } // get_val_double
         
         void StringComp::set_val_byte(byte v)
         {
            OStrAscStream temp;
            temp.imbue(std::locale::classic());
            temp << static_cast<uint2>(v);
            set_val_str(temp.str());
         } // set_val_byte
         
         void StringComp::set_val_int2(int2 v)
         {
            OStrAscStream temp;
            temp.imbue(std::locale::classic());
            temp << v;
            set_val_str(temp.str());
         } // set_val_int2
         
         void StringComp::set_val_uint2(uint2 v)
         {
            OStrAscStream temp;
            temp.imbue(std::locale::classic());
            temp << v;
            set_val_str(temp.str());
         } // set_val_uint2
         
         void StringComp::set_val_int4(int4 v)
         {
            OStrAscStream temp;
            temp.imbue(std::locale::classic());
            temp << v;
            set_val_str(temp.str());
         } // set_val_int4
         
         void StringComp::set_val_uint4(uint4 v)
         {
            OStrAscStream temp;
            temp.imbue(std::locale::classic());
            temp << v;
            set_val_str(temp.str());
         } // set_val_uint4
         
         void StringComp::set_val_float(float v)
         {
            OStrAscStream temp;
            temp.imbue(std::locale::classic());
            csiFloatToStream(temp, v);
            set_val_str(temp.str());
         } // set_val_float
         
         void StringComp::set_val_double(double v)
         {
            OStrAscStream temp;
            temp.imbue(std::locale::classic());
            csiFloatToStream(temp, v);
            set_val_str(temp.str());
         } // set_val_double

         void StringComp::set_val_str(StrAsc const &val, bool apply_checks)
         {
            StrAsc temp(val);
            while(temp.length() > 0 && std::isspace(temp.last()))
               temp.cut(temp.length() - 1);
            while(temp.length() > 0 && std::isspace(temp.first()))
               temp.cut(0);
            if(apply_checks)
            {
               if(temp.length() > desc->get_max_len())
                  throw std::invalid_argument("too long");
               if(!desc->is_valid_input(temp.c_str()))
                  throw std::invalid_argument("invalid content");
            }
            value = temp;
            set_has_changed(true);
         } // set_val_str
         
         
         bool StringCompDesc::is_valid_input(char const *s)
         {
            bool rtn = strlen(s) <= max_len;
            if(rtn && validate_pattern.length())
            {
               std::regex ex(validate_pattern.c_str());
               std::cmatch what;
               
               rtn = std::regex_match(s,what,ex); 
            }
            return rtn;
         } // is_valid_input

         
         void ChoiceCompDesc::init_from_xml(
            Csi::Xml::Element &xml_data,
            StrAsc const &library_dir)
         {
            StringCompDesc::init_from_xml(xml_data,library_dir);
            for(Xml::Element::iterator ei = xml_data.begin();
                ei != xml_data.end();
                ++ei)
            {
               Xml::Element::value_type &xml_value = *ei;
               if(xml_value->get_name() == L"item")
               {
                  byte min_minor(0);
                  if(xml_value->has_attribute(L"min-minor"))
                     min_minor = xml_value->get_attr_uint1(L"min-minor");
                  values.push_back(
                     value_type(
                        xml_value->get_attr_str(L"value"),
                        xml_value->get_attr_str(L"name"),
                        min_minor));
               }
            }
            if(xml_data.has_attribute(L"accept-current-value"))
               accept_current_value = xml_data.get_attr_bool(L"accept-current-value");
            else
               accept_current_value = false;
         } // init_from_xml

         void ChoiceCompDesc::output_value(
            std::ostream &out,
            StrAsc const &value,
            bool translate)
         {
            bool found(false);
            for(values_type::const_iterator vi = values.begin();
                vi != values.end();
                ++vi)
            {
               if(translate && vi->value == value)
               {
                  out << vi->name;
                  found = true;
                  break;
               }
               else if(!translate && vi->value == value)
               {
                  out << vi->value;
                  found = true;
                  break;
               }
            }
            if(!found && accept_current_value)
               out << value;
         } // output_value

         void ChoiceCompDesc::input_value(
            std::istream &in,
            StrAsc &value,
            bool translate)
         {
            StrAsc temp;
            values_type::const_iterator vi;
            bool found(false);
            temp.readLine(in);
            for(vi = values.begin(); vi != values.end(); ++vi)
            {
               if(translate && vi->name == temp)
               {
                  value = vi->value;
                  found = true;
                  break;
               }
               else if(!translate && vi->value == temp)
               {
                  value = vi->value;
                  found = true;
                  break;
               }
            }
            if(!found && accept_current_value)
               value = temp;
            else if(!found && !accept_current_value)
               throw std::range_error(name.c_str());
         } // input_value
      };
   };
};

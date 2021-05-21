/* Csi.DevConfig.SettingComp.CompScalar.cpp

   Copyright (C) 2003, 2011 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 17 December 2003
   Last Change: Thursday 20 January 2011
   Last Commit: $Date: 2018-11-30 16:37:18 -0600 (Fri, 30 Nov 2018) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.DevConfig.SettingComp.CompScalar.h"


namespace Csi
{
   namespace DevConfig
   {
      namespace SettingComp
      {
         ////////////////////////////////////////////////////////////
         // class CompEnumDesc definitions
         ////////////////////////////////////////////////////////////
         void CompEnumDesc::init_from_xml(
            Xml::Element &xml_data,
            StrAsc const &library_dir)
         {
            DescBase::init_from_xml(xml_data,library_dir);
            if(xml_data.has_attribute(L"case-sensitive"))
               case_sensitive = xml_data.get_attr_bool(L"case-sensitive");
            else
               case_sensitive = false;
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
                        xml_value->get_attr_int1(L"value"),
                        xml_value->get_attr_str(L"name"),
                        min_minor));
               }
            }
         } // init_from_xml


         CompBase *CompEnumDesc::make_component(
            SharedPtr<DescBase> &desc,
            SharedPtr<CompBase> &previous_component)
         { return new CompEnum(desc); }


         void CompEnumDesc::output_value(
            std::ostream &out,
            byte value,
            bool translate)
         {
            if(translate)
            {
               values_type::const_iterator vi;
               for(vi = values.begin(); vi != values.end(); ++vi)
               {
                  if(vi->value == value)
                  {
                     out << vi->name;
                     break;
                  }
               } 
               if(vi == values.end())
                  out << static_cast<int2>(value);
            }
            else
               out << static_cast<int2>(value); 
         } // output_value


         void CompEnumDesc::output_value(
            std::wostream &out,
            byte value,
            bool translate)
         {
            if(translate)
            {
               values_type::const_iterator vi;
               for(vi = values.begin(); vi != values.end(); ++vi)
               {
                  if(vi->value == value)
                  {
                     out << vi->name;
                     break;
                  }
               } 
               if(vi == values.end())
                  out << static_cast<int2>(value);
            }
            else
               out << static_cast<int2>(value); 
         } // output_value (wide stream)


         void CompEnumDesc::input_value(
            std::istream &in,
            byte &value,
            bool translate,
            byte minor_version)
         {
            if(translate)
            {
               // we will first read the value as a line from the stream.  If the user placed
               // whitespace at the beginning or end of the line, we will strip that off. 
               StrAsc temp;
               size_t space_count = 0;
               
               temp.readLine(in);
               while(space_count < temp.length() && isspace(temp[space_count]))
                  ++space_count;
               if(space_count > 0)
                  temp.cut(0, space_count);
               while(temp.length() > 0 && isspace(temp.last()))
                  temp.cut(temp.length() - 1);
               
               // we will now attempt to recognise the value
               values_type::const_iterator vi;
               for(vi = values.begin(); vi != values.end(); ++vi)
               {
                  if(vi->name.compare(temp, case_sensitive) == 0)
                  {
                     if(minor_version >= vi->min_minor)
                        value = vi->value;
                     else
                        throw std::out_of_range(name.c_str());
                     break;
                  }
               }
               if(vi == values.end())
                  throw std::out_of_range(name.c_str());
            }
            else
            {
               int2 temp;
               if(in >> temp)
               {
                  values_type::iterator vi;
                  for(vi = values.begin(); vi != values.end(); ++vi)
                  {
                     if(vi->value == static_cast<byte>(temp))
                     {
                        if(minor_version >= vi->min_minor)
                           value = vi->value;
                        else
                           throw std::out_of_range(name.c_str());
                        break;
                     }
                  }
                  if(vi == values.end())
                     throw std::out_of_range(name.c_str());
               }
               else
                  throw MsgExcept("stream read error");
            }
         } // input_value


         ////////////////////////////////////////////////////////////
         // class CompEnum definitions
         ////////////////////////////////////////////////////////////
         void CompEnum::set_val_byte(byte val)
         {
            desc_type::const_iterator vi(desc->find(val));
            if(vi == desc->end())
               throw std::out_of_range(desc->get_name().c_str());
            if(minor_version < vi->min_minor)
               throw std::out_of_range(desc->get_name().c_str());
            value = val;
            has_changed = true;
         } // set_val_byte
         
         
         void CompEnum::do_copy(CompBase *other_) 
         {
            // we need to ensure that the value being set works with our minor version
            CompEnum *other(static_cast<CompEnum *>(other_));
            desc_type::const_iterator value_it(desc->find(other->value));
            if(value_it != desc->end())
            {
               CompEnumDesc::value_type const &value_desc(*value_it);
               if(minor_version >= value_desc.min_minor)
                  value = other->value;
               else
                  throw std::out_of_range(desc->get_name().c_str());
            }
            else
               throw std::out_of_range("Attempt to set undefined value"); 
         } // do_copy


         ////////////////////////////////////////////////////////////
         // class CompEnumI4Desc definitions
         ////////////////////////////////////////////////////////////
         void CompEnumI4Desc::init_from_xml(
            Xml::Element &xml_data,
            StrAsc const &library_dir)
         {
            DescBase::init_from_xml(xml_data,library_dir);
            if(xml_data.has_attribute(L"case-sensitive"))
               case_sensitive = xml_data.get_attr_bool(L"case-sensitive");
            else
               case_sensitive = false;
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
                        xml_value->get_attr_int4(L"value"),
                        xml_value->get_attr_str(L"name"),
                        min_minor));
               }
            }
         } // init_from_xml


         CompBase *CompEnumI4Desc::make_component(
            SharedPtr<DescBase> &desc,
            SharedPtr<CompBase> &previous_component)
         { return new CompEnumI4(desc); }


         void CompEnumI4Desc::output_value(
            std::ostream &out,
            int4 value,
            bool translate)
         {
            if(translate)
            {
               values_type::const_iterator vi;
               for(vi = values.begin(); vi != values.end(); ++vi)
               {
                  if(vi->value == value)
                  {
                     out << vi->name;
                     break;
                  }
               } 
               if(vi == values.end())
                  out << value;
            }
            else
               out << value;
         } // output_value


         void CompEnumI4Desc::output_value(
            std::wostream &out,
            int4 value,
            bool translate)
         {
            if(translate)
            {
               values_type::const_iterator vi;
               for(vi = values.begin(); vi != values.end(); ++vi)
               {
                  if(vi->value == value)
                  {
                     out << vi->name;
                     break;
                  }
               } 
               if(vi == values.end())
                  out << value;
            }
            else
               out << value;
         } // output_value


         void CompEnumI4Desc::input_value(
            std::istream &in,
            int4 &value,
            bool translate,
            byte minor_version)
         {
            if(translate)
            {
               // we will first read the value as a line from the stream.  If the user placed
               // whitespace at the beginning or end of the line, we will strip that off. 
               StrAsc temp;
               size_t space_count = 0;
               
               temp.readLine(in);
               while(space_count < temp.length() && isspace(temp[space_count]))
                  ++space_count;
               if(space_count > 0)
                  temp.cut(0, space_count);
               while(temp.length() > 0 && isspace(temp.last()))
                  temp.cut(temp.length() - 1);

               // we will now attempt to recognise the value
               values_type::const_iterator vi;
               for(vi = values.begin(); vi != values.end(); ++vi)
               {
                  if(vi->name.compare(temp, case_sensitive) == 0)
                  {
                     if(minor_version >= vi->min_minor)
                        value = vi->value;
                     else
                        throw std::out_of_range(name.c_str());
                     break;
                  }
               }
               if(vi == values.end())
                  throw std::out_of_range(name.c_str());
            }
            else
            {
               int4 temp;
               if(in >> temp)
               {
                  values_type::iterator vi;
                  for(vi = values.begin(); vi != values.end(); ++vi)
                  {
                     if(vi->value == temp)
                     {
                        if(minor_version >= vi->min_minor)
                           value = vi->value;
                        else
                           throw std::out_of_range(name.c_str());
                        break;
                     }
                  }
                  if(vi == values.end())
                     throw std::out_of_range(name.c_str());
               }
               else
                  throw MsgExcept("stream read error");
            }
         } // input_value


         ////////////////////////////////////////////////////////////
         // class CompEnumI4 definitions
         ////////////////////////////////////////////////////////////
         void CompEnumI4::set_val_int4(int4 val)
         {
            desc_type::const_iterator vi(desc->find(val));
            if(vi != desc->end())
            {
               if(minor_version >= vi->min_minor)
               {
                  value = val;
                  has_changed = true;
               }
               else
                  throw std::out_of_range(desc->get_name().c_str());
            }
            else
               throw std::out_of_range(desc->get_name().c_str());
         } // set_val_int4

         
         void CompEnumI4::do_copy(CompBase *other_) 
         {
            // we need to ensure that the value being set works with our minor version
            CompEnumI4 *other(static_cast<CompEnumI4 *>(other_));
            desc_type::const_iterator value_it(desc->find(other->value));
            if(value_it != desc->end())
            {
               CompEnumI4Desc::value_type const &value_desc(*value_it);
               if(minor_version >= value_desc.min_minor)
                  value = other->value;
               else
                  throw std::out_of_range(desc->get_name().c_str());
            }
            else
               throw std::out_of_range(desc->get_name().c_str());
         } // do_copy
         

         ////////////////////////////////////////////////////////////
         // class CompEnumI2Desc definitions
         ////////////////////////////////////////////////////////////
         void CompEnumI2Desc::init_from_xml(
            Xml::Element &xml_data,
            StrAsc const &library_dir)
         {
            DescBase::init_from_xml(xml_data,library_dir);
            if(xml_data.has_attribute(L"case-sensitive"))
               case_sensitive = xml_data.get_attr_bool(L"case-sensitive");
            else
               case_sensitive = false;
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
                        xml_value->get_attr_int2(L"value"),
                        xml_value->get_attr_str(L"name"),
                        min_minor));
               }
            }
         } // init_from_xml


         CompBase *CompEnumI2Desc::make_component(
            SharedPtr<DescBase> &desc,
            SharedPtr<CompBase> &previous_component)
         { return new CompEnumI2(desc); }


         void CompEnumI2Desc::output_value(
            std::ostream &out,
            int2 value,
            bool translate)
         {
            if(translate)
            {
               values_type::const_iterator vi;
               for(vi = values.begin(); vi != values.end(); ++vi)
               {
                  if(vi->value == value)
                  {
                     out << vi->name;
                     break;
                  }
               } 
               if(vi == values.end())
                  out << value;
            }
            else
               out << value;
         } // output_value


         void CompEnumI2Desc::output_value(
            std::wostream &out,
            int2 value,
            bool translate)
         {
            if(translate)
            {
               values_type::const_iterator vi;
               for(vi = values.begin(); vi != values.end(); ++vi)
               {
                  if(vi->value == value)
                  {
                     out << vi->name;
                     break;
                  }
               } 
               if(vi == values.end())
                  out << value;
            }
            else
               out << value;
         } // output_value


         void CompEnumI2Desc::input_value(
            std::istream &in,
            int2 &value,
            bool translate,
            byte minor_version)
         {
            if(translate)
            {
               // we will first read the value as a line from the stream.  If the user placed
               // whitespace at the beginning or end of the line, we will strip that off. 
               StrAsc temp;
               size_t space_count = 0;
               
               temp.readLine(in);
               while(space_count < temp.length() && isspace(temp[space_count]))
                  ++space_count;
               if(space_count > 0)
                  temp.cut(0, space_count);
               while(temp.length() > 0 && isspace(temp.last()))
                  temp.cut(temp.length() - 1);
               
               // we will now attempt to recognise the value
               values_type::const_iterator vi;
               for(vi = values.begin(); vi != values.end(); ++vi)
               {
                  if(vi->name.compare(temp, case_sensitive) == 0)
                  {
                     if(minor_version >= vi->min_minor)
                        value = vi->value;
                     else
                        throw std::out_of_range(name.c_str());
                     break;
                  }
               }
               if(vi == values.end())
                  throw std::out_of_range(name.c_str());
            }
            else
            {
               int4 temp;
               if(in >> temp)
               {
                  values_type::iterator vi;
                  for(vi = values.begin(); vi != values.end(); ++vi)
                  {
                     if(vi->value == temp)
                     {
                        if(minor_version >= vi->min_minor)
                           value = vi->value;
                        else
                           throw std::out_of_range(name.c_str());
                        break;
                     }
                  }
                  if(vi == values.end())
                     throw std::out_of_range(name.c_str());
               }
               else
                  throw MsgExcept("stream read error");
            }
         } // input_value


         ////////////////////////////////////////////////////////////
         // class CompEnumI2 definitions
         ////////////////////////////////////////////////////////////
         void CompEnumI2::set_val_int2(int2 val)
         {
            desc_type::const_iterator value_it(desc->find(val));
            if(value_it != desc->end())
            {
               if(minor_version >= value_it->min_minor)
               {
                  value = val;
                  has_changed = true;
               }
               else
                  throw std::out_of_range(desc->get_name().c_str());
            }
            else
               throw std::out_of_range(desc->get_name().c_str());
         } // set_val_int2

         
         void CompEnumI2::do_copy(CompBase *other_) 
         {
            // we need to ensure that the value being set works with our minor version
            CompEnumI2 *other(static_cast<CompEnumI2 *>(other_));
            desc_type::const_iterator value_it(desc->find(other->value));
            if(value_it != desc->end())
            {
               CompEnumI2Desc::value_type const &value_desc(*value_it);
               if(minor_version >= value_desc.min_minor)
               {
                  has_changed = true;
                  value = other->value;
               }
               else
                  throw std::out_of_range(desc->get_name().c_str());
            }
            else
               throw std::out_of_range(desc->get_name().c_str());
         } // do_copy 
      };
   };
};


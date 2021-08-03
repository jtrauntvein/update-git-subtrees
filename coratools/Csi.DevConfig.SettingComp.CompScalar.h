/* Csi.DevConfig.SettingComp.CompScalar.h

   Copyright (C) 2003, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 17 December 2003
   Last Change: Tuesday 03 December 2019
   Last Commit: $Date: 2019-12-03 17:02:16 -0600 (Tue, 03 Dec 2019) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Csi_DevConfig_SettingComp_CompScalar_h
#define Csi_DevConfig_SettingComp_CompScalar_h

#include <limits>
#include <list>
#include "Csi.DevConfig.SettingComp.DescBase.h"
#include "Csi.ByteOrder.h"
#include "Csi.PolySharedPtr.h"
#include "Csi.MsgExcept.h"
#include "Csi.StrAscStream.h"
#include "CsiTypes.h"
#include "Csi.FloatUtils.h"
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
#pragma warning(disable: 4800)


namespace Csi
{
   namespace DevConfig
   {
      namespace SettingComp
      {
         /**
          * Defines the traits of a type of scalar component.
          */
         template <typename scalar_type>
         struct CompTraits
         {
            /**
             * @return Returns the scalar value read from the message.
             *
             * @param in Specifies the message to read.
             */
            static scalar_type read_message(SharedPtr<Message> &in)
            {
               scalar_type rtn;
               in->readBytes(&rtn,sizeof(rtn),!is_big_endian());
               return rtn;
            }

            /**
             * Writes the specified value to the message.

             * @param out Specifies the message to write.
             *
             * @param val Specifies the value to write.
             */
            static void write_message(
               SharedPtr<Message> &out, 
               scalar_type const &val)
            { out->addBytes(&val,sizeof(val),!is_big_endian()); }
            
            /**
             * Formats the specified value to the specified stream.
             *
             * @param out Specifies the stream to write.
             *
             * @param val Specifies the value to write.
             *
             * @param digits Specifies the number of significant digits to use.
             */
            static void output(std::ostream &out, scalar_type const &val, int digits)
            { out << val; }
            static void output(std::wostream &out, scalar_type const &val, int digits)
            { out << val; }

            /**
             * Parses the value from the stream.
             */
            static void input(std::istream &in, scalar_type &val)
            {
               in >> val;
            }

            /**
             * Reads the limits from the specified XML structure to the parameters.
             *
             * @param xml_data Specifies the descriptioon structure.
             *
             * @param min_value Specifeis the minimum value reference.
             *
             8 @param max_value Specifies the maximum value reference.
            */
            static bool read_limits(
               Csi::Xml::Element &xml_data,
               scalar_type &min_value,
               scalar_type &max_value)
            {
               bool rtn = false;
               if(xml_data.has_attribute(L"min"))
               {
                  StrUni const &attr = xml_data.get_attr_wstr(L"min");
                  IBuffStreamw temp(attr.c_str(), attr.length());
                  temp >> min_value;
                  rtn = true;
               }
               if(xml_data.has_attribute(L"max"))
               {
                  StrUni const &attr = xml_data.get_attr_wstr(L"max");
                  IBuffStreamw temp(attr.c_str(), attr.length());
                  temp >> max_value;
                  rtn = true;
               }
               return rtn;
            }

            /**
             * @return Returns the maximum number of significant digits from the XML description
             * structure.
             */
            static int read_digits(Xml::Element &xml_data)
            { return 0; }

            /**
             * Adds the specified value to the specified JSON array.
             */
            static void add_json(Json::Array &components_json, scalar_type value)
            { components_json.push_back(new Json::Number(value)); }

            /**
             * Assigns the specified value from the current array iterator.
             */
            static void get_json(Json::Array::iterator current, scalar_type &value)
            {
               Json::NumberHandle value_json(*current);
               value = (scalar_type)value_json->get_value();
            }
         };


         template < >
         struct CompTraits<int1>
         {
            static int1 read_message(SharedPtr<Message> &in)
            {
               int1 rtn;
               in->readBytes(&rtn,1,false);
               return rtn;
            }

            static void write_message(SharedPtr<Message> &out, int1 const &val)
            { out->addBytes(&val,1,false); }
            
            static void output(std::ostream &out, int1 const &val, int digits)
            { out << static_cast<int2>(val); }
            static void output(std::wostream &out, int1 const &val, int digits)
            { out << static_cast<int2>(val); }

            static void input(std::istream &in, int1 &val)
            {
               int2 temp;
               in >> temp;
               val = static_cast<int1>(temp);
            }

            static bool read_limits(
               Csi::Xml::Element &xml_data,
               int1 &min_value,
               int1 &max_value)
            {
               bool rtn = false;
               if(xml_data.has_attribute(L"min"))
               {
                  min_value = xml_data.get_attr_int1(L"min");
                  rtn = true;
               }
               if(xml_data.has_attribute(L"max"))
               {
                  max_value = xml_data.get_attr_int1(L"max");
                  rtn = true;
               }
               return false; 
            }

            static int read_digits(Xml::Element &xml)
            { return 0; }

            static void add_json(Json::Array &json, int1 value)
            { json.push_back(new Json::Number(value)); }

            static void get_json(Json::Array::iterator current, int1 &value)
            {
               Json::NumberHandle value_json(*current);
               value = (int1)value_json->get_value();
            }
         };


         template < >
         struct CompTraits<uint1>
         {
            static uint1 read_message(SharedPtr<Message> &in)
            { return in->readByte(); }

            static void write_message(SharedPtr<Message> &out, uint1 const &val)
            { out->addByte(val); }
            
            static void output(std::ostream &out, uint1 const &val, int digits)
            { out << static_cast<uint2>(val); }
            static void output(std::wostream &out, uint1 const &val, int digits)
            { out << static_cast<uint2>(val); }

            static void input(std::istream &in, uint1 &val)
            {
               uint2 temp;
               in >> temp;
               val = static_cast<uint1>(temp);
            }

            static bool read_limits(
               Xml::Element &xml_data,
               uint1 &min_value,
               uint1 &max_value)
            {
               bool rtn = false;
               if(xml_data.has_attribute(L"min"))
               {
                  min_value = xml_data.get_attr_uint1(L"min");
                  rtn = true;
               }
               if(xml_data.has_attribute(L"max"))
               {
                  max_value = xml_data.get_attr_uint1(L"max");
                  rtn = true;
               }
               return rtn;
            }

            static int read_digits(Xml::Element &xml)
            { return 0; }

            static void add_json(Json::Array &json, uint1 val)
            { json.push_back(new Json::Number(val)); }

            static void get_json(Json::Array::iterator current, uint1 &val)
            {
               Json::NumberHandle value_json(*current);
               val = (uint1)value_json->get_value();
            }
         };


         template < >
         struct CompTraits<bool>
         {
            ////////////////////////////////////////////////////////////
            // read_message
            ////////////////////////////////////////////////////////////
            static bool read_message(SharedPtr<Message> &in)
            { return in->readBool(); }

            static void write_message(SharedPtr<Message> &out, bool val)
            { out->addByte(val ? 1 : 0); }

            static void output(std::ostream &out, bool val, int digits)
            { out << val; }
            static void output(std::wostream &out, bool val, int digits)
            { out << val; }

            static void input(std::istream &in, bool &val)
            {
               in >> val;
            }

            static bool read_limits(
               Xml::Element &xml_data,
               bool &min_value,
               bool &max_value)
            { return false; }

            static int read_digits(Xml::Element &xml)
            { return 0; }

            static void add_json(Json::Array &json, bool val)
            { json.push_back(new Json::Boolean(val)); }

            static void get_json(Json::Array::iterator current, bool &val)
            {
               Json::BooleanHandle value_json(*current);
               val = value_json->get_value();
            }
         };


         template < >
         struct CompTraits<float>
         {
            static float read_message(SharedPtr<Message> &in)
            { return in->readIeee4(); }

            static void write_message(SharedPtr<Message> &out, float val)
            { out->addIeee4(val); }

            static void output(std::ostream &out, float val, int digits)
            { csiFloatToStream(out, val, digits); }
            
            static void output(std::wostream &out, float val, int digits)
            {
               Csi::OStrAscStream temp;
               temp.imbue(out.getloc());
               csiFloatToStream(temp, val, digits);
               out << temp.str();
            }

            static void input(std::istream &in, float &val)
            {
               StrAsc temp;
               in >> temp;
               val = static_cast<float>(csiStringToFloat(temp.c_str()));
               if(!in && temp.length() > 0)
                  in.clear();
            }


            static bool read_limits(
               Xml::Element &xml_data,
               float &min_value,
               float &max_value)
            {
               bool rtn = false;
               if(xml_data.has_attribute(L"min"))
               {
                  min_value = xml_data.get_attr_float(L"min");
                  rtn = true;
               }
               else
                  min_value = -std::numeric_limits<float>::max();
               if(xml_data.has_attribute(L"max"))
               {
                  max_value = xml_data.get_attr_float(L"max");
                  rtn = true;
               }
               return rtn;
            }

            static int read_digits(Xml::Element &xml)
            {
               int rtn = 7;
               if(xml.has_attribute(L"digits"))
                  rtn = xml.get_attr_int4(L"digits");
               return rtn;
            }

            static void add_json(Json::Array &json, float val)
            { json.push_back(new Json::Number(val)); }

            static void get_json(Json::Array::iterator current, float &val)
            {
               Json::NumberHandle value_json(*current);
               val = (float)value_json->get_value();
            }
         };


         template < >
         struct CompTraits<double>
         {
            static double read_message(SharedPtr<Message> &in)
            { return in->readIeee8(); }

            static void write_message(SharedPtr<Message> &out, double val)
            { out->addIeee8(val); }

            static void output(std::ostream &out, double val, int digits)
            { csiFloatToStream(out, val, digits); }
            
            static void output(std::wostream &out, double val, int digits)
            {
               Csi::OStrAscStream temp;
               temp.imbue(out.getloc());
               csiFloatToStream(temp, val, digits);
               out << temp.str();
            }

            static void input(std::istream &in, double &val)
            {
               StrAsc temp;
               in >> temp;
               if(!in && temp.length() > 0)
                  in.clear();
               val = csiStringToFloat(temp.c_str());
            }


            static bool read_limits(
               Xml::Element &xml_data,
               double &min_value,
               double &max_value)
            {
               bool rtn = false;
               if(xml_data.has_attribute(L"min"))
               {
                  min_value = xml_data.get_attr_double(L"min");
                  rtn = true;
               }
               else
                  min_value = -std::numeric_limits<double>::max();
               if(xml_data.has_attribute(L"max"))
               {
                  max_value = xml_data.get_attr_double(L"max");
                  rtn = true;
               }
               return rtn;
            }

            static int read_digits(Xml::Element &xml)
            {
               int rtn = 15;
               if(xml.has_attribute(L"digits"))
                  rtn = xml.get_attr_int4(L"digits");
               return rtn;
            }

            static void add_json(Json::Array &json, double val)
            { json.push_back(new Json::Number(val)); }

            static void get_json(Json::Array::iterator current, double &val)
            {
               Json::NumberHandle value_json(*current);
               val = value_json->get_value();
            }
         };


         namespace
         {
            template <typename scalar_type>
            uint4 component_type_code(scalar_type var)
            { return 0; }
            template < >
            uint4 component_type_code<int1>(int1 var)
            { return Components::comp_int1; }
            template < >
            uint4 component_type_code<uint1>(uint1 var)
            { return Components::comp_uint1; }
            template < >
            uint4 component_type_code<int2>(int2 var)
            { return Components::comp_int2; }
            template < >
            uint4 component_type_code<uint2>(uint2 var)
            { return Components::comp_uint2; }
            template < >
            uint4 component_type_code<int4>(int4 var)
            { return Components::comp_int4; }
            template < >
            uint4 component_type_code<uint4>(uint4 var)
            { return Components::comp_uint4; }
            template < >
            uint4 component_type_code<float>(float var)
            { return Components::comp_float; }
            template < >
            uint4 component_type_code<double>(double var)
            { return Components::comp_double; }
            template < >
            uint4 component_type_code<bool>(bool var)
            { return Components::comp_bool; }
         };
         

         /**
          * Defines a template for the component description for scalar types.
          */
         template <typename scalar_type>
         class CompScalarDesc: public DescBase
         {
         protected:
            /**
             * Specifies the minimum value.
             */
            scalar_type min_value;

            /**
             * Specifies the maximum value.
             */
            scalar_type max_value;

            /**
             * Set to true if there is a max or min value.
             */
            bool has_max_min;

            /**
             * Specifies the number of significant digits.
             */
            int digits;

         public:
            /**
             * Constructor
             */
            CompScalarDesc():
               min_value(std::numeric_limits<scalar_type>::min()),
               max_value(std::numeric_limits<scalar_type>::max()),
               DescBase(component_type_code<scalar_type>(scalar_type())),
               has_max_min(false),
               digits(0)
            { }

            /**
             * Constructor with explicit type code.
             *
             * @param code Specifies the type code for this description.
             */
            CompScalarDesc(Components::component_code_type code):
               min_value(std::numeric_limits<scalar_type>::min()),
               max_value(std::numeric_limits<scalar_type>::max()),
               DescBase(code),
               has_max_min(false),
               digits(0)
            { }

            /**
             * Overloads the base class version to read from the provided structure.
             */
            virtual void init_from_xml(
               Xml::Element &xml_data,
               StrAsc const &library_dir)
            {
               DescBase::init_from_xml(xml_data,library_dir);
               has_max_min = CompTraits<scalar_type>::read_limits(
                  xml_data, min_value, max_value);
               digits = CompTraits<scalar_type>::read_digits(xml_data);
            }

            /**
             * @return Overloads the base class version to return the component object.
             */
            virtual CompBase *make_component(
               SharedPtr<DescBase> &desc,
               SharedPtr<CompBase> &previous_component);

            /**
             * @return Returns the minimum value.
             */
            scalar_type const &get_min_value() const
            { return min_value; }

            /**
             * @return Returns the maximum value.
             */
            scalar_type const &get_max_value() const
            { return max_value; }

            /** @return Returns true if there is a max or min value.
             */
            virtual bool has_maxima() const
            { return has_max_min; }

            /**
             * Overload the base class version to format the maximum value.
             */
            virtual void format_max(std::ostream &out) const
            {
               if(component_type == Components::comp_int1 || component_type == Components::comp_uint1)
                  out << static_cast<int>(max_value);
               else
                  out << max_value;
            }

            /**
             * Overloads the base class version to format the minimum value.
             */
            virtual void format_min(std::ostream &out) const
            {
               if(component_type == Components::comp_int1 || component_type == Components::comp_uint1)
                  out << static_cast<int>(min_value);
               else
                  out << min_value;
            }

            /**
             * @return Overloads the base class version to return the number of significant digits.
             */
            virtual int get_significant_digits() const
            { return digits; }

            /**
             * Overloads the base class to write a JSON description.
             */
            virtual void describe_json(Csi::Json::Object &desc)
            {
               DescBase::describe_json(desc);
               desc.set_property_number("min_value", min_value);
               desc.set_property_number("max_value", max_value);
               desc.set_property_bool("has_max_min", has_max_min);
               desc.set_property_number("digits", digits);
            }
         };


         /**
          * Defines a template that is used to represent the various types of scalar components.
          */
         template <typename scalar_type, class desc_type_ = CompScalarDesc<scalar_type> >
         class CompScalar: public CompBase
         {
         protected:
            /**
             * Specifies the component value.
             */
            scalar_type value;

            /**
             * Specifies a reference to the specific description.
             */
         public:
            typedef desc_type_ desc_type;
         protected:
            Csi::PolySharedPtr<DescBase, desc_type > desc;
            
         public:
            /**
             * Constructor
             *
             * @param desc_ Specifies the component description.
             */
            CompScalar(
               SharedPtr<DescBase> &desc_):
               desc(desc_),
               CompBase(desc_)
            { value = desc->get_min_value(); }

            /**
             * @return Returns the value.
             */
            scalar_type const &get_value() const
            { return value; }

            /**
             * Overloads the base class version to read the component value from the message.
             */
            virtual void read(SharedPtr<Message> &in)
            { value = CompTraits<scalar_type>::read_message(in); }

            /**
             * Overloads the base class version to write the component value to the message.
             */
            virtual void write(SharedPtr<Message> &out)
            { CompTraits<scalar_type>::write_message(out,value); }

            /**
             * Overloads the base class version to write the component value to the stream.
             */
            virtual void output(std::ostream &out, bool translate)
            { CompTraits<scalar_type>::output(out, value, desc->get_significant_digits()); }
            virtual void output(std::wostream &out, bool translate)
            { CompTraits<scalar_type>::output(out, value, desc->get_significant_digits()); }

            /**
             * Overloads the base class version to read the component value from the stream.
             */
            virtual void input(std::istream &in, bool translate)
            {
               scalar_type temp = 0;
               CompTraits<scalar_type>::input(in,temp);
               if(in)
               {
                  if(!desc->has_maxima() ||
                     (temp >= desc->get_min_value() && temp <= desc->get_max_value()))
                  {
                     value = temp;
                     has_changed  = true;
                  }
                  else
                     throw std::out_of_range(desc->get_name().c_str());
               }
               else
                  throw std::invalid_argument(desc->get_name().c_str());
            }

            /**
             * Overloads the base class version to add the value to the specified JSIN array.
             */
            virtual void write(Json::Array &json)
            { CompTraits<scalar_type>::add_json(json, value); }

            /**
             * Overloads the base class version to read the value from the current array position.
             */
            virtual Json::Array::iterator read(Json::Array::iterator current)
            {
               CompTraits<scalar_type>::get_json(current, value);
               return ++current;
            }

            // @group: overloads of the base class get/set methods
            
            /**
             * Overloads the base class version to cast the value as a byte
             */
            virtual byte get_val_byte()
            { return static_cast<byte>(value); }

            /**
             * Overloads the base class version to set the value as a byte.
             */
            virtual void set_val_byte(byte val)
            {
               scalar_type candidate = static_cast<scalar_type>(val);
               if(desc->has_maxima() &&
                  (candidate < desc->get_min_value() || candidate > desc->get_max_value()))
                  throw std::out_of_range(get_name().c_str());
               value = candidate;
               has_changed = true; 
            }

            /**
             * Overloads the base class version to return the value as an int2.
             */
            virtual int2 get_val_int2()
            { return static_cast<int2>(value); }

            /**
             * Overloads the base class version to set the value as an int2.
             */
            virtual void set_val_int2(int2 val)
            {
               scalar_type candidate(static_cast<scalar_type>(val));
               if(desc->has_maxima() &&
                  (candidate < desc->get_min_value() || candidate > desc->get_max_value()))
                  throw std::out_of_range(get_name().c_str());
               value = candidate;
               has_changed = true;
            }

            /**
             * Overloads the base class version to return the value as a uint2.
             */
            virtual uint2 get_val_uint2()
            { return static_cast<uint2>(value); }

            /**
             * Overloads the base class version to set the value as a uint2.
             */
            virtual void set_val_uint2(uint2 val)
            {
               scalar_type candidate(static_cast<scalar_type>(val));
               if(desc->has_maxima() &&
                  (candidate < desc->get_min_value() || candidate > desc->get_max_value()))
                  throw std::out_of_range(get_name().c_str());
               value = candidate;
               has_changed = true; 
            }

            /**
             * Overloads the base class version to return the value as an int4.
             */
            virtual int4 get_val_int4()
            { return static_cast<int4>(value); }

            /**
             * Overloads the base class version to set the value as an int4.
             */
            virtual void set_val_int4(int4 val)
            {
               scalar_type candidate(static_cast<scalar_type>(val));
               if(desc->has_maxima() &&
                  (candidate < desc->get_min_value() || candidate > desc->get_max_value()))
                  throw std::out_of_range(get_name().c_str());
               value = candidate;
               has_changed = true;
            }

            /**
             * Overloads the base class version to return the value as a uint4.
             */
            virtual uint4 get_val_uint4()
            { return static_cast<uint4>(value); }

            /**
             * Overloads the base class version to set the value as a uint4.
             */
            virtual void set_val_uint4(uint4 val)
            {
               scalar_type candidate(static_cast<scalar_type>(val));
               if(desc->has_maxima() &&
                  (candidate < desc->get_min_value() || candidate > desc->get_max_value()))
                  throw std::out_of_range(get_name().c_str());
               value = candidate;
               has_changed = true;
            }

            /**
             * Overloads the base class version to return the value as a float.
             */
            virtual float get_val_float()
            { return static_cast<float>(value); }

            /**
             * Overloads the base class version to set the value as a float.
             */
            virtual void set_val_float(float val)
            {
               scalar_type candidate(static_cast<scalar_type>(val));
               if(desc->has_maxima())
               {
                  if(!is_finite(candidate) ||
                     candidate < desc->get_min_value() ||
                     candidate > desc->get_max_value())
                     throw std::out_of_range(get_name().c_str());
               }
               value = candidate;
               has_changed = true;
            }

            /**
             * Overloads the base class version to return the value as a double.
             */
            virtual double get_val_double()
            { return static_cast<double>(value); }

            /**
             * Overloads the base class version to set the value as a double.
             */
            virtual void set_val_double(double val)
            {
               scalar_type candidate(static_cast<scalar_type>(val));
               if(desc->has_maxima())
               {
                  if(!is_finite(val) ||
                     candidate < desc->get_min_value() ||
                     candidate > desc->get_max_value())
                     throw std::out_of_range(get_name().c_str());
               }
               value = candidate;
               has_changed = true;
            }

            // @endgroup
            
         protected:
            /**
             * Overloads the base class version to perform the copy.
             */
            virtual void do_copy(CompBase *other_)
            {
               CompScalar *other = static_cast<CompScalar *>(other_);
               value = other->value; 
            }
         };


         /**
          * Defines a template that implements the make_component method.
          */
         template <typename scalar_type>
         inline CompBase *CompScalarDesc<scalar_type>::make_component(
            SharedPtr<DescBase> &desc,
            SharedPtr<CompBase> &previous_component)
         { return new CompScalar<scalar_type>(desc); }


         /**
          * Defines a template that describes an item for an enum type component.
          */
         template <typename value_type>
         class EnumItem
         {
         public:
            /**
             * Specifies the name for this choice.
             */
            StrAsc name;

            /**
             * Specifies the value for this choice.
             */
            value_type value;

            /**
             * Specifies the minimum minor version for this choice.
             */
            byte min_minor;

            /**
             * Constructor
             */
            EnumItem(
               value_type value_ = 0, StrAsc const &name_ = "", byte min_minor_ = 0):
               name(name_),
               value(value_),
               min_minor(min_minor_)
            { }

            /**
             * Copy contructor
             */
            EnumItem(EnumItem const &other):
               name(other.name),
               value(other.value),
               min_minor(other.min_minor)
            { }

            /**
             * Copy operator
             */
            EnumItem &operator =(EnumItem const &other)
            {
               name = other.name;
               value = other.value;
               min_minor = other.min_minor;
               return *this;
            }
         };


         /**
          * Defines a functor that returns true if a given item has a specified value.
          */
         template <typename value_type>
         struct item_has_val
         {
            value_type const val;
            item_has_val(value_type val_):
               val(val_)
            { }

            bool operator ()(EnumItem<value_type> const &item) const
            { return item.value == val; }
         };
         
         
         /**
          * Defines a description for an enum component type.
          */
         class CompEnumDesc: public DescBase
         {
         public:
            /**
             * Specifies the list of values available.
             */
            typedef EnumItem<byte> value_type;
            typedef std::list<value_type> values_type;
         protected:
            values_type values;

            /**
             * Set to true if the item names should be considered to be case sensitive.
             */
            bool case_sensitive;

         public:
            /**
             * Constructor
             */
            CompEnumDesc():
               case_sensitive(false),
               DescBase(Components::comp_enum)
            { }

            /**
             * Overloads the base class version to initialise this description from the specified
             * XML structure.
             */
            virtual void init_from_xml(
               Xml::Element &xml_data,
               StrAsc const &library_dir);

            /**
             * overloads the base class version to generate the component.
             */
            virtual CompBase *make_component(
               SharedPtr<DescBase> &desc,
               SharedPtr<CompBase> &previous_component);

            /**
             * Outputs the specified value to the specified stream.
             *
             * @param out Specifies the stream to write.
             *
             * @param value Specifies the value to write.
             *
             * @param translate Set to true if the name of the value should be written.
             */
            virtual void output_value(
               std::ostream &out,
               byte value,
               bool translate);
            virtual void output_value(
               std::wostream &out,
               byte value,
               bool translate);

            /**
             * Reads the value from the specified stream.
             *
             * @param in Specifies the stream to parse.
             *
             * @param value Specifies the value to read.
             *
             * @param translate Set to true if the item name should be expected.
             */
            virtual void input_value(
               std::istream &in,
               byte &value,
               bool translate,
               byte minor_version);

            /**
             * @return Returns an iterator to the first choice.
             */
            typedef values_type::const_iterator const_iterator;
            const_iterator begin() const
            { return values.begin(); }

            /**
             * @return Returns an iterator beyond the last choice.
             */
            const_iterator end() const
            { return values.end(); }

            /**
             * @return Returns true if there are no choices available.
             */
            bool empty() const
            { return values.empty(); }

            /**
             * @return Returns a reference to the first choice.
             */
            value_type const &front() const
            { return values.front(); }

            /**
             * @return Returns an iterator to the choice associated with the specified value.
             */
            const_iterator find(byte value) const
            { return std::find_if(values.begin(), values.end(), item_has_val<byte>(value)); }
         };


         /**
          * Defines a component object that represents a single byte value that belongs in a set of
          * enumerated values.
          */
         class CompEnum: public CompBase
         {
         protected:
            /**
             * Specifies the current value for this component.
             */
            byte value;

            /**
             * Specifies the description.
             */
            PolySharedPtr<DescBase, CompEnumDesc> desc;
            
         public:
            /**
             * Constructor
             */
            CompEnum(SharedPtr<DescBase> &desc):
               CompBase(desc)
            {
               this->desc = desc;
               if(!this->desc->empty())
                  value = this->desc->front().value;
            }

            /**
             * Typedef to the description class.
             */
            typedef CompEnumDesc desc_type;

            /**
             * @return Returns the current component value.
             */
            byte get_value() const
            { return value; }

            /**
             * @param value_ Specifies the current component value.
             */
            void set_value(byte value_)
            {
               value = value_;
               has_changed = true;
            }

            /**
             * @return Returns the description object.
             */
            PolySharedPtr<DescBase, desc_type> &get_desc()
            { return desc; }

            /**
             * Overloads the base class versin to to read the value from the message.
             */
            virtual void read(SharedPtr<Message> &message)
            { message->readBytes(&value,1,false); }

            /**
             * Overloads the base class versionn to read the value from the message.
             */
            virtual void write(SharedPtr<Message> &out)
            { out->addBytes(&value,1,false); }

            /**
             * Overloads the base class version to add the current value to the array.
             */
            virtual void write(Json::Array &json)
            { json.push_back(new Json::Number(value)); }

            /**
             * Overloads the base class version to extract the value from the current position.
             */
            virtual Json::Array::iterator read(Json::Array::iterator current)
            {
               Json::NumberHandle value_json(*current);
               value = (uint1)value_json->get_value();
               return ++current;
            }

            /**
             * Overloads the base class version to write the value or its name to the specified
             * stream.
             */
            virtual void output(std::ostream &out, bool translate)
            { desc->output_value(out,value,translate); }
            virtual void output(std::wostream &out, bool translate)
            { desc->output_value(out,value,translate); }

            /**
             * Overloads the base class version to read the value or its name from the specified
             * stream.
             */
            virtual void input(std::istream &in, bool translate)
            {
               desc->input_value(in, value, translate, minor_version);
               has_changed = true;
            }

            // @group: overloads for set/get methods

            virtual byte get_val_byte()
            { return value; }

            virtual void set_val_byte(byte val);

            virtual int2 get_val_int2()
            { return value; }

            virtual void set_val_int2(int2 val)
            {
               if(val < 0 || val > 0xFF)
                  throw std::range_error(desc->get_name().c_str());
               set_val_byte(static_cast<byte>(val));
            }

            virtual uint2 get_val_uint2()
            { return value; }

            virtual void set_val_uint2(uint2 val)
            {
               if(val > 0xFF)
                  throw std::out_of_range(desc->get_name().c_str());
               set_val_byte(static_cast<byte>(val));
            }

            virtual int4 get_val_int4()
            { return value; }

            virtual void set_val_int4(int4 val)
            {
               if(val < 0 || val > 0xFF)
                  throw std::out_of_range(desc->get_name().c_str());
               set_val_byte(static_cast<byte>(val));
            }

            virtual uint4 get_val_uint4()
            { return value; }

            virtual void set_val_uint4(uint4 val)
            {
               if(val > 0xFF)
                  throw std::out_of_range(desc->get_name().c_str());
               set_val_byte(static_cast<byte>(val));
            }

            virtual float get_val_float()
            { return value; }

            virtual void set_val_float(float val)
            { set_val_int4(static_cast<int4>(val)); }

            virtual double get_val_double()
            { return value; }

            virtual void set_val_double(double val)
            { set_val_float(static_cast<float>(val)); }
            
            // @endgroup

         protected:
            /**
             * Overloads the base class version to copy the value.
             */
            virtual void do_copy(CompBase *other_);
         };


         /**
          * Defines an object that represents the description for an enum component that uses int4
          * values.
          */
         class CompEnumI4Desc: public DescBase
         {
         public:
            /**
             * Specifies the collection of values available.
             */
            typedef EnumItem<int4> value_type;
            typedef std::list<value_type> values_type;
         protected:
            values_type values;

            /**
             * Set to true if item names should be cae sensitive.
             */
            bool case_sensitive;

         public:
            /**
             * Constructor
             */
            CompEnumI4Desc():
               case_sensitive(false),
               DescBase(Components::comp_enumi4)
            { }

            /**
             * Overloads the base class version to read description properties from the specified
             * XML structure.
             */
            virtual void init_from_xml(
               Xml::Element &xml_data,
               StrAsc const &library_dir);

            /**
             * Overloads the base class version to generate the component.
             */
            virtual CompBase *make_component(
               SharedPtr<DescBase> &desc,
               SharedPtr<CompBase> &previous_component);

            /**
             * Overloads the base class version to write the current value or its name to the
             * streamm.
             */
            virtual void output_value(
               std::ostream &out,
               int4 value,
               bool translate);
            virtual void output_value(
               std::wostream &out,
               int4 value,
               bool translate);

            /**
             * Overloads the base class version to read the current value or its name from the
             * stream.
             */
            virtual void input_value(
               std::istream &in,
               int4 &value,
               bool translate,
               byte minor_version);

            /**
             * @return Returns an iterator to the first choice.
             */
            typedef values_type::const_iterator const_iterator;
            const_iterator begin() const
            { return values.begin(); }

            /**
             * @return Returns an iterator beyond the last choice.
             */
            const_iterator end() const
            { return values.end(); }

            /**
             * @return Returns true if there are no choices.
             */
            bool empty() const
            { return values.empty(); }

            /**
             * @return Returns a reference to the first choice.
             */
            value_type const &front() const
            { return values.front(); } 

            /**
             * @return Returns an iterator associated with the specified value.
             *
             * @param value Specifies the value to look up.
             */
            const_iterator find(int4 value) const
            { return std::find_if(values.begin(), values.end(), item_has_val<int4>(value)); }
         };


         /**
          * Defines a component object that presents an enumeration of int4 values.
          */
         class CompEnumI4: public CompBase
         {
         protected:
            /**
             * Specifies the current value.
             */
            int4 value;

            /**
             * Specifies the description.
             */
         public:
            typedef CompEnumI4Desc desc_type;
            typedef PolySharedPtr<DescBase, desc_type> desc_handle;
         protected:
            desc_handle desc;

         public:
            /**
             * Constructor
             */
            CompEnumI4(SharedPtr<DescBase> &desc):
               CompBase(desc)
            {
               this->desc = desc;
               if(!this->desc->empty())
                  value = this->desc->front().value;
            }

            /**
             * @return Returns the description.
             */
            desc_handle &get_desc()
            { return desc; }
            
            /**
             * @return Returns the current value.
             */
            int4 get_value() const
            { return value; }

            /**
             * Overloads the base class to read the value from the message.
             */
            virtual void read(SharedPtr<Message> &message)
            { value = message->readInt4(); }

            /**
             * Overloads the base class version to write the value to the message.
             */
            virtual void write(SharedPtr<Message> &message)
            { message->addInt4(value); }

            /**
             * Overloads the base class version to format the value or its name to the specified
             * stream.
             */
            virtual void output(std::ostream &out, bool translate)
            { desc->output_value(out,value,translate); }
            virtual void output(std::wostream &out, bool translate)
            { desc->output_value(out,value,translate); }

            /**
             * Overloads the base class version to read the value or its name from the stream.
             */
            virtual void input(std::istream &in, bool translate)
            {
               desc->input_value(in, value, translate, minor_version);
               has_changed = true;
            }

            /**
             * Overloads the base class version to add the current value to the array.
             */
            virtual void write(Json::Array &json)
            { json.push_back(new Json::Number(value)); }

            /**
             * Overloads the base class version to read the value from the specified array iterator.
             */
            virtual Json::Array::iterator read(Json::Array::iterator current)
            {
               Json::NumberHandle value_json(*current);
               value = (int4)value_json->get_value();
               return ++current;
            }

            /**
             * Overloaded to return the value as an int4.
             */
            virtual int4 get_val_int4()
            { return value; }

            /**
             * Overloaded to set the value as an int4.
             */
            virtual void set_val_int4(int4 val);

            /**
             * @return Returns the value as a floating point value.
             */
            virtual float get_val_float()
            { return static_cast<float>(value); }

            /**
             * @param val Specifies the new value for this component.
             */
            virtual void set_val_float(float val)
            { set_val_int4(static_cast<int4>(val)); }

            /**
             * @return Returns the value as a double.
             */
            virtual double get_val_double()
            { return value; }

            /**
             * @param val Specifies the value of the enum as a double.
             */
            virtual void set_val_double(double val)
            { set_val_int4(static_cast<int4>(value)); }

         protected:
            /**
             * Overloads the base class version to copy the value.
             */
            virtual void do_copy(CompBase *other_);
         };


         /**
          * Defines a description object for an enumerated component that uses int2 values.
          */
         class CompEnumI2Desc: public DescBase
         {
         public:
            /**
             * Specifies the values that are supported.
             */
            typedef EnumItem<int2> value_type;
            typedef std::list<value_type> values_type;
         protected:
            values_type values;

            /**
             * Set to true if the names are case sensitive.
             */
            bool case_sensitive;

         public:
            /**
             * Constructor
             */
            CompEnumI2Desc():
               case_sensitive(false),
               DescBase(Components::comp_enumi2)
            { }

            /**
             * Ocverloads the base class version to read properties from the specified XML
             * structure.
             */
            virtual void init_from_xml(
               Xml::Element &xml_data,
               StrAsc const &library_dir);

            /**
             * Overloads the base class version to allocate the component.
             */
            virtual CompBase *make_component(
               SharedPtr<DescBase> &desc,
               SharedPtr<CompBase> &previous_component);

            /**
             * Outputs the specified value or its name to the stream.
             */
            virtual void output_value(
               std::ostream &out,
               int2 value,
               bool translate);
            virtual void output_value(
               std::wostream &out,
               int2 value,
               bool translate);

            /**
             * Inputs the specified value or its name from the input stream.
             */
            virtual void input_value(
               std::istream &in,
               int2 &value,
               bool translate,
               byte minor_version);

            /**
             * @return Returns the iterator to the first choice.
             */
            typedef values_type::const_iterator const_iterator;
            const_iterator begin() const
            { return values.begin(); }

            /**
             * @return Returns an iterator beyond the end of the last choice.
             */
            const_iterator end() const
            { return values.end(); }

            /**
             * @return Returns true if there are no choices.
             */
            bool empty() const
            { return values.empty(); }

            /**
             * @return Returns a reference to the first choice.
             */
            value_type const &front() const
            { return values.front(); }

            /**
             * @return Returns an iterator to the choice associated with the specified value.
             */
            const_iterator find(int2 value) const
            { return std::find_if(values.begin(), values.end(), item_has_val<int2>(value)); }
         };


         /**
          * Defines a component object that uses an enumeration of possible int2 values.
          */
         class CompEnumI2: public CompBase
         {
         protected:
            /**
             * Specifies the current value.
             */
            int2 value;

            /**
             * Specifies the description
             */
         public:
            typedef CompEnumI2Desc desc_type;
            typedef PolySharedPtr<DescBase, desc_type> desc_handle;
         protected:
            desc_handle desc;

         public:
            /**
             * Constructor
             */
            CompEnumI2(SharedPtr<DescBase> &desc):
               CompBase(desc)
            {
               this->desc = desc;
               if(!this->desc->empty())
                  value = this->desc->front().value;
            }

            /**
             * @return Returns the descrition.
             */
            desc_handle &get_desc()
            { return desc; }
            
            /**
             * @return Returns the current value.
             */
            int4 get_value() const
            { return value; }

            /**
             * Overloads the base class version to read the value from the stream.
             */
            virtual void read(SharedPtr<Message> &message)
            { value = message->readInt2(); }

            /**
             * Overloads the base class version to write the value to the stream.
             */
            virtual void write(SharedPtr<Message> &message)
            { message->addInt2(value); }

            /**
             * Overloads the base class version to read the value or its name from the stream.
             */
            virtual void output(std::ostream &out, bool translate)
            { desc->output_value(out,value,translate); }
            virtual void output(std::wostream &out, bool translate)
            { desc->output_value(out,value,translate); }

            /**
             * Overloads the base class version to write the value or its name to the stream.
             */
            virtual void input(std::istream &in, bool translate)
            {
               desc->input_value(in, value, translate, minor_version);
               has_changed = true;
            }

            /**
             * Overloads the base class version to write the value to the array.
             */
            virtual void write(Json::Array &json)
            { json.push_back(new Json::Number(value)); }

            /**
             * Overloads the base class version to read the value from the specified array iterator.
             */
            virtual Json::Array::iterator read(Json::Array::iterator current)
            {
               Json::NumberHandle value_json(*current);
               value = (int2)value_json->get_value();
               return ++current;
            }
            
            /**
             * @return Returns the value.
             */
            virtual int2 get_val_int2()
            { return value; }

            /**
             * Overloads the base class version to set the value.
             */
            virtual void set_val_int2(int2 val);

            /**
             * @return Returns the value as four byte signed integer.
             */
            virtual int4 get_val_int4()
            { return value; }

            /**
             * @param val Specifies the value as a four byte signed integer.
             */
            virtual void set_val_int4(int4 val)
            { set_val_int2(static_cast<int2>(val)); }

            /**
             * @return Returns the value as a float.
             */
            virtual float get_val_float()
            { return value; }

            /**
             * @param val Specifies the value as a float.
             */
            virtual void set_val_float(float val)
            { set_val_int2(static_cast<int2>(val)); }

            /**
             * @return Returns the value as a double.
             */
            virtual double get_val_double()
            { return value; }

            /**
             * @param val Specifies the value as a double.
             */
            virtual void set_val_double(double val)
            { set_val_int2(static_cast<int2>(val)); }

         protected:
            /**
             * Overloads the base class version to perform the copy.
             */
            virtual void do_copy(CompBase *other_);
         };
      };
   };
};


#endif

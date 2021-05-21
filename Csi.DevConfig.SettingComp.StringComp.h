/* Csi.DevConfig.SettingComp.StringComp.h

   Copyright (C) 2004, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 12 January 2004
   Last Change: Tuesday 03 December 2019
   Last Commit: $Date: 2019-12-03 17:02:16 -0600 (Tue, 03 Dec 2019) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Csi_DevConfig_SettingComp_StringComp_h
#define Csi_DevConfig_SettingComp_StringComp_h

#include <list>
#include "Csi.DevConfig.SettingComp.DescBase.h"
#include "Csi.DevConfig.SettingComp.CompBase.h"
#include "Csi.PolySharedPtr.h"
#include "Csi.MsgExcept.h"


namespace Csi
{
   namespace DevConfig
   {
      namespace SettingComp
      {
         /**
          * Defines a component description class for a string based component.
          */
         class StringCompDesc: public DescBase
         {
         protected:
            /**
             * Specifies the maximum length, in bytes, for the string.
             */
            uint4 max_len;

            /**
             * Specifies a regular expression against which any input will be validated.  If this
             * value is empty, any input within the specified length will be accepted.
             */
            StrAsc validate_pattern;

            /**
             * Set to true if the control used for editing this component should be a multi-line
             * control.
             */
            bool multi_line;

            /**
             * Set to true if the string should have password semantics.
             */
            bool password;

            /**
             * Set to true if the password needs confirmation.
             */
            bool password_needs_confirm;

            /**
             * Set to true if the editor should provided a browse button that will allow the user to
             * selevt the contents of a file as the value of the component.
             */
            bool supports_browse;
            
         public:
            /**
             * Constructor
             *
             * @param component_type Specifies the type code for this component.  If not specified,
             * it will default to a string component type.
             */
            StringCompDesc(uint4 component_type = Components::comp_string):
               DescBase(component_type),
               max_len(0),
               multi_line(false),
               password(false),
               supports_browse(false),
               password_needs_confirm(true)
            { }

            /**
             * Destructor
             */
            virtual ~StringCompDesc()
            { }

            /**
             * @return Overloads the base class to generate a string component type.
             */
            virtual CompBase *make_component(
               SharedPtr<DescBase> &desc,
               SharedPtr<CompBase> &previous_component);

            /**
             * Overloads the base class version to read description properties from the XML
             * structure.
             */
            virtual void init_from_xml(
               Csi::Xml::Element &xml_data,
               StrAsc const &library_dir)
            {
               DescBase::init_from_xml(xml_data,library_dir);
               max_len = xml_data.get_attr_uint4(L"length");
               validate_pattern = xml_data.get_attr_str(L"validate");
               if(xml_data.has_attribute(L"multi-line"))
                  multi_line = xml_data.get_attr_bool(L"multi-line");
               if(xml_data.has_attribute(L"password"))
               {
                  password = xml_data.get_attr_bool(L"password");
                  if(xml_data.has_attribute(L"password-needs-confirm"))
                     password_needs_confirm = xml_data.get_attr_bool(L"password-needs-confirm");
               }
               if(xml_data.has_attribute(L"browse"))
                  supports_browse = xml_data.get_attr_bool(L"browse");
            }

            /**
             * @return Returns the maximum length limit of the string in bytes.
             */
            uint4 get_max_len() const
            { return max_len; }

            /**
             * @return Returns true if the specified null terminated string is considered valid
             * according to length limits and the validattion regular expression if supplied.
             *
             * @param s Specifies the string to validate.
             */
            bool is_valid_input(char const *s);

            /**
             * @return Returns true if the editor should allow the string to be presented with
             * multiple lines of text.
             */
            bool get_multi_line() const
            { return multi_line; }

            /**
             * @return Returns true if the editor should have password semantics.
             */
            virtual bool get_password() const
            { return password; }

            /**
             * @return Returns true if this component is a password and needs confirmation.
             */
            virtual bool get_password_needs_confirm() const
            { return password && password_needs_confirm; }

            /**
             * @return Returns true if the editor should have a button that the user can click to
             * specify a file that can be used to fill the string.
             */
            virtual bool get_supports_browse() const
            { return supports_browse; }

            /**
             * @return Returns true if the component specifies a value that should not be exposed.
             */
            virtual bool has_protected_value() const
            { return password; }

            /**
             * Overloads the base class version to write fields for this description type.
             */
            virtual void describe_json(Csi::Json::Object &desc)
            {
               DescBase::describe_json(desc);
               desc.set_property_number("max_len", max_len);
               desc.set_property_str("validate_pattern", validate_pattern);
               desc.set_property_bool("multi_line", multi_line);
               desc.set_property_bool("password", password);
               desc.set_property_bool("password_needs_confirm", password_needs_confirm);
               desc.set_property_bool("supports_browse", supports_browse);
            }
         };


         /**
          * Defines an object the represents a utf-8 encoded nul terminated string.
          */
         class StringComp: public CompBase
         {
         protected:
            /**
             * Specifies the value for the component.
             */
            StrAsc value;

            /**
             * Reference to the description.
             */
            PolySharedPtr<DescBase, StringCompDesc> desc;
            
         public:
            /**
             * Defines the description type for this component.
             */
            typedef StringCompDesc desc_type;
            
            /**
             * Constructor
             *
             * @param desc_ Specifies the component description.
             */
            StringComp(SharedPtr<DescBase> &desc_):
               CompBase(desc_),
               desc(desc_)
            { }

            /**
             * Overloads the base class version to read the value from the message.
             */
            virtual void read(SharedPtr<Message> &in)
            { in->readAsciiZ(value); }

            /**
             * Overloads the base class version to write the value to the message.
             */
            virtual void write(SharedPtr<Message> &out)
            {
               if(value.length() > desc->get_max_len())
                  value.cut(desc->get_max_len() - 1);
               out->addAsciiZ(value.c_str());
            }

            /**
             * Overloads the base class version to write the component value to the stream.
             */
            virtual void output(std::ostream &out, bool translate)
            {
               out << value;
               if(!translate)
                  out << "\n";
            }
            virtual void output(std::wostream &out, bool translate)
            {
               out << value;
               if(!translate)
                  out << L"\n";
            }

            /**
             * Overloads the base class version to read the string component from the stream.
             */
            virtual void input(std::istream &in, bool translate);

            /**
             * Overloads the base class version to add the string value to the specified JSON array.
             */
            virtual void write(Json::Array &json)
            { json.push_back(new Json::String(value)); }

            /**
             * Overloads the base class version to load the string value from the current iterator
             * and return the next.
             */
            virtual Json::Array::iterator read(Json::Array::iterator current)
            {
               Json::StringHandle value_json(*current);
               value = value_json->get_value();
               return ++current;
            }
            
            /**
             * Overloads the base class version to return the string value.
             */
            virtual StrAsc get_val_str()
            { return value; }

            /**
             * Overloads the base class version to set the string value.
             */
            virtual void set_val_str(StrAsc const &val, bool apply_checks = false);

            /**
             * Overloads the base class version to parse the string as a single byte integer.
             */
            virtual byte get_val_byte();

            /**
             * Overloads the base class version to parse the string as a two byte integer.
             */
            virtual int2 get_val_int2();

            /**
             * Overloads the base class version to parse the string as a two byte integer.
             */
            virtual uint2 get_val_uint2();

            /**
             * Overloads the base class version to parse the string as a four byte integer.
             */
            virtual int4 get_val_int4();

            /**
             * Overloads the base class version to parse the string as a floating point number.
             */
            virtual float get_val_float();

            /**
             * Overloads the base class version to parse the string as a floating point number.
             */
            virtual double get_val_double();

            /**
             * Overloads the base class version to format the value.
             */
            virtual void set_val_byte(byte v);

            /**
             * Overloads the base class version to format the value.
             */
            virtual void set_val_int2(int2 v);

            /**
             * Overloads the base class version to format the value.
             */
            virtual void set_val_uint2(uint2 v);

            /**
             * Overloads the base class version to format the value.
             */
            virtual void set_val_int4(int4 v);

            /**
             * Overloads the base class version to format the value.
             */
            virtual void set_val_uint4(uint4 v);

            /**
             * Overloads the base class version to format the value.
             */
            virtual void set_val_float(float v);

            /**
             * Overloads the base class version to format the value.
             */
            virtual void set_val_double(double v);

            /**
             * @return Returns the maximum length allowed in the description.
             */
            uint4 get_max_len() const
            { return desc->get_max_len(); }

            /**
             * @return Returns true if this component should be treated as a password.
             */
            bool get_password() const
            { return desc->get_password(); }

            /**
             * @return Returns true if the component editor should have a file browse button.
             */
            bool get_supports_browse() const
            { return desc->get_supports_browse(); }
            
         protected:
            /**
             * Overloads the base class version to copy this component type.
             */
            virtual void do_copy(CompBase *other_)
            {
               StringComp *other = static_cast<StringComp *>(other_);
               value = other->value;
            }
         };


         /**
          * Definition of the method.
          */
         inline CompBase *StringCompDesc::make_component(
            SharedPtr<DescBase> &desc,
            SharedPtr<CompBase> &previous_component)
         { return new StringComp(desc); }


         /**
          * Defines an item that is available for a choice control.
          */
         class ChoiceItem
         {
         public:
            /**
             * Specifies the name for this item  that will be presented to the user.
             */
            StrAsc name;

            /**
             * Specifies the value for this item.
             */
            StrAsc value;

            /**
             * Specifies the minimum minor version in which this choice is expected to be supported.
             */
            byte min_minor;

            /**
             * Constructor
             *
             * @param value_ Specifies the value for this choice.
             *
             * @param name_ Specifies the name presented to the user.
             *
             * @param min_minor_ Specifies the minimum minor version.
             */
            ChoiceItem(
               StrAsc const &value_ = "",
               StrAsc const &name_ = "",
               byte min_minor_ = 0):
               name(name_),
               value(value_),
               min_minor(min_minor_)
            { }

            /**
             * Copy contructor
             */
            ChoiceItem(ChoiceItem const &other):
               name(other.name),
               value(other.value),
               min_minor(other.min_minor)
            { }

            /**
             * Copy operator
             */
            ChoiceItem &operator =(ChoiceItem const &other)
            {
               name = other.name;
               value = other.value;
               min_minor = other.min_minor;
               return *this;
            }
         };

         
         /**
          * Defines the description class for a choice component type.
          */
         class ChoiceCompDesc: public StringCompDesc
         {
         private:
            /**
             * Set to true if the current value should be accepted even if it is not a defined
             * choice.
             */
            bool accept_current_value;
            
         public:
            /**
             * Constructor
             */
            ChoiceCompDesc():
               StringCompDesc(Components::comp_choice),
               accept_current_value(false)
            { }

            /**
             * Destructor
             */
            virtual ~ChoiceCompDesc()
            { }

            /**
             * Overloads the base class to make the component.
             */
            virtual CompBase *make_component(
               SharedPtr<DescBase> &desc,
               SharedPtr<CompBase> &previous_component);

            /**
             * Overloads the base class to initialise this description from the specified XML
             * structure.
             */
            virtual void init_from_xml(
               Csi::Xml::Element &xml_data,
               StrAsc const &library_dir);

            /**
             * Specifies the collection of choices available for the component.
             */
            typedef ChoiceItem value_type;
            typedef std::deque<value_type> values_type;
            values_type values;

            /**
             * Outputs the specified value to the specified stream.
             */
            virtual void output_value(
               std::ostream &out,
               StrAsc const &value,
               bool translate);

            /**
             * Reads the specified value from the specified stream.
             */
            virtual void input_value(
               std::istream &in,
               StrAsc &value,
               bool translate);

            /**
             * @return Returns true if there are no choices available.
             */
            bool empty() const
            { return values.empty(); }

            /**
             * @return Returns the first iterator for the list of choices.
             */
            typedef values_type::const_iterator const_iterator;
            const_iterator begin() const
            { return values.begin(); }

            /**
             * @return Returns the last iterator for the container of choices.
             */
            const_iterator end() const
            { return values.end(); }

            /**
             * @return Returns the first choice.
             */
            value_type const &front() const
            { return values.front(); }

            /**
             * @return Returns true if the application should accept the current value even if it is
             * not a defined choice.
             */
            bool get_accept_current_value() const
            { return accept_current_value; }
         };


         /**
          * Defines an object that allows the user to set the value from a list of choices.
          */
         class ChoiceComp: public StringComp
         {
         protected:
            /**
             * Specifies the description.
             */
            PolySharedPtr<DescBase, ChoiceCompDesc> desc;

         public:
            /**
             * Specifies the type of the description.
             */
            typedef ChoiceCompDesc desc_type;
            
            /**
             * Constructor
             */
            ChoiceComp(SharedPtr<DescBase> &desc_):
               desc(desc_),
               StringComp(desc_)
            {
               if(!desc->empty())
                  value = desc->front().value;
            }

            /**
             * Destructor
             */
            virtual ~ChoiceComp()
            { }

            /**
             * Overloads the base class version to write the component value to the stream.
             */
            virtual void output(std::ostream &out, bool translate)
            { desc->output_value(out,value,translate); }

            /**
             * Overloads the base class version to read the component value from the stream.
             */
            virtual void input(std::istream &in, bool translate)
            {
               desc->input_value(in,value,translate);
               has_changed = true;
            }

            /**
             * Sets the value as an integer by selecting the item indexed by the integer.
             */
            virtual void set_val_int4(int4 val)
            {
               if(val < std::distance(desc->begin(), desc->end()))
               {
                  desc_type::value_type const &item(*(desc->begin() + val));
                  value = item.value;
                  has_changed = true;
               }
            }
         };


         /**
          * Definition of the make_component method.
          */
         inline CompBase *ChoiceCompDesc::make_component(
            SharedPtr<DescBase> &desc,
            SharedPtr<CompBase> &previous_component)
         { return new ChoiceComp(desc); }
      };
   };
};


#endif

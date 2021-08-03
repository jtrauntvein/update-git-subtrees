/* Csi.DevConfig.SettingComp.UrlAddress.h

   Copyright (C) 2013, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Thursday 21 November 2013
   Last Change: Tuesday 03 December 2019
   Last Commit: $Date: 2019-12-03 17:02:16 -0600 (Tue, 03 Dec 2019) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_DevConfig_SettingComp_UrlAddress_h
#define Csi_DevConfig_SettingComp_UrlAddress_h

#include "Csi.DevConfig.SettingComp.CompBase.h"
#include "Csi.DevConfig.SettingComp.DescBase.h"
#include "Csi.BuffStream.h"
#include "Csi.StrAscStream.h"


namespace Csi
{
   namespace DevConfig
   {
      namespace SettingComp
      {
         /**
          * Defines an object that acts as a description for a component that allows the user to
          * enter an IPv4 address, an IPv6 address or a domain address.
          */
         class UrlAddressDesc: public DescBase
         {
         public:
            /**
             * Constructor
             */
            UrlAddressDesc():
               DescBase(Components::comp_url_address),
               max_length(255),
               min_length(0)
            { }

            /**
             * @return Overloads the base class version to create the component.
             */
            virtual CompBase *make_component(
               SharedPtr<DescBase> &desc, SharedPtr<CompBase> &previous);

            /**
             * @return Overloads the base class version to indicate that there is no min or max
             * value.
             */
            virtual bool has_maxima() const
            { return false; }

            /**
             * @return Returns the minimum length.
             */
            uint4 get_min_length() const
            { return min_length; }

            /**
             * @return Returns the maximum length.
             */
            uint4 get_max_length() const
            { return max_length; }

            /**
             * Overloads the base class version to initialise properties from the specified XML
             * structure.
             */
            virtual void init_from_xml(
               Xml::Element &xml_data, StrAsc const &library_dir)
            {
               DescBase::init_from_xml(xml_data, library_dir);
               if(xml_data.has_attribute(L"max-length"))
                  max_length = xml_data.get_attr_uint4(L"max-length");
               if(xml_data.has_attribute(L"min-length"))
                  min_length = xml_data.get_attr_uint4(L"min-length");
            }

            /**
             * Overloads the base class version to write specific properties.
             */
            virtual void describe_json(Csi::Json::Object &desc)
            {
               DescBase::describe_json(desc);
               desc.set_property_number("max_length", max_length);
               desc.set_property_number("min_length", min_length);
            }
            
         private:
            /**
             * Specifies the maximum length.
             */
            uint4 max_length;

            /**
             * Specifies the minimum length.
             */
            uint4 min_length;
         };


         /**
          * Defines a setting component object that allows the user to enter an IPv4, IPv6, or
          * domain address.
          */
         class UrlAddress: public CompBase
         {
         protected:
            /**
             * Specifies the current value for this component.
             */
            StrAsc value;

         public:
            /**
             * Constructor
             */
            UrlAddress(SharedPtr<DescBase> &desc_):
               CompBase(desc_)
            { }

            /**
             * Overloads the base class version to read the value from the specified message.
             */
            virtual void read(SharedPtr<Message> &message)
            { message->readAsciiZ(value); }

            /**
             * Overloads the base class version to write the value to the specified message.
             */
            virtual void write(SharedPtr<Message> &message)
            { message->addAsciiZ(value.c_str()); }

            /**
             * Overloads the base class version to add the value to the specified components array.
             */
            virtual void write(Json::Array &json)
            { json.push_back(new Json::String(value)); }

            /**
             * Overloads the base class to read the value from the array iterator.
             */
            virtual Json::Array::iterator read(Json::Array::iterator current)
            {
               Json::StringHandle value_json(*current);
               value = value_json->get_value();
               return ++current;
            }
            
            /**
             * Overloads the base class version to format the value to the specified stream.
             */
            virtual void output(std::ostream &out, bool translate)
            { out << value; }
            virtual void output(std::wostream &out, bool translate)
            { out << value; }

            /**
             * Overloads the base class version to parse the value from the specified stream.
             */
            virtual void input(std::istream &in, bool translate);

            /**
             * Overloads the base class version to copy the component.
             */
            virtual void do_copy(CompBase *other_)
            {
               UrlAddress *other(static_cast<UrlAddress *>(other_));
               value = other->value;
            }

            virtual StrAsc get_value_str()
            { return value; }

            virtual void set_val_str(StrAsc const &val, bool apply_checks = false);
         };
      };
   };
};


#endif

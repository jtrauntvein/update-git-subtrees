/* Csi.DevConfig.SettingComp.CompIpAddr.h

   Copyright (C) 2005, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 02 June 2005
   Last Change: Tuesday 03 December 2019
   Last Commit: $Date: 2019-12-03 17:02:16 -0600 (Tue, 03 Dec 2019) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Csi_DevConfig_SettingComp_CompIpAddr_h
#define Csi_DevConfig_SettingComp_CompIpAddr_h

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
          * Defines a description object for a component that represents an IPv4 address either as a
          * string or as a uint4 value.
          */
         class CompIpAddrDesc: public DescBase
         {
         public:
            /**
             * Constructor
             */
            CompIpAddrDesc():
               DescBase(Components::comp_ipaddr),
               as_string(false)
            { }

            /**
             * Overloads the base class version to create the component.
             */
            virtual CompBase *make_component(
               SharedPtr<DescBase> &desc,
               SharedPtr<CompBase> &previous_component);

            /**
             * @return Returns false to indicate that there is no maximum value.
             */
            virtual bool has_maxima() const
            { return false; }

            /**
             * @return Returns true if the component must be formatted as a string to be sent to
             * the device.
             */
            bool get_as_string() const
            { return as_string; }

            /**
             * Initialises this description from the specified XML structure.
             */
            virtual void init_from_xml(
               Xml::Element &xml_data, StrAsc const &library_dir)
            {
               DescBase::init_from_xml(xml_data, library_dir);
               if(xml_data.has_attribute(L"as-string"))
                  as_string = xml_data.get_attr_bool(L"as-string");
               else
                  as_string = false;
            }

            /**
             * Overloads the base class version to write the description fields to the specified
             * object.
             */
            virtual void describe_json(Csi::Json::Object &desc)
            {
               DescBase::describe_json(desc);
               desc.set_property_bool("as_string", as_string);
            }
            
         private:
            /**
             * Set to true if the component must send its value as a string.
             */
            bool as_string;
         };


         /**
          * Defines a component object that represents an IPv4 address either as a string or as a
          * uint4.
          */
         class CompIpAddr: public CompBase
         {
         protected:
            /**
             * Specifies the current value.
             */
            uint4 value;

         public:
            /**
             * Constructor
             */
            CompIpAddr(
               SharedPtr<DescBase> &desc_):
               CompBase(desc_),
               value(0)
            { }

            /**
             * Overloads the base class version to read the value from the message.
             */
            virtual void read(SharedPtr<Message> &in)
            {
               CompIpAddrDesc *temp(static_cast<CompIpAddrDesc *>(get_desc().get_rep()));
               if(!temp->get_as_string())
                  value = in->readUInt4();
               else
               {
                  StrAsc str;
                  in->readAsciiZ(str);
                  Csi::IBuffStream val_str(str.c_str(), str.length());
                  input(val_str, true);
               }
            }

            /**
             * Overloads the base class version to write the value to the message.
             */
            virtual void write(SharedPtr<Message> &out)
            {
               CompIpAddrDesc *temp(static_cast<CompIpAddrDesc *>(get_desc().get_rep()));
               if(!temp->get_as_string())
                  out->addUInt4(value);
               else
               {
                  Csi::OStrAscStream str;
                  output(str, true);
                  out->addAsciiZ(str.str().c_str());
               }
            }

            /**
             * Overloads the base class version to write the value to the stream.
             */
            virtual void output(std::ostream &out, bool translate);
            virtual void output(std::wostream &out, bool translate);

            /**
             * Overloads the base class version to read the value from the stream.
             */
            virtual void input(std::istream &in, bool translate);

            /**
             * Overloads the base class version to add the value to the components array.
             */
            virtual void write(Json::Array &json)
            {
               CompIpAddrDesc *temp(static_cast<CompIpAddrDesc *>(get_desc().get_rep()));
               if(!temp->get_as_string())
                  json.push_back(new Json::Number(value));
               else
               {
                  Csi::OStrAscStream temp;
                  output(temp, true);
                  json.push_back(new Json::String(temp.str()));
               }
            }

            /**
             * Overloads the base class version to load the value from the specified array iterator.
             */
            virtual Json::Array::iterator read(Json::Array::iterator current)
            {
               CompIpAddrDesc *temp(static_cast<CompIpAddrDesc *>(get_desc().get_rep()));
               if(!temp->get_as_string())
               {
                  Json::NumberHandle value_json(*current);
                  value = (uint4)value_json->get_value();
               }
               else
               {
                  Json::StringHandle value_str(*current);
                  IBuffStream temp(value_str->get_value().c_str(), value_str->get_value().length());
                  input(temp, true);
               }
               return ++current;
            }
            
            /**
             * Overloads the base class version to copy the value.
             */
            virtual void do_copy(CompBase *other_)
            {
               CompIpAddr *other = static_cast<CompIpAddr *>(other_);
               value = other->value;
            }

            virtual uint4 get_val_uint4()
            { return value; }

            virtual void set_val_uint4(uint4 value_)
            {
               value = value_;
               has_changed = true;
            }
         };
      };
   };
};


#endif

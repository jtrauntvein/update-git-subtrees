/* Csi.DevConfig.SettingComp.CompIpAddr6.h

   Copyright (C) 2005, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 02 June 2005
   Last Change: Tuesday 03 December 2019
   Last Commit: $Date: 2012-07-13 13:45:59 -0600 (Fri, 13 Jul 2012) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Csi_DevConfig_SettingComp_CompIpAddr6_h
#define Csi_DevConfig_SettingComp_CompIpAddr6_h

#include "Csi.DevConfig.SettingComp.CompBase.h"
#include "Csi.DevConfig.SettingComp.DescBase.h"
#include "Csi.BuffStream.h"
#include "Csi.StrAscStream.h"
#include "Csi.SocketAddress.h"


namespace Csi
{
   namespace DevConfig
   {
      namespace SettingComp
      {
         /**
          * Defines an object that implements a description for a component that represents an IPv6
          * address.
          */
         class CompIpAddr6Desc: public DescBase
         {
         public:
            /**
             * Constructor
             */
            CompIpAddr6Desc():
               DescBase(Components::comp_ipaddr6),
               as_string(false),
               include_net_size(0)
            { }

            /**
             * @return Overloads the base class method to create an IPv6 component.
             */
            virtual CompBase *make_component(
               SharedPtr<DescBase> &desc,
               SharedPtr<CompBase> &previous_component);

            /**
             * @return Overloads the base class version to indicate that there is no maxima.
             */
            virtual bool has_maxima() const
            { return false; }

            /**
             * @return Returns true if the component is supposed to be transmitted as a string.
             */
            bool get_as_string() const
            { return as_string; }

            /**
             * @return Returns true if the address is supposed to include the network size.
             */
            bool get_include_net_size() const
            { return include_net_size; }

            /**
             * Overloads the base class version to initialise properties for this description from
             * the XML structure.
             */
            virtual void init_from_xml(
               Xml::Element &xml_data, StrAsc const &library_dir)
            {
               DescBase::init_from_xml(xml_data, library_dir);
               if(xml_data.has_attribute(L"as-string"))
                  as_string = xml_data.get_attr_bool(L"as-string");
               else
                  as_string = false;
               if(xml_data.has_attribute(L"include-net-size"))
                  include_net_size = xml_data.get_attr_bool(L"include-net-size");
               else
                  include_net_size = false;
            }

            /**
             * Overloads the base class version to write the properties to a JSON object.
             */
            virtual void describe_json(Csi::Json::Object &desc)
            {
               DescBase::describe_json(desc);
               desc.set_property_bool("as_string", as_string);
               desc.set_property_bool("include_net_size", include_net_size);
            }

         private:
            /**
             * Set to true if the component is supposed to be transmitted as a string.
             */
            bool as_string;

            /**
             * Set to true if the component is supposed to include the net size.
             */
            bool include_net_size;
         };


         /**
          * Defines a component object that represents an IPv6 address.
          */
         class CompIpAddr6: public CompBase
         {
         protected:
            /**
             * Specifies the current value.
             */
            SocketAddress value;

            /**
             * Specifies the net size.
             */
            uint4 net_size;

         public:
            /**
             * Constructor
             */
            CompIpAddr6(
               SharedPtr<DescBase> &desc_):
               CompBase(desc_),
               net_size(64)
            { }

            /**
             * Overloads the base class version to read the address.
             */
            virtual void read(SharedPtr<Message> &in);

            /**
             * Overloads the base class version to write the address.
             */
            virtual void write(SharedPtr<Message> &out);

            /**
             * Overloads the base class version to format the address to the stream.
             */
            virtual void output(std::ostream &out, bool translate);
            virtual void output(std::wostream &out, bool translate);

            /**
             * Overloads the base class version to add the value to the JSON array.
             */
            virtual void write(Json::Array &json);

            /**
             * Overloads the base class version to extract the value(s) from the JSON array
             * iterator.
             */
            virtual Json::Array::iterator read(Json::Array::iterator current);

            /**
             * Overloads the base class version to parse the address from the stream.
             */
            virtual void input(std::istream &in, bool translate);

            /**
             * Overloads the base class version to copy the address.
             */
            virtual void do_copy(CompBase *other_)
            {
               CompIpAddr6 *other = static_cast<CompIpAddr6 *>(other_);
               value = other->value;
               net_size = other->net_size;
            }

            /**
             * @param address Sets the current value for this component.
             */
            void set_val_address(Csi::SocketAddress const &address)
            {
               if(address.get_family() == Csi::SocketAddress::family_ipv6)
                  value = address;
               else
                  throw std::invalid_argument("invalid socket address");
            }

            /**
             * @return Returns the current value for this component.
             */
            Csi::SocketAddress const &get_val_address() const
            { return value; }

            /**
             * @param size Specifies the value of the network size.
             */
            void set_net_size(uint4 size)
            {
               if(size <= 128)
                  net_size = size;
               else
                  throw std::invalid_argument("invalid net size");
            }

            /**
             * @return Returns the current value of the network size.
             */
            uint4 get_net_size() const
            { return net_size; }

            /**
             * @return Returns true if the component should send the value as a string.
             */
            bool get_as_string() const
            {
               CompIpAddr6Desc const *temp(static_cast<CompIpAddr6Desc const *>(desc.get_rep()));
               return temp->get_as_string();
            }

            /**
             * @param Returns true if the network size should be sent.
             */
            bool get_include_net_size() const
            {
               CompIpAddr6Desc const *temp(static_cast<CompIpAddr6Desc const *>(desc.get_rep()));
               return temp->get_include_net_size();
            }
         };
      };
   };
};


#endif

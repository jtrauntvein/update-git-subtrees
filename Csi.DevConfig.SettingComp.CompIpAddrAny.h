/* Csi.DevConfig.SettingComp.CompIpAddrAny.h

   Copyright (C) 2019, Campbell Scientific, Inc

   Written by: Jon Trauntvein 
   Date Begun: Friday 08 March 2019
   Last Change: Friday 08 March 2019
   Last Commit: $Date: 2019-03-11 11:39:36 -0600 (Mon, 11 Mar 2019) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Csi_DevConfig_SettingComp_CompIpAddrAny_h
#define Csi_DevConfig_SettingComp_CompIpAddrAny_h
#include "Csi.DevConfig.SettingComp.CompBase.h"
#include "Csi.DevConfig.SettingComp.DescBase.h"


namespace Csi
{
   namespace DevConfig
   {
      namespace SettingComp
      {
         /**
          * Defines a description object for the component that can represent either an IPv4 or IPv6
          * or no address.
          */
         class CompIpAddrAnyDesc: public DescBase
         {
         public:
            /**
             * Constructor
             */
            CompIpAddrAnyDesc():
               DescBase(Components::comp_ipaddr_any)
            { }

            /**
             * Overloads the base class version to create the component.
             */
            virtual CompBase *make_component(SharedPtr<DescBase> &desc, SharedPtr<CompBase> &previous);

            /**
             * Overloads the base class version to indicate that there is no min or max value.
             */
            virtual bool has_maxima() const
            { return false; }
         };


         /**
          * Defines a component object that represents an IPv4 or IPv6 address as a string.
          */
         class CompIpAddrAny: public CompBase
         {
         protected:
            /**
             * Specifies the value.
             */
            StrAsc value;

         public:
            /**
             * Constructor
             */
            CompIpAddrAny(SharedPtr<DescBase> &desc_):
               CompBase(desc_)
            { }

            /**
             * Overloads the base class version to read the content from the message.
             */
            virtual void read(SharedPtr<Message> &in)
            {
               StrAsc temp;
               in->readAsciiZ(temp);
               Csi::IBuffStream val_str(temp.c_str(), temp.length());
               input(val_str, true);
            }

            /**
             * Overloads the base class version to write the content to the message.
             */
            virtual void write(SharedPtr<Message> &out)
            {
               out->addAsciiZ(value.c_str());
            }

            /**
             * Overloads the vase class version to write the value to the specified stream.
             */
            virtual void output(std::ostream &out, bool translate)
            { out << value; }
            virtual void output(std::wostream &out, bool translate)
            { out << value; }
            
            /**
             * Overloads the base class version to read the value from the specified stream.
             */
            virtual void input(std::istream &in, bool translate);

            /**
             * Overloads the base class version to add the value to the components array.
             */
            virtual void write(Json::Array &json)
            {
               json.push_back(new Json::String(value));
            }

            /**
             * Overloads the base class version to extract the value from the array member.
             */
            virtual Json::Array::iterator read(Json::Array::iterator current)
            {
               Json::StringHandle value_str(*current);
               IBuffStream temp(value_str->get_value().c_str(), value_str->get_value().length());
               input(temp, true);
               return ++current;
            }

            /**
             * Overloads the base class version to copy the value.
             */
            virtual void do_copy(CompBase *other_)
            {
               CompIpAddrAny *other(static_cast<CompIpAddrAny *>(other_));
               value = other->value;
            }

            /**
             * Overloads the string insertion.
             */
            virtual void set_val_str(StrAsc const &value_, bool apply_checks = false)
            {
               if(apply_checks)
                  CompBase::set_val_str(value_, apply_checks);
               else
               {
                  value = value_;
                  has_changed = true;
               }
            }

            /**
             * Overloads the accessor.
             */
            virtual StrAsc get_val_str()
            { return value; }
         };
      };
   };
};

#endif

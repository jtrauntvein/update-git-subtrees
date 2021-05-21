/* Csi.DevConfig.SettingComp.CompSsid.h

   Copyright (C) 2012, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Thursday 01 March 2012
   Last Change: Tuesday 03 December 2019
   Last Commit: $Date: 2019-12-03 17:02:16 -0600 (Tue, 03 Dec 2019) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_DevConfig_SettingComp_CompSsid_h
#define Csi_DevConfig_SettingComp_CompSsid_h

#include "Csi.DevConfig.SettingComp.CompBase.h"
#include "Csi.DevConfig.SettingComp.DescBase.h"
#include "Csi.PolySharedPtr.h"


namespace Csi
{
   namespace DevConfig
   {
      namespace SettingComp
      {
         /**
          * Defines an object that describes an SSID setting.
          */
         class CompSsidDesc: public DescBase
         {
         private:
            /**
             * Specifies the identifier for the setting that will provided the pick list.
             */
            uint2 pick_id;

            /**
             * Specifies the zero based offset of the component in the pick setting that holds the
             * service name.
             */
            uint4 ssid_pos;

            /**
             * Specifies the zero based offset of the component in the pick setting that holds the
             * receive power in dBm.
             */
            uint4 dbm_pos;

            /**
             * Specifies the zero based offset of the component in the pick setting that holds the
             * flag that will indicate whether the service is encrypted.
             */
            uint4 encrypted_pos;
            
         public:
            /**
             * Constructor
             */
            CompSsidDesc();

            /**
             * @return Returns the identifier for the setting that will provide the pick list.
             */
            uint2 get_pick_id() const
            { return pick_id; }

            /**
             * @return Returns the zero based offset for the description component that reports the
             * SSID.
             */
            uint4 get_ssid_pos() const
            { return ssid_pos; }

            /**
             * @return Returns the zero based offset for the description component that reports the
             * signal strength in dBm.
             */
            uint4 get_dbm_pos() const
            { return dbm_pos; }

            /**
             * @return Returns the zero based offset for the description component that reports
             * whether the network is encrypted.
             */
            uint4 get_encrypted_pos() const
            { return encrypted_pos; }

            /**
             * @return Overloads the base class version to create the component.
             */
            virtual CompBase *make_component(
               SharedPtr<DescBase> &desc,
               SharedPtr<CompBase> &previous_component);

            /**
             * Overloads the base class version to initialise this description from the XML
             * structure.
             */
            virtual void init_from_xml(
               Xml::Element &xml_data, StrAsc const &library_dir);

            /**
             * Overloads the base class to write specific meta-fields.
             */
            virtual void describe_json(Csi::Json::Object &desc)
            {
               DescBase::describe_json(desc);
               desc.set_property_number("pick_id", pick_id);
               desc.set_property_number("ssid_pos", ssid_pos);
               desc.set_property_number("dbm_pos", dbm_pos);
               desc.set_property_number("encrypted_pos", encrypted_pos);
            }
         };


         /**
          * Defines a component object that represents a Wifi SSID.
          */
         class CompSsid: public CompBase
         {
         private:
            /**
             * Specifies the current value for this component.
             */
            StrAsc value;

            /**
             * Specifies the description.
             */
            PolySharedPtr<DescBase, CompSsidDesc> desc;
            
         public:
            /**
             * Constructor
             */
            CompSsid(SharedPtr<DescBase> &desc_):
               CompBase(desc_),
               desc(desc_)
            { }

            /**
             * Destructor
             */
            virtual ~CompSsid()
            { }

            /**
             * Overloads the base class version to read the value from the message.
             */
            virtual void read(SharedPtr<Message> &message)
            {
               message->readAsciiZ(value);
            }

            /**
             * Overloads the base class version to write the value to the message.
             */
            virtual void write(SharedPtr<Message> &message)
            {
               if(value.length() > 33)
                  value.cut(32);
               message->addAsciiZ(value.c_str());
            }

            /**
             * Overloads the base class version to add the value to the specified array.
             */
            virtual void write(Json::Array &json)
            {
               json.push_back(new Json::String(value));
            }

            /**
             * Overloads the base class version to load the value from the specified array iterator.
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
             * Overloads the base class version to parse the value from the specified stream.
             */
            virtual void input(std::istream &in, bool translate);

            virtual StrAsc get_val_str()
            { return value; }

            virtual void set_val_str(StrAsc const &value_, bool apply_checks)
            {
               if(value.compare(value_, true) != 0)
               {
                  if(apply_checks)
                  {
                     if(value_.length() > 32)
                        throw std::invalid_argument("too long");
                  }
                  value = value_;
                  set_has_changed(true);
               }
            }

            /**
             * @return Returns the identifier for the setting that defines the pick list.
             */
            uint2 get_pick_id() const
            { return desc->get_pick_id(); }

            /**
             * @return Returns the zero based offset for the description component that specifies
             * the SSID.
             */
            uint4 get_ssid_pos() const
            { return desc->get_ssid_pos(); }

            /**
             * @return Specifies the zero based offset for the description component that specified
             * the signal strength in dBm.
             */
            uint4 get_dbm_pos() const
            { return desc->get_dbm_pos(); }

            /**
             * @return Specifies the zero based offset for the description component that specifies
             * whether the network is encrypted.
             */
            uint4 get_encrypted_pos() const
            { return desc->get_encrypted_pos(); }

         protected:
            /**
             * Overloads the base class version to copy this component.
             */
            virtual void do_copy(CompBase *other_)
            {
               CompSsid *other = dynamic_cast<CompSsid *>(other_);
               if(other)
                  value = other->value;
            }
         };
      };
   };
};


#endif

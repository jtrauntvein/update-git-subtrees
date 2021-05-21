/* Csi.DevConfig.SettingComp.TimeStamp.h

   Copyright (C) 2014, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Thursday 22 May 2014
   Last Change: Tuesday 03 December 2019
   Last Commit: $Date: 2019-12-03 17:02:16 -0600 (Tue, 03 Dec 2019) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_DevConfig_SettingComp_TimeStamp_h
#define Csi_DevConfig_SettingComp_TimeStamp_h

#include "Csi.DevConfig.SettingComp.CompBase.h"
#include "Csi.DevConfig.SettingComp.DescBase.h"
#include "Csi.LgrDate.h"


namespace Csi
{
   namespace DevConfig
   {
      namespace SettingComp
      {
         /**
          * Defines a description for a component that allows the user to enter or see a time stamp
          * value.
          */
         class TimeStampDesc: public DescBase
         {
         public:  
            /**
             * Specifies the representation of the time stamp component.
             */
            enum representation_type
            {
               representation_sec,
               representation_nsec
            } representation;


            /**
             * Default Constructor
             */
            TimeStampDesc():
               DescBase(Components::comp_time_stamp)
            { }


            /**
             * Overloads the base class version to read time-stamp specific information.
             */
            virtual void init_from_xml(
               Xml::Element &xml_data, StrAsc const &library_dir)
            {
               DescBase::init_from_xml(xml_data, library_dir);
               StrUni const representation_str(xml_data.get_attr_wstr(L"representation"));
               if(representation_str == L"sec")
                  representation = representation_sec;
               else if(representation_str == L"nsec")
                  representation = representation_nsec;
               else
                  throw std::invalid_argument("invalid time stamp representation specified");
            }


            /**
             * @return Returns a component object associated with this description.
             */
            virtual CompBase *make_component(
               SharedPtr<DescBase> &desc, SharedPtr<CompBase> &previous);

            /**
             * Overloads the base class version to write specific properties.
             */
            virtual void describe_json(Csi::Json::Object &desc)
            {
               DescBase::describe_json(desc);
               desc.set_property_str("representation", representation == representation_nsec ? "nsec" : "sec");
            }
         };


         /**
          * Defines a setting component that represents a time stamp.
          */
         class TimeStamp: public CompBase
         {
         protected:
            /**
             * Specifies the value associated with this component.
             */
            Csi::LgrDate value;

         public:
            /**
             * Constructor
             */
            TimeStamp(SharedPtr<DescBase> &desc):
               CompBase(desc)
            { }


            /**
             * Overloaded to read the component value from the message.
             */
            virtual void read(SharedPtr<Message> &message);


            /**
             * Overloaded to write the component value to the specified message.
             */
            virtual void write(SharedPtr<Message> &message);

            /**
             * Overloads the base class version to add the value to the specified components array.
             */
            virtual void write(Json::Array &json)
            { json.push_back(new Json::Date(value)); }

            /**
             * Overloads the base class version to read the value from the specified json array
             * iterator.
             */
            virtual Json::Array::iterator read(Json::Array::iterator current)
            {
               Json::StringHandle value_json(*current);
               value = LgrDate::fromStr(value_json->get_value().c_str());
               return ++current;
            }

            /**
             * Overloaded to format this component to the specified stream.
             */
            virtual void output(std::ostream &out, bool translate);
            virtual void output(std::wostream &out, bool translate);

            /**
             * Overloaded the read this component from the specified stream.
             */
            virtual void input(std::istream &in, bool translate);

            /**
             * Overloaded to perform the copy operation.
             */
            virtual void do_copy(CompBase *other_)
            {
               TimeStamp *other(static_cast<TimeStamp *>(other_));
               value = other->value;
            }

            /**
             * Overloads the base class version to return the number of nanoseconds elapsed since 1
             * January 1990
             */
            virtual int8 get_val_int8()
            { return value.get_nanoSec(); }

            /**
             * Overloads the base class version to set the number of nanoseconds elapsed since 1
             * January 1990.
             */
            virtual void set_val_int8(int8 val)
            {
               value = Csi::LgrDate(val);
               has_changed = true;
            }
         };
      };
   };
};


#endif

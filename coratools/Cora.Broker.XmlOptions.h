/* Cora.Broker.XmlOptions.h

   Copyright (C) 2006, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 28 April 2006
   Last Change: Tuesday 17 September 2019
   Last Commit: $Date: 2019-09-18 10:56:12 -0600 (Wed, 18 Sep 2019) $ 
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Cora_Broker_XmlOptions_h
#define Cora_Broker_XmlOptions_h

#include "CsiTypeDefs.h"


namespace Cora
{
   namespace Broker
   {
      /**
       * Describes the list of options that can be encoded for CSIXML output.
       */
      class XmlOptions
      {
      private:
         /**
          * Encodes all of the options.
          */
         uint4 options;

      public:
         /**
          * Constructor
          *
          * @param options_ Specifies the initial options.
          */
         enum enum_options
         {
            options_include_record_no   = 0x00000002,
            options_include_time_stamp  = 0x00000001,
            options_include_value_name  = 0x00000004,
            options_midnight_is_2400 = 0x00000008
         };
         XmlOptions(uint4 options_ = (options_include_record_no | options_include_time_stamp)):
            options(options_)
         { }

         /**
          * Copy constructor
          *
          * @param other Specifies the source.
          */
         XmlOptions(XmlOptions const &other):
            options(other.options)
         { }

         /**
          * Copy operator
          *
          * @param other Specifies the source.
          *
          * @param options_ Specifies the set of options.
          */
         XmlOptions &operator =(XmlOptions const &other)
         {
            options = other.options;
            return *this;
         }
         XmlOptions &operator = (uint4 options_)
         {
            options = options_;
            return *this;
         }

         /**
          * @return Returns the encoded options.
          */
         uint4 get_options() const
         { return options; }

         /**
          * @return Returns true if the record number is to be included.
          */
         bool get_include_record_no() const
         { return (options & options_include_record_no) != 0; }

         /**
          * @param val Set to true if the record number is to be included,.
          */
         XmlOptions &set_include_record_no(bool val)
         {
            if(val)
               options |= options_include_record_no;
            else
               options &= ~options_include_record_no;
            return *this;
         }

         /**
          * @return Returns true if the time stamp is to be included.
          */
         bool get_include_time_stamp() const
         { return (options & options_include_time_stamp) != 0; }

         /**
          * @param val Set to true if the time stamp is to be included.
          */
         XmlOptions &set_include_time_stamp(bool val)
         {
            if(val)
               options |= options_include_time_stamp;
            else
               options &= ~options_include_time_stamp;
            return *this;
         }

         /**
          * @return Returns true if the value name is to be encoded with the values in the data
          * section.
          */
         bool get_include_value_name() const
         { return (options & options_include_value_name) != 0; }
         
         /**
          * @param val Set to true if the value name should be encoded with the values in the data
          * section.
          */
         XmlOptions &set_include_value_name(bool val)
         {
            if(val)
               options |= options_include_value_name;
            else
               options &= ~options_include_value_name;
            return *this;
         }

         /**
          * @return Returns true if the time stamp is to be 24:00 at midnight.
          */
         bool get_midnight_is_2400() const
         { return (options & options_midnight_is_2400) != 0; }

         /**
          * @param val Set to true if the time stamp is to be encoded as 24:00 at midnight.
          */
         XmlOptions &set_midnight_is_2400(bool val)
         {
            if(val)
               options |= options_midnight_is_2400;
            else
               options &= ~options_midnight_is_2400;
            return *this;
         }
      };
   };
};


#endif

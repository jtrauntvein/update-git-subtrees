/* Cora.Broker.CustomCsvOptions.h

   Copyright (C) 2006, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 14 February 2006
   Last Change: Tuesday 17 September 2019
   Last Commit: $Date: 2019-09-18 10:56:12 -0600 (Wed, 18 Sep 2019) $ 
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Cora_Broker_CustomCsvOptions_h
#define Cora_Broker_CustomCsvOptions_h

#include "CsiTypeDefs.h"


namespace Cora
{
   namespace Broker
   {
      /**
       * Defines an object that encodes the options for generating a mixed array file from table
       * data.
       */
      class CustomCsvOptions
      {
      private:
         /**
          * Specifies the encoded options.
          */
         uint4 options;

      public:
         /**
          * Consructor
          *
          * @param options_ Specifies the encoded set of options.
          */
         enum options_bits
         {
            options_timestamp_seconds    = 0x00000001,
            options_timestamp_hour_min   = 0x00000002,
            options_timestamp_julian_day = 0x00000004,
            options_timestamp_year       = 0x00000008,
            options_midnight_is_2400     = 0x00000010,
            options_embedded_as_string   = 0x00000020, 
            options_embedded_seconds     = 0x00000040,
            options_embedded_hour_min    = 0x00000080,
            options_include_array_id     = 0x00000100,
            options_array_id_mask        = 0x03ff0000
         };
         CustomCsvOptions(
            uint4 options_ = (options_timestamp_seconds |
                              options_timestamp_hour_min |
                              options_timestamp_julian_day |
                              options_timestamp_year |
                              options_embedded_seconds |
                              options_embedded_hour_min |
                              options_include_array_id |
                              (101 << 16))):
            options(options_)
         { }

         /**
          * Copy constructo
          *
          * @param other Specifies the source object.
          */
         CustomCsvOptions(CustomCsvOptions const &other):
            options(other.options)
         { }

         /**
          * Copy operator
          *
          * @param other Specifies the source.
          *
          8 @param options_ Specifies the encoded options.
         */
         CustomCsvOptions &operator =(CustomCsvOptions const &other)
         {
            options = other.options;
            return *this;
         }
         CustomCsvOptions &operator =(uint4 options_)
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
          * @return Returns the array ID.
          */
         uint4 get_array_id() const
         { return ((options & options_array_id_mask) >> 16); }

         /**
          * @param array_id Specifies the array ID.
          */
         CustomCsvOptions &set_array_id(uint4 array_id)
         {
            options &= ~options_array_id_mask;
            options |= ((array_id & 0x3ff) << 16);
            return *this;
         }

         /**
          * @return Returns true if the aray ID is to be included.
          */
         bool get_include_array_id() const
         { return (options & options_include_array_id) != 0; }

         /**
          * @param val Set to true if the array ID is to be included.
          */
         CustomCsvOptions &set_include_array_id(bool val)
         {
            if(val)
               options |= options_include_array_id;
            else
               options &= ~options_include_array_id;
            return *this;
         }

         /**
          * @return Returns true if the time stamp is to be included.
          */
         bool get_include_timestamp() const
         {
            return
               get_timestamp_year() ||
               get_timestamp_julian_day() ||
               get_timestamp_hour_min() ||
               get_timestamp_seconds();
         }

         /**
          * @return Returns the flags that control the parts of the timestamp that are included.
          */
         uint4 get_timestamp_format_flags() const
         { return options & 0x1f; }

         /**
          * @return Returns the embedded format flags.
          */
         uint4 get_embedded_format_flags() const
         {
            uint4 rtn = 0;
            if((options & options_embedded_as_string) == 0)
            {
               rtn = (options & options_midnight_is_2400);
               if((options & options_embedded_seconds) != 0)
                  rtn |= options_timestamp_seconds;
               if((options & options_embedded_hour_min) != 0)
                  rtn |= options_timestamp_hour_min;
            }
            return rtn;
         }

         /**
          * @return Returns true if the seconds from the time stamp are to be included.
          */
         bool get_timestamp_seconds() const
         { return (options & options_timestamp_seconds) != 0; }
         
         /**
          * @param val Set to true if seconds are to be included in the time stamp.
          */
         CustomCsvOptions &set_timestamp_seconds(bool val)
         {
            if(val)
               options |= options_timestamp_seconds;
            else
               options &= ~options_timestamp_seconds;
            return *this;
         }

         /**
          * @return Returns true if the hour and minute parts of the time stamp are to be included.
          */
         bool get_timestamp_hour_min() const
         { return (options & options_timestamp_hour_min) != 0; }
         
         /**
          * @param val Set to true if the hour and minute part of the time stamp should be included.
          */
         CustomCsvOptions &set_timestamp_hour_min(bool val)
         {
            if(val)
               options |= options_timestamp_hour_min;
            else
               options &= ~options_timestamp_hour_min;
            return *this;
         }

         /**
          * @return Returns true if the julian day is to be included.
          */
         bool get_timestamp_julian_day() const
         { return (options & options_timestamp_julian_day) != 0; }

         /**
          * @param val Set to true if the julian day is to be included.
          */
         CustomCsvOptions &set_timestamp_julian_day(bool val)
         {
            if(val)
               options |= options_timestamp_julian_day;
            else
               options &= ~options_timestamp_julian_day;
            return *this;
         }

         /**
          * @return Returns true if the year is to be included.
          */
         bool get_timestamp_year() const
         { return (options & options_timestamp_year) != 0; }
         
         /**
          * @param val Set to true if the year is to be included.
          */
         CustomCsvOptions &set_timestamp_year(bool val)
         {
            if(val)
               options |= options_timestamp_year;
            else
               options &= ~options_timestamp_year;
            return *this;
         }

         /**
          * @return Returns true if midnight should be formatted as 24:00.
          */
         bool get_midnight_is_2400() const
         { return (options & options_midnight_is_2400) != 0; }
         
         /**
          * @param val Set to true if midnight is to be formatted as 24:00.
          */
         CustomCsvOptions &set_midnight_is_2400(bool val)
         {
            if(val)
               options |= options_midnight_is_2400;
            else
               options &= ~options_midnight_is_2400;
            return *this;
         }

         /**
          * @return Returns true if embedded time stamps should be printed as strings.
          */
         bool get_embedded_as_string() const
         { return (options & options_embedded_as_string) != 0; }
         
         /**
          * @param val Set to true if embedded time stamps are to be printed as strings.
          */
         CustomCsvOptions &set_embedded_as_string(bool val)
         {
            if(val)
               options |= options_embedded_as_string;
            else
               options &= ~options_embedded_as_string;
            return *this;
         }

         /**
          * @return Returns true if seconds for embedded time stamps should be included.
          */
         bool get_embedded_seconds() const
         { return (options & options_embedded_seconds) != 0; }
         
         /**
          * @param val Set to true if the seconds for embedded time stamps should be included.
          */
         void set_embedded_seconds(bool val)
         {
            if(val)
               options |= options_embedded_seconds;
            else
               options &= ~options_embedded_seconds;
         }

         /**
          * @return Returns true if the hour and minute for embedded time stamps should be included.
          */
         bool get_embedded_hour_min() const
         { return (options & options_embedded_hour_min) != 0; }
         
         /**
          * @param val Set to true if the hour and minute for embedded time stamps should be
          * included.
          */
         CustomCsvOptions &set_embedded_hour_min(bool val)
         {
            if(val)
               options |= options_embedded_hour_min;
            else
               options &= ~options_embedded_hour_min;
            return *this;
         }
         //@endgroup
      };
   };
};


#endif

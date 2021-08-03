/* Cora.Broker.Toa5Options.h

   Copyright (C) 2013, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Thursday 23 May 2013
   Last Change: Tuesday 17 September 2019
   Last Commit: $Date: 2019-09-17 11:01:17 -0600 (Tue, 17 Sep 2019) $
   Last Changed by: $Author: jon $

*/

#ifndef Cora_Broker_Toa5Options_h
#define Cora_Broker_Toa5Options_h


namespace Cora
{
   namespace Broker
   {
      /**
       * Maintains a set of options encoded in a four byte integer for formatting TOA5 data.
       */
      class Toa5Options
      {
      private:
         /**
          * Specifies the encoded options.
          */
         uint4 options;

      public:
         /**
          * Constructor
          *
          * @param options_ Specifies the set of options to use.
          */
         enum enum_options
         {
            options_include_time_stamp = 0x00000001,
            options_include_record_no = 0x00000002,
            options_midnight_is_2400 = 0x00000004
         };
         Toa5Options(uint4 options_ = (options_include_time_stamp | options_include_record_no)):
            options(options_)
         { }

         /**
          * Copy constructor.
          *
          * @param other Specifies the source.
          */
         Toa5Options(Toa5Options const &other):
            options(other.options)
         { }

         /**
          * Copy operator.
          *
          * @param other Specifies the source.
          *
          * @param options_ Specifies the options bitmap.
          */
         Toa5Options &operator =(Toa5Options const &other)
         {
            options = other.options;
            return *this;
         }
         Toa5Options &operator =(uint4 options_)
         {
            options = options_;
            return *this;
         }

         /**
          * @return Returns the options.
          */
         uint4 get_options() const
         { return options; }

         /**
          * @return Returns true if the time stamp is to be included.
          */
         bool get_include_time_stamp() const
         { return (options & options_include_time_stamp) != 0; }

         /**
          * @return Returns true if the record number is to be included.
          */
         bool get_include_record_no() const
         { return (options & options_include_record_no) != 0; }

         /**
          * @return Returns true if the time stamp is to be reported as 24:00 at midnight.
          */
         bool get_midnight_is_2400() const
         { return (options & options_midnight_is_2400) != 0; }

         /**
          * @param val Set to true if the time stamp is to be included.
          */
         Toa5Options &set_include_time_stamp(bool val)
         {
            if(val)
               options |= options_include_time_stamp;
            else
               options &= ~options_include_time_stamp;
            return *this;
         }

         /**
          * val Set to true if the record number is to be included.
          */
         Toa5Options &set_include_record_no(bool val)
         {
            if(val)
               options |= options_include_record_no;
            else
               options &= ~options_include_record_no;
            return *this;
         }

         /**
          * @param val Set to true if midnight should be formatted as 24:00.
          */
         Toa5Options &set_midnight_is_2400(bool val)
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

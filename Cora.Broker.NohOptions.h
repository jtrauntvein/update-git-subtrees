/* Cora.Broker.NohFormatOptions.h

   Copyright (C) 2019, 2021 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Monday 16 September 2019
   Last Change: Wednesday 28 April 2021
   Last Commit: $Date: 2021-04-28 16:34:28 -0600 (Wed, 28 Apr 2021) $
   Last Changed by: $Author: jon $

*/

#ifndef Cora_Broker_NohFormatOptions_h
#define Cora_Broker_NohFormatOptions_h


namespace Cora
{
   namespace Broker
   {
      /**
       * Defines an object that provides shorthand to set format options for the NOH CSV file
       * format.
       */
      class NohOptions
      {
      private:
         /**
          * Specifies the format options.
          */
         uint4 options;

      public:
         /**
          * Constructor
          *
          * @param options_ Specifies the options to use.
          */
         enum options_type
         {
            options_include_time_stamp = 0x00000001,
            options_include_record_no = 0x00000002,
            options_dont_quote_strings = 0x00000004,
            options_midnight_is_2400 = 0x00000008
         };
         NohOptions(uint4 options_ = (options_include_time_stamp | options_include_record_no)):
            options(options_)
         { }

         /**
          * Copy constructor
          *
          * @param other Specifies the source.
          */
         NohOptions(NohOptions const &other):
            options(other.options)
         { }

         /**
          * Copy operator
          */
         NohOptions &operator =(NohOptions const &other)
         {
            options = other.options;
            return *this;
         }

         /**
          * @return Returns the options aggregate.
          */
         uint4 get_options() const
         { return options; }

         /**
          * @return Returns true if the time stamp is to be included.
          */
         bool get_include_time_stamp() const
         { return (options & options_include_time_stamp) != 0; }

         /**
          * @param value Set to true if the time stamp is to be included.
          */
         NohOptions &set_include_time_stamp(bool value)
         {
            if(value)
               options |= options_include_time_stamp;
            else
               options &= ~options_include_time_stamp;
            return *this;
         }

         /**
          * @return Returns true of the record number is to be included.
          */
         bool get_include_record_no() const
         { return (options & options_include_record_no) != 0; }

         /**
          * @param value Set to true if the record number should be included.
          */
         NohOptions &set_include_record_no(bool value)
         {
            if(value)
               options |= options_include_record_no;
            else
               options &= ~options_include_record_no;
            return *this;
         }

         /**
          * @return Returns true if strings are not to be quoted.
          */
         bool get_dont_quote_strings() const
         { return (options & options_dont_quote_strings) != 0; }

         /**
          * @param value Set to true if strings should not be quoted.
          */
         NohOptions &set_dont_quote_strings(bool value)
         {
            if(value)
               options |= options_dont_quote_strings;
            else
               options &= ~options_dont_quote_strings;
            return *this;
         }

         /**
          * @return Returns true if the time statmp at midnight is to be formatted as 24:00.
          */
         bool get_midnight_is_2400() const
         { return (options & options_midnight_is_2400) != 0; }

         /**
          * @param value Set to true if midnight is to be formatted as 24:00
          */
         NohOptions &set_midnight_is_2400(bool value)
         {
            if(value)
               options |= options_midnight_is_2400;
            else
               options &= ~options_midnight_is_2400;
            return *this;
         }
      };
   
   };
};


#endif


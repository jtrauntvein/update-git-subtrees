/* Cora.Broker.Tob1Options.h

   Copyright (C) 2013, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Thursday 23 May 2013
   Last Change: Monday 16 September 2019
   Last Commit: $Date: 2019-09-16 17:52:06 -0600 (Mon, 16 Sep 2019) $
   Last Changed by: $Author: jon $

*/

#ifndef Cora_Broker_Tob1Options_h
#define Cora_Broker_Tob1Options_h


namespace Cora
{
   namespace Broker
   {
      /**
       * Defines an object that maintains options for the TOB1 file format.
       */
      class Tob1Options
      {
      private:
         /**
          * Specifies the options.
          */
         uint4 options;

      public:
         /**
          * @param options_ Specifies the format options.
          */
         enum enum_options
         {
            options_include_time_stamp = 0x00000001,
            options_include_record_no = 0x00000002
         };
         Tob1Options(uint4 options_ = (options_include_time_stamp | options_include_record_no)):
            options(options_)
         { }

         /**
          * Copy constructor
          *
          * @param other Specifies the source.
          */
         Tob1Options(Tob1Options const &other):
            options(other.options)
         { }

         /**
          * Copy operator
          *
          * @param other Specifies the source.
          */
         Tob1Options &operator =(Tob1Options const &other)
         {
            options = other.options;
            return *this;
         }
         Tob1Options &operator =(uint4 options_)
         {
            options = options_;
            return *this;
         }

         /**
          * @return Returns the aggregate options
          */
         uint4 get_options() const
         { return options; }

         /**
          * @return Returns true if the time stamp should be included.
          */
         bool get_include_time_stamp() const
         { return (options & options_include_time_stamp) != 0; }

         /**
          * @return returns true if the record number is to be included.
          */
         bool get_include_record_no() const
         { return (options & options_include_record_no) != 0; }

         /**
          * @param val Set to true if the time stamp is to be included.
          */
         Tob1Options &set_include_time_stamp(bool val)
         {
            if(val)
               options |= options_include_time_stamp;
            else
               options &= ~options_include_time_stamp;
            return *this;
         }

         /**
          * @param val Set to true if the record number is to be included.
          */
         Tob1Options &set_include_record_no(bool val)
         {
            if(val)
               options |= options_include_record_no;
            else
               options &= ~options_include_record_no;
            return *this;
         }
      };
   };
};

#endif

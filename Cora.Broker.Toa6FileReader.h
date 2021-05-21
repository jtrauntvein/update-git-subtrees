/* Cora.Broker.Toa6FileReader.h

   Copyright (C) 2017, Campbell Scientific, Inc

   Written by: Jon Trauntvein 
   Date Begun: Thursday 07 September 2017
   Last Change: Thursday 07 September 2017
   Last Commit: $Date: 2017-09-07 16:55:39 -0600 (Thu, 07 Sep 2017) $
   Last Changed by: $Author: jon $

*/

#ifndef Cora_Broker_Toa6FileReader_h
#define Cora_Broker_Toa6FileReader_h
#include "Cora.Broker.DataFileReader.h"


namespace Cora
{
   namespace Broker
   {
      /**
       * Defines a component that parses a TOA6 data file to produce data indices, meta-data, and
       * record data.
       */
      class Toa6FileReader: public DataFileReader
      {
      public:
         /**
          * Constructor
          *
          * @param value_factory Specifies the object that will generate value objects for records.
          */
         Toa6FileReader(value_factory_handle value_factory = 0);

         /**
          * Destructor
          */
         virtual ~Toa6FileReader();

         /**
          * Overloads the base class version to attempt to open the specifdied file and sets up the
          * record description based upon information found in the file header.  if a file is
          * already opened, that file will be closed.
          */
         virtual void open(
            StrAsc const &file_name, StrAsc const &label_file_name);

         /**
          * Overloads the base class version to relesae any resources associated with the current
          * file.
          */
         virtual void close();

         /**
          * Overloads the base class version  to store the current header signature and to close the
          * input file.
          */
         virtual void hibernate();

         /**
          * Overloads the base class version to re-open the file, check its header, and determine
          * whether the file has been overwritten.
          */
         virtual bool wake_up(bool &all_data_overwritten);

         /**
          * Overloads the base class version to read the next record from the file based upon the
          * current position.
          */
         virtual read_outcome_type read_next_record(
            record_handle &destination,
            bool *file_mark_after_record = 0,
            bool *remove_mark_after_record = 0,
            uint4 array_id = 0);

         /**
          * Overloads the base class version to generate an index of the records in the data file.
          */
         virtual void generate_index(
            index_type &index, bool &should_abort, uint4 *next_record_no);

         /**
          * @return Overloads the base class version to indicate if the last time stamp specified
          * midnight at 24:00.
          */
         virtual bool last_time_was_2400() const;

         /**
          * @return Returns the number of header fields that are expectd.
          */
         uint4 fixed_cols() const
         {
            uint4 rtn(0);
            if(time_stamp_present)
               ++rtn;
            if(record_no_present)
               ++rtn;
            return rtn;
         }

         /**
          * @return Returns true if this file contains record time stamps.
          */
         virtual bool has_time_stamp() const
         { return time_stamp_present; }
         
         /**
          * @return Returns true if this file contains record numbers.
          */
         virtual bool has_record_no() const
         { return record_no_present; }

         /**
          * @return Overloads the base class version to return the size of the header portion of the
          * file.
          */
         virtual int8 get_header_len()
         { return header_len; }

         /**
          * @return Overloads the base class version to return the signature of the header.
          */
         virtual uint2 get_header_sig();

         /**
          * @return Overloads the base class version to return the length of the data portion of the
          * file.
          */
         virtual int8 get_data_len();

         /**
          * @return Overloads the base class version to return the current offset into the data
          * portion of the file.
          */
         virtual int8 get_data_offset();

         /**
          * Overloads the base class version to seek into the data portion of the file.
          */
         virtual void seek_data(int8 offset, bool search_for_prev = true);

      private:
         /**
          * Specifies the name of the file currently being read.
          */
         StrAsc file_name;

         /**
          * Specifies the file handle.
          */
         FILE *input;

         /**
          * Set to true if the record number is present in the header.
          */
         bool record_no_present;

         /**
          * Set to true if the time stamp field is present in the header.
          */
         bool time_stamp_present;

         /**
          * Specifies the length of the header.
          */
         uint4 header_len;

         /**
          * Set to true if this reader has been hibernated.
          */
         bool is_sleeping;

         /**
          * Specifies the signature of the header.
          */
         uint2 header_sig;

         /**
          * Specifies the current position in the hibernated file.
          */
         int8 hibernate_pos;

         /**
          * Specifies the length of the data portion of the file.
          */
         int8 data_len;

         /**
          * Specifies the signature for data.
          */
         uint2 data_sig;

         /**
          * Specifies the last time string,
          */
         StrAsc last_time_str;
      };
   };
};


#endif

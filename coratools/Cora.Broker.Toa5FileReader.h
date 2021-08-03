/* Cora.Broker.Toa5FileReader.h

   Copyright (C) 2002, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Saturday 16 August 2003
   Last Change: Monday 11 July 2016
   Last Commit: $Date: 2016-07-11 09:29:07 -0600 (Mon, 11 Jul 2016) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Cora_Broker_Toa5FileReader_h
#define Cora_Broker_Toa5FileReader_h

#include "Cora.Broker.DataFileReader.h"


namespace Cora
{
   namespace Broker
   {
      ////////////////////////////////////////////////////////////
      // class Toa5FileReader
      //
      // Defines a class that parses a TOA5 file in terms of Cora::Broker::record objects. 
      ////////////////////////////////////////////////////////////
      class Toa5FileReader: public DataFileReader
      {
      public:
         ////////////////////////////////////////////////////////////
         // default constructor
         //
         // Creates the object in an unopened state
         ////////////////////////////////////////////////////////////
         Toa5FileReader(
            value_factory_handle value_factory = 0);

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~Toa5FileReader();

         ////////////////////////////////////////////////////////////
         // open
         //
         // Attempts to open the specified file and sets up the current description based upon
         // information obtained from the file header.  If another file is already opened, that file
         // will be closed.
         ////////////////////////////////////////////////////////////
         virtual void open(
            StrAsc const &file_name, StrAsc const &label_file_name);

         ////////////////////////////////////////////////////////////
         // close
         //
         // Closes the currently opened file and invalidates the current description.
         ////////////////////////////////////////////////////////////
         virtual void close();

         ////////////////////////////////////////////////////////////
         // hibernate
         //
         // Overloads the base class' to store the current header signature and
         // close the input file. 
         ////////////////////////////////////////////////////////////
         virtual void hibernate();

         ////////////////////////////////////////////////////////////
         // wake_up
         ////////////////////////////////////////////////////////////
         virtual bool wake_up(bool &all_data_overwritten);

         ////////////////////////////////////////////////////////////
         // read_next_record
         //
         // Attempts to read the next record from the currently opened file.  The application is
         // responsible for providing a record object for the data to be read into.  It can create
         // these objects by calling make_record().
         ////////////////////////////////////////////////////////////
         virtual read_outcome_type read_next_record(
            record_handle &destination,
            bool *file_mark_after_record = 0,
            bool *remove_mark_after_record = 0,
            uint4 array_id = 0);

         ////////////////////////////////////////////////////////////
         // generate_index
         ////////////////////////////////////////////////////////////
         virtual void generate_index(index_type &index, bool &should_abort, uint4 *next_record_no);

         ////////////////////////////////////////////////////////////
         // last_time_was_2400
         ////////////////////////////////////////////////////////////
         virtual bool last_time_was_2400() const;

         ////////////////////////////////////////////////////////////
         // fixed_cols
         //
         // Returns the number of header fields are expected
         ////////////////////////////////////////////////////////////
         uint4 fixed_cols() const
         {
            uint4 rtn = 0;
            if(time_stamp_present)
               ++rtn;
            if(record_no_present)
               ++rtn;
            return rtn;
         }

         ////////////////////////////////////////////////////////////
         // has_time_stamp
         ////////////////////////////////////////////////////////////
         virtual bool has_time_stamp() const
         { return time_stamp_present; }

         ////////////////////////////////////////////////////////////
         // has_record_no
         ////////////////////////////////////////////////////////////
         virtual bool has_record_no() const
         { return record_no_present; }

         ////////////////////////////////////////////////////////////
         // get_header_len
         ////////////////////////////////////////////////////////////
         virtual int8 get_header_len()
         { return header_len; }

         ////////////////////////////////////////////////////////////
         // get_header_sig
         ////////////////////////////////////////////////////////////
         virtual uint2 get_header_sig();

         ////////////////////////////////////////////////////////////
         // get_data_len
         ////////////////////////////////////////////////////////////
         virtual int8 get_data_len();

         ////////////////////////////////////////////////////////////
         // get_data_offset
         ////////////////////////////////////////////////////////////
         virtual int8 get_data_offset();

         ////////////////////////////////////////////////////////////
         // seek_data
         ////////////////////////////////////////////////////////////
         virtual void seek_data(int8 offset, bool search_for_prev = true);

         ////////////////////////////////////////////////////////////
         // output_ieee8
         //
         // Controls whether TOA5 readers should create IEEE8 or IEEE4 floating
         // point values.  
         ////////////////////////////////////////////////////////////
         static bool output_ieee8;

      private:
         ////////////////////////////////////////////////////////////
         // file_name
         ////////////////////////////////////////////////////////////
         StrAsc file_name;
         
         ////////////////////////////////////////////////////////////
         // input
         //
         // If not null, refers to the current file that the record description was generated from.  
         ////////////////////////////////////////////////////////////
         FILE *input;

         ////////////////////////////////////////////////////////////
         // record_no_present
         //
         // Indicates whether the record number field was located in the header.  With TOA5, this
         // value is considered to be optional.
         ////////////////////////////////////////////////////////////
         bool record_no_present;

         ////////////////////////////////////////////////////////////
         // time_stamp_present
         ////////////////////////////////////////////////////////////
         bool time_stamp_present;

         ////////////////////////////////////////////////////////////
         // header_len
         ////////////////////////////////////////////////////////////
         uint4 header_len;

         ////////////////////////////////////////////////////////////
         // is_sleeping
         ////////////////////////////////////////////////////////////
         bool is_sleeping;

         ////////////////////////////////////////////////////////////
         // header_sig
         ////////////////////////////////////////////////////////////
         uint2 header_sig;

         ////////////////////////////////////////////////////////////
         // hibernate_pos
         ////////////////////////////////////////////////////////////
         int8 hibernate_pos;

         ////////////////////////////////////////////////////////////
         // data_sig
         ////////////////////////////////////////////////////////////
         uint2 data_sig;

         ////////////////////////////////////////////////////////////
         // data_len
         ////////////////////////////////////////////////////////////
         int8 data_len;

         ////////////////////////////////////////////////////////////
         // last_time_str
         ////////////////////////////////////////////////////////////
         StrAsc last_time_str;
      };
   };
};


#endif

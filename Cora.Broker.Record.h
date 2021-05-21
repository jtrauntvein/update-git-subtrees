/* Cora.Broker.Record.h

   Copyright (C) 1998, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 24 August 1999
   Last Change: Thursday 06 February 2020
   Last Commit: $Date: 2020-02-06 19:41:32 -0600 (Thu, 06 Feb 2020) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Cora_Broker_Record_h
#define Cora_Broker_Record_h


#include "Csi.SharedPtr.h"
#include "Csi.LgrDate.h"
#include <vector>
#include "Csi.Messaging.Message.h"
#include "Cora.Broker.ValueFactory.h"
#include "Cora.Broker.CustomCsvOptions.h"
#include "Cora.Broker.XmlOptions.h"
#include "Cora.Broker.Toa5Options.h"
#include "Cora.Broker.Tob1Options.h"
#include "Cora.Broker.RecordDesc.h"
#include "Cora.Broker.Value.h"
#include "Csi.StrAscStream.h"


//@group forward class declarations
class StrUni;
//@endgroup

namespace Cora
{
   namespace Broker
   {
      ////////////////////////////////////////////////////////////
      // class Record
      //
      // Defines a record from a data set resulting from a data advise or data
      // query operation.
      ////////////////////////////////////////////////////////////
      class Record
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         //
         // Builds up the list of values from the specifications specified in
         // ack_desc using the factory parameter to construct value objects.
         ////////////////////////////////////////////////////////////
         typedef Csi::SharedPtr<RecordDesc> desc_handle;
         Record(desc_handle &description, ValueFactory &factory);

         ////////////////////////////////////////////////////////////
         // copy constructor
         //
         // Creates a new record object where with a new set of data
         // values. This new copy can be read and changed without affecting the
         // source. This new record copy will, however, use the same meta-data
         // references as the original record.
         ////////////////////////////////////////////////////////////
         Record(Record &other);

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~Record();

         ////////////////////////////////////////////////////////////
         // copy operator
         //
         // Insures that the other record is using the same meta-data as this
         // record and then copies the values from the other record into this
         // record. If the meta-data does not match, a std::invalid_argument
         // exception will be thrown.
         ////////////////////////////////////////////////////////////
         Record &operator =(Record const &other);

         //@group value iterator declarations
         ////////////////////////////////////////////////////////////
         // begin
         ////////////////////////////////////////////////////////////
         typedef Csi::SharedPtr<Value> value_handle;
         typedef value_handle value_type;
         typedef std::vector<value_handle> values_type;
         typedef values_type::iterator iterator;
         typedef values_type::const_iterator const_iterator;
         typedef values_type::reverse_iterator reverse_iterator;
         typedef values_type::const_reverse_iterator const_reverse_iterator;
         iterator begin() { return values.begin(); }
         const_iterator begin() const { return values.begin(); }

         ////////////////////////////////////////////////////////////
         // rbegin
         ////////////////////////////////////////////////////////////
         reverse_iterator rbegin() { return values.rbegin(); }
         const_reverse_iterator rbegin() const { return values.rbegin(); }

         ////////////////////////////////////////////////////////////
         // end
         ////////////////////////////////////////////////////////////
         iterator end() { return values.end(); }
         const_iterator end() const { return values.end(); }

         ////////////////////////////////////////////////////////////
         // rend
         ////////////////////////////////////////////////////////////
         reverse_iterator rend() { return values.rend(); }
         const_reverse_iterator rend() const { return values.rend(); }

         ////////////////////////////////////////////////////////////
         // size
         ////////////////////////////////////////////////////////////
         values_type::size_type size() const { return values.size(); }

         ////////////////////////////////////////////////////////////
         // subscript operator
         ////////////////////////////////////////////////////////////
         value_handle &operator [](values_type::size_type index)
         { return values[index]; }
         value_handle const &operator [](values_type::size_type index) const
         { return values[index]; }

         /**
          * @return Returns the value associated with the specified index.
          *
          * @param index Specifies the index.
          */
         value_handle &at(size_t index)
         { return values.at(index); }
         value_handle const &at(size_t index) const
         { return values.at(index); }

         ////////////////////////////////////////////////////////////
         // find
         //
         // Searches for the value with the specified name and returns the
         // iterator for that value or end() if not found. 
         ////////////////////////////////////////////////////////////
         iterator find(StrUni const &name);

         ////////////////////////////////////////////////////////////
         // find_formatted
         //
         // Searches for the value whose formatted name matches the name
         // provided and returns an iterator to that value.  If no value is
         // found, the return value will match the value returned by end().
         ////////////////////////////////////////////////////////////
         iterator find_formatted(StrAsc const &name);
         iterator find_formatted(StrUni const &name_)
         {
            StrAsc name;
            name_.toMulti(name);
            return find_formatted(name);
         }
         
         //@endgroup

         //@group member access methods
         ////////////////////////////////////////////////////////////
         // get_broker_name
         ////////////////////////////////////////////////////////////
         StrUni const &get_broker_name() const;

         ////////////////////////////////////////////////////////////
         // get_table_name
         ////////////////////////////////////////////////////////////
         StrUni const &get_table_name() const;

         ////////////////////////////////////////////////////////////
         // get_stamp
         ////////////////////////////////////////////////////////////
         Csi::LgrDate const &get_stamp() const { return stamp; }

         ////////////////////////////////////////////////////////////
         // set_stamp
         ////////////////////////////////////////////////////////////
         void set_stamp(Csi::LgrDate const &stamp_)
         { stamp = stamp_; }

         ////////////////////////////////////////////////////////////
         // get_record_no
         ////////////////////////////////////////////////////////////
         uint4 get_record_no() const { return record_no; }

         ////////////////////////////////////////////////////////////
         // set_record_no
         ////////////////////////////////////////////////////////////
         void set_record_no(uint4 record_no_)
         { record_no = record_no_; }

         ////////////////////////////////////////////////////////////
         // get_file_mark_no
         ////////////////////////////////////////////////////////////
         uint4 get_file_mark_no() const { return file_mark_no; }

         ///////////////////////////////////////////////////////////
         // set_file_mark_no
         ///////////////////////////////////////////////////////////
         void set_file_mark_no(uint4 file_mark_no_)
         { file_mark_no = file_mark_no_; }
         //@endgroup

         ////////////////////////////////////////////////////////////
         // read
         //
         // Reads the record number, file mark number, stamp, and values from
         // msg assuming that msg is either a broker data advise notification
         // or a broker data query return records message. Returns true if the
         // record information could be read or false if an error occurred in
         // reading the message.
         ////////////////////////////////////////////////////////////
         bool read(
            Csi::Messaging::Message &msg,
            bool read_file_mark = true);

         ////////////////////////////////////////////////////////////
         // read
         //
         // Reads the values from the specified buffer.  The time stamp, file
         // mark number and record number are already presumed to be extracted
         // and should be passed as parameters.  The return value will be false
         // if the specified buffer does not have enough storage to read all of
         // the values.
         ////////////////////////////////////////////////////////////
         bool read(
            uint4 record_no_,
            uint4 file_mark_no_,
            Csi::LgrDate const &stamp,
            void const *buffer,
            uint4 buffer_len);

         ////////////////////////////////////////////////////////////
         // get_description
         ////////////////////////////////////////////////////////////
         desc_handle &get_description()
         { return description; }

         ////////////////////////////////////////////////////////////
         // write_toaci1
         //
         // Formats this record as a it should appear in a TOACI1 file.
         ////////////////////////////////////////////////////////////
         void write_toaci1_format(
            StrBin &buffer);

         ////////////////////////////////////////////////////////////
         // write_toa5_format
         //
         // Formats this record as it should appear in a TOA5 file.
         ////////////////////////////////////////////////////////////
         void write_toa5_format(
            StrBin &buffer,
            bool write_stamp = true,
            bool write_record_no = true,
            uint4 begin_value = 0,
            uint4 end_value = 0xFFFFFFFF,
            bool midnight_is_2400 = false);
         void write_toa5_format(
            StrBin &buffer,
            Toa5Options const &options,
            uint4 begin_value = 0,
            uint4 end_value = 0xFFFFFFFF)
         {
            write_toa5_format(
               buffer,
               options.get_include_time_stamp(),
               options.get_include_record_no(),
               begin_value,
               end_value,
               options.get_midnight_is_2400());
         }

         ////////////////////////////////////////////////////////////
         // write_tob1_file
         //
         // Formats this record as it should appear in a TOB1 file
         ////////////////////////////////////////////////////////////
         void write_tob1_format(
            StrBin &buffer,
            bool write_stamp = true,
            bool write_record_no = true,
            uint4 begin_value = 0,
            uint4 end_value = 0xFFFFFFFF);
         void write_tob1_format(
            StrBin &buffer,
            Tob1Options const &options,
            uint4 begin_value = 0,
            uint4 end_value = 0xFFFFFFFF)
         {
            write_tob1_format(
               buffer,
               options.get_include_time_stamp(),
               options.get_include_record_no(),
               begin_value,
               end_value);
         }

         ////////////////////////////////////////////////////////////
         // write_custom_csv
         //
         // Writes the record as a custom classic datalogger compatible comma
         // separated values data file.  The custom options are specified by
         // the options field.
         ////////////////////////////////////////////////////////////
         void write_custom_csv(
            StrBin &buffer,
            CustomCsvOptions options = CustomCsvOptions(),
            uint4 begin_value = 0,
            uint4 end_value = 0xFFFFFFFF);

         ////////////////////////////////////////////////////////////
         // write_xml_format
         ////////////////////////////////////////////////////////////
         void write_xml_format(
            StrBin &buffer,
            XmlOptions options = XmlOptions(),
            uint4 begin_value = 0,
            uint4 end_value = 0xFFFFFFFF);

         ////////////////////////////////////////////////////////////
         // write_json_format
         ////////////////////////////////////////////////////////////
         void write_json_format(
            StrBin &buffer,
            uint4 begin_value = 0,
            uint4 end_value = 0xFFFFFFFF,
            bool append = false);

         /**
          * @return Returns a JSON object that represents this record
          *
          * @param begin_value Specifies the value number to start on.
          *
          * @param end_value Specifies the value index before which to end.
          */
         Csi::Json::ObjectHandle write_json(uint4 begin_value = 0, uint4 end_value = 0xffffffff);
         
         ////////////////////////////////////////////////////////////
         // write_comma_delimited
         ////////////////////////////////////////////////////////////
         void write_comma_delimited(
            std::ostream &out,
            bool write_stamp = true,
            bool write_record_no = true,
            bool specials_as_numbers = false,
            uint4 begin_value = 0,
            uint4 end_value = 0xFFFFFFFF,
            bool midnight_is_2400 = false,
            bool quote_strings = true);

         ////////////////////////////////////////////////////////////
         // get_value_buffer
         ////////////////////////////////////////////////////////////
         void const *get_value_buffer() const
         { return values_storage; }

         ////////////////////////////////////////////////////////////
         // get_value_buffer_len
         ////////////////////////////////////////////////////////////
         uint4 get_value_buffer_len() const
         { return values_storage_len; }

         ////////////////////////////////////////////////////////////
         // set_null_values
         //
         // Sets all of the values in the record to their "null" values. 
         ////////////////////////////////////////////////////////////
         void set_null_values();

         ////////////////////////////////////////////////////////////
         // calc_sig
         //
         // Calculates a signature that is representative of this record and its meta-data
         // (including its record number and time stamp). 
         ////////////////////////////////////////////////////////////
         uint2 calc_sig() const;

      private:
         ////////////////////////////////////////////////////////////
         // values
         //
         // contains the values for this record
         ////////////////////////////////////////////////////////////
         values_type values;

         ////////////////////////////////////////////////////////////
         // description
         //
         ////////////////////////////////////////////////////////////
         desc_handle description;
      
         ////////////////////////////////////////////////////////////
         // stamp
         ////////////////////////////////////////////////////////////
         Csi::LgrDate stamp;

         ////////////////////////////////////////////////////////////
         // record_no
         ////////////////////////////////////////////////////////////
         uint4 record_no;

         ////////////////////////////////////////////////////////////
         // file_mark_no
         ////////////////////////////////////////////////////////////
         uint4 file_mark_no;

         ////////////////////////////////////////////////////////////
         // values_storage
         ////////////////////////////////////////////////////////////
         byte *values_storage;

         ////////////////////////////////////////////////////////////
         // values_storage_len
         ////////////////////////////////////////////////////////////
         uint4 values_storage_len;

         ////////////////////////////////////////////////////////////
         // formatter
         //
         // Used to format records for the various calls.  This avoids the
         // creation of temporary stream objects for this job.
         ////////////////////////////////////////////////////////////
         Csi::OStrAscStream formatter;

         ////////////////////////////////////////////////////////////
         // tob1_native
         //
         // Set to true if all of the values in the record are TOB1 native
         // types.  If true, this means that formatting this record to TOB1 is
         // a matter of merely copying the record buffer to the destination. 
         ////////////////////////////////////////////////////////////
         bool tob1_native;
      };
   };
};

#endif

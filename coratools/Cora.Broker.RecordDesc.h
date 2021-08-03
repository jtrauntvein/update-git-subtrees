/* Cora.Broker.RecordDesc.h

   Copyright (C) 1998, 2014 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 25 August 1999
   Last Change: Wednesday 17 December 2014
   Last Commit: $Date: 2018-12-19 17:04:29 -0600 (Wed, 19 Dec 2018) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Cora_Broker_RecordDesc_h
#define Cora_Broker_RecordDesc_h

#include <vector>
#include "StrUni.h"
#include "Csi.SharedPtr.h"
#include "Csi.Messaging.Message.h" 
#include "Cora.Broker.Toa5Options.h"
#include "Cora.Broker.Tob1Options.h"
#include "Csi.Json.h"


namespace Cora
{
   namespace Broker
   {
      //@group forward class declarations
      class ValueDesc;
      //@endgroup
      
      ////////////////////////////////////////////////////////////
      // class RecordDesc
      //
      // Describes a record in a data set
      //////////////////////////////////////////////////////////// 
      class RecordDesc
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         //////////////////////////////////////////////////////////// 
         RecordDesc(StrUni const &broker_name, StrUni const &table_name);

         ////////////////////////////////////////////////////////////
         // destructor
         //////////////////////////////////////////////////////////// 
         ~RecordDesc();

         ////////////////////////////////////////////////////////////
         // read
         //
         // Reads a partial acknowledgement message from the data broker data
         // query or data broker data advise transactions. Returns true if the
         // acknowledgement could be understood or false otherwise
         //////////////////////////////////////////////////////////// 
         bool read(
            Csi::Messaging::Message &ack,
            bool read_value_descriptions = false);

         ////////////////////////////////////////////////////////////
         // write_toaci1_header
         ////////////////////////////////////////////////////////////
         void write_toaci1_header(
            std::ostream &out,
            bool binary_stream = true);

         ////////////////////////////////////////////////////////////
         // write_toa5_header
         ////////////////////////////////////////////////////////////
         void write_toa5_header(
            std::ostream &out,
            bool binary_stream = true,
            bool write_stamp = true,
            bool write_record_no = true,
            uint4 begin_value = 0,
            uint4 end_value = 0xFFFFFFFF);
         void write_toa5_header(
            std::ostream &out,
            bool binary_stream = true,
            Toa5Options const &options = Toa5Options(),
            uint4 begin_value = 0,
            uint4 end_value = 0xFFFFFFFF)
         {
            write_toa5_header(
               out,
               binary_stream,
               options.get_include_time_stamp(),
               options.get_include_record_no(),
               begin_value,
               end_value);
         }

         /**
          * Writes a CSV header that includes lines only for field names and
          * units.
          *
          * @param out Specifies the stream to which the header will be formatted.
          *
          * @param write_stamp Set to true if the time stamp column is to be included.
          *
          * @param write_record_no Set to true of the record number column is to be included.
          *
          * @param begin_value Specifies the index of the first value to be included.
          *
          * @param end_value Specifies the index of the last value to be included.
          */
         void write_csv_header(
            std::ostream &out,
            bool write_stamp = true,
            bool write_record_no = true,
            uint4 begin_value = 0,
            uint4 end_value = 0xFFFFFFFF);
         
         ////////////////////////////////////////////////////////////
         // write_tob1_header
         ////////////////////////////////////////////////////////////
         void write_tob1_header(
            std::ostream &out,
            bool write_stamp = true,
            bool write_record_no = true,
            uint4 begin_value = 0,
            uint4 end_value = 0xFFFFFFFF);
         void write_tob1_header(
            std::ostream &out,
            Tob1Options const &options = Tob1Options(),
            uint4 begin_value = 0,
            uint4 end_value = 0xFFFFFFFF)
         {
            write_tob1_header(
               out,
               options.get_include_time_stamp(),
               options.get_include_record_no(),
               begin_value,
               end_value);
         }

         ////////////////////////////////////////////////////////////
         // write_xml_header
         ////////////////////////////////////////////////////////////
         void write_xml_header(
            std::ostream &out,
            bool write_head_only = false,
            uint4 begin_value = 0,
            uint4 end_value = 0xFFFFFFFF);

         ////////////////////////////////////////////////////////////
         // write_json_header
         ////////////////////////////////////////////////////////////
         void write_json_header(
            std::ostream &out,
            bool write_head_only = false,
            uint4 begin_value = 0,
            uint4 end_value = 0xFFFFFFFF,
            int transaction = -1,
            uint2 signature = 0xAAAA);

         /**
          * Creates a header object for the CSIJson file format.
          *
          * @return Returns the JSON object that represents the header.
          *
          * @param begin_value  Specifies the index for the starting value.
          *
          * @param end_value Specifies the index for the end value.
          *
          * @param transaction Specifies the transaction number to report.  Set
          * to a negative value if this is to be automatically generated.
          */
         Csi::Json::ObjectHandle create_json_header(
            uint4 begin_value = 0,
            uint4 end_value = 0xffffffff,
            int transaction = -1,
            uint2 signature = 0xAAAA);

         ////////////////////////////////////////////////////////////
         // write_field_names
         ////////////////////////////////////////////////////////////
         void write_field_names(
            std::ostream &out,
            bool merge_strings = true,
            uint4 begin_value = 0,
            uint4 end_value = 0xFFFFFFFF);

         ////////////////////////////////////////////////////////////
         // write_units
         ////////////////////////////////////////////////////////////
         void write_units(
            std::ostream &out,
            uint4 begin_value = 0,
            uint4 end_value = 0xFFFFFFF);

         ////////////////////////////////////////////////////////////
         // write_processes
         ////////////////////////////////////////////////////////////
         void write_processes(
            std::ostream &out,
            uint4 begin_value = 0,
            uint4 end_value = 0xFFFFFFFF);

         ////////////////////////////////////////////////////////////
         // get_record_len
         //
         // Calculates the length of the record in bytes.
         ////////////////////////////////////////////////////////////
         uint4 get_record_len();

         ////////////////////////////////////////////////////////////
         // get_sig
         //
         // Calculates the signature of the table definitions for the specified columns
         ////////////////////////////////////////////////////////////
         uint2 get_sig(uint4 begin_value = 0, uint4 end_value = 0xFFFFFFFF);

      public:
         ////////////////////////////////////////////////////////////
         // broker_name
         //
         // Name of the station (broker) that provided the description
         //////////////////////////////////////////////////////////// 
         StrUni broker_name;

         ////////////////////////////////////////////////////////////
         // table_name
         //
         // Name of the table that provided the description
         //////////////////////////////////////////////////////////// 
         StrUni table_name;

         //@group header members: Provided to store values read from the
         //headers of some (TOB1 and TOA5) data formats.
         
         ////////////////////////////////////////////////////////////
         // model_name
         ////////////////////////////////////////////////////////////
         StrAsc model_name;

         ////////////////////////////////////////////////////////////
         // serial_number
         ////////////////////////////////////////////////////////////
         StrAsc serial_number;

         ////////////////////////////////////////////////////////////
         // os_version
         ////////////////////////////////////////////////////////////
         StrAsc os_version;

         ////////////////////////////////////////////////////////////
         // dld_name
         ////////////////////////////////////////////////////////////
         StrAsc dld_name;

         ////////////////////////////////////////////////////////////
         // dld_signature
         ////////////////////////////////////////////////////////////
         StrAsc dld_signature;
         //@endgroup

         ////////////////////////////////////////////////////////////
         // values
         //////////////////////////////////////////////////////////// 
         typedef Csi::SharedPtr<ValueDesc> value_desc_handle;
         typedef value_desc_handle value_type;
         typedef std::vector<value_desc_handle> values_type;
         values_type values;

         ////////////////////////////////////////////////////////////
         // last_record_len
         ////////////////////////////////////////////////////////////
         uint4 last_record_len;

      public:
         //@group Value definitions access declarations

         ////////////////////////////////////////////////////////////
         // begin
         ////////////////////////////////////////////////////////////
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
         typedef values_type::size_type size_type;
         size_type size() const
         { return values.size(); }

         ////////////////////////////////////////////////////////////
         // empty
         ////////////////////////////////////////////////////////////
         bool empty() const
         { return values.empty(); }
         //@endgroup
      };
   };
};

#endif

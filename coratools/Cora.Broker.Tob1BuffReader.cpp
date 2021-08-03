/* Cora.Broker.Tob1BuffReader.cpp

   Copyright (C) 2009, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Saturday 09 May 2009
   Last Change: Saturday 01 September 2018
   Last Commit: $Date: 2018-09-04 12:20:54 -0600 (Tue, 04 Sep 2018) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Broker.Tob1BuffReader.h"
#include "Csi.Utils.h"


namespace Cora
{
   namespace Broker
   {
      ////////////////////////////////////////////////////////////
      // class Tob1BuffReader definitions
      ////////////////////////////////////////////////////////////
      Tob1BuffReader::Tob1BuffReader(
         void const *tob1_header,
         uint4 tob1_header_len,
         value_factory_handle value_factory):
         DataFileReader(value_factory),
         read_offset(0)
      {
         header.bind(new Tob1Header(tob1_header, tob1_header_len));
         header_sig = Csi::calcSigFor(tob1_header, header->get_header_end());
         record_descriptions[0] = header->make_record_desc();
      } // constructor


      Tob1BuffReader::~Tob1BuffReader()
      { }


      void Tob1BuffReader::open(
         StrAsc const &file_name, StrAsc const &labels_file_name)
      { throw Csi::MsgExcept("open not support for memory streams"); }


      Tob1BuffReader::read_outcome_type Tob1BuffReader::read_next_record(
         record_handle &dest,
         bool *file_mark_after_record,
         bool *remove_mark_after_record,
         uint4 array_id)
      {
         read_outcome_type rtn = read_outcome_success;
         if(read_offset < records_buff.length())
         {
            header->read_record(
               dest,
               records_buff.getContents() + read_offset,
               header->get_record_len());
            read_offset += header->get_record_len();
            if(file_mark_after_record)
               *file_mark_after_record = false;
            if(remove_mark_after_record)
               *remove_mark_after_record = false;
         }
         else
            rtn = read_outcome_end_of_file;
         return rtn;
      } // read_next_record


      void Tob1BuffReader::generate_index(index_type &index, bool &should_abort, uint4 *next_record_no)
      { }


      bool Tob1BuffReader::has_time_stamp() const
      { return header->has_time_stamp(); }


      bool Tob1BuffReader::has_record_no() const
      { return header->has_record_no(); }


      int8 Tob1BuffReader::get_header_len()
      { return header->get_header_end(); }


      uint2 Tob1BuffReader::get_header_sig()
      { return header_sig; }

      
      int8 Tob1BuffReader::get_data_len()
      { return records_buff.length(); }


      int8 Tob1BuffReader::get_data_offset()
      { return read_offset; }


      void Tob1BuffReader::seek_data(int8 offset, bool search_for_prev_record)
      {
         read_offset = static_cast<uint4>(offset) % header->get_record_len();
         if(read_offset > records_buff.length())
            read_offset = records_buff.length();
      } // seek_data


      void Tob1BuffReader::set_data(void const *buff, uint4 buff_len)
      {
         if(buff_len > 0 && buff_len % header->get_record_len() == 0)
         {
            records_buff.setContents(buff, buff_len);
            read_offset = 0;
         }
         else
            throw std::invalid_argument("invalid buffer length - must be evenly divisible by the record length");
      } // set_data
   };
};



/* Cora.Broker.Tob1Header.cpp

   Copyright (C) 2009, 2010 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 02 March 2009
   Last Change: Thursday 22 July 2010
   Last Commit: $Date: 2011-07-15 13:23:26 -0600 (Fri, 15 Jul 2011) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Broker.Tob1Header.h"
#include "Csi.BuffStream.h"
#include "Csi.CsvRec.h"
#include "Csi.Utils.h"
#include "Csi.ByteOrder.h"


namespace Cora
{
   namespace Broker
   {
      Tob1Header::Tob1Header(void const *buff, size_type buff_len):
         record_len(0)
      {
         // read the header fields
         Csi::IBuffStream temp(reinterpret_cast<char const *>(buff), buff_len);
         Csi::CsvRec environment, field_names, units, processes, data_types;
         
         environment.read(temp);
         if(environment.at(0) != "TOB1")
            throw std::invalid_argument("invalid TOB1 structure");
         station_name = environment.at(1);
         model = environment.at(2);
         serial_no = environment.at(3);
         os_version = environment.at(4);
         dld_name = environment.at(5);
         dld_sig = environment.at(6);
         table_name = environment.at(7);
         field_names.read(temp);
         units.read(temp);
         processes.read(temp);
         data_types.read(temp);
         if(field_names.empty())
            throw std::invalid_argument("too few field names");
         if(units.size() != field_names.size())
            throw std::invalid_argument("too few units");
         if(processes.size() != field_names.size())
            throw std::invalid_argument("too few processes");
         if(data_types.size() != field_names.size())
            throw std::invalid_argument("too few data types");
         header_end = static_cast<uint4>(temp.tellg());

         // we now need to build up our list of fields
         fields.reserve(field_names.size());
         for(uint4 i = 0; i < field_names.size(); ++i)
         {
            fields.push_back(
               Tob1Field(
                  field_names[i], data_types[i], units[i], processes[i]));
            record_len += fields.back().get_field_size();
         }
      } // constructor


      Tob1Header::record_desc_handle Tob1Header::make_record_desc()
      {
         record_desc_handle rtn(
            new RecordDesc(station_name.c_str(), table_name.c_str()));
         rtn->model_name = model;
         rtn->serial_number = serial_no;
         rtn->os_version = os_version;
         rtn->dld_name = dld_name;
         rtn->dld_signature = dld_sig;
         data_start_pos = 0;
         seconds_pos = nsecs_pos = record_pos = 0xFFFFFFFF;
         for(const_iterator fi = begin(); fi != end(); ++fi)
         {
            value_type const &field = *fi;
            if(field.get_field_name() == "SECONDS" && seconds_pos == 0xFFFFFFFF)
            {
               seconds_pos = data_start_pos;
               data_start_pos += field.get_field_size();
            }
            else if(field.get_field_name() == "NANOSECONDS" && nsecs_pos == 0xFFFFFFFF)
            {
               nsecs_pos = data_start_pos;
               data_start_pos += field.get_field_size(); 
            }
            else if(field.get_field_name() == "RECORD" && record_pos == 0xFFFFFFFF)
            {
               record_pos = data_start_pos;
               data_start_pos += field.get_field_size();
            }
            else
            {
               for(uint4 i = 0; i == 0 || i < field.get_string_len(); ++i)
               {
                  Broker::RecordDesc::value_type value_desc(new ValueDesc);
                  value_desc->name = field.get_field_name().c_str();
                  value_desc->data_type = lgr_to_csi_type(field.get_data_type());
                  value_desc->units = field.get_units().c_str();
                  value_desc->process = field.get_process().c_str();
                  value_desc->modifying_cmd = 0;
                  if(field.get_string_len() > 0)
                     value_desc->array_address.push_back(i + 1);
                  rtn->values.push_back(value_desc);
               }
            }
         }
         return rtn;
      } // make_record_desc


      Tob1Header::record_handle Tob1Header::make_record()
      {
         if(record_desc == 0)
            record_desc = make_record_desc();
         return new Broker::Record(record_desc, factory);
      } // make_record

      
      void Tob1Header::read_record(
         record_handle &record, void const *buff_, uint4 buff_len)
      {
         Csi::LgrDate stamp;
         uint4 record_no = 0;
         byte const *buff = static_cast<byte const *>(buff_);
         int4 seconds = 0;
         uint4 nsecs = 0;
         if(seconds_pos != 0xFFFFFFFF)
         {
            memcpy(&seconds, buff + seconds_pos, sizeof(seconds));
            if(Csi::is_big_endian())
               Csi::reverse_byte_order(&seconds, sizeof(seconds));
         }
         if(nsecs_pos != 0xFFFFFFFF)
         {
            memcpy(&nsecs, buff + nsecs_pos, sizeof(nsecs));
            if(Csi::is_big_endian())
               Csi::reverse_byte_order(&nsecs, sizeof(nsecs)); 
         }
         if(record_pos != 0xFFFFFFFF)
         {
            memcpy(&record_no, buff + record_pos, sizeof(record_no));
            if(Csi::is_big_endian())
               Csi::reverse_byte_order(&record_no, sizeof(record_no));
         }
         stamp = static_cast<int8>(seconds * Csi::LgrDate::nsecPerSec) + nsecs;
         record->read(record_no, 0, stamp, buff + data_start_pos, buff_len - data_start_pos);
      } // read_record
   };
};


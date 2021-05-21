/* Cora.Broker.Record.cpp

   Copyright (C) 1998, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 25 August 1999
   Last Change: Friday 07 February 2020
   Last Commit: $Date: 2020-02-07 16:05:54 -0600 (Fri, 07 Feb 2020) $ 
   Committed by: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include <sstream>
#include "Cora.Broker.Record.h"
#include "Cora.Broker.RecordDesc.h"
#include "Cora.Broker.Value.h"
#include "Cora.Broker.ValueFactory.h"
#include "Cora.Broker.ValueDesc.h"
#include "Csi.Utils.h"
#include "Csi.Xml.Element.h"


namespace Cora
{
   namespace Broker
   {
      ////////////////////////////////////////////////////////////
      // class Record definitions
      ////////////////////////////////////////////////////////////
      Record::Record(desc_handle &description_, ValueFactory &factory):
         description(description_),
         tob1_native(true),
         record_no(0),
         file_mark_no(0)
      {
         // allocate the values storage
         values_storage_len = description->get_record_len();
         values_storage = new byte[values_storage_len];
         formatter.imbue(std::locale::classic());
         
         // create a new value to correspond with each value description
         value_handle last_value;
         for(RecordDesc::iterator di = description->begin(); 
             di != description->end(); 
             di++)
         {
            Csi::SharedPtr<ValueDesc> &value_desc = *di;
            if(last_value != 0 &&
               last_value->get_name() == value_desc->name)
            {
               if(!last_value->combine_with_adjacent_values(value_desc))
               {
                  last_value = factory.make_value(value_desc);
                  values.push_back(last_value);
               }
            }
            else
            {
               last_value = factory.make_value(*di);
               values.push_back(last_value);
            }
         }
         uint4 offset = 0;
         for(values_type::iterator vi = values.begin();
             vi != values.end();
             ++vi)
         {
            offset += (*vi)->set_storage(values_storage + offset);
            if(tob1_native)
               tob1_native = (*vi)->is_tob1_native();
         }
      } // constructor


      Record::Record(Record &other):
         description(other.description),
         stamp(other.stamp),
         record_no(other.record_no),
         file_mark_no(other.file_mark_no),
         values_storage_len(other.values_storage_len),
         tob1_native(other.tob1_native)
      {
         uint4 offset = 0;

         formatter.imbue(std::locale::classic());
         values_storage = new byte[values_storage_len];
         memcpy(values_storage,other.values_storage,values_storage_len);
         for(values_type::iterator vi = other.begin();
             vi != other.end();
             ++vi)
         {
            values.push_back(value_handle((*vi)->clone()));
            offset += values.back()->set_storage(values_storage + offset);
         }
      } // copy constructor


      Record::~Record()
      { delete[] values_storage; }


      Record &Record::operator =(Record const &other)
      {
         if(description.get_rep() == other.description.get_rep() &&
            values.size() == other.values.size() &&
            values_storage_len == other.values_storage_len)
         {
            // copy the record information
            stamp = other.stamp;
            record_no = other.record_no;
            file_mark_no = other.file_mark_no;
            memcpy(values_storage,other.values_storage,values_storage_len);
            tob1_native = other.tob1_native;
         }
         else
            throw std::invalid_argument("Cannot copy records with different meta-data");
         return *this;
      } // copy operator


      namespace
      {
         struct value_has_name
         {
            StrUni const &name;
            value_has_name(StrUni const &name_):
               name(name_)
            { }

            bool operator ()(Record::value_type const &value)
            { return value->get_name() == name; }
         };


         struct value_has_formatted_name
         {
            StrAsc const &name;
            Csi::OStrAscStream temp;
            value_has_formatted_name(StrAsc const &name_):
               name(name_)
            { }
            value_has_formatted_name(value_has_formatted_name const &other):
               name(other.name)
            { }

            bool operator ()(Record::value_type const &value)
            {
               temp.str("");
               value->format_name_ex(temp);
               return name == temp.str();
            }
         };
      };
      
               
      Record::iterator Record::find(StrUni const &name)
      {
         return std::find_if(begin(), end(), value_has_name(name));
      } // find


      Record::iterator Record::find_formatted(StrAsc const &name)
      {
         return std::find_if(begin(), end(), value_has_formatted_name(name));
      } // find_ex

      
      StrUni const &Record::get_broker_name() const
      { return description->broker_name; }


      StrUni const &Record::get_table_name() const
      { return description->table_name; }


      bool Record::read(
         Csi::Messaging::Message &msg,
         bool read_file_mark)
      {
         int8 nsec;
         uint4 num_bytes;
         bool rtn;

         rtn = msg.readUInt4(record_no);
         if(rtn && read_file_mark)
            rtn = msg.readUInt4(file_mark_no);
         else
            file_mark_no = 0;
         if(rtn)
            rtn = msg.readInt8(nsec) && msg.readUInt4(num_bytes);
         if(rtn)
         {
            stamp = nsec;
            rtn = msg.readBlock(values_storage,values_storage_len);
         }
         return rtn;
      } // read


      bool Record::read(
         uint4 record_no_,
         uint4 file_mark_no_,
         Csi::LgrDate const &stamp_,
         void const *buffer,
         uint4 buffer_len)
      {
         bool rtn = false;
         if(buffer_len >= values_storage_len)
         {
            // store the known values
            record_no = record_no_;
            file_mark_no = file_mark_no_;
            stamp = stamp_;
            memcpy(values_storage,buffer,values_storage_len);
            rtn = true;
         }
         return rtn;
      } // read


      void Record::write_toaci1_format(StrBin &buffer)
      {
         formatter.str("");
         write_comma_delimited(formatter,true);
         buffer.setContents(formatter.str().c_str(),formatter.str().length());
      } // write_toaci1_format


      void Record::write_toa5_format(
         StrBin &buffer,
         bool write_stamp,
         bool write_record_no,
         uint4 begin_value,
         uint4 end_value,
         bool midnight_is_2400)
      {
         formatter.str("");
         write_comma_delimited(
            formatter,
            write_stamp,
            write_record_no,
            false,
            begin_value,
            end_value,
            midnight_is_2400);
         buffer.setContents(formatter.str().c_str(),formatter.str().length());
      } // write_toa5_format


      void Record::write_tob1_format(
         StrBin &buffer,
         bool write_stamp,
         bool write_record_no,
         uint4 begin_value,
         uint4 end_value)
      {
         // format the record header
         if(write_stamp || write_record_no)
         {
            int4 seconds = stamp.get_sec();
            int4 nanoseconds = stamp.nsec();
            uint4 rec_no = record_no;
            
            if(Csi::is_big_endian())
            {
               Csi::reverse_byte_order(&seconds,sizeof(seconds));
               Csi::reverse_byte_order(&nanoseconds,sizeof(nanoseconds));
               Csi::reverse_byte_order(&rec_no,sizeof(rec_no));
            }
            if(write_stamp)
            {
               buffer.append(&seconds,sizeof(seconds));
               buffer.append(&nanoseconds,sizeof(nanoseconds));
            }
            if(write_record_no)
               buffer.append(&rec_no,sizeof(rec_no));
         }

         if(tob1_native && begin_value == 0 && end_value >= values.size())
         {
            buffer.append(values_storage,values_storage_len);
         }
         else
         {
            // format each of the values
            uint4 value_no(0);
            for(values_type::iterator vi = values.begin();
                vi != values.end();
                ++vi)
            {
               value_handle &value = *vi;
               if(value_no >= begin_value && value_no < end_value)
                  value->write_tob1(buffer);
               ++value_no;
            }
         }
      } // write_tob1_format


      void Record::write_custom_csv(
         StrBin &buffer,
         CustomCsvOptions options,
         uint4 begin_value,
         uint4 end_value)
      {
         int fields_count = 0;
         uint4 value_no(0);
         formatter.str("");
         if(options.get_include_array_id() != 0)
         {
            ++fields_count;
            formatter << options.get_array_id();
         }
         if(options.get_include_timestamp())
         {
            if(fields_count++ != 0)
               formatter << ",";
            stamp.format_custom_classic(
               formatter,
               options.get_timestamp_format_flags());
         }
         for(values_type::iterator vi = values.begin();
             vi != values.end();
             ++vi)
         {
            if(value_no >= begin_value && value_no < end_value)
            {
               value_handle &value = *vi; 
               if(fields_count++ != 0)
                  formatter << ",";
               if(value->quote_when_formatting(&options))
                  formatter << "\"";
               value->format(formatter, false, 0, &options, true, true, true);
               if(value->quote_when_formatting(&options))
                  formatter << "\"";
            }
            ++value_no;
         }
         formatter << "\r\n";
         buffer.setContents(
            formatter.str().c_str(),
            formatter.str().length());
      } // write_custom_csv
      

      void Record::write_xml_format(
         StrBin &out,
         XmlOptions options,
         uint4 begin_value,
         uint4 end_value)
      {
         using Csi::Xml::Element;
         using Csi::Xml::OAttributeStream;
         using Csi::Xml::OElementStream;
         typedef Csi::SharedPtr<Element> element_handle;
         element_handle record(new Element(L"r"));
         uint4 value_no = 0;
         uint4 values_count(0);
         StrUni time_format(L"%Y-%m-%d %H:%M:%S%x");
         
         if(options.get_midnight_is_2400())
            time_format = L"%Y-%m-%d %#H:%M:%S%x";
         if(options.get_include_record_no())
         {
            OAttributeStream s(record.get_rep(),L"no");
            s.imbue(formatter.getloc());
            s << record_no;
         }
         if(options.get_include_time_stamp())
         {
            OAttributeStream s(record.get_rep(),L"time");
            s.imbue(formatter.getloc());
            stamp.format(s, time_format);
         }
         for(values_type::iterator vi = values.begin(); vi != values.end(); ++vi)
         {
            if(value_no >= begin_value && value_no < end_value)
            {
               value_handle &value = *vi;
               std::wostringstream vname;
               vname << L"v" << (values_count + 1);
               element_handle v(record->add_element(vname.str().c_str()));
               OElementStream vo(v.get_rep());
               
               vo.imbue(formatter.getloc());
               if(options.get_include_value_name())
               {
                  OAttributeStream s(v.get_rep(),L"n");
                  s.imbue(formatter.getloc());
                  value->format_name_ex(s,true,L"(",L"",L",",L")");
               } 
               value->format(vo, false, time_format.c_str(), 0, false, true);
               ++values_count;
            }
            ++value_no;
         }
         formatter.str("");
         record->output(formatter, false);
         formatter << "\n";
         out.append(formatter.str().c_str(),formatter.str().length());
      } // write_xml_format


      void Record::write_json_format(
         StrBin &out,
         uint4 begin_value,
         uint4 end_value,
         bool append)
      {
         uint4 value_no(0);
         formatter.str("");
         formatter << "      {\n"
                   << "        \"no\": " << record_no << ",\n"
                   << "        \"time\": ";
         stamp.format(formatter, "\"%Y-%m-%dT%H:%M:%S%x\",\n");
         formatter << "        \"vals\": [ ";
         for(values_type::iterator vi = values.begin(); vi != values.end(); ++vi)
         {
            if(value_no >= begin_value && value_no < end_value)
            {
               value_type &value(*vi);
               if(value_no != begin_value)
                  formatter << ", ";
               value->format_json(formatter);
            }
            else if(value_no > end_value)
               break;
            ++value_no;
         }
         formatter << " ]\n"
                   << "      }";
         if(!append)
            out.setContents(formatter.c_str(), formatter.length());
         else
            out.append(formatter.c_str(), formatter.length());
      } // write_json_format


      Csi::Json::ObjectHandle Record::write_json(uint4 begin_value, uint4 end_value)
      {
         Csi::Json::ObjectHandle rtn(new Csi::Json::Object);
         Csi::Json::ArrayHandle values_json(new Csi::Json::Array);
         rtn->set_property_number("no", record_no);
         rtn->set_property_date("time", stamp);
         rtn->set_property("vals", values_json.get_handle());
         for(auto vi = values.begin(); vi != values.end(); ++vi)
         {
            uint4 value_no((uint4)std::distance(values.begin(), vi));
            if(value_no >= begin_value && value_no < end_value)
            {
               value_type &value(*vi);
               values_json->push_back(value->write_json());
            }
            else if(value_no > end_value)
               break;
         }
         return rtn;
      } // write_json
      
      
      void Record::write_comma_delimited(
         std::ostream &out,
         bool write_stamp,
         bool write_record_no,
         bool specials_as_numbers,
         uint4 begin_value,
         uint4 end_value,
         bool midnight_is_2400,
         bool quote_strings)
      {
         int fields_printed = 0;
         uint4 value_no(0);
         StrAsc time_format("%Y-%m-%d %H:%M:%S%x");

         if(midnight_is_2400)
            time_format = "%Y-%m-%d %#H:%M:%S%x";
         if(write_stamp)
         {
            out << "\"";
            stamp.format(out, time_format);
            out << "\"";
            ++fields_printed;
         }
         if(write_record_no)
         {
            if(fields_printed++ != 0)
               out << ",";
            out << record_no;
         }
         for(values_type::iterator vi = values.begin();
             vi != values.end();
             ++vi)
         {
            if(value_no >= begin_value && value_no < end_value)
            {
               value_handle &value = *vi;
               if(fields_printed++ != 0)
                  out << ",";
               if(quote_strings && value->quote_when_formatting())
                  out << "\"";
               value->format(out, false, time_format.c_str(), 0, specials_as_numbers, true, true);
               if(quote_strings && value->quote_when_formatting())
                  out << "\"";
            }
            ++value_no;
         }
         out << "\r\n";
      } // write_comma_delimited


      void Record::set_null_values()
      {
         for(values_type::iterator vi = values.begin(); vi != values.end(); ++vi)
            (*vi)->set_to_null();
      } // set_null_values


      uint2 Record::calc_sig() const
      {
         int8 stamp_nsec = stamp.get_nanoSec();
         uint2 rtn = Csi::calcSigFor(&stamp_nsec, sizeof(stamp_nsec));
         rtn = Csi::calcSigFor(&record_no, sizeof(record_no), rtn);
         rtn = Csi::calcSigFor(values_storage, values_storage_len, rtn);
         return rtn;
      } // calc_sig
   };
};

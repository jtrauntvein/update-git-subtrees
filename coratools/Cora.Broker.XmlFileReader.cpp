/* Cora.Broker.XmlFileReader.cpp

   Copyright (C) 2006, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 07 September 2006
   Last Change: Friday 29 April 2016
   Last Commit: $Date: 2016-07-11 09:29:07 -0600 (Mon, 11 Jul 2016) $
   Last Changed by: $Author: jon $

*/

#pragma warning(disable: 4996)
#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Broker.XmlFileReader.h"
#include "Csi.Utils.h"
#include "Csi.BuffStream.h"
#include "Csi.Xml.Element.h"
#include "Csi.StrAscStream.h"


namespace Cora
{
   namespace Broker
   {
      namespace
      {
         ////////////////////////////////////////////////////////////
         // function xml_type_to_csi_type
         ////////////////////////////////////////////////////////////
         CsiDbTypeCode xml_type_to_csi_type(StrUni const &xml_type)
         {
            CsiDbTypeCode rtn = CsiUnknown;
            bool big_endian = Csi::is_big_endian();
            
            if(xml_type == L"xsd:string")
               rtn = CsiAscii;
            else if(xml_type == L"xsd:long")
               rtn = big_endian ? CsiInt8 : CsiInt8Lsf;
            else if(xml_type == L"xsd:int")
               rtn = big_endian ? CsiInt4 : CsiInt4Lsf;
            else if(xml_type == L"xsd:unsignedInt")
               rtn = big_endian ? CsiUInt4 : CsiUInt4Lsf;
            else if(xml_type == L"xsd:short")
               rtn = big_endian ? CsiInt2 : CsiInt2Lsf;
            else if(xml_type == L"xsd:unsignedShort")
               rtn = big_endian ? CsiUInt2 : CsiUInt2Lsf;
            else if(xml_type == L"xsd:byte")
               rtn = CsiInt1;
            else if(xml_type == L"xsd:unsignedByte")
               rtn = CsiUInt1;
            else if(xml_type == L"xsd:float")
               rtn = big_endian ? CsiIeee4 : CsiIeee4Lsf;
            else if(xml_type == L"xsd:double")
               rtn = big_endian ? CsiIeee8 : CsiIeee8Lsf;
            else if(xml_type == L"xsd:boolean")
               rtn = CsiBool;
            else if(xml_type == L"xsd:dateTime")
               rtn = big_endian ? CsiLgrDate : CsiLgrDateLsf;
            return rtn;
         } // xml_type_to_csi_type


         ////////////////////////////////////////////////////////////
         // extract_next_record
         //
         // Parses the file looking for the next begin and end record tags.  Returns the content
         // between them.  Will return an empty string if no tags are found.
         ////////////////////////////////////////////////////////////
         void extract_next_record(
            StrAsc &dest,
            FILE *in)
         {
            int ch;
            enum state_type {
               state_before_r_begin,
               state_within_possible_begin,
               state_in_r,
               state_in_r_tag,
               state_in_possible_end,
               state_complete
            } state = state_before_r_begin;
            StrAsc tag_name;

            while(state != state_complete && (ch = fgetc(in)) != EOF)
            {
               switch(state)
               {
               case state_before_r_begin:
                  if(ch == '<')
                  {
                     tag_name.cut(0);
                     tag_name.append('<');
                     state = state_within_possible_begin;
                  }
                  break;
                  
               case state_within_possible_begin:
                  if(ch == '>' || isspace(ch))
                  {
                     if(tag_name == "<r")
                     {
                        state = state_in_r;
                        dest = tag_name;
                        dest.append(static_cast<char>(ch));
                        tag_name.cut(0);
                     }
                     else
                     {
                        state = state_before_r_begin;
                        tag_name.cut(0);
                     }
                  }
                  else
                     tag_name.append(static_cast<char>(ch));
                  break;
                  
               case state_in_r:
                  dest.append(static_cast<char>(ch));
                  if(ch == '<')
                  {
                     tag_name.cut(0);
                     tag_name.append('<');
                     state = state_in_r_tag;
                  }
                  break;
                     
               case state_in_r_tag:
                  dest.append(static_cast<char>(ch));
                  if(ch == '/')
                  {
                     tag_name.append('/');
                     state = state_in_possible_end;
                  }
                  else if(!isspace(ch))
                     state = state_in_r;
                  break;
                  
               case state_in_possible_end:
                  dest.append(static_cast<char>(ch));
                  if(ch == '>')
                  {
                     if(tag_name == "</r")
                        state = state_complete;
                     else
                        state = state_in_r;
                  }
                  else if(!isspace(ch))
                     tag_name.append(static_cast<char>(ch));
                  break;
               }
            }
         } // extract_next_record
      };

      
      ////////////////////////////////////////////////////////////
      // class XmlFileReader definitions
      ////////////////////////////////////////////////////////////
      XmlFileReader::XmlFileReader(value_factory_handle value_factory_):
         DataFileReader(value_factory_),
         input(0),
         is_sleeping(false),
         header_sig(0),
         hibernate_pos(0),
         data_sig(0),
         data_len(0),
         data_has_time_stamp(false),
         data_has_record_no(false)
      { }

      
      void XmlFileReader::open(
         StrAsc const &file_name, StrAsc const &labels_file_name)
      {
         // if we are already in an open state, we will close first
         if(input != 0)
            close();

         // we will first try to open the input file
         input = Csi::open_file(file_name.c_str(), "rb");
         if(input == 0)
            throw DataFileException(DataFileException::failure_cannot_open);

         try
         {
            // now extract the header from the file
            parse_buff.cut(0);
            Csi::extract_csi_xml_header(parse_buff,input);
            header_len = ftell(input);

            // we need to parse the header
            Csi::IBuffStream header_str(parse_buff.c_str(),parse_buff.length());
            Csi::Xml::Element header_xml(L"head");

            header_xml.input(header_str);

            // we can now initialise the record description with information from the header
            using namespace Csi::Xml;
            using Csi::SharedPtr;
            SharedPtr<Element> environment(header_xml.find_elem(L"environment"));
            SharedPtr<Element> station_name(environment->find_elem(L"station-name"));
            SharedPtr<Element> table_name(environment->find_elem(L"table-name"));
            Element::iterator model_it = environment->find(L"model");
            Element::iterator serial_no_it = environment->find(L"serial-no");
            Element::iterator os_version_it = environment->find(L"os-version");
            Element::iterator dld_name_it = environment->find(L"dld-name");
            Element::iterator dld_sig_it = environment->find(L"dld-sig");
            
            record_description_handle record_description(
               new RecordDesc(
                  station_name->get_cdata(),
                  table_name->get_cdata()));
            record_descriptions[0] = record_description;

            if(model_it != environment->end())
               record_description->model_name = (*model_it)->get_cdata_str();
            if(serial_no_it != environment->end())
               record_description->serial_number = (*serial_no_it)->get_cdata_str();
            if(os_version_it != environment->end())
               record_description->os_version = (*os_version_it)->get_cdata_str();
            if(dld_name_it != environment->end())
               record_description->dld_name = (*dld_name_it)->get_cdata_str();
            if(dld_sig_it != environment->end())
               record_description->dld_signature = (*dld_sig_it)->get_cdata_str();

            // we are now ready to process the field names
            SharedPtr<Element> fields(header_xml.find_elem(L"fields"));
            StrAsc field_name;
            StrUni field_type;
            StrUni field_units;
            StrUni field_process;
            
            for(Element::iterator fi = fields->begin(); fi != fields->end(); ++fi)
            {
               // make sure that this is a field element
               SharedPtr<Element> &field = *fi;
               Cora::Broker::ValueDesc *value_desc;
               if(field->get_name() != L"field")
                  continue;
               field_name = field->get_attr_str(L"name");
               field_type = field->get_attr_wstr(L"type");
               field_units = field->get_attr_wstr(L"units");
               field_process = field->get_attr_wstr(L"process");
               process_field_name(field_name.c_str());
               value_desc = record_description->values.back().get_rep();
               value_desc->data_type = xml_type_to_csi_type(field_type);
               value_desc->units = field_units;
               value_desc->process = field_process;
               if(value_desc->data_type == CsiUnknown)
                  throw_init_failure(DataFileException::failure_corrupt_file);
               if(value_desc->data_type == CsiAscii)
               {
                  // now duplicate the value 63 times, this will give us a total of 64 character
                  // values (these will be merged when the record is formed).  If a string-len tag
                  // is added to the field element, we will be able to better predict the size of
                  // the string.
                  uint4 string_len(64);
                  if(field->has_attribute(L"string-len"))
                     string_len = field->get_attr_uint4(L"string-len");
                  value_desc->array_address.push_back(1);
                  for(uint4 i = 0; i < string_len - 1; ++i)
                  {
                     Csi::SharedPtr<ValueDesc> str_value(new ValueDesc);
                     str_value->name = value_desc->name;
                     str_value->data_type = value_desc->data_type;
                     str_value->modifying_cmd = value_desc->modifying_cmd;
                     str_value->units = value_desc->units;
                     str_value->process = value_desc->process;
                     str_value->array_address = value_desc->array_address;
                     str_value->array_address.back() = i + 2;
                     record_description->values.push_back(str_value);
                  }
               }
            }

            try
            {
               // we need to read the first record element to determine whether the file has time
               // stamps and record number.  we will read the content of the next record element. 
               parse_buff.cut(0);
               extract_next_record(parse_buff, input);
               Csi::file_seek(input, header_len, SEEK_SET);
               
               ////////////////////////////////////////////////////////////
               // now try to parse the element
               ////////////////////////////////////////////////////////////
               Csi::IBuffStream record_str(parse_buff.c_str(), parse_buff.length());
               Element record_xml(L"r");
               record_xml.input(record_str);
               data_has_time_stamp = record_xml.has_attribute(L"time");
               data_has_record_no = record_xml.has_attribute(L"no");
            }
            catch(std::exception &)
            {
               data_has_time_stamp = false;
               data_has_record_no = false;
            }
         }
         catch(DataFileException &)
         { throw; }
         catch(std::exception &)
         { throw_init_failure(DataFileException::failure_corrupt_file); }
         this->file_name = file_name; 
      } // open

      
      void XmlFileReader::close()
      {
         if(input)
         {
            fclose(input);
            input = 0;
         }
         is_sleeping = false;
         record_descriptions.clear();
      } // close


      void XmlFileReader::hibernate()
      {
         if(input != 0 && !is_sleeping)
         {
            int8 file_len(Csi::long_file_length(input));
            data_len = 0;
            data_sig = 0xAAAA;
            if(file_len > header_len)
            {
               hibernate_pos = Csi::file_tell(input);
               Csi::file_seek(input, 0, SEEK_SET);
               header_sig = Csi::calc_file_sig(input, header_len);
               if(file_len > header_len)
               {
                  data_len = 1024;
                  if(file_len - header_len < data_len)
                     data_len = file_len - header_len;
                  data_sig = Csi::calc_file_sig(input, data_len);
               }
            }
            fclose(input);
            input = 0;
            is_sleeping = true;
         }
      } // hibernate


      bool XmlFileReader::wake_up(bool &all_data_overwritten)
      {
         bool rtn = false;
         if(is_sleeping)
         {
            input = Csi::open_file(file_name.c_str(), "rb");
            if(input != 0)
            {
               uint2 new_header_sig(Csi::calc_file_sig(input, header_len));
               if(new_header_sig == header_sig)
               {
                  int8 file_len(Csi::long_file_length(input));
                  int8 new_data_len(file_len - header_len);
                  if(new_data_len >= data_len)
                  {
                     uint2 new_data_sig(Csi::calc_file_sig(input, data_len));
                     if(new_data_sig == data_sig && hibernate_pos <= file_len)
                     {
                        all_data_overwritten = false;
                        Csi::file_seek(input, hibernate_pos, SEEK_SET);
                     }
                     else
                     {
                        seek_data(0);
                        all_data_overwritten = true;
                     }
                  }
                  else
                  {
                     seek_data(0);
                     all_data_overwritten = true;
                  }
                  rtn = true;
                  is_sleeping = false;
               }
               else
               {
                  fclose(input);
                  input = 0;
               }
            }
         }
         return rtn;
      } // wake_up

      
      XmlFileReader::read_outcome_type XmlFileReader::read_next_record(
         record_handle &destination,
         bool *file_mark_after_record,
         bool *remove_mark_after_record,
         uint4 array_id)
      {
         read_outcome_type rtn = read_outcome_not_initialised;
         if(file_mark_after_record)
            *file_mark_after_record = false;
         if(remove_mark_after_record)
            *remove_mark_after_record = false;
         if(input != 0)
         {
            try
            {
               // read the next record section from the file using XML tag syntax to mark the
               // beginning and end of the record (this is cheaper than parsing the whole file).
               parse_buff.cut(0);
               extract_next_record(parse_buff, input);
               if(parse_buff.length())
               {
                  // Experience has shown that using the standard Xml::Element class to parse the
                  // record can result in excessive string copying.  As a result, we will break down
                  // the attributes and element values ourselves.  We will first try to isolate the
                  // record number and timestamp attributes
                  static char const record_no_name[] = "no=\"";
                  static char const time_name[] = "time=\"";
                  size_t record_tag_end = parse_buff.find(">");
                  size_t record_no_start = parse_buff.find(record_no_name);
                  size_t time_start = parse_buff.find(time_name);
                  StrAsc temp;

                  if(record_no_start < record_tag_end)
                     destination->set_record_no(
                        strtoul(parse_buff.c_str() + record_no_start + sizeof(record_no_name) - 1, 0, 10));
                  if(time_start < record_tag_end)
                  {
                     size_t time_end = parse_buff.find("\"", time_start + sizeof(time_name));
                     size_t time_buff_len = time_end - (time_start + sizeof(time_name) - 1);
                     parse_buff.sub(last_time_str, time_start + sizeof(time_name) - 1, time_buff_len);
                     destination->set_stamp(Csi::LgrDate::fromStr(last_time_str.c_str()));
                  }
                  else
                     destination->set_stamp(Csi::LgrDate::system());

                  // we now need to fill in each of the values for the record
                  size_t last_value_end = record_tag_end + 1;
                  Csi::OStrAscStream value_name;
                  StrAsc value_buff;
                  for(uint4 i = 0; i < destination->size(); ++i) 
                  {
                     // we need to search for the start of the value element
                     value_name.str("");
                     value_name << "<v" << (i + 1);
                     size_t value_start = parse_buff.find(value_name.str().c_str(), last_value_end);
                     if(value_start >= parse_buff.length())
                        throw Csi::MsgExcept("record has invalid value");

                     // we also need to search for the end of the value element.  If there is no
                     // end, we will consider the value to be empty.
                     value_name.str("");
                     value_name << "</v" << (i + 1) << ">";
                     size_t value_end = parse_buff.find(value_name.str().c_str(), value_start);
                     if(value_end < parse_buff.length())
                     {
                        // we now need to locate the position of the value element cdata
                        size_t cdata_begin = parse_buff.find(">", value_start) + 1;
                        if(cdata_begin <= value_end)
                        {
                           Record::value_type &value = (*destination)[i];
                           value_buff.cut(0);
                           Csi::Xml::input_xml_data(value_buff, parse_buff.c_str() + cdata_begin, value_end - cdata_begin);
                           value->read_text(value_buff.c_str());
                        }
                        else if(cdata_begin > value_end)
                           throw Csi::MsgExcept("value syntax error");
                     }
                     else
                     {
                        // we couldn't find the value end so this element is likely to be empty.  We
                        // will search, instead, for the empty element terminator.
                        value_end = parse_buff.find("/>", value_start);
                        if(value_end < parse_buff.length())
                        {
                           Record::value_type &value = (*destination)[i];
                           value->read_text("");
                        }
                        else
                           throw Csi::MsgExcept("value syntax error");
                     }
                     last_value_end = value_end;
                  }
                  rtn = read_outcome_success;
               }
            }
            catch(std::exception &)
            { rtn = read_outcome_invalid_record; } 
         }
         return rtn;
      } // read_next_record


      void XmlFileReader::generate_index(index_type &index, bool &should_abort, uint4 *next_record_no)
      {
         int8 pos;
         StrAsc const record_no_name("no=\"");
         StrAsc const time_name("time=\"");
         StrAsc time_str;
         size_t record_no_start;
         size_t time_start;
         Csi::LgrDate time_stamp;
         uint4 record_no(0xffffffff);
         
         while(!feof(input) && !should_abort)
         {
            pos = get_data_offset();
            parse_buff.cut(0);
            extract_next_record(parse_buff, input);
            record_no_start = parse_buff.find(record_no_name.c_str());
            time_start = parse_buff.find(time_name.c_str());
            if(record_no_start < parse_buff.length())
               record_no = strtoul(parse_buff.c_str() + record_no_start + record_no_name.length(), 0, 10);
            else if(next_record_no)
               record_no = *next_record_no;
            else
               record_no = 0xffffffff;
            if(next_record_no)
               *next_record_no = record_no + 1;
            if(time_start < parse_buff.length())
            {
               size_t time_end(parse_buff.find("\"", time_start + time_name.length()));
               size_t time_buff_len(time_end - (time_start + time_name.length()));
               parse_buff.sub(time_str, time_start + time_name.length(), time_buff_len);
               time_stamp = Csi::LgrDate::fromStr(time_str.c_str());
               index.push_back(DataFileIndexEntry(pos, time_stamp, record_no));
            }
         }
      } // generate_index
      

      bool XmlFileReader::last_time_was_2400() const
      {
         bool rtn(last_time_str.find("T24:00") < last_time_str.length());
         return rtn;
      } // last_time_was_2400

      
      int8 XmlFileReader::get_header_len()
      { return header_len; }


      uint2 XmlFileReader::get_header_sig()
      {
         if(input == 0)
            throw DataFileException(DataFileException::failure_not_initialised);
         int8 current_pos = Csi::file_tell(input);
         Csi::file_seek(input, 0, SEEK_SET);
         uint2 rtn  = Csi::calc_file_sig(input, header_len);
         Csi::file_seek(input, current_pos, SEEK_SET);
         return rtn;
      } // get_header_sig

   
      int8 XmlFileReader::get_data_len()
      {
         if(input == 0)
            throw DataFileException(DataFileException::failure_not_initialised);
         return Csi::long_file_length(input) - header_len;
      } // get_data_len

      
      int8 XmlFileReader::get_data_offset()
      {
         if(input == 0)
            throw DataFileException(DataFileException::failure_not_initialised);
         return Csi::file_tell(input) - header_len;
      } // get_data_offset

      
      void XmlFileReader::seek_data(int8 data_offset, bool search_for_prev)
      {
         if(input == 0)
            throw DataFileException(DataFileException::failure_not_initialised);
         int8 file_offset = data_offset + header_len;
         int8 file_len = Csi::long_file_length(input);
         
         if(file_offset > file_len)
            file_offset = file_len;
         Csi::file_seek(input,file_offset,SEEK_SET);
         if(data_offset != 0 && search_for_prev)
         {
            char const search_token[] = "<r";
            Csi::search_file_backward(input, search_token, sizeof(search_token) - 1);
         }
      } // seek_data
   };
};

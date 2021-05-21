/* Cora.Broker.RecordDesc.cpp

   Copyright (C) 1998, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 25 August 1999
   Last Change: Wednesday 19 December 2018
   Last Commit: $Date: 2018-12-19 17:04:29 -0600 (Wed, 19 Dec 2018) $ 
   Committed by: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Broker.RecordDesc.h"
#include "Cora.Broker.ValueDesc.h"
#include "Csi.Messaging.Message.h"
#include "CsiTypes.h"
#include "Csi.Xml.Element.h"
#include "Csi.PolySharedPtr.h"
#include "Csi.Utils.h"
#include <iostream>


namespace Cora
{
   namespace Broker
   {
      ////////////////////////////////////////////////////////////
      // class RecordDesc definitions
      ////////////////////////////////////////////////////////////
      RecordDesc::RecordDesc(StrUni const &broker_name_, StrUni const &table_name_):
         broker_name(broker_name_),
         table_name(table_name_),
         last_record_len(0)
      { }

      RecordDesc::~RecordDesc()
      { }


      bool RecordDesc::read(
         Csi::Messaging::Message &ack,
         bool read_value_descriptions)
      {
         uint4 num_values;
         bool rtn;
         if(ack.readUInt4(num_values))
         {
            rtn = true;
            for(uint4 i = 0; i < num_values && rtn; i++)
            {
               value_desc_handle desc(new ValueDesc);
               if(desc->read(ack,read_value_descriptions))
                  values.push_back(desc);
               else
                  rtn = false;
            }
         }
         else
            rtn = false;
         return rtn;
      } // read


      void RecordDesc::write_toaci1_header(
         std::ostream &out,
         bool binary_stream)
      {
         // first line
         std::locale old_loc = out.getloc();
         out.imbue(std::locale::classic());
         out << "\"TOACI1\",\"" << broker_name << "\",\"" << table_name << "\""
             << (binary_stream ? "\r\n" : "\n");

         // second line (field names)
         values_type::iterator vi;
         out << "\"TMSTAMP\",\"RECNBR\",";
         write_field_names(out);
         out << (binary_stream ? "\r\n" : "\n");
         out.imbue(old_loc);
      } // write_toaci1_header


      void RecordDesc::write_toa5_header(
         std::ostream &out,
         bool binary_stream,
         bool write_stamp,
         bool write_record_no,
         uint4 begin_value,
         uint4 end_value)
      {
         // first line
         std::locale old_loc = out.getloc();
         out.imbue(std::locale::classic());
         out << "\"TOA5\",\"" << broker_name << "\""
             << ",\"" << model_name << "\""
             << ",\"" << serial_number << "\""
             << ",\"" << os_version << "\""
             << ",\"" << dld_name << "\""
             << ",\"" << dld_signature << "\""
             << ",\"" << table_name << "\""
             << (binary_stream ? "\r\n" : "\n");

         // second line (field names)
         if(write_stamp)
            out << "\"TIMESTAMP\",";
         if(write_record_no)
            out << "\"RECORD\",";
         write_field_names(out, true, begin_value, end_value);
         out << (binary_stream ? "\r\n" : "\n");

         // third line (units)
         if(write_stamp)
            out << "\"TS\",";
         if(write_record_no)
            out << "\"RN\",";
         write_units(out, begin_value, end_value);
         out << (binary_stream ? "\r\n" : "\n");

         // fourth line (processes)
         if(write_stamp)
            out << "\"\",";
         if(write_record_no)
            out << "\"\",";
         write_processes(out, begin_value, end_value);
         out << (binary_stream ? "\r\n" : "\n");
         out.imbue(old_loc);
      } // write_toa5_header


      void RecordDesc::write_csv_header(
         std::ostream &out,
         bool write_stamp,
         bool write_record_no,
         uint4 begin_value,
         uint4 end_value)
      {
         // write the byte order mark
         out << "\xef\xbb\xbf";
         if(write_stamp)
            out << "\"TIMESTAMP\",";
         if(write_record_no)
            out << "\"RECORD\",";
         write_field_names(out, true, begin_value, end_value);
         out << "\r\n";
         if(write_stamp)
            out << "\"TS\",";
         if(write_record_no)
            out << "\"\",";
         write_units(out, begin_value, end_value);
         out << "\r\n";
      } // write


      void RecordDesc::write_tob1_header(
         std::ostream &out,
         bool write_stamp,
         bool write_record_no,
         uint4 begin_value,
         uint4 end_value)
      {
         // first line
         std::locale old_loc = out.getloc();
         out.imbue(std::locale::classic());
         out << "\"TOB1\",\"" << broker_name << "\""
             << ",\"" << model_name << "\""
             << ",\"" << serial_number << "\""
             << ",\"" << os_version << "\""
             << ",\"" << dld_name << "\""
             << ",\"" << dld_signature << "\""
             << ",\"" << table_name << "\"\r\n";

         // second line
         if(write_stamp)
            out << "\"SECONDS\",\"NANOSECONDS\",";
         if(write_record_no)
            out << "\"RECORD\",";
         write_field_names(out, true, begin_value, end_value);
         out << "\r\n";

         // third line (units)
         if(write_stamp)
            out << "\"SECONDS\",\"NANOSECONDS\",";
         if(write_record_no)
            out << "\"RN\",";
         write_units(out, begin_value, end_value);
         out << "\r\n";

         // fourth line (processes)
         if(write_stamp)
            out << "\"\",\"\",";
         if(write_record_no)
            out << "\"\",";
         write_processes(out, begin_value, end_value);
         out << "\r\n";

         // fifth line (types)
         uint4 value_no = 0;
         if(write_stamp)
            out << "\"ULONG\",\"ULONG\",";
         if(write_record_no)
            out << "\"ULONG\",";
         for(values_type::iterator vi = values.begin();
             vi != values.end();
             ++vi)
         {
            value_desc_handle &value = *vi;
            uint4 merge_count = 0;
            if(value->start_of_merge())
            {
               for(values_type::iterator mvi = vi + 1;
                   mvi != values.end();
                   ++mvi)
               {
                  value_desc_handle &mvalue = *mvi;
                  if(mvalue->should_be_merged())
                     ++merge_count;
                  else
                     break;
               } 
            }
            if(!value->should_be_merged())
            {
               if(value_no >= begin_value && value_no < end_value)
               {
                  if(vi != values.begin())
                     out << ",";
                  out << "\"" << tob_type_string(value->data_type);
                  if(merge_count > 0)
                     out << "(" << (merge_count + 1) << ")";
                  out << "\"";
               }
               ++value_no;
            } 
         }
         out << "\r\n";
         out.imbue(old_loc);
      } // write_tob1_header


      void RecordDesc::write_xml_header(
         std::ostream &out,
         bool write_head_only,
         uint4 begin_value,
         uint4 end_value)
      {
         std::locale old_loc = out.getloc();
         out.imbue(std::locale::classic());
         if(!write_head_only)
         {
            out << "<?xml version=\"1.0\" standalone=\"yes\"?>\r\n"
                << "<csixml version=\"1.0\">\r\n";
         }

         // form the environment tag
         using Csi::Xml::Element;
         typedef Csi::SharedPtr<Element> element_handle;
         element_handle head(new Element(L"head"));
         element_handle environment(
            head->add_element(
               new Element(L"environment")));

         environment->add_element(
            new Element(L"station-name",broker_name));
         environment->add_element(
            new Element(L"table-name",table_name));
         if(model_name.length())
            environment->add_element(new Element(L"model",model_name.c_str()));
         if(serial_number.length())
            environment->add_element(new Element(L"serial-no",serial_number.c_str())); 
         if(os_version.length())
            environment->add_element(new Element(L"os-version",os_version.c_str()));
         if(dld_name.length())
            environment->add_element(new Element(L"dld-name",dld_name.c_str()));
         if(dld_signature.length())
            environment->add_element(new Element(L"dld-sig",dld_signature.c_str()));

         // form the fields tag
         element_handle fields = head->add_element(new Element(L"fields"));
         uint4 value_no = 0;
         for(values_type::iterator vi = values.begin();
             vi != values.end();
             ++vi)
         {
            // get a reference to the value description and determine whether it should be passed
            // over. 
            value_desc_handle &value = *vi; 
            if(value->should_be_merged())
               continue;

            // create an element for the value description
            if(value_no >= begin_value && value_no < end_value)
            {
               using Csi::Xml::OAttributeStream;
               element_handle field(fields->add_element(new Element(L"field")));
               OAttributeStream field_name(field.get_rep(),L"name");
               std::vector<uint4> subscript(value->begin(),value->end());
               
               field_name << value->name;
               if(!subscript.empty() && value->start_of_merge())
                  subscript.pop_back();
               if(!subscript.empty())
               {
                  field_name << "(";
                  for(std::vector<uint4>::iterator ai = subscript.begin();
                      ai != subscript.end();
                      ++ai)
                  {
                     if(ai != subscript.begin())
                        field_name << ",";
                     field_name << *ai;
                  }
                  field_name << ")";
               }
               field->add_attribute(L"type",xml_type_string(value->data_type));
               if(value->start_of_merge())
               {
                  uint4 string_len(1);
                  for(values_type::iterator mvi = vi + 1; mvi != values.end(); ++mvi)
                  {
                     value_desc_handle &mvalue(*mvi);
                     if(mvalue->should_be_merged())
                        ++string_len;
                     else
                        break;
                  }
                  field->set_attr_uint4(string_len, L"string-len");
               }
               if(value->units.length())
                  field->add_attribute(L"units",value->units);
               if(value->process.length())
                  field->add_attribute(L"process",value->process);
            }
            ++value_no;
         }
         head->output(out,true,1);
         if(!write_head_only)
            out << "\r\n  <data>\n";
         out.imbue(old_loc);
      } // write_xml_header


      void RecordDesc::write_json_header(
         std::ostream &out,
         bool write_head_only,
         uint4 begin_value,
         uint4 end_value,
         int transaction,
         uint2 signature)
      {
         using namespace Csi::Json;
         ObjectHandle header(create_json_header(begin_value, end_value, transaction, signature));
         if(!write_head_only)
            out << "{\n  \"head\": ";
         header->format(out, true, 1);
         if(!write_head_only)
            out << ",\n  \"data\": [\n";
      } // write_json_header


      Csi::Json::ObjectHandle RecordDesc::create_json_header(
         uint4 begin_value, uint4 end_value, int transaction, uint2 signature)
      {
         using namespace Csi::Json;
         ObjectHandle rtn(new Object);
         uint2 def_sig(get_sig(begin_value, end_value));
         Csi::OStrAscStream temp;
         uint4 value_no(0);
         
         if(transaction >= 0)
            rtn->set_property_number("transaction", transaction, false);
         rtn->set_property_number("signature", def_sig);
         if(def_sig != signature)
         {
            ObjectHandle environment(new Object);
            ArrayHandle fields(new Array);

            rtn->set_property("environment", environment.get_handle(), false);
            rtn->set_property("fields", fields.get_handle(), false);
            environment->set_property_str("station_name", broker_name.to_utf8(), false);
            environment->set_property_str("table_name", table_name.to_utf8(), false);
            if(model_name.length() > 0)
               environment->set_property_str("model", model_name, false);
            if(serial_number.length() > 0)
               environment->set_property_str("serial_no", serial_number, false);
            if(os_version.length() > 0)
               environment->set_property_str("os_version", os_version, false);
            if(dld_name.length() > 0)
               environment->set_property_str("dld_name", dld_name, false);
            if(dld_signature.length() > 0)
               environment->set_property_str("dld_sig", dld_signature, false);
            for(values_type::iterator vi = values.begin(); vi != values.end(); ++vi)
            {
               value_desc_handle &value(*vi);
               uint4 string_len(0);
               if(value->start_of_merge())
               {
                  string_len = 1;
                  for(values_type::iterator mvi = vi + 1; mvi != values.end(); ++mvi)
                  {
                     value_desc_handle &mvalue(*mvi);
                     if(mvalue->should_be_merged())
                        ++string_len;
                     else
                        break;
                  }
               }
               if(!value->should_be_merged())
               {
                  if(value_no >= begin_value && value_no < end_value)
                  {
                     typedef std::vector<uint4> subscript_type;
                     ObjectHandle field(new Object);
                     subscript_type subscript(value->begin(), value->end());
                     temp.str("");
                     temp << value->name;
                     if(!subscript.empty() && value->start_of_merge())
                        subscript.pop_back();
                     if(!subscript.empty())
                     {
                        temp << "(";
                        for(subscript_type::iterator si = subscript.begin(); si != subscript.end(); ++si)
                        {
                           if(si != subscript.begin())
                              temp << ",";
                           temp << *si;
                        }
                        temp << ")";
                     }
                     field->set_property_str("name", temp.str(), false);
                     field->set_property_str("type", xml_type_string(value->data_type), false);
                     if(value->data_type == CsiAscii)
                        field->set_property_number("string_len", string_len, false);
                     if(value->units.length())
                        field->set_property_str("units", value->units.to_utf8(), false);
                     if(value->process.length())
                        field->set_property_str("process", value->process.to_utf8(), false);
                     field->set_property_bool("settable", value->modifying_cmd != 0, false);
                     fields->push_back(field.get_handle());
                  }
                  ++value_no;
               }
            }
         }
         return rtn;
      }
      

      void RecordDesc::write_field_names(
         std::ostream &out,
         bool merge_strings,
         uint4 begin_value,
         uint4 end_value)
      {
         values_type::iterator vi;
         uint4 value_no(0);
         for(vi = values.begin(); vi != values.end(); ++vi)
         {
            // descriptions that should be merged with previous ones should be ignored
            value_desc_handle &value = *vi;
            
            if(value->should_be_merged())
               continue;
            
            // write the value name and subscripts
            if(value_no >= begin_value && value_no < end_value)
            {
               std::vector<uint4> subscript(value->begin(),value->end());
               if(!subscript.empty() && value->start_of_merge())
                  subscript.pop_back();
               if(vi != values.begin() && value_no != begin_value)
                  out << ",";
               out << "\"" << value->name;
               if(!subscript.empty())
               {
                  out << "(";
                  for(std::vector<uint4>::iterator ai = subscript.begin();
                      ai != subscript.end();
                      ++ai)
                  {
                     if(ai != subscript.begin())
                        out << ",";
                     out << *ai;
                  }
                  out << ")";
               }
               out << "\"";
            }
            ++value_no;
         }
      } // write_field_names


      void RecordDesc::write_units(
         std::ostream &out,
         uint4 begin_value,
         uint4 end_value)
      {
         uint4 value_no = 0;
         for(values_type::iterator vi = values.begin(); vi != values.end(); ++vi)
         {
            value_desc_handle &value = *vi;
            if(!value->should_be_merged())
            {
               if(value_no >= begin_value && value_no < end_value)
               {
                  if(vi != values.begin() && value_no != begin_value)
                     out << ",";
                  out << "\"" << value->units.to_utf8() << "\"";
               }
               ++value_no;
            }
         } 
      } // write_units


      void RecordDesc::write_processes(
         std::ostream &out,
         uint4 begin_value,
         uint4 end_value)
      {
         uint4 value_no(0);
         for(values_type::iterator vi = values.begin(); vi != values.end(); ++vi)
         {
            value_desc_handle &value = *vi;
            if(!value->should_be_merged())
            {
               if(value_no >= begin_value && value_no < end_value)
               {
                  if(vi != values.begin() && value_no != begin_value)
                     out << ",";
                  out << "\"" << value->process.to_utf8() << "\"";
               }
               ++value_no;
            }
         } 
      } // write_processes


      uint4 RecordDesc::get_record_len()
      {
         uint4 rtn = last_record_len;
         if(rtn == 0)
         {
            for(values_type::iterator vi = values.begin();
                vi != values.end();
                ++vi)
            {
               value_desc_handle &value = *vi;
               rtn += csiTypeLen(value->data_type);
            }
            last_record_len = rtn;
         }
         return rtn;
      } // get_record_len


      uint2 RecordDesc::get_sig(uint4 begin_value, uint4 end_value)
      {
         uint2 rtn;
         uint4 value_no(0);
         
         rtn = Csi::calcSigFor(broker_name.c_str(), broker_name.length());
         rtn = Csi::calcSigFor(table_name.c_str(), table_name.length(), rtn);
         for(values_type::iterator vi = values.begin(); vi != values.end(); ++vi)
         {
            value_desc_handle &value(*vi);
            if(!value->should_be_merged())
            {
               if(value_no >= begin_value && value_no < end_value)
               {
                  typedef std::vector<uint4> subscript_type;
                  subscript_type subscript(value->begin(), value->end());
                  rtn = Csi::calcSigFor(value->name.c_str(), value->name.length());
                  if(!subscript.empty() && value->start_of_merge())
                     subscript.pop_back();
                  for(subscript_type::iterator si = subscript.begin(); si != subscript.end(); ++si)
                  {
                     uint4 dim(*si);
                     rtn = Csi::calcSigFor(&dim, sizeof(dim), rtn);
                  }
                  rtn = Csi::calcSigFor(value->units.c_str(), value->units.length(), rtn);
                  rtn = Csi::calcSigFor(value->process.c_str(), value->process.length(), rtn);
               }
               else if(value_no >= end_value)
                  break;
               ++value_no;
            }
         }
         return rtn;
      } // get_sig
   };
};

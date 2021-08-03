/* Cora.Broker.Toa1FileReader.cpp

   Copyright (C) 2002, 2015 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 06 December 2002
   Last Change: Tuesday 07 April 2015
   Last Commit: $Date: 2016-07-11 09:29:07 -0600 (Mon, 11 Jul 2016) $ 
   Last Changed by: $Author: jon $

*/

#pragma warning(disable: 4996)
#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Broker.Toa1FileReader.h"
#include "Cora.Broker.ValueName.h"
#include "Csi.CsvRec.h"
#include "Csi.OsException.h"
#include "Csi.PakBus.Bmp5TableDef.h"
#include "Csi.Utils.h"
#include <algorithm>


namespace Cora
{
   namespace Broker
   {
      ////////////////////////////////////////////////////////////
      // class Toa1FileReader definitions
      ////////////////////////////////////////////////////////////
      Toa1FileReader::Toa1FileReader(value_factory_handle value_factory):
         DataFileReader(value_factory),
         input(0),
         header_len(0),
         header_sig(0),
         is_sleeping(false),
         data_len(0),
         data_sig(0)
      { }

      
      Toa1FileReader::~Toa1FileReader()
      { close(); }


      void Toa1FileReader::open(
         StrAsc const &file_name,
         StrAsc const &tdf_file_name)
      {
         // make sure that the input is closed first
         if(input)
            close();

         // try to open the input file.  It needs to be binary because that's how the data will be
         // formatted.  We should be able to parse the header all right in that mode
         input = Csi::open_file(file_name.c_str(), "rb");
         if(input == 0)
            throw DataFileException(DataFileException::failure_cannot_open);

         // if a TDF file name has been specified (the name is not empty), we will need to read the
         // table definitions file for that name.
         typedef Csi::PakBus::Bmp5TableDef table_def_type;
         typedef std::deque<table_def_type> table_defs_type;
         table_defs_type table_defs;
         table_defs_type::iterator tdi(table_defs.begin());
         if(tdf_file_name.length())
         {
            try
            {
               Csi::PakBus::read_table_defs(table_defs, tdf_file_name);
            }
            catch(std::exception &)
            { throw DataFileException(DataFileException::failure_fsl); }
         }

         // read and parse the fields of the environment header
         Csi::CsvRec line;
         
         line.read(input);
         if(line.size() != 3)
            throw DataFileException(DataFileException::failure_corrupt_file);
         if(line[0] != "TOACI1" &&
            line[0] != "TOACS1" &&
            line[0] != "TOARI1")
            throw DataFileException(DataFileException::failure_corrupt_file);
         record_description_handle record_description(
            new RecordDesc(
               StrUni(line[1].c_str()),
               StrUni(line[2].c_str())));
         record_descriptions[0] = record_description;
         if(!table_defs.empty())
         {
            tdi = std::find_if(
               table_defs.begin(),
               table_defs.end(),
               Csi::PakBus::table_has_name(line[2]));
            if(tdi == table_defs.end())
               throw DataFileException(DataFileException::failure_fsl);
         }
         
         // the next line should be the field names.  we will expect the seconds, nano-seconds, and
         // record number fields to be present.
         line.read(input);
         if(line.size() < 2)
            throw DataFileException(DataFileException::failure_corrupt_file);
         if(line[0] != "TMSTAMP")
            throw DataFileException(DataFileException::failure_corrupt_file); 
         if(line[1] != "RECNBR")
            throw DataFileException(DataFileException::failure_corrupt_file);

         // the rest of the fields in the list should be value names.
         Csi::CsvRec::iterator li;
         for(li = line.begin() + 2; li != line.end(); ++li)
            process_field_name(li->c_str());

         // we need to read the first record in the file to determine the data types of the values.
         // We will assume that the data types are either floating point values or dates.  Strings
         // will be mapped to the CsiAsciiZ data type.  Before reading the line, we need to save the
         // current file offset so that the file pointer can be restored there before exiting
         int8 data_start_pos = Csi::file_tell(input);
         header_len = data_start_pos;

         // if we don't have the table definition, we will need to infer data types and dimensiions
         // based upon the first record.
         if(tdi == table_defs.end())
         {
            RecordDesc::iterator vi = record_description->begin();
            line.read(input);
            Csi::file_seek(input,data_start_pos,SEEK_SET);
            if(line.size() != record_description->values.size() + 2)
               throw DataFileException(DataFileException::failure_corrupt_file);
            for(li = line.begin() + 2;
                vi != record_description->end() && li != line.end();
                ++vi, ++li)
            {
               Csi::SharedPtr<ValueDesc> value = *vi;
               
               if(line.was_quoted(li))
               {
                  // we need to try to convert the string into a date.  If that works, we will consider
                  // the value a stamp, otherwise, it will be considered to be a string.
                  if(*li == "NaN" || *li == "INF" || *li == "-INF")
                     value->data_type = CsiIeee4Lsf;
                  else
                  {
                     if(Csi::is_toa_time(li->c_str()))
                        value->data_type = CsiNSecLsf;
                     else
                     {
                        // we will treat the field as a string but we don't know the length.  we will
                        // assume a length of 64 bytes.
                        size_t current_pos = vi - record_description->begin();
                        
                        value->data_type = CsiAscii;
                        value->array_address.push_back(1);
                        record_description->values.insert(vi + 1,63,value);
                        vi = record_description->begin() + current_pos + 1;
                        
                        // now duplicate the value 63 times, this will give us a total of 64 character
                        // values (these will be merged when the record is formed).
                        for(uint4 i = 0; i < 63; ++i)
                        {
                           Csi::SharedPtr<ValueDesc> str_value(new ValueDesc);
                           str_value->name = value->name;
                           str_value->data_type = value->data_type;
                           str_value->modifying_cmd = value->modifying_cmd;
                           str_value->units = value->units;
                           str_value->process = value->process;
                           str_value->array_address = value->array_address;
                           str_value->array_address.back() = i + 2;
                           *vi = str_value;
                           if(i + 1 < 63)
                              ++vi;
                        }
                     }
                  }
               }
               else
                  value->data_type = CsiIeee4Lsf;
            }
         }
         else
         {
            RecordDesc::iterator vi(record_description->begin());
            while(vi != record_description->end())
            {
               RecordDesc::value_type value(*vi);
               table_def_type::iterator fdi(
                  std::find_if(
                     tdi->begin(),
                     tdi->end(),
                     Csi::PakBus::field_has_name(value->name)));
               if(fdi != tdi->end())
               {
                  value->data_type = lgr_to_csi_type((CsiLgrTypeCode)fdi->get_data_type());
                  if(value->data_type == CsiAscii && !fdi->get_array_dimensions().empty())
                  {
                     uint4 string_len(fdi->get_array_dimensions().back());
                     if(string_len > 1)
                     {
                        // we need to add new value descriptions for each string character
                        size_t current_pos(vi - record_description->begin());
                        value->array_address.push_back(1);
                        record_description->values.insert(vi + 1, string_len - 1, 0);
                        vi = record_description->begin() + current_pos + 1;

                        // we can now duplicate the value for the other cells
                        for(uint4 i = 0; i < string_len - 1; ++i)
                        {
                           Csi::SharedPtr<ValueDesc> str_value(new ValueDesc);
                           str_value->name = value->name;
                           str_value->data_type = value->data_type;
                           str_value->modifying_cmd = value->modifying_cmd;
                           str_value->units = value->units;
                           str_value->process = value->process;
                           str_value->array_address = value->array_address;
                           str_value->array_address.back() = i + 2;
                           *vi = str_value;
                           if(i + 1 < string_len - 1)
                              ++vi;
                        }
                     }
                  }
                  else if(value->data_type == CsiBool8)
                     value->data_type = CsiBool;
               }
               else
                  throw DataFileException(DataFileException::failure_fsl);
               ++vi;
            }
         }
         this->file_name = file_name;
      } // open


      void Toa1FileReader::close()
      {
         if(input)
         {
            fclose(input);
            input = 0;
         }
         is_sleeping = false;
         file_name.cut(0);
         DataFileReader::close();
      } // close


      void Toa1FileReader::hibernate()
      {
         if(!is_sleeping && input)
         {
            // we need to calculate the signature of the header as well as the first chunk of data.
            // These items will be used by wake_up() in order to ensure that the file meta-data has
            // not changed and also to determine whether our previous position is still valid. 
            int8 file_len(Csi::long_file_length(input));
            data_len = 0;
            data_sig = 0xAAAA;
            hibernate_pos = 0;
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


      bool Toa1FileReader::wake_up(bool &all_data_overwritten)
      {
         bool rtn  = false;
         if(is_sleeping && input == 0)
         {
            input = Csi::open_file(file_name.c_str(), "rb");
            if(input)
            {
               uint2 new_header_sig(Csi::calc_file_sig(input, header_len));
               if(new_header_sig == header_sig)
               {
                  int8 file_len(Csi::long_file_length(input));
                  int8 new_data_len = file_len - header_len;
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
                        all_data_overwritten = true;
                        seek_data(0);
                     }
                  }
                  else
                  {
                     all_data_overwritten = true;
                     seek_data(0);
                  }
                  is_sleeping = false;
                  rtn = true;
               }
               else
               {
                  fclose(input);
                  input = 0;
                  is_sleeping = false;
               }
            }
         }
         return rtn;
      } // wake_up


      Toa1FileReader::read_outcome_type Toa1FileReader::read_next_record(
         record_handle &record,
         bool *file_mark_after_record,
         bool *remove_mark_after_record,
         uint4 array_id)
      {
         read_outcome_type rtn = read_outcome_success;

         if(file_mark_after_record)
            *file_mark_after_record = false;
         if(remove_mark_after_record)
            *remove_mark_after_record = false;
         if(input != 0)
         {
            record_descriptions_type::iterator di = find(array_id);
            if(di == end())
               return read_outcome_invalid_record;
            if(record == 0 || record->get_description() != di->second)
               record.bind(new Record(di->second, *value_factory));
            try
            {
               // read the next line of text from the file
               Csi::CsvRec line;
               
               line.read(input);
               if(line.size() == record->size() + 2)
               {
                  record->set_stamp(Csi::LgrDate::fromStr(line[0].c_str()));
                  record->set_record_no(strtoul(line[1].c_str(),0,10));
                  for(Record::iterator vi = record->begin();
                      vi != record->end();
                      ++vi)
                  {
                     Record::value_handle &value = *vi;
                     value->read_text(
                        line[std::distance(record->begin(),vi) + 2].c_str());
                  }
               }
               else if(feof(input))
                  rtn = read_outcome_end_of_file;
               else
                  rtn = read_outcome_invalid_record;
            }
            catch(std::exception &)
            { rtn = read_outcome_invalid_record; }
         }
         else
            rtn = read_outcome_not_initialised;
         return rtn;
      } // read_next_record


      void Toa1FileReader::generate_index(index_type &index, bool &should_abort, uint4 *next_record_no)
      {
         Csi::CsvRec parser;
         int8 position;
         uint4 record_no;
         Csi::LgrDate time_stamp;
         uint4 fields_count(0);
         record_handle record(make_record(0));
         StrAsc time_stamp_str;
         StrAsc record_no_str;

         while(!feof(input) && !should_abort)
         {
            position = get_data_offset();
            fields_count = parser.read(input, 2);
            if(fields_count == record->size() + 2)
            {
               time_stamp_str = parser[0];
               record_no_str = parser[1];
               time_stamp = Csi::LgrDate::fromStr(time_stamp_str.c_str());
               record_no = strtoul(record_no_str.c_str(), 0, 10);
               if(next_record_no)
                  record_no = *next_record_no + 1;
               index.push_back(DataFileIndexEntry(position, time_stamp, record_no));
            }
         }
      } // generate_index

      
      uint2 Toa1FileReader::get_header_sig()
      {
         if(input == 0)
            throw DataFileException(DataFileException::failure_not_initialised);
         Csi::file_seek(input, 0, SEEK_SET);
         int8 current_pos = Csi::file_tell(input);
         uint2 rtn = Csi::calc_file_sig(input, header_len);
         Csi::file_seek(input, current_pos, SEEK_SET);
         return rtn;
      } // get_header_sig

      
      int8 Toa1FileReader::get_data_len()
      {
         if(input == 0)
            throw DataFileException(DataFileException::failure_not_initialised);
         return Csi::long_file_length(input) - header_len;
      }


      int8 Toa1FileReader::get_data_offset()
      {
         if(input == 0)
            throw DataFileException(DataFileException::failure_not_initialised);
         return Csi::file_tell(input) - header_len; 
      } // get_data_offset


      void Toa1FileReader::seek_data(int8 data_offset, bool search_for_prev)
      {
         // there is nothing to do if the file is not open
         if(input == 0)
            throw DataFileException(DataFileException::failure_not_initialised);

         // we need to adjust the data offset to be a "real" file offset.  This value can then be
         // adjusted to match the file length if need be
         int8 seek_pos = data_offset + header_len;
         int8 file_len = Csi::long_file_length(input);
         
         if(seek_pos >= file_len)
            seek_pos = file_len - 1;
	 Csi::file_seek(input, seek_pos, SEEK_SET);
         if(search_for_prev)
         {
            seek_pos = Csi::search_file_backward(input, "\n", 1);
            if(seek_pos != file_len)
               Csi::file_seek(input, seek_pos + 1, SEEK_SET);
         }
      } // seek_data
   };
};

/* Cora.Broker.Toa5FileReader.cpp

   Copyright (C) 2002, 2014 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 06 December 2002
   Last Change: Friday 07 November 2014
   Last Commit: $Date: 2017-09-07 16:55:39 -0600 (Thu, 07 Sep 2017) $ 
   Last Changed by: $Author: jon $

*/

#pragma warning(disable: 4996)
#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Broker.Toa5FileReader.h"
#include "Cora.Broker.ValueName.h"
#include "Csi.CsvRec.h"
#include "Csi.OsException.h"
#include <algorithm>
#include "Csi.Utils.h"
#include "Csi.PakBus.Bmp5TableDef.h"


namespace Cora
{
   namespace Broker
   {
      ////////////////////////////////////////////////////////////
      // class Toa5FileReader definitions
      ////////////////////////////////////////////////////////////
      bool Toa5FileReader::output_ieee8 = true;

      
      Toa5FileReader::Toa5FileReader(value_factory_handle value_factory):
         DataFileReader(value_factory),
         input(0),
         record_no_present(false),
         header_len(0),
         is_sleeping(false),
         header_sig(0),
         hibernate_pos(0),
         data_sig(0),
         data_len(0)
      { }

      
      Toa5FileReader::~Toa5FileReader()
      { close(); }


      void Toa5FileReader::open(
         StrAsc const &file_name, StrAsc const &tdf_file_name)
      {
         // make sure that the input is closed first
         if(input)
            close();

         // try to open the input file.  It needs to be binary because that's how the data will be
         // formatted.  We should be able to parse the header all right in that mode
         input = Csi::open_file(file_name.c_str(), "rb");
         if(input == 0)
         {
            trace("input file open failed");
            throw DataFileException(DataFileException::failure_cannot_open);
         }

         // if a TDF file name has been specified (the name is not empty), we will need to read the
         // table definitions from that name.
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
            catch(std::exception &e)
            {
               trace("tdf read failed: %s", e.what());
               throw_init_failure(DataFileException::failure_fsl);
            }
         }

         // read and parse the fields of the environment header
         Csi::CsvRec line;
         
         line.read(input);
         if(line.size() != 8)
         {
            trace("not enough environment header fields");
            throw_init_failure(DataFileException::failure_corrupt_file);
         }
         if(line[0] != "TOA5")
         {
            trace("invalid file format identifier");
            throw_init_failure(DataFileException::failure_corrupt_file);
         }
         
         record_description_handle record_description(
            new RecordDesc(
               StrUni(line[1]),
               StrUni(line[7])));
         record_descriptions[0] = record_description;
         record_description->model_name = line[2];
         record_description->serial_number = line[3];
         record_description->os_version = line[4];
         record_description->dld_name = line[5];
         record_description->dld_signature = line[6];
         if(!table_defs.empty())
         {
            tdi = std::find_if(
               table_defs.begin(),
               table_defs.end(),
               Csi::PakBus::table_has_name(line[7]));
            if(tdi == table_defs.end())
            {
               trace("table not present in fsl");
               throw_init_failure(DataFileException::failure_fsl);
            }
         }
         
         // the next line should be the field names.  The time stamp and record number columns are
         // considered to be optional so we will assume that they are not there until tests prove
         // otherwise. 
         line.read(input);
         record_no_present = time_stamp_present = false;
         if(line.size() < 1)
         {
            trace("not enough field names");
            throw_init_failure(DataFileException::failure_corrupt_file);
         }
         if(line[fixed_cols()] == "TIMESTAMP")
            time_stamp_present = true;
         if(line[fixed_cols()] == "RECORD")
            record_no_present = true;

         // the rest of the fields in the list should be value names.
         Csi::CsvRec::iterator li;
         for(li = line.begin() + fixed_cols();
             li != line.end();
             ++li)
            process_field_name(li->c_str());

         // next we need to read the units from the next header line
         RecordDesc::iterator ri;
         line.read(input);
         if(line.size() != record_description->values.size() + fixed_cols())
         {
            trace("not enough units");
            throw_init_failure(DataFileException::failure_corrupt_file);
         }
         for(ri = record_description->begin(), li = line.begin() + fixed_cols();
             ri != record_description->end();
             ++li, ++ri)
         {
            Csi::SharedPtr<ValueDesc> &value_desc = *ri;
            value_desc->units = li->c_str();
         }

         // next we need to read the process strings from the next header line
         line.read(input);
         if(line.size() != record_description->values.size() + fixed_cols())
         {
            trace("not enough process strings");
            throw_init_failure(DataFileException::failure_corrupt_file);
         }
         for(ri = record_description->begin(), li = line.begin() + fixed_cols();
             ri != record_description->end();
             ++li, ++ri)
         {
            Csi::SharedPtr<ValueDesc> &value_desc = *ri;
            value_desc->process = li->c_str();
         }

         // we need to read the first record in the file to determine the data types of the values.
         // We will assume that the data types are either floating point values or dates.  Strings
         // will be mapped to the CsiAsciiZ data type.  Before reading the line, we need to save the
         // current file offset so that the file pointer can be restored there before exiting
         int8 data_start_pos = Csi::file_tell(input);

         // if we don't have the table definition, we will need to infer data types and dimensions
         // based upon the first record.
         header_len = static_cast<uint4>(data_start_pos);
         if(tdi == table_defs.end())
         {
            RecordDesc::iterator vi = record_description->begin();
            line.read(input);
            Csi::file_seek(input,data_start_pos,SEEK_SET);
            if(line.size() != record_description->values.size() + fixed_cols())
            {
               trace("not enough description values");
               throw_init_failure(DataFileException::failure_corrupt_file);
            }
            for(li = line.begin() + fixed_cols();
                vi != record_description->end() && li != line.end();
                ++vi, ++li)
            {
               Csi::SharedPtr<ValueDesc> value = *vi;
               
               if(line.was_quoted(li))
               {
                  // we need to try to convert the string into a date.  If that works, we will consider
                  // the value a stamp, otherwise, it will be considered to be a string.
                  if(*li == "NaN" || *li == "Inf" || *li == "-Inf")
                     value->data_type = (output_ieee8 ? CsiIeee8Lsf : CsiIeee4Lsf);
                  else
                  {
                     // if this is a date value, it should conform to very strict syntax rules.
                     if(Csi::is_toa_time(li->c_str()))
                        value->data_type = CsiNSecLsf;
                     else
                     {
                        // we will treat the field as a string but we don't know the length.  we will
                        // assume a length of 64 bytes.
                        size_t current_pos = vi - record_description->begin();
                        size_t max_len(li->length() * 10);

                        if(max_len < 64)
                           max_len = 64;
                        value->data_type = CsiAscii;
                        value->array_address.push_back(1);
                        record_description->values.insert(vi + 1, max_len - 1, value);
                        vi = record_description->begin() + current_pos + 1;
                        
                        // now duplicate the value 63 times, this will give us a total of 64 character
                        // values (these will be merged when the record is formed).
                        for(uint4 i = 0; i < max_len - 1; ++i)
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
                           if(i + 1 < max_len - 1)
                              ++vi;
                        }
                     }
                  }
               }
               else
                  value->data_type = (output_ieee8 ? CsiIeee8Lsf : CsiIeee4Lsf);
            }
         }
         else
         {
            RecordDesc::iterator vi = record_description->begin();
            while(vi != record_description->end())
            {
               RecordDesc::value_type value(*vi);
               table_def_type::iterator fdi = std::find_if(
                  tdi->begin(),
                  tdi->end(),
                  Csi::PakBus::field_has_name(value->name));
               if(fdi != tdi->end())
               {
                  value->data_type = lgr_to_csi_type((CsiLgrTypeCode)fdi->get_data_type());
                  if(value->data_type == CsiAscii && !fdi->get_array_dimensions().empty())
                  {
                     uint4 string_len(fdi->get_array_dimensions().back());
                     if(string_len > 1)
                     {
                        // we need to add new value descriptions for each string cell
                        size_t current_pos(vi - record_description->begin());
                        value->array_address.push_back(1);
                        record_description->values.insert(vi + 1, string_len - 1, 0);
                        vi = record_description->begin() + current_pos + 1;

                        // we can now duplicate the value for the other cells. 
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
                  {
                     // we are going to assume here that the file is coming from the LoggerNet
                     // server which translates BOOL8 values into an array of eight BOOL values.  If
                     // that is the case, we can simply translate the data type to a BOOL.
                     value->data_type = CsiBool;
                  }
               }
               else
               {
                  trace("invalid tdf dimensions: %s", value->name.to_utf8().c_str());
                  throw_init_failure(DataFileException::failure_fsl);
               }
               ++vi;
            }
         }
         this->file_name = file_name;
      } // open


      void Toa5FileReader::close()
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


      void Toa5FileReader::hibernate()
      {
         if(input && !is_sleeping)
         {
            // we need to calculate the signatures of both the header and the first chunk of data
            // following.  These values will be used in wake_up() in order to ensure that the file
            // meta-data has not changed and also to ensure that the first data in the file has not
            // been overwritten.
            int8 file_len(Csi::long_file_length(input));
            data_len = 0;
            data_sig = 0xAAAA;
            hibernate_pos = 0;
            if(file_len >= header_len)
            {
               hibernate_pos = Csi::file_tell(input);
               Csi::file_seek(input, 0, SEEK_SET);
               header_sig = Csi::calc_file_sig(input, header_len);
               if(file_len > header_len)
               {
                  data_len = 1024;
                  if(header_len + data_len > file_len)
                     data_len = file_len - header_len;
                  data_sig = Csi::calc_file_sig(input, data_len);
               }
            }
            fclose(input);
            input = 0;
            is_sleeping = true;
         }
      } // hibernate


      bool Toa5FileReader::wake_up(bool &all_data_overwritten)
      {
         bool rtn = false;
         if(is_sleeping)
         {
            input = Csi::open_file(file_name.c_str(), "rb");
            if(input)
            {
               // if the meta-data has changed since we hibernated, we will need to fail and
               // completely reparse the file.
               uint2 new_header_sig(Csi::calc_file_sig(input, header_len));
               if(new_header_sig == header_sig)
               {
                  // we will read the first section of data from the beginning.  If this is not the
                  // same as it was when we hibernated, then the whole file has changed and will
                  // will need to process it from the beginning. 
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
                        seek_data(0);
                        all_data_overwritten = true;
                     }
                  }
                  else
                  {
                     seek_data(0);
                     all_data_overwritten = true;
                  }
                  is_sleeping = false;
                  rtn = true;
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


      Toa5FileReader::read_outcome_type Toa5FileReader::read_next_record(
         record_handle &record,
         bool *file_mark_after_record,
         bool *remove_mark_after_record,
         uint4 array_id)
      {
         read_outcome_type rtn = read_outcome_success;
         iterator di = find(array_id);
         
         if(file_mark_after_record)
            *file_mark_after_record = false;
         if(remove_mark_after_record)
            *remove_mark_after_record = false;
         if(input != 0 && di != end())
         {
            if(record == 0 || record->get_description() != di->second)
               record.bind(new Record(di->second, *value_factory));
            try
            {
               // read the next line of text from the file
               Csi::CsvRec line;
               
               line.read(input);
               if(line.size() == record->size() + fixed_cols())
               {
                  if(time_stamp_present)
                  {
                     last_time_str = line[0];
                     record->set_stamp(Csi::LgrDate::fromStr(last_time_str.c_str()));
                  }
                  else
                     record->set_stamp(Csi::LgrDate::system());
                  if(record_no_present)
                     record->set_record_no(strtoul(line[fixed_cols() - 1].c_str(),0,10));
                  else
                     record->set_record_no(0);
                  for(Record::iterator vi = record->begin();
                      vi != record->end();
                      ++vi)
                  {
                     Record::value_handle &value = *vi;
                     value->read_text(
                        line[std::distance(record->begin(),vi) + fixed_cols()].c_str());
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


      void Toa5FileReader::generate_index(index_type &index, bool &should_abort, uint4 *next_record_no)
      {
         // we will not bother to index if there is no time stamp present
         if(time_stamp_present)
         {
            // we need to parse each line of the file from the current position to the end and pick
            // out the time stamp and record number columns of each valid line.
            Csi::CsvRec parser;
            int cols_limit(static_cast<int>(fixed_cols()));
            int8 position;
            uint4 record_no(0xffffffff);
            Csi::LgrDate time_stamp;
            uint4 fields_count(0);
            record_handle record(make_record(0));
            StrAsc time_stamp_str;
            StrAsc record_no_str;

            while(!feof(input) && !should_abort)
            {
               // we will parse the first columns of each line
               position = get_data_offset();
               fields_count = parser.read(input, cols_limit);
               if(fields_count == record->size() + cols_limit)
               {
                  time_stamp_str = parser[0];
                  if(record_no_present)
                     record_no_str = parser[1];
                  time_stamp = Csi::LgrDate::fromStr(time_stamp_str.c_str());
                  if(record_no_str.length() > 0)
                     record_no = strtoul(record_no_str.c_str(), 0, 10);
                  else if(next_record_no)
                     record_no = *next_record_no;
                  if(next_record_no)
                     *next_record_no = record_no + 1;
                  index.push_back(DataFileIndexEntry(position, time_stamp, record_no));
               }
            }
         }
      } // generate_index

      
      bool Toa5FileReader::last_time_was_2400() const
      {
         bool rtn(last_time_str.find(" 24:00") < last_time_str.length());
         return rtn;
      }


      uint2 Toa5FileReader::get_header_sig()
      {
         if(input == 0)
            throw DataFileException(DataFileException::failure_not_initialised);
         int8 current_pos = Csi::file_tell(input);
         Csi::file_seek(input, 0, SEEK_SET);
         uint2 rtn = Csi::calc_file_sig(input, header_len);
         Csi::file_seek(input, current_pos, SEEK_SET);
         return rtn;
      } // get_header_sig

      
      int8 Toa5FileReader::get_data_len()
      {
         if(input == 0)
            throw DataFileException(DataFileException::failure_not_initialised);
         return Csi::long_file_length(input) - header_len;
      }


      int8 Toa5FileReader::get_data_offset()
      {
         if(input == 0)
            throw DataFileException(DataFileException::failure_not_initialised);
         return Csi::file_tell(input) - header_len; 
      } // get_data_offset


      void Toa5FileReader::seek_data(int8 data_offset, bool search_for_prev)
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
            bool is_valid_record = false;
            Csi::CsvRec line;
            while(!is_valid_record && seek_pos >= header_len)
            {
               seek_pos = Csi::search_file_backward(input, "\n", 1);
               if(seek_pos != file_len && seek_pos > header_len)
               {
                  Csi::file_seek(input, seek_pos + 1, SEEK_SET);
                  line.read(input);
                  if(line.size() > fixed_cols())
                  {
                     Csi::file_seek(input, seek_pos + 1, SEEK_SET);
                     is_valid_record = true;
                  }
                  else
                     Csi::file_seek(input, seek_pos - 1, SEEK_SET); 
               }
               else
               {
                  Csi::file_seek(input, header_len, SEEK_SET);
                  break;
               }
            }
         }
      } // seek_data
   };
};

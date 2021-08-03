/* Cora.Broker.Toa6FileReader.cpp

   Copyright (C) 2017, Campbell Scientific, Inc

   Written by: Jon Trauntvein 
   Date Begun: Thursday 07 September 2017
   Last Change: Thursday 07 September 2017
   Last Commit: $Date: 2017-09-08 09:55:28 -0600 (Fri, 08 Sep 2017) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Broker.Toa6FileReader.h"
#include "Cora.Broker.ValueName.h"
#include "Csi.CsvRec.h"
#include "Csi.OsException.h"
#include "Csi.Utils.h"


namespace Cora
{
   namespace Broker
   {
      Toa6FileReader::Toa6FileReader(value_factory_handle value_factory):
         DataFileReader(value_factory),
         input(0),
         record_no_present(false),
         time_stamp_present(false),
         header_len(0),
         is_sleeping(false),
         header_sig(0),
         hibernate_pos(0),
         data_sig(0),
         data_len(0)
      { }


      Toa6FileReader::~Toa6FileReader()
      { close(); }


      void Toa6FileReader::open(StrAsc const &file_name, StrAsc const &tdf_file_name)
      {
         // make sure that the current input is closed.
         if(input)
            close();

         // we need to try to open the new file name.
         input = Csi::open_file(file_name.c_str(), "rb");
         if(input == 0)
            throw DataFileException(DataFileException::failure_cannot_open);

         // read and parse the fields of the environment header
         Csi::CsvRec line(true);
         record_description_handle record_description;
         
         line.read(input);
         if(line.size() != 8)
         {
            trace("not enough environment header fields");
            throw_init_failure(DataFileException::failure_corrupt_file);
         }
         if(line[0] != "TOA6")
         {
            trace("invalid file format identifier");
            throw_init_failure(DataFileException::failure_corrupt_file);
         }
         record_description.bind(new RecordDesc(StrUni(line[1]), StrUni(line[7])));
         record_descriptions[0] = record_description;
         record_description->model_name = line[2];
         record_description->serial_number = line[3];
         record_description->os_version = line[4];
         record_description->dld_name = line[5];
         record_description->dld_signature = line[6];

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

         // the next line should specify the field data types.
         RecordDesc::iterator vi(record_description->begin());
         line.read(input);
         if(line.size() != record_description->values.size() + fixed_cols())
            throw_init_failure(DataFileException::failure_corrupt_file);
         for(li = line.begin() + fixed_cols(); vi != record_description->end() && li != line.end(); ++vi, ++li)
         {
            Csi::SharedPtr<ValueDesc> value(*vi);
            uint4 string_len(0);
            CsiLgrTypeCode data_type;
            parse_tob_data_type(li->c_str(), data_type, string_len);
            value->data_type = lgr_to_csi_type(data_type);
            if(data_type == LgrAscii && string_len > 1)
            {
               size_t current_pos = std::distance(record_description->begin(), vi);
               value->array_address.push_back(1);
               record_description->values.insert(vi + 1, string_len - 1, value);
               vi = record_description->begin() + current_pos + 1;
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

         // we have now processed all official lines in the header although there may be meta-data
         // in the file.  For now, we will ignore the meta-data as part of the data.
         int8 data_start_pos(Csi::file_tell(input));
         header_len = static_cast<uint4>(data_start_pos);
         this->file_name = file_name;
      } // open


      void Toa6FileReader::close()
      {
         if(input)
         {
            fclose(input);
            input = 0;
         }
         is_sleeping = false;
         DataFileReader::close();
      } // close


      void Toa6FileReader::hibernate()
      {
         if(input && !is_sleeping)
         {
            // we need to calculate the signatures of both the header and the first chunk of data
            // following.  These value will be used in wake_up() in order to ensure that the file
            // metadata has not changed and also to ensure that the first data in the file has not
            // been overwritten.
            int8 file_len(Csi::long_file_length(input));
            data_len = 0;
            data_sig = 0xaaaa;
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


      bool Toa6FileReader::wake_up(bool &all_data_overwritten)
      {
         bool rtn(false);
         if(is_sleeping)
         {
            input = Csi::open_file(file_name.c_str(), "rb");
            if(input)
            {
               // if the header has changed since we hibernated, we will need to fail and to
               // completely reparse the file.
               uint2 new_header_sig(Csi::calc_file_sig(input, header_len));
               if(new_header_sig == header_sig)
               {
                  // we will read the first section of data from the beginning.  If this is not the
                  // same as when we hibernated, them the whole file has been changed and we will
                  // need to process it from the beginning.
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


      Toa6FileReader::read_outcome_type Toa6FileReader::read_next_record(
         record_handle &record, bool *file_mark_after_record, bool *remove_mark_after_record, uint4 array_id)
      {
         read_outcome_type rtn(read_outcome_success);
         iterator di(find(array_id));

         if(remove_mark_after_record)
            *remove_mark_after_record = false;
         if(file_mark_after_record)
            *file_mark_after_record = false;
         if(input != 0 && di != end())
         {
            if(record == 0 || record->get_description() != di->second)
               record.bind(new Record(di->second, *value_factory));
            try
            {
               // we'll ignore any lines that contain metadata
               Csi::CsvRec line(true);
               bool is_meta(true);
               
               while(is_meta)
               {
                  line.read(input);
                  if(!line.empty() && line[0].first() != '#')
                     is_meta = false;
               }
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
                     record->set_record_no(strtoul(line[fixed_cols() - 1].c_str(), 0, 10));
                  else
                     record->set_record_no(0);
                  for(Record::iterator vi = record->begin(); vi != record->end(); ++vi)
                  {
                     Record::value_handle &value(*vi);
                     value->read_text(line[std::distance(record->begin(), vi) + fixed_cols()].c_str());
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


      void Toa6FileReader::generate_index(index_type &index, bool &should_abort, uint4 *next_record_no)
      {
         // we will not bother to index if there is no time stamp present.
         if(time_stamp_present)
         {
            // we need to parse each line of the file from the current position to the end and pick
            // out the time stamp and record number columns of each valid line.
            Csi::CsvRec parser(true);
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
               if(!parser.empty() && parser[0].first() == '#')
                  continue; // ignore any meta-data records
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
      
      
      bool Toa6FileReader::last_time_was_2400() const
      {
         return (last_time_str.find(" 24:00") < last_time_str.length());
      } // last_time_was_2400


      uint2 Toa6FileReader::get_header_sig()
      {
         int8 current_pos;
         uint2 rtn;
         
         if(input == 0)
            throw DataFileException(DataFileException::failure_not_initialised);
         current_pos = Csi::file_tell(input);
         Csi::file_seek(input, 0, SEEK_SET);
         rtn = Csi::calc_file_sig(input, header_len);
         Csi::file_seek(input, current_pos, SEEK_SET);
         return rtn;
      } // get_header_sig


      int8 Toa6FileReader::get_data_len()
      {
         if(input == 0)
            throw DataFileException(DataFileException::failure_not_initialised);
         return Csi::long_file_length(input) - header_len;
      } // get_data_len

      
      int8 Toa6FileReader::get_data_offset()
      {
         if(input == 0)
            throw DataFileException(DataFileException::failure_not_initialised);
         return Csi::file_tell(input) - header_len;
      } // get_data_offset


      void Toa6FileReader::seek_data(int8 data_offset, bool search_for_prev)
      {
         // there is nothing to do if the file is not open.
         if(input == 0)
            throw DataFileException(DataFileException::failure_not_initialised);

         // we need to adjust the specified data offset to be the real file offset.  This value can
         // then be adjusted to match the file's length, if necessary.
         int8 seek_pos(data_offset + header_len);
         int8 file_len(Csi::long_file_length(input));
         if(seek_pos >= file_len)
            seek_pos = file_len - 1;
         Csi::file_seek(input, seek_pos, SEEK_SET);

         // if we need to search for the start of the previous record, we will need to start at the
         // position and search backward.
         if(search_for_prev)
         {
            bool is_valid_record(false);
            Csi::CsvRec line(true);
            while(!is_valid_record && seek_pos > header_len)
            {
               seek_pos = Csi::search_file_backward(input, "\n", 1);
               if(seek_pos != file_len && seek_pos >= header_len)
               {
                  Csi::file_seek(input, seek_pos + 1, SEEK_SET);
                  line.read(input);
                  if(line.size() > fixed_cols() && line[0].first() != '#')
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




/* Cora.Broker.MixedCsvFileReader.cpp

   Copyright (C) 2006, 2016 Campbell Scientific, Inc.

   Written by: Tyler Mecham
   Date Begun: Tuesday 19 December 2006
   Last Change: Monday 11 July 2016
   Last Commit: $Date: 2016-07-11 09:29:07 -0600 (Mon, 11 Jul 2016) $
   Last Changed by: $Author: jon $

*/

#pragma warning(disable:4996)
#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Broker.MixedCsvFileReader.h"
#include "Cora.Broker.ValueName.h"
#include "Csi.CsvRec.h"
#include "Csi.OsException.h"
#include <algorithm>
#include <sstream>


namespace Cora
{
   namespace Broker
   {
      ////////////////////////////////////////////////////////////
      // class MixedCsvFileReader definitions
      ////////////////////////////////////////////////////////////
      MixedCsvFileReader::MixedCsvFileReader(value_factory_handle value_factory):
         DataFileReader(value_factory),
         input(0),
         is_sleeping(false),
         hibernate_pos(0),
         hibernate_sig(0),
         hibernate_len(0)
      {
         fsl_reader.bind(new FSLReader);
      }

      
      MixedCsvFileReader::~MixedCsvFileReader()
      { close(); }


      void MixedCsvFileReader::open(
         StrAsc const &file_name, StrAsc const &labels_file_name)
      {
         // make sure that the input is closed first
         if(input)
            close();

         // set up the labels
         set_fsl(labels_file_name);
         if(record_descriptions.empty())
            throw DataFileException(DataFileException::failure_cannot_open);

         // try to open the input file.  It needs to be binary because that's how the data will be
         // formatted.  We should be able to parse the header all right in that mode
         input = Csi::open_file(file_name.c_str(), "rb");
         if(input == 0)
            throw DataFileException(DataFileException::failure_cannot_open);
         this->file_name = file_name;
      } // open


      void MixedCsvFileReader::close()
      {
         if(input)
         {
            fclose(input);
            input = 0;
         }
         is_sleeping = false;
         DataFileReader::close();
      } // close


      void MixedCsvFileReader::hibernate()
      {
         if(input)
         {
            // we need to save the current file position.  If we are not positioned at the beginning
            // of the file, we will also need to calculate the signature for the first line of data
            // in the file.
            hibernate_pos = Csi::file_tell(input);
            if(hibernate_pos != 0)
            {
               // we will read the first 1K of data in the file
               Csi::file_seek(input, 0, SEEK_SET);
               hibernate_len = Csi::long_file_length(input);
               if(hibernate_len >= 1024)
                  hibernate_len = 1024;
               hibernate_sig = Csi::calc_file_sig(input, hibernate_len);
            }
            fclose(input);
            input = 0;
            is_sleeping = true;
         }
      } // hibernate


      bool MixedCsvFileReader::wake_up(bool &all_data_overwritten)
      {
         bool rtn = false;
         if(is_sleeping)
         {
            input = Csi::open_file(file_name.c_str(), "rb");
            if(input)
            {
               if(hibernate_pos <= Csi::long_file_length(input) && hibernate_pos > 0)
               {
                  // we need to verify the hibernate signature from the beginning of the file.  If
                  // the beginning of the file has changed, then we will assume that it has been
                  // overwritten.
                  uint2 current_sig = Csi::calc_file_sig(input, hibernate_len);
                  if(current_sig == hibernate_sig)
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
                  seek_data(0);
               is_sleeping = false;
               rtn = true;
            }
         }
         return rtn;
      } // wake_up


      MixedCsvFileReader::read_outcome_type MixedCsvFileReader::read_next_record(
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
         if(input != 0 && record_descriptions.size() != 0)
         {
            iterator records_it = record_descriptions.find(array_id);
            if(records_it != end())
            {
               try
               {
                  // if the record has not been allocated or uses the wrong description, we will
                  // need to re-allocate the record
                  if(record == 0 || record->get_description() != records_it->second)
                     record.bind(new Record(records_it->second, *value_factory));
                  
                  // read the next line of text from the file
                  Csi::CsvRec line;
                  bool read_record = false;

                  line.read(input);
                  while(!read_record)
                  {
                     if(line.size() >= record->size() + 1 &&
                        atoi(line[0].c_str()) == array_id)
                     {
                        double year = 0;
                        double day_of_year = 0;
                        double hour_minute = 0;
                        double seconds = 0.0;
                        int count = 1; //Skip the Array ID

                        for(Record::iterator vi = record->begin();
                            vi != record->end();
                            ++vi)
                        {
                           Record::value_handle &value = *vi;
                           value->read_text(line[count++].c_str());
                           if(year == 0.0 && value->get_name() == L"Year_RTM")
                              value->to_float(year);
                           else if(day_of_year == 0.0 && value->get_name() == L"Day_RTM")
                              value->to_float(day_of_year);
                           else if(hour_minute == 0.0 && value->get_name() == L"Hour_Minute_RTM")
                              value->to_float(hour_minute);
                           else if(seconds == 0.0 && value->get_name() == L"Seconds_RTM")
                              value->to_float(seconds);
                        }
                        record->set_stamp(
                           Csi::LgrDate::fStorage(
                              static_cast<int>(year),
                              static_cast<int>(day_of_year),
                              static_cast<int4>(hour_minute),
                              seconds));
                        read_record = true;
                     }
                     else if(!feof(input))
                        line.read(input); //Try to see if the next line is for the array id
                     else
                        break;
                  }

                  if(!read_record)
                  {
                     if(feof(input))
                        rtn = read_outcome_end_of_file;
                     else
                        rtn = read_outcome_invalid_record;
                  }
               }
               catch(std::exception &)
               { rtn = read_outcome_invalid_record; }
            }
            else
               rtn = read_outcome_invalid_record;
         }
         else
            rtn = read_outcome_not_initialised;
         return rtn;
      } // read_next_record


      int8 MixedCsvFileReader::get_data_len()
      {
         if(input == 0)
            throw DataFileException(DataFileException::failure_not_initialised);
         return Csi::long_file_length(input);
      } // get_data_len


      void MixedCsvFileReader::generate_index(index_type &index, bool &should_abort, uint4 *next_record_no)
      {
         Csi::CsvRec parser;
         int8 pos;
         uint4 fields_count;
         uint4 record_id;
         double year, day_of_year, hour_minute, seconds;
         Csi::LgrDate time_stamp;
         typedef std::map<uint4, record_handle> records_type;
         records_type records;
         
         while(input && !feof(input) && !should_abort)
         {
            record_handle record;
            uint4 count(1); // skip array ID
            pos = get_data_offset();
            fields_count = parser.read(input);
            if(fields_count > 0)
            {
               try
               {
                  record_id = strtoul(parser[0].c_str(), 0, 10);
                  record = records[record_id];
                  if(record == 0)
                  {
                     record = make_record(record_id);
                     records[record_id] = record;
                  }
                  if(record != 0 && fields_count >= record->size() + 1)
                  {
                     uint4 record_no(0xffffffff);
                     if(next_record_no)
                        record_no = (*next_record_no)++;
                     year = day_of_year = hour_minute = seconds = 0;
                     for(Record::iterator vi = record->begin(); vi != record->end(); ++vi)
                     {
                        Record::value_handle &value(*vi);
                        value->read_text(parser[count++].c_str());
                        if(year == 0 && value->get_name() == L"Year_RTM")
                           value->to_float(year);
                        else if(day_of_year == 0 && value->get_name() == L"Day_RTM")
                           value->to_float(day_of_year);
                        else if(hour_minute == 0 && value->get_name() == L"Hour_Minute_RTM")
                           value->to_float(hour_minute);
                        else if(seconds == 0 && value->get_name() == L"Seconds_RTM")
                           value->to_float(seconds);
                     }
                     time_stamp = Csi::LgrDate::fStorage(
                        static_cast<int4>(year),
                        static_cast<int4>(day_of_year),
                        static_cast<int4>(hour_minute),
                        seconds);
                     index.push_back(DataFileIndexEntry(pos, time_stamp, record_no, record_id));
                  }
               }
               catch(std::exception &)
               { }
            }
         }
      } // generate_index

      
      uint2 MixedCsvFileReader::get_header_sig()
      {
         if(input == 0)
            throw DataFileException(DataFileException::failure_not_initialised);
         return fsl_reader->get_labels_sig();
      } // get_header_sig


      int8 MixedCsvFileReader::get_data_offset()
      {
         if(input == 0)
            throw DataFileException(DataFileException::failure_not_initialised);
         return Csi::file_tell(input); 
      } // get_data_offset


      void MixedCsvFileReader::seek_data(int8 data_offset, bool search_for_prev)
      {
         // there is nothing to do if the file is not open
         if(input == 0)
            throw DataFileException(DataFileException::failure_not_initialised);

         // we need to adjust the data offset to be a "real" file offset.  This value can then be
         // adjusted to match the file length if need be
         int8 seek_pos = data_offset;
         int8 file_len = Csi::long_file_length(input);
         
         if(seek_pos >= file_len)
            seek_pos = file_len - 1;
         Csi::file_seek(input, seek_pos, SEEK_SET);

         if(seek_pos > 0 && search_for_prev)
            seek_pos = Csi::search_file_backward(input, "\r\n", 2);
      } // seek_data


      void MixedCsvFileReader::seek_data_with_index(int8 data_offset, uint4 array_id)
      {
         // there is nothing to do if the file is not open
         if(input == 0)
            throw DataFileException(DataFileException::failure_not_initialised);

         // we need to adjust the data offset to be a "real" file offset.  This value can then be
         // adjusted to match the file length if need be
         int8 seek_pos = data_offset;
         int8 file_len = Csi::long_file_length(input);
         
         if(seek_pos >= file_len)
            seek_pos = file_len - 1;
         Csi::file_seek(input, seek_pos, SEEK_SET);
         
         std::ostringstream search_str;
         search_str << "\r\n" << array_id << ",";
         Csi::search_file_backward(input, search_str.str().c_str(), static_cast<uint4>(search_str.str().length()));
      } // seek_data_with_index


      void MixedCsvFileReader::set_fsl(StrAsc const &labels_file_name)
      {
         fsl_reader->extract_labels(labels_file_name);
         record_descriptions.clear();
         for(FSLReader::iterator it = fsl_reader->begin(); it != fsl_reader->end(); ++it)
            record_descriptions[it->first] = it->second;
      } // set_fsl
   };
};

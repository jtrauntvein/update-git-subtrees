/* Csi.Tob2.DataFile.cpp

   Copyright (C) 2005, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 09 June 2005
   Last Change: Friday 16 November 2012
   Last Commit: $Date: 2013-01-10 08:29:11 -0600 (Thu, 10 Jan 2013) $ 
   Last Changed by: $Author: jon $

*/

#pragma warning(disable: 4996)
#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Tob2.DataFile.h"
#include "Csi.OsException.h"
#include "Csi.CsvRec.h"
#include "Csi.Utils.h"
#include <stdlib.h>


namespace Csi
{
   namespace Tob2
   {
      namespace
      {
         ////////////////////////////////////////////////////////////
         // parse_data_type
         ////////////////////////////////////////////////////////////
         void parse_data_type(
            StrAsc const &data_type_name,
            CsiLgrTypeCode &data_type,
            uint4 &string_len)
         {
            return parse_tob_data_type(
               data_type_name.c_str(), data_type, string_len);
         }  // parse_data_type


         ////////////////////////////////////////////////////////////
         // stamps_are_related
         ////////////////////////////////////////////////////////////
         bool stamps_are_related(uint2 val1, uint2 val2)
         { return (val1 == val2 || val1 == (~val2 & 0xffff)); }


         ////////////////////////////////////////////////////////////
         // parse_record_interval
         //
         // Parses the string used in the header to express the record interval.  This value is
         // returned in terms of nano-seconds.
         ////////////////////////////////////////////////////////////
         int8 parse_record_interval(char const *s)
         {
            // we need to copy the factor number
            int8 rtn = 0;
            int i = 0;
            StrAsc temp;

            while(s[i] != 0 && isdigit(s[i]))
               temp.append(s[i++]);
            if(temp.length() == 0)
               throw std::invalid_argument("Invalid record interval");
            rtn = strtoul(temp.c_str(),0,10);
            temp.cut(0);

            // skip over any intervening whitespace.  Whats left over should be the units
            while(s[i] != 0 && isspace(s[i]))
               ++i;
            temp = s + i;
            
            // we now need to interpret the units specifier
            if(temp == "SEC")
               rtn *= LgrDate::nsecPerSec;
            else if(temp == "MSEC")
               rtn *= LgrDate::nsecPerMSec;
            else if(temp == "USEC")
               rtn *= LgrDate::nsecPerUSec;
            else if(temp == "NSEC" || temp.length() == 0)
               0;               // nothing needs to be done
            else if(temp == "MIN")
               rtn *= LgrDate::nsecPerMin;
            else if(temp == "HR")
               rtn *= LgrDate::nsecPerHour;
            else if(temp == "DAY")
               rtn *= LgrDate::nsecPerDay;
            else
               throw std::invalid_argument("Invalid record interval units");
            return rtn;
         } // parse_record_interval


         ////////////////////////////////////////////////////////////
         // parse_frame_time_res
         //
         // Parses a frame time resolution expression and returns the expected sub-seconds
         // resolution factor in terms of nano-seconds.  Will throw a std::invalid_argument
         // exception if the expression cannot be parsed.
         ////////////////////////////////////////////////////////////
         uint4 parse_frame_time_res(char const *s)
         {
            // ignore all initial non-digit chars
            int i = 0;
            while(s[i] != 0 && !isdigit(s[i]))
               ++i;

            // gather together all the adjacent digits into a string that can be used to produce the
            // initial, unitless factor.
            StrAsc factor_string;
            uint4 factor;
            while(s[i] != 0 && isdigit(s[i]))
               factor_string.append(s[i++]);
            if(factor_string.length() == 0) 
            {
               factor_string = s;
               if(factor_string == "SECUSEC")
                  return 10000; // assume this type (no longer used) refers to 10 usecs
               else
                  throw std::invalid_argument("No factor in frame time resolution");
            }
            factor = strtoul(factor_string.c_str(),0,10);
            
            // the remaining characters in the string should specify the units of the factor.
            StrAsc factor_units(s + i); 
            if(factor_units == "Usec")
               factor *= 1000;
            else if(factor_units == "Msec")
               factor *= 1000000;
            else if(factor_units != "NSec")
               throw std::invalid_argument("Invalid factor units in frame time resolution");
            return factor;
         } // parse_frame_time_res
      };


      ////////////////////////////////////////////////////////////
      // class Field definitions
      ////////////////////////////////////////////////////////////
      uint4 Field::get_field_size() const
      {
         uint4 rtn = csiTypeLen(
            lgr_to_csi_type(data_type));
         if(string_len != 0)
            rtn *= string_len;
         return rtn;
      } // get_field_size
      
      
      ////////////////////////////////////////////////////////////
      // class DataFile definitions
      ////////////////////////////////////////////////////////////
      DataFile::DataFile(char const *file_name):
         file(0),
         dld_signature(0),
         frame_size(0),
         intended_table_size(0),
         validation_stamp(0),
         ring_record_no(0xFFFFFFFF),
         card_removal_time(0),
         header_length(0),
         cached_record_size(0),
         tob1_secs_pos(0xffffffff),
         tob1_nsecs_pos(0xFFFFFFFF),
         tob1_record_no_pos(0xFFFFFFFF),
         is_sleeping(false),
         hibernate_pos(0),
         header_sig(0),
         data_sig(0),
         data_len(0)
      {
         try
         {
            // try to open the data file
            file = Csi::open_file(file_name,"rb");
            if(file == 0)
               throw OsException("TOBx file open failure");
            
            // we will read all of the header lines in at once.  We will check the count of
            // elemements in each line and will also initialise the file type so that it can be
            // verified. 
            CsvRec environment;
            CsvRec decode_info;
            CsvRec field_names;
            CsvRec field_units;
            CsvRec field_processing;
            CsvRec field_data_types;
            
            environment.read(file);
            if(environment.size() < 8)
               throw MsgExcept("Environment line has too few fields");
            if(environment[0] == "TOB2")
               file_type = type_tob2;
            else if(environment[0] == "TOB3")
               file_type = type_tob3;
            else if(environment[0] == "TOB1")
               file_type = type_tob1;
            else
               throw MsgExcept("Invalid file type");
            if(file_type != type_tob1)
            {
               decode_info.read(file);
               if(decode_info.size() < 6)
                  throw MsgExcept("Decode Info line has too few fields");
            }
            field_names.read(file);
            if(field_names.size() < 1)
               throw MsgExcept("Too few fields defined");
            field_units.read(file);
            if(field_units.size() != field_names.size())
               throw MsgExcept("Invalid number of field units"); 
            field_processing.read(file);
            if(field_processing.size() != field_names.size())
               throw MsgExcept("Invalid number of field processes"); 
            field_data_types.read(file);
            if(field_data_types.size() != field_names.size())
               throw MsgExcept("Invalid number of field types");
            header_length = ftell(file);

            // we can now split the information out of the environment line (ww've already
            // initialised file type)
            station_name = environment[1];
            model_no = environment[2];
            serial_no = environment[3];
            os_version = environment[4];
            dld_name = environment[5];
            dld_signature = static_cast<uint2>(strtoul(environment[6].c_str(),0,10));
            if(file_type != type_tob1)
               create_time = environment[7];
            else
               table_name = environment[7];

            // split the info from the decode line
            if(file_type != type_tob1)
            {
               table_name = decode_info[0];
               record_interval_string = decode_info[1];
               record_interval = parse_record_interval(record_interval_string.c_str());
               frame_size = strtoul(decode_info[2].c_str(),0,10);
               intended_table_size = strtoul(decode_info[3].c_str(),0,10);
               validation_stamp = static_cast<uint2>(strtoul(decode_info[4].c_str(),0,10));
               time_resolution = decode_info[5];
               subsecs_factor = parse_frame_time_res(time_resolution.c_str());
               if(file_type == type_tob3 && decode_info.size() < 8)
                  throw MsgExcept("Too few fields in decode info");
               if(file_type == type_tob3)
               {
                  ring_record_no = strtoul(decode_info[6].c_str(),0,10);
                  card_removal_time = strtoul(decode_info[7].c_str(),0,10);
               }
            }
            else
            {
               record_interval = 0;
               subsecs_factor = 1;
               validation_stamp = 0;
               frame_size = 0;  // will calculate based upon fields
            }

            // we now need to instantiate each field.  we have already verified that the number of
            // elements in each list matches the list of field names.
            fields.reserve(field_names.size());
            for(size_t i = 0; i < field_names.size(); ++i)
            {
               Field field;
               bool use_field = true;
               
               field.name = field_names[i];
               field.units = field_units[i];
               field.processing = field_processing[i];
               parse_data_type(
                  field_data_types[i],
                  field.data_type,
                  field.string_len); 
               if(file_type == type_tob1)
               {
                  use_field = false;
                  if(field.name == "SECONDS" && tob1_secs_pos == 0xFFFFFFFF)
                     tob1_secs_pos = frame_size;
                  else if(field.name == "NANOSECONDS" && tob1_nsecs_pos == 0xFFFFFFFF)
                     tob1_nsecs_pos = frame_size;
                  else if(field.name == "RECORD" && tob1_record_no_pos == 0xFFFFFFFF)
                     tob1_record_no_pos = frame_size;
                  else
                     use_field = true;
               }
               if(use_field)
                  fields.push_back(field);
               if(file_type == type_tob1)
                  frame_size += field.get_field_size();
            }
         }
         catch(std::exception &)
         {
            if(file)
               fclose(file);
            throw;
         }
         this->file_name = file_name;
      } // constructor


      DataFile::~DataFile()
      {
         if(file)
            fclose(file);
      } // destructor


      void DataFile::hibernate()
      {
         if(file)
         {
            int8 file_len(Csi::long_file_length(file));
            data_len = 0;
            data_sig = 0xAAAA;
            if(file_len >= header_length)
            {
               hibernate_pos = Csi::file_tell(file);
               Csi::file_seek(file, 0, SEEK_SET);
               header_sig = Csi::calc_file_sig(file, header_length);
               if(file_len > header_length)
               {
                  data_len = 1024;
                  if(file_len - header_length < data_len)
                     data_len = file_len - header_length;
                  data_sig = Csi::calc_file_sig(file, data_len);
               }
            }
            fclose(file);
            file = 0;
            is_sleeping = true;
         }
      } // hibernate


      bool DataFile::wake_up(bool &all_data_overwritten)
      {
         bool rtn = false;
         if(is_sleeping)
         {
            file = Csi::open_file(file_name.c_str(), "rb");
            if(file != 0)
            {
               uint2 new_header_sig(Csi::calc_file_sig(file, header_length));
               if(new_header_sig == header_sig)
               {
                  int8 file_len(Csi::long_file_length(file));
                  int8 new_data_len = file_len - header_length;
                  if(new_data_len >= data_len)
                  {
                     uint2 new_data_sig(Csi::calc_file_sig(file, data_len));
                     if(new_data_sig == data_sig && hibernate_pos <= file_len)
                     {
                        all_data_overwritten = false;
                        Csi::file_seek(file, hibernate_pos, SEEK_SET);
                     }
                     else
                     {
                        all_data_overwritten = true;
                        seek_to_oldest();
                     }
                  }
                  else
                  {
                     all_data_overwritten = true;
                     seek_to_oldest();
                  }
                  rtn = true;
                  is_sleeping = false;
               }
               else
               {
                  fclose(file);
                  file = 0;
               }
            }
         }
         return rtn;
      } // wake_up
      

      char const *DataFile::get_header(StrAsc &dest)
      {
         dest.cut(0);
         if(file)
         {
            int8 current_pos = file_tell(file);
            file_seek(file,0,SEEK_SET);
            dest.readFile(file,header_length);
            file_seek(file,current_pos,SEEK_SET);
         }
         return dest.c_str();
      } // get_header


      uint4 DataFile::get_record_size()
      {
         if(cached_record_size == 0)
         {
            for(fields_type::const_iterator fi = fields.begin();
                fi != fields.end();
                ++fi)
               cached_record_size += fi->get_field_size();
         }
         return cached_record_size;
      } // get_record_size
      

      int8 DataFile::get_frames_count()
      {
         int8 rtn = 0;
         if(file)
            rtn = (long_file_length(file) - header_length) / frame_size;
         return rtn;
      } // get_frames_count


      void DataFile::seek_to_frame(int8 frame_no)
      {
         // verify that the frame number specified is within the set allowed by the file size. 
         if(frame_no > get_frames_count())
            throw std::invalid_argument("invalid frame number");
         file_seek(file, header_length + (frame_no * frame_size),SEEK_SET);
      } // seek_to_frame


      int8 DataFile::seek_to_oldest()
      {
         int8 rtn = 0;
         int8 frames_count = get_frames_count();
         if(file_type == type_tob1 && frames_count > 0)
            seek_to_frame(0);
         else if(frames_count > 0)
         {
            // we need the validation stamp for the first frame in the file.
            Frame frame;
            uint2 first_val_stamp;
            
            seek_to_frame(0);
            read_next_frame(frame);
            first_val_stamp = frame.get_validation_stamp();
            
            // we will also need the validation stamp for the last frame in the file.
            uint2 last_val_stamp = 0;
            int8 last_frame_pos = get_frames_count() - 1;
            seek_to_frame(last_frame_pos);
            try
            {
               read_next_frame(frame);
               last_val_stamp = frame.get_validation_stamp();
            }
            catch(ExcInvalidFrame &)
            { }

            // check to see if the file is full.  In this case, the oldest frame will be the first
            if(first_val_stamp == last_val_stamp)
            {
               rtn = 0;
            }
            // check to see if the file is partially filled.  In this case, the oldest frame will still
            // be the first
            else if(first_val_stamp != last_val_stamp &&
                    !stamps_are_related(last_val_stamp,validation_stamp))
            {
               rtn = 0;
            }
            else if(stamps_are_related(first_val_stamp,validation_stamp) &&
                    stamps_are_related(last_val_stamp,validation_stamp))
            {
               // if we got this far it is because the first and last validation stamps are related to
               // the file's and they are different from each other.  This implies that the oldest
               // frame is someplace in the middle of the file.  The fastest way to do this is to use a
               // binary search algorithm to locate the cross-over point
               int8 begin_pos = 0;
               int8 end_pos = last_frame_pos;
               int8 mid_pos = (end_pos - begin_pos) / 2;
               
               while(mid_pos != begin_pos)
               {
                  uint2 mid_val_stamp;
                  
                  seek_to_frame(mid_pos);
                  read_next_frame(frame);
                  mid_val_stamp = frame.get_validation_stamp();
                  if(mid_val_stamp == first_val_stamp)
                  {
                     begin_pos = mid_pos;
                     if(begin_pos != end_pos)
                        mid_pos = begin_pos + ((end_pos - begin_pos) / 2);
                     else
                        mid_pos = begin_pos;
                  }
                  else if(mid_val_stamp == last_val_stamp)
                  {
                     end_pos = mid_pos;
                     if(begin_pos != end_pos) 
                        mid_pos = begin_pos + ((end_pos - begin_pos) / 2);
                     else
                        mid_pos = begin_pos;
                  }
                  if((end_pos - begin_pos) == 1)
                     mid_pos = begin_pos = end_pos;
               }
               rtn = mid_pos;
            }
            else
               throw MsgExcept("No valid frames in file");
            seek_to_frame(rtn);
         }
         else
            throw MsgExcept("No valid frames in file");
         return rtn;
      } // seek_to_oldest


      int8 DataFile::seek_to_newest()
      {
         int8 rtn = 0;
         int8 frames_count = get_frames_count();
         if(file_type == type_tob1 && frames_count > 0)
         {
            rtn = get_frames_count() - 1;
            seek_to_frame(rtn);
         }
         else if(frames_count > 0)
         {
            // we'll start by seeking to the oldest frame.  This may seem a waste but this will
            // tell us if the oldest frame is the first in the file.  If the file has wrapped,
            // the newest frame is adjacent to the oldest.  If the file has not wrapped, we will
            // have to perform a binary search for the oldest frame.
            rtn = seek_to_oldest();
            if(rtn == 0)
            {
               // the wrap point is either right at the beginning of the file or there is no wrap
               // point.  Either way, we will do a binary search to find the newest frame.  This
               // will be the last frame in the file that has appears to be valid.
               int8 begin_pos = 0;
               int8 end_pos = frames_count - 1;
               int8 mid_pos = (end_pos - begin_pos) / 2;
               Frame frame;
               
               while(mid_pos != begin_pos)
               {
                  seek_to_frame(mid_pos);
                  read_buff.readFile(file,frame_size);
                  frame.set_contents(
                     read_buff.getContents(),
                     read_buff.length(),
                     (mid_pos * frame_size) + header_length,
                     this);
                  if(frame.get_is_valid(validation_stamp))
                  {
                     begin_pos = mid_pos;
                     if(begin_pos != end_pos)
                        mid_pos = begin_pos + ((end_pos - begin_pos) / 2);
                     else
                        mid_pos = begin_pos;
                  }
                  else
                  {
                     end_pos = mid_pos;
                     if(begin_pos != end_pos)
                        mid_pos = begin_pos + ((end_pos - begin_pos) / 2);
                     else
                        mid_pos = begin_pos;
                  }
                  rtn = mid_pos;
               }
               seek_to_frame(rtn);
            }
            else
            {
               // the file has wrapped.  Since the position is now at the oldest record, the newest
               // record is going to be the frame preceding the current position.
               --rtn;
               seek_to_frame(rtn);
            }
         }
         return rtn;
      } // seek_to_newest


      void DataFile::read_next_frame(Frame &dest)
      {
         int8 current_pos = file_tell(file);
         read_buff.readFile(file, frame_size);
         if(read_buff.length() != frame_size)
            throw ExcEndOfFile();
         dest.set_contents(
            read_buff.getContents(),
            read_buff.length(),
            current_pos,
            this);
         if(!dest.get_is_valid(validation_stamp))
            throw ExcInvalidFrame();
      } // read_next_frame


      bool DataFile::do_resynch()
      {
         bool rtn = false;
         if(file_type != type_tob1)
         {
            // seek back by one frame if we are in the position to do so
            int8 current_pos = file_tell(file);
            int8 first_pos = current_pos;
            if(current_pos >= header_length + frame_size)
            {
               file_seek(
                  file,
                  current_pos - frame_size,
                  SEEK_SET);
               first_pos -= frame_size;
            }
            
            // we now need to scan for the validation stamp or its complement anyplace within
            // the next 10*frame_size bytes.
            uint2 pattern = validation_stamp;
            uint2 pattern_comp;
            
            if(is_big_endian())
               reverse_byte_order(&pattern,sizeof(pattern));
            pattern_comp = (~pattern & 0xffff);
            for(uint4 scan_count = 0; !rtn && scan_count < 10; ++scan_count)
            {
               // read the space for the next bytes
               current_pos = file_tell(file);
               read_buff.readFile(file, frame_size + 2);
               if(read_buff.length() < frame_size)
                  throw ExcEndOfFile();
               
               // search for the pattern or its complement
               size_t pattern_pos = read_buff.find(&pattern,sizeof(pattern));
               if(pattern_pos >= read_buff.length())
                  pattern_pos = read_buff.find(&pattern_comp,sizeof(pattern_comp));
               
               // if the pattern or its complement was found, we need to save the current position and
               // try to scan ahead for the next frame
               if(pattern_pos < read_buff.length())
               {
                  try
                  {
                     // we will read the next frame to validate that we are at a major frame boundary.
                     // If that succeeds, we will back up by one frame and assume success. 
                     Frame next_frame;
                     int8 next_pos = current_pos + pattern_pos + 2;
                     file_seek(file, next_pos, SEEK_SET);
                     read_next_frame(next_frame);
                     if(next_pos >= frame_size && next_pos - frame_size > first_pos)
                     {
                        rtn = true;
                        file_seek(file, next_pos - frame_size, SEEK_SET);
                     }
                     else
                        throw ExcInvalidFrame();
                  }
                  catch(ExcInvalidFrame &)
                  {
                     file_seek(file, current_pos + pattern_pos + 2, SEEK_SET);
                     --scan_count;
                  }
               }
               else
                  // we are reading slightly more than the frame size with each step.  The reason for
                  // this is that the first part of the validation stamp could show up at the end of the
                  // buffer.  In order to get around this, we will over-read by two bytes and then back
                  // up by two bytes with each iteration. 
                  file_seek(file, -2, SEEK_CUR);
            }
         }
         return rtn;
      } // do_resynch


      int8 DataFile::get_current_file_offset()
      {
         int8 rtn = 0;
         if(file)
            rtn = file_tell(file);
         return rtn;
      } // get_current_file_offset
   };
};


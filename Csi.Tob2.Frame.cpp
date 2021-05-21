/* Csi.Tob2.Frame.cpp

   Copyright (C) 2005, 2017 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 09 June 2005
   Last Change: Tuesday 19 December 2017
   Last Commit: $Date: 2017-12-19 16:29:39 -0600 (Tue, 19 Dec 2017) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Tob2.DataFile.h"
#include "Csi.Utils.h"
#include "Csi.ByteOrder.h"
#include <assert.h>


namespace Csi
{
   namespace Tob2
   {
      namespace
      {
         size_t const sec_pos = 0;
         size_t const subsec_pos = 4;
         size_t const recordno_pos = 8;
         size_t const min_tob2_frame_size = 12;
         size_t const min_tob3_frame_size = min_tob2_frame_size + 4;
         uint4 const offset_mask      = 0x00000FFF;
         uint4 const minor_mask       = 0x00008000;
         uint4 const empty_mask       = 0x00004000;
         uint4 const remove_mark_mask = 0x00002000;
         uint4 const file_mark_mask   = 0x00001000;
         uint4 const validation_mask  = 0xFFFF0000;
      };

      
      ////////////////////////////////////////////////////////////
      // class Frame definitions
      ////////////////////////////////////////////////////////////
      void Frame::set_contents(
         void const *buff,
         size_t buff_len,
         int8 file_offset_,
         DataFile *source_file_)
      {
         file_offset = file_offset_;
         contents.setContents(buff,buff_len);
         source_file = source_file_;
         subframes.clear();
         if(source_file != 0 && source_file->get_file_type() != DataFile::type_tob1)
            flags = read_word(contents.length() - 4);
         if(!get_is_valid())
            throw std::invalid_argument("Invalid frame size");
      } // set_contents


      bool Frame::get_is_valid(uint2 file_validation_stamp) const
      {
         bool rtn = true;

         assert(DataFile::is_valid_instance(source_file));
         if(source_file->get_file_type() != DataFile::type_tob1 && contents.length() < 4)
            rtn = false;
         if(rtn && !is_empty())
         {
            switch(source_file->get_file_type())
            {
            case DataFile::type_tob1:
               if(contents.length() < source_file->get_frame_size())
                  rtn = false;
               break;
               
            case DataFile::type_tob2:
               if(contents.length() < min_tob2_frame_size)
                  rtn = false;
               break;
               
            case DataFile::type_tob3:
               if(contents.length() < min_tob3_frame_size)
                  rtn = false;
               break;
            }
         }
         if(rtn &&
            file_validation_stamp != 0 &&
            source_file->get_file_type() != DataFile::type_tob1 &&
            contents.length() == source_file->get_frame_size())
         {
            uint2 val_stamp = get_validation_stamp();
            if(val_stamp != file_validation_stamp &&
               val_stamp != (~file_validation_stamp & 0xffff))
               rtn = false;
         }
         return rtn;
      } // get_is_valid


      uint4 Frame::get_time_sec() const
      {
         uint4 rtn = 0;
         assert(DataFile::is_valid_instance(source_file));
         if(!is_empty())
         {
            if(source_file->get_file_type() != DataFile::type_tob1)
               rtn = read_word(sec_pos);
            else if(source_file->get_tob1_has_secs()) 
               rtn = read_word(source_file->get_tob1_secs_pos());
         }
         return rtn;
      } // get_time_sec


      uint4 Frame::get_time_subsec() const
      {
         uint4 rtn = 0;
         assert(DataFile::is_valid_instance(source_file));
         if(!is_empty())
         {
            if(source_file->get_file_type() != DataFile::type_tob1)
               rtn = read_word(subsec_pos);
            else if(source_file->get_tob1_has_nsecs())
               rtn = read_word(source_file->get_tob1_nsecs_pos());
         }
         return rtn;
      } // get_time_subsec


      LgrDate Frame::get_time(uint4 subsec_res)
      {
         int8 seconds = get_time_sec();
         int8 subsecs = get_time_subsec() * subsec_res;
         return LgrDate((seconds * LgrDate::nsecPerSec) + subsecs);
      } // get_time


      uint4 Frame::get_record_no() const
      {
         uint4 rtn = 0;
         assert(DataFile::is_valid_instance(source_file));
         if(!is_empty())
         {
            if(source_file->get_file_type() == DataFile::type_tob2 ||
               !source_file->get_tob1_has_record_no())
               throw std::invalid_argument("Cannot read record_no from the frame");
            if(source_file->get_file_type() == DataFile::type_tob3)
               rtn = read_word(recordno_pos);
            else
               rtn = read_word(source_file->get_tob1_record_no_pos());
         }
         return rtn;
      } // get_record_no


      uint2 Frame::get_minor_frame_size() const
      {
         uint2 rtn = 0;
         assert(DataFile::is_valid_instance(source_file));
         if(source_file->get_file_type() != DataFile::type_tob1)
            rtn = static_cast<uint2>(flags & offset_mask);
         else
            throw std::invalid_argument("Invalid frame or file type");
         return rtn;
      } // get_minor_frame_size


      bool Frame::has_file_mark(bool already_called)
      {
         bool rtn = false;
         assert(DataFile::is_valid_instance(source_file));
         if(source_file->get_file_type() != DataFile::type_tob1)
         {
            // evaluate whether the major frame indicates a file mark
            rtn = (flags & file_mark_mask) != 0;

            // if the frame is dirty and the major frame boundary indicates no file mark, there
            // still may be one on a minor frame.
            if(!rtn && is_minor() && !already_called)
            {
               generate_subframes();
               for(subframes_type::iterator si = subframes.begin();
                   !rtn && si != subframes.end();
                   ++si)
               {
                  rtn = si->has_file_mark(true);
               }
            }
            
         }
         return rtn;
      } // has_file_mark


      bool Frame::has_remove_mark(bool already_called)
      {
         bool rtn = false;
         assert(DataFile::is_valid_instance(source_file));
         if(source_file->get_file_type() != DataFile::type_tob1)
         {
            // if the frame is dirty and the major frame boundary indicates no remove mark, there
            // still may be one on a minor frame.
            rtn = (flags & remove_mark_mask) != 0;
            if(!rtn && is_minor() && !already_called)
            {
               generate_subframes();
               for(subframes_type::iterator si = subframes.begin();
                   !rtn && si != subframes.end();
                   ++si)
               {
                  rtn = si->has_remove_mark(true);
               }
            }
         }
         return rtn;
      } // has_remove_mark


      bool Frame::is_empty() const
      {
         bool rtn = false;
         assert(DataFile::is_valid_instance(source_file));
         if(source_file->get_file_type() != DataFile::type_tob1)
            rtn = (flags & empty_mask) != 0;
         return rtn;
      } // is_empty


      bool Frame::is_minor() const
      {
         bool rtn = false;
         assert(DataFile::is_valid_instance(source_file));
         if(source_file->get_file_type() != DataFile::type_tob1)
            rtn = (flags & minor_mask) != 0;
         return rtn;
      } // is_minor


      uint2 Frame::get_validation_stamp() const
      {
         uint2 rtn = 0;
         assert(DataFile::is_valid_instance(source_file));
         if(source_file->get_file_type() != DataFile::type_tob1)
            rtn = static_cast<uint2>((flags & validation_mask) >> 16);
         return rtn;
      } // get_validation_stamp


      void const *Frame::get_data_ptr() const
      {
         if(!get_is_valid() || is_empty())
            throw std::invalid_argument("No frame data to extract");
         return contents.getContents() + get_header_len();
      } // get_data_ptr


      size_t Frame::get_header_len() const
      {
         size_t rtn = 0;
         switch(source_file->get_file_type())
         {
         case DataFile::type_tob3:
            rtn = recordno_pos + 4;
            break;

         case DataFile::type_tob2:
            rtn = subsec_pos + 4;
            break;

         case DataFile::type_tob1:
            rtn = source_file->get_tob1_header_len();
            break;
         }
         return rtn;
      } // get_header_len

      
      size_t Frame::get_data_len() const
      {
         size_t rtn = 0;
         if(!get_is_valid() || is_empty())
            throw std::invalid_argument("No frame data");
         switch(source_file->get_file_type())
         {
         case DataFile::type_tob3:
            rtn = contents.length()- min_tob3_frame_size;
            break;
            
         case DataFile::type_tob2:
            rtn = contents.length() - min_tob2_frame_size;
            break;
            
         case DataFile::type_tob1:
            rtn = contents.length() - source_file->get_tob1_header_len();
            break;
         }
         return rtn;
      } // get_data_len


      uint4 Frame::get_records_count(bool already_called)
      {
         uint4 rtn = 0;
         if(is_minor() && contents.length() == source_file->get_frame_size() && !already_called)
         {
            generate_subframes();
            for(subframes_type::iterator si = subframes.begin();
                si != subframes.end();
                ++si)
            {
               rtn += si->get_records_count(true);
            }
         }
         else if(!is_empty())
            rtn = static_cast<uint4>(get_data_len() / source_file->get_record_size());
         return rtn;
      } // get_records_count


      void Frame::extract_subframes(subframes_type &subframes)
      {
         generate_subframes();
         subframes = this->subframes;
      } // extract_subframes


      int8 Frame::get_frame_pos()
      {
         int8 rtn = (file_offset - source_file->get_header_length()) / source_file->get_frame_size();
         return rtn;
      } // get_frame_pos

      
      bool Frame::get_newest_record_info(
         LgrDate *stamp,
         uint4 *record_no,
         bool do_recurse)
      {
         bool rtn = !is_empty() || (is_minor() && do_recurse);
         assert(DataFile::is_valid_instance(source_file));
         if(rtn)
         {
            // if this is a clean frame, the task is simpler.  We have only to project the values
            // based upon the max number of records in a frame. 
            if(!do_recurse || !is_minor())
            {
               uint4 records_count = static_cast<uint4>(get_data_len() / source_file->get_record_size());
               if(stamp != 0)
               {
                  int8 interval = source_file->get_record_interval();
                  *stamp = get_time(
                     source_file->get_subsecs_factor());
                  if(interval != 0)
                     *stamp += (records_count - 1) * interval;
               }
               if(record_no != 0 && source_file->get_tob1_has_record_no())
                  *record_no = get_record_no() + records_count - 1;
               rtn = true;
            }
            else
            {
               // This is a dirty frame so we need to generate the list of sub-frames and find the
               // last non-empty subframe
               subframes_type subframes;
               extract_subframes(subframes);
               rtn = false;
               while(!rtn && !subframes.empty())
               {
                  Frame &first = subframes.front();
                  rtn = first.get_newest_record_info(stamp,record_no,false);
                  subframes.pop_front();
               }
            }
         }
         return rtn;
      } // get_newest_record_info


      bool Frame::get_record(
         LgrDate *stamp,
         uint4 *record_no,
         StrBin *record_data,
         bool *file_mark_after,
         bool *remove_mark_after,
         uint4 record_index,
         bool already_called)
      {
         // if the data file pointer is invalid, this frame is as well
         assert(DataFile::is_valid_instance(source_file));
         
         // if this frame is a major frame but is dirty, we will have to use the subframes
         bool use_subframes = is_minor() && contents.length() == source_file->get_frame_size();
         bool rtn = false;
         if(use_subframes && !already_called)
         {
            // we need to go through all of the subframes and count the number of records in each
            uint4 previous_records_count = 0; 
            generate_subframes();
            subframes_type::iterator si = subframes.begin();
            while(!rtn &&
                  previous_records_count <= record_index &&
                  si != subframes.end())
            {
               uint4 frame_records = si->get_records_count();
               if(previous_records_count + frame_records >= record_index)
               {
                  // this frame should have the record we want
                  rtn = si->get_record(
                     stamp,
                     record_no,
                     record_data,
                     file_mark_after,
                     remove_mark_after,
                     record_index - previous_records_count,
                     true);
               }
               previous_records_count += frame_records;
               ++si;
            }
         }
         else
         {
            if(record_index < get_records_count())
            {
               if(stamp != 0)
               {
                  *stamp = get_time(source_file->get_subsecs_factor());
                  *stamp += source_file->get_record_interval() * record_index;
               }
               if(record_no != 0 && source_file->get_tob1_has_record_no())
                  *record_no = get_record_no() + record_index;
               if(record_data)
               {
                  byte const *record_start =
                     static_cast<byte const *>(get_data_ptr()) +
                     record_index * source_file->get_record_size();
                  record_data->setContents(
                     record_start,
                     source_file->get_record_size()); 
               }
               if(record_index == get_records_count() - 1)
               {
                  if(file_mark_after)
                     *file_mark_after = has_file_mark();
                  if(remove_mark_after)
                     *remove_mark_after = has_remove_mark();
               }
               else
               {
                  if(file_mark_after)
                     *file_mark_after = false;
                  if(remove_mark_after)
                     *remove_mark_after = false;
               }
               rtn = true;
            }
         }
         return rtn;
      } // get_record


      int8 Frame::get_record_offset(uint4 record_index)
      {
         int8 rtn = 0;
         bool use_subframes = is_minor() && contents.length() == source_file->get_frame_size();
         if(use_subframes)
         {
            uint4 previous_records_count = 0;
            subframes_type::iterator si;

            generate_subframes();
            si = subframes.begin();
            while(previous_records_count <= record_index && si != subframes.end())
            {
               uint4 frame_records = si->get_records_count();
               if(previous_records_count + frame_records >= record_index)
               {
                  rtn = si->file_offset - file_offset + si->get_header_len();
                  rtn += source_file->get_record_size() * (record_index - previous_records_count);
                  break;
               }
               previous_records_count += frame_records;
               ++si;
            }
         }
         else if(record_index < get_records_count())
            rtn += get_header_len() + record_index * source_file->get_record_size();
         else
            rtn = contents.length();
         return rtn;
      } // get_record_offset
      

      uint4 Frame::read_word(size_t offset) const
      {
         uint4 rtn = 0;
         if(offset + 4 <= contents.length())
         {
            char const *word = contents.getContents() + offset;
            rtn = (static_cast<uint4>(word[3]) & 0xff) << 24;
            rtn |= (static_cast<uint4>(word[2]) & 0xff) << 16;
            rtn |= (static_cast<uint4>(word[1]) & 0xff) << 8;
            rtn |= (static_cast<uint4>(word[0]) & 0xff);
         }
         return rtn;
      } // read_word


      void Frame::generate_subframes()
      {
         if(subframes.empty() &&
            get_is_valid() && is_minor() &&
            contents.length() == source_file->get_frame_size())
         {
            // the last minor frame uses the same footer as the major frame.  As we iterate the list
            // of minor frames, we will keep the position of one past the footer that we are
            // considering.  
            size_t last_frame_end = contents.length();
            while(last_frame_end > min_tob2_frame_size)
            {
               // before attempting to create the sub-frame, we need to validate its footer
               // information and size
               uint4 subframe_flags = read_word(last_frame_end - 4);
               size_t subframe_size = (subframe_flags & offset_mask);
               uint4 subframe_val = ((subframe_flags & validation_mask) >> 16);
               if(subframe_size > last_frame_end ||
                  (subframe_flags & minor_mask) == 0)
                  break;
               if(subframe_size == 0)
                  throw std::invalid_argument("Invalid subframe size");
               
               // we can now create the subframe and insert it at the head of the list
               size_t subframe_begin = last_frame_end - subframe_size;
               if((source_file->get_file_type() == DataFile::type_tob2 &&
                   subframe_size >= min_tob2_frame_size) ||
                  (source_file->get_file_type() == DataFile::type_tob3 &&
                   subframe_size >= min_tob3_frame_size))
               {
                  subframes.push_front(
                     Frame(
                        contents.getContents() + subframe_begin,
                        subframe_size,
                        file_offset + subframe_begin,
                        source_file));
                  if(subframes.front().is_empty())
                     subframes.pop_front();
               }
               last_frame_end = subframe_begin;
            }
         }
      } // generate_subframes
   };
};


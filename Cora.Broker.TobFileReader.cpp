/* Cora.Broker.TobFileReader.cpp

   Copyright (C) 2006, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 11 August 2006
   Last Change: Friday 29 April 2016
   Last Commit: $Date: 2016-07-11 09:29:07 -0600 (Mon, 11 Jul 2016) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Broker.TobFileReader.h"
#include "Csi.Tob2.DataFile.h"
#include "Csi.Tob2.Frame.h"
#include <sstream>


namespace Cora
{
   namespace Broker
   {
      ////////////////////////////////////////////////////////////
      // class TobFileReader definitions
      ////////////////////////////////////////////////////////////
      TobFileReader::TobFileReader(
         value_factory_handle value_factory_):
         DataFileReader(value_factory_),
         last_record_no(0xFFFFFFFF)
      { }


      TobFileReader::~TobFileReader()
      { close(); }


      void TobFileReader::open(
         StrAsc const &file_name, StrAsc const &labels_file_name)
      {
         // generate the record meta-data
         using namespace Csi::Tob2;
         std::ostringstream temp;

         temp.imbue(std::locale::classic());
         data_file.bind(new DataFile(file_name.c_str()));
         record_description_handle record_description(
            new Cora::Broker::RecordDesc(
               StrUni(data_file->get_station_name().c_str()),
               StrUni(data_file->get_table_name().c_str())));
         record_descriptions[0] = record_description;
         last_record_no = 0xFFFFFFFF;
         record_description->model_name = data_file->get_model_no();
         record_description->serial_number = data_file->get_serial_no();
         record_description->os_version = data_file->get_os_version();
         record_description->dld_name = data_file->get_dld_name();
         temp << data_file->get_dld_signature();
         record_description->dld_signature = temp.str().c_str();
         for(DataFile::const_iterator fi = data_file->begin();
             fi != data_file->end();
             ++fi)
         {
            Field const &field = *fi;
            for(uint4 j = 0; j == 0 || j < field.get_string_len(); ++j)
            {
               using  Cora::Broker::ValueDesc;
               Csi::SharedPtr<ValueDesc> value_desc(new ValueDesc);
               value_desc->name = field.get_name();
               value_desc->data_type = lgr_to_csi_type(
                  field.get_data_type());
               value_desc->units = field.get_units();
               value_desc->process = field.get_processing();
               if(field.get_string_len() > 0)
                  value_desc->array_address.push_back(j + 1);
               record_description->values.push_back(value_desc);
            }
         }

         // seek to the oldest frame and read it
         resynch();
      } // open


      void TobFileReader::resynch()
      {
         try
         {
            newest_frame_pos = data_file->seek_to_newest();
            oldest_frame_pos = data_file->seek_to_oldest();
            current_frame.bind(new Csi::Tob2::Frame);
            data_file->read_next_frame(*current_frame);
            current_frame_is_valid = true;
            frame_record = 0;
         }
         catch(Csi::Tob2::ExcEndOfFile &)
         {
            current_frame_is_valid = false;
            frame_record = 0;
         }
         catch(Csi::Tob2::ExcInvalidFrame &)
         {
            current_frame_is_valid = false;
            frame_record = 0;
            newest_frame_pos = oldest_frame_pos = 0;
         }
      } // resynch


      void TobFileReader::close()
      {
         current_frame.clear();
         data_file.clear();
      } // close


      void TobFileReader::hibernate()
      {
         if(data_file != 0)
            data_file->hibernate();
      } // hibernate


      bool TobFileReader::wake_up(bool &all_data_overwritten)
      {
         bool rtn = false;
         if(data_file != 0)
            rtn = data_file->wake_up(all_data_overwritten);
         return rtn;
      } // wake_up


      TobFileReader::read_outcome_type TobFileReader::read_next_record(
         record_handle &record,
         bool *file_mark_after_record,
         bool *remove_mark_after_record,
         uint4 array_id)
      {
         read_outcome_type rtn = read_outcome_not_initialised;
         Csi::LgrDate stamp;
         uint4 record_no(last_record_no + 1);
         
         while(rtn == read_outcome_not_initialised && data_file != 0)
         {
            bool record_result = false;
            if(current_frame_is_valid)
            {
               record_result = current_frame->get_record(
                  &stamp,
                  &record_no,
                  &record_buffer,
                  file_mark_after_record,
                  remove_mark_after_record,
                  frame_record++);
               last_record_no = record_no;
            }
            if(record_result)
            {
               record->read(
                  record_no,
                  0,
                  stamp,
                  record_buffer.getContents(),
                  static_cast<uint4>(record_buffer.length()));
               rtn = read_outcome_success;
            }
            else
            {
               try
               {
                  int8 stop_pos = data_file->get_header_length() +
                     (oldest_frame_pos * data_file->get_frame_size());
                  frame_record = 0;
                  if(data_file->get_current_file_offset() == stop_pos)
                     rtn = read_outcome_end_of_file;
                  else if(current_frame != 0)
                  {
                     data_file->read_next_frame(*current_frame);
                     current_frame_is_valid = true;
                  }
                  else
                     rtn = read_outcome_end_of_file;
               }
               catch(Csi::Tob2::ExcInvalidFrame &)
               { rtn = read_outcome_end_of_file; }
               catch(Csi::Tob2::ExcEndOfFile &)
               {
                  data_file->seek_to_frame(0);
                  current_frame_is_valid = false;
               }
               catch(std::exception &)
               {
                  rtn = read_outcome_invalid_record;
               }
            }
         }
         return rtn;
      } // read_next_record


      void TobFileReader::generate_index(index_type &index, bool &should_abort, uint4 *next_record_no)
      {
         if(has_time_stamp())
         {
            bool at_end(false);
            uint4 record_no;
            Csi::LgrDate time_stamp;
            int8 pos;
            bool file_mark_after_record;
            bool remove_mark_after_record;
            while(data_file != 0 && !at_end && !should_abort)
            {
               bool record_result(false);
               if(current_frame_is_valid)
               {
                  pos = get_data_offset();
                  record_result = current_frame->get_record(
                     &time_stamp,
                     &record_no,
                     &record_buffer,
                     &file_mark_after_record,
                     &remove_mark_after_record,
                     frame_record++);
               }
               if(record_result)
               {
                  index.push_back(DataFileIndexEntry(pos, time_stamp, record_no));
                  if(next_record_no)
                     *next_record_no = record_no + 1;
               }
               else
               {
                  try
                  {
                     int8 stop_pos(
                        data_file->get_header_length() + (oldest_frame_pos * data_file->get_frame_size()));
                     frame_record = 0;
                     if(data_file->get_current_file_offset() == stop_pos)
                        at_end = true;
                     else if(current_frame != 0)
                     {
                        data_file->read_next_frame(*current_frame);
                        current_frame_is_valid = true;
                     }
                     else
                        at_end = true;
                  }
                  catch(Csi::Tob2::ExcInvalidFrame &)
                  { at_end = true; }
                  catch(Csi::Tob2::ExcEndOfFile &)
                  {
                     data_file->seek_to_frame(0);
                     current_frame_is_valid = false;
                  }
                  catch(std::exception &)
                  { at_end = true; }
               }
            }
         }
      } // generate_index

      
      bool TobFileReader::has_time_stamp() const
      {
         bool rtn = false;
         if(data_file != 0)
         {
            if(data_file->get_file_type() == Csi::Tob2::DataFile::type_tob1)
               rtn = data_file->get_tob1_has_secs();
            else
               rtn = true;
         }
         return rtn;
      } // has_time_stamp


      bool TobFileReader::has_record_no() const
      {
         bool rtn = false;
         if(data_file != 0)
         {
            if(data_file->get_file_type() == Csi::Tob2::DataFile::type_tob1)
               rtn = data_file->get_tob1_has_record_no();
            else
               rtn = true;
         }
         return rtn;
      } // has_record_no


      int8 TobFileReader::get_header_len()
      {
         if(data_file == 0)
            throw DataFileException(DataFileException::failure_not_initialised);
         return data_file->get_header_length();
      } // get_header_len


      uint2 TobFileReader::get_header_sig()
      {
         StrAsc header;
         
         if(data_file == 0)
            throw DataFileException(DataFileException::failure_not_initialised);
         data_file->get_header(header);
         return Csi::calcSigFor(header.c_str(), header.length());
      } // get_header_sig

      
      int8 TobFileReader::get_data_len()
      {
         int8 rtn = 0;
         if(data_file == 0)
            throw DataFileException(DataFileException::failure_not_initialised);
         if(oldest_frame_pos < newest_frame_pos)
            rtn = (newest_frame_pos - oldest_frame_pos) * data_file->get_frame_size();
         else
            rtn = data_file->get_frame_size() * data_file->get_frames_count();
         return rtn;
      } // get_data_len

      
      int8 TobFileReader::get_data_offset()
      {
         // do nothing if the file is not open
         if(data_file == 0)
            throw Csi::MsgExcept("Data file is not open");
         if(!current_frame_is_valid)
            throw Csi::MsgExcept("current frame is not valid");

         // we need to calculate the "data offset" for the start of the current frame.  This offset
         // is defined as the number of frames between the oldest frame and the current frame.
         int8 current_frame_pos = current_frame->get_frame_pos();
         int8 frames_count = data_file->get_frames_count();
         int8 rtn = 0;

         if(current_frame_pos >= oldest_frame_pos)
            rtn = current_frame_pos - oldest_frame_pos;
         else
            rtn = (frames_count - oldest_frame_pos) + current_frame_pos;
         rtn *= data_file->get_frame_size();

         // we now have to take into account the position of the current record within the frame
         rtn += current_frame->get_record_offset(frame_record);
         return rtn;
      } // get_data_offset


      void TobFileReader::seek_to_newest(uint4 array_id)
      {
         if(data_file == 0)
            throw Csi::MsgExcept("data file is not open");
         last_record_no = 0xFFFFFFFF;
         if(data_file->get_file_type() == Csi::Tob2::DataFile::type_tob1)
            DataFileReader::seek_to_newest(array_id);
         else
         {
            data_file->seek_to_newest();
            data_file->read_next_frame(*current_frame);
            if(current_frame->get_is_valid())
            {
               frame_record = current_frame->get_records_count() - 1;
               current_frame_is_valid = true;
            }
         }
      } // seek_to_newest

      
      void TobFileReader::seek_data(int8 offset, bool search_for_prev)
      {
         // do nothing if the file is not open
         if(data_file == 0)
            throw Csi::MsgExcept("Data file is not open");

         // we need to calculate the selected frame number.  The byte offset into the frame should
         // just be the remainder of dividing the offset by frame size.
         int8 selected_frame(
            (offset / data_file->get_frame_size()) + oldest_frame_pos);
         int8 frame_offset(offset % data_file->get_frame_size());

         last_record_no = 0xFFFFFFFF;
         if(selected_frame >= data_file->get_frames_count())
            selected_frame -= data_file->get_frames_count();

         // now that we have calculated the selected frame, we can seek to it
         data_file->seek_to_frame(selected_frame);
         data_file->read_next_frame(*current_frame);
         if(current_frame->get_is_valid(data_file->get_validation_stamp()))
         {
            // we must now calculate the beginning position of the record within the frame.  This is
            // done by going through the list of subframes until we find one whose start offset is
            // just larger than our frame offset
            using namespace Csi::Tob2;
            Frame::subframes_type subframes;
            Frame *selected_frame_ptr = current_frame.get_rep();
            
            current_frame->extract_subframes(subframes);
            frame_record = 0;
            for(Frame::subframes_type::iterator fi = subframes.begin();
                fi != subframes.end();
                ++fi)
            {
               int8 this_frame_offset = fi->get_file_offset() - current_frame->get_file_offset();
               if(!fi->is_empty() && this_frame_offset <= frame_offset)
               {
                  if(current_frame != selected_frame_ptr)
                     frame_record += selected_frame_ptr->get_records_count(); 
                  selected_frame_ptr = &(*fi);
               }
               else
                  break;
            }

            // coming out of the previous loop, we should have a pointer to the frame that
            // incorporates the specified offset.  The frame record counter should also have
            // advanced to cover the records in all of the previous frames or be pointing to the
            // first record in the selected frame.  we need to further refine it within the current
            // frame so that it points to the beginning of the record that overlaps the offset.
            int8 frame_bytes_left = frame_offset - (selected_frame_ptr->get_file_offset() - current_frame->get_file_offset());
            if(frame_bytes_left > (int8)selected_frame_ptr->get_header_len())
            {
               frame_bytes_left -= selected_frame_ptr->get_header_len();
               frame_record += static_cast<uint4>(frame_bytes_left / data_file->get_record_size());
            }
            current_frame_is_valid = true;
         }
         else
            current_frame_is_valid = false;
      } // seek_data
   };
};

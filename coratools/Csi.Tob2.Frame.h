/* Csi.Tob2.Frame.h

   Copyright (C) 2005, 2011 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 09 June 2005
   Last Change: Thursday 10 February 2011
   Last Commit: $Date: 2011-02-10 10:12:04 -0600 (Thu, 10 Feb 2011) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Csi_Tob2_Frame_h
#define Csi_Tob2_Frame_h

#include "StrBin.h"
#include "CsiTypeDefs.h"
#include "Csi.LgrDate.h"
#include <list>


namespace Csi
{
   namespace Tob2
   {
      //@group class forward declarations
      class DataFile;
      //@endgroup
      
      
      ////////////////////////////////////////////////////////////
      // class Frame
      //
      // Defines an object that represents a frame (either major or minor) from
      // a TOB2/3 file.  This object, once properly initialised, will contain
      // the frame data and meta-data and will provide methods to access these
      // fields.  
      ////////////////////////////////////////////////////////////
      class Frame
      {
      protected:
         ////////////////////////////////////////////////////////////
         // contents
         //
         // This holds the data for the entire frame.
         ////////////////////////////////////////////////////////////
         StrBin contents;

         ////////////////////////////////////////////////////////////
         // file_offset
         //
         // Records the offset that this frame occupies in the file. 
         ////////////////////////////////////////////////////////////
         int8 file_offset;

         ////////////////////////////////////////////////////////////
         // source_file
         //
         // Reference to the source file from which this frame was last read.
         // This value will be filled in by class DataFile whenever a frame is
         // read from the file.  It is used to help extract record related
         // info. 
         ////////////////////////////////////////////////////////////
         DataFile *source_file;

         ////////////////////////////////////////////////////////////
         // flags
         ////////////////////////////////////////////////////////////
         uint4 flags;

      public:
         ////////////////////////////////////////////////////////////
         // constructor (default)
         ////////////////////////////////////////////////////////////
         Frame():
            file_offset(0),
            source_file(0),
            flags(0)
         { }

         ////////////////////////////////////////////////////////////
         // constructor (copy)
         ////////////////////////////////////////////////////////////
         Frame(Frame const &other):
            contents(other.contents),
            file_offset(other.file_offset),
            source_file(other.source_file),
            flags(other.flags)
         { }

         ////////////////////////////////////////////////////////////
         // set_contents
         ////////////////////////////////////////////////////////////
         void set_contents(
            void const *buff,
            size_t buff_len,
            int8 file_offset,
            DataFile *source_file_ = 0);

         ////////////////////////////////////////////////////////////
         // get_contents
         ////////////////////////////////////////////////////////////
         StrBin const &get_contents() const
         { return contents; }
         
         ////////////////////////////////////////////////////////////
         // data constructor
         ////////////////////////////////////////////////////////////
         Frame(
            void const *buff,
            size_t buff_len,
            int8 file_offset_,
            DataFile *source_file_ = 0)
         { set_contents(buff,buff_len,file_offset_,source_file_); } 

         ////////////////////////////////////////////////////////////
         // get_is_valid
         ////////////////////////////////////////////////////////////
         bool get_is_valid(uint2 file_validation_stamp = 0) const;

         ////////////////////////////////////////////////////////////
         // get_time_sec
         ////////////////////////////////////////////////////////////
         uint4 get_time_sec() const;

         ////////////////////////////////////////////////////////////
         // get_time_subsec
         ////////////////////////////////////////////////////////////
         uint4 get_time_subsec() const;

         ////////////////////////////////////////////////////////////
         // get_time
         //
         // Returns the beginning time stamp associated with this frame based
         // upon the provided sub-second resolution.  The sub-second resolution
         // value provided is expected to report the number of nano-seconds in
         // the sub-second value. 
         ////////////////////////////////////////////////////////////
         LgrDate get_time(uint4 subsec_res);

         ////////////////////////////////////////////////////////////
         // get_record_no
         ////////////////////////////////////////////////////////////
         uint4 get_record_no() const;

         ////////////////////////////////////////////////////////////
         // get_minor_frame_size
         ////////////////////////////////////////////////////////////
         uint2 get_minor_frame_size() const;

         ////////////////////////////////////////////////////////////
         // has_file_mark
         ////////////////////////////////////////////////////////////
         bool has_file_mark(bool already_called = false);

         ////////////////////////////////////////////////////////////
         // has_remove_mark
         ////////////////////////////////////////////////////////////
         bool has_remove_mark(bool already_called = false);

         ////////////////////////////////////////////////////////////
         // is_empty
         ////////////////////////////////////////////////////////////
         bool is_empty() const;

         ////////////////////////////////////////////////////////////
         // is_minor
         ////////////////////////////////////////////////////////////
         bool is_minor() const;

         ////////////////////////////////////////////////////////////
         // get_validation_stamp
         ////////////////////////////////////////////////////////////
         uint2 get_validation_stamp() const;

         ////////////////////////////////////////////////////////////
         // get_data_ptr
         ////////////////////////////////////////////////////////////
         void const *get_data_ptr() const;

         ////////////////////////////////////////////////////////////
         // get_data_len
         ////////////////////////////////////////////////////////////
         size_t get_data_len() const;

         ////////////////////////////////////////////////////////////
         // get_header_len
         ////////////////////////////////////////////////////////////
         size_t get_header_len() const;

         ////////////////////////////////////////////////////////////
         // get_records_count
         //
         // Returns the number of records stored in this frame or subframe. 
         ////////////////////////////////////////////////////////////
         uint4 get_records_count(bool already_called = false);

         ////////////////////////////////////////////////////////////
         // extract_subframes
         //
         // Generates ther list of subframes that are contained within this
         // frame.  These sub-frames must share the same validation stamp (or
         // its complement) as the container and will be generated in the order
         // in which they occur in the parent frame.  The generated list will
         // be empty if the parent container is not marked as a minor frame.  
         ////////////////////////////////////////////////////////////
         typedef std::list<Frame> subframes_type;
         void extract_subframes(subframes_type &subframes);

         ////////////////////////////////////////////////////////////
         // get_file_offset
         ////////////////////////////////////////////////////////////
         int8 get_file_offset() const
         { return file_offset; }

         ////////////////////////////////////////////////////////////
         // get_frame_pos
         //
         // Returns the position of this frame in the source file. 
         ////////////////////////////////////////////////////////////
         int8 get_frame_pos();

         ////////////////////////////////////////////////////////////
         // get_newest_record_info
         //
         // Returns the stamp and record number for the newest record within
         // this frame or its sub-frames.  If this frame has not been
         // appropriately initialised, a std::exception derived object will be
         // thrown.  The return value will be set to true if the information
         // could be read.  It will be false if the frame is empty.
         //
         // The do_recurse parameter controls whether the method will be called
         // for the last subframe.  The application should leave this
         // parameter as false or not specify it (let the default rule).
         ////////////////////////////////////////////////////////////
         bool get_newest_record_info(
            LgrDate *stamp,
            uint4 *record_no = 0,
            bool do_recurse = true);

         ////////////////////////////////////////////////////////////
         // get_record
         //
         // Evaluates the stamp, record number, and record data for the
         // supplied record index.  If this frame has not been initialised, a
         // std::exception derived object will be thrown.  The values will be
         // returned in the non-null parameters.  The return value will be true
         // if a record associated with the record index exists in this frame
         // (or one of its subframes).
         //
         // If the file_mark_after and remove_mark_after pointers are non-null,
         // this method will mark these either true or false depending on
         // whether there should be a file mark or a remove mark after this
         // record. 
         ////////////////////////////////////////////////////////////
         bool get_record(
            LgrDate *stamp,
            uint4 *record_no,
            StrBin *record_data,
            bool *file_mark_after,
            bool *remove_mark_after,
            uint4 record_index,
            bool already_called = false);

         ////////////////////////////////////////////////////////////
         // get_record_offset
         //
         // Determines the byte offset into the frame for the specified record
         // index.  This offset will be relative to the beginning of the frame
         // and will not involve the frame's beginning position in the file. 
         ////////////////////////////////////////////////////////////
         int8 get_record_offset(uint4 record_index);

      private:
         ////////////////////////////////////////////////////////////
         // read_word
         ////////////////////////////////////////////////////////////
         uint4 read_word(size_t offset) const;

         ////////////////////////////////////////////////////////////
         // generate_subframes
         ////////////////////////////////////////////////////////////
         void generate_subframes();

         ////////////////////////////////////////////////////////////
         // subframes
         //
         // Caches the subframes extracted through the extract_subframes
         // method.  This cache is needed so that frame methods that need to
         // access subframes do not have to continually regenerate the list of
         // subframes. 
         ////////////////////////////////////////////////////////////
         subframes_type subframes;
      };
   };
};


#endif

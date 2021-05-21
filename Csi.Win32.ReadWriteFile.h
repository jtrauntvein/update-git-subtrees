/* Csi.Win32.ReadWriteFile.h

   Copyright (C) 2000, 2011 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 29 November 2000
   Last Change: Tuesday 05 July 2011
   Last Commit: $Date: 2011-07-05 12:55:49 -0600 (Tue, 05 Jul 2011) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Csi_Win32_ReadWriteFile_h
#define Csi_Win32_ReadWriteFile_h

#define WIN32_LEAN_AND_MEAN
#define _WINSOCKAPI_ // prevent windows.h from including winsock.h
#include <windows.h>
#include <stdexcept>
#include "CsiTypeDefs.h"
#include "StrAsc.h"


namespace Csi
{
   namespace Win32
   {
      ////////////////////////////////////////////////////////////
      // class ReadWriteFile
      //
      // Defines an object that uses Win32 API calls to maintain an open handle to a file opened for
      // both reading and writing. It provides a collection of methods simlar to those available in
      // the "C" run-time library and is intended to be used in places like the cora server cache
      // where the "C" run time imposes too many limitations.
      //
      // The files that are created or opened will have the following attributes:
      //   - not sharable for writing
      //   - read and write access
      ////////////////////////////////////////////////////////////
      class ReadWriteFile
      {
      public:
         ////////////////////////////////////////////////////////////
         // class exc_invalid_state
         //
         // Defines the class of object that will be thrown if any operation is attempted while the
         // object is in an invalid state for that operation.
         ////////////////////////////////////////////////////////////
         class exc_invalid_state: public std::exception
         {
         public:
            virtual char const *what() const throw ()
            { return "Invalid state for Csi::Win32::ReadWriteFile object"; }
         };

      public:
         ////////////////////////////////////////////////////////////
         // constructor
         //////////////////////////////////////////////////////////// 
         ReadWriteFile();

         ////////////////////////////////////////////////////////////
         // destructor
         //////////////////////////////////////////////////////////// 
         ~ReadWriteFile();

         ////////////////////////////////////////////////////////////
         // open
         //
         // Attempts to open the specified file with the assumption that the file already
         // exists. Will throw a std::exception derived object if the operation fails or the file
         // handle is already open.
         //////////////////////////////////////////////////////////// 
         void open(char const *file_name_);

         ////////////////////////////////////////////////////////////
         // create
         //
         // Attempts to create the specified file. The file will be overwritten if it already
         // exists. Will throw a std::exception derived object if the operation fails or the file
         // handle is already open.
         //////////////////////////////////////////////////////////// 
         void create(char const *file_name_);

         ////////////////////////////////////////////////////////////
         // append
         //
         // Attempts to open the file to append data.  If the file does not
         // exist, a new file will be created.  If the file is already in an
         // open state, a std::exception derived object will be thrown. 
         ////////////////////////////////////////////////////////////
         void append(char const *file_name_);

         ////////////////////////////////////////////////////////////
         // close
         //
         // Closes the file handle (if open) and sets the file handle to an invalid value again. In
         // this state, open() or create() can be called again.
         //////////////////////////////////////////////////////////// 
         void close();

         ////////////////////////////////////////////////////////////
         // read
         //
         // Reads the specified number of bytes into the specified buffer. Will throw a
         // std::exception derived object if the read fails.
         //////////////////////////////////////////////////////////// 
         void read(void *buffer, uint4 buffer_len);

         ////////////////////////////////////////////////////////////
         // write
         //
         // Writes the specified number of bytes from the specified buffer. Will throw a
         // std::exception derived object if the operation fails.
         //////////////////////////////////////////////////////////// 
         void write(void const *buffer, uint4 buffer_len);

         ////////////////////////////////////////////////////////////
         // tell
         //
         // Returns the current read/write location in the file. Will throw a std::exception derived
         // object if the operation fails.
         //////////////////////////////////////////////////////////// 
         int8 tell();

         ////////////////////////////////////////////////////////////
         // seek
         //
         // Sets the current read/write location in the file. Will throw a std::exception derived
         // object if the operation fails.
         //////////////////////////////////////////////////////////// 
         enum seek_mode_type
         {
            seek_from_begin = FILE_BEGIN,
            seek_from_current = FILE_CURRENT,
            seek_from_end = FILE_END
         };
         void seek(int8 seek_offset, seek_mode_type seek_mode = seek_from_begin);

         ////////////////////////////////////////////////////////////
         // flush
         //
         // Causes any bytes that have accumulated in the operating system buffers to be flushed to
         // disc.
         //////////////////////////////////////////////////////////// 
         void flush();

         ////////////////////////////////////////////////////////////
         // is_open
         //
         // Returns true if the file is open
         //////////////////////////////////////////////////////////// 
         bool is_open() const;

         ////////////////////////////////////////////////////////////
         // flush_os_buffers
         //
         // Performs the same task as flush() but does not return until everything has been written
         // to disc.
         //////////////////////////////////////////////////////////// 
         void flush_os_buffers();

         ////////////////////////////////////////////////////////////
         // get_file_name
         ////////////////////////////////////////////////////////////
         StrAsc const &get_file_name() const
         { return file_name; }

         ////////////////////////////////////////////////////////////
         // size
         //
         // Returns the size of the file
         ////////////////////////////////////////////////////////////
         int8 size();
         
      private:
         ////////////////////////////////////////////////////////////
         // file_name
         ////////////////////////////////////////////////////////////
         StrAsc file_name;
         
         ////////////////////////////////////////////////////////////
         // file_handle
         //
         // Handle to the file. In a closed state, this member will be set to
         // INVALID_HANDLE_VALUE. Any other value will indicate that the file is in an open state.
         //////////////////////////////////////////////////////////// 
         HANDLE file_handle;

         ////////////////////////////////////////////////////////////
         // buffer_size
         //
         // Controls the number of bytes that will be allocated for the buffer
         //////////////////////////////////////////////////////////// 
         static uint4 const buffer_size;

         ////////////////////////////////////////////////////////////
         // write_buffer
         //
         // Holds characters that have not yet been written to the disc file. This buffer will be
         // flushed whenever a seek or flush operation occurs.
         //////////////////////////////////////////////////////////// 
         byte *write_buffer;

         ////////////////////////////////////////////////////////////
         // write_buffer_len
         //
         // Holds the number of bytes that are in the write buffer that still need to be written to
         // disc
         //////////////////////////////////////////////////////////// 
         uint4 write_buffer_len;
      };
   };
};

#endif

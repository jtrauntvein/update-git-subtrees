/* Csi.Posix.ReadWriteFile.h

   Copyright (C) 2005, 2011 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 03 August 2005
   Last Change: Tuesday 05 July 2011
   Last Commit: $Date: 2011-07-05 12:55:49 -0600 (Tue, 05 Jul 2011) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Csi_Posix_ReadWriteFile_h
#define Csi_Posix_ReadWriteFile_h

#include "StrAsc.h"
#include "CsiTypeDefs.h"
#include <stdexcept>
#include <stdio.h>


namespace Csi
{
   namespace Posix
   {
      ////////////////////////////////////////////////////////////
      // class ReadWriteFile
      //
      // Defines an object that uses posix calls to maintain an open
      // handle to a file opened for bothn reading and writing in binary mode.
      // The files will be created/opened using the following modes:
      //
      //  - exclusive write access
      //  - sharable for reading
      ////////////////////////////////////////////////////////////
      class ReadWriteFile
      {
      public:
         ////////////////////////////////////////////////////////////
         // class exc_invalid_state
         //
         // Defines the class of object that will be thrown if any operation is
         // attempted while the object is in an invalid state for that
         // operation.
         ////////////////////////////////////////////////////////////
         class exc_invalid_state: public std::exception
         {
         public:
            virtual char const *what() const throw ()
            { return "Invalid state for Csi::Posix::ReadWriteFile object"; }
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
         // Attempts to open the specified file with the assumption that the
         // file already exists. Will throw a std::exception derived object if
         // the operation fails or the file handle is already open.
         //////////////////////////////////////////////////////////// 
         void open(char const *file_name_);

         ////////////////////////////////////////////////////////////
         // create
         //
         // Attempts to create the specified file. The file will be overwritten
         // if it already exists. Will throw a std::exception derived object if
         // the operation fails or the file handle is already open.
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
         // Closes the file handle (if open) and sets the file handle to an
         // invalid value again. In this state, open() or create() can be
         // called again.
         //////////////////////////////////////////////////////////// 
         void close();

         ////////////////////////////////////////////////////////////
         // read
         //
         // Reads the specified number of bytes into the specified buffer. Will
         // throw a std::exception derived object if the read fails.
         //////////////////////////////////////////////////////////// 
         void read(void *buffer, uint4 buffer_len);

         ////////////////////////////////////////////////////////////
         // write
         //
         // Writes the specified number of bytes from the specified
         // buffer. Will throw a std::exception derived object if the operation
         // fails.
         //////////////////////////////////////////////////////////// 
         void write(void const *buffer, uint4 buffer_len);

         ////////////////////////////////////////////////////////////
         // tell
         //
         // Returns the current read/write location in the file. Will throw a
         // std::exception derived object if the operation fails.
         //////////////////////////////////////////////////////////// 
         int8 tell();

         ////////////////////////////////////////////////////////////
         // seek
         //
         // Sets the current read/write location in the file. Will throw a
         // std::exception derived object if the operation fails.
         //////////////////////////////////////////////////////////// 
         enum seek_mode_type
         {
            seek_from_begin = SEEK_SET,
            seek_from_current = SEEK_CUR,
            seek_from_end = SEEK_END
         };
         void seek(int8 seek_offset, seek_mode_type seek_mode = seek_from_begin);

         ////////////////////////////////////////////////////////////
         // flush
         //
         // Causes any bytes that have accumulated in the operating system
         // buffers to be flushed to disc.
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
         // file_desc
         ////////////////////////////////////////////////////////////
         int file_desc;

         ////////////////////////////////////////////////////////////
         // write_buffer
         //
         // Used to buffer bytes that would otherwise be written directly to
         // the file.  This policy is similar to the buffering done by the "C"
         // run time library. 
         ////////////////////////////////////////////////////////////
         byte *write_buffer;

         ////////////////////////////////////////////////////////////
         // write_buffer_len
         //
         // Specifies the number of bytes waiting in the write buffer to be
         // written. 
         ////////////////////////////////////////////////////////////
         uint4 write_buffer_len;
      };
   };
};


#endif

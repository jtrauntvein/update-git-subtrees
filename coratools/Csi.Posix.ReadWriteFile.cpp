/* Csi.Posix.ReadWriteFile.cpp

   Copyright (C) 2005, 2014 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 03 August 2005
   Last Change: Friday 29 August 2014
   Last Commit: $Date: 2014-09-02 08:44:49 -0600 (Tue, 02 Sep 2014) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Posix.ReadWriteFile.h"
#include "Csi.Posix.OsException.h"
#include "Csi.MaxMin.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


namespace Csi
{
   namespace Posix
   {
      namespace
      {
         ////////////////////////////////////////////////////////////
         // buffer_size
         //
         // Specifies the size of the write buffers used.  
         ////////////////////////////////////////////////////////////
         uint4 const buffer_size = 2048;
      };

      
      ////////////////////////////////////////////////////////////
      // class ReadWriteFile definitions
      //////////////////////////////////////////////////////////// 
      ReadWriteFile::ReadWriteFile():
         file_desc(-1),
         write_buffer(0),
         write_buffer_len(0)
      { }


      ReadWriteFile::~ReadWriteFile()
      { close(); }


      void ReadWriteFile::open(char const *file_name_)
      {
         close();
         file_desc = ::open(
            file_name_,
            O_RDWR);
         if(file_desc == -1)
            throw OsException("File open failed");
         file_name = file_name_;
         write_buffer = new byte[buffer_size];
         write_buffer_len = 0;
      } // open


      void ReadWriteFile::create(char const *file_name_)
      {
         close();
         file_desc = ::open(
            file_name_,
            O_RDWR | O_CREAT | O_TRUNC,
            S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
         if(file_desc == -1)
            throw OsException("File open failed");
         file_name = file_name_;
         write_buffer = new byte[buffer_size];
         write_buffer_len = 0;
      } // create


      void ReadWriteFile::append(char const *file_name_)
      {
         close();
         file_desc = ::open(
            file_name_,
            O_RDWR | O_CREAT,
            S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
         if(file_desc == -1)
            throw OsException("File open failed");
         file_name = file_name_;
         write_buffer = new byte[buffer_size];
         write_buffer_len = 0;
         lseek(file_desc, 0, SEEK_END);
      } // append


      void ReadWriteFile::close()
      {
         if(file_desc != -1)
         {
            flush();
            ::close(file_desc);
            file_desc = -1;
            file_name.cut(0);
            delete[] write_buffer;
            write_buffer = 0;
            write_buffer_len = 0;
         }
      } // close


      void ReadWriteFile::read(void *buffer, uint4 buffer_len)
      {
         if(file_desc == -1)
            throw exc_invalid_state();
         if(write_buffer_len > 0)
            flush();
         ssize_t rcd = ::read(file_desc,buffer,buffer_len);
         if(rcd < buffer_len)
            throw OsException("read failure");
      } // read


      void ReadWriteFile::write(void const *buffer, uint4 buffer_len)
      {
         // check the file descriptor
         if(file_desc == -1)
            throw exc_invalid_state();

         // we need to calculate how much of the specified buffer will fit in the write buffer
         uint4 available = buffer_size - write_buffer_len;
         uint4 remaining = buffer_len;
         uint4 what_will_fit;
         byte const *b = static_cast<byte const *>(buffer);

         while(remaining > 0)
         {
            what_will_fit = csimin(available,remaining);
            memcpy(
               write_buffer + write_buffer_len,
               b,
               what_will_fit);
            b += what_will_fit;
            available -= what_will_fit;
            remaining -= what_will_fit;
            write_buffer_len += what_will_fit;

            // if the buffer is full, it needs to be written to the file.
            if(available == 0)
            {
               flush();
               available = buffer_size;
            }
         }
      } // write


      int8 ReadWriteFile::tell()
      {
         if(file_desc == -1)
            throw exc_invalid_state();
         off_t rtn = lseek(file_desc,0,SEEK_CUR);
         if(rtn == -1)
            throw OsException("tell failure");
         rtn += write_buffer_len;
         return rtn;
      } // tell


      void ReadWriteFile::seek(int8 seek_offset, seek_mode_type seek_mode)
      {
         flush();
         off_t rcd = lseek(
            file_desc,
            seek_offset,
            seek_mode);
         if(rcd == -1)
            throw OsException("seek failure");
      } // seek


      void ReadWriteFile::flush()
      {
         if(file_desc == -1)
            throw exc_invalid_state();
         if(write_buffer_len > 0)
         {
            ssize_t rcd = ::write(file_desc,write_buffer,write_buffer_len);
            if(rcd != write_buffer_len)
               throw OsException("flush failure");
            write_buffer_len = 0;
         }
      } // flush


      bool ReadWriteFile::is_open() const
      { return file_desc != -1; }


      void ReadWriteFile::flush_os_buffers()
      {
         int rcd;
         flush();
         fsync(file_desc);
         if(rcd == -1)
            throw OsException("flush_os_buffers failure");
      } // flush_os_buffers


      int8 ReadWriteFile::size()
      {
         struct stat info;
         int rcd;
         
         flush();
         rcd = fstat(file_desc,&info);
         if(rcd == -1)
            throw OsException("size failure");
         return info.st_size;
      } // size
   };
};


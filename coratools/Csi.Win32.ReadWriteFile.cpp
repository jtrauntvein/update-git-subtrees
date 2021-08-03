/* Csi.Win32.ReadWriteFile.cpp

   Copyright (C) 2000, 2013 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 29 November 2000
   Last Change: Thursday 14 November 2013
   Last Commit: $Date: 2013-11-14 12:02:16 -0600 (Thu, 14 Nov 2013) $ 
   Committed by: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Win32.ReadWriteFile.h"
#include "Csi.OsException.h"
#include "Csi.MaxMin.h"
#include "StrUni.h"

namespace Csi
{
   namespace Win32
   {
      ////////////////////////////////////////////////////////////
      // class ReadWriteFile definitions
      ////////////////////////////////////////////////////////////
      uint4 const ReadWriteFile::buffer_size = 2048;
      
      
      ReadWriteFile::ReadWriteFile():
         file_handle(INVALID_HANDLE_VALUE),
         write_buffer(0),
         write_buffer_len(0)
      { }

      
      ReadWriteFile::~ReadWriteFile()
      { close(); }


      void ReadWriteFile::open(char const *file_name_)
      {
         if(file_handle == INVALID_HANDLE_VALUE)
         {
            StrUni wfile(file_name_);
            file_handle = ::CreateFileW(
               wfile.c_str(),
               GENERIC_READ|GENERIC_WRITE,
               FILE_SHARE_READ,
               0,               // default security
               OPEN_EXISTING,
               FILE_ATTRIBUTE_NORMAL,
               0);              // no template
            if(file_handle == INVALID_HANDLE_VALUE)
               throw OsException("Failed to open file");
            file_name = file_name_;
            write_buffer = new byte[buffer_size]; 
         }
         else
            throw exc_invalid_state();
      } // open

      
      void ReadWriteFile::create(char const *file_name_)
      {
         if(file_handle == INVALID_HANDLE_VALUE)
         {
            StrUni wfile(file_name_);
            file_handle = ::CreateFileW(
               wfile.c_str(),
               GENERIC_READ|GENERIC_WRITE,
               FILE_SHARE_READ,
               0,               // default security
               CREATE_ALWAYS,
               FILE_ATTRIBUTE_NORMAL,
               0);              // no template
            if(file_handle == INVALID_HANDLE_VALUE)
               throw OsException("Failed to open file");
            file_name = file_name_;
            write_buffer = new byte[buffer_size];
         }
         else
            throw exc_invalid_state();
      } // create


      void ReadWriteFile::append(char const *file_name_)
      {
         if(file_handle == INVALID_HANDLE_VALUE)
         {
            StrUni wfile(file_name_);
            file_handle = ::CreateFileW(
               wfile.c_str(),
               GENERIC_READ|GENERIC_WRITE,
               FILE_SHARE_READ,
               0,               // default security
               OPEN_ALWAYS,
               FILE_ATTRIBUTE_NORMAL,
               0);              // no template
            if(file_handle == INVALID_HANDLE_VALUE)
               throw OsException("Failed to open file");
            file_name = file_name_;
            write_buffer = new byte[buffer_size];
            seek(0, seek_from_end);
         }
         else
            throw exc_invalid_state();
      } // append

      
      void ReadWriteFile::close()
      {
         if(file_handle != INVALID_HANDLE_VALUE)
         {
            flush();
            CloseHandle(file_handle);
            file_handle = INVALID_HANDLE_VALUE;
            delete[] write_buffer;
            write_buffer = 0;
         }
      } // close

      
      void ReadWriteFile::read(void *buffer, uint4 buffer_len)
      {
         if(file_handle != INVALID_HANDLE_VALUE)
         {
            uint4 count;
            BOOL rcd = ::ReadFile(
               file_handle,
               buffer,
               buffer_len,
               &count,
               0);              // no overlapped structure
            if(!rcd)
               throw OsException("Csi::Win32::ReadWriteFile::read failure");
            if(count < buffer_len)
               throw MsgExcept("Csi::Win32::ReadWriteFile::read end of file");
         }
         else
            throw exc_invalid_state();
      } // read

      
      void ReadWriteFile::write(void const *buffer, uint4 buffer_len)
      {
         if(file_handle != INVALID_HANDLE_VALUE)
         {
            // calculate how much room is available in the write buffer
            uint4 available = buffer_size - write_buffer_len;
            uint4 remaining = buffer_len;
            uint4 what_will_fit;
            byte const *byte_buffer = static_cast<byte const *>(buffer);
            
            while(remaining > 0)
            {
               // copy what will fit to the end of the write buffer.
               what_will_fit = Csi::csimin(available,remaining);
               memcpy(
                  write_buffer + write_buffer_len,
                  byte_buffer,
                  what_will_fit);
               byte_buffer += what_will_fit;
               available -= what_will_fit;
               remaining -= what_will_fit;
               write_buffer_len += what_will_fit;

               // if the buffer is full, it is now time to write to the disc
               if(available == 0)
               {
                  flush();
                  available = buffer_size;
               }
            }
         }
         else
            throw exc_invalid_state();
      } // write

      
      int8 ReadWriteFile::tell()
      {
         LARGE_INTEGER rtn;
         rtn.QuadPart = 0;
         if(file_handle != INVALID_HANDLE_VALUE)
         {
            // we get the current file position by calling SetFilePointer with arguments that will
            // not change that position.
            int rcd = ::SetFilePointerEx(
               file_handle,
               rtn,               // do not move the pointer
               &rtn,
               FILE_CURRENT);
            if(!rcd)
               throw OsException("Csi::Win32::ReadWriteFile::tell error");

            // The number of bytes that remain to be written in the write cache should be included
            // in the return value
            rtn.QuadPart += write_buffer_len;
         }
         else
            throw exc_invalid_state();
         return rtn.QuadPart;
      } // tell

      
      void ReadWriteFile::seek(int8 seek_offset, seek_mode_type seek_mode)
      {
         if(file_handle != INVALID_HANDLE_VALUE)
         {
            // we need to flush the write buffer before seeking so that remaining bytes can be
            // written in the appropriate spot
            flush();

            // set the file pointer
            LARGE_INTEGER offset;
            offset.QuadPart = seek_offset;
            BOOL rcd = ::SetFilePointerEx(
               file_handle,
               offset,
               0,               // don't worry about the return value
               seek_mode);
            if(!rcd)
               throw OsException("Csi::Win32::ReadWriteFile::seek error");
         }
         else
            throw exc_invalid_state();
      } // seek


      void ReadWriteFile::flush()
      {
         if(file_handle != INVALID_HANDLE_VALUE)
         {
            if(write_buffer_len > 0)
            {
               uint4 write_count;
               BOOL rcd = ::WriteFile(
                  file_handle,
                  write_buffer,
                  write_buffer_len,
                  &write_count,
                  0);        // no overlapped
               if(!rcd)
                  throw OsException("Csi::Win32::ReadWriteFile::flush failed");
               write_buffer_len = 0;
            }
         }
         else
            throw exc_invalid_state();
      } // flush

      
      bool ReadWriteFile::is_open() const
      { return file_handle != INVALID_HANDLE_VALUE; }


      void ReadWriteFile::flush_os_buffers()
      {
         if(file_handle != INVALID_HANDLE_VALUE)
         {
            flush();
            ::FlushFileBuffers(file_handle);
         }
         else
            throw exc_invalid_state();
      } // flush_os_buffers


      int8 ReadWriteFile::size()
      {
         LARGE_INTEGER rtn;
         if(file_handle != INVALID_HANDLE_VALUE)
            ::GetFileSizeEx(file_handle, &rtn);
         return rtn.QuadPart;
      } // size
   };
};

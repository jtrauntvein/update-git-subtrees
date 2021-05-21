/* Csi.FileStream.h

   Copyright (C) 2008, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 09 October 2008
   Last Change: Wednesday 03 October 2012
   Last Commit: $Date: 2012-10-04 07:49:06 -0600 (Thu, 04 Oct 2012) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Csi_FileStream_h
#define Csi_FileStream_h

#include "Csi.ReadWriteFile.h"
#include <iostream>


namespace Csi
{
   ////////////////////////////////////////////////////////////
   // class OFileStreamBuff
   ////////////////////////////////////////////////////////////
   class OFileStreamBuff: public std::streambuf
   {
   private:
      ////////////////////////////////////////////////////////////
      // output
      ////////////////////////////////////////////////////////////
      ReadWriteFile output;

   public:
      ////////////////////////////////////////////////////////////
      // constructor (default)
      ////////////////////////////////////////////////////////////
      OFileStreamBuff()
      { }

      ////////////////////////////////////////////////////////////
      // open
      ////////////////////////////////////////////////////////////
      void open(char const *file_name, std::ios_base::openmode mode = std::ios_base::out)
      {
         if(output.is_open())
            output.close();
         if(mode & std::ios_base::app)
            output.append(file_name);
         else
            output.create(file_name);
      }

      ////////////////////////////////////////////////////////////
      // close
      ////////////////////////////////////////////////////////////
      void close()
      { output.close(); }

      ////////////////////////////////////////////////////////////
      // overflow
      ////////////////////////////////////////////////////////////
      virtual int_type overflow(int_type ch_)
      {
         char ch = static_cast<char>(ch_);
         if(output.is_open())
            output.write(&ch, 1);
         return ch_;
      }

      ////////////////////////////////////////////////////////////
      // xsputn
      ////////////////////////////////////////////////////////////
      virtual std::streamsize xsputn(char const *buff, std::streamsize buff_len)
      {
         std::streamsize rtn = 0;
         if(output.is_open())
         {
            output.write(buff, static_cast<uint4>(buff_len));
            rtn = buff_len;
         }
         return rtn;
      } // xsputn

      ////////////////////////////////////////////////////////////
      // sync
      ////////////////////////////////////////////////////////////
      virtual int sync()
      {
         int rtn = -1;
         if(output.is_open())
         {
            output.flush();
            rtn = 0;
         }
         return rtn;
      }

      ////////////////////////////////////////////////////////////
      // seekoff
      ////////////////////////////////////////////////////////////
      virtual std::streampos seekoff(
         std::streamoff offset, std::ios_base::seekdir way, std::ios_base::openmode)
      {
         ReadWriteFile::seek_mode_type seek_mode;
         switch(way)
         {
         default:
         case std::ios_base::beg:
            seek_mode = ReadWriteFile::seek_from_begin;
            break;
            
         case std::ios_base::end:
            seek_mode = ReadWriteFile::seek_from_end;
            break;
            
         case std::ios_base::cur:
            seek_mode = ReadWriteFile::seek_from_current;
            break;
         }
         output.seek(static_cast<int4>(offset), seek_mode);
         return output.tell();
      }

      ////////////////////////////////////////////////////////////
      // tellp
      ////////////////////////////////////////////////////////////
      std::streampos tellp()
      { return output.tell(); }

      ////////////////////////////////////////////////////////////
      // size
      ////////////////////////////////////////////////////////////
      int8 size()
      { return output.size(); }
   };


   ////////////////////////////////////////////////////////////
   // class OFileStream
   ////////////////////////////////////////////////////////////
   class OFileStream: public std::ostream
   {
   protected:
      ////////////////////////////////////////////////////////////
      // buffer
      ////////////////////////////////////////////////////////////
      OFileStreamBuff buffer;

   public:
      ////////////////////////////////////////////////////////////
      // constructor (default)
      ////////////////////////////////////////////////////////////
      OFileStream():
         std::ostream(&buffer)
      { }

      ////////////////////////////////////////////////////////////
      // constructor (for file name)
      ////////////////////////////////////////////////////////////
      explicit OFileStream(char const *file_name, std::ios_base::openmode mode = std::ios_base::out):
         std::ostream(&buffer)
      {
         try
         {
            buffer.open(file_name, mode);
         }
         catch(std::exception &)
         { setstate(std::ios::badbit); }
      }

      ////////////////////////////////////////////////////////////
      // open
      ////////////////////////////////////////////////////////////
      void open(char const *file_name, std::ios_base::openmode mode = std::ios_base::out)
      {
         try
         {
            buffer.open(file_name, mode);
         }
         catch(std::exception &)
         { setstate(std::ios::badbit); }
      }

      ////////////////////////////////////////////////////////////
      // close
      ////////////////////////////////////////////////////////////
      void close()
      {
         buffer.close();
         setstate(std::ios::badbit); 
      } // close

      ////////////////////////////////////////////////////////////
      // size
      //
      // Returns the size of the opened file.
      ////////////////////////////////////////////////////////////
      int8 size()
      { return buffer.size(); }

      ////////////////////////////////////////////////////////////
      // tellp
      ////////////////////////////////////////////////////////////
      std::streampos tellp()
      { return buffer.tellp(); }
   };
};


#endif

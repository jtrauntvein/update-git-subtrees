/* Csi.BuffStream.h

   Copyright (C) 2006, 2010 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 24 May 2006
   Last Change: Saturday 14 August 2010
   Last Commit: $Date: 2010-08-16 07:30:10 -0600 (Mon, 16 Aug 2010) $ 
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Csi_BuffStream_h
#define Csi_BuffStream_h

#include <iostream>


namespace Csi
{
   ////////////////////////////////////////////////////////////
   // class IBuffStreamBuf
   //
   // Defines a stream buffer that provides read capabilities for a supplied
   // array.  
   ////////////////////////////////////////////////////////////
   class IBuffStreamBuf: public std::streambuf
   {
   public:
      ////////////////////////////////////////////////////////////
      // constructor
      ////////////////////////////////////////////////////////////
      IBuffStreamBuf(char const *buff, size_t buff_len)
      { set_buffer(buff, buff_len); }

      ////////////////////////////////////////////////////////////
      // set_buffer
      ////////////////////////////////////////////////////////////
      void set_buffer(char const *buff, size_t buff_len)
      {
         if(buff)
         {
            setg(
               const_cast<char *>(buff),
               const_cast<char *>(buff),
               const_cast<char *>(buff + buff_len));
         }
      }

   protected:
      ////////////////////////////////////////////////////////////
      // seekoff
      ////////////////////////////////////////////////////////////
      virtual pos_type seekoff(
         off_type offset,
         std::ios::seekdir way,
         std::ios::openmode which)
      {
         pos_type rtn;
         switch(way)
         {
         case std::ios::beg:
            rtn = seekpos(pos_type(offset),which);
            break;
            
         case std::ios::cur:
            rtn = seekpos(pos_type(offset) + pos_type(gptr() - eback()),which);
            break;
            
         case std::ios::end:
            rtn = seekpos(pos_type(offset) + pos_type(egptr() - eback()),which);
            break;
            
         default:
            rtn = pos_type(off_type(-1));
            break;
         }
         return rtn;
      }

      ////////////////////////////////////////////////////////////
      // seekpos
      ////////////////////////////////////////////////////////////
      virtual pos_type seekpos(
         pos_type pos,
         std::ios::openmode which)
      {
         pos_type rtn = pos_type(off_type(-1));
         if(which == std::ios::in)
         {
            off_type offset = off_type(pos);
            if(offset >= 0 && offset <= egptr() - eback())
            {
               setg(eback(), eback() + offset, egptr());
               rtn = pos;
            }
         }
         return rtn;
      }
   };


   ////////////////////////////////////////////////////////////
   // class IBuffStream
   ////////////////////////////////////////////////////////////
   class IBuffStream: public std::istream
   {
   private:
      ////////////////////////////////////////////////////////////
      // buff
      ////////////////////////////////////////////////////////////
      IBuffStreamBuf buffer;

   public:
      ////////////////////////////////////////////////////////////
      // constructor
      ////////////////////////////////////////////////////////////
      IBuffStream(void const *buff, size_t buff_len):
         buffer(static_cast<char const *>(buff),buff_len),
         std::istream(&buffer)
      { }
   };


   ////////////////////////////////////////////////////////////
   // class IBuffStreamBufw
   //
   // Defines a stream buffer that provides read capabilities for a supplied
   // array.  
   ////////////////////////////////////////////////////////////
   class IBuffStreamBufw: public std::wstreambuf
   {
   public:
      ////////////////////////////////////////////////////////////
      // constructor
      ////////////////////////////////////////////////////////////
      IBuffStreamBufw(wchar_t const *buff, size_t buff_len)
      { set_buffer(buff, buff_len); }


      ////////////////////////////////////////////////////////////
      // set_buffer
      ////////////////////////////////////////////////////////////
      void set_buffer(wchar_t const *buff, size_t buff_len)
      {
         if(buff)
         {
            setg(
               const_cast<wchar_t *>(buff),
               const_cast<wchar_t *>(buff),
               const_cast<wchar_t *>(buff + buff_len));
         }
      }
      
   protected:
      ////////////////////////////////////////////////////////////
      // seekoff
      ////////////////////////////////////////////////////////////
      virtual pos_type seekoff(
         off_type offset,
         std::ios::seekdir way,
         std::ios::openmode which)
      {
         pos_type rtn;
         switch(way)
         {
         case std::ios::beg:
            rtn = seekpos(pos_type(offset),which);
            break;
            
         case std::ios::cur:
            rtn = seekpos(pos_type(offset) + pos_type(gptr() - eback()),which);
            break;
            
         case std::ios::end:
            rtn = seekpos(pos_type(offset) + pos_type(egptr() - eback()),which);
            break;
            
         default:
            rtn = pos_type(off_type(-1));
            break;
         }
         return rtn;
      }

      ////////////////////////////////////////////////////////////
      // seekpos
      ////////////////////////////////////////////////////////////
      virtual pos_type seekpos(
         pos_type pos,
         std::ios::openmode which)
      {
         pos_type rtn = pos_type(off_type(-1));
         if(which == std::ios::in)
         {
            off_type offset = off_type(pos);
            if(offset >= 0 && offset <= egptr() - eback())
            {
               setg(eback(), eback() + offset, egptr());
               rtn = pos;
            }
         }
         return rtn;
      }
   };


   ////////////////////////////////////////////////////////////
   // class IBuffStream
   ////////////////////////////////////////////////////////////
   class IBuffStreamw: public std::wistream
   {
   protected:
      ////////////////////////////////////////////////////////////
      // buffer
      ////////////////////////////////////////////////////////////
      IBuffStreamBufw buffer;

   public:
      ////////////////////////////////////////////////////////////
      // constructor
      ////////////////////////////////////////////////////////////
      IBuffStreamw(wchar_t const *buff, size_t buff_len):
         buffer(buff,buff_len),
         std::wistream(&buffer)
      { }
   };   
};


#endif

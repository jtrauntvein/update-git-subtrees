/* Csi.StrAscStream.h

   Copyright (C) 2005, 2015 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 11 March 2005
   Last Change: Wednesday 09 December 2015
   Last Commit: $Date: 2015-12-09 11:40:21 -0600 (Wed, 09 Dec 2015) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Csi_StrUniStream_h
#define Csi_StrUniStream_h

#include "StrUni.h"
#include <iostream>


namespace Csi
{
   //@group class forward declarations
   class OStrUniStream;
   //@endgroup

   
   ////////////////////////////////////////////////////////////
   // class OStrUniStreamBuffer
   //
   // Defines a stream buffer that uses a shared pointer to a StrUni object as
   // its destination.
   ////////////////////////////////////////////////////////////
   class OStrUniStreamBuffer: public std::wstreambuf
   {
   private:
      ////////////////////////////////////////////////////////////
      // output
      ////////////////////////////////////////////////////////////
      StrUni output;

      friend class OStrUniStream;
      
   public:
      ////////////////////////////////////////////////////////////
      // constructor
      ////////////////////////////////////////////////////////////
      OStrUniStreamBuffer(StrUni output_ = StrUni()):
         output(output_)
      { }

      ////////////////////////////////////////////////////////////
      // overflow
      ////////////////////////////////////////////////////////////
      virtual int_type overflow(int_type ch)
      {
         output.append(static_cast<wchar_t>(ch));
         return ch;
      }

      ////////////////////////////////////////////////////////////
      // xsputn
      ////////////////////////////////////////////////////////////
      virtual std::streamsize xsputn(
         wchar_t const *buff,
         std::streamsize buff_len)
      {
         output.append(buff, static_cast<size_t>(buff_len));
         return buff_len;
      }
   };


   ////////////////////////////////////////////////////////////
   // class OStrUniStream
   ////////////////////////////////////////////////////////////
   class OStrUniStream:
      public std::wostream
   {
   protected:
      ////////////////////////////////////////////////////////////
      // buffer
      ////////////////////////////////////////////////////////////
      OStrUniStreamBuffer buffer;

   public:
      ////////////////////////////////////////////////////////////
      // constructor
      ////////////////////////////////////////////////////////////
      typedef StrUni string_type;
      OStrUniStream(StrUni output = StrUni()):
         buffer(output),
         std::wostream(&buffer)
      { }

      ////////////////////////////////////////////////////////////
      // other string constructor (null terminated)
      ////////////////////////////////////////////////////////////
      OStrUniStream(StrUni const &buff):
         buffer(buff),
         std::wostream(&buffer)
      { }

      ////////////////////////////////////////////////////////////
      // copy constructor
      ////////////////////////////////////////////////////////////
      OStrUniStream(OStrUniStream &s):
         buffer(s.str()),
         std::wostream(&buffer)
      { }

      ////////////////////////////////////////////////////////////
      // str
      //
      // Returns the string used by the buffer
      ////////////////////////////////////////////////////////////
      StrUni &str()
      { return buffer.output; }

      ////////////////////////////////////////////////////////////
      // str (const version)
      ////////////////////////////////////////////////////////////
      StrUni const &str() const
      { return buffer.output; }

      ////////////////////////////////////////////////////////////
      // str (with assignment)
      ////////////////////////////////////////////////////////////
      StrUni &str(StrUni const &value)
      {
         buffer.output = value;
         return buffer.output;
      }

      /**
       * @return Returns the pointer to the beginning of the buffer.
       */
      wchar_t const *c_str() const
      { return buffer.output.c_str(); }

      /**
       * @return Returns the number of characters that have been stored.
       */
      size_t length() const
      { return buffer.output.length(); }
   };
};


#endif

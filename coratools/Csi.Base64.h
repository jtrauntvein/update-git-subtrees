/* Csi.Base64.h

   Copyright (C) 2004, 2017 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 21 April 2004
   Last Change: Friday 31 March 2017
   Last Commit: $Date: 2017-04-03 16:25:43 -0600 (Mon, 03 Apr 2017) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Csi_Base64_h
#define Csi_Base64_h

#include "StrAsc.h"
#include "StrBin.h"


namespace Csi
{
   namespace Base64
   {
      ////////////////////////////////////////////////////////////
      // encode
      //
      // Encodes the supplied buffer (interpreted as a string of unsigned
      // characters) to the supplied StrAsc destination buffer.
      ////////////////////////////////////////////////////////////
      void encode(
         StrAsc &dest,
         void const *buff,
         size_t buff_len);

      ////////////////////////////////////////////////////////////
      // encode (stream)
      ////////////////////////////////////////////////////////////
      void encode(
         std::ostream &dest,
         void const *buff,
         size_t buff_len,
         bool break_lines = true);

      ////////////////////////////////////////////////////////////
      // encode (wide stream)
      ////////////////////////////////////////////////////////////
      void encode(
         std::wostream &dest,
         void const *buff,
         size_t buff_len,
         bool break_lines = true);

      ////////////////////////////////////////////////////////////
      // decode
      //
      // Decodes the supplied base64 string to a binary buffer.
      ////////////////////////////////////////////////////////////
      void decode(
         StrBin &dest,
         char const *buff,
         size_t buff_len);
      void decode(
         StrBin &dest,
         wchar_t const *buff,
         size_t buff_len);
   };
};


#endif

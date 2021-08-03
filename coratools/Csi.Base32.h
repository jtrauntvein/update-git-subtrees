/* Csi.Base32.h

   Copyright (C) 2004, 2005 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 23 April 2004
   Last Change: Friday 25 March 2005
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)
   Last Changed by: $Author: tmecham $

*/

#ifndef Csi_Base32_h
#define Csi_Base32_h

#include "StrAsc.h"
#include "StrBin.h"


namespace Csi
{
   namespace Base32
   {
      ////////////////////////////////////////////////////////////
      // function encode
      //
      // Encodes the supplied buffer (interpreted as a string of unsigned
      // characters) to the supplied StrAsc buffer.  This algorithm works most
      // efficiently when the size of the input buffer is a multiple of five.
      ////////////////////////////////////////////////////////////
      void encode(
         StrAsc &dest,
         void const *buff,
         size_t buff_len);

      ////////////////////////////////////////////////////////////
      // function decode
      //
      // Decodes the supplied input buffer assuming that it was encoded using
      // Base32 encoding.  
      ////////////////////////////////////////////////////////////
      void decode(
         StrBin &dest,
         void const *buff,
         size_t buff_len);
   };
};


#endif

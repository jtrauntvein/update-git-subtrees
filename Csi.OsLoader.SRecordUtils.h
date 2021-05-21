/* Csi.OsLoader.SRecordUtils.h

   Copyright (C) 2016, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Friday 18 March 2016
   Last Change: Thursday 31 March 2016
   Last Commit: $Date: 2016-03-31 11:37:58 -0600 (Thu, 31 Mar 2016) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_OsLoader_SRecordUtils_h
#define Csi_OsLoader_SRecordUtils_h

#include "CsiTypeDefs.h"


namespace Csi
{
   namespace OsLoader
   {
      /**
       * @return Returns the byte value for the specified hexadecimal character.
       *
       * @param ch Specifies the character to convert.
       */
      uint2 hex_nibble(char ch);
      
      /**
       * @return Returns the byte value for the two adjacent hex characters at the beginning of
          * the specified string buffer.
          *
          * @param s Specifies the string to convert.  This value must have at least two bytes
          * allocated.  Termination does not matter.
          */
      inline uint2 hex_to_byte(char const *s)
      { return (hex_nibble(s[0]) << 4) + hex_nibble(s[1]); }
      
      /**
       * @return Returns the two byte value associated with four adjacent hex characters in the
       * specifed string buffer.
       *
       * @param s Specifies the string buffer to convert.  This value must have at least four
       * byets allocated and termination is ignored.
       */
      inline uint2 hex_to_uint2(char const *s)
      {
         uint2 b1(hex_to_byte(s));
         uint2 b2(hex_to_byte(s + 2));
         return (b1 << 8) + b2;
      }
      
      /**
       * @return Returns the check sum for the S-record starting at the specified line buffer.
       *
       * @param s Specifies the start of the line to calculate.
       *
       * @param slen Specifies the length of the line to check.
       */
      uint4 srecord_checksum(char const *s, uint4 slen);
      
      /**
       * @return Returns true if the specified line is considered to be a valid SRecord.
       *
       * @param s Specifies the beginning of the line to consider.
       *
       8 @param slen Specifies the number of byets in the line.
      */
      bool is_valid_srecord(char const *s, uint4 slen);
   };
};


#endif

/* Csi.OsLoader.SRecordUtils.cpp

   Copyright (C) 2016, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Friday 18 March 2016
   Last Change: Thursday 31 March 2016
   Last Commit: $Date: 2016-03-31 11:37:58 -0600 (Thu, 31 Mar 2016) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.OsLoader.SRecordUtils.h"
#include <stdexcept>


namespace Csi
{
   namespace OsLoader
   {
      uint2 hex_nibble(char ch)
      {
         byte rtn(0);
         switch(ch)
         {
         case '0':
            rtn = 0;
            break;
            
         case '1':
            rtn = 1;
            break;
            
         case '2':
            rtn = 2;
            break;
            
         case '3':
            rtn = 3;
            break;
            
         case '4':
            rtn = 4;
            break;
            
         case '5':
            rtn = 5;
            break;
            
         case '6':
            rtn = 6;
            break;
            
         case '7':
            rtn = 7;
            break;
            
         case '8':
            rtn = 8;
            break;
            
         case '9':
            rtn = 9;
            break;
            
         case 'a':
         case 'A':
            rtn = 0x0a;
            break;
            
         case 'b':
         case 'B':
            rtn = 0x0b;
            break;
            
         case 'c':
         case 'C':
            rtn = 0x0c;
            break;
            
         case 'd':
         case 'D':
            rtn = 0x0d;
            break;
            
         case 'e':
         case 'E':
            rtn = 0x0e;
            break;
            
         case 'f':
         case 'F':
            rtn = 0x0f;
            break;
            
         default:
            throw std::invalid_argument("invalid hexadecimal character");
            break;
         }
         return rtn;
      } // hex_nibble
      
      
      uint4 srecord_checksum(char const *s, uint4 slen)
      {
         uint4 rtn(0);
         uint4 start(s[0] == ':' ? 1 : 0);
         for(uint4 i = start; i + 1 < slen; i += 2)
         {
            uint4 value(hex_to_byte(s + i));
            rtn += value;
         }
         rtn &= 0xff;
         rtn = 256 - rtn;
         if(rtn == 256)
            rtn = 0;
         return rtn;
      }
      
      
      bool is_valid_srecord(char const *s, uint4 slen)
      {
         bool rtn(true);
         try
         {
            if(slen > 3 && s[0] == ':')
            {
               uint4 calc_checksum = srecord_checksum(s, slen - 2);
               uint4 stored_checksum = hex_to_byte(s + slen - 2);
               if(calc_checksum != stored_checksum)
                  rtn = false;
            }
            else
               rtn = false;
         }
         catch(std::exception &)
         { rtn = false; }
         return rtn;
      } // is_valid_srecord
   };
};

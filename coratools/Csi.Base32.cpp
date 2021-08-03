/* Csi.Base32.cpp

   Copyright (C) 2004, 2006 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 23 April 2004
   Last Change: Tuesday 17 January 2006
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)
   Last Changed by: $Author: tmecham $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Base32.h"
#include "CsiTypeDefs.h"


namespace Csi
{
   namespace Base32
   {
      namespace
      {
         ////////////////////////////////////////////////////////////
         // function encode_char
         ////////////////////////////////////////////////////////////
         char encode_char(byte value)
         {
            char rtn = 'A';
            static char const value_map[] =
               "ABCDEFGHJKLMNPQRSTUVWXYZ23456789";
            if(value < sizeof(value_map) - 1)
               rtn = value_map[value];
            return rtn;
         } // encode_char


         ////////////////////////////////////////////////////////////
         // function decode_char
         ////////////////////////////////////////////////////////////
         byte decode_char(char value)
         {
            byte rtn = 32;
            switch(value)
            {
            case 'a':
            case 'A':
               rtn = 0;
               break;
            case 'b':
            case 'B':
               rtn = 1;
               break;
            case 'c':
            case 'C':
               rtn = 2;
               break;
            case 'd':
            case 'D':
               rtn = 3;
               break;
            case 'e':
            case 'E':
               rtn = 4;
               break;
            case 'f':
            case 'F':
               rtn = 5;
               break;
            case 'g':
            case 'G':
               rtn = 6;
               break;
            case 'h':
            case 'H':
               rtn = 7;
               break;
            case 'j':
            case 'J':
               rtn = 8;
               break;
            case 'k':
            case 'K':
               rtn = 9;
               break;
            case 'l':
            case 'L':
               rtn = 10;
               break;
            case 'm':
            case 'M':
               rtn = 11;
               break;
            case 'n':
            case 'N':
               rtn = 12;
               break;
            case 'p':
            case 'P':
               rtn = 13;
               break;
            case 'q':
            case 'Q':
               rtn = 14;
               break;
            case 'r':
            case 'R':
               rtn = 15;
               break;
            case 's':
            case 'S':
               rtn = 16;
               break;
            case 't':
            case 'T':
               rtn = 17;
               break;
            case 'u':
            case 'U':
               rtn = 18;
               break;
            case 'v':
            case 'V':
               rtn = 19;
               break;
            case 'w':
            case 'W':
               rtn = 20;
               break;
            case 'x':
            case 'X':
               rtn = 21;
               break;
            case 'y':
            case 'Y':
               rtn = 22;
               break;
            case 'z':
            case 'Z':
               rtn = 23;
               break;
            case '2':
               rtn = 24;
               break;
            case '3':
               rtn = 25;
               break;
            case '4':
               rtn = 26;
               break;
            case '5':
               rtn = 27;
               break;
            case '6':
               rtn = 28;
               break;
            case '7':
               rtn = 29;
               break;
            case '8':
               rtn = 30;
               break;
            case '9':
               rtn = 31;
               break;
            }
            return rtn;
         } // decode_char
      };


      ////////////////////////////////////////////////////////////
      // function encode definition
      ////////////////////////////////////////////////////////////
      void encode(
         StrAsc &dest,
         void const *buff_,
         size_t buff_len)
      {
         // the algorithm handles its input five bytes at a time.  If the length of the input is not
         // evenly divisible by five, we will pad with zeroes.
         byte const *buff = static_cast<byte const *>(buff_);
         dest.cut(0);
         for(size_t i = 0; i < buff_len; i += 5)
         {
            // assign the input byte values
            byte i1 = buff[i];
            byte i2 = (i + 1 < buff_len ? buff[i + 1] : 0);
            byte i3 = (i + 2 < buff_len ? buff[i + 2] : 0);
            byte i4 = (i + 3 < buff_len ? buff[i + 3] : 0);
            byte i5 = (i + 4 < buff_len ? buff[i + 4] : 0);

            // these values will now be arranged into eight words of five bits each
            byte o1 = ((i1 & 0xF8) >> 3);
            byte o2 = ((i1 & 0x07) << 2) | ((i2 & 0xC0) >> 6);
            byte o3 = ((i2 & 0x3E) >> 1);
            byte o4 = ((i2 & 0x01) << 4) | ((i3 & 0xF0) >> 4);
            byte o5 = ((i3 & 0x0F) << 1) | ((i4 & 0x80) >> 7);
            byte o6 = ((i4 & 0x7C) >> 2);
            byte o7 = ((i4 & 0x03) << 3) | ((i5 & 0xE0) >> 5);
            byte o8 = (i5 & 0x1F);

            // we will now output as many words that are needed to represent the input
            dest += encode_char(o1);
            dest += encode_char(o2);
            if(i + 1 < buff_len)
            {
               dest += encode_char(o3);
               dest += encode_char(o4);
            }
            else
               dest += '*';
            if(i + 2 < buff_len)
            {
               dest += encode_char(o5);
               dest += encode_char(o6);
            }
            else
               dest += '*';
            if(i + 3 < buff_len)
            {
               dest += encode_char(o7);
               dest += encode_char(o8);
            }
            else
               dest += '*';
         }
      } // encode


      ////////////////////////////////////////////////////////////
      // function decode definition
      ////////////////////////////////////////////////////////////
      void decode(
         StrBin &dest,
         void const *buff_,
         size_t buff_len)
      {
         // the input will be processed eight bytes at a time.  It will be padded with 'A'
         // characters if the input buffer length is not evenly divisible by eight.
         char const *buff = static_cast<char const *>(buff_);
         dest.cut(0);
         for(size_t i = 0; i < buff_len; i += 8)
         {
            // assign eight valeus of the input
            char c1 = buff[i];
            char c2 = (i + 1 < buff_len ? buff[i + 1] : 'A');
            char c3 = (i + 2 < buff_len ? buff[i + 2] : 'A');
            char c4 = (i + 3 < buff_len ? buff[i + 3] : 'A');
            char c5 = (i + 4 < buff_len ? buff[i + 4] : 'A');
            char c6 = (i + 5 < buff_len ? buff[i + 5] : 'A');
            char c7 = (i + 6 < buff_len ? buff[i + 6] : 'A');
            char c8 = (i + 7 < buff_len ? buff[i + 7] : 'A');
            
            // assign eight of the values of the input
            char i1 = decode_char(c1);
            char i2 = decode_char(c2);
            char i3 = decode_char(c3);
            char i4 = decode_char(c4);
            char i5 = decode_char(c5);
            char i6 = decode_char(c6);
            char i7 = decode_char(c7);
            char i8 = decode_char(c8);

            // we can now manipulate these words into bytes for the output
            byte o1 = (i1 << 3) | ((i2 & 0x1C) >> 2);
            dest.append(o1);
            if(c3 != '*')
            {
               byte o2 = ((i2 & 0x03) << 6) | (i3 << 1) | ((i4 & 0x10) >> 4);
               dest.append(o2);
            }
            if(c4 != '*')
            {
               byte o3 = ((i4 & 0x0F) << 4) | ((i5 & 0x1E) >> 1);
               dest.append(o3);
            }
            if(c5 != '*')
            {
               byte o4 = ((i5 & 0x01) << 7) | (i6 << 2) | ((i7 & 0x18) >> 3);
               dest.append(o4);
            }
            if(c7 != '*')
            {
               byte o5 = ((i7 & 0x07) << 5) | i8;
               dest.append(o5);
            }
         }
      } // decode
   };
};



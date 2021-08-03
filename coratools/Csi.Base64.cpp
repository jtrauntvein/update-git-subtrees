/* Csi.Base64.cpp

   Copyright (C) 2004, 2014 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 21 April 2004
   Last Change: Thursday 21 August 2014
   Last Commit: $Date: 2017-04-03 16:25:43 -0600 (Mon, 03 Apr 2017) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Base64.h"
#include "CsiTypeDefs.h"
#include <iostream>
#include <cctype>
#include <cwctype>


namespace Csi
{
   namespace Base64
   {
      namespace
      {
         ////////////////////////////////////////////////////////////
         // function encode_char
         ////////////////////////////////////////////////////////////
         char encode_char(byte value)
         {
            char rtn = '/';
            if(value < 26)
               rtn = 'A' + static_cast<char>(value);
            else if(value < 52)
               rtn = 'a' + static_cast<char>(value) - 26;
            else if(value < 62)
               rtn = '0' + static_cast<char>(value) - 52;
            else if(value == 62)
               rtn = '+';
            return rtn;
         } // encode_char


         ////////////////////////////////////////////////////////////
         // function decode_char
         ////////////////////////////////////////////////////////////
         byte decode_char(char value)
         {
            byte rtn = 64;
            if(value >= 'A' && value <= 'Z')
               rtn = static_cast<byte>(value) - 'A';
            else if(value >= 'a' && value <= 'z')
               rtn = static_cast<byte>(value) - 'a' + 26;
            else if(value >= '0' && value <= '9')
               rtn = static_cast<byte>(value) - '0' + 52;
            else if(value == '+')
               rtn = 62;
            else if(value == '/')
               rtn = 63;
            return rtn;
         } // decode_char
      };


      ////////////////////////////////////////////////////////////
      // function encode(string)
      ////////////////////////////////////////////////////////////
      void encode(
         StrAsc &dest,
         void const *buff_,
         size_t buff_len)
      {
         // the algorithm handles the input three bytes at a time.  If the length of the input is
         // not evenly divisible by three, we will pad with zeroes.
         byte const *buff = static_cast<byte const *>(buff_);
         dest.cut(0);
         dest.reserve(buff_len * 4 / 3 + 1);
         for(size_t i = 0; i < buff_len; i += 3)
         {
            // get the three bytes from the array
            byte b1 = buff[i], b2 = 0, b3 = 0;
            if(i + 1 < buff_len)
               b2 = buff[i + 1];
            if(i + 2 < buff_len)
               b3 = buff[i + 2];

            // we will now arrange these
            byte b4 = b1 >> 2;
            byte b5 = ((b1 & 0x03) << 4) | (b2 >> 4);
            byte b6 = ((b2 & 0x0f) << 2) | (b3 >> 6);
            byte b7 = (b3 & 0x3f);
            dest += encode_char(b4);
            dest += encode_char(b5);
            if(i + 1 < buff_len)
               dest += encode_char(b6);
            else
               dest += '=';
            if(i + 2 < buff_len)
               dest += encode_char(b7);
            else
               dest += '=';
         }         
      } // encode (buffer)


      ////////////////////////////////////////////////////////////
      // function encode(stream) definition
      ////////////////////////////////////////////////////////////
      void encode(
         std::ostream &out,
         void const *buff_,
         size_t buff_len,
         bool break_lines)
      {
         // the algorithm handles the input three bytes at a time.  If the length of the input is
         // not evenly divisible by three, we will pad with zeroes.
         byte const *buff = static_cast<byte const *>(buff_);
         uint4 bytes_written(0);
         for(size_t i = 0; i < buff_len; i += 3)
         {
            // get the three bytes from the array
            byte b1 = buff[i], b2 = 0, b3 = 0;
            if(i + 1 < buff_len)
               b2 = buff[i + 1];
            if(i + 2 < buff_len)
               b3 = buff[i + 2];

            // we will now arrange these
            byte b4 = b1 >> 2;
            byte b5 = ((b1 & 0x03) << 4) | (b2 >> 4);
            byte b6 = ((b2 & 0x0f) << 2) | (b3 >> 6);
            byte b7 = (b3 & 0x3f);
            out << encode_char(b4);
            out << encode_char(b5);
            if(i + 1 < buff_len)
               out << encode_char(b6);
            else
               out << '=';
            if(i + 2 < buff_len)
               out << encode_char(b7);
            else
               out << '=';
            bytes_written += 4;
            if(break_lines && bytes_written % 64 == 0)
               out << "\r\n";
         }
      } // encode


      ////////////////////////////////////////////////////////////
      // function encode(stream) definition
      ////////////////////////////////////////////////////////////
      void encode(
         std::wostream &out,
         void const *buff_,
         size_t buff_len,
         bool break_lines)
      {
         // the algorithm handles the input three bytes at a time.  If the length of the input is
         // not evenly divisible by three, we will pad with zeroes.
         byte const *buff = static_cast<byte const *>(buff_);
         uint4 bytes_written(0);
         for(size_t i = 0; i < buff_len; i += 3)
         {
            // get the three bytes from the array
            byte b1 = buff[i], b2 = 0, b3 = 0;
            if(i + 1 < buff_len)
               b2 = buff[i + 1];
            if(i + 2 < buff_len)
               b3 = buff[i + 2];

            // we will now arrange these
            byte b4 = b1 >> 2;
            byte b5 = ((b1 & 0x03) << 4) | (b2 >> 4);
            byte b6 = ((b2 & 0x0f) << 2) | (b3 >> 6);
            byte b7 = (b3 & 0x3f);
            out << static_cast<wchar_t>(encode_char(b4));
            out << static_cast<wchar_t>(encode_char(b5));
            if(i + 1 < buff_len)
               out << static_cast<wchar_t>(encode_char(b6));
            else
               out << L'=';
            if(i + 2 < buff_len)
               out << static_cast<wchar_t>(encode_char(b7));
            else
               out << L'=';
            bytes_written += 4;
            if(break_lines && bytes_written % 64 == 0)
               out << '\n';
         }
      } // encode


      ////////////////////////////////////////////////////////////
      // function decode definition
      ////////////////////////////////////////////////////////////
      void decode(
         StrBin &dest,
         char const *buff,
         size_t const buff_len)
      {
         // we will decide the string four bytes at a time.
         size_t i = 0;
         dest.cut(0);
         dest.reserve(buff_len);
         while(i < buff_len)
         {
            // pull the values off of the input
            char c1 = buff[i];
            char c2 = 'A';
            char c3 = 'A';
            char c4 = 'A';
            if(!std::isspace(c1))
            {
               if(i + 1 < buff_len)
                  c2 = buff[++i];
               if(i + 1 < buff_len)
                  c3 = buff[++i];
               if(i + 1 < buff_len)
                  c4 = buff[++i];
               
               // decode the values
               byte b1 = decode_char(c1);
               byte b2 = decode_char(c2);
               byte b3 = decode_char(c3);
               byte b4 = decode_char(c4);
               dest.append((b1 << 2) | (b2 >> 4));
               if(c3 != '=')
                  dest.append(((b2 & 0x0f) << 4) | (b3 >> 2));
               if(c4 != '=')
                  dest.append(((b3 & 0x03) << 6) | b4);
            }
            ++i;
         }
      } // decode


      void decode(
         StrBin &dest,
         wchar_t const *buff,
         size_t const buff_len)
      {
         // we will decide the string four bytes at a time.
         size_t i = 0;
         dest.cut(0);
         dest.reserve(buff_len);
         while(i < buff_len)
         {
            // pull the values off of the input
            wchar_t c1 = buff[i];
            wchar_t c2 = L'A';
            wchar_t c3 = L'A';
            wchar_t c4 = L'A';
            if(!std::iswspace(c1))
            {
               if(i + 1 < buff_len)
                  c2 = buff[++i];
               if(i + 1 < buff_len)
                  c3 = buff[++i];
               if(i + 1 < buff_len)
                  c4 = buff[++i];
               
               // decode the values
               byte b1 = decode_char(static_cast<char>(c1));
               byte b2 = decode_char(static_cast<char>(c2));
               byte b3 = decode_char(static_cast<char>(c3));
               byte b4 = decode_char(static_cast<char>(c4));
               dest.append((b1 << 2) | (b2 >> 4));
               if(c3 != '=')
                  dest.append(((b2 & 0x0f) << 4) | (b3 >> 2));
               if(c4 != '=')
                  dest.append(((b3 & 0x03) << 6) | b4);
            }
            ++i;
         }
      } // decode
   };
};


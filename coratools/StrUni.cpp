/* StrUni.cpp

   Copyright (C) 1998, 2013 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 21 September 1998
   Last Change: Tuesday 30 April 2013
   Last Commit: $Date: 2017-12-19 16:29:39 -0600 (Tue, 19 Dec 2017) $ 
   Committed by: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "StrUni.h"
#include "Csi.StringLoader.h"
#include "Csi.Utils.h"
#include <iostream>
#include <limits.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define _WINSOCKAPI_ // prevent windows.h from including winsock.h
#include <windows.h>
#endif


////////////////////////////////////////////////////////////
// class StrUni definitions
////////////////////////////////////////////////////////////
StrUni::StrUni()
{ }


StrUni::StrUni(StrUni const &other)
{ setContents(other); }


StrUni::StrUni(wchar_t const *buff)
{ setContents(buff); }


StrUni::StrUni(char const *buff, bool utf8)
{
   if(utf8)
      append_utf8(buff);
   else
      append_mb(buff);
}


StrUni::StrUni(wchar_t const *buff, size_t buff_len)
{ setContents(buff,buff_len); }


StrUni::StrUni(StrAsc const &s, bool utf8)
{
   if(utf8)
      append_utf8(s.c_str(), s.length());
   else
      append_mb(s.c_str(), s.length());
} // construct from StrAsc


StrUni &StrUni::operator =(StrUni const &other)
{ setContents(other); return *this; }


StrUni &StrUni::operator =(wchar_t const *buff)
{ setContents(buff); return *this; }
   

StrUni &StrUni::operator =(char const *buff)
{
   cut(0);
   append_utf8(buff);
   return *this;
} // copy from char pointer


StrUni &StrUni::operator =(StrAsc const &s)
{
   cut(0);
   append_utf8(s.c_str(), s.length());
   return *this;
} // copy from StrAsc


void StrUni::toMulti(StrAsc &dest) const
{
   // we need a locale to perform the appropriate conversion
   using namespace std;
   std::locale loc(Csi::StringLoader::make_locale());
   char temp[MB_LEN_MAX];
   mbstate_t state = { 0 };
   wchar_t const *next1;
   char *next2;
   int result;

   // convert each unicode character into the appropriate multi-byte sequence
   dest.cut(0);
   for(size_t i = 0; i < buff_len; i++)
   {
      result = use_facet<codecvt<wchar_t, char, mbstate_t> >(loc).out(
         state,
         storage + i,
         storage + i + 1,
         next1,
         temp,
         temp + sizeof(temp),
         next2);
      if(result != codecvt_base::error)
         dest.append(temp,next2 - temp);
   }
} // toMulti


StrAsc StrUni::to_utf8() const
{
   StrAsc rtn;
   rtn.reserve(buff_len);
   for(size_t i = 0; i < buff_len; ++i)
      Csi::unicode_to_utf8(rtn, storage[i]);
   return rtn;
} // to_utf8


void StrUni::append_mb(char const *buff)
{ append_mb(buff,strlen(buff)); }


void StrUni::append_mb(char const *buff, size_t buff_len)
{
#ifdef _WIN32
   // we need to get the number of characters that will be needed for the conversion.  
   UINT current_code_page = ::GetACP();
   int space_needed;

   if(buff_len == 0)
      return;
   space_needed = ::MultiByteToWideChar(
      current_code_page,
      MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
      buff,
      (int)buff_len,
      0,
      0);
   if(space_needed > 0)
   {
      reserve(this->buff_len + space_needed + 1);
      MultiByteToWideChar(
         current_code_page,
         MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
         buff,
         (int)buff_len,
         storage + this->buff_len,
         space_needed);
      this->buff_len += space_needed;
      terminate();
   }
#else
   // measure the length of the buffer
   using namespace std;
   locale loc(Csi::StringLoader::make_locale());
   mbstate_t state = { 0 };
   size_t new_len = use_facet<codecvt<wchar_t, char, mbstate_t> >(loc).length(
      state,
      buff,
      buff + buff_len,
      buff_len);   

   // we can now reserve the required amount and perform the conversion
   wchar_t ch;
   char const *next1 = buff;
   wchar_t *next2;
   int result;
   
   reserve(new_len + length());
   while(next1 < buff + buff_len)
   {
      result = use_facet<codecvt<wchar_t, char, mbstate_t> >(loc).in(
         state,
         next1,
         buff + buff_len,
         next1,
         &ch,
         &ch + 1,
         next2);
      if(result != codecvt_base::error)
         append(ch);
      else
      {
         ch = *next1;
         ++next1;
      }
   }
#endif
} // append_mb


void StrUni::append_utf8(char const *buff)
{ append_utf8(buff, strlen(buff)); }


void StrUni::append_utf8(char const *buff, size_t buff_len)
{
   StrUni temp;
   size_t i = 0;
   bool error(false);
   temp.reserve(buff_len);

   while(!error && i < buff_len)
   {
      byte ch1(buff[i]);
      size_t extra(0);
      uint4 entity(0);

      if((ch1 & 0xfc) == 0xfc)
      {
         entity = (ch1 & 0x01);
         extra = 5;
      }
      else if((ch1 & 0xf8) == 0xf8)
      {
         entity = (ch1 & 0x03);
         extra = 4;
      }
      else if((ch1 & 0xf0) == 0xf0)
      {
         entity = (ch1 & 0x07);
         extra = 3;
      }
      else if((ch1 & 0xe0) == 0xe0)
      {
         entity = (ch1 & 0x0f);
         extra = 2;
      }
      else if((ch1 & 0xc0) == 0xc0)
      {
         entity = (ch1 & 0x1f);
         extra = 1;
      }
      else if((ch1 & 0x80) == 0)
      {
         entity = ch1;
         extra = 0;
      }
      else
         error = true;
      if(!error && extra > 0)
      {
         if(i + extra < buff_len && extra <= 5)
         {
            byte extra_bytes[6];
            memcpy(extra_bytes, buff + i + 1, extra);
            for(size_t j = 0; !error && j < extra; ++j)
            {
               byte ch2(extra_bytes[j]);
               if((ch2 & 0x80) == 0x80)
               {
                  entity <<= 6;
                  entity |= (ch2 & 0x3f);
               }
               else
                  error = true;
            }
         }
         else
            error = true;
      }
      if(!error)
      {
         i += extra + 1;
         if(entity <= 0xffff)
            temp.append(static_cast<wchar_t>(entity));
         else if(entity <= 0x10ffff)
         {
            uint4 uc1(0xd800);
            uint4 uc2(0xdc00);
            uint4 uprime(entity - 0x10000);
            uc1 |= (uprime >> 10) & 0x3ff;
            uc2 |= uprime & 0x3ff;
            temp.append(static_cast<wchar_t>(uc1));
            temp.append(static_cast<wchar_t>(uc2));
         }
         else
            error = true;
      }
   }
   if(error)
      append_mb(buff, buff_len);
   else
      append(temp);
} // append_utf8


std::ostream &operator <<(std::ostream &out, wchar_t const *s)
{
   StrUni temp(s);
   out << temp.to_utf8();
   return out;
} // insertion operator


std::wostream &operator <<(std::wostream &out, StrUni const &s)
{
   out << s.c_str();
   return out;
} // wide insertion operator


std::istream &operator >>(std::istream &in, StrUni &s)
{
   char ch;
   bool skippingWs = true;

   s.cut(0);
   while(in.good())
   {
      //@bugfix 19 May 1999 by Jon Trauntvein
      // The stream state can be good while positioned at the end of the stream. We therefore need
      // to check the stream state after the read. 
      //@endbugfix
      in.get(ch);
      if(in.good())
      {
         if(isspace(ch) && skippingWs)
            continue;
         else if(isspace(ch))
            break;
         else
         {
            s.append(ch);
            skippingWs = false;
         }
      }
   }
   return in;
} // extraction operator

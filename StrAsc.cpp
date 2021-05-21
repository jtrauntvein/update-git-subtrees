/* StrAsc.cpp

   Copyright (C) 2000, 2014 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 18 September 1998
   Last Change: Wednesday 10 December 2014
   Last Commit: $Date: 2014-12-10 15:41:44 -0600 (Wed, 10 Dec 2014) $ 
   Committed by; $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "StrAsc.h"
#include "StrUni.h"
#include "CsiTypeDefs.h"
#include <iostream>


////////////////////////////////////////////////////////////
// class StrAsc definitions
////////////////////////////////////////////////////////////
StrAsc::StrAsc()
{ }


StrAsc::StrAsc(StrAsc const &other)
{ setContents(other); }


StrAsc::StrAsc(char const *buff)
{ setContents(buff); }


StrAsc::StrAsc(char const *buff, size_t buff_len):
   TermStr<char>(buff, buff_len)
{ }



StrAsc &StrAsc::operator =(StrAsc const &other)
{ setContents(other); return *this; }


StrAsc &StrAsc::operator =(char const *buff)
{ setContents(buff); return *this; }


void StrAsc::readLine(FILE *in)
{
   int ch;
   cut(0);
   while((ch = fgetc(in)) != EOF && ch != '\n')
   {
      if(ch != '\r')
         append(char(ch));
   }
} // readLine


void StrAsc::readLine(std::istream &in)
{
   char ch = '\0';

   cut(0);
   while(ch != '\n' && in.good())
   {
      in.get(ch);
      if(ch != '\n' && in.good())
      {
         if(ch != '\r')
            append(ch);
      }
   }
} // readLine


void StrAsc::readToken(std::istream &in)
{
   char ch;
   bool skipping_ws = true;

   cut(0);
   while(in.good())
   {
      in.get(ch);
      if(!in.good())
         break;
      if(isspace(ch) && skipping_ws)
         continue;
      else if(isspace(ch))
         break;
      else
      {
         append(ch);
         skipping_ws = false;
      }
   }
} // readToken


void StrAsc::encodeHex(void const *buff, size_t len, bool use_sep)
{
   byte const *b = (byte const *)buff;
   char temp[3];

   reserve(len*3);              // reserve three spaces for each byte
   for(size_t i = 0; i < len; i++)
   {
#pragma warning(disable: 4996)
      sprintf(temp,"%02x",int(b[i]));
#pragma warning(default: 4996)
      if(temp[1] == '\0')
         append('0');
      append(temp);
      if(i + 1 < len && use_sep)
         append(' ');
   }
} // encodeHex


std::istream &operator >>(std::istream &in, StrAsc &buff)
{ buff.readToken(in); return in; }


std::ostream &operator <<(std::ostream &out, StrAsc const &buff)
{ out << buff.c_str(); return out; }


std::wostream &operator <<(std::wostream &out, StrAsc const &buff)
{
   StrUni temp(buff);
   out << temp;
   return out;
}

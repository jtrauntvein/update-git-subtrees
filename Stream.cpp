/* Stream.cpp

   Copyright (C) 1998, 2017 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 19 February 1999
   Last Change: Tuesday 19 December 2017
   Last Commit: $Date: 2017-12-19 16:29:39 -0600 (Tue, 19 Dec 2017) $ 
   Committed by: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "Stream.h"
#include "StrAsc.h"
#include "truediv.h"
#include <string.h>
#include <algorithm>


////////////////////////////////////////////////////////////
// class Stream definitions
////////////////////////////////////////////////////////////
void Stream::addByte(byte val)
{ addBytes_impl(&val,sizeof(val),false); }


void Stream::addBool(bool val)
{
   byte temp(0);
   if(val)
      temp = 1;
   addBytes_impl(&temp, sizeof(temp), false);
} // addBool


void Stream::addUInt2(uint2 val, bool swapOrder)
{ addBytes_impl(&val,sizeof(val),swapOrder); } // addUInt2


void Stream::addInt2(int2 val, bool swapOrder)
{ addBytes_impl(&val,sizeof(val),swapOrder); } // addInt2


void Stream::addUInt4(uint4 val, bool swapOrder)
{ addBytes_impl(&val,sizeof(val),swapOrder); } // addUInt4


void Stream::addInt4(int4 val, bool swapOrder)
{ addBytes_impl(&val,sizeof(val),swapOrder); } // addInt4


void Stream::addInt8(int8 val, bool swapOrder)
{ addBytes_impl(&val,sizeof(val),swapOrder); } // addInt8


void Stream::addIeee4(float val, bool swapOrder)
{ addBytes_impl(&val,sizeof(val),swapOrder); } // addIeee4


void Stream::addIeee8(double val, bool swapOrder)
{ addBytes_impl(&val,sizeof(val),swapOrder); } // addIeee8


void Stream::addAsciiZ(char const *s)
{ addBytes_impl(s, (uint4)strlen(s) + 1, false); } // addAsciiZ


void Stream::addAscii(char const *s, uint4 len)
{ addBytes_impl(s,len,false); } // addAscii


void Stream::addBytes(void const *src, uint4 len, bool swapOrder)
{ addBytes_impl(src,len,swapOrder); }


void Stream::addBytes(FILE *in, uint4 len)
{
   char buff[512];
   uint4 written = 0;
   while(written < len)
   {
      uint4 to_read = (uint4)sizeof(buff);
      uint4 actual;
      if(to_read > len - written)
         to_read = len - written;
      actual = (uint4)fread(buff, 1, to_read, in);
      if(actual > 0)
      {
         addBytes_impl(buff, actual, false);
         written += actual;
      }
      else
         break;
   }
} // addBytes


void Stream::addSec(Csi::LgrDate const &val, bool swap_order)
{
   uint4 temp = val.get_sec();
   addUInt4(temp,swap_order);
} // addSec


void Stream::addUSec(Csi::LgrDate const &val)
{
   // convert the nanoseconds offset to tens of usec
   int8 usec = val.get_nanoSec() / 10000;
   for(int i = 5; i >= 0; --i)
      addByte(byte((usec >> (i * 8)) & 0xff));
} // addUSec


void Stream::addNSec(Csi::LgrDate const &val, bool swap_order)
{
   int8 q,r;
   truediv(q,r,val.get_nanoSec(),Csi::LgrDate::nsecPerSec);
   addUInt4(static_cast<uint4>(q),swap_order);
   addUInt4(static_cast<uint4>(r),swap_order);
} // addNSec


byte Stream::readByte()
{
   byte rtn;
   readBytes_impl(&rtn,sizeof(rtn),false);
   return rtn;
} // readByte


bool Stream::readBool()
{
   byte temp;
   bool rtn(false);
   readBytes_impl(&temp, sizeof(temp), false);
   if(temp != 0)
      rtn = true;
   return rtn;
} // readBool


uint2 Stream::readUInt2(bool swapOrder)
{
   uint2 rtn;
   readBytes_impl(&rtn,sizeof(rtn),swapOrder);
   return rtn;
} // readUInt2


int2 Stream::readInt2(bool swapOrder)
{
   int2 rtn;
   readBytes_impl(&rtn,sizeof(rtn),swapOrder);
   return rtn;
} // readInt2


uint4 Stream::readUInt4(bool swapOrder)
{
   uint4 rtn;
   readBytes_impl(&rtn,sizeof(rtn),swapOrder);
   return rtn;
} // readUInt4


int4 Stream::readInt4(bool swapOrder)
{
   int4 rtn;
   readBytes_impl(&rtn,sizeof(rtn),swapOrder);
   return rtn;
} // readInt4


float Stream::readIeee4(bool swapOrder)
{
   float rtn;
   readBytes_impl(&rtn,sizeof(rtn),swapOrder);
   return rtn;
} // readIeee4


double Stream::readIeee8(bool swapOrder)
{
   double rtn;
   readBytes_impl(&rtn,sizeof(rtn),swapOrder);
   return rtn;
} // readIeee8


void Stream::readAsciiZ(char *buff, uint4 max_len)
{
   // we will read the buffer a char at a time until the null terminator is found or we have read at
   // most (maxLen - 1) bytes. We will then read ahead until a null is found without storing the
   // bytes. If the null is never found, a ReadException object will be thrown
   char temp;
   uint4 current_pos = 0;

   memset(buff,0,max_len);
   while(current_pos < max_len - 1 && (temp = static_cast<char>(readByte())) != '\0')
      buff[current_pos++] = temp;
   while(temp != '\0')
      temp = static_cast<char>(readByte());
} // readAsciiZ


void Stream::readAsciiZ(StrAsc &buff)
{
   // we will read the buffer a character at a time until we encounter the end of stream (an
   // exception will be throw or a null character.
   char temp;
   buff.cut(0);
   while((temp = static_cast<char>(readByte())) != 0)
      buff.append(temp);
} // readAsciiZ


void Stream::readAscii(char *buff, uint4 len)
{ readBytes_impl(buff,len,false); } // readAscii


void Stream::readBytes(void *dest, uint4 len, bool swapOrder)
{ readBytes_impl(dest,len,swapOrder); } // readBytes


void Stream::readBytes(StrBin &buff, uint4 len, bool swapOrder)
{
   if(length() >= len)
   {
      buff.cut(0);
      buff.reserve(len);
      for(uint4 i = 0; i < len; i++)
         buff.append(readByte());
      if(swapOrder)
         buff.reverse();
   }
   else
      throw ReadException();
} // readBytes_impl


void Stream::readBytes(Stream &buff, uint4 len)
{
   if(length() >= len)
   {
      for(uint4 i = 0; i < len; i++)
         buff.addByte(readByte());
   }
   else
      throw ReadException();
} // readBytes


void Stream::readArray(void *buff, uint4 objLen, uint4 cnt, bool swapOrder)
{
   if(cnt*objLen <= length())
   {
      byte *dest = (byte *)buff;
      for(uint4 i = 0; i < cnt; i++)
      {
         readBytes_impl(dest,objLen,swapOrder);
         dest += objLen;
      }
   }
   else
      throw ReadException();
} // readArray


Csi::LgrDate Stream::readSec(bool swap_order)
{
   uint4 secs = readUInt4(swap_order);
   return Csi::LgrDate(secs * Csi::LgrDate::nsecPerSec);
} // readSec


Csi::LgrDate Stream::readUSec()
{
   int8 rtn = 0;
   byte temp;

   for(int i = 0; i < 6; ++i)
   {
      temp = readByte();
      rtn <<= 8;
      rtn += temp;
   }
   return Csi::LgrDate(rtn * 10000);
} // readUSec


Csi::LgrDate Stream::readNSec(bool swap_order)
{
   int8 sec = readInt4(swap_order);
   int8 nsec = readInt4(swap_order);
   return Csi::LgrDate(Csi::LgrDate::nsecPerSec * sec + nsec);
} // readNSec


void Stream::reverse(void *buff, uint4 len)
{
   byte *b = (byte *)buff;
   uint4 rev_count = len/2;
   
   if(len%2 == 0)
      rev_count--;
   for(uint4 i = 0; i <= rev_count; i++)
      std::swap(b[i],b[len - 1 - i]);   
} // reverse


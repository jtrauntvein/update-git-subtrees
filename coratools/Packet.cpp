/* Packet.cpp

   Copyright (C) 1998, 2017 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 19 August 1998
   Last Change: Tuesday 19 December 2017
   Last Commit: $Date: 2017-12-19 16:29:39 -0600 (Tue, 19 Dec 2017) $ 
   Committed by: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "StrBin.h"
#include "Packet.h"
#include "MsgExcept.h"


////////////////////////////////////////////////////////////
// class Packet definitions
////////////////////////////////////////////////////////////
Packet::Packet(uint4 headerLen_):
   headerLen(headerLen_),
   readIdx(headerLen_),
   msg(new OwnerStream)
{
   msg->init(headerLen);
} //  constructor


Packet::Packet(Packet const &other, uint4 headerLen_, bool deep_copy):
   headerLen(headerLen_),
   readIdx(other.readIdx)
{
   if(deep_copy)
      msg.bind(
         new OwnerStream(
            other.msg->get(),
            other.msg->length()));
   else
      msg = other.msg;
   if(headerLen_ == 0)
      headerLen = other.headerLen;
   if(readIdx < headerLen)
      readIdx = headerLen;
} // copy constructor


Packet::Packet(void const *buff, uint4 len, uint4 headerLen_, bool copy):
   headerLen(headerLen_),
   readIdx(headerLen_)
{
   // the specified buffer length must be at least as large as the header length
   if(len < headerLen)
      throw WriteException();
   if(copy)
      msg.bind(new OwnerStream(buff,len));
   else
      msg.bind(new RenterStream(buff,len));
} // constructor


Packet::Packet(uint4 reserveLen, uint4 headerLen_):
   headerLen(headerLen_),
   readIdx(headerLen_)
{ msg.bind(new OwnerStream(reserveLen)); }


Packet &Packet::operator =(Packet const &other)
{
   msg = other.msg;
   readIdx = other.readIdx;
   headerLen = other.headerLen;
   return *this;
} // copy operator


Packet::~Packet()
{
} // destructor



void Packet::replaceByte(byte val, uint4 offset)
{
   replaceBytes(&val,sizeof(val),offset);
} // replaceByte


void Packet::replaceBool(bool val, uint4 offset)
{
   replaceBytes(&val,sizeof(val),offset);
} // replaceBool


void Packet::replaceUInt2(uint2 val, uint4 offset, bool swapOrder)
{
   replaceBytes(&val,sizeof(val),offset,swapOrder);
} // replaceUInt2


void Packet::replaceInt2(int2 val, uint4 offset, bool swapOrder)
{
   replaceBytes(&val,sizeof(val),offset,swapOrder);
} // replaceInt2


void Packet::replaceUInt4(uint4 val, uint4 offset, bool swapOrder)
{
   replaceBytes(&val,sizeof(val),offset,swapOrder);
} // replaceUInt4


void Packet::replaceInt4(int4 val, uint4 offset, bool swapOrder)
{
   replaceBytes(&val,sizeof(val),offset,swapOrder);
} // replaceInt4


void Packet::replaceIeee4(float val, uint4 offset, bool swapOrder)
{
   replaceBytes(&val,sizeof(val),offset,swapOrder);
} // replaceIeee4


void Packet::replaceIeee8(double val, uint4 offset, bool swapOrder)
{
   replaceBytes(&val,sizeof(val),offset,swapOrder);
} // replaceIeee8


void Packet::replaceBytes(void const *buff, uint4 len, uint4 offset, bool swapOrder)
{
   if(msg.get_rep() == 0)
      throw WriteException();
   if(swapOrder)
   {
      byte const *b = (byte const *)buff;
      for(uint4 i = 0; i < len; i++)
         msg->replace(b + i,1,offset + len - i - 1);
   }
   else
      msg->replace(buff,len,offset);
} // replaceBytes


void Packet::addBytes_impl(void const *buff, uint4 len, bool swapOrder)
{
   if(msg.get_rep() == 0)
      throw WriteException();
   if(swapOrder)
   {
      byte const *b = (byte const *)buff;
      for(uint4 i = 0; i < len; i++)
         msg->write(b + len - i - 1,1);
   }
   else
      msg->write(buff,len);
} // addBytes_impl


void Packet::readBytes_impl(void *buff, uint4 len, bool swapOrder)
{
   if(msg.get_rep() == 0)
      throw ReadException();
   msg->read(buff,len,readIdx,swapOrder);
   readIdx += len;
} // readBytes_impl


Packet::OwnerStream::OwnerStream()
{
} // constructor


Packet::OwnerStream::OwnerStream(uint4 reserveLen)
{ src.reserve(reserveLen); }


Packet::OwnerStream::OwnerStream(void const *buff, uint4 len):
   src(buff,len)
{
} // constructor


void Packet::OwnerStream::read(void *buff, uint4 len, uint4 offset, bool swapOrder)
{
   if(offset + len > src.getLen())
      throw ReadException();
   if(swapOrder)
   {
      byte *b = (byte *)buff;
      byte const *s = (byte const *)src.getContents() + offset;
      for(uint4 i = 0; i < len; i++)
         b[i] = s[len - i - 1];
   }
   else 
      memcpy(buff,src.getContents() + offset,len);
} // read


void Packet::OwnerStream::write(void const *buff, uint4 len, bool swapOrder)
{
   try
   {
      if(swapOrder)
      {
         byte const *b = (byte const *)buff;
         for(uint4 i = 0; i < len; i++)
            src.append(b + len - i - 1,1);
      }
      else
         src.append(buff,len);
   }
   catch(MsgExcept &)
   {
      throw WriteException();
   }
} // write


void Packet::OwnerStream::replace(void const *buff, uint4 len, uint4 offset, bool swapOrder)
{
   if(offset + len > src.getLen())
      throw WriteException();
   if(swapOrder)
   {
      byte const *b = (byte const *)buff;
      byte *s = (byte *)src.getContents() + offset;
      for(uint4 i = 0; i < len; i++)
         s[len - i - 1] = b[i];
   }
   else
      src.replace(buff,len,offset);
} // replace


void Packet::OwnerStream::init(uint4 len)
{
   src.fill((byte)'\0',len);
} // init


uint4 Packet::OwnerStream::length() const
{ return (uint4)src.getLen(); }


void const *Packet::OwnerStream::get() const
{ return src.getContents(); }


Packet::RenterStream::RenterStream(void const *buff_, uint4 len_):
   rentedBuff((byte const *)buff_),
   rentedBuffLen(len_)
{
} // constructor


Packet::RenterStream::~RenterStream()
{
} // destructor


void Packet::RenterStream::read(void *buff, uint4 len, uint4 offset, bool swapOrder)
{
   if(offset + len > rentedBuffLen)
      throw ReadException();
   if(swapOrder)
   {
      byte *b = (byte *)buff;
      for(uint4 i = 0; i < len; i++)
         b[len - i - 1] = rentedBuff[i + offset];
   }
   else
      memcpy(buff,rentedBuff + offset,len);
} // read


uint4 Packet::RenterStream::length() const
{
   return rentedBuffLen;
} // length


void const *Packet::RenterStream::get() const
{
   return rentedBuff;
} // get


void Packet::RenterStream::cut(uint4 pos)
{
   if(pos < rentedBuffLen)
      rentedBuffLen = pos;
} // cut

/* MsgStream.cpp

   Copyright (C) 1998, 2005 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 18 February 1999
   Last Change: Wednesday 19 January 2005
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)
   Committed by: $Author: tmecham $
   
*/

#pragma hdrstop
#include "StrBin.h"
#include "MsgStream.h"
#include "Csi.Utils.h"


////////////////////////////////////////////////////////////
// class MsgStream definitions
////////////////////////////////////////////////////////////
MsgStream::MsgStream(uint4 initial_capacity):
   msg(initial_capacity),
   atEnd(false)
{ }


MsgStream::MsgStream(MsgStream const &other):
   msg(other.msg),
   atEnd(other.atEnd)
{ }


MsgStream &MsgStream::operator =(MsgStream const &other)
{
   msg = other.msg;
   atEnd = other.atEnd;
   return *this;
} // copy operator


MsgStream::~MsgStream()
{ }


byte MsgStream::peekByte()
{
   byte rtn;
   peekBytes(&rtn,sizeof(rtn));
   return rtn;
} // peekByte


void MsgStream::peekBytes(void *dest, uint4 len, bool swapOrder)
{
   if(msg.size() >= len)
   {
      msg.copy(dest,len);
      if(swapOrder)
         reverse(dest,len);
   }
   else
      throw ReadException();
} // peekBytes


void MsgStream::addBytes_impl(void const *buff, uint4 len, bool swapOrder)
{
   if(!atEnd)
   {
      if(swapOrder)
      {
         byte const *b = (byte const *)buff;
         msg.reserve(len);
         for(uint4 i = 0; i < len; i++)
            msg.push(b + len - i - 1,1);
      }
      else
         msg.push(buff,len);
   }
   else
      throw WriteException();
} // addBytes


void MsgStream::readBytes_impl(void *buff, uint4 len, bool swapOrder)
{
   if(len <= msg.size())
   {
      msg.pop(buff,len);
      if(swapOrder)
         reverse(buff,len);
   }
   else
      throw ReadException();
} // readBytes



/* Csi.Messaging.Message.cpp

   Copyright (C) 2000, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 2 August 1996
   Last Change: Friday 29 April 2016
   Last Commit: $Date: 2016-05-03 16:18:03 -0600 (Tue, 03 May 2016) $ 
   Committed by: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Messaging.Message.h"
#include <string.h>
#include <assert.h>


namespace Csi
{
   namespace Messaging
   {
      ////////////////////////////////////////////////////////////
      // class Message Definitions
      ////////////////////////////////////////////////////////////
      const uint4 Message::sesNoPos = 0;
      const uint4 Message::msgTypePos = Message::sesNoPos + sizeof(uint4);
      const uint4 Message::msgHeaderLen = Message::msgTypePos + sizeof(uint4);


      Message::Message():
         Packet(msgHeaderLen)
      { }


      Message::Message(uint4 sesNo, uint4 msgType):
         Packet(msgHeaderLen)
      {  
         Packet::replaceUInt4(sesNo,sesNoPos,true);
         Packet::replaceUInt4(msgType,msgTypePos,true);
      } // end constructor


      Message::Message(Message const &other, bool deep_copy):
         Packet(other,0,deep_copy)
      { }


      Message::Message(void const *buff, uint4 buffLen, bool makeCopy):
         Packet(buff,buffLen,msgHeaderLen,makeCopy)
      { }


      Message::Message(ByteQueue &bq, uint4 len):
         Packet(len,msgHeaderLen)
      {
         // pop the bytes from the queue until the message is filled
         for(uint4 i = 0; i < len; i++)
         {
            byte temp;
            if(!bq.pop(&temp,1))
               assert(false);
            Packet::addByte(temp);
         }
         reset();
      } // queue constructor


      uint4 Message::getClntSesNo()
      {
         uint4 rtn;

         msg->read(&rtn,sizeof(rtn),sesNoPos,true);
         return rtn;
      } // end GetClntSesNo


      void Message::setClntSesNo(uint4 sesNo)
      { Packet::replaceUInt4(sesNo,sesNoPos,true); }


      void Message::setMsgType(uint4 msgType)
      { Packet::replaceUInt4(msgType,msgTypePos,true); }


      uint4 Message::getMsgType()
      {
         uint4 rtn;
         msg->read(&rtn,sizeof(rtn),msgTypePos,true);
         return rtn;
      } // end GetMsgType


      bool Message::movePast(uint4 blkSize)
      {
         try { Packet::movePast(blkSize); } catch (ReadException &) { return false; }
         return true;
      } // movePast


      void Message::addStr(char const *s)
      {
         uint4 len = (uint4)strlen(s);
         addUInt4(len);
         Packet::addAscii(s,len);
      } // addStr


      void Message::addStr(StrAsc const &s)
      {
         addUInt4((uint4)s.length());
         addBlock(s.c_str(), (uint4)s.length());
      } // addStr


      void Message::addStr(std::string const &s)
      {
         addUInt4((uint4)s.length());
         addBlock(s.c_str(), (uint4)s.length());
      } // addStr


      void Message::addWStr(wchar_t const *s)
      {
         uint4 len = (uint4)wcslen(s);
         addUInt4(len);
         for(uint4 i = 0; i < len; ++i)
            addUInt2(s[i]);
      } // addWStr


      void Message::addWStr(StrUni const &s)
      {
         addUInt4((uint4)s.length());
         for(uint4 i = 0; i < (uint4)s.length(); ++i)
            addUInt2(s[i]);
      } // addWStr


      void Message::addWStr(std::wstring const &s)
      {
         addUInt4((uint4)s.length());
         for(uint4 i = 0; i < s.length(); ++i)
            addUInt2(s[i]);     
      } // addWStr


      bool Message::readBool(bool &val)
      {
         try { val = Packet::readByte() != 0; }
         catch(ReadException &) { return false; }
         return true;
      } // readBool


      bool Message::readByte(Byte &val)
      {
         try { val = Packet::readByte(); }
         catch(ReadException &) { return false; }
         return true;
      } // readByte


      bool Message::readUnicode(wchar_t &val)
      {
         try { val = Packet::readUInt2(true); }
         catch(ReadException &) { return false; }
         return true;
      } // readUnicode


      bool Message::readAscii(char &val)
      {
         try { Packet::readAscii(&val,sizeof(val)); }
         catch(ReadException &) { return false; }
         return true;
      } // readAscii


      bool Message::readInt2(int2 &val, bool as_lsf)
      {
         try { val = Packet::readInt2(!as_lsf); }
         catch(ReadException &) { return false; }
         return true;
      } // readInt2


      bool Message::readUInt2(uint2 &val, bool as_lsf)
      {
         try { val = Packet::readUInt2(!as_lsf); }
         catch(ReadException &) { return false; }
         return true;
      } // readUInt2


      bool Message::readInt4(int4 &val, bool as_lsf)
      {
         try { val = Packet::readInt4(!as_lsf); }
         catch(ReadException &) { return false; }
         return true;
      } // readInt4


      bool Message::readUInt4(uint4 &val, bool as_lsf)
      {
         try { val = Packet::readUInt4(!as_lsf); }
         catch(ReadException &) { return false; }
         return true;
      } // readUInt4
                           

      bool Message::readInt8(int8 &val, bool as_lsf)
      {
         try 
         { Packet::readBytes(&val,sizeof(val),!as_lsf); } 
         catch(ReadException &) { return false; }
         return true;
      } // readInt8


      bool Message::readFloat(float &val, bool as_lsf)
      {
         try { val = Packet::readIeee4(!as_lsf); }
         catch(ReadException &) { return false; }
         return true;
      } // readFloat


      bool Message::readDouble(double &val, bool as_lsf)
      {
         try { val = Packet::readIeee8(!as_lsf); }
         catch(ReadException &) { return false; }
         return true;
      } // readDouble


      bool Message::readStr(StrAsc &buff)
      {
         //@bugfix 12 April 1999 by Jon Trauntvein
         // Added code to check on the remaining bytes after the string length has been read. This will
         // prevent unneeded allocations.
         //@endbugfix
         bool rtn;
         try
         {
            // read the string length
            uint4 len = Packet::readUInt4(true);
            if(len <= whatsLeft())
            {
               buff.reserve(len); buff.cut(0);
               for(uint4 i = 0; i < len; i++)
               { char temp; readAscii(temp); buff.append(temp); }
               rtn = true;
            }
            else
               rtn = false;
         }
         catch(ReadException &)
         { rtn = false; }
         return rtn; 
      } // readStr


      bool Message::readStr(std::string &buff)
      {
         bool rtn;
         try
         {
            uint4 len = Packet::readUInt4(true);
            if(len <= whatsLeft())
            {
               buff = ""; buff.reserve(len);
               for(uint4 i = 0; i < len; i++)
               { char temp; readAscii(temp); buff += temp; }
               rtn = true;
            }
            else
               rtn = false;
         }
         catch(ReadException &)
         { rtn = false; }
         return rtn;
      } // readStr


      bool Message::readWStr(StrUni &buff)
      {
         //@bugfix 12 April 1999 by Jon Trauntvein
         // Added code to check on the remaining bytes after the string length has been read. This will
         // prevent unneeded allocations.
         //@endbugfix
         bool rtn;
         try
         {
            // read the string length
            uint4 len = Packet::readUInt4(true);
            if(len <= whatsLeft())
            {
               buff.reserve(len); buff.cut(0);
               for(uint4 i = 0; i < len; i++)
               { wchar_t temp; readUnicode(temp); buff.append(temp); }
               rtn = true;
            }
            else
               rtn = false;
         }
         catch(ReadException &)
         { rtn = false; }
         return rtn;
      } // readWStr


      bool Message::readWStr(std::wstring &buff)
      {
         bool rtn;
         try
         {
            // read the string length
            uint4 len = Packet::readUInt4(true);
            if(len <= whatsLeft())
            {
               buff = L""; buff.reserve(len); 
               for(uint4 i = 0; i < len; i++)
               { wchar_t temp; readUnicode(temp); buff += temp; }
               rtn = true;
            }
            else
               rtn = false;
         }
         catch(ReadException &)
         { rtn = false; }
         return rtn;
      } // readStr


      bool Message::readBStr(StrBin &buff, bool append)
      {
         //@bugfix 12 April 1999 by Jon Trauntvein
         // Added code to check on the remaining bytes after the string length has been read. This will
         // prevent unneeded allocations.
         //@endbugfix
         bool rtn;
         try
         {
            // read the buffer length
            uint4 len = Packet::readUInt4(true);
            if(len <= whatsLeft())
            {
               buff.reserve(len);
               if(!append)
                  buff.cut(0);
               buff.append(objAtReadIdx(),len);
               movePast(len);
               rtn = true;
            }
            else
               rtn = false;
         }
         catch(ReadException &)
         { rtn = false; }
         return rtn;
      } // readBStr


      bool Message::readBytes(void *buff, uint4 maxLen)
      {
         try
         {
            uint4 len = Packet::readUInt4(true);
            Packet::readBytes(buff,len);
         }
         catch(ReadException &)
         { return false; }
         return true;
      } // readBytes


      bool Message::readBlock(void *buff, UInt4 len)
      {
         bool rtn = true;
         try { Packet::readBytes(buff,len); }
         catch(ReadException &)
         { rtn = false; }
         return rtn;
      } // readBlock


      bool Message::readBlock(Stream &dest, uint4 len)
      {
         bool rtn = true;
         try { Packet::readBytes(dest,len); }
         catch(ReadException &)
         { rtn = false; }
         return rtn;
      } // readBlock
   };
};

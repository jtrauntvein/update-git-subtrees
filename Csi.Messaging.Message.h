/* Csi.Messaging.Message.h

   Copyright (C) 2000, 2010 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 21 June 2000
   Last Change: Thursday 28 October 2010
   Last Commit: $Date: 2010-10-28 15:49:00 -0600 (Thu, 28 Oct 2010) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Csi_Messaging_Message_h
#define Csi_Messaging_Message_h

#include "Csi.ByteQueue.h"
#include "Packet.h"
#include <string>


namespace Csi
{
   namespace Messaging
   {
      ////////////////////////////////////////////////////////////
      // class Message
      //
      // Represents a message that can be passed between objects of class Node. Each message has a
      // header that is used for routing and type identification. It also has a body that acts as a
      // stream would. Message provides methods that allow an application to read and write
      // primitive objects to and from this stream.
      //////////////////////////////////////////////////////////// 
      class Message: public Packet
      {
      public:
         ////////// Default constructor
         ////////////////////////////////////////////////////////////
         // default constructor
         //
         // Initialises the header with null values
         ////////////////////////////////////////////////////////////
         Message();
   
         ////////////////////////////////////////////////////////////
         // writing constructor
         //
         // Use this constructor to create a message from scratch.  It will write
         // the header values in their defined order.
         ////////////////////////////////////////////////////////////
         Message(uint4 sesNo, uint4 msgType);

         ////////////////////////////////////////////////////////////
         // copy constructor
         //
         // Makes a copy of the message object referred to.  If the deep_copy parameter is set to
         // false, the internal message reference will be copied but, if the deep_copy parameter is
         // true, a new buffer will be created for the message contents. 
         ////////////////////////////////////////////////////////////
         Message(Message const &other, bool deep_copy = false);

         ////////////////////////////////////////////////////////////
         // reading constructor (buffer)
         //
         // Use this constructor to initialise a message after it has been read from an
         // endpoint. The third parameter is used to specify whether the bytes used to construct the
         // message should be copied into an independent buffer or whether the buffer provided
         // should be used. In most instances, makeCopy should be set to true. If it is set to
         // false, the invoking code should take responsibility for verifying that the pointer and
         // its contents remain valid throughout the life of the message object.
         ////////////////////////////////////////////////////////////
         Message(void const *buff, uint4 buffLen, bool makeCopy);

         ////////////////////////////////////////////////////////////
         // reading constructor (queue)
         //
         // Initialises the message contents with a QueueOf<Byte> object.
         ////////////////////////////////////////////////////////////
         Message(ByteQueue &bq, uint4 len);
   
         ////////////////////////////////////////////////////////////
         // getMsg
         //
         // Returns a read-only pointer to the message bytes
         ////////////////////////////////////////////////////////////
         char const *getMsg() { return (char const *)Packet::getMsg(); }

         ////////////////////////////////////////////////////////////
         // getLen
         //
         // Returns the length of the message (including that of the header)
         ////////////////////////////////////////////////////////////
         uint4 getLen() { return length(); }

         ////////////////////////////////////////////////////////////
         // get_msgHeaderLen
         ////////////////////////////////////////////////////////////
         static uint4 get_msgHeaderLen()
         { return msgHeaderLen; }

         ////////////////////////////////////////////////////////////
         // getBody
         //
         // Returns a pointer to the beginning of the message body (excluding the
         // header)
         ////////////////////////////////////////////////////////////
         void const *getBody() 
         { return getMsg() + get_headerLen(); }

         ////////////////////////////////////////////////////////////
         // getBodyLen
         //
         // Returns the length of the message body (excluding the header length)
         ////////////////////////////////////////////////////////////
         uint4 getBodyLen()
         { return length() - get_headerLen(); }
   
         // @group Methods that write primitives to the message body
         ////////////////////////////////////////////////////////////
         // addBool
         ////////////////////////////////////////////////////////////
         void addBool(bool val)
         { Packet::addByte(val ? '\xFF' : '\x00'); }

         ////////////////////////////////////////////////////////////
         // addAscii
         ////////////////////////////////////////////////////////////
         void addAscii(char val)
         { Packet::addByte(val); }
   
         ////////////////////////////////////////////////////////////
         // addUnicode
         ////////////////////////////////////////////////////////////
         void addUnicode(wchar_t val)
         { Packet::addUInt2(val,true); }
   
         ////////////////////////////////////////////////////////////
         // addShort
         ////////////////////////////////////////////////////////////
         void addInt2(Int2 val)
         { Packet::addInt2(val,true); }

         ////////////////////////////////////////////////////////////
         // addUInt2
         ////////////////////////////////////////////////////////////
         void addUInt2(UInt2 val)
         { Packet::addUInt2(val,true); }
   
         ////////////////////////////////////////////////////////////
         // addInt
         ////////////////////////////////////////////////////////////
         void addInt4(Int4 val)
         { Packet::addInt4(val,true); }

         ////////////////////////////////////////////////////////////
         // addUInt4
         ////////////////////////////////////////////////////////////
         void addUInt4(UInt4 val)
         { Packet::addUInt4(val,true); }

         ////////////////////////////////////////////////////////////
         // addInt8
         ////////////////////////////////////////////////////////////
         void addInt8(int8 val)
         { Packet::addInt8(val,true); }

         ////////////////////////////////////////////////////////////
         // addFloat
         ////////////////////////////////////////////////////////////
         void addFloat(float val)
         { Packet::addIeee4(val,true); }
   
         ////////////////////////////////////////////////////////////
         // addDouble
         ////////////////////////////////////////////////////////////
         void addDouble(double val)
         { Packet::addIeee8(val,true); }
   
         ////////////////////////////////////////////////////////////
         // addStr
         //
         // appends the length of the nul terminated string <i>s</i> to the message
         // body in network order and then concatenates the string to the body
         ////////////////////////////////////////////////////////////
         void addStr(char const *s);
         void addStr(StrAsc const &s);
         void addStr(std::string const &s);
   
         ////////////////////////////////////////////////////////////
         // addWStr
         //
         // appends the length of the unicode string <i>s</i> to the message body and
         // then concatenates the contents of <i>s</i> to the message body.
         ////////////////////////////////////////////////////////////
         void addWStr(wchar_t const *s);
         void addWStr(StrUni const &s);
         void addWStr(std::wstring const &s);
   
         ////////////////////////////////////////////////////////////
         // addBytes
         //
         // Adds an array of bytes to the message stream. This array is preceded by
         // an integer that provides the length
         ////////////////////////////////////////////////////////////
         void addBytes(void const *b, uint4 len, bool swap_order = false)
         { addUInt4(len); Packet::addBytes(b,len,swap_order); }
   
         ////////////////////////////////////////////////////////////
         // addBlock
         //
         // Writes a block of bytes to the message stream that is not preceded by any
         // length.
         ////////////////////////////////////////////////////////////
         void addBlock(void const *b, uint4 len, bool swap_order = false)
         { Packet::addBytes(b,len,swap_order); }
         //@endgroup

         //@group Methods that replace the values of primitives in the message body
         // at a specified offset.
         ////////////////////////////////////////////////////////////
         // replaceBool
         ////////////////////////////////////////////////////////////
         void replaceBool(bool val, uint4 idx)
         { Packet::replaceByte((val ? '\xFF' : '\x00'),idx + get_headerLen()); }
   
         ////////////////////////////////////////////////////////////
         // replaceByte
         ////////////////////////////////////////////////////////////
         void replaceByte(byte val, uint4 idx)
         { Packet::replaceByte(val,idx + get_headerLen()); }
   
         ////////////////////////////////////////////////////////////
         // replaceChar
         ////////////////////////////////////////////////////////////
         void replaceUnicode(wchar_t val, uint4 idx)
         { Packet::replaceUInt2(val,idx + get_headerLen(),true); }
   
         ////////////////////////////////////////////////////////////
         // replaceInt2
         ////////////////////////////////////////////////////////////
         void replaceInt2(int2 val, uint4 idx)
         { Packet::replaceInt2(val,idx + get_headerLen(),true); }
   
         ////////////////////////////////////////////////////////////
         // replaceUInt2
         ////////////////////////////////////////////////////////////
         void replaceUInt2(uint2 val, uint4 idx)
         { Packet::replaceUInt2(val,idx + get_headerLen(),true); }

         ////////////////////////////////////////////////////////////
         // replaceInt4
         ////////////////////////////////////////////////////////////
         void replaceInt4(int4 val, uint4 idx)
         { Packet::replaceInt4(val,idx + get_headerLen(),true); }

         ////////////////////////////////////////////////////////////
         // replaceUInt4
         ////////////////////////////////////////////////////////////
         void replaceUInt4(uint4 val, uint4 idx)
         { Packet::replaceUInt4(val,idx + get_headerLen(),true); }
   
         ////////////////////////////////////////////////////////////
         // replaceInt8
         ////////////////////////////////////////////////////////////
         void replaceInt8(int8 val, uint4 idx)
         { Packet::replaceBytes(&val,sizeof(val),idx + get_headerLen(),true); }
   
         ////////////////////////////////////////////////////////////
         // replaceFloat
         ////////////////////////////////////////////////////////////
         void replaceFloat(float val, uint4 idx)
         { Packet::replaceIeee4(val,idx + get_headerLen(),true); }
   
         ////////////////////////////////////////////////////////////
         // replaceDouble
         ////////////////////////////////////////////////////////////
         void replaceDouble(double val, uint4 idx)
         { Packet::replaceIeee8(val,idx + get_headerLen(),true); }
         //@endgroup

         // @group primitive type extraction methods
         // Methods in this group are responsible for extracting objects of primitive
         // types from the message body. Typically, they will return a boolean value
         // indicating the outcome of the operation.
         ////////////////////////////////////////////////////////////
         // readBool
         ////////////////////////////////////////////////////////////
         bool readBool(bool &val);
   
         ////////////////////////////////////////////////////////////
         // readByte
         ////////////////////////////////////////////////////////////
         bool readByte(byte &val);
   
         ////////////////////////////////////////////////////////////
         // readUnicode
         ////////////////////////////////////////////////////////////
         bool readUnicode(wchar_t &val);

         ////////////////////////////////////////////////////////////
         // readAscii
         ////////////////////////////////////////////////////////////
         bool readAscii(char &val);
   
         ////////////////////////////////////////////////////////////
         // readInt2
         ////////////////////////////////////////////////////////////
         bool readInt2(int2 &val, bool as_lsf = false);

         ////////////////////////////////////////////////////////////
         // readUInt2
         ////////////////////////////////////////////////////////////
         bool readUInt2(uint2 &val, bool as_lsf = false);
   
         ////////////////////////////////////////////////////////////
         // readInt4
         ////////////////////////////////////////////////////////////
         bool readInt4(int4 &val, bool as_lsf = false);

         ////////////////////////////////////////////////////////////
         // readUInt4
         ////////////////////////////////////////////////////////////
         bool readUInt4(uint4 &val, bool as_lsf = false);
   
         ////////////////////////////////////////////////////////////
         // readInt8
         ////////////////////////////////////////////////////////////
         bool readInt8(int8 &val, bool as_lsf = false);
   
         ////////////////////////////////////////////////////////////
         // readFloat
         ////////////////////////////////////////////////////////////
         bool readFloat(float &val, bool as_lsf = false);
   
         ////////////////////////////////////////////////////////////
         // readDouble
         ////////////////////////////////////////////////////////////
         bool readDouble(double &val, bool as_lsf = false);
   
         ////////////////////////////////////////////////////////////
         // readStr (string)
         //
         // Reads an ASCII string from the message body (preceded by the string
         // length) and places it in the <i>buff</i> parameter. Returns false if
         // either the length or the string could not be read. 
         ////////////////////////////////////////////////////////////
         bool readStr(StrAsc &buff);
         bool readStr(std::string &buff);
   
         ////////////////////////////////////////////////////////////
         // readWStr (string)
         //
         // Reads a unicode string from the message body (preceded by the string
         // length) and places the it in the buff parameter. Returns false if the
         // length or the string cannot be read
         ////////////////////////////////////////////////////////////
         bool readWStr(StrUni &buff);
         bool readWStr(std::wstring &buff);
   
         ////////////////////////////////////////////////////////////
         // readBStr
         //
         // Reads a binary string from the message body (preceded by the string
         // length) and places it in the buff parameter. Returns false if either the
         // length or the string could not be read.
         ////////////////////////////////////////////////////////////
         bool readBStr(StrBin &buff, bool append = false);

         ////////////////////////////////////////////////////////////
         // readBytes
         //
         // Reads a binary string from the message body (preceded by the string
         // length) and places at most maxLen bytes of it in the buff
         // parameter. Returns false if either the length or the string could not be
         // read.
         ////////////////////////////////////////////////////////////
         bool readBytes(void *buff, uint4 maxLen);
   
         ////////////////////////////////////////////////////////////
         // readBlock
         //
         // Reads a binary string of length len from the message body and places it
         // in the buff parameter. Returns false if the string could not be read.
         ////////////////////////////////////////////////////////////
         bool readBlock(void *buff, uint4 len);

         ////////////////////////////////////////////////////////////
         // readBlock
         ////////////////////////////////////////////////////////////
         bool readBlock(Stream &dest, uint4 len);
         // @endgroup primitive type extraction methods
   
         //@group Miscellaneous access methods.
         ////////////////////////////////////////////////////////////
         // getClntSesNo
         //
         // Returns the client session number from the message header.
         ////////////////////////////////////////////////////////////
         uint4 getClntSesNo();
   
         ////////////////////////////////////////////////////////////
         // getMsgType
         //
         // Returns the message type code from the message header.
         ////////////////////////////////////////////////////////////
         uint4 getMsgType();
   
         ////////////////////////////////////////////////////////////
         // setClntSesNo
         //
         // Sets the client session number in the message header.
         ////////////////////////////////////////////////////////////
         void setClntSesNo(uint4 sesNo);

         ////////////////////////////////////////////////////////////
         // setMsgType
         //
         // Sets the message type field in the message header
         ////////////////////////////////////////////////////////////
         void setMsgType(uint4 msgType);

         ////////////////////////////////////////////////////////////
         // movePast
         //
         // Moves the read index past a block of bytes
         ////////////////////////////////////////////////////////////
         bool movePast(uint4 blkSize);
         //@endgroup

      private:
         ////////////////////////////////////////////////////////////
         // sesNoPos
         //
         // Records the position of the session number in the message header
         ////////////////////////////////////////////////////////////
         static const uint4 sesNoPos;

         ////////////////////////////////////////////////////////////
         // msgTypePos
         //
         // Records the position of the message type in the message header
         ////////////////////////////////////////////////////////////
         static const uint4 msgTypePos;

         ////////////////////////////////////////////////////////////
         // msgHeaderLen
         //
         // Records the total length of the message header
         ////////////////////////////////////////////////////////////
         static const uint4 msgHeaderLen;
      };
   };
};

#endif

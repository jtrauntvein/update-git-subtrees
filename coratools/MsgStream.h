/* MsgStream.h

   Copyright (C) 1998, 2010 Campbell Scientific, Inc.
   
   Written by: Jon Trauntvein
   Date Begun: Thursday 18 February 1999
   Last Change: Thursday 28 October 2010
   Last Commit: $Date: 2010-10-28 15:49:00 -0600 (Thu, 28 Oct 2010) $ 
   Committed by: $Author: jon $
   
*/

#ifndef MsgStream_h
#define MsgStream_h

#include "Stream.h"
#include "Csi.ByteQueue.h"


////////////////////////////////////////////////////////////
// class MsgStream 
//
// Concrete class that provides capabilities similar to those provided by class
// Packet. This class is more restricted in that it will not allow random
// access (reading and writing) like Packet objects do. This also differs from
// the packet class in that there is no concept of header supported at this
// level.
////////////////////////////////////////////////////////////
class MsgStream: public Stream
{
public:
   ////////////////////////////////////////////////////////////
   // constructor
   //
   // Creates a stream object using the specified initial capacity
   //////////////////////////////////////////////////////////// 
   MsgStream(uint4 initial_capacity);

   ////////////////////////////////////////////////////////////
   // copy constructor
   //
   // Initialises this stream so that its state reflects the state of the other
   // stream but does not share storage with the other stream.
   //////////////////////////////////////////////////////////// 
   MsgStream(MsgStream const &other);

   ////////////////////////////////////////////////////////////
   // copy operator
   //////////////////////////////////////////////////////////// 
   MsgStream &operator =(MsgStream const &other);

   ////////////////////////////////////////////////////////////
   // destructor
   //////////////////////////////////////////////////////////// 
   virtual ~MsgStream();

   //@group primitive peek operations 
   // Allow objects to be read from the head of the message stream without
   // affecting to the position of the head
   
   ////////////////////////////////////////////////////////////
   // peekByte
   //////////////////////////////////////////////////////////// 
   byte peekByte();

   ////////////////////////////////////////////////////////////
   // peekBytes
   //////////////////////////////////////////////////////////// 
   void peekBytes(void *dest, uint4 len, bool swapOrder = false);
   //@endgroup

   ////////////////////////////////////////////////////////////
   // eof
   //
   // Returns true if the message size is empty and the atEnd flag is set
   //////////////////////////////////////////////////////////// 
   bool eof() const
   { return (msg.size() == 0 && atEnd); }

   ////////////////////////////////////////////////////////////
   // whatsLeft
   //
   // Returns the number of bytes that are currently available for reading
   //////////////////////////////////////////////////////////// 
   virtual uint4 whatsLeft() const
   { return msg.size(); }

   ////////////////////////////////////////////////////////////
   // movePast
   //
   // Removes up to the number of bytes specified
   //////////////////////////////////////////////////////////// 
   void movePast(uint4 len)
   { msg.pop(len); }

   ////////////////////////////////////////////////////////////
   // setEnd
   //
   // Sets the atEnd flag
   //////////////////////////////////////////////////////////// 
   void setEnd()
   { atEnd = true; }

   ////////////////////////////////////////////////////////////
   // endSet
   //
   // Returns true if the end flag has been set. This is not synonymous with
   // the eof method since eof only returns true if the read pointer is at the
   // end as well
   //////////////////////////////////////////////////////////// 
   bool endSet() const
   { return atEnd; }

public:
   //@group overloaded Stream methods
   ////////////////////////////////////////////////////////////
   // length
   //////////////////////////////////////////////////////////// 
   virtual uint4 length() const
   { return msg.size(); }

protected:
   ////////////////////////////////////////////////////////////
   // addBytes
   //////////////////////////////////////////////////////////// 
   virtual void addBytes_impl(void const *buff, uint4 len, bool swapOrder);

   ////////////////////////////////////////////////////////////
   // readBytes
   //
   // reads exactly len bytes into the StrBin
   //////////////////////////////////////////////////////////// 
   virtual void readBytes_impl(void *buff, uint4 len, bool swapOrder);
   //@endgroup

private:
   ////////////////////////////////////////////////////////////
   // msg
   //
   // Provides the storage for the message
   //////////////////////////////////////////////////////////// 
   Csi::ByteQueue msg;

   ////////////////////////////////////////////////////////////
   // atEnd
   //
   // Indicates that the message is closed for writing
   //////////////////////////////////////////////////////////// 
   bool atEnd;
};

#endif
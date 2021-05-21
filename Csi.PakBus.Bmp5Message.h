/* Csi.PakBus.Bmp5Message.h

   Copyright (C) 2001, 2021 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 09 March 2001
   Last Change: Thursday 11 March 2021
   Last Commit: $Date: 2021-03-11 11:59:32 -0600 (Thu, 11 Mar 2021) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Csi_PakBus_Bmp5Message_h
#define Csi_PakBus_Bmp5Message_h

#include "Csi.PakBus.Message.h"
#include "Csi.PakBus.Bmp5.Defs.h"
#include "LgrDate.h"
#include "Csi.ByteOrder.h"


namespace Csi
{
   namespace PakBus
   {
      /**
       * Defines an object that represents a  message that can be sent or received in the BMP5
       * protocol. 
       */
      class Bmp5Message: public ::Csi::PakBus::Message
      {
      public:
         /**
          * Specifies the required length of the header for BMP5 message objects.  This will include
          * the message type and transaction number fields.
          */
         static uint4 const header_len_bytes;

         /**
          * Specifies the absolute maxumum number of bytes that can be transmitted using PakBus
          */
         static uint4 const max_body_len;
         
         /**
          * Constructor
          *
          * @param message_type Specifies the message type field.
          * @param other Specifies a message to copy.
          */
         typedef Bmp5Messages::code_type message_type_code;
         Bmp5Message();
         Bmp5Message(message_type_code message_type);
         Bmp5Message(Message &other);

         /**
          * @return Returns the message type.
          */
         message_type_code get_message_type();

         /**
          * @param message_type Specifies the message type code.
          */
         void set_message_type(message_type_code message_type);

         /**
          * @return Returns the transaction number.
          */
         byte get_transaction_no();

         /**
          * @param transaction_no Specifies the transaction number that will be encoded in the
          * header.
          */
         void set_transaction_no(byte transaction_no);

         /**
          * @return Reads a two byte unsigned integer in big endian formatfrom the current cursor.
          */
         uint2 readUInt2()
         { return Message::readUInt2(!is_big_endian()); }

         /**
          * @return Reads a two byte unsigned integer in little endian format from the current
          * cursor.
          */
         uint2 readUInt2Lsf()
         { return Message::readUInt2(is_big_endian()); }

         /**
          * @return Reads a two byte signed integer in big endian format from the current cursor.
          */
         int2 readInt2()
         { return Message::readInt2(!is_big_endian()); }

         /**
          * @return Reads a two byte signed integer in little endian format from the current cursor.
          */
         int2 readInt2Lsf()
         { return Message::readInt2(is_big_endian()); }

         /**
          * @return Reads a four byte unsigned integer in big endian format from the current cursor.
          */
         uint4 readUInt4()
         { return Message::readUInt4(!is_big_endian()); }

         /**
          * @return Reads a four byte unsigned integer in little endian format from the current
          * cursor.
          */
         uint4 readUInt4Lsf()
         { return Message::readUInt4(is_big_endian()); } 

         /**
          * @return Reads a four byte signed integer in big endian format from the current cursor.
          */
         int4 readInt4()
         { return Message::readInt4(!is_big_endian()); }

         /**
          * @return Reads a four byte signed integer in little endian format from the current
          * cursor.
          */
         int4 readInt4Lsf()
         { return Message::readInt4(is_big_endian()); }

         /**
          * @return Reads an eight byte signed integer in big endian format from the current cursor.
          */
         int8 readInt8()
         {
            int8 rtn;
            readBytes_impl(&rtn, sizeof(rtn), !is_big_endian());
            return rtn;
         }

         /**
          * @return Reads an eight byte signed integer in little endian format from the current
          * cursor.
          */
         int8 readInt8Lsf()
         {
            int8 rtn;
            readBytes_impl(&rtn, sizeof(rtn), is_big_endian());
            return rtn;
         }

         /**
          * @return Reads a four byte floating point number in big endian format from the current
          * position.
          */
         float readIeee4()
         { return Message::readIeee4(!is_big_endian()); }

         /**
          * @return Reads a four byte floating point number in little endian format from the current
          * position.
          */
         float readIeee4Lsf()
         { return Message::readIeee4(is_big_endian()); }

         /**
          * @return Reads an eight byte floating point number in big endian format from the current
          * position.
          */
         double readIeee8()
         { return Message::readIeee8(!is_big_endian()); }

         /**
          * @return Reads a time stamp from a four byte integer that represents seconds since 1990.
          */
         LgrDate readSec()
         { return Message::readSec(!is_big_endian()); }

         /**
          * @return Reads a time stamp with nsec resolution in big endian format from the current
          * position.
          */
         LgrDate readNSec()
         { return Message::readNSec(!is_big_endian()); }

         /**
          * @return Reads a time stamp with nsec resolution in little endian format from the current
          * cursor.
          */
         LgrDate readNSecLsf()
         { return Message::readNSec(is_big_endian()); }

         /**
          * @return Reads an FP3 format floating point number from the current cursor.
          */
         float readFp3();

         /**
          * Adds the specified value to the message.
          */
         void addByte(byte val)
         { addBytes_impl(&val,sizeof(val),!is_big_endian()); }

         /**
          * Adds the specified boolean value to the message as either 0xFF (true) or 0 (false).
          */
         void addBool(bool val)
         { addByte(val ? 0xFF : 0x00); }

         /**
          * Adds a two byte unsigned integer in big endian format.
          */
         void addUInt2(uint2 val)
         { addBytes_impl(&val,sizeof(val),!is_big_endian()); }

         /**
          * Adds a two byte unsigned integer in little endian format.
          */
         void addUInt2Lsf(uint2 val)
         { addBytes_impl(&val, sizeof(val), is_big_endian()); }

         /**
          * Adds a two byte signed integer in big endian format.
          */
         void addInt2(int2 val)
         { addBytes_impl(&val,sizeof(val),!is_big_endian()); }

         /**
          * Adds a two byte signed integer in little endian format.
          */
         void addInt2Lsf(int2 val)
         { addBytes_impl(&val, sizeof(val), is_big_endian()); }

         /**
          * Adds a four byte unsigned integer in big endian format.
          */
         void addUInt4(uint4 val)
         { addBytes_impl(&val,sizeof(val),!is_big_endian()); }

         /**
          * Adds a four byte unsigned integer in little endian format.
          */
         void addUInt4Lsf(uint4 val)
         { addBytes_impl(&val, sizeof(val), is_big_endian()); }

         /**
          * Adds a four byte signed integer in big endian format.
          */
         void addInt4(int4 val)
         { addBytes_impl(&val,sizeof(val),!is_big_endian()); }

         /**
          * Adds a four byte unsigned integer in little endian format.
          */
         void addInt4Lsf(int4 val)
         { addBytes_impl(&val, sizeof(val), is_big_endian()); }

         /**
          * Adds an eight byte signed integer in big endian format.
          */
         void addInt8(int8 val)
         { addBytes_impl(&val, sizeof(val), !is_big_endian()); }

         /**
          * Adds an eight byte signed integer in little endian format.
          */
         void addInt8Lsf(int8 val)
         { addBytes_impl(&val, sizeof(val), is_big_endian()); }

         /**
          * Adds a four byte floating point value in big endian format.
          */
         void addIeee4(float val)
         { addBytes_impl(&val,sizeof(val),!is_big_endian()); }

         /**
          * Adds a four byte floating point number in little endian format.
          */
         void addIeee4Lsf(float val)
         { addBytes_impl(&val, sizeof(val), is_big_endian()); }

         /**
          * Adds an eight byte floating point number in big endian format.
          */
         void addIeee8(double val)
         { addBytes_impl(&val,sizeof(val),!is_big_endian()); }

         /**
          * Adds an eight byte floating point number in big endian format.
          */
         void addIeee8Lsf(double val)
         { addBytes_impl(&val, sizeof(val), is_big_endian()); }

         /**
          * Adds a time stamp with nsec resolution in big endian format.
          */
         void addNSec(LgrDate const &val)
         { Message::addNSec(val,!is_big_endian()); }

         /**
          * Adds a time stamp with nsec resolution in little endian format.
          */
         void addNSecLsf(LgrDate const &val)
         { Message::addNSec(val,is_big_endian()); }

         /**
          * @return Overloaded to return true for all BMP5 messages.
          */
         virtual bool should_encrypt()
         { return true; }

      public:
         static uint4 const message_type_start;
         static uint4 const transaction_no_start;
      };
   };
};


#endif

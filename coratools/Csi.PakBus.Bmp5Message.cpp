/* Csi.PakBus.Bmp5Message.cpp

   Copyright (C) 2001, 2005 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 09 March 2001
   Last Change: Wednesday 19 January 2005
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)
   Committed by: $Author: tmecham $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.PakBus.Bmp5Message.h"
#include "CsiTypes.h"
#include "truediv.h"


namespace Csi
{
   namespace PakBus
   {
      ////////////////////////////////////////////////////////////
      // class Bmp5Message definitions
      ////////////////////////////////////////////////////////////
      uint4 const Bmp5Message::header_len_bytes = 2;
      uint4 const Bmp5Message::max_body_len = Message::max_body_len - header_len_bytes;
      uint4 const Bmp5Message::message_type_start = 0;
      uint4 const Bmp5Message::transaction_no_start = 1;


      Bmp5Message::Bmp5Message():
         Message(header_len_bytes)
      { set_high_protocol(ProtocolTypes::bmp); }


      Bmp5Message::Bmp5Message(message_type_code message_type):
         Message(header_len_bytes)
      {
         set_high_protocol(ProtocolTypes::bmp);
         set_message_type(message_type);
      }


      Bmp5Message::Bmp5Message(Message &other):
         Message(other,header_len_bytes)
      { }


      Bmp5Message::message_type_code Bmp5Message::get_message_type()
      {
         byte temp;
         msg->read(&temp,sizeof(temp),message_type_start,false);
         return static_cast<message_type_code>(temp);
      } // get_message


      void Bmp5Message::set_message_type(message_type_code message_type)
      { replaceByte(message_type,message_type_start); }


      byte Bmp5Message::get_transaction_no()
      {
         byte temp;
         msg->read(&temp,1,transaction_no_start,false);
         return temp;
      } // get_transaction_no


      void Bmp5Message::set_transaction_no(byte transaction_no)
      { replaceByte(transaction_no,transaction_no_start); }


      float Bmp5Message::readFp3()
      {
         char temp[3];
         readBytes(temp,sizeof(temp));
         return csiFs3ToFloat(temp);
      } // readFp3
   };
};


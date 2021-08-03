/* Csi.DevConfig.Message.cpp

   Copyright (C) 2003, Campbell Scientific, Inc

   Written by: Jon Trauntvein
   Date Begun: Thursday 20 November 2003
   Last Change: Tuesday 30 December 2003
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)
   Last Changed by: $Author: tmecham $

*/

#pragma warning(disable:4786) // disable overlong symbols warning
#pragma hdrstop               // stop creation of precompiled header
#include "Csi.DevConfig.Message.h"


namespace Csi
{
   namespace DevConfig
   {
      namespace
      {
         uint4 const marker_offset = 0;
         uint4 const message_type_offset = marker_offset + 1;
         uint4 const tran_no_offset = message_type_offset + 1;
      };

      
      ////////////////////////////////////////////////////////////
      // class Message definitions
      ////////////////////////////////////////////////////////////
      byte const Message::packet_type_byte = 0xF2;
      uint4 const Message::header_len = tran_no_offset + 1;

      
      Message::Message():
         Packet(header_len)
      {
         replaceByte(packet_type_byte,marker_offset);
      } // constructor

      
      Message::Message(
         void const *buff,
         uint4 buff_len,
         bool copy_buff):
         Packet(buff,buff_len,header_len,copy_buff)
      { }


      Message::message_id_type Message::get_message_type()
      {
         byte temp;
         msg->read(&temp,1,message_type_offset,false);
         return static_cast<message_id_type>(temp);
      } // get_message_type


      void Message::set_message_type(message_id_type message_type)
      { replaceByte(message_type,message_type_offset); }


      byte Message::get_tran_no()
      {
         byte rtn;
         msg->read(&rtn,1,tran_no_offset,false);
         return rtn;
      } // get_tran_no


      void Message::set_tran_no(byte tran_no)
      { replaceByte(tran_no,tran_no_offset); }
   };
};


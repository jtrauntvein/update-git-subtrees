/* Csi.PakBus.SerialPacket.cpp

   Copyright (C) 2001, 2006 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 06 March 2001
   Last Change: Tuesday 11 July 2006
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ 
   Committed by: $Author: tmecham $
   
*/


#pragma hdrstop               // stop creation of precompiled header
#include "Csi.PakBus.SerialPacket.h"
#include "Csi.PakBus.Message.h"
#include "Csi.ByteOrder.h"


namespace Csi
{
   namespace PakBus
   {
      ////////////////////////////////////////////////////////////
      // class SerialPacket definitions
      ////////////////////////////////////////////////////////////
      uint4 const SerialPacket::min_header_len = 4;
      uint4 const SerialPacket::max_header_len = 8;
      uint4 const SerialPacket::max_body_len = 1000;
      uint4 const SerialPacket::max_packet_len = max_body_len + max_header_len;
      uint4 const SerialPacket::link_state_start = 0;
      uint4 const SerialPacket::destination_physical_address_start = 0;
      uint4 const SerialPacket::expect_more_start = 2;
      uint4 const SerialPacket::ignore_expect_more_start = 2;
      uint4 const SerialPacket::priority_start = 2;
      uint4 const SerialPacket::source_physical_address_start = 2;
      uint4 const SerialPacket::high_proto_code_start = 4;
      uint4 const SerialPacket::destination_start = 4;
      uint4 const SerialPacket::hop_count_start = 6;
      uint4 const SerialPacket::source_start = 6;
      

      SerialPacket::SerialPacket(uint4 header_len):
         Packet(header_len)
      { }


      SerialPacket::SerialPacket(void const *buff, uint4 buff_len, bool make_copy):
         Packet(buff,buff_len,4,make_copy)
      {
         if(buff_len >= max_header_len)
            set_headerLen(max_header_len);
      } // constructor


      SerialPacket::SerialPacket(Message &pakbus_message):
         Packet(max_header_len)
      {
         set_high_proto_code(pakbus_message.get_high_protocol());
         set_destination(pakbus_message.get_destination());
         set_hop_count(pakbus_message.get_hop_count());
         set_source(pakbus_message.get_source());
         set_expect_more(pakbus_message.get_expect_more());
         set_priority(pakbus_message.get_priority());
         set_source_physical_address(pakbus_message.get_physical_source());
         set_destination_physical_address(pakbus_message.get_physical_destination());
         addBytes(pakbus_message.getMsg(),pakbus_message.length(),false);
      } // message constructor

      
      SerialPacket::link_state_type SerialPacket::get_link_state()
      {
         byte temp;
         msg->read(&temp,1,link_state_start,false);
         temp = ((temp & 0xf0) >> 4);
         return static_cast<link_state_type>(temp);
      } // get_link_state


      void SerialPacket::set_link_state(link_state_type link_state)
      {
         byte t1, t2 = link_state;
         msg->read(&t1,1,link_state_start,false);
         t1 &= ~0xf0;
         t2 = ((t2 & 0x0f) << 4);
         t1 |= t2;
         replaceByte(t1,link_state_start);
      } // set_link_state


      uint2 SerialPacket::get_destination_physical_address()
      {
         uint2 temp;
         msg->read(&temp,2,destination_physical_address_start,!is_big_endian());
         return (temp & 0x0fff);
      } // get_destination_physical_address


      void SerialPacket::set_destination_physical_address(uint2 destination_physical_address)
      {
         uint2 temp;
         msg->read(&temp,2,destination_physical_address_start,!is_big_endian());
         temp &= 0xf000;
         temp |= (destination_physical_address & 0x0fff);
         replaceUInt2(temp,destination_physical_address_start,!is_big_endian());
      } // set_destination_physical_address


      SerialPacket::expect_more_type SerialPacket::get_expect_more()
      {
         byte temp;
         msg->read(&temp,1,expect_more_start,false);
         temp = ((temp & 0xC0) >> 6);
         return static_cast<expect_more_type>(temp);
      } // get_expect_more


      void SerialPacket::set_expect_more(expect_more_type expect_more)
      {
         byte t1, t2 = expect_more;
         msg->read(&t1,1,expect_more_start,false);
         t1 &= ~0xC0;
         t2 = ((t2 & 0x03) << 6);
         t1 |= t2;
         replaceByte(t1,expect_more_start);
      } // set_expect_more


      SerialPacket::priority_type SerialPacket::get_priority()
      {
         byte temp;
         msg->read(&temp,1,priority_start,false);
         temp = ((temp & 0x30) >> 4);
         return static_cast<priority_type>(temp);
      } // get_priority


      void SerialPacket::set_priority(priority_type priority)
      {
         byte t1, t2 = priority;
         msg->read(&t1,1,priority_start,false);
         t1 &= ~0x30;
         t2 = ((t2 & 0x03) << 4);
         t1 |= t2;
         replaceByte(t1,priority_start);
      } // set_priority


      uint2 SerialPacket::get_source_physical_address()
      {
         uint2 temp;
         msg->read(&temp,2,source_physical_address_start,!is_big_endian());
         return (temp & 0x0fff);
      } // get_source_physical_address


      void SerialPacket::set_source_physical_address(uint2 source_physical_address)
      {
         uint2 temp;
         msg->read(&temp,2,source_physical_address_start,!is_big_endian());
         temp &= 0xf000;
         temp |= (source_physical_address & 0x0fff);
         replaceUInt2(temp,source_physical_address_start,!is_big_endian());
      } // set_source_physical_address


      SerialPacket::high_proto_code_type SerialPacket::get_high_proto_code()
      {
         byte temp;
         msg->read(&temp,1,high_proto_code_start,false);
         temp = ((temp & 0xF0) >> 4);
         return static_cast<high_proto_code_type>(temp);
      } // get_high_proto_code


      void SerialPacket::set_high_proto_code(high_proto_code_type high_proto_code)
      {
         byte t1, t2 = high_proto_code;
         msg->read(&t1,1,high_proto_code_start,false);
         t1 &= ~0xF0;
         t1 |= (t2 << 4);
         replaceByte(t1,high_proto_code_start);
      } // set_high_proto_code


      uint2 SerialPacket::get_destination()
      {
         uint2 temp;
         msg->read(&temp,sizeof(temp),destination_start,!is_big_endian());
         return (temp & 0x0fff);
      } // get_destination


      void SerialPacket::set_destination(uint2 destination)
      {
         uint2 temp;
         msg->read(&temp,sizeof(temp),destination_start,!is_big_endian());
         temp &= 0xf000;
         temp |= (destination & 0x0fff);
         replaceUInt2(temp,destination_start,!is_big_endian());
      } // set_destination


      byte SerialPacket::get_hop_count()
      {
         byte temp;
         msg->read(&temp,1,hop_count_start,false);
         temp = ((temp & 0xf0) >> 4);
         return temp;
      } // get_hop_count


      void SerialPacket::set_hop_count(byte hop_count)
      {
         byte t1, t2 = hop_count;
         msg->read(&t1,1,hop_count_start,false);
         t1 &= ~0xf0;
         t1 |= (t2 << 4);
         replaceByte(t1,hop_count_start);
      } // set_hop_count


      uint2 SerialPacket::get_source()
      {
         uint2 temp;
         msg->read(&temp,sizeof(temp),source_start,!is_big_endian());
         return (temp & 0x0fff);
      } // get_source


      void SerialPacket::set_source(uint2 source)
      {
         uint2 temp;
         msg->read(&temp,sizeof(temp),source_start,!is_big_endian());
         temp &= 0xf000;
         temp |= (source & 0x0fff);
         replaceUInt2(temp,source_start,!is_big_endian());
      } // set_source


      SerialPacket::capability_type SerialPacket::get_capability()
      {
         capability_type rtn = cap_linkstate;
         if(is_control())
         {
            byte temp;
            msg->read(&temp,1,expect_more_start,false);
            temp >>= 4;
            switch(temp)
            {
            case 1:
               rtn = cap_unquoted;
               break;
               
            case 2:
               rtn = cap_linkstate_unquoted_retrying;
               break;
            }
         }
         return rtn;
      } // get_capability


      void SerialPacket::set_capability(capability_type capability)
      {
         byte temp;
         byte c2 = capability;
         
         msg->read(&temp,1,expect_more_start,false);
         temp &= 0x0f;
         c2 <<= 4;
         temp |= c2;
         replaceByte(temp,expect_more_start);
         set_link_state(control_capabilities);
      } // set_capability
      
      
      Csi::SharedPtr<Message> SerialPacket::make_pakbus_message()
      {
         Csi::SharedPtr<Message> rtn(
            new Message(objAtReadIdx(),whatsLeft()));
         rtn->set_priority(get_priority());
         rtn->set_expect_more(get_expect_more());
         rtn->set_physical_destination(get_destination_physical_address());
         rtn->set_physical_source(get_source_physical_address());
         rtn->set_destination(get_destination());
         rtn->set_source(get_source());
         rtn->set_high_protocol(get_high_proto_code());
         rtn->set_hop_count(get_hop_count());
         return rtn;
      } // make_pakbus_message
   };
};

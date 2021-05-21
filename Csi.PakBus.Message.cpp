/* Csi.PakBus.Message.cpp

   Copyright (C) 2001, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 28 February 2001
   Last Change: Thursday 18 October 2012
   Last Commit: $Date: 2012-10-18 09:11:36 -0600 (Thu, 18 Oct 2012) $ 
   Committed by: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.PakBus.Message.h"
#include "Csi.PakBus.Bmp5Message.h"
#include "Csi.ByteOrder.h"
#include "Csi.Utils.h"
#include <iomanip>


namespace Csi
{
   namespace PakBus
   {
      ////////////////////////////////////////////////////////////
      // class Message definitions
      ////////////////////////////////////////////////////////////
      uint4 const Message::max_body_len = 1000;

      
      Message::Message(uint4 header_len):
         Packet(header_len),
         expect_more(ExpectMoreCodes::neutral),
         priority(Priorities::low),
         physical_destination(0),
         physical_source(0),
         source(0),
         destination(0),
         hop_count(0),
         high_protocol(ProtocolTypes::control),
         use_own_route(false),
         will_close(false),
         encrypted(false)
      { reset_age(); }


      Message::Message(
         void const *buff,
         uint4 buff_len,
         uint4 header_len,
         bool make_copy):
         Packet(buff,buff_len,header_len,make_copy),
         expect_more(ExpectMoreCodes::neutral),
         priority(Priorities::low),
         physical_destination(0),
         physical_source(0),
         source(0),
         destination(0),
         hop_count(0),
         high_protocol(ProtocolTypes::control),
         use_own_route(false),
         will_close(false),
         encrypted(false)
      { reset_age(); }


      Message::Message(Message &other, uint4 header_len, bool deep_copy):
         Packet(other, header_len, deep_copy),
         priority(other.priority),
         port(other.port),
         physical_destination(other.physical_destination),
         physical_source(other.physical_source),
         expect_more(other.expect_more),
         high_protocol(other.high_protocol),
         source(other.source),
         destination(other.destination),
         hop_count(other.hop_count),
         use_own_route(false),
         will_close(false),
         encrypted(other.encrypted)
      { reset_age(); }


      uint4 Message::get_age_msec() const
      { return counter(age_base); }


      void Message::reset_age()
      { age_base = counter(0); }


      bool Message::describe_message(std::ostream &out)
      {
         static char const *protocol_type_strings[] =
         {
            "PakCtrl",
            "BMP5",
            "datagram",
            "encrypted",
            "unknown"
         };
         int protocol_type = get_high_protocol();
         bool rtn = false;
         
         if(protocol_type > 3)
            protocol_type = 3;
         out << "src: " << get_source() << "\",\""
             << "dest: " << get_destination() << "\",\""
             << "proto: " << protocol_type_strings[protocol_type];
         if(protocol_type == ProtocolTypes::control || protocol_type == ProtocolTypes::bmp)
         {
            // we know something about these types so we will look for a header in the message
            if(length() >= 2)
            {
               // we will peek into the message to get its transaction number and message type
               byte temp;
               int tran_no;
               int message_type;
               
               msg->read(&temp, 1, Bmp5Message::transaction_no_start, false);
               tran_no = temp;
               msg->read(&temp, 1, Bmp5Message::message_type_start, false);
               message_type = temp;
               out << "\",\"type: 0x" << std::hex << std::setw(2) << std::setfill('0') << message_type
                   << "\",\"tran: " << std::dec << tran_no;
               if(protocol_type == ProtocolTypes::bmp && message_type == Bmp5Messages::please_wait_notification)
                  rtn = true;
            }
            else
               out << "\",\"empty";
         }
         return rtn;
      } // describe_message
   };
};

      

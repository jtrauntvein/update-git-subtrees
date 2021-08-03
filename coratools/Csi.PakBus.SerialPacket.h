/* Csi.PakBus.SerialPacket.h

   Copyright (C) 2001, 2006 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 06 March 2001
   Last Change: Tuesday 11 July 2006
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ 
   Committed by: $Author: tmecham $
   
*/


#ifndef Csi_PakBus_SerialPacket_h
#define Csi_PakBus_SerialPacket_h


#include "Packet.h"
#include "Csi.PakBus.Defs.h" 


namespace Csi
{
   namespace PakBus
   {
      //@group class forward declarations
      class Message;
      //@endgroup

      
      ////////////////////////////////////////////////////////////
      // class SerialPacket
      //
      // Defines the structure of the SerPkt type message that is used to
      // transfer PakBus messages from point to point.
      ////////////////////////////////////////////////////////////
      class SerialPacket: public ::Packet
      {
      public:
         ////////////////////////////////////////////////////////////
         // min_header_len
         //
         // Describes the minumum header length.  This is the four byte header
         // that contains only low-level serial information.
         ////////////////////////////////////////////////////////////
         static uint4 const min_header_len;

         ////////////////////////////////////////////////////////////
         // max_header_len
         //
         // Describes the maximum size the header can be.  This is the full
         // header that also contains the PakBus message information.
         ////////////////////////////////////////////////////////////
         static uint4 const max_header_len;

         ////////////////////////////////////////////////////////////
         // max_body_len
         //
         // The maximum size that the serial packet body (the part of the
         // packet the excludes the header) is allowed to grow.
         ////////////////////////////////////////////////////////////
         static uint4 const max_body_len;
         
         ////////////////////////////////////////////////////////////
         // max_packet_len
         //
         // The maximum allowed packet size for a serial packet. This includes
         // the size of the header and the serial packet body.
         ////////////////////////////////////////////////////////////
         static uint4 const max_packet_len;

         ////////////////////////////////////////////////////////////
         // default constructor
         ////////////////////////////////////////////////////////////
         SerialPacket(uint4 header_len = 4);

         ////////////////////////////////////////////////////////////
         // raw block constructor
         ////////////////////////////////////////////////////////////
         SerialPacket(void const *buff, uint4 buff_len, bool make_copy = true);

         ////////////////////////////////////////////////////////////
         // construct from PakBus message
         ////////////////////////////////////////////////////////////
         SerialPacket(Message &pakbus_message);
         
         //@group header field access methods
         ////////////////////////////////////////////////////////////
         // link_state
         //
         // Describes the current state of the link to/of the peer depending on
         // whether the message is outgoing or incoming.
         ////////////////////////////////////////////////////////////
         enum link_state_type
         {
            link_state_off_line = 8,
            link_state_ring = 9,
            link_state_ready = 10,
            link_state_finished = 11,
            link_state_pause = 12,
            control_reserved = 13,
            control_capabilities = 14
         };
         link_state_type get_link_state();
         void set_link_state(link_state_type link_state);

         ////////////////////////////////////////////////////////////
         // destination_physical_address access methods
         ////////////////////////////////////////////////////////////
         uint2 get_destination_physical_address();
         void set_destination_physical_address(uint2 destination_physical_address);

         ////////////////////////////////////////////////////////////
         // source_physical_address access methods
         ////////////////////////////////////////////////////////////
         uint2 get_source_physical_address();
         void set_source_physical_address(uint2 source_physical_address);

         ////////////////////////////////////////////////////////////
         // expect_more access methods
         ////////////////////////////////////////////////////////////
         typedef ExpectMoreCodes::expect_more_code_type expect_more_type;
         expect_more_type get_expect_more();
         void set_expect_more(expect_more_type expect_more);

         ////////////////////////////////////////////////////////////
         // priority access methods
         ////////////////////////////////////////////////////////////
         typedef Priorities::priority_type priority_type;
         priority_type get_priority();
         void set_priority(priority_type priority);

         ////////////////////////////////////////////////////////////
         // high_proto_code access methods
         ////////////////////////////////////////////////////////////
         typedef ProtocolTypes::protocol_type high_proto_code_type;
         high_proto_code_type get_high_proto_code();
         void set_high_proto_code(high_proto_code_type high_proto_code);

         ////////////////////////////////////////////////////////////
         // destination access methods
         ////////////////////////////////////////////////////////////
         uint2 get_destination();
         void set_destination(uint2 destination);

         ////////////////////////////////////////////////////////////
         // source access methods
         ////////////////////////////////////////////////////////////
         uint2 get_source();
         void set_source(uint2 source);

         ////////////////////////////////////////////////////////////
         // hop_count access methods
         ////////////////////////////////////////////////////////////
         byte get_hop_count();
         void set_hop_count(byte hop_count);

         ////////////////////////////////////////////////////////////
         // is_control
         //
         // Returns true if this packet is in the control group
         ////////////////////////////////////////////////////////////
         bool is_control()
         {
            link_state_type link_state = get_link_state();
            bool rtn = false;
            if(link_state == link_state_ring ||
               link_state == control_reserved ||
               link_state == control_capabilities)
               rtn = true;
            return rtn;
         }

         ////////////////////////////////////////////////////////////
         // get_capability
         //
         // Returns the capability of the device (applicable only for control
         // packets).
         ////////////////////////////////////////////////////////////
         enum capability_type
         {
            cap_linkstate = 0,
            cap_unquoted = 1,
            cap_linkstate_unquoted_retrying = 2
         };
         capability_type get_capability();

         ////////////////////////////////////////////////////////////
         // set_capability
         //
         // Sets this packet as a control packet and sets the capability field
         // to the specified value.
         ////////////////////////////////////////////////////////////
         void set_capability(capability_type capability);
         //@endgroup

         ////////////////////////////////////////////////////////////
         // make_pakbus_message
         //
         // Constructs a PakBus message from the content of this serial packet.
         ////////////////////////////////////////////////////////////
         Csi::SharedPtr<Message> make_pakbus_message();

      private:
         ////////////////////////////////////////////////////////////
         // header field offsets
         ////////////////////////////////////////////////////////////
         static uint4 const link_state_start;
         static uint4 const destination_physical_address_start;
         static uint4 const ignore_expect_more_start;
         static uint4 const expect_more_start;
         static uint4 const priority_start;
         static uint4 const source_physical_address_start;
         static uint4 const high_proto_code_start;
         static uint4 const destination_start;
         static uint4 const hop_count_start;
         static uint4 const source_start;
      };
   };
};


#endif

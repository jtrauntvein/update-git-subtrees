/* Csi.PakBus.Defs.h

   Copyright (C) 2001, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 02 March 2001
   Last Change: Friday 26 October 2012
   Last Commit: $Date: 2012-10-26 12:57:41 -0600 (Fri, 26 Oct 2012) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Csi_PakBus_Defs_h
#define Csi_PakBus_Defs_h


namespace Csi
{
   namespace PakBus
   {
      namespace ProtocolTypes
      {
         enum protocol_type
         {
            control = 0,
            bmp = 1,
            datagram = 2,
            encrypted = 3
         };
      };


      namespace Priorities
      {
         enum priority_type
         {
            low = 0,
            normal = 1,
            high = 2,
            extra_high = 3,
         };
      };


      namespace ExpectMoreCodes
      {
         enum expect_more_code_type
         {
            last = 0,
            expect_more = 1,
            neutral = 2,
            expect_more_opposite = 3,
         };
      };


      ////////////////////////////////////////////////////////////
      // namespace PakCtrl
      //
      // Definitions associated with the control layer of the pakbus protocol.
      ////////////////////////////////////////////////////////////
      namespace PakCtrl
      {
         namespace Messages
         {
            enum message_type
            {
               delivery_failure = 0x81,
               clock_status = 0x02,
               route_information = 0x03,
               leaf_information = 0x83,
               device_type_query_cmd = 0x04,
               device_type_query_ack = 0x84,
               echo_cmd = 0x05,
               echo_ack = 0x85,
               list_neighbours_cmd = 0x06,
               list_neighbours_ack = 0x86,
               get_settings_cmd = 0x07,
               get_settings_ack = 0x87,
               set_settings_cmd = 0x08,
               set_settings_ack = 0x88,
               hello_cmd = 0x09,
               hello_ack = 0x89,
               send_neighbour_list_cmd = 0x0a,
               send_neighbour_list_ack = 0x8a,
               get_neighbour_list_cmd = 0x0b,
               get_neighbour_list_ack = 0x8b,
               reset_router_cmd = 0x0c,
               goodbye_cmd = 0x0d,
               hello_request_cmd = 0x0e,
               devconfig_get_settings_cmd = 0x0f,
               devconfig_get_settings_ack = 0x8f,
               devconfig_set_settings_cmd = 0x10,
               devconfig_set_settings_ack = 0x90,
               devconfig_get_setting_fragment_cmd = 0x11,
               devconfig_get_setting_fragment_ack = 0x91,
               devconfig_set_setting_fragment_cmd = 0x12,
               devconfig_set_setting_fragment_ack = 0x92,
               devconfig_control_cmd = 0x13,
               devconfig_control_ack = 0x93,
               remote_echo_cmd = 0x14,
               remote_echo_ack = 0x94,
               list_port_names_cmd = 0x15,
               list_port_names_ack = 0x95,
               remote_hello_request_cmd = 0x16,
               remote_hello_request_ack = 0x96
            };
         };

         
         namespace DeliveryFailure
         {
            enum failure_type
            {
               unknown_reason = 0,
               unreachable_destination = 1,
               unreachable_high_level_protocol = 2,
               timed_out_or_resource_error = 3,
               unsupported_message_type = 4,
               malformed_message = 5,
               failed_static_route = 6,
               packet_too_big_64 = 7,
               packet_too_big_90 = 8,
               packet_too_big_128 = 9,
               packet_too_big_256 = 10,
               packet_too_big_512 = 11,
               unsupported_encryption_cipher = 12,
               encryption_required = 13,
               max_delivery_failure
            };
         };
      };
   };
};


#endif

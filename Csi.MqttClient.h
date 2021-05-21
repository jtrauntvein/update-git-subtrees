/* Csi.MqttClient.h

   Copyright (C) 2020, Campbell Scientific, Inc

   Written by: Jon Trauntvein 
   Date Begun: Friday 04 September 2020
   Last Change: Friday 04 September 2020
   Last Commit: $Date: 2020-09-10 09:10:17 -0600 (Thu, 10 Sep 2020) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_MqttClient_h
#define Csi_MqttClient_h

#include "CsiTypeDefs.h"
#include "StrAsc.h"
#include "StrBin.h"
#include "Packet.h"
#include "Csi.SocketConnection.h"


namespace Csi
{
   namespace Mqtt
   {
      /**
       * Defines the codes for control packets.
       */
      enum control_packet_type
      {
         control_reserved = 0,
         control_connect = 1,
         control_connect_ack = 2,
         control_publish = 3,
         control_publish_ack = 4,
         control_publish_received = 5,
         control_publish_release = 6,
         control_publish_complete = 7,
         control_subscribe = 8,
         control_subscribe_ack = 9,
         control_ping_request = 12,
         control_ping_response = 13,
         control_disconnect = 14,
         control_auth = 15
      };

      /**
       * Specifies the identifiers for properties that can appear in a variable length header.
       */
      enum var_header_property_id_type
      {
         property_payload_format = 1,
         property_message_expire_interval = 2,
         property_content_type = 3,
         property_response_topic = 8,
         property_correlation_data = 9,
         property_subscription_id = 11,
         property_session_expire_interval = 17,
         property_assigned_client_id = 18,
         property_server_keep_alive = 19,
         property_authentication_method = 21,
         property_authentication_data = 22,
         property_request_problem_info = 23,
         property_will_delay_interval = 24,
         property_request_response_info = 25,
         property_response_info = 26,
         property_server_reference = 28,
         property_reason_string = 31,
         property_receive_max = 33,
         property_topic_alias_max = 34,
         property_topic_alias = 35,
         property_max_qos = 36,
         property_retain_available = 37,
         property_user = 38,
         property_max_packet_size = 39,
         property_wildcard_subscription_available = 40,
         property_subscription_id_available = 41,
         property_shared_subscription_available = 42
      };

      /**
       * Specifies reason codes that can be reported by various server sent packet types.
       */
      enum reason_code_type
      {
         reason_success = 0,
         reason_granted_qos1 = 1,
         reason_granted_qos2 = 2,
         reason_disconnect_with_will = 4,
         reason_no_matching_subscribers = 16,
         reason_no_subscription_existed = 17,
         reason_continue_authenticate = 24,
         reason_reauthenticate = 25,
         reason_unspecified_error = 128,
         reason_malformed_packet = 129,
         reason_implementation_error = 131,
         reason_unsupported_protocol = 132,
         reason_invalid_client_id = 133,
         reason_bad_username_or_password = 134,
         reason_not_authorised = 135,
         reason_server_unavailable = 136,
         reason_server_busy = 137,
         reason_banned = 138,
         reason_server_shutting_down = 139,
         reason_baud_authentication_method = 140,
         reason_keep_alive_timeout = 141,
         reason_session_taken_over = 142,
         reason_topic_filter_invalid = 143,
         reason_topic_name_invalid = 144,
         reason_packet_id_in_user = 145,
         reason_packet_id_not_found = 146,
         reason_receive_max_exceeded = 147,
         reason_topic_alias_invalid = 148,
         reason_packet_too_large = 149,
         reason_message_rate_too_high = 150,
         reason_quota_exceeded = 151,
         reason_admin_action = 152,
         reason_invalid_payload_format = 153,
         reason_retain_not_supported = 154,
         reason_qos_not_supported = 155,
         reason_use_other_server = 156,
         reason_server_moved = 157,
         reason_shared_subscriptions_not_supported = 161,
         reason_connection_rate_exceeded = 159,
         reason_max_connect_time = 160,
         reason_subscription_ids_not_supported = 161,
         reason_wildcard_subscriptions_not_supported = 162
      };

      /**
       * Defines an object that can be used to encode and decode an MQTT packet.
       */
      class Message: ::Packet
      {
      public:
         /**
          * Default Constructor
          */
         Message();

      };
      
   };
};


#endif

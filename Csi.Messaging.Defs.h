/* Csi.Messaging.Defs.h

   Copyright (C) 2000, Campbell Scientific, Inc

   Written by: Jon Trauntvein
   Date Begun: Thursday 27 February 1997
   Last Change: Friday 08 February 2002
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)

*/

#ifndef Csi_Messaging_Defs_h
#define Csi_Messaging_Defs_h

namespace Csi
{
   namespace Messaging
   {
      ////////// namespace Messages
      // Contains enumerations of messages defined by the base messaging protocol
      namespace Messages
      {
         ////////// enum message_identifier_type
         // Lists the type identifiers for messages used in the base protocol
         enum message_identifier_type
         {
            type_heart_beat = 1,
            type_session_close_cmd = 2,
            type_session_closed_not = 3,
            type_message_rejected_not = 4,
            type_query_server_cmd = 5,
            type_query_server_ack = 6,
         };

         ////////// enum session_closed_reason_type
         // Lists the response codes that can accompany a session closed notification
         enum session_closed_reason_type
         {
            session_closed_no_object = 1,
            session_closed_no_resources = 2,
            session_closed_shut_down = 3,
         };

         ////////// enum message_rejected_reason_type
         // Lists the response codes that can accompany a message rejected notification
         enum message_rejected_reason_type
         {
            message_rejected_unsupported = 1,
            message_rejected_malformed = 2,
            message_rejected_orphaned_session = 3,
            message_rejected_security = 4,
         };
      };

      ////////// namespace SessionBrokenReasons
      // Describes the reason codes that can accompany session broker notifications
      namespace SessionBrokenReasons
      {
         enum session_broken_reason_type
         {
            no_object = Messages::session_closed_no_object,
            no_resources = Messages::session_closed_no_resources,
            shut_down = Messages::session_closed_shut_down,
            connection_failed = 4,
            heart_beat_failed = 5,
         };
      };
   };
};

#endif

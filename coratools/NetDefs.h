/* NetDefs.h

   Copyright (C) 2000, Campbell Scientific, Inc

   Written by: Jon Trauntvein
   Date Begun: Thursday 27 February 1997
   Last Change: Wednesday 21 June 2000
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)

*/

#ifndef NetDefs_h
#define NetDefs_h

#include "Csi.Messaging.Defs.h"

#define Net_HeartBeat          (Csi::Messaging::Messages::type_heart_beat)
#define Net_SesCloseCmd        (Csi::Messaging::Messages::type_session_close_cmd)
#define Net_SesClosedNot       (Csi::Messaging::Messages::type_session_closed_not)
#define Net_RejectedMsg        (Csi::Messaging::Messages::type_message_rejected_not)
#define Net_QuerySvrCmd        (Csi::Messaging::Messages::type_query_server_cmd)
#define Net_QuerySvrAck        (Csi::Messaging::Messages::type_query_server_ack)

typedef Csi::Messaging::Messages::session_closed_reason_type NetSC_Codes;
typedef Csi::Messaging::Messages::message_rejected_reason_type NetR_Codes;
#define NetR_SecurityBlk (Csi::Messaging::Messages::message_rejected_security)
#define NetR_OrphanedSes (Csi::Messaging::Messages::message_rejected_orphaned_session)
#define NetR_Malformed   (Csi::Messaging::Messages::message_rejected_malformed)
#define NetR_Unsupported (Csi::Messaging::Messages::message_rejected_unsupported)
#define NetSC_ShutDown   (Csi::Messaging::Messages::session_closed_shut_down)
#define NetSC_NoObject   (Csi::Messaging::Messages::session_closed_no_object)

#endif

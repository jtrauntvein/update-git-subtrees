/* Csi.Messaging.Proxy.Defs.h

   Copyright (C) 2014, 2014 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Wednesday 06 August 2014
   Last Change: Monday 18 August 2014
   Last Commit: $Date: 2014-08-18 12:36:04 -0600 (Mon, 18 Aug 2014) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_Messaging_Proxy_Defs_h
#define Csi_Messaging_Proxy_Defs_h

#include "Csi.Messaging.Defs.h"


namespace Csi
{
   namespace Messaging
   {
      namespace Proxy
      {
         /**
          * Defines the various message identifiers used in the cora proxy interface.
          */
         namespace Messages
         {
            enum message_identifiers
            {
               server_logon_cmd = 2000,
               server_logon_challenge = 2001,
               server_logon_response = 2002,
               server_logon_ack = 2003,

               client_logon_cmd = 2004,
               client_logon_challenge = 2005,
               client_logon_response = 2006,
               client_logon_ack = 2007,

               virtual_conn_not = 2008,
               virtual_conn_close_cmd = 2009,
               virtual_conn_forward_cmd = 2010,

               server_register_cmd = 2011,
               server_register_ack = 2012
            };
         };


         /**
          * Defines the possible outcome codes used in the server logon transaction
          */
         enum server_logon_outcome_type
         {
            server_logon_outcome_success = 1,
            server_logon_outcome_invalid_proxy = 2,
            server_logon_outcome_proxy_already_registered = 3,
            server_logon_outcome_challenge_time_out = 4,
            server_logon_outcome_invalid_response = 5
         };


         /**
          * Defines the possible outcome codes used by the client logon transaction.
          */
         enum client_logon_outcome_type
         {
            client_logon_outcome_success = 1,
            client_logon_outcome_invalid_proxy = 2,
            client_logon_outcome_proxy_inactive = 3,
            client_logon_outcome_challenge_time_out = 4,
            client_logon_outcome_invalid_response = 5
         };
      };
   };
};


#endif

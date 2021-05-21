/* Cora.PbRouter.Defs.h

   Copyright (C) 2002, 2005 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 05 June 2002
   Last Change: Friday 25 March 2005
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)
   Last Changed by: $Author: tmecham $

*/


#ifndef Cora_PbRouter_Defs_h
#define Cora_PbRouter_Defs_h


namespace Cora
{
   namespace PbRouter
   {
      namespace Messages
      {
         enum Type
         {
            list_nodes_cmd = 501,
            list_nodes_ack = 502,

            get_settings_cmd = 503,
            get_settings_ack = 504,

            set_settings_cmd = 505,
            set_settings_ack = 506,

            links_enum_start_cmd = 507,
            links_enum_start_ack = 508,
            links_enum_link_not = 509,
            links_enum_stop_cmd = 510,
            links_enum_stopped_not = 511,

            router_reset_cmd = 512,
            router_reset_ack = 513,

            ping_cmd = 514,
            ping_ack = 515,

            manage_comm_resource_start_cmd = 516,
            manage_comm_resource_start_ack = 517,
            manage_comm_resource_stop_cmd = 518,
            manage_comm_resource_stopped_not = 519,

            announce_access_level = 520,
            announce_address = 521,

            send_pakctrl_message_cmd = 522,
            send_pakctrl_message_ack = 523,
         };
      };
   };
};


#endif

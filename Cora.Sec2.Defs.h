/* Cora.Sec2.Defs.h

   Copyright (C) 2002, 2005 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 25 November 2002
   Last Change: Friday 25 March 2005
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)
   Last Changed by: $Author: tmecham $

*/

#ifndef Cora_Sec2_Defs_h
#define Cora_Sec2_Defs_h


namespace Cora
{
   namespace Sec2
   {
      namespace Messages
      {
         ////////////////////////////////////////////////////////////
         // enum MessageType
         ////////////////////////////////////////////////////////////
         enum MessageType
         {
            monitor_status_start_cmd = 716,
            monitor_status_not = 718,
            monitor_status_stop_cmd = 719,
            monitor_status_stopped_not = 720,

            enable_cmd = 721,
            enable_ack = 722,

            enum_accounts_start_cmd = 701,
            enum_accounts_start_ack = 702,
            enum_accounts_not = 703,
            enum_accounts_stop_cmd = 704,
            enum_accounts_stopped_not = 705,

            lock_start_cmd = 706,
            lock_start_ack = 707,
            lock_stop_cmd = 708,
            lock_stopped_not = 709,
            
            add_account_cmd = 710,
            add_account_ack = 711,

            delete_account_cmd = 712,
            delete_account_ack = 713,

            change_account_cmd = 714,
            change_account_ack = 715,

            announce_access_level = 723
         };
      };


      ////////////////////////////////////////////////////////////
      // AccessLevels
      //
      // Lists the access levels supported by the server interface
      ////////////////////////////////////////////////////////////
      namespace AccessLevels
      {
         enum AccessLevelType
         {
            level_read_only = 1000,
            level_operator = 2000,
            level_station_manager = 3000,
            level_administrator = 4000,
            level_root = 5000,
         };
      };
   };
};


#endif

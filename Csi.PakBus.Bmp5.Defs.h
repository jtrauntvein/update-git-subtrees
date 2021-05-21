/* Csi.PakBus.Bmp5.Defs.h

   Copyright (C) 2001, 2021 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 09 March 2001
   Last Change: Friday 26 March 2021
   Last Commit: $Date: 2021-03-26 13:51:03 -0600 (Fri, 26 Mar 2021) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Csi_PakBus_Bmp5_Defs_h
#define Csi_PakBus_Bmp5_Defs_h


#include "Csi.PakBus.Defs.h"


namespace Csi
{
   namespace PakBus
   {
      ////////////////////////////////////////////////////////////
      // namespace Bmp5Messages
      //
      // Defines the messages that can be sent or received using the BMP5 protocol.
      //////////////////////////////////////////////////////////// 
      namespace Bmp5Messages
      {
         enum code_type
         {
            // query capabilities
            query_capabilities_cmd = 0x1f,
            query_capabilities_ack = 0x9f,
            
            // SWF related messages
            one_way_swf_notification = 0x01,
            get_swf_cmd = 0x12,
            get_swf_ack = 0x92,

            // clock check/set messages
            clock_check_set_cmd = 0x03,// used with CR10X-TD loggers
            clock_check_set_ack = 0x83,
            clock_check_set_ex_cmd = 0x17, // used with x000 dataloggers
            clock_check_set_ex_ack = 0x97,

            // program file send messages
            program_file_send_cmd = 0x04,
            program_file_send_ack = 0x84,
            please_wait_notification = 0xa1,

            // program file receive messages
            program_file_receive_cmd = 0x05,
            program_file_receive_ack = 0x85,

            // get table definitions messages
            get_table_defs_cmd = 0x0e,
            get_table_defs_ack = 0x8e,

            // data advise messages
            start_data_advise_cmd = 0x07,
            start_data_advise_ack = 0x87,
            data_advise_notification = 0x88,

            // collect data messages
            collect_data_cmd = 0x09,
            collect_data_ack = 0x89,

            // extended collect data messages
            extended_collect_data_start_cmd = 0x29,
            extended_collect_data_fragment = 0xa9,
            extended_collect_data_fragment_ack = 0x2A,

            // one way table definition and data
            xtd_one_way_table_def_not = 0x13,
            one_way_table_def_not = 0x20,
            one_way_data_not = 0x14,

            // old set variable
            control_cmd = 0x0a,
            control_ack = 0x8a,

            // get variable
            get_variable_cmd = 0x15,
            get_variable_ack = 0x95,

            // set variable
            set_variable_cmd = 0x16,
            set_variable_ack = 0x96,

            // user i/o (terminal emulation) messages
            user_io_cmd = 0x0b,
            user_io_notification = 0x8b,

            // rf test messages
            rf_test_cmd = 0x0f,
            rf_test_ack = 0x8f,

            // table control message
            table_control_cmd = 0x19,
            table_control_ack = 0x99,

            // file send messages
            file_send_cmd = 0x1c,
            file_send_ack = 0x9c,
            extended_file_send_start_cmd = 0x26,
            extended_file_send_ack = 0xa6,
            extended_file_send_fragment = 0x27,

            // file receive messages
            file_receive_cmd = 0x1d,
            file_receive_ack = 0x9d,
            extended_file_receive_cmd = 0x24,
            extended_file_receive_fragment = 0xa4,
            extended_file_receive_ack = 0x25,

            // file control
            file_control_cmd = 0x1e,
            file_control_ack = 0x9e,

            // compile results
            get_compile_results_cmd = 0x18,
            get_compile_results_ack = 0x98,

            // get value
            get_value_cmd = 0x1a,
            get_value_ack = 0x9a,

            // set value
            set_value_cmd = 0x1b,
            set_value_ack = 0x9b,

            // wireless sensor transaction
            wireless_cmd = 0x22,
            wireless_ack = 0xa2,

            // memory receive/send transactions
            memory_rcv_cmd = 0x0d,
            memory_rcv_ack = 0x8d,
            memory_send_cmd = 0x0c,
            memory_send_ack = 0x8c,

            // check access level transaction
            check_access_cmd = 0x28,
            check_access_ack = 0xa8
         };
      };
   };

};

#endif

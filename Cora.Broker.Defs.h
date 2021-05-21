/* Cora.Broker.Defs.h

   Copyright (C) 1998, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 30 July 1999
   Last Change: Tuesday 04 September 2012
   Last Commit: $Date: 2012-09-05 10:57:02 -0600 (Wed, 05 Sep 2012) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Cora_Broker_Defs_h
#define Cora_Broker_Defs_h


namespace Cora
{
   namespace Broker
   {
      namespace Type
      {
         ////////// enum Code
         // Lists all possible data broker type codes
         enum Code
         {
            active = 1,
            backup = 2,
            client_defined = 3,
            statistics = 4,
         };
      };

      namespace Messages
      {
         ////////// enum Code
         // Enumeration of all message identifiers used in the data broker interface
         enum Code
         {
            table_defs_enum_cmd = 400,
            table_defs_enum_not = 401,
            table_defs_enum_stop_cmd = 402,
            
            table_def_get_cmd = 403,
            table_def_get_ack = 404,
            
            data_advise_start_cmd = 405,
            data_advise_start_ack = 406,
            data_advise_start_ack_ex = 439,
            data_advise_not = 407,
            data_advise_cont_cmd = 408,
            
            data_query_cmd = 409,
            data_query_ack = 410,
            data_query_return_recs = 411,
            data_query_cont_cmd = 412,
            
            get_table_data_index_cmd = 413,
            get_table_data_index_ack = 414,
            
            table_resize_cmd = 415,
            table_resize_ack = 416,
            
            metadata_enum_cmd = 417,
            metadata_enum_not = 418,
            metadata_enum_stop_cmd = 419,
            
            metadata_set_start_cmd = 423,
            metadata_set_ack = 424,
            metadata_set_send_cmd = 428,
            
            metadata_get_cmd = 420,
            metadata_get_ack = 421,
            metadata_get_cont_cmd = 422,
            
            metadata_del_cmd = 425,
            metadata_del_ack = 426,

            exception = 427,

            statistics_reset_cmd = 428,
            statistics_reset_ack = 429,

            extended_table_def_get_cmd = 430,
            extended_table_def_get_ack = 431,

            formatted_data_advise_start_cmd = 433,
            formatted_data_advise_start_ack = 434,
            formatted_data_advise_not = 435,
            formatted_data_advise_cont_cmd = 436,
            formatted_data_advise_stopped_not = 437,

            announce_access_level = 438,
         };
         
      };
   };
};

#endif

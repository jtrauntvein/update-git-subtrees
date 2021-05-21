/* Csi.PakBus.LowLevelDecoder.h

   Copyright (C) 2006, 2011 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 03 November 2006
   Last Change: Wednesday 13 April 2011
   Last Commit: $Date: 2011-04-13 14:41:11 -0600 (Wed, 13 Apr 2011) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Csi_PakBus_LowLevelDecoder_h
#define Csi_PakBus_LowLevelDecoder_h

#include "CsiTypeDefs.h"
#include "StrBin.h"
#include "Csi.ByteQueue.h"


namespace Csi
{
   namespace PakBus
   {
      ////////////////////////////////////////////////////////////
      // class LowLevelDecoder
      //
      // Defines an object that is able to decode the various PakBus
      // sub-protocols.
      ////////////////////////////////////////////////////////////
      class LowLevelDecoder
      {
      public:
         ////////////////////////////////////////////////////////////
         // synch_byte
         //
         // Used to mark the begining and end of packets
         ////////////////////////////////////////////////////////////
         static byte const synch_byte;

         ////////////////////////////////////////////////////////////
         // quoted_synch_byte
         ////////////////////////////////////////////////////////////
         static byte const quoted_synch_byte;

         ////////////////////////////////////////////////////////////
         // quote_byte
         ////////////////////////////////////////////////////////////
         static byte const quote_byte;

         ////////////////////////////////////////////////////////////
         // quoted_quote_byte
         ////////////////////////////////////////////////////////////
         static byte const quoted_quote_byte;

         ////////////////////////////////////////////////////////////
         // control_ring
         ////////////////////////////////////////////////////////////
         static byte const control_ring;

         ////////////////////////////////////////////////////////////
         // control_reserved
         ////////////////////////////////////////////////////////////
         static byte const control_reserved;

         ////////////////////////////////////////////////////////////
         // control_capabilities
         ////////////////////////////////////////////////////////////
         static byte const control_capabilities;

         ////////////////////////////////////////////////////////////
         // link_off_line
         ////////////////////////////////////////////////////////////
         static byte const link_off_line;

         ////////////////////////////////////////////////////////////
         // link_ready
         ////////////////////////////////////////////////////////////
         static byte const link_ready;

         ////////////////////////////////////////////////////////////
         // link_finished
         ////////////////////////////////////////////////////////////
         static byte const link_finished;

         ////////////////////////////////////////////////////////////
         // link_pause
         ////////////////////////////////////////////////////////////
         static byte const link_pause;
         
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         LowLevelDecoder(bool keep_out_of_band_ = false):
            state(state_wait_for_synch),
            storage_len(0),
            unquoted_len(0),
            keep_out_of_band(keep_out_of_band_)
         { }

         ////////////////////////////////////////////////////////////
         // get_storage
         ////////////////////////////////////////////////////////////
         void const *get_storage() const
         { return storage; }

         ////////////////////////////////////////////////////////////
         // get_storage_len
         ////////////////////////////////////////////////////////////
         uint4 get_storage_len() const
         { return storage_len; }

         ////////////////////////////////////////////////////////////
         // decode
         //
         // Decodes the bytes provided in the buffer and returns the number of
         // bytes processed in the processed parameter.  The return value will
         // identify the type of packet found, if any.  The decoded packet can
         // be obtained by calling get_storage() and get_storage_len().
         //
         // The begins_at parameter will be set to the offset at which the
         // potential beginning for a packet was found.  The processed
         // parameter will indicate the number of bytes that were processed
         // from the input buffer.
         ////////////////////////////////////////////////////////////
         enum decode_outcome_type
         {
            decode_incomplete,
            decode_found_control_packet,
            decode_found_serial_packet,
            decode_found_unquoted_packet,
            decode_found_devconfig_packet,
         };
         decode_outcome_type decode(
            void const *buff_,
            uint4 buff_len,
            uint4 &begins_at,
            uint4 &processed);
         decode_outcome_type decode(
            ByteQueue &buff);

         ////////////////////////////////////////////////////////////
         // get_state
         ////////////////////////////////////////////////////////////
         enum state_type
         {
            state_wait_for_synch,
            state_synch_found,
            state_serpkt,
            state_serpkt_quoted,
            state_control,
            state_control_quoted,
            state_devconfig,
            state_devconfig_quoted,
            state_unquoted_len,
            state_unquoted_body
         };
         state_type get_state() const
         { return state; }

         ////////////////////////////////////////////////////////////
         // get_out_of_band_buffer
         ////////////////////////////////////////////////////////////
         StrBin const &get_out_of_band_buffer() const
         { return out_of_band_buffer; }
         
      private:
         ////////////////////////////////////////////////////////////
         // state
         ////////////////////////////////////////////////////////////
         state_type state;
         
         ////////////////////////////////////////////////////////////
         // storage
         //
         // Holds the bytes accumulated during the decode process
         ////////////////////////////////////////////////////////////
         byte storage[1026];

         ////////////////////////////////////////////////////////////
         // storage_len
         //
         // Holds the number of bytes that have been placed in the storage
         // buffer.
         ////////////////////////////////////////////////////////////
         uint4 storage_len;

         ////////////////////////////////////////////////////////////
         // unquoted_len
         //
         // Holds the expected length of the unquoted body
         ////////////////////////////////////////////////////////////
         uint4 unquoted_len;

         ////////////////////////////////////////////////////////////
         // keep_out_of_band
         //
         // Set to true if the decoder should store all of the "out-of-band"
         // data that is read "between" messages to out_of_band_buffer.
         ////////////////////////////////////////////////////////////
         bool const keep_out_of_band;

         ////////////////////////////////////////////////////////////
         // out_of_band_buffer
         ////////////////////////////////////////////////////////////
         StrBin out_of_band_buffer; 
      };
   }
}
   
#endif

/* Csi.PakBus.LowLevelDecoder.cpp

   Copyright (C) 2006, 2011 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 03 November 2006
   Last Change: Wednesday 13 April 2011
   Last Commit: $Date: 2011-04-13 14:41:11 -0600 (Wed, 13 Apr 2011) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.PakBus.LowLevelDecoder.h"
#include "Csi.Utils.h"


namespace Csi
{
   namespace PakBus
   {
      ////////////////////////////////////////////////////////////
      // class LowLevelDecoder definitions
      ////////////////////////////////////////////////////////////
      byte const LowLevelDecoder::synch_byte = 0xbd;
      byte const LowLevelDecoder::quoted_synch_byte = 0xdd;
      byte const LowLevelDecoder::quote_byte = 0xbc;
      byte const LowLevelDecoder::quoted_quote_byte = 0xdc;
      byte const LowLevelDecoder::control_ring = 0x09;
      byte const LowLevelDecoder::control_reserved = 0x0d;
      byte const LowLevelDecoder::control_capabilities = 0x0e;
      byte const LowLevelDecoder::link_off_line = 0x08;
      byte const LowLevelDecoder::link_ready = 0x0a;
      byte const LowLevelDecoder::link_finished = 0x0b;
      byte const LowLevelDecoder::link_pause = 0x0c;

      
      LowLevelDecoder::decode_outcome_type
      LowLevelDecoder::decode(
         void const *buff_,
         uint4 buff_len,
         uint4 &begins_at,
         uint4 &processed)
      {
         byte const *buff = static_cast<byte const *>(buff_);
         decode_outcome_type rtn = decode_incomplete;
         begins_at = 0;
         out_of_band_buffer.cut(0);
         for(processed = 0;
             processed < buff_len && rtn == decode_incomplete;
             ++processed)
         {
            byte ch = buff[processed];
            byte high_nibble = ((ch & 0xf0) >> 4);
            switch(state)
            {
            case state_wait_for_synch:
               if(ch == synch_byte)
               {
                  state = state_synch_found;
                  begins_at = processed;
               }
               else if(keep_out_of_band)
                  out_of_band_buffer.append(&ch, 1);
               break;
               
            case state_synch_found:
               if(ch == synch_byte)
               {
                  begins_at = processed;
                  continue;
               }
               switch(high_nibble)
               {
               case control_ring:
               case control_reserved:
               case control_capabilities:
                  state = state_control;
                  storage_len = 1;
                  storage[0] = ch;
                  break;

               case link_off_line:
               case link_ready:
               case link_finished:
               case link_pause:
                  state = state_serpkt;
                  storage_len = 1;
                  storage[0] = ch;
                  break;

               default:
                  switch(ch)
                  {
                  case 0xf0:
                     state = state_unquoted_len;
                     storage_len = 0;
                     break;
                     
                  case 0xf2:
                     state = state_devconfig;
                     storage_len = 1;
                     storage[0] = ch;
                     break;
                     
                  default:
                     state = state_wait_for_synch;
                     break;
                  }
                  break;
               }
               break;

            case state_serpkt:
            case state_control:
            case state_devconfig:
               if(ch == quote_byte)
               {
                  switch(state)
                  {
                  case state_serpkt:
                     state = state_serpkt_quoted;
                     break;
                     
                  case state_control:
                     state = state_control_quoted;
                     break;
                     
                  case state_devconfig:
                     state = state_devconfig_quoted;
                     break;
                  }
               }
               else if(ch == synch_byte)
               {
                  uint2 sig = calcSigFor(storage,storage_len);
                  if(sig == 0)
                  {
                     switch(state)
                     {
                     case state_serpkt:
                        rtn = decode_found_serial_packet;
                        break;
                        
                     case state_control:
                        rtn = decode_found_control_packet;
                        break;
                        
                     case state_devconfig:
                        rtn = decode_found_devconfig_packet;
                        break;
                     }
                  }
                  state = state_wait_for_synch;
               }
               else
               {
                  if(storage_len >= sizeof(storage))
                     state = state_wait_for_synch;
                  else
                     storage[storage_len++] = ch;
               }
               break;

            case state_serpkt_quoted:
               if(storage_len + 1 < sizeof(storage))
               {
                  storage[storage_len++] = ch - 0x20;
                  state = state_serpkt;
               }
               else
                  state = state_wait_for_synch;
               break;
               
            case state_control_quoted:
               if(storage_len + 1 < sizeof(storage))
               {
                  storage[storage_len++] = ch - 0x20;
                  state = state_control;
               }
               else
                  state = state_wait_for_synch;
               break;
               
            case state_devconfig_quoted:
               if(storage_len + 1 < sizeof(storage))
               {
                  storage[storage_len++] = ch - 0x20;
                  state = state_devconfig;
               }
               else
                  state = state_wait_for_synch;
               break;

            case state_unquoted_len:
               storage[storage_len++] = ch;
               if(storage_len == 2)
               {
                  state = state_unquoted_body;
                  storage_len = 0;
                  unquoted_len = (static_cast<uint4>(storage[0]) << 8) + storage[1];
                  if(unquoted_len < 8 || unquoted_len > sizeof(storage))
                     state = state_wait_for_synch;
               }
               break;

            case state_unquoted_body:
               storage[storage_len++] = ch;
               if(storage_len == unquoted_len)
               {
                  rtn = decode_found_unquoted_packet;
                  state = state_wait_for_synch;
               }
               break;
            }
         }
         return rtn;
      } // decode


      LowLevelDecoder::decode_outcome_type
      LowLevelDecoder::decode(ByteQueue &buff)
      {
         uint4 begins_at = 0;
         uint4 processed = 0;
         char temp[1024];
         uint4 size(buff.copy(temp, sizeof(temp)));
         decode_outcome_type rtn(decode(temp, size, begins_at, processed));

         buff.pop(processed);
         return rtn;
      } // decode (from byte queue)
   }
}

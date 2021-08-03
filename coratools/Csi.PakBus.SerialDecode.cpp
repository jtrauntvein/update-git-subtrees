/* Csi.PakBus.SerialDecode.cpp

   Copyright (C) 2003, 2005 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 29 July 2003
   Last Change: Tuesday 01 March 2005
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)
   Last Changed by: $Author: tmecham $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.PakBus.SerialDecode.h"


namespace Csi
{
   namespace PakBus
   {
      namespace SerialDecode
      {
         byte const synch_byte = 0xbd;
         byte const quoted_synch_byte = 0xdd;
         byte const quote_byte = 0xbc;
         byte const quoted_quote_byte = 0xdc;


         uint4 decode_quoted_data(
            StrBin &dest,
            bool &unquote_next,
            decode_outcome_type &outcome,
            void const *source,
            uint4 source_len)
         {
            byte const *s = static_cast<byte const *>(source);
            uint4 rtn = 0;
            outcome = decode_synch_not_found;
            
            while(rtn < source_len)
            {
               byte temp = s[rtn++];
               if(unquote_next)
               {
                  unquote_next = false;
                  if(temp == quoted_quote_byte || temp == quoted_synch_byte)
                     dest.append(temp - 0x20);
                  else
                  {
                     dest.cut(0);
                     outcome = decode_quote_error;
                     break;
                  }
               }
               else if(temp == quote_byte)
                  unquote_next = true;
               else if(temp != synch_byte)
                  dest.append(temp);
               else 
               {
                  outcome = decode_synch_found;
                  break;
               }
               if(dest.length() > 1024)
               {
                  outcome = decode_packet_too_long;
                  break;
               }
            }
            return rtn;
         } // decode_quoted_data 
      };
   };
};


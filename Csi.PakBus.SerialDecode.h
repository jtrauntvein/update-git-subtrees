/* Csi.PakBus.SerialDecode.h

   Copyright (C) 2003, 2005 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 29 July 2003
   Last Change: Friday 25 March 2005
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)
   Last Changed by: $Author: tmecham $

*/

#ifndef Csi_PakBus_SerialDecode_h
#define Csi_PakBus_SerialDecode_h

#include "StrBin.h"
#include "CsiTypeDefs.h"


namespace Csi
{
   namespace PakBus
   {
      namespace SerialDecode
      {
         //@group extern definitions
         extern byte const synch_byte;
         extern byte const quoted_synch_byte;
         extern byte const quote_byte;
         extern byte const quoted_quote_byte; 
         //@endgroup

         ////////////////////////////////////////////////////////////
         // decode_quoted_data
         //
         // Implements the decoding algorithm for decoding quoted data.  All
         // the state for the decoder is passed through the arguments so that
         // this method can be called by derived classes without affecting the
         // state of this port.  For this reason, the method is declared as
         // static.  The method will stop decoding once an uncoded serial synch
         // byte is encountered.  The return value will indicate the number of
         // bytes from the source buffer that were decoded.
         ////////////////////////////////////////////////////////////
         enum decode_outcome_type
         {
            decode_synch_found,
            decode_synch_not_found,
            decode_quote_error,
            decode_packet_too_long
         };
         uint4 decode_quoted_data(
            StrBin &dest,
            bool &unquote_next,
            decode_outcome_type &outcome,
            void const *source,
            uint4 source_len);
      };
   };
};


#endif

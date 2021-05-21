/* Csi.PakBus.BlowfishCipher.cpp

   Copyright (C) 2012, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Monday 15 October 2012
   Last Change: Wednesday 24 October 2012
   Last Commit: $Date: 2012-10-24 16:54:27 -0600 (Wed, 24 Oct 2012) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.PakBus.BlowfishCipher.h"


namespace Csi
{
   namespace PakBus
   {
      void BlowfishCipher::encrypt(StrBin &dest, void const *buff, uint4 buff_len)
      {
         encoder.encrypt(dest, buff, buff_len, true);
      } // encrypt


      void BlowfishCipher::decrypt(StrBin &dest, void const *buff, uint4 buff_len)
      {
         encoder.decrypt(dest, buff, buff_len, true);
      } // decrypt
   };
};


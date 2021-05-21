/* Csi.PakBus.BlowfishCipher.h

   Copyright (C) 2012, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Monday 15 October 2012
   Last Change: Friday 29 April 2016
   Last Commit: $Date: 2016-05-03 16:18:03 -0600 (Tue, 03 May 2016) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_PakBus_BlowfishCipher_h
#define Csi_PakBus_BlowfishCipher_h

#include "Csi.PakBus.CipherBase.h"
#include "Csi.PakBus.PakCtrlMessage.h"
#include "Csi.BlowFish.h"
#include "StrAsc.h"


namespace Csi
{
   namespace PakBus
   {
      ////////////////////////////////////////////////////////////
      // class BlowfishCipher
      //
      // Defines a class that uses the BlowFish encryption algorithm to encode
      // and decode PakBus messages.
      ////////////////////////////////////////////////////////////
      class BlowfishCipher: public CipherBase
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         BlowfishCipher(StrAsc const &key):
         encoder(key.c_str(), (uint4)key.length())
         { }

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~BlowfishCipher()
         { }
         
         ////////////////////////////////////////////////////////////
         // max_payload_size
         ////////////////////////////////////////////////////////////
         virtual uint4 max_payload_size() const
         { return PakCtrlMessage::max_body_len - CipherBase::header_len() - 8; }

         ////////////////////////////////////////////////////////////
         // encrypt
         ////////////////////////////////////////////////////////////
         virtual void encrypt(StrBin &dest, void const *buff, uint4 buff_len);

         ////////////////////////////////////////////////////////////
         // decrypt
         ////////////////////////////////////////////////////////////
         virtual void decrypt(StrBin &dest, void const *buff, uint4 buff_len);

         ////////////////////////////////////////////////////////////
         // get_cipher_code
         ////////////////////////////////////////////////////////////
         virtual byte get_cipher_code() const
         { return 0; }

      private:
         ////////////////////////////////////////////////////////////
         // encoder
         ////////////////////////////////////////////////////////////
         BlowFish encoder;
      };
   };
};


#endif

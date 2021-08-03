/* Csi.PakBus.BlowfishCipher.h

   Copyright (C) 2012, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Monday 15 October 2012
   Last Change: Friday 03 June 2016
   Last Commit: $Date: 2012-10-18 09:11:36 -0600 (Thu, 18 Oct 2012) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_PakBus_BlowfishCipher_h
#define Csi_PakBus_BlowfishCipher_h

#include "Csi.PakBus.CipherBase.h"
#include "Csi.PakBus.PakCtrlMessage.h"
#include "StrAsc.h"


namespace Csi
{
   namespace PakBus
   {
      ////////////////////////////////////////////////////////////
      // class AesCipher
      //
      // Defines a class that uses the BlowFish encryption algorithm to encode
      // and decode PakBus messages.
      ////////////////////////////////////////////////////////////
      class AesCipher: public CipherBase
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         AesCipher(StrAsc const &key);

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~AesCipher();
         
         ////////////////////////////////////////////////////////////
         // max_payload_size
         ////////////////////////////////////////////////////////////
         virtual uint4 max_payload_size() const
         { return PakCtrlMessage::max_body_len - CipherBase::header_len() - 16; }

         ////////////////////////////////////////////////////////////
         // set_initialisation_vector
         ////////////////////////////////////////////////////////////
         virtual void set_initialisation_vector(
            void const *buff, uint4 buff_len);

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
         { return 1; }

      private:
         ////////////////////////////////////////////////////////////
         // encrypt_context
         ////////////////////////////////////////////////////////////
         byte *encrypt_context;

         ////////////////////////////////////////////////////////////
         // decrypt_context
         ////////////////////////////////////////////////////////////
         byte *decrypt_context;

         /**
          * Specifies the current last initialisation vector.
          */
         byte init_vector[16];
      };
   };
};


#endif

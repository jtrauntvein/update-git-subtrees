/* Csi.PakBus.CipherBase.h

   Copyright (C) 2012, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Monday 15 October 2012
   Last Change: Wednesday 17 October 2012
   Last Commit: $Date: 2013-01-07 14:28:53 -0600 (Mon, 07 Jan 2013) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_PakBus_CipherBase_h
#define Csi_PakBus_CipherBase_h

#include "StrBin.h"
#include "CsiTypeDefs.h"


namespace Csi
{
   namespace PakBus
   {
      ////////////////////////////////////////////////////////////
      // class CipherBase
      //
      // Defines a base class for an object that manages encryption and
      // decryption of PakBus messages.  
      ////////////////////////////////////////////////////////////
      class CipherBase
      {
      public:
         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~CipherBase()
         { }

         ////////////////////////////////////////////////////////////
         // max_payload_size
         //
         // Must be overloaded to return the maximum payload size supported for
         // this cipher.  
         ////////////////////////////////////////////////////////////
         virtual uint4 max_payload_size() const = 0;

         ////////////////////////////////////////////////////////////
         // encrypt
         //
         // Must be overloaded to decrypt the specified buffer into the
         // specified destination. 
         ////////////////////////////////////////////////////////////
         virtual void encrypt(StrBin &dest, void const *buff, uint4 buff_len) = 0;

         ////////////////////////////////////////////////////////////
         // decrypt
         //
         // Must be overloaded to encrypt the specified buffer into the
         // specified destination
         ////////////////////////////////////////////////////////////
         virtual void decrypt(StrBin &dest, void const *buff, uint4 buff_len) = 0;

         ////////////////////////////////////////////////////////////
         // header_len
         //
         // Specifies the number of bytes in the encrypted message header.
         ////////////////////////////////////////////////////////////
         virtual uint4 header_len() const
         { return 8; }

         ////////////////////////////////////////////////////////////
         // get_cipher_code
         //
         // Must be overloaded to return the code for the encryption cipher for
         // which this object will work.
         ////////////////////////////////////////////////////////////
         virtual byte get_cipher_code() const = 0;

         ////////////////////////////////////////////////////////////
         // set_initialisation_vector
         ////////////////////////////////////////////////////////////
         virtual void set_initialisation_vector(
            void const *header, uint4 header_len)
         { }
      };
   };
};


#endif

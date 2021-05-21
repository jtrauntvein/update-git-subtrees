/* Csi.PakBus.AesCipher.cpp

   Copyright (C) 2013, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Monday 07 January 2013
   Last Change: Monday 13 June 2016
   Last Commit: $Date:$
   Last Changed by: $Author:$

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.PakBus.AesCipher.h"
#include "Csi.Digest.h"
#include "mbedtls/aes.h"


namespace Csi
{
   namespace PakBus
   {
      ////////////////////////////////////////////////////////////
      // class AesCipher definitions
      ////////////////////////////////////////////////////////////
      AesCipher::AesCipher(StrAsc const &key)
      {
         // we will calculate the MD5 checksum of the pakbus encryption key to use as the AES key
         Md5Digest md5;
         uint8_t aes_key[Md5Digest::digest_size];

         md5.add(key.c_str(), key.length());
         memcpy(aes_key, md5.final(), sizeof(aes_key));
         
         // we need to construct the encrypt context
         mbedtls_aes_context *encrypt_struct;
         mbedtls_aes_context *decrypt_struct;
         
         memset(init_vector, 0, sizeof(init_vector));
         encrypt_context = new byte[sizeof(mbedtls_aes_context)];
         decrypt_context = new byte[sizeof(mbedtls_aes_context)];
         encrypt_struct = reinterpret_cast<mbedtls_aes_context *>(encrypt_context);
         decrypt_struct = reinterpret_cast<mbedtls_aes_context *>(decrypt_context);
         mbedtls_aes_init(encrypt_struct);
         mbedtls_aes_init(decrypt_struct);
         mbedtls_aes_setkey_enc(encrypt_struct, aes_key, 128);
         mbedtls_aes_setkey_dec(decrypt_struct, aes_key, 128);
      } // constructor


      AesCipher::~AesCipher()
      {
         delete[] encrypt_context;
         delete[] decrypt_context;
      } // destructor


      void AesCipher::set_initialisation_vector(
         void const *buff, uint4 buff_len)
      {
         Md5Digest md5;
         md5.add(buff, buff_len);
         memcpy(init_vector, md5.final(), Md5Digest::digest_size);
      } // set_initialisation_vector


      void AesCipher::encrypt(
         StrBin &dest, void const *buff, uint4 buff_len)
      {
         // we need to ensure that the input buffer length is divisible by 16
         byte input[1024];
         byte output[sizeof(input)];
         assert(buff_len <= sizeof(input));
         memset(input, 0, sizeof(input));
         memset(output, 0, sizeof(output));
         memcpy(input, buff, buff_len);
         if(buff_len % 16 != 0)
            buff_len += 16 - buff_len % 16;

         // now call the encrypt function
         mbedtls_aes_crypt_cbc(
            reinterpret_cast<mbedtls_aes_context *>(encrypt_context),
            MBEDTLS_AES_ENCRYPT,
            buff_len,
            init_vector,
            input,
            output);
         dest.append(output, buff_len);
      } // encrypt


      void AesCipher::decrypt(
         StrBin &dest, void const *buff, uint4 buff_len)
      {
         // the length of the input buffer should already be divisible by the block size
         byte output[1024];
         assert(buff_len % 16 == 0);
         assert(buff_len <= sizeof(output));
         mbedtls_aes_crypt_cbc(
            reinterpret_cast<mbedtls_aes_context *>(decrypt_context),
            MBEDTLS_AES_DECRYPT,
            buff_len,
            init_vector,
            static_cast<byte const *>(buff),
            output);
         dest.append(output, buff_len);
      } // decrypt
   };
};



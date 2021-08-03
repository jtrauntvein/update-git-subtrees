/* Csi.Digest.cpp

   Copyright (C) 2014, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Monday 15 December 2014
   Last Change: Thursday 01 October 2020
   Last Commit: $Date: 2020-10-02 10:01:41 -0600 (Fri, 02 Oct 2020) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Digest.h"
#include "Csi.Base64.h"
#include "mbedtls/md5.h"
#include "mbedtls/sha1.h"
#include "mbedtls/sha256.h"
#include <cstring>


namespace Csi
{
   class Md5Digest::impl_type
   {
   public:
      /**
       * Holds the digest
       */
      uint8_t digest[Md5Digest::digest_size];
      
      /**
       * Holds the context used for the calculations.
       */
      mbedtls_md5_context context;
   };


   Md5Digest::Md5Digest():
      impl(new impl_type)
   {
      reset();
   }


   Md5Digest::~Md5Digest()
   { delete impl; }


   void Md5Digest::reset()
   {
      mbedtls_md5_init(&impl->context);
      mbedtls_md5_starts(&impl->context);
   } // reset


   void Md5Digest::add(void const *buff, size_t buff_len)
   {
      mbedtls_md5_update(&impl->context, static_cast<uint8_t const *>(buff), buff_len);
   } // add


   void const *Md5Digest::final()
   {
      mbedtls_md5_finish(&impl->context, impl->digest);
      return impl->digest;
   } // final


   StrAsc Md5Digest::final_base64()
   {
      StrAsc rtn;
      Base64::encode(rtn, final(), digest_size);
      return rtn;
   } // final_base64


   Md5HmacDigest::Md5HmacDigest(void const *key, size_t key_len)
   {
      memset(opad, 0, sizeof(opad));
      memset(ipad, 0, sizeof(ipad));
      if(key_len < sizeof(ipad))
      {
         std::memcpy(ipad, key, key_len);
         std::memcpy(opad, key, key_len);
      }
      else
      {
         void const *key_digest;
         digest.add(key, key_len);
         key_digest = digest.final();
         std::memcpy(ipad, key_digest, digest_size);
         std::memcpy(opad, key_digest, digest_size);
         digest.reset();
      }
      for(size_t i = 0; i < sizeof(ipad); ++i)
      {
         ipad[i] ^= 0x36;
         opad[i] ^= 0x5c;
      }
      reset();
   } // constructor


   Md5HmacDigest::~Md5HmacDigest()
   { }


   void Md5HmacDigest::reset()
   {
      digest.reset();
      digest.add(ipad, sizeof(ipad));
   } // reset


   void const *Md5HmacDigest::final()
   {
      uint8_t temp[digest_size];
      memcpy(temp, digest.final(), digest_size);
      digest.reset();
      digest.add(opad, sizeof(opad));
      digest.add(temp, digest_size);
      return digest.final();
   } // final


   StrAsc Md5HmacDigest::final_base64()
   {
      StrAsc rtn;
      Base64::encode(rtn, final(), digest_size);
      return rtn;
   } // final_base64
   

   class Sha1Digest::impl_type
   {
   public:
      /**
       * Holds the calculated digest.
       */
      uint8_t digest[Sha1Digest::digest_size];

      /**
       * Holds the context
       */
      mbedtls_sha1_context context;
   };


   Sha1Digest::Sha1Digest():
      impl(new impl_type)
   { reset(); }


   Sha1Digest::~Sha1Digest()
   { delete impl; }


   void Sha1Digest::reset()
   {
      mbedtls_sha1_init(&impl->context);
      mbedtls_sha1_starts(&impl->context);
   }


   void Sha1Digest::add(void const *buff, size_t buff_len)
   {
      mbedtls_sha1_update(
         &impl->context, static_cast<uint8_t const *>(buff), buff_len);
   }


   void const *Sha1Digest::final()
   {
      mbedtls_sha1_finish(&impl->context, impl->digest);
      return impl->digest;
   } // final


   StrAsc Sha1Digest::final_base64()
   {
      StrAsc rtn;
      Base64::encode(rtn, final(), digest_size);
      return rtn;
   } // final_base64


   Sha1HmacDigest::Sha1HmacDigest(void const *key, size_t key_len)
   {
      memset(opad, 0, sizeof(opad));
      memset(ipad, 0, sizeof(ipad));
      if(key_len < sizeof(ipad))
      {
         memcpy(ipad, key, key_len);
         memcpy(opad, key, key_len);
      }
      else
      {
         void const *key_digest;
         digest.add(key, key_len);
         key_digest = digest.final();
         memcpy(ipad, key_digest, digest_size);
         memcpy(opad, key_digest, digest_size);
         digest.reset();
      }
      for(size_t i = 0; i < sizeof(ipad); ++i)
      {
         ipad[i] ^= 0x36;
         opad[i] ^= 0x5c;
      }
      reset();
   } // constructor


   Sha1HmacDigest::~Sha1HmacDigest()
   { }


   void Sha1HmacDigest::reset()
   {
      digest.reset();
      digest.add(ipad, sizeof(ipad));
   } // reset


   void const *Sha1HmacDigest::final()
   {
      uint8_t temp[digest_size];
      memcpy(temp, digest.final(), digest_size);
      digest.reset();
      digest.add(opad, sizeof(opad));
      digest.add(temp, digest_size);
      return digest.final();
   } // final


   StrAsc Sha1HmacDigest::final_base64()
   {
      StrAsc rtn;
      Base64::encode(rtn, final(), digest_size);
      return rtn;
   } // final_base64


   class Sha256Digest::impl_type
   {
   public:
      /**
       * Holds the calculated digest.
       */
      uint8_t digest[Sha256Digest::digest_size];

      /**
       * Holds the context.
       */
      mbedtls_sha256_context context;
   };


   Sha256Digest::Sha256Digest():
      impl(new impl_type)
   { reset(); }


   Sha256Digest::~Sha256Digest()
   { delete impl; }


   void Sha256Digest::reset()
   {
      mbedtls_sha256_init(&impl->context);
      mbedtls_sha256_starts(&impl->context, 0);
   } // reset


   void Sha256Digest::add(void const *buff, size_t buff_len)
   {
      mbedtls_sha256_update(&impl->context, static_cast<uint8_t const *>(buff), buff_len); 
   } // add


   void const *Sha256Digest::final()
   {
      mbedtls_sha256_finish(&impl->context, impl->digest);
      return impl->digest;
   } // final


   StrAsc Sha256Digest::final_base64()
   {
      StrAsc rtn;
      Base64::encode(rtn, final(), digest_size);
      return rtn;
   } // final_base64


   Sha256HmacDigest::Sha256HmacDigest(void const *key, size_t key_len)
   {
      memset(opad, 0, sizeof(opad));
      memset(ipad, 0, sizeof(ipad));
      if(key_len < sizeof(ipad))
      {
         memcpy(ipad, key, key_len);
         memcpy(opad, key, key_len);
      }
      else
      {
         void const *key_digest;
         digest.add(key, key_len);
         key_digest = digest.final();
         memcpy(ipad, key_digest, digest_size);
         memcpy(opad, key_digest, digest_size);
         digest.reset();
      }
      for(size_t i = 0; i < sizeof(ipad); ++i)
      {
         ipad[i] ^= 0x36;
         opad[i] ^= 0x5c;
      }
      reset();
   } // constructor

   Sha256HmacDigest::~Sha256HmacDigest()
   { }

   void Sha256HmacDigest::reset()
   {
      digest.reset();
      digest.add(opad, sizeof(opad));
   } // reset

   void const *Sha256HmacDigest::final()
   {
      uint8_t temp[digest_size];
      memcpy(temp, digest.final(), digest_size);
      digest.reset();
      digest.add(opad, sizeof(opad));
      digest.add(temp, digest_size);
      return digest.final();
   } // final

   StrAsc Sha256HmacDigest::final_base64()
   {
      StrAsc rtn;
      Base64::encode(rtn, final(), digest_size);
      return rtn;
   } // final_base64
};


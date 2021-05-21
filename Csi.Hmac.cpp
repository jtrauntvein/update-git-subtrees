/* Csi.Hmac.cpp

   Copyright (C) 2014, 2016 Campbell Scientific, Inc.

   Written by: Tyler Mecham
   Date Begun: Monday, May 19, 2014
   Last Change: Friday 03 June 2016
   Last Commit: $Date: 2016-07-14 14:42:44 -0600 (Thu, 14 Jul 2016) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header

#include "Csi.Hmac.h"
#include "Csi.Digest.h"
#include "StrBin.h"


namespace Csi
{
   void csi_hmac_md5(
      StrAsc const &msg,
      StrAsc const &key,
      unsigned char *digest)
   {
      uint8_t ipad[64];
      uint8_t opad[64];
      Md5Digest md5;
      
      memset(ipad, 0, sizeof(ipad));
      memset(opad, 0, sizeof(opad));
      if(key.length() < sizeof(ipad))
      {
         memcpy(ipad, key.c_str(), key.length());
         memcpy(opad, key.c_str(), key.length());
      }
      else
      {
         void const *key_digest;
         md5.add(key.c_str(), key.length());
         key_digest = md5.final();
         memcpy(ipad, digest, Md5Digest::digest_size);
         memcpy(opad, digest, Md5Digest::digest_size);
      }
      for(size_t i = 0; i < sizeof(ipad); ++i)
      {
         ipad[i] ^= 0x36;
         opad[i] ^= 0x5c;
      }
      md5.reset();
      md5.add(ipad, sizeof(ipad));
      md5.add(msg.c_str(), msg.length());
      memcpy(digest, md5.final(), Md5Digest::digest_size);
      md5.reset();
      md5.add(opad, sizeof(opad));
      md5.add(digest, Md5Digest::digest_size);
      memcpy(digest, md5.final(), Md5Digest::digest_size);
   } // csi_hmac_md5


   void csi_hmac_sha1(
      StrAsc const &msg,
      StrAsc const &key,
      unsigned char *digest)
   {
      // calculate the actual key based upon the provided key length
      Sha1Digest sha1;
      uint8_t ipad[64], opad[64];
      memset(ipad, 0, sizeof(ipad));
      memset(opad, 0, sizeof(opad));
      if(key.length() > 64)
      {
         sha1.add(key.c_str(), key.length());
         memcpy(ipad, sha1.final(), Sha1Digest::digest_size);
         memcpy(opad, ipad, Sha1Digest::digest_size);
         sha1.reset();
      }
      else
      {
         memcpy(ipad, key.c_str(), key.length());
         memcpy(opad, key.c_str(), key.length());
      }
      for(size_t i = 0; i < sizeof(ipad); ++i)
      {
         ipad[i] ^= 0x36;
         opad[i] ^= 0x5c;
      }

      // perform the hmac calculation.
      sha1.add(ipad, sizeof(ipad));
      sha1.add(msg.c_str(), msg.length());
      memcpy(digest, sha1.final(), Sha1Digest::digest_size);
      sha1.reset();
      sha1.add(opad, sizeof(opad));
      sha1.add(digest, Sha1Digest::digest_size);
      memcpy(digest, sha1.final(), Sha1Digest::digest_size);
   } // csi_hmac_sha1
};

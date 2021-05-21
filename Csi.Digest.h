/* Csi.Digest.h

   Copyright (C) 2014, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Friday 12 December 2014
   Last Change: Friday 02 October 2020
   Last Commit: $Date: 2020-10-02 10:54:16 -0600 (Fri, 02 Oct 2020) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_Digest_h
#define Csi_Digest_h

#include <stddef.h>
#include <stdint.h>
#include "StrAsc.h"


namespace Csi
{
   /**
    * Defines an object that calculates a digest using the MD5 digest algorithm.
    */
   class Md5Digest
   {
   public:
      /**
       * Constructor
       */
      Md5Digest();

      /**
       * Destructor
       */
      ~Md5Digest();
      
      /**
       * Resets the digest context for performing a new calculation.
       */
      void reset();
      
      /**
       * Adds the contents of the specified buffer to the digest calculation.
       *
       * @param buff Specifies the beginning of the content to add.
       *
       * @param buff_len Specifies the number of bytes to add.
       */
      void add(void const *buff, size_t buff_len);
      
      /**
       * @return Returns the final digest.  
       */
      void const *final();

      /**
       * @return Returns the final checksum as a base64 encoded string.
       */
      StrAsc final_base64();

      /**
       * Specifies the size of the MD5 digest.
       */
      static size_t const digest_size = 16;
      
   private:
      /**
       * Specifies the details of this algorithm.
       */
      class impl_type;
      impl_type *impl;
   };


   /**
    * Defines a digest that uses an MD5-HMAC algorithm with an application spplication specified key
    * to calculate the digest.
    */
   class Md5HmacDigest
   {
   public:
      /**
       * Constructor
       *
       * @param key Specifies the key that will be used to calculate the final hmac digest.
       *
       * @param key_len Specifies the number of bytes in the key.
       */
      Md5HmacDigest(void const *key, size_t key_len);

      /**
       * Destructor
       */
      ~Md5HmacDigest();

      /**
       * Resets the digest context for performing a new calculation.
       */
      void reset();

      /**
       * Adds the contents of the specified buffer to the digest calculation.
       *
       * @param buff Specifies the start of the byte buffer.
       *
       * @param buff_len Specifies the number of bytes to add.
       */
      void add(void const *buff, size_t buff_len)
      { digest.add(buff, buff_len); }

      /**
       * @return Returns the final digest calculated with the HMAC included.  The length of the
       * returned buffer will be equal to digest_size.
       */
      void const *final();

      /**
       * @return Returns the checksum as a base64 encoded string.
       */
      StrAsc final_base64();

      /**
       * Specifies the size of the digest returned from final().
       */
      static size_t const digest_size = Md5Digest::digest_size;
      
   private:
      /**
       * Specifies the ipad value (derived from the key given in the constructor).
       */
      uint8_t ipad[64];

      /**
       * Specifies the opad value (derived from the key given in the constructor)
       */
      uint8_t opad[64];

      /**
       * Specifies the object used to calculate the digest.
       */
      Md5Digest digest;
   };


   /**
    * Defines an object that calculates an SHA1 digest.
    */
   class Sha1Digest
   {
   public:
      /**
       * Constructor
       */
      Sha1Digest();

      /**
       * Destructor
       */
      ~Sha1Digest();
      
      /**
       * Resets the digest calculation.
       */
       void reset();

      /**
       * Ads the specified bytes to the digest.
       *
       * @param buff Specifies the beginning of the buffer to add.
       *
       * @param buff_len  Specifies the number of bytes.
       */
       void add(void const *buff, size_t buff_len);

      /**
       * Specifies the size of the SHA1 digest.
       */
      static size_t const digest_size = 20;

      /**
       * @return Returns the final calculation of the digest.
       */
      void const *final();

      /**
       * @return Returns the checksum as a base64 encoded string,
       */
      StrAsc final_base64();

   private:
      /**
       * Specifies the object that holds implementation details for this digest.
       */
      class impl_type;
      impl_type *impl;
   };


   /**
    * Defines an object that calculates an HMAC based upon a SHA1 digest and an application provided
    * key. 
    */
   class Sha1HmacDigest
   {
   public:
      /**
       * Constructor
       *
       * @param key Specifies the key that will be used to calculate the final hmac digest.
       *
       * @param key_len Specifies the number of bytes in the key.
       */
      Sha1HmacDigest(void const *key, size_t key_len);

      /**
       * Destructor
       */
      ~Sha1HmacDigest();

      /**
       * Resets the digest context for performing a new calculation.
       */
      void reset();

      /**
       * Adds the contents of the specified buffer to the digest calculation.
       *
       * @param buff Specifies the start of the byte buffer.
       *
       * @param buff_len Specifies the number of bytes to add.
       */
      void add(void const *buff, size_t buff_len)
      { digest.add(buff, buff_len); }

      /**
       * @return Returns the final digest calculated with the HMAC included.  The length of the
       * returned buffer will be equal to digest_size.
       */
      void const *final();

      /**
       * @return Returns the checksum encoded as base64.
       */
      StrAsc final_base64();

      /**
       * Specifies the size of the digest returned from final().
       */
      static size_t const digest_size = Sha1Digest::digest_size;
      
   private:
      /**
       * Specifies the ipad value (derived from the key given in the constructor).
       */
      uint8_t ipad[64];

      /**
       * Specifies the opad value (derived from the key given in the constructor)
       */
      uint8_t opad[64];

      /**
       * Specifies the object used to calculate the digest.
       */
      Sha1Digest digest;
   };


   /**
    * Defines an object that calculates an SHA-256 digest.
    */
   class Sha256Digest
   {
   public:
      /**
       * Constructor
       */
      Sha256Digest();

      /**
       * Destructor
       */
      ~Sha256Digest();

      /**
       * Resets the digest calculation.
       */
      void reset();

      /**
       * Adds the specifies bytes to the digest.
       *
       * @param buff Specifies the beginning of the buffer to add.
       *
       * @param buff_len Specifies the number of bytes to add.
       */
      void add(void const *buff, size_t buff_len);

      /**
       * Specifies the size of the SHA256 Digest.
       */
      static size_t const digest_size = 32;

      /**
       * @return Returns the start of a buffer that contains the final digest.
       */
      void const *final();

      /**
       * @return Returns the checksum as a base64 encoded string,
       */
      StrAsc final_base64();
      
   private:
      /**
       * Specifies the object that performs the actual implementation.
       */
      class impl_type;
      impl_type *impl;
   };


   /**
    * Defines an object that calcumates the HMAC based upon a SHA256 digest and an application
    * provided key.
    */
   class Sha256HmacDigest
   {
   private:
      /**
       * Specifies the ipad value (calculated from the key).
       */
      uint8_t ipad[64];

      /**
       * Specifies the opad value (calculated from the key).
       */
      uint8_t opad[64];

      /**
       * Specifies the digest
       */
      Sha256Digest digest;
      
   public:
      /**
       * Constructor
       *
       * @param key Specifies the key that will be used to calculate the final digest.
       *
       * @param key_len Specifies the length of the key.
       */
      Sha256HmacDigest(void const *key, size_t key_len);

      /**
       * Destructor
       */
      ~Sha256HmacDigest();

      /**
       * Resets the calculation.
       */
      void reset();

      /**
       * Adds the contents of the specified buff to the digest.
       *
       * @param buff Specifies the start of the buffer.
       *
       @param buff_len Specifies the number of bytes in the buffer.
      */
      void add(void const *buff, size_t buff_len)
      { digest.add(buff, buff_len); }
         
      /**
       * Specifies the digest size.
       */
      static size_t const digest_size = Sha256Digest::digest_size;

      /**
       * @return Returns the final digest with the HMAC included.  The length of the returned buffer
       * will be equal to the digest_size property.
       */
      void const *final();

      /**
       * @return Returns the the final digest encoded in base64.
       */
      StrAsc final_base64();
   };
};


#endif

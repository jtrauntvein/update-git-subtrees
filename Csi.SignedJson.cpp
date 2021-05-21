/* Csi.SignedJson.cpp

   Copyright (C) 2017, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Saturday 15 July 2017
   Last Change: Wednesday 10 January 2018
   Last Commit: $Date: 2018-02-21 18:00:05 -0600 (Wed, 21 Feb 2018) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.SignedJson.h"
#include "Csi.Base64.h"
#include "Csi.Digest.h"
#include "coratools.strings.h"
#include "Csi.Utils.h"
#include "Csi.StrAscStream.h"


namespace Csi
{
   namespace
   {
      enum hmac_id_type
      {
         hmac_id_none,
         hmac_id_sha1
      };
      struct available_algorithm_type
      {
         StrAsc const name;
         hmac_id_type hmac_id;
         StrAsc const private_key;
      } available_algorithms[] =
      {
         { "CSI1", hmac_id_sha1, "b78364d7-16c6-4e1f-aa4c-bb937ae7a15a" },
         { "", hmac_id_none, "" }
      };
      size_t const available_algorithms_count(sizeof(available_algorithms) / sizeof(available_algorithms[0]));
   };
   
   
   char const *SignedJsonException::what() const throw()
   {
      using namespace Csi::SignedJsonStrings;
      return my_strings[failure].c_str();
   }


   void SignedJson::initialise(Csi::Json::Object &source)
   {
      try
      {
         // we need to extract the envelope fields and decode the signature and content.
         StrAsc signature_encoded(source.get_property_str("signature"));
         StrAsc content_encoded(source.get_property_str("content"));
         signature_algorithm = source.get_property_str("signature_alg");
         nonce = source.get_property_str("nonce");
         Base64::decode(signature, signature_encoded.c_str(), signature_encoded.length());
         Base64::decode(content, content_encoded.c_str(), content_encoded.length());
         bool found(false);
         
         // we now need to verify the signature.  We start by searching for the algorithm.
         for(size_t i = 0; !found && i < available_algorithms_count; ++i)
         {
            available_algorithm_type const &algorithm(available_algorithms[i]);
            if(algorithm.hmac_id == hmac_id_none)
               throw SignedJsonException(signed_json_failure_unsupported_algorithm);
            if(signature_algorithm == algorithm.name)
            {
               found = true;
               if(algorithm.hmac_id == hmac_id_sha1)
               {
                  Sha1HmacDigest digest(algorithm.private_key.c_str(), algorithm.private_key.length());
                  byte temp[Sha1HmacDigest::digest_size];
                  
                  if(signature.length() != Sha1HmacDigest::digest_size)
                     throw SignedJsonException(signed_json_failure_invalid_signature);
                  digest.add(content_encoded.c_str(), content_encoded.length());
                  digest.add(nonce.c_str(), nonce.length());
                  memcpy(temp, digest.final(), Sha1HmacDigest::digest_size);
                  if(memcmp(temp, signature.getContents(), signature.length()) != 0)
                     throw SignedJsonException(signed_json_failure_invalid_signature);
               }
               else
                  throw SignedJsonException(signed_json_failure_unsupported_algorithm);
            }
         }
      }
      catch(std::exception &)
      { throw SignedJsonException(signed_json_failure_malformed); }
   } // initialise


   void SignedJson::generate_envelope(
      Csi::Json::Object &envelope,
      void const *content,
      size_t content_len,
      Json::Array *supported_algorithms)
   {
      // we need to determine what signature algorithm to use
      bool found(false);
      for(size_t i = 0; !found && i < available_algorithms_count; ++i)
      {
         available_algorithm_type const &algorithm(available_algorithms[i]);
         if(algorithm.hmac_id == hmac_id_none)
            throw SignedJsonException(signed_json_failure_unsupported_algorithm);
         if(supported_algorithms != 0)
         {
            for(Json::Array::iterator ai = supported_algorithms->begin(); !found && ai != supported_algorithms->end(); ++ai)
            {
               Json::StringHandle algorithm_name(*ai);
               if(algorithm_name->get_value() == algorithm.name)
                  found = true;
            }
         }
         else
            found = true;

         // if we found an algorithm, we will construct the envelope
         if(found)
         {
            StrAsc content_encoded;
            StrAsc signature_encoded;
            StrAsc nonce(Csi::make_guid());
            
            found = true;
            Base64::encode(content_encoded, content, content_len);
            envelope.set_property_str("content", content_encoded);
            envelope.set_property_str("signature_alg", algorithm.name);
            envelope.set_property_str("nonce", nonce);
            if(algorithm.hmac_id == hmac_id_sha1)
            {
               Sha1HmacDigest digest(algorithm.private_key.c_str(), algorithm.private_key.length());
               byte temp[Sha1HmacDigest::digest_size];
               StrAsc signature_encoded;
               digest.add(content_encoded.c_str(), content_encoded.length());
               digest.add(nonce.c_str(), nonce.length());
               memcpy(temp, digest.final(), Sha1HmacDigest::digest_size);
               Base64::encode(signature_encoded, temp, sizeof(temp));
               envelope.set_property_str("signature", signature_encoded);
            }
            else
               throw SignedJsonException(signed_json_failure_unsupported_algorithm);
         }
      }
   } // generate_envelope


   void SignedJson::generate_envelope(
      Json::Object &envelope,
      Json::Object const &content,
      Json::Array *supported_algorithms)
   {
      OStrAscStream temp;
      temp.imbue(StringLoader::make_locale(0));
      content.format(temp);
      generate_envelope(envelope, temp.c_str(), temp.length(), supported_algorithms);
   } // generate_envelope


   void SignedJson::generate_supported_algorithms(Json::Array &algorithms)
   {
      for(size_t i = 0; available_algorithms[i].hmac_id != hmac_id_none; ++i)
         algorithms.push_back(available_algorithms[i].name);
   } // generate_supported_algorithms
};


/* Csi.SignedJson.h

   Copyright (C) 2017, 2017 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Saturday 15 July 2017
   Last Change: Tuesday 26 September 2017
   Last Commit: $Date: 2018-02-21 18:00:05 -0600 (Wed, 21 Feb 2018) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_SignedJson_h
#define Csi_SignedJson_h
#include "Csi.Json.h"
#include "StrBin.h"


namespace Csi
{
   /**
    * Lists the reasons why an attempt to parse a signed JSON envelope can fail.
    */
   enum SignedJsonFailureType
   {
      signed_json_failure_malformed = 1,
      signed_json_failure_unsupported_algorithm,
      signed_json_failure_invalid_signature
   };


   /**
    * Defines the type of exception that can be thrown while attempting to process a signed
    * envelope.
    */
   class SignedJsonException: public std::exception
   {
   public:
      /**
       * @param failure_ Specifies the failure code.
       */
      SignedJsonException(SignedJsonFailureType failure_):
         failure(failure_)
      { }

      /**
       * @param other Specifies the exception to copy.
       */
      SignedJsonException(SignedJsonException const &other):
         failure(other.failure)
      { }

      /**
       * Copy operator
       *
       * @param other Specifies the exception to copy.
       */
      SignedJsonException &operator =(SignedJsonException const &other)
      {
         failure = other.failure;
         return *this;
      }

      /**
       * @return Returns a string that explains the failure code.
       */
      virtual char const *what() const throw();

      /**
       * @return Returns a string that explains the failure code
       */
      virtual char const *why() const
      { return what(); }

      /**
       * @return Returns the failure code.
       */
      SignedJsonFailureType get_failure() const
      { return failure; }
      
   private:
      /**
       * Specifies the failure.
       */
      SignedJsonFailureType failure;
   };

   
   /**
    * Defines an object that is able to read and create signed JSON documents.
    */
   class SignedJson
   {
   public:
      /**
       * Construct from an input stream.
       *
       * @param input Specifies the input stream.
       *
       * @throws Throws a SignedJsonException object.
       */
      SignedJson(std::istream &input)
      {
         Csi::Json::Object source;
         try
         {
            source.parse(input);
         }
         catch(std::exception &)
         { throw SignedJsonException(signed_json_failure_malformed); }
         initialise(source);
      }

      /**
       * Construct from a JSON envelope object.
       *
       * @param source Specifies the envelope JSON object.
       *
       * @throw Throws a SignedJsonException object.
       */
      SignedJson(Csi::Json::Object &source)
      {
         initialise(source);
      }

      /**
       * @return Returns the signature algorithm
       */
      StrAsc const &get_signature_algorithm() const
      { return signature_algorithm; }

      /**
       * @return Returns the signature.
       */
      StrBin const &get_signature() const
      { return signature; }

      /**
       * @return Returns the nonce value.
       */
      StrAsc const &get_nonce() const
      { return nonce; }
      
      /**
       * @return Returns the decoded content.
       */
      StrBin const &get_content() const
      { return content; }

      /**
       * Generates a signed envelope structure for the specified content and set of supported keys.
       *
       * @param content Specifies the start of the content to sign.
       *
       * @param content_len Specifies the number of bytes to sign.
       *
       * @param envelope Specifies the JSON structure to which the envelope parameters will be
       * written.
       *
       * @param supported_algorithms Specifies the list of algorithms that are supported.  If this
       * is specified as null (the default), the most secure known algorithm will be chosen.
       */
      static void generate_envelope(
         Csi::Json::Object &envelope,
         void const *content,
         size_t content_len,
         Json::Array *supported_algorithms = 0);

      /**
       * Generates a signed envelope using the specified JSON structure as the content.
       *
       * @param envelope Specifies the JSON structure to which the signed content will be written.
       *
       * @param content Specifies the JSON structure that will be signed as part of the content.
       *
       * @param supported_algorithms Specifies the list of algorithms that are supported.  If the
       * default is specified (null), the most secure known algorithm will be chosen.
       */
      static void generate_envelope(
         Json::Object &envelope,
         Json::Object const &content,
         Json::Array *supported_algorithms = 0);

      /**
       * Generates an array of supported algorithm names.
       *
       * @param algorithms Specifies the JSON array that will receive the list of names.
       */
      static void generate_supported_algorithms(Csi::Json::Array &algorithms);
      
   private:
      /**
       * Initialises this structure from the specified JSON object.
       *
       * @param source Specifies the JSON structure that defines the signed envelope.
       *
       * @throws std::exception Will throw a std::exception or std::invalid_argument derived object
       * if the envelope structure is wrong.
       */
      void initialise(Csi::Json::Object &source);
      
   private:
      /**
       * Specifies the signature algorithm that was selected.
       */
      StrAsc signature_algorithm;

      /**
       * Specifies the signature of the content.
       */
      StrBin signature;

      /**
       * Specifies the content.
       */
      StrBin content;

      /**
       * Specifies the nonce value from the envelope.
       */
      StrAsc nonce;
   };
};


#endif

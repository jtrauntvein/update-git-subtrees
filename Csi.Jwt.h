/* Csi.Jwt.h

   Copyright (C) 2020, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Wednesday 23 September 2020
   Last Change: Thursday 08 October 2020
   Last Commit: $Date: 2020-11-11 09:27:11 -0600 (Wed, 11 Nov 2020) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_Jwt_h
#define Csi_Jwt_h

#include "Csi.Json.h"


namespace Csi
{
   /**
    * Defines an object that can be used to encode and to decode JSON Web Tokens (JWT) that can be
    * used for HTTP authentication.
    */
   class Jwt
   {
   private:
      /**
       * Specifies the type of token as given in the header.
       */
      StrAsc type;

      /**
       * Specifies the algorithm as specified in the header.
       */
      StrAsc algorithm;

      /**
       * Specifies the Fields in the body.
       */
      Csi::Json::ObjectHandle body;

   public:
      /**
       * Default Constructor
       */
      Jwt()
      { body.bind(new Json::Object); }
       
      /**
       * Parses the token using the specified encoded string.
       *
       * @param encoded Specifies an encoded JWT string.
       */
      Jwt(StrAsc const &encoded)
      { parse(encoded); }

      /**
       * Assigns the tokens from the specified encoded string.
       *
       * @param encoded Specifies the encoded string.
       */
      Jwt &operator =(StrAsc const &encoded)
      {
         parse(encoded);
         return *this;
      }

      /**
       * Validates the specified token using the provided private key.
       *
       * @param encoded Specifies the encoded token.
       *
       * @param key Specifies the private key used to validate this token.
       */
      static bool validate(StrAsc const &token, StrAsc const &key);

      /**
       * @return Returns the token type reported in the header.
       */
      StrAsc const &get_type() const
      { return type; }

      /**
       * @return Returns the algorithm, reported in the header.
       */
      StrAsc const &get_algorithm() const
      { return algorithm; }
      
      /**
       * @return Returns the expiration time of the token.  If no expiration time is specified, a
       * value with the greatest positive range will be returned.
       */
      int8 get_expiration();

      /**
       * Sets the expiration date for this token.
       *
       * @param value Specifies the expiration date for this token as a unix epoch timestamp.
       */
      void set_expiration(int8 value);

      /**
       * @return Returns true if the current time is beyond the expiration date claim.
       */
      bool has_expired();

      /**
       * Sets the issued at and expiration date for the token.
       *
       * @param issued_at Specifies the date and time for when the token was issued as a unix epoch
       * (seconds since 1970 GMT).
       *
       * @param expiration_interval Specifies the interval, in seconds, for which the token will be
       * valid.  This value will be added to issued_at in order to set the expiration date.
       */
      void set_issued_at(int8 issued_at, int8 expiration_interval);

      /**
       * @return Returns the issued at date.
       */
      int8 get_issued_at();

      /**
       * @return Returns a string that represents the claim value associated with the given claim
       * name.
       *
       * @param claim_name Specifies the claim to look up.
       */
      StrAsc get_claim(StrAsc const &claim_name);

      /**
       * @return Returns the body of the JWT.
       */
      Json::ObjectHandle &get_body()
      { return body; }

      /**
       * @return Returns the the encoded JWT.
       *
       * @param password Specifies the password used to sign the JWT.  If specified as an empty
       * string, the JWT will be unsigned.
       */
      StrAsc encode(StrAsc const &password);
      
   private:
      /**
       * Parses the encoded string.
       *
       * @param encoded Specifies the encoded string.
       */
      void parse(StrAsc const &encoded);
   };
};


#endif

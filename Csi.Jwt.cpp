/* Csi.Jwt.cpp

   Copyright (C) 2020, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Wednesday 23 September 2020
   Last Change: Wednesday 11 November 2020
   Last Commit: $Date: 2020-11-11 09:27:11 -0600 (Wed, 11 Nov 2020) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Jwt.h"
#include "Csi.Base64.h"
#include "Csi.BuffStream.h"
#include "Csi.Digest.h"
#include "Csi.StrAscStream.h"


namespace Csi
{
   namespace
   {
      StrAsc const header_end_name("header_end");
      StrAsc const exp_name("exp");
      StrAsc const iat_name("iat");
      StrAsc const alg_name("alg");
      StrAsc const typ_name("typ");
      StrAsc const sha256_hmac_name("HS256");
      StrAsc const jwt_name("JWT");

      

      /**
       * @return Returns a base64-url encoded string for the specified buffer and length.
       *
       * @param buff Specifies the start of the buffer to encode.
       *
       * @param buff_len Specifies the number of bytes in the buffer.
       */
      StrAsc encode_base64_url(void const *buff, size_t buff_len)
      {
         StrAsc rtn;
         Base64::encode(rtn, buff, buff_len);
         rtn.replace("=", "");
         rtn.replace("+", "-");
         rtn.replace("/", "_");
         return rtn;
      }

      /**
       * @return Returns the content of the specified base64 url-encoded string.
       *
       * @param s Specifies the string to decode.
       */
      StrBin decode_base64_url(StrAsc const &s)
      {
         StrAsc temp(s);
         StrBin rtn;
         
         temp.replace("_", "/");
         temp.replace("-", "+");
         Base64::decode(rtn, temp.c_str(), temp.length());
         return rtn;
      } // decode_base64_url
      
      /**
       * @return Returns a string that is the base64 url encoded version of the specified JSON
       * structure.
       *
       * @param o Specifies the JSON object to format
       */
      StrAsc url_encode_json(Json::Object &o)
      {
         OStrAscStream temp;
         o.format(temp, false);
         return encode_base64_url(temp.c_str(), temp.length());
      } // url_encode_json
      
      /**
       * @return Returns a JSON object that represents the decoded header from the specified JWT.
       *
       * @param token Specifies the token string.
       */
      Json::ObjectHandle parse_header(StrAsc const &token)
      {
         // we will first pick off and decode the encoded header (evertyhing from the start to the
         // first period.
         size_t header_end(token.find("."));
         StrAsc header_encoded, header_decoded;
         StrBin buff;
         if(header_end >= token.length())
            throw std::invalid_argument("improperly encoded JWT header");
         token.sub(header_encoded, 0, header_end);
         buff = decode_base64_url(header_encoded);
         
         // We can now attempt to parse the decoded header buffer.
         Csi::IBuffStream temp(buff.getContents(), buff.length());
         Json::ObjectHandle rtn(new Json::Object);
         rtn->parse(temp);
         rtn->set_property_number(header_end_name, header_end);
         return rtn;
      } // parse_header
   };
   
   
   bool Jwt::validate(StrAsc const &token, StrAsc const &key)
   {
      // we need to parse the header and determine the algorithm
      bool rtn(true);
      try
      {
         auto header(parse_header(token));
         StrAsc alg(header->get_property_str(alg_name));
         if(alg != "none")
         {
            // we are going to need to separate out the encoded portion of the JWT so that we can
            // calculate its signature.
            size_t header_end(header->get_property_uint4(header_end_name));
            size_t body_end(token.find(".", header_end + 1));
            StrAsc reported_sig;
            token.sub(reported_sig, body_end + 1, token.length());
            
            // At this time, we will only support SHA256-HMAC digests
            if(alg == sha256_hmac_name)
            {
               StrAsc calculated;
               Sha256HmacDigest digest(key.c_str(), key.length());
               digest.add(token.c_str(), body_end);
               calculated = encode_base64_url(digest.final(), Sha256HmacDigest::digest_size);
               rtn = (reported_sig.compare(calculated, true) == 0);
            }
         }
      }
      catch(std::exception &)
      { rtn = false; }
      return rtn;
   } // validate

   int8 Jwt::get_expiration()
   {
      int8 rtn(Int8_Max);
      if(body->get_property_type(exp_name) == Json::value_number)
         rtn = body->get_property_int8(exp_name);
      return rtn;
   } // get_expiration

   void Jwt::set_expiration(int8 value)
   {
      body->set_property_number(exp_name, (double)value);
   } // set_expiration

   bool Jwt::has_expired()
   {
      int8 now(LgrDate::gmt().to_time_t(true));
      return now >= get_expiration();
   } // has_expired

   int8 Jwt::get_issued_at()
   {
      int8 rtn(Int8_Max);
      if(body->get_property_type(iat_name) == Json::value_number)
         rtn = body->get_property_int8(iat_name);
      return rtn;
   } // get_issued_at

   void Jwt::set_issued_at(int8 issued_at, int8 expiration_interval)
   {
      body->set_property_number(iat_name, (double)issued_at);
      if(expiration_interval > 0)
         body->set_property_number(exp_name, (double)(issued_at + expiration_interval));
   } // set_issued_at

   StrAsc Jwt::get_claim(StrAsc const &claim)
   {
      StrAsc rtn;
      if(body->get_property_type(claim) == Json::value_string)
         rtn = body->get_property_str(claim);
      return rtn;
   } // get_claim
   
   StrAsc Jwt::encode(StrAsc const &password)
   {
      // we need to format the header
      StrAsc rtn;
      Json::Object header;
      if(password.length() == 0)
         header.set_property_str(alg_name, "none");
      else
      {
         header.set_property_str(alg_name, sha256_hmac_name);
         header.set_property_str(typ_name, jwt_name);
      }
      rtn = url_encode_json(header);
      rtn.append('.');
      rtn.append(url_encode_json(*body));
      if(password.length() > 0)
      {
         Csi::OStrAscStream temp;
         Sha256HmacDigest digest(password.c_str(), password.length());
         digest.add(rtn.c_str(), rtn.length());
         rtn.append('.');
         rtn.append(encode_base64_url(digest.final(), Sha256HmacDigest::digest_size));
      }
      return rtn;
   } // encode

   void Jwt::parse(StrAsc const &token)
   {
      // we will first parse the header
      auto header_json(parse_header(token));
      algorithm = header_json->get_property_str(alg_name);
      if(header_json->has_property(typ_name))
         type = header_json->get_property_str(typ_name);
      else if(algorithm != "none")
         throw std::invalid_argument("type not specified when required");

      // we can now attempt to separate the body
      size_t header_end(header_json->get_property_uint4(header_end_name));
      size_t body_end(token.find(".", header_end + 1));
      StrAsc body_encoded, body_decoded;
      StrBin buff;
      token.sub(body_encoded, header_end + 1, body_end - header_end - 1);
      buff = decode_base64_url(body_encoded);

      // finally, we can attempt to parse the body.
      IBuffStream body_in(buff.getContents(), buff.length());
      body.bind(new Json::Object);
      body->parse(body_in);
   } // parse
};

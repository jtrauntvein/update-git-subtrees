/* Csi.ClaimCode.cpp

   Copyright (C) 2020, 2021 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Wednesday 11 March 2020
   Last Change: Thursday 01 April 2021
   Last Commit: $Date: 2021-04-01 13:54:02 -0600 (Thu, 01 Apr 2021) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.ClaimCode.h"
#include "Csi.Utils.h"
#include "Csi.Base32.h"
#include "Csi.ByteOrder.h"
#include "Cora.LgrNet.Defs.h"
#include <map>
#include <cctype>


namespace Csi
{
   namespace
   {
      // Specifies the collection of known models mapped by their signature.
      typedef std::map<uint2, ClaimCode::model_type> known_models_type;
      known_models_type known_models;


      // Specifies the private key used to verify the claim code for algorithm 0.
      StrAsc const claim_private_key("fe3b3942-63e1-11ea-bc55-0242ac130003");


      /**
       * Initialises the collection of known products that have serial numbers.
       */
      void init_known_models()
      {
         ClaimCode::models_type models{
            { "CR1000", Cora::LgrNet::DeviceTypes::cr1000 },
            { "CR800", Cora::LgrNet::DeviceTypes::cr800 },
            { "CR3000", Cora::LgrNet::DeviceTypes::cr3000 },
            { "CR200", Cora::LgrNet::DeviceTypes::cr2xx },
            { "CR6", Cora::LgrNet::DeviceTypes::cr6 },
            { "CR300", Cora::LgrNet::DeviceTypes::cr300 },
            { "CR310", Cora::LgrNet::DeviceTypes::cr300 },
            { "CR1000X", Cora::LgrNet::DeviceTypes::cr1000x },
            { "GRANITE6", Cora::LgrNet::DeviceTypes::granite_6 },
            { "GRANITE9", Cora::LgrNet::DeviceTypes::granite_9 },
            { "GRANITE10", Cora::LgrNet::DeviceTypes::granite_10 },
            { "CRVW3", Cora::LgrNet::DeviceTypes::crvw },
            { "NL200", Cora::LgrNet::DeviceTypes::other_pb_router },
            { "NL201", Cora::LgrNet::DeviceTypes::other_pb_router },
            { "NL240", Cora::LgrNet::DeviceTypes::other_pb_router },
            { "NL241", Cora::LgrNet::DeviceTypes::other_pb_router },
            { "EFWS", Cora::LgrNet::DeviceTypes::unknown },
            { "EFWSL10", Cora::LgrNet::DeviceTypes::unknown }
         };
         for(auto mi = models.begin(); mi != models.end(); ++mi)
            known_models[calcSigFor(mi->first.c_str(), mi->first.length())] = *mi;
      } // init_known_models
   };


   void ClaimCode::parse(StrAsc const &network_id)
   {
      size_t dash_pos(network_id.rfind("-"));
      StrAsc model_str, serial_str;
      uint4 country(0);
      
      if(dash_pos >= network_id.length())
         throw std::invalid_argument("no serial number was specified");
      network_id.sub(model_str, 0, dash_pos);
      network_id.sub(serial_str, dash_pos + 1, network_id.length());
      if(model_str.length() == 0)
         throw std::invalid_argument("empty model string");
      if(serial_str.length() == 0)
         throw std::invalid_argument("empty serial string");
      if(!std::isdigit(serial_str.first()))
      {
         country = serial_str.first();
         serial_str.cut(0, 1);
      }
      model = model_str;
      serial_no = strtoul(serial_str.c_str(), 0, 10);
      if(country)
         serial_no |= (country << 24);
   } // parse
   
   
   void ClaimCode::decode(StrAsc const &claim_code)
   {
      // we need to copy the claim code and reduce it so any dashes are eliminated.  Once that is
      // done, we can attempt to decode string.
      StrAsc encoded(claim_code);
      StrBin decoded;
      char const *decoded_str;
      encoded.replace("-", "");
      Base32::decode(decoded, encoded.c_str(), encoded.length());
      decoded_str = decoded.getContents();
      
      // we can decode parts of the packet
      uint2 model_sig;
      uint4 encoded_serial;
      uint2 verify_alg;
      size_t const model_sig_pos(0);
      size_t const encoded_serial_pos(model_sig_pos + sizeof(model_sig));
      size_t const verify_alg_pos(encoded_serial_pos + sizeof(encoded_serial));
      size_t const min_required_size(sizeof(model_sig) + sizeof(encoded_serial) + sizeof(verify_alg));
      if(decoded.length() < min_required_size)
         throw std::invalid_argument("claim code is too short");
      memcpy(&model_sig, decoded_str + model_sig_pos, sizeof(model_sig));
      memcpy(&encoded_serial, decoded_str +  encoded_serial_pos, sizeof(encoded_serial));
      memcpy(&verify_alg, decoded_str + verify_alg_pos, sizeof(verify_alg));
      if(!is_big_endian())
      {
         reverse_byte_order(&model_sig, sizeof(model_sig));
         reverse_byte_order(&encoded_serial, sizeof(encoded_serial));
         reverse_byte_order(&verify_alg, sizeof(verify_alg));
      }

      // check to ensure if the model is known
      if(known_models.empty())
         init_known_models();
      auto mi(known_models.find(model_sig));
      if(mi == known_models.end())
         throw std::invalid_argument("unknown model was specified");

      // we need to check the validation
      if(verify_alg == 0)
      {
         // check the length and verify the signature.
         uint2 reported_sig;
         uint2 calc_sig(calcSigFor(decoded_str, verify_alg_pos));
         uint4 const reported_sig_pos(verify_alg_pos + sizeof(verify_alg));
         
         if(decoded.length() < min_required_size + sizeof(reported_sig))
            throw std::invalid_argument("claim code does not include validation");
         memcpy(&reported_sig, decoded_str + reported_sig_pos, sizeof(reported_sig));
         if(!is_big_endian())
            reverse_byte_order(&reported_sig, sizeof(reported_sig));
         calc_sig = calcSigFor(claim_private_key.c_str(), claim_private_key.length(), calc_sig);
         if(reported_sig != calc_sig)
            throw std::invalid_argument("claim code fails validation");
      }
      else
         throw std::invalid_argument("unsupported validation algorithm");

      // we can now assign the model and serial number
      model = mi->second.first;
      serial_no = encoded_serial;
   } // decode


   StrAsc ClaimCode::encode() const
   {
      // we need to generate the content buffer.
      StrBin content;
      uint2 model_sig(calcSigFor(model.c_str(), model.length()));
      uint4 encoded_serial(serial_no);
      uint2 content_sig;
      uint2 verify_alg(0);
      
      if(!is_big_endian())
      {
         reverse_byte_order(&model_sig, sizeof(model_sig));
         reverse_byte_order(&encoded_serial, sizeof(encoded_serial));
         reverse_byte_order(&verify_alg, sizeof(verify_alg));
      }
      content.append(&model_sig, sizeof(model_sig));
      content.append(&encoded_serial, sizeof(encoded_serial));
      content_sig = calcSigFor(content.getContents(), content.length());
      content_sig = calcSigFor(claim_private_key.c_str(), claim_private_key.length(), content_sig);
      if(!is_big_endian())
         reverse_byte_order(&content_sig, sizeof(content_sig));
      content.append(&verify_alg, sizeof(verify_alg));
      content.append(&content_sig, sizeof(content_sig));
      
      // we can now base32 encode the content and will add dashes for every four blocks.
      StrAsc rtn;
      uint4 insert_count(0);
      Base32::encode(rtn, content.getContents(), content.length());
      for(size_t i = 4; i + 4 < rtn.length(); i += 4)
         rtn.insert("-", i + insert_count++);
      return rtn;
   } // encode


   uint4 ClaimCode::get_lgrnet_device_type() const
   {
      uint4 rtn(0);
      uint2 model_sig(calcSigFor(model.c_str(), model.length()));
      if(known_models.empty())
         init_known_models();
      auto mi(known_models.find(model_sig));
      if(mi != known_models.end())
         rtn = mi->second.second;
      return rtn;
   } // get_lgrnet_device_type
   

   void ClaimCode::add_model(StrAsc const &model_, uint4 lgrnet_device_type)
   {
      StrAsc model(model_);
      uint2 model_sig;
      
      model.to_upper();
      model_sig = calcSigFor(model.c_str(), model.length());
      if(known_models.empty())
         init_known_models();
      known_models[model_sig] = std::make_pair(model, lgrnet_device_type);
   } // add_model


   void ClaimCode::list_models(models_type &models)
   {
      models.clear();
      if(known_models.empty())
         init_known_models();
      for(auto mi = known_models.begin(); mi != known_models.end(); ++mi)
         models.push_back(mi->second);
   } // list_models
};


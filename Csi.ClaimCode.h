/* Csi.ClaimCode.h

   Copyright (C) 2020, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Wednesday 11 March 2020
   Last Change: Friday 13 March 2020
   Last Commit: $Date: 2020-03-13 07:26:28 -0600 (Fri, 13 Mar 2020) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_ClaimCode_h
#define Csi_ClaimCode_h
#include "StrAsc.h"
#include <iostream>
#include <deque>
#include <utility>


namespace Csi
{
   /**
    * Defines an object that can format and parse a claim code.  A claim code is a signature
    * verified BASE32 encoding of a product model and serial number.
    */
   class ClaimCode
   {
   private:
      /**
       * Specifies the product model.
       */
      StrAsc model;

      /**
       * Specifies the product serial number and country code encoded in four bytes.
       */
      uint4 serial_no;
      
   public:
      /**
       * Construct with model and serial number.
       *
       * @param model_ Specifies the model.
       *
       * @param serial_no_ Specifies the serial number.
       *
       * @param country_code Specifies a single character country code.
       */
      ClaimCode(StrAsc const &model_, uint4 serial_no_, StrAsc const &country_code = ""):
         model(model_),
         serial_no(serial_no_)
      {
         if(country_code.length() > 0)
         {
            uint4 country_val = country_code.first();
            country_val <<= 24;
            serial_no |= country_val;
         }
      }

      /**
       * Default Constructor
       */
      ClaimCode():
         serial_no(0)
      { }

      /**
       * Construct from a claim code.
       *
       * @param claim_code Specifies the claim code to be parsed.
       */
      ClaimCode(StrAsc const &claim_code):
         serial_no(0)
      { decode(claim_code); }

      /**
       * Copy Constructor
       */
      ClaimCode(ClaimCode const &other):
         serial_no(other.serial_no),
         model(other.model)
      { }

      /**
       * Copy operator
       */
      ClaimCode &operator =(ClaimCode const &other)
      {
         model = other.model;
         serial_no = other.serial_no;
         return *this;
      }

      /**
       * Convert from string operator.
       *
       * @param claim_code
       */
      ClaimCode &operator =(StrAsc const &claim_code)
      {
         decode(claim_code);
         return *this;
      }

      /**
       * Parses the specified formatted network ID string (expects the model and serial number to be
       * separated by a dash) to extract the model and serial.
       */
      void parse(StrAsc const &network_id);

      /**
       * Parses the specified claim code to extract the model and serial number.
       *
       * @param claim_code Specifies the claim code to parse.
       */
      void decode(StrAsc const &claim_code);

      /**
       * @return Returns the encoded claim code.
       */
      StrAsc encode() const;

      /**
       * @return Returns the claim code model.
       */
      StrAsc const &get_model() const
      { return model; }

      /**
       * @return Returns the country code.
       */
      StrAsc get_country_code() const
      {
         StrAsc rtn;
         if((serial_no & 0xff000000) != 0)
            rtn.append(static_cast<char>(serial_no >> 24));
         return rtn;
      }

      /**
       * @return Returns the serial number only.
       */
      uint4 get_serial_no() const
      { return serial_no & 0x00ffffff; }

      /**
       * @return Returns the LoggerNet device type associated with the model.
       */
      uint4 get_lgrnet_device_type() const;
      
      /**
       * Adds a product model to the list of recognised models used when decoding the claim code.
       *
       * @param model Specifies the model to add.
       *
       * @param lgrnet_device_type Specifies the device type code that would be used to represent
       * the device in the network map.
       */
      static void add_model(StrAsc const &model, uint4 lgrnet_device_type = 0);

      /**
       * Retrieves the list of models from the known set.
       *
       * @param models Specifies the container to which the models will be written.
       */
      typedef std::pair<StrAsc, uint4> model_type;
      typedef std::deque<model_type> models_type;
      static void list_models(models_type &models);
   };


   inline std::ostream &operator <<(std::ostream &out, ClaimCode const &code)
   {
      out << code.get_model() << "-" << code.get_country_code() << code.get_serial_no();
      return out;
   }
   inline std::wostream &operator <<(std::wostream &out, ClaimCode const &code)
   {
      out << code.get_model() << L"-" << code.get_country_code() << code.get_serial_no();
      return out;
   }
};


#endif


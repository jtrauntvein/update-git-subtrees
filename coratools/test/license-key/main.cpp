/* main.cpp

   Copyright (C) 2020, 2021 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Monday 03 August 2020
   Last Change: Wednesday 17 February 2021
   Last Commit: $Date: 2021-02-19 09:13:53 -0600 (Fri, 19 Feb 2021) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.SoftwareLicense.h"
#include "Csi.CommandLine.h"
#include "Csi.Utils.h"
#include <iostream>


int main(int argc, char const *argv[])
{
   int rtn(0);
   enum action_type
   {
      action_encode,
      action_decode
   } action(action_decode);
   Csi::CommandLine parser;
   StrAsc const action_name("action");
   StrAsc const validate_name("validate");
   StrAsc const company_code_name("company-code");
   StrAsc const timestamp_name("timestamp");

   parser.add_expected_option(action_name);
   parser.add_expected_option(validate_name);
   parser.add_expected_option(company_code_name);
   parser.add_expected_option(timestamp_name);
   Csi::set_command_line(argc, argv);
   try
   {
      // decode the action first as that will guid what we are looking for on the command line
      StrAsc temp;
      parser.parse_command_line(Csi::get_command_line());
      if(parser.get_option_value(action_name, temp))
      {
         if(temp == "encode")
            action = action_encode;
         else if(temp == "decode")
            action = action_decode;
         else
            throw std::invalid_argument("invalid action specified");
      }
      if(action == action_encode)
      {
         // decode the model, serial, and version from the command line
         StrAsc model, serial_no_str, version, company_code, timestamp_str;
         Csi::LgrDate timestamp;
         
         if(!parser.get_argument(model, 1))
            throw std::invalid_argument("expected the model");
         if(!parser.get_argument(serial_no_str, 2))
            throw std::invalid_argument("expected the serial number");
         parser.get_argument(version, 3);
         parser.get_option_value(company_code_name, company_code);
         parser.get_option_value(timestamp_name, timestamp_str);
         if(timestamp_str.length())
            timestamp = Csi::LgrDate::fromStr(timestamp_str.c_str());
         else
            timestamp = Csi::LgrDate::gmt();

         // encode the license key
         Csi::SoftwareLicense::LicenseKey key(
            model, company_code.first(), strtoul(serial_no_str.c_str(), 0, 10), timestamp);
         std::cout << key.generate_key(version) << "\n";
      }
      else if(action == action_decode)
      {
         // parse the command line arguments
         StrAsc key_str, version;
         bool validate(false);
         if(!parser.get_argument(key_str, 1))
            throw std::invalid_argument("expected the key string");
         if(parser.get_option_value(validate_name, temp))
         {
            if(temp == "true" || temp == "1")
               validate = true;
            else if(temp == "false" || temp == "0")
               validate = false;
            else
               throw std::invalid_argument("invalid validate value");
         }
         parser.get_argument(version, 2);

         // we can now attempt to decode the key
         Csi::SoftwareLicense::LicenseKey key(key_str, version, !validate);
         std::cout << "  model: " << key.get_model() << "\n"
                   << "  serial: " << key.get_company_code() << key.get_serial_no(false) << "\n";
         key.get_time_stamp().format(std::cout, "  stamp: %Y-%m-%dT%H:%M:%S%x\n");
      }
   }
   catch(std::exception &e)
   {
      std::cout << "uncaught exception: " << e.what() << std::endl;
      rtn = 1;
   }
   return rtn;
}

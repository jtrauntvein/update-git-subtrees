/* main.cpp

   Copyright (C) 2020, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Thursday 12 March 2020
   Last Change: Thursday 12 March 2020
   Last Commit: $Date: 2020-03-12 14:19:00 -0600 (Thu, 12 Mar 2020) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.CommandLine.h"
#include "Csi.ClaimCode.h"
#include "Csi.Utils.h"
#include <iostream>


int main(int argc, char const *argv[])
{
   int rtn(0);
   try
   {
      Csi::CommandLine command_line;
      StrAsc action;
      
      Csi::set_command_line(argc, argv);
      command_line.add_expected_option("action");
      command_line.parse_command_line(Csi::get_command_line());
      if(!command_line.get_option_value("action", action))
         action = "encode";
      if(action == "encode")
      {
         // get the serial number and model from the command line.
         StrAsc model, serial, country_code;
         
         if(!command_line.get_argument(model, 1))
            throw std::invalid_argument("expected the model string");
         model.to_upper();
         if(!command_line.get_argument(serial, 2))
            throw std::invalid_argument("expected the serial number");
         command_line.get_argument(country_code, 3);
         country_code.to_upper();

         // we can now take these arguments and try to encode the claim code.
         Csi::ClaimCode claim_code(model, strtoul(serial.c_str(), 0, 10), country_code);
         std::cout << claim_code.encode() << std::endl;
      }
      else if(action == "decode")
      {
         StrAsc claim;
         Csi::ClaimCode claim_code;
         
         if(!command_line.get_argument(claim, 1))
            throw std::invalid_argument("expected the claim code");
         claim_code = claim;
         std::cout << claim_code << std::endl;
      }
      else if(action == "parse")
      {
         StrAsc network_id;
         Csi::ClaimCode claim_code;
         
         if(!command_line.get_argument(network_id, 1))
            throw std::invalid_argument("network identifier was expected");
         claim_code.parse(network_id);
         std::cout << claim_code.encode() << std::endl;
      }
      else
         throw std::invalid_argument("invalid action specified");
   }
   catch(std::exception &e)
   {
      std::cout << "unhandled exception: " << e.what() << std::endl;
      rtn = 1;
   }
   return rtn;
} // main

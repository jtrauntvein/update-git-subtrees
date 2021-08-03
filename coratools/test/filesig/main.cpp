/* main.cpp

   Copyright (C) 2014, Campbell Scientific, Inc

   Written by: Jon Trauntvein 
   Date Begun: Saturday 15 February 2014
   Last Change: Saturday 15 February 2014
   Last Commit: $Date:$
   Last Changed by: $Author:$

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Utils.h"
#include "Csi.CommandLine.h"
#include <iostream>
#include <iomanip>


int main(int argc, char const *argv[])
{
   int rtn(0);
   try
   {
      // we need to parse the command line
      Csi::CommandLine parser;
      bool hex_sig(false);
      StrAsc temp;
      
      parser.add_expected_option("hex");
      Csi::set_command_line(argc, argv);
      parser.parse_command_line(Csi::get_command_line());
      if(parser.get_option_value("hex", temp))
      {
         if(temp == "1" || temp == "true")
            hex_sig = true;
      }

      // we now need to read each file specified on the command line and calculate its signature.
      StrAsc file_name;
      for(size_t i = 1; i < parser.args_size(); ++i)
      {
         FILE *input;
         parser.get_argument(file_name, i);
         input = Csi::open_file(file_name, "rb");
         if(input)
         {
            uint2 file_sig = Csi::calc_file_sig(input, Csi::long_file_length(input));
            std::cout << "\"" << file_name << "\" ";
            if(hex_sig)
               std::cout << std::hex;
            std::cout << file_sig << std::endl;
            fclose(input);
         }
         else
         {
            Csi::OsException error("open file failed");
            std::cerr << "\"" << error.what() << "\"" << std::endl;
         }
      }
   }
   catch(std::exception &e)
   {
      std::cerr << "Uncaught exception: " << e.what() << std::endl;
      rtn = -1;
   }
   return rtn;
} // main

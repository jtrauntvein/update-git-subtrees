/* main.cpp

   Copyright (C) 2018, Campbell Scientific, Inc

   Written by: Jon Trauntvein 
   Date Begun: Monday 26 November 2018
   Last Change: Monday 26 November 2018
   Last Commit: $Date: 2018-11-27 11:18:29 -0600 (Tue, 27 Nov 2018) $
   Last Changed by: $Author: jon $

*/

#include "Csi.Units.h"
#include "Csi.CommandLine.h"
#include "Csi.Utils.h"
#include "CsiTypes.h"
#include <iostream>


int main(int argc, char const *argv[])
{
   int rtn(0);
   Csi::CommandLine parser;
   StrUni source_unit;
   StrUni dest_unit;
   double value, result;
   Csi::Units::Converter converter;
   
   try
   {
      Csi::set_command_line(argc, argv);
      parser.parse_command_line(Csi::get_command_line());
      for(size_t i = 1; i < parser.args_size(); i += 3)
      {
         // pick off the arguments.
         source_unit = parser[i];
         dest_unit = parser[i + 1];
         value = csiStringToFloat(parser[i + 2].c_str());
         result = converter.convert(value, source_unit, dest_unit);
         std::cout << value << " " << source_unit << " -> " << result << " " << dest_unit << "\n";
      }
   }
   catch(std::exception &e)
   {
      std::cout << "uncaught exception: " << e.what() << std::endl;
      rtn = 1;
   }
   return rtn;
} // main


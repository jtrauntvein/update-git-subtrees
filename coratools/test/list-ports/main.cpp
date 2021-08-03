/* main.cpp

   Copyright (C) 2015, Campbell Scientific, Inc

   Written by: Jon Trauntvein 
   Date Begun: Thursday 16 July 2015
   Last Change: Thursday 16 July 2015
   Last Commit: $Date:$
   Last Changed by: $Author:$

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.SerialPortBase.h"
#include <iostream>


int main(int argc, char const *argv[])
{
   int rtn(0);
   try
   {
      typedef Csi::SerialPortBase port_type;
      port_type::friendly_names_type ports;
      port_type::list_ports_friendly(ports);
      for(auto pi = ports.begin(); pi != ports.end(); ++pi)
         std::cout << pi->first << ": \"" << pi->second << "\"" << std::endl;
   }
   catch(std::exception &e)
   {
      std::cout << "unhandled exception: " << e.what() << std::endl;
      rtn = 1;
   }
   return rtn;
} // main

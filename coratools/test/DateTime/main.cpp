/* main.cpp

   Copyright (C) 2005, Campbell Scientific, Inc

   Written by: Jon Trauntvein
   Date Begun: Wednesday 27 April 2005
   Last Change: Wednesday 27 April 2005
   Last Commit: $Date: 2005/04/27 20:24:18 $ (UTC)
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.LgrDate.h"
#include <iostream>


int main()
{
   Csi::LgrDate::gmt().format(std::cout,"gmt: %c%x\n");
   Csi::LgrDate::local().format(std::cout,"local: %c%x\n");
   Csi::LgrDate::local_na().format(std::cout,"local_na: %c%x\n");
   return 0;
} // main

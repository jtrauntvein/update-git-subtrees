/* main.cpp

   Copyright (C) 2005, Campbell Scientific, Inc

   Written by: Jon Trauntvein
   Date Begun: Tuesday 02 August 2005
   Last Change: Tuesday 02 August 2005
   Last Commit: $Date: 2005/08/19 14:35:44 $ (UTC)
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "../../Csi.FileSystemObject.h"
#include <iostream>


int main()
{
   using namespace Csi;
   int rtn = 0;
   FileSystemObject f(".");
   FileSystemObject::children_type children;

   std::cout << "children of " << f.get_complete_name() << "\n";
   f.get_children(children,"*.cpp");
   while(!children.empty())
   {
      FileSystemObject &child = children.front();
      
      std::cout << child.get_complete_name() << "\t"
                << child.get_size() << "\t"
                << child.get_last_write_date() << "\n";
      children.pop_front();
   }
   return rtn;
} // main

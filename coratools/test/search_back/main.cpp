/* main.cpp

   Copyright (C) 2006, Campbell Scientific, Inc

   Written by: Jon Trauntvein
   Date Begun: Tuesday 05 September 2006
   Last Change: Tuesday 05 September 2006
   Last Commit: $Date:$
   Last Changed by: $Author:$

*/

#pragma warning(disable: 4996)
#pragma hdrstop               // stop creation of precompiled header
#include "../../Csi.Utils.h"


int main()
{
   int rtn = 0;
   FILE *test = fopen("../../Csi.Utils.cpp","rb");
   if(test)
   {
      char const test1[] = "search_file_backward(";
      char const test2[] = "#pragma warning(disable: 4996)";
      uint4 test1_rcd;
      uint4 test2_rcd;
      
      fseek(test,Csi::file_length(test),SEEK_SET);
      test1_rcd = Csi::search_file_backward(test,test1,sizeof(test1) - 1);
      test2_rcd = Csi::search_file_backward(test,test2,sizeof(test2) - 1);
      if(test1_rcd == 0 || test2_rcd == 0)
         rtn = 1;
   }
   else
      rtn = -1;
   return rtn;
} // main

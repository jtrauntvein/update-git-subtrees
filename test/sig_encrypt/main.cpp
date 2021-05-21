/* main.cpp

   Copyright (C) 2016, Campbell Scientific, Inc

   Written by: Jon Trauntvein 
   Date Begun: Tuesday 22 March 2016
   Last Change: Tuesday 22 March 2016
   Last Commit: $Date:$
   Last Changed by: $Author:$

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Utils.h"
#include <iostream>


int main(int argc, char const *argv[])
{
   int rtn(0);
   try
   {
      StrBin contents;
      StrBin encrypted;
      
      for(int i = 1; i < argc; ++i)
      {
         // open the input file and read its contents.
         FILE *input(Csi::open_file(argv[i], "rb"));
         uint4 file_len;
         if(input == 0)
            throw Csi::OsException("input file open failed");
         file_len = Csi::file_length(input);
         contents.cut(0);
         contents.readFile(input, file_len);
         fclose(input);

         // we will encrypt the content that we read above.
         encrypted.cut(0);
         Csi::encrypt_sig(encrypted, contents.getContents(), contents.length());

         // we can now overwrite the file to write the encrypted output.
         FILE *output(Csi::open_file(argv[i], "wb"));
         if(!output)
            throw Csi::OsException("output file open failed");
         Csi::efwrite(encrypted.getContents(), encrypted.length(), 1, output);
         fclose(output);
      }
   }
   catch(std::exception &e)
   {
      std::cout << "encryption failed: \"" << e.what() << "\"" << std::endl;
      rtn = 1;
   }
   return rtn;
}


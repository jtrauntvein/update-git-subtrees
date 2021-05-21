#include "../../Csi.Base32.h"
#include <iostream>

int main()
{
   int rtn = 0;

   std::cout << "type 'quit' to exit" << std::endl << std::endl;

   std::string user_text;
   do
   {
      user_text = "";
      std::cout << "Enter a String:" << std::endl;
      std::cin >> user_text;

      StrAsc encoded_result;
      Csi::Base32::encode(encoded_result, user_text.c_str(), user_text.length());

      StrBin decoded_result;
      Csi::Base32::decode(decoded_result, encoded_result.c_str(), encoded_result.length());
      StrAsc const decoded_asc(decoded_result.getContents());

      std::cout << "Encoded String: " << encoded_result.c_str() << std::endl;
      std::cout << "Re-decoded String: " << decoded_asc.c_str() << std::endl;

      StrAsc user_asc(user_text.c_str(), user_text.length());
      if(decoded_asc.compare(user_asc, true) == 0)
      {
         std::cout << "TEST PASSED :)" << std::endl << std::endl;
      } 
      else
      {
         std::cout << "TEST FAILED :(" << std::endl << std::endl;
      }
      
   } while (user_text != "q" && user_text != "quit");

   return rtn;
} // main

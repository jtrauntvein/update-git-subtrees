/* main.cpp

   Copyright (C) 2015, Campbell Scientific, Inc

   Written by: Jon Trauntvein 
   Date Begun: Tuesday 27 January 2015
   Last Change: Tuesday 27 January 2015
   Last Commit: $Date: 2015-01-27 17:47:43 -0600 (Tue, 27 Jan 2015) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "StrAsc.h"
#include <iostream>
#include <deque>

struct test_type
{
   StrAsc haystack;
   StrAsc needle;
   int expected;
   bool case_sensitive;
};
std::deque<test_type> tests {
   { "Baud Rate", "bad", 1, false },
   { "Baud Rate RS232", "bad", 1, false }
};

int main(int argc, char const *argv[])
{
   int rtn(0);
   for(auto ti = tests.begin(); ti != tests.end(); ++ti)
   {
      auto &test(*ti);
      int rcd(test.haystack.fuzzy_find(test.needle.c_str(), test.case_sensitive));
      std::cout << "searching for \"" << test.needle << " in \"" << test.haystack << "\": " << rcd << std::endl; 
      if(rcd != test.expected)
         rtn = 1;
   }
   return rtn;
}

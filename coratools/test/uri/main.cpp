/* main.cpp

   Copyright (C) 2007, 2011 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Saturday 24 February 2007
   Last Change: Friday 17 June 2011
   Last Commit: $Date: 2011-06-17 17:14:26 -0600 (Fri, 17 Jun 2011) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "../../Csi.Uri.h"
#include <assert.h>


int main(int argc, char const *argv[])
{
   struct param_type
   {
      char const *name;
      char const *value;
   };
   struct test_case_type
   {
      char const *uri;
      char const *expected_protocol;
      char const *expected_address;
      uint2 expected_port;
      char const *expected_path;
      char const *expected_anchor;
      param_type const expected_params[10];
   } test_cases[] = {
      { "?cmd=delete-dev&id=2",  "http", "", 80, "", "",
        { { "cmd", "delete-dev" },
          { "id", "2" },
          { 0, 0 } } },
      { "http://engsoft", "http", "engsoft", 80, "", "", { { 0, 0 } } },
      { "http://engsoft/~jon", "http", "engsoft", 80, "~jon", "", { { 0, 0 } } },
      { "http://engsoft/~jon/index.html#anchor", "http", "engsoft", 80, "~jon/index.html", "anchor",
        { { 0, 0 } } },
      { "http://engsoft:8080/~jon", "http", "engsoft", 8080, "~jon", "", { { 0, 0 } } },
      { "engsoft/~jon", "http", "engsoft", 80, "~jon", "", { { 0, 0 } } },
      { "https://localhost/office-temp", "https", "localhost", 443, "office-temp", "", { { 0, 0 } } },
      { 0, 0, 0, 0, 0, 0, { { 0, 0 } } }
   };
   for(int i = 0; test_cases[i].uri != 0; ++i)
   {
      Csi::Uri test(test_cases[i].uri);
      uint4 params_count = 0;
      
      assert(test.get_protocol() == test_cases[i].expected_protocol);
      assert(test.get_server_address() == test_cases[i].expected_address);
      assert(test.get_server_port() == test_cases[i].expected_port);
      assert(test.get_path() == test_cases[i].expected_path);
      assert(test.get_anchor() == test_cases[i].expected_anchor);
      for(Csi::Uri::iterator ui = test.begin(); ui != test.end(); ++ui)
      {
         char const *param_name = test_cases[i].expected_params[params_count].name;
         char const *param_value = test_cases[i].expected_params[params_count].value;
         assert(param_name != 0);
         assert(param_value != 0);
         assert(ui->first == param_name);
         assert(ui->second == param_value);
         ++params_count;
      }
      assert(params_count == test.size());
   }
   return 0;
} // main

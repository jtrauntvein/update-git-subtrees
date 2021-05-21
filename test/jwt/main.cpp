/* main.cpp

   Copyright (C) 2020, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Wednesday 23 September 2020
   Last Change: Monday 05 October 2020
   Last Commit: $Date: 2020-10-06 09:15:27 -0600 (Tue, 06 Oct 2020) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Jwt.h"
#include <iostream>


namespace
{
   void format_jwt(StrAsc const &token)
   {
      Csi::Jwt parsed(token);
      std::cout << "token: " << token << "\n"
                << "type: " << parsed.get_type() << "\n"
                << "algorithm: " << parsed.get_algorithm() << "\n"
                << "body: \n";
      parsed.get_body()->format(std::cout, true);
      std::cout << std::endl;
   }
}

   
int main(int argc, char const *argv[])
{
   int rtn(0);
   try
   {
      // we need to parse some known tokens.
      StrAsc const tokens[] = {
         "eyJhbGciOiJub25lIn0.eyJpc3MiOiJqb2UiLA0KICJleHAiOjEzMDA4MTkzODAsDQogImh0dHA6Ly9leGFtcGxlLmNvbS9pc19yb290Ijp0cnVlfQ",
         "eyJ0eXAiOiJKV1QiLA0KICJhbGciOiJIUzI1NiJ9.eyJpc3MiOiJqb2UiLA0KICJleHAiOjEzMDA4MTkzODAsDQogImh0dHA6Ly9leGFtcGxlLmNvbS9pc19yb290Ijp0cnVlfQ.dBjftJeZ4CVP-mB92K27uhbUJU1p1r_wW1gFWFOEjXk",
         "eyJhbGciOiJIUzI1NiIsInR5cCIgOiAiSldUIiwia2lkIiA6ICI1YTM1ZjNmZC0zYTg1LTRjNDAtYjQwNS1iZGU2MWI5OGRkZmYifQ.eyJleHAiOjE2MDE5MjA5NjAsImlhdCI6MTYwMTkxOTE2MCwianRpIjoiN2ZlY2NkN2EtNGVjYi00MzExLTk0ZWEtN2JmMGMyY2QzMGY4IiwiaXNzIjoiaHR0cHM6Ly9hdXRoLmNhbXBiZWxsY2xvdWQueHl6L3JlYWxtcy9jYW1wYmVsbC1jbG91ZCIsImF1ZCI6Imh0dHBzOi8vYXV0aC5jYW1wYmVsbGNsb3VkLnh5ei9yZWFsbXMvY2FtcGJlbGwtY2xvdWQiLCJzdWIiOiIzZDhiYTFhZi00NWI0LTQ0OGQtODg1Zi1jNzM4M2M0ZGQxOTQiLCJ0eXAiOiJSZWZyZXNoIiwiYXpwIjoiYWRtaW4tY2xpIiwic2Vzc2lvbl9zdGF0ZSI6IjQ4YmZjOTk3LWIyNWUtNDg4NC1iNGExLWJjOWNmODI0NDM4YSIsInNjb3BlIjoiZW1haWwgcHJvZmlsZSB1c2VyLXNlc3Npb24taW5mbyJ9.57RwqchuufqYwDZAE8y1lgAgCgznbj872G-DeiHzOGI",
         "eyJhbGciOiJSUzI1NiIsInR5cCIgOiAiSldUIiwia2lkIiA6ICI1UG1tWkZ4emUzNWF4NlFXdWRXOHFkWjBpRHNqMDNfRm05eE1GZmNGdUY0In0.eyJleHAiOjE2MDE5MjcyNDcsImlhdCI6MTYwMTkyMzY0NywianRpIjoiYjZjODMyZDQtM2YwZS00ZDk1LWI5MDAtMDhkY2I1NDg3YWFhIiwiaXNzIjoiaHR0cHM6Ly9hdXRoLmNhbXBiZWxsY2xvdWQueHl6L3JlYWxtcy9jYW1wYmVsbC1jbG91ZCIsInN1YiI6IjNkOGJhMWFmLTQ1YjQtNDQ4ZC04ODVmLWM3MzgzYzRkZDE5NCIsInR5cCI6IkJlYXJlciIsImF6cCI6ImFkbWluLWNsaSIsInNlc3Npb25fc3RhdGUiOiI4MzU3MjJmMS0zNmJiLTQxMjItYjhiMy00ZTFkYmU5N2RkN2UiLCJhY3IiOiIxIiwic2NvcGUiOiJlbWFpbCBwcm9maWxlIHVzZXItc2Vzc2lvbi1pbmZvIiwiZW1haWxfdmVyaWZpZWQiOnRydWUsInByb2plY3RfaWQiOiJQVU0yRDlBOEVNIiwibGVnYWN5X2lkIjoiVUwyWlVOUFVGWSIsInByZWZlcnJlZF91c2VybmFtZSI6ImpvbkBjYW1wYmVsbHNjaS5jb20iLCJnaXZlbl9uYW1lIjoiIiwiZmFtaWx5X25hbWUiOiIiLCJlbWFpbCI6ImpvbkBjYW1wYmVsbHNjaS5jb20iLCJ1c2VybmFtZSI6ImpvbkBjYW1wYmVsbHNjaS5jb20ifQ.V_doUgSK3Sa0mYehTcJJymZQH29HeXeumgpAGtcGlS6pqRxFPvZ2HMvZKd7kR-dWJC86Qck2s32qikoOWwnUtzdimJbqu8orippID0S-ayxbxzi2VEo1UrQ1VjOgCl3FUQHxmjLfAMYEryeJAp8Ixbi5taWUi9cALJLbF_71f3UhLEGA_LahlnIMd5Hy1Kbo2ohrnWueggrHWfE-Jeh25tN8i_SifeIaCTr9GZRT4DgH8gOHu15G4n-Ehn5LP-ShX_axIpFDLPcC9wOwx6VXONoHYdKNCk8PeppnRvbs2npbfu5j3kIcJzCderT7fWbE1aT_Gx_-HUshqrNd_XMrHQ",
         ""
      };
      for(int i = 0; tokens[i].length() > 0; ++i)
         format_jwt(tokens[i]);
      
      // we need to test our own token generation and validation ability
      StrAsc const token_key("Numa folha qualquer Eu desenho um sol amarello");
      Csi::LgrDate expiration(Csi::LgrDate::fromStr("2020-11-01"));
      Csi::Jwt verified;
      StrAsc verified_encoded;
      verified.set_expiration(expiration.to_time_t());
      verified_encoded = verified.encode(token_key);
      assert(Csi::Jwt::validate(verified_encoded, token_key));
      Csi::Jwt parsed(verified_encoded);
      assert(parsed.get_expiration() == verified.get_expiration());
   }
   catch(std::exception &e)
   {
      std::cout << "uncaught exception: " << e.what() << std::endl;
      rtn = 1;
   }
   return rtn;
}

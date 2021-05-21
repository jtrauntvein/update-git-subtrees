/* Csi.TokenStringExpander.cpp

   Copyright (C) 2001, Campbell Scientific, Inc

   Written by: Jon Trauntvein
   Date Begun: Wednesday 30 May 2001
   Last Change: Wednesday 30 May 2001
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)

*/

#pragma warning(disable:4786) // disable overlong symbols warning
#pragma hdrstop               // stop creation of precompiled header
#include "Csi.TokenStringExpander.h"


namespace Csi
{
   ////////////////////////////////////////////////////////////
   // class TokenStringExpander definitions
   ////////////////////////////////////////////////////////////
   TokenStringExpander::TokenStringExpander()
   { }


   TokenStringExpander::~TokenStringExpander()
   { }


   void TokenStringExpander::add_expander(expander_handle &expander)
   { expanders.push_back(expander); }


   char const *TokenStringExpander::expand(
      StrAsc &destination,
      char const *source)
   {
      // initialise the destination buffer
      destination.cut(0);

      // work through the source one character at a time
      char const *s = source;
      size_t characters_used;

      while(s[0] != 0)
      {
         if(*s == '%')
         {
            expanders_type::iterator ei = expanders.begin();
            characters_used = 0;

            while(characters_used == 0 && ei != expanders.end())
               characters_used = (*ei)->expand(destination,s + 1);
            if(characters_used == 0)
               destination += '%';
            else
               s += characters_used;
         }
         else
            destination += *s;
         ++s;
      }
      return destination.c_str();
   } // expand
};

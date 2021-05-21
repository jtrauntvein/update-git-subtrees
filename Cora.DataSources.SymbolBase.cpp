/* Cora.DataSources.SymbolBase.cpp

   Copyright (C) 2009, 2014 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 13 February 2009
   Last Change: Friday 27 June 2014
   Last Commit: $Date: 2014-06-27 16:43:57 -0600 (Fri, 27 Jun 2014) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.DataSources.SymbolBase.h"
#include <algorithm>


namespace Cora
{
   namespace DataSources
   {
      namespace
      {
         ////////////////////////////////////////////////////////////
         // predicate symbol_has_name
         ////////////////////////////////////////////////////////////
         struct symbol_has_name
         {
            StrUni const &name;
            symbol_has_name(StrUni const &name_):
               name(name_)
            { }

            bool operator ()(SymbolBase::value_type const &s) const
            { return s->get_name() == name; }
         };
      };

      
      ////////////////////////////////////////////////////////////
      // class SymbolBase definitions
      ////////////////////////////////////////////////////////////
      SymbolBase::iterator SymbolBase::find(StrUni const &name)
      { return std::find_if(children.begin(), children.end(), symbol_has_name(name)); }


      SymbolBase::const_iterator SymbolBase::find(StrUni const &name) const
      { return std::find_if(children.begin(), children.end(), symbol_has_name(name)); }

      
      void SymbolBase::set_browser(SymbolBrowser *browser_)
      {
         browser = browser_;
         for(iterator ci = begin(); ci != end(); ++ci)
         {
            value_type &child = *ci;
            child->set_browser(browser);
         }
      } // set_browser
   };
};

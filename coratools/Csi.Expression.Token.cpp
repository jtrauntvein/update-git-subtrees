/* Csi.Expression.Token.cpp

   Copyright (C) 2020, Campbell Scientific, Inc

   Written by: Tyler Mecham
   Date Begun: 13 February 2020
   Last Change: Saturday 18 July 2020
   Last Commit: $Date:$
   Last Changed by: $Author:$

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Expression.Token.h"
#include "Cora.DataSources.SymbolBase.h"
#include "Cora.DataSources.SourceBase.h"
#include "Cora.DataSources.Manager.h"


namespace Csi
{
   namespace Expression
   {
      void Variable::generate_constructor_js(SharedPtr<Cora::DataSources::Manager> &sources, std::ostream &out)
      {
         if(get_request() != 0)
         {
            Cora::DataSources::SourceBase::symbols_type symbols;
            sources->breakdown_uri(symbols, get_request()->get_uri());
            if(!symbols.empty())
            {
               StrUni symbol_name = symbols.back().first;
               Csi::Json::format_string new_symbol_name(symbol_name);
               out << "new CsiVariable(\"" << new_symbol_name << "\"";
               if(symbols.back().second == Cora::DataSources::SymbolBase::type_table)
                  out << ", true)";
               else
                  out << ", false)";
            }
            else
               out << "null";
         }
         else
            out << "null";
      }
   };
};

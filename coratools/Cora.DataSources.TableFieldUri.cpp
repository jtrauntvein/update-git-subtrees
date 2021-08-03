/* Cora.DataSources.TableFieldUri.cpp

   Copyright (C) 2010, 2014 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 12 October 2010
   Last Change: Friday 27 June 2014
   Last Commit: $Date: 2014-06-27 16:43:57 -0600 (Fri, 27 Jun 2014) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.DataSources.TableFieldUri.h"
#include <list>


namespace Cora
{
   namespace DataSources
   {
      ////////////////////////////////////////////////////////////
      // class TableFieldUri definitions
      ////////////////////////////////////////////////////////////
      void TableFieldUri::parse(StrUni const &uri)
      {
         size_t source_end_pos = uri.rfind(L":");
         size_t start_pos = 0;
         if(uri.first() == '\"')
            start_pos = 1;
         source_name.cut(0);
         table_name.cut(0);
         column_name.cut(0);
         uri.sub(source_name, start_pos, source_end_pos - start_pos);
         if(source_name.last() == '\"')
            source_name.cut(source_name.length() - 1);
         if(source_end_pos < uri.length())
         {
            StrUni remnant;
            typedef std::list<StrUni> tokens_type;
            tokens_type tokens;
            StrUni token;
            
            uri.sub(remnant, source_end_pos + 1, uri.length());
            if(remnant.last() == '\"')
               remnant.cut(remnant.length() - 1);
            
            while(remnant.length() > 0)
            {
               size_t token_end_pos = remnant.rfind(L".");
               if(token_end_pos < remnant.length())
               {
                  remnant.sub(token, token_end_pos + 1, remnant.length());
                  tokens.push_front(token);
                  remnant.cut(token_end_pos);
               }
               else
               {
                  tokens.push_front(remnant);
                  remnant.cut(0);
               }
            }
            
            // we need to combine any tokens that may have been "commented"
            tokens_type::iterator ti = tokens.begin();
            while(ti != tokens.end())
            {
               if(ti->last() == '\\')
               {
                  tokens_type::iterator ti_next = ti;
                  if(++ti_next != tokens.end())
                  {
                     ti->cut(ti->length() - 1);
                     ti->append('.');
                     ti->append(*ti_next);
                     tokens.erase(ti_next);
                  }
                  ++ti;
               }
               else
                  ++ti;
            }
            
            // the data file uri should have, at most, two levels: table name, and column name
            if(!tokens.empty())
            {
               table_name = tokens.front();
               tokens.pop_front();
            }
            while(!tokens.empty())
            {
               column_name += tokens.front();
               tokens.pop_front();
            }
         }
      } // parse
   };
};

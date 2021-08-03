/* Cora.DataSources.TableFieldUri.h

   Copyright (C) 2010, 2014 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 12 October 2010
   Last Change: Friday 27 June 2014
   Last Commit: $Date: 2014-06-27 16:43:57 -0600 (Fri, 27 Jun 2014) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Cora_DataSources_TableFieldUri_h
#define Cora_DataSources_TableFieldUri_h

#include "Cora.DataSources.Request.h"


namespace Cora
{
   namespace DataSources
   {
      ////////////////////////////////////////////////////////////
      // class TableFieldUri
      //
      // Defines an object that can parse a source URI that incorporates
      // source, table name, and field name elements.  This type is shared in
      // common between database data file, and datalogger HTTP data sources. 
      ////////////////////////////////////////////////////////////
      class TableFieldUri: public WartBase
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         TableFieldUri(StrUni const &uri)
         { parse(uri); }

         ////////////////////////////////////////////////////////////
         // parse
         ////////////////////////////////////////////////////////////
         void parse(StrUni const &uri);

         ////////////////////////////////////////////////////////////
         // get_source_name
         ////////////////////////////////////////////////////////////
         StrUni const &get_source_name() const
         { return source_name; }
         
         ////////////////////////////////////////////////////////////
         // get_table_name
         ////////////////////////////////////////////////////////////
         StrUni const &get_table_name() const
         { return table_name; }
         
         ////////////////////////////////////////////////////////////
         // get_column_name
         ////////////////////////////////////////////////////////////
         StrUni const get_column_name() const
         { return column_name; }
         
      private:
         ////////////////////////////////////////////////////////////
         // source_name
         ////////////////////////////////////////////////////////////
         StrUni source_name;
         
         ////////////////////////////////////////////////////////////
         // table_name
         ////////////////////////////////////////////////////////////
         StrUni table_name;
         
         ////////////////////////////////////////////////////////////
         // column_name
         ////////////////////////////////////////////////////////////
         StrUni column_name;
      };
   };
};


#endif

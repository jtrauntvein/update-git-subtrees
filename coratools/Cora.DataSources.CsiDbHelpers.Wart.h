/* Cora.DataSources.CsiDbHelpers.Wart.h

   Copyright (C) 2009, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 30 April 2009
   Last Change: Monday 28 September 2009
   Last Commit: $Date: 2009-09-28 13:15:48 -0600 (Mon, 28 Sep 2009) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Cora_DataSources_CsiDbHelpers_Wart_h
#define Cora_DataSources_CsiDbHelpers_Wart_h

#include "Cora.DataSources.Request.h"
#include "StrAsc.h"


namespace Cora
{
   namespace DataSources
   {
      namespace CsiDbHelpers
      {
         ////////////////////////////////////////////////////////////
         // class Wart
         //
         // Defines the "wart" class for csidb based requests.
         ////////////////////////////////////////////////////////////
         class Wart: public WartBase
         {
         public:
            ////////////////////////////////////////////////////////////
            // table_name
            ////////////////////////////////////////////////////////////
            StrAsc const table_name;

            ////////////////////////////////////////////////////////////
            // column_name
            ////////////////////////////////////////////////////////////
            StrAsc const column_name;
            
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            Wart(StrAsc const &table_name_, StrAsc const &column_name_):
               table_name(table_name_),
               column_name(column_name_)
            { }

            
         };
      };
   };
};


#endif

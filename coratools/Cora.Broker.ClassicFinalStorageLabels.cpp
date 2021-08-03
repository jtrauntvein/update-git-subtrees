/* ClassicFinalStorageLabels.cpp

   Copyright (C) 2000, 2006 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 20 December 2006
   Last Change: Wednesday 20 December 2006
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ 
   Committed by: $Author: tmecham $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Broker.ClassicFinalStorageLabels.h"
#include "Cora.Broker.ClassicFinalStorageTable.h"
#include "CsiUtils.h"
#include <sstream>


namespace Cora
{
   namespace Broker
   {
      ////////////////////////////////////////////////////////////
      // class ClassicFinalStorageLabels definitions
      ////////////////////////////////////////////////////////////
      ClassicFinalStorageLabels::ClassicFinalStorageLabels()
      { }


      ClassicFinalStorageLabels::~ClassicFinalStorageLabels()
      { }


      void ClassicFinalStorageLabels::extract_from_dld(std::istream &in)
      {
         // scan for a comment line that contains the begin FSL mark (#)
         StrAsc line;
         StrAsc fsl_contents;
         enum state_type
         {
            state_before_block,
            state_in_block,
            state_complete
         } state = state_before_block;

         while(in && state != state_complete)
         {
            line.readLine(in);
            if(state == state_before_block)
            {
               if(line.length() >= 2 && line[0] == ';' && line[1] == '%')
                  state = state_in_block;
            }
            else if(state == state_in_block)
            {
               if(line.length() > 0 && line[0] == ';')
               {
                  if(line.length() >= 2 && line[1] == '%')
                     state = state_complete;
                  else
                  {
                     fsl_contents += line;
                     fsl_contents += "\n";
                  }
               }
               else
                  state = state_complete;
            }
         }

         // use the contents that were extracted to read the tables
         tables.clear();
         if(state == state_complete && fsl_contents.length() > 0)
         {
            std::istringstream temp(
               std::string(
                  fsl_contents.c_str(),
                  fsl_contents.length()));
            extract_from_fsl(temp);
         }
      } // extract_from_dld


      void ClassicFinalStorageLabels::extract_from_fsl(std::istream &in)
      {
         bool is_complete = false;
         while(!is_complete)
         {
            value_type table(new ClassicFinalStorageTable);
            if(table->extract(in))
               tables[table->get_array_id()] = table;
            else
               is_complete = true;
         }
      } // extract_from_fsl
   };
};

/* Csi.PakBus.Bmp5TableDef.cpp

   Copyright (C) 2014, 2015 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Tuesday 11 February 2014
   Last Change: Thursday 26 February 2015
   Last Commit: $Date: 2015-02-26 12:00:25 -0600 (Thu, 26 Feb 2015) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.PakBus.Bmp5TableDef.h"
#include "Csi.Utils.h"
#include "Csi.OsException.h"
#include "trace.h"


namespace Csi
{
   namespace PakBus
   {
      void read_table_defs(
         std::deque<Bmp5TableDef> &table_defs,
         StrUni const &tdf_file_name)
      {
         FILE *tdf_file = open_file(tdf_file_name, L"rb");
         if(tdf_file)
         {
            // we need to read the file contents into a buffer
            Bmp5Message message;
            message.addBytes(tdf_file, file_length(tdf_file));
            fclose(tdf_file);

            // we must now check the version number (the first byte)
            byte version(message.readByte());
            if(version == 1)
            {
               Bmp5TableDef table_def;
               table_defs.clear();
               while(message.whatsLeft() > 0)
               {
                  table_def.read(message);
                  table_defs.push_back(table_def);
               }
            }
            else
            {
               trace("invalid TDF version: %d", (int)version);
               throw std::invalid_argument("invalid TDF version");
            }
         }
         else
         {
            OsException e("tdf file open failed");
            trace(e.what());
            throw e;
         }
      }
   };
};

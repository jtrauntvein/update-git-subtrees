/* Csi.Alarms.AlarmLogger.cpp

   Copyright (C) 2012, 2013 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Wednesday 03 October 2012
   Last Change: Tuesday 18 June 2013
   Last Commit: $Date: 2013-06-18 09:04:28 -0600 (Tue, 18 Jun 2013) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Alarms.AlarmLogger.h"
#include "Csi.Alarms.Manager.h"
#include "Csi.Utils.h"


namespace Csi
{
   namespace Alarms
   {
      ////////////////////////////////////////////////////////////
      // class AlarmLogger definitions
      ////////////////////////////////////////////////////////////
      AlarmLogger::~AlarmLogger()
      { }


      void AlarmLogger::wr(LogRecord const &rec)
      {
         LogBaler::wr(rec);
         if(output != 0)
         {
            std::streampos pos(output->tellp());
            *output << "</alarm-log>\r\n";
            output->seekp(pos, std::ios_base::beg);
         }
      } // wr


      void AlarmLogger::open_output()
      {
         if(manager->get_was_started())
         {
            LogBaler::open_output();
            if(output != 0)
            {
               int8 output_size(output->size());
               if(output_size == 0)
               {
                  std::streampos pos;
                  *output << "<alarm-log>\r\n";
                  pos = output->tellp();
                  *output << "</alarm-log>\r\n";
                  output->seekp(pos, std::ios_base::beg);
               }
               else
               {
                  // data has already been written to the file.  We need to search for the position of
                  // the end tag in the file.  Since this is an output stream, however, we will open a
                  // regular file for input which will allow us to also use the search_file_backward()
                  // function.
                  FILE *temp(fopen(workFileName.c_str(), "rb"));
                  char const pattern[] = "</alarm-log>";
                  if(temp)
                  {
                     int8 end_tag_pos(0);
                     file_seek(temp, output_size, SEEK_SET);
                     end_tag_pos = search_file_backward(temp, pattern, sizeof(pattern) - 1);
                     if(end_tag_pos < output_size)
                        output->seekp(end_tag_pos, std::ios_base::beg);
                     fclose(temp); 
                  }
               }
            }
         }
      } // open_output
   };
};


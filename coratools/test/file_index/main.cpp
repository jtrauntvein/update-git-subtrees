/* main.cpp

   Copyright (C) 2014, 2014 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Wednesday 05 November 2014
   Last Change: Friday 07 November 2014
   Last Commit: $Date: 2014-11-07 16:55:18 -0600 (Fri, 07 Nov 2014) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Broker.DataFileReader.h"
#include <iostream>


int main(int argc, char const *argv[])
{
   using namespace Cora::Broker;
   int rtn(0);
   try
   {
      if(argc >= 1)
      {
         // we need to load the options
         StrAsc file_name(argv[1]);
         StrAsc fsl_name;

         if(argc >= 2)
            fsl_name = argv[2];
         Csi::SharedPtr<DataFileReader> reader(
            make_data_file_reader(file_name, 0, fsl_name));
         DataFileReader::index_type index;
         reader->generate_index(index);
         std::cout << "OFFSET\",\"TIMESTAMP\",\"RECORD\",\"ARRAYID\"" << std::endl;
         for(auto ei = index.begin(); ei != index.end(); ++ei)
            std::cout << ei->data_offset << ",\"" << ei->time_stamp << "\"," << ei->record_no
                      << "," << ei->array_id << std::endl;
      }
      else
      {
         rtn = 1;
         std::cout << "input file name required" << std::endl;
      }
   }
   catch(std::exception &e)
   {
      rtn = 2;
      std::cout << "unhandled exception caught: " << e.what() << std::endl;
   }
   return rtn;
}

/* main.cpp

   Copyright (C) 2016, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Thursday 07 July 2016
   Last Change: Thursday 28 July 2016
   Last Commit: $Date: 2020-01-09 12:02:32 -0600 (Thu, 09 Jan 2020) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Alert2.h"
#include "Csi.Utils.h"
#include "Csi.FileSystemObject.h"
#include <iostream>


class MyClient: public Csi::Alert2::IndStreamClient
{
public:
   virtual void on_message_content(Csi::Alert2::IndStream *sender, StrBin const &content)
   {
      std::cout.write(content.getContents(), content.length());
      std::cout << std::endl;
   }
   
   virtual uint4 on_message(Csi::Alert2::IndStream *sender, message_handle &message)
   {
      using namespace Csi::Alert2;
      uint4 rtn(0);
      if(message->get_message_type() == ind_message_airlink)
         std::cout << "Airlink message received" << std::endl;
      else if(message->get_message_type() == ind_message_mant)
      {
         Csi::LightPolySharedPtr<IndMessageBase, IndMessageMant> mant(message);
         auto pdu(mant->get_pdu());
         std::cout << "MANT message received\n"
                   << "  received: " << mant->get_received_time() << "\n"
                   << "  source: " << mant->get_source_address() << "\n"
                   << "  port: " << mant->get_service_port() << "\n";
         if(pdu != 0)
         {
            std::cout << "  pdu time: " << pdu->get_time_stamp() << "\n"
                      << "  apdu id: " << static_cast<uint4>(pdu->get_apdu_id()) << "\n"
                      << "  test: " << pdu->get_from_test() << "\n";
            for(auto ri = pdu->begin(); ri != pdu->end(); ++ri)
            {
               auto &report(*ri);
               std::cout << "  Report of Type: " << report->get_report_type() << "\n";
               for(auto vi = report->begin(); vi != report->end(); ++vi)
               {
                  auto &value(*vi);
                  std::cout << "    sensor id: " << value.get_sensor_id() << "\n"
                            << "    value type: " << value.get_value_type() << "\n"
                            << "    value: " << value.get_value_float() << "\n"
                            << "    timestamp: " << value.get_time_stamp() << "\n";
                  ++rtn;
               }
            }
         }
         std::cout << std::endl;
      }
      else if(message->get_message_type() == ind_message_concentration)
      {
         std::cout << "Concentration message received\n" << std::endl;
      }
      else if(message->get_message_type() == ind_message_status)
      {
         std::cout << "Status message received" << std::endl;
      }
      return rtn;
   } // on_message

   virtual void on_error(
      Csi::Alert2::IndStream *sender,
      StrAsc const &message,
      StrBin const &content)
   {
      std::cout << "Message decode error: " << message << "\n";
      std::cout.write(content.getContents(), content.length());
      std::cout << std::endl;
   }
};


int main(int argc, char const *argv[])
{
   int rtn(0);
   try
   {
      // we need to extract the list of files to process from the command line.
      std::deque<StrAsc> files;
      for(int i = 1; i < argc; ++i)
      {
         StrAsc file_name(argv[i]);
         if(file_name.find("*") < file_name.length() || file_name.find("?") < file_name.length())
         {
            // split the expression into a path and pattern.
            StrAsc dir;
            StrAsc pattern;
            Csi::split_path(&dir, &pattern, file_name);

            // now we can use the file system to expand the pattern.
            Csi::FileSystemObject dir_info(dir.c_str());
            Csi::FileSystemObject::children_type children;
            dir_info.get_children(children);
            for(auto ci = children.begin(); ci != children.end(); ++ci)
            {
               if(!ci->is_directory())
                  files.push_back(ci->get_complete_name().c_str());
            }
         }
         else
            files.push_back(file_name);
      }

      // we can now attempt to parse all of the files.
      MyClient client;
      Csi::Alert2::IndStream stream(&client);
      char buffer[1024];
      for(auto fi = files.begin(); fi != files.end(); ++fi)
      {
         FILE *input(Csi::open_file(fi->c_str(), "rb"));
         if(input)
         {
            uint4 bytes_read(fread(buffer, 1, sizeof(buffer), input));
            stream.clear();
            while(bytes_read)
            {
               stream.on_data(buffer, bytes_read);
               bytes_read = fread(buffer, 1, sizeof(buffer), input);
            }
         }
      }
   }
   catch(std::exception &e)
   {
      std::cout << "exception thrown: " << e.what() << std::endl;
      rtn = 1;
   }
} // main


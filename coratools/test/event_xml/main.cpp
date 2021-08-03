/* main.cpp

   Copyright (C) 2016, Campbell Scientific, Inc

   Written by: Jon Trauntvein 
   Date Begun: Thursday 03 March 2016
   Last Change: Thursday 03 March 2016
   Last Commit: $Date: 2016-03-03 15:24:10 -0600 (Thu, 03 Mar 2016) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Xml.EventParser.h"
#include "Csi.fstream.h"
#include "Csi.OsException.h"
#include <iostream>


int main(int argc, char const *argv[])
{
   int rtn(0);
   for(int i = 1; i < argc; ++i)
   {
      try
      {
         // open the input stream
         Csi::ifstream input(argv[i], std::ios::binary);
         std::cout << "Reading file: \"" << argv[i] << "\"" << std::endl;
         if(!input)
            throw Csi::OsException("file open failed");

         // we can now construct a parser and parse the content of the file
         using Csi::Xml::EventParser;
         EventParser parser;
         auto rcd(parser.parse(input));
         while(rcd != EventParser::parse_end_of_document)
         {
            switch(rcd)
            {
            case EventParser::parse_start_of_element:
               std::cout << "Start of Element: ns=" << parser.get_elem_namespace() << " name=" << parser.get_elem_name() << std::endl;
               break;
               
            case EventParser::parse_attribute_read:
               std::cout << "Attribute read: ns=" << parser.get_attr_namespace()
                         << " name=" << parser.get_attr_name()
                         << " value=" << parser.get_value() << std::endl;
               break;
               
            case EventParser::parse_end_of_element:
               std::cout << "End of Element: ns=" << parser.get_elem_namespace()
                         << " name=" << parser.get_elem_name() << " Content:\n"
                         << parser.get_value() << std::endl;
               break;
               
            case EventParser::parse_end_of_document:
               std::cout << "End of document: ns=" << parser.get_elem_namespace()
                         << " name=" << parser.get_elem_name() << " Content:\n"
                         << parser.get_value() << std::endl;
               break;
            }
            rcd = parser.parse(input);
         }
      }
      catch(std::exception &e)
      {
         std::cout << "unable to parse \"" << argv[i] << "\": " << e.what() << std::endl;
         ++rtn;
      }
   }
   return rtn;
}

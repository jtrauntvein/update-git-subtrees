/* main.cpp

   Copyright (C) 2021, 2021 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Tuesday 19 January 2021
   Last Change: Wednesday 20 January 2021
   Last Commit: $Date: 2021-01-20 15:39:21 -0600 (Wed, 20 Jan 2021) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.NmeaParser.h"
#include "test1.h"
#include <iostream>


class Tester: public Csi::NmeaParserClient
{
public:
   /**
    * Overloads the base class version to handle the sentence notification.
    */
   typedef Csi::NmeaParser parser_type;
   virtual void on_sentence(parser_type *sender, StrAsc const &sentence)
   {
      std::cout << sender->get_sentence_time() << ": " << sentence;
      if(sender->get_position_updated())
      {
         std::cout << "  stamp: " << sender->get_stamp() << "\n"
                   << "  latitude: " << sender->get_latitude() << "\n"
                   << "  longitude: " << sender->get_longitude() << "\n"
                   << "  altitude: " << sender->get_altitude() << "\n"
                   << "  horz dilution: " << sender->get_horizontal_dilution()
                   << "\n\n";
      }
      else if(sender->get_satellites_updated())
      {
         for(auto si = sender->begin(); si != sender->end(); ++si)
         {
            std::cout << "  id: " << si->get_id()
                      << "  elevation: " << si->get_elevation()
                      << "  azimuth: " << si->get_azimuth()
                      << "  snr: " << si->get_snr()
                      << "\n";
         }
         std::cout << "\n\n";
      }
   }
};


int main(int argc, char const *argv[])
{
   int rtn(0);
   try
   {
      Tester tester;
      Csi::NmeaParser parser(&tester);
      parser.on_data(Tests::test1_txt, sizeof(Tests::test1_txt));
   }
   catch(std::exception &e)
   {
      std::cout << "unhandled exception: " << e.what() << std::endl;
      rtn = 1;
   }
   return rtn;
} // main

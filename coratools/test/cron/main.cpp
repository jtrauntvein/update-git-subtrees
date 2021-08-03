/* main.cpp

   Copyright (C) 2012, Campbell Scientific, Inc

   Written by: Jon Trauntvein 
   Date Begun: Monday 12 November 2012
   Last Change: Monday 12 November 2012
   Last Commit: $Date: 2012-11-12 11:49:23 -0600 (Mon, 12 Nov 2012) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.CronPredictor.h"
#include "Csi.Xml.Element.h"
#include "Csi.BuffStream.h"
#include "Csi.OsException.h"
#include <iostream>
#include <fstream>


namespace
{
   template <class container>
   void parse_values(container &values, StrAsc const &s)
   {
      Csi::IBuffStream temp(s.c_str(), s.length());
      temp.imbue(std::locale::classic());
      values.insert(
         std::istream_iterator<uint4>(temp),
         std::istream_iterator<uint4>());
   } // parse_values
};


int main(int argc, char *argv[])
{
   int rtn(0);
   try
   {
      // parse the config file. as XML
      Csi::Xml::Element config(L"config");
      typedef Csi::CronPredictor::allowed_type allowed_type;
      allowed_type months;
      allowed_type days;
      allowed_type week_days;
      allowed_type hours;
      allowed_type minutes;
      Csi::LgrDate start_date(Csi::LgrDate::system());
      uint4 count(1440);
      int8 interval(1);
      
      if(argc < 2)
         throw std::invalid_argument("no test file specified");
      std::ifstream input(argv[1], std::ios::binary);
      if(!input)
         throw Csi::OsException("failed to open the config file");
      config.input(input);
      for(auto ci = config.begin(); ci != config.end(); ++ci)
      {
         auto &child(*ci);
         if(child->get_name() == L"months")
            parse_values(months, child->get_cdata_str());
         else if(child->get_name() == L"days")
            parse_values(days, child->get_cdata_str());
         else if(child->get_name() == L"weekdays")
            parse_values(week_days, child->get_cdata_str());
         else if(child->get_name() == L"hours")
            parse_values(hours, child->get_cdata_str());
         else if(child->get_name() == L"minutes")
            parse_values(minutes, child->get_cdata_str());
         else if(child->get_name() == L"start")
            start_date = Csi::LgrDate::fromStr(child->get_cdata_str().c_str());
         else if(child->get_name() == L"count")
            count = child->get_cdata_uint4();
         else if(child->get_name() == L"interval")
            interval = child->get_cdata_int8();
      }

      // we can now create the predictor
      Csi::CronPredictor predictor(months, days, week_days, hours, minutes);
      Csi::LgrDate value(start_date);
      for(uint4 i = 0; i < count; ++i)
      {
         Csi::LgrDate next(predictor.predict(value));
         std::cout << "next time for \"" << value << "\" is \"" << next << "\"" << std::endl;
         value += interval * Csi::LgrDate::nsecPerMin;
      }
   }
   catch(std::exception &e)
   {
      std::cerr << "unhandled exception: \"" << e.what() << "\"" << std::endl;
      rtn = -1;
   }  
   return rtn;
} // main

/* main.cpp

   Copyright (C) 2009, Campbell Scientific, Inc

   Written by: Jon Trauntvein
   Date Begun: Thursday 11 June 2009
   Last Change: Thursday 11 June 2009
   Last Commit: $Date: 2009-06-11 12:16:30 -0600 (Thu, 11 Jun 2009) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.LogBaler.h"
#include "Csi.CommandLine.h"
#include "Csi.Utils.h"
#include <iostream>


namespace
{
   ////////////////////////////////////////////////////////////
   // class MyLogRecord
   ////////////////////////////////////////////////////////////
   class MyLogRecord: public Csi::LogRecord
   {
   private:
      ////////////////////////////////////////////////////////////
      // stamp
      ////////////////////////////////////////////////////////////
      Csi::LgrDate const stamp;
      
   public:
      ////////////////////////////////////////////////////////////
      // constructor
      ////////////////////////////////////////////////////////////
      MyLogRecord():
         stamp(Csi::LgrDate::system())
      { }
      
      ////////////////////////////////////////////////////////////
      // format
      ////////////////////////////////////////////////////////////
      virtual void format(std::ostream &out) const
      {
         stamp.format(out, "\"%Y-%m-%d %H:%M:%S%x\",\"");
         for(int i = 0; i < 40; ++i)
            out << "AB";
         out << "\"\r\n";
      }

      ////////////////////////////////////////////////////////////
      // formatReq
      ////////////////////////////////////////////////////////////
      virtual uint4 formatReq() const
      { return 82 + 23; }
   };
};


int main(int argc, char const *argv[])
{
   // parse the command line
   int rtn = 0;
   try
   {
      // parse the command line arguments
      Csi::CommandLine arguments;
      Csi::set_command_line(argc, argv);
      arguments.parse_command_line(Csi::get_command_line());
      if(arguments.args_size() < 4)
         throw std::invalid_argument("invalid number of arguments");

      // create the log baler
      Csi::LogBaler log(arguments[1].c_str(), arguments[2].c_str());
      if(arguments[3] == "size")
      {
         uint4 max_size = log.get_baleSize();
         uint4 bale_count = log.get_baleCnt();
         if(arguments.args_size() >= 5)
            max_size = strtoul(arguments[4].c_str(), 0, 10);
         if(arguments.args_size() >= 6)
            bale_count = strtoul(arguments[5].c_str(), 0, 10);
         log.setBaleParams(max_size, bale_count);
      }
      else if(arguments[3] == "time")
      {
         int8 interval = 60000;
         if(arguments.args_size() >= 5)
            interval = strtoul(arguments[4].c_str(), 0, 10);
         log.set_time_based_baling(true, interval);
      }
      else
         throw std::invalid_argument("invalid baling option");

      // we can now enter a more-or-less infinite loop to output logs
      uint4 time_base = Csi::counter(0);
      log.setEnable(true);
      while(Csi::counter(time_base) < 5 * Csi::LgrDate::msecPerMin)
      {
         log.wr(MyLogRecord());
         Sleep(100);
      }
   }
   catch(std::exception &e)
   {
      std::cout << "unhandled exception: " << e.what() << std::endl;
      rtn = 1;
   }
   return rtn;
} // main

/* main.cpp

   Copyright (C) 2005, 2005 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 08 August 2005
   Last Change: Tuesday 09 August 2005
   Last Commit: $Date: 2005/08/19 14:35:44 $ (UTC)
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "../../Csi.SimpleDispatch.h"
#include "../../Scheduler.h"
#include <iostream>
#include <stdexcept>


namespace
{
   ////////////////////////////////////////////////////////////
   // class client_type
   ////////////////////////////////////////////////////////////
   class client_type: public SchedulerClient
   {
   public:
      ////////////////////////////////////////////////////////////
      // onScheduledEvent
      ////////////////////////////////////////////////////////////
      virtual void onScheduledEvent(uint4 event_id)
      {
         std::cout << Csi::LgrDate::system()
                   << ": Scheduler fired event " << event_id << std::endl;
      }
   };
};


int main()
{
   int rtn = 0;
   try
   {
      Csi::SimpleDispatch *dispatch = new Csi::SimpleDispatch;
      Csi::Event::set_dispatcher(dispatch);
      Scheduler scheduler;
      client_type client;
      uint4 sched_id = scheduler.start(&client,0,5000,true);

      while(true)
         dispatch->do_dispatch(); 
   }
   catch(std::exception &e)
   {
      std::cerr << e.what() << std::endl;
      rtn = 1;
   }
   return rtn;
} // main

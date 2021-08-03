/* Csi.Win32.AppExcluder.cpp

   Copyright (C) 2004, 2008 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 23 September 2004
   Last Change: Thursday 24 July 2008
   Last Commit: $Date: 2008-07-25 10:48:19 -0600 (Fri, 25 Jul 2008) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Win32.AppExcluder.h"
#include "Csi.Utils.h"
#include "Csi.StrAscStream.h"


namespace Csi
{
   namespace Win32
   {
      ////////////////////////////////////////////////////////////
      // class AppExcluder definitions
      ////////////////////////////////////////////////////////////
      uint4 const AppExcluder::comm_event_signalled =
      Csi::Event::registerType("Csi::Win32::AppExcluder::comm_event_signalled");

      
      AppExcluder::AppExcluder(
         EventReceiver *receiver_,
         StrAsc const &app_name_):
         app_name(app_name_),
         receiver(receiver_) 
      {
         if(app_name.length() == 0)
         {
            StrAsc app_path;
            get_name_from_file_path(app_name,get_program_path(app_path));
            app_name.cut(app_name.find("."));
            for(size_t i = 0; i < app_name.length(); ++i)
               app_name[i] = tolower(app_name[i]);
         }
      } // constructor

      
      AppExcluder::~AppExcluder()
      {
         wait_for_end();
         app_mutex.clear();
         comm_event.clear();
      } // destructor

      
      void AppExcluder::start()
      {
         should_stop = false;
         owns_mutex = false;
         start_wait_event.reset();
         Thread::start();
         start_wait_event.wait();
      } // start

      
      void AppExcluder::wait_for_end()
      {
         should_stop = true;
         if(comm_event != 0)
            comm_event->set();
         Thread::wait_for_end();
      } // wait_for_end

      
      void AppExcluder::execute()
      {
         // the name of the mutex and comm events will be based upon the app name
         OStrAscStream mutex_name;
         OStrAscStream comm_name;

         mutex_name << app_name << "_mutex";
         comm_name << app_name << "_comm";

         // we will try to create the mutex and claim ownership at the same time
         comm_event.bind(new Condition(comm_name.str().c_str()));
         app_mutex.bind(new Mutex(mutex_name.str().c_str()));
         if(app_mutex->try_lock())
            owns_mutex = true;
         else
         {
            owns_mutex = false;
            comm_event->set();
         }
                              
         // we will now enter a loop where we are waiting on the comm event to be signalled.  This
         // can be signalled by another app or by the wait_for_end() method when the thread must
         // close.
         start_wait_event.set();
         while(!should_stop && owns_mutex)
         {
            comm_event->reset();
            comm_event->wait();
            if(!should_stop && EventReceiver::is_valid_instance(receiver))
            {
               try
               {
                  Csi::Event *notification = Csi::Event::create(
                     comm_event_signalled,
                     receiver);
                  notification->post();
               }
               catch(Csi::Event::BadPost &)
               { }
            }
         }
      } // execute
   };
};


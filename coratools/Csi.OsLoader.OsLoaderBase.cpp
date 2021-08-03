/* Csi.OsLoader.OsLoaderBase.cpp

   Copyright (C) 2004, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 15 March 2004
   Last Change: Monday 12 October 2009
   Last Commit: $Date: 2014-11-11 13:01:41 -0600 (Tue, 11 Nov 2014) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.OsLoader.OsLoaderBase.h"
#include "coratools.strings.h"
#include <sstream>


namespace Csi
{
   namespace OsLoader
   {
      using namespace OsLoaderBaseStrings;

      
      ////////////////////////////////////////////////////////////
      // class OsLoaderBase definitions
      ////////////////////////////////////////////////////////////
      uint4 const OsLoaderStatusEvent::event_id =
      Event::registerType("Csi::OsLoaderBase::StatusEvent");
      
      
      uint4 const OsLoaderCompleteEvent::event_id =
      Event::registerType("Csi::OsLoaderBase::CompleteEvent");
      
      
      void OsLoaderBase::on_driver_error(
         OsLoaderDriver *driver,
         char const *error_message)
      {
         std::ostringstream temp;
         temp << my_strings[1] << error_message;
         on_complete(temp.str().c_str(),false);
      } // on_driver_error
      
      
      void OsLoaderBase::on_status(
         char const *status_message,
         uint4 bytes_sent,
         uint4 bytes_to_send)
      {
         if(EventReceiver::is_valid_instance(client))
            OsLoaderStatusEvent::cpost(
               client,
               this,
               status_message,
               bytes_sent,
               bytes_to_send); 
      } // on_status
      
      
      void OsLoaderBase::on_complete(
         char const *message,
         bool succeeded,
         bool display_elapsed_time)
      {
         if(EventReceiver::is_valid_instance(client))
            OsLoaderCompleteEvent::cpost(
               client,
               this,
               succeeded,
               message,
               display_elapsed_time);
         client = 0;
         driver.clear();
      } // on_complete
   };
};


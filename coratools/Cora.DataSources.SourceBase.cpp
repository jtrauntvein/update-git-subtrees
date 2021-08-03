/* Cora.DataSources.SourceBase.cpp

   Copyright (C) 2008, 2014 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 07 October 2008
   Last Change: Friday 27 June 2014
   Last Commit: $Date: 2014-06-27 16:43:57 -0600 (Fri, 27 Jun 2014) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.DataSources.SourceBase.h"
#include "Cora.DataSources.Manager.h"


namespace Cora
{
   namespace DataSources
   {
      ////////////////////////////////////////////////////////////
      // class SourceBase definitions
      ////////////////////////////////////////////////////////////
      StrUni const SourceBase::settings_name(L"settings");

      
      SourceBase::SourceBase(StrUni const &name_):
         manager(0),
         name(name_),
         was_started(false)
      {
         // remove any trailing or preceding whitespace
         while(isspace(name.first()))
            name.cut(0, 1);
         while(isspace(name.last()))
            name.cut(name.length() - 1);

         // ensure that the name has valid characters
         for(size_t i = 0; i < name.length(); ++i)
         {
            if(!isalpha(name[i]) && !isdigit(name[i]) && name[i] != '_')
               name[i] = '_';
         }
         
         // finally, we need to ensure that the source name is not empty
         if(name.length() == 0)
            throw std::invalid_argument("invalid source name");
      } // constructor


      void SourceBase::start()
      {
         was_started = true;
         if(manager != 0 && is_connected())
         {
            for(Manager::requests_iterator ri = manager->requests_begin(); ri != manager->requests_end(); ++ri)
            {
               Manager::request_handle &request = *ri;
               if(request->get_source() == this || request->get_source_name() == name)
               {
                  request->set_source(this);
                  add_request(request, true);
               }
            }
            activate_requests();
         }
      } // start


      void SourceBase::stop()
      {
         was_started = false;
         if(manager != 0)
         {
            for(Manager::requests_iterator ri = manager->requests_begin(); ri != manager->requests_end(); ++ri)
            {
               Manager::request_handle &request = *ri;
               if(request->get_source() == this)
                  remove_request(request);
            }
         }
      } // stop


      void SourceBase::get_table_range(
         Csi::EventReceiver *client,
         StrUni const &uri)
      {
         typedef GetTableRangeCompleteEvent event_type;
         event_type::cpost(client, uri, event_type::outcome_not_implemented);
      } // get_table_range


      void SourceBase::log_event(StrAsc const &message)
      { manager->report_source_log(this, message); }
   };
};



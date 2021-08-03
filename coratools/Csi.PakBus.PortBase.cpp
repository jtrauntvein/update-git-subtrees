/* Csi.PakBus.PortBase.cpp

   Copyright (C) 2005, Campbell Scientific, Inc

   Written by: Jon Trauntvein
   Date Begun: Saturday 02 April 2005
   Last Change: Saturday 02 April 2005
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)
   Last Changed by: $Author: tmecham $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.PakBus.PortBase.h"
#include "Csi.PakBus.Router.h"


namespace Csi
{
   namespace PakBus
   {
      ////////////////////////////////////////////////////////////
      // class PortBase definitions
      ////////////////////////////////////////////////////////////
      bool PortBase::should_process_message(
         uint2 physical_source,
         uint2 physical_destination,
         Router *router)
      {
         bool rtn = false;
         if(physical_source != router->get_this_node_address() &&
            can_accept_neighbour(physical_source) &&
            physical_source != 0)
         {
            if(physical_destination == Router::broadcast_address ||
               physical_destination == router->get_this_node_address())
               rtn = true;
         }
         return rtn;
      } // should_process_message
   };
};

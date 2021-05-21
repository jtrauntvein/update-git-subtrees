/* Csi.Alarms.ActionBase.cpp

   Copyright (C) 2012, Campbell Scientific, Inc

   Written by: Jon Trauntvein 
   Date Begun: Tuesday 02 October 2012
   Last Change: Tuesday 02 October 2012
   Last Commit: $Date: 2012-10-02 13:00:59 -0600 (Tue, 02 Oct 2012) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Alarms.ActionBase.h"
#include "Csi.Alarms.Alarm.h"


namespace Csi
{
   namespace Alarms
   {
      ////////////////////////////////////////////////////////////
      // class ActionBase definitions
      ////////////////////////////////////////////////////////////
      Alarm *ActionBase::get_alarm()
      { return get_condition()->get_alarm(); }


      void ActionBase::report_complete()
      { get_alarm()->on_action_complete(this); }
   };
};


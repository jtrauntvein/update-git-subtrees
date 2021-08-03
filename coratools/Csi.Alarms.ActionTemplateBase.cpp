/* Csi.Alarms.ActionTemplateBase.cpp

   Copyright (C) 2012, 2015 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Wednesday 26 September 2012
   Last Change: Tuesday 06 January 2015
   Last Commit: $Date: 2015-01-06 15:15:32 -0600 (Tue, 06 Jan 2015) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Alarms.ActionTemplateBase.h"
#include "Csi.Alarms.Manager.h"
#include "Csi.Alarms.ActionEmail.h"
#include "Csi.Alarms.ActionForward.h"
#include "Csi.Alarms.ActionExec.h"


namespace Csi
{
   namespace Alarms
   {
      ////////////////////////////////////////////////////////////
      // class ActionTemplateBase definitions
      ////////////////////////////////////////////////////////////
      ActionTemplateBase::ActionTemplateBase(Condition *condition_):
         condition(condition_),
         initial_delay(0),
         interval(0),
         delay_id(0),
         timer(condition->get_alarm()->get_manager()->get_timer())
      { }


      ActionTemplateBase::~ActionTemplateBase()
      {
         if(delay_id != 0)
            timer->disarm(delay_id);
      } // destructor
      
      
      ActionTemplateBase *ActionTemplateBase::make_template(
         Condition *condition, StrUni const &type_name)
      {
         ActionTemplateBase *rtn(0);
         if(type_name == L"email")
            rtn = new ActionEmailTemplate(condition);
         else if(type_name == L"forward")
            rtn = new ActionForwardTemplate(condition);
         else if(type_name == L"exec")
            rtn = new ActionExecTemplate(condition);
         return rtn;
      } // make_template


      namespace
      {
         StrUni const initial_delay_name(L"initial-delay");
         StrUni const interval_name(L"interval");
         StrUni const type_name_name(L"type");
      };
      

      void ActionTemplateBase::read(Xml::Element &elem)
      {
         if(elem.has_attribute(initial_delay_name))
            initial_delay = elem.get_attr_uint4(initial_delay_name);
         if(elem.has_attribute(interval_name))
            interval = elem.get_attr_uint4(interval_name);
      } // read


      void ActionTemplateBase::write(Xml::Element &elem)
      {
         elem.set_attr_wstr(get_type_name(), type_name_name);
         elem.set_attr_uint4(initial_delay, initial_delay_name);
         elem.set_attr_uint4(interval, interval_name);
      } // write


      void ActionTemplateBase::onOneShotFired(uint4 id)
      {
         if(id == delay_id)
         {
            delay_id = 0;
            try
            {
               perform_action();
            }
            catch(std::exception &)
            { }
            if(interval != 0)
               delay_id = timer->arm(this, interval);
         }
      } // onOneShotFired


      void ActionTemplateBase::on_alarm_on()
      {
         if(initial_delay != 0)
            delay_id = timer->arm(this, initial_delay);
         else
         {
            try
            {
               perform_action();
            }
            catch(std::exception &)
            { }
            if(interval != 0)
               delay_id = timer->arm(this, interval);
         }
      } // on_alarm_on


      void ActionTemplateBase::on_alarm_off()
      {
         if(delay_id != 0)
            timer->disarm(delay_id);
      } // on_alarm_off
   };
};



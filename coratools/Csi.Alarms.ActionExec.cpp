/* Csi.Alarms.ActionExec.cpp

   Copyright (C) 2012, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Tuesday 02 October 2012
   Last Change: Thursday 04 October 2012
   Last Commit: $Date: 2012-10-04 10:59:32 -0600 (Thu, 04 Oct 2012) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Alarms.ActionExec.h"
#include "Csi.Alarms.Manager.h"
#include "Csi.ProgramRunner.h"


namespace Csi
{
   namespace Alarms
   {
      ////////////////////////////////////////////////////////////
      // class ActionExec
      ////////////////////////////////////////////////////////////
      class ActionExec: public ActionBase, public EventReceiver
      {
      private:
         ////////////////////////////////////////////////////////////
         // path
         ////////////////////////////////////////////////////////////
         StrAsc path;

         ////////////////////////////////////////////////////////////
         // command_line
         ////////////////////////////////////////////////////////////
         StrAsc command_line;

         ////////////////////////////////////////////////////////////
         // show_mode
         ////////////////////////////////////////////////////////////
         int show_mode;

         ////////////////////////////////////////////////////////////
         // runner
         ////////////////////////////////////////////////////////////
         SharedPtr<ProgramRunner> runner;

         ////////////////////////////////////////////////////////////
         // last_error
         ////////////////////////////////////////////////////////////
         StrAsc last_error;

         ////////////////////////////////////////////////////////////
         // complete
         ////////////////////////////////////////////////////////////
         bool complete;
         
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         ActionExec(
            ActionTemplateBase *action_template,
            StrAsc const &path_,
            StrAsc const &command_line_,
            int show_mode_):
            ActionBase(action_template),
            path(path_),
            command_line(command_line_),
            show_mode(show_mode_),
            complete(false)
         { }

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~ActionExec()
         {
            runner.clear();
         }

         ////////////////////////////////////////////////////////////
         // execute
         ////////////////////////////////////////////////////////////
         virtual void execute()
         {
            Manager *manager(action_template->get_condition()->get_alarm()->get_manager());
            if(manager->get_exec_actions_allowed())
            {
               try
               {
                  runner.bind(
                     new ProgramRunner(
                        this,
                        path.c_str(),
                        command_line.c_str(),
                        UInt4_Max,
                        show_mode));
                  runner->start();
               }
               catch(std::exception &e)
               {
                  last_error = e.what();
                  report_complete();
               }
            }
            else
            {
               last_error = "program execute actions are not allowed";
               complete = true;
               report_complete();
            }
         }

         ////////////////////////////////////////////////////////////
         // get_last_error
         ////////////////////////////////////////////////////////////
         StrAsc const &get_last_error()
         { return last_error; }

         ////////////////////////////////////////////////////////////
         // describe_log
         ////////////////////////////////////////////////////////////
         virtual void describe_log(Xml::Element &elem)
         {
            elem.set_attr_str(path, L"path");
            elem.set_attr_str(command_line, L"command-line");
            if(complete)
            {
               Xml::Element::value_type outcome_xml(elem.add_element(L"outcome"));
               if(last_error.length() == 0)
                  outcome_xml->set_cdata_wstr(L"success");
               else
                  outcome_xml->set_cdata_str(last_error);
            }
         } // describe_log
         

         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(SharedPtr<Event> &ev)
         {
            typedef ProgramRunner::event_program_ended event_type;
            if(ev->getType() == event_type::event_id)
            {
               event_type *event(static_cast<event_type *>(ev.get_rep()));
               if(event->outcome != ProgramRunner::outcome_terminated_normally)
               {
                  switch(event->outcome)
                  {
                  case ProgramRunner::outcome_timed_out:
                     last_error = "timed out waiting for the process";
                     break;

                  case ProgramRunner::outcome_start_failed:
                     last_error = event->start_error;
                     break;

                  case ProgramRunner::outcome_aborted:
                     last_error = "aborted";
                     break;
                  }
               }
               complete = true;
               report_complete();
            }
         } // receive
      };

      
      ////////////////////////////////////////////////////////////
      // class ActionExecTemplate definitions
      ////////////////////////////////////////////////////////////
      ActionExecTemplate::ActionExecTemplate(Condition *condition):
         ActionTemplateBase(condition),
         show_mode(SW_SHOWNORMAL)
      { }


      ActionExecTemplate::~ActionExecTemplate()
      { }


      namespace
      {
         StrUni const path_name(L"path");
         StrUni const command_line_name(L"command-line");
         StrUni const show_mode_name(L"show-mode");
         StrUni const show_normal(L"normal");
         StrUni const show_minimised(L"minimised");
         StrUni const show_maximised(L"maximised");
      };

      
      void ActionExecTemplate::read(Xml::Element &elem)
      {
         ActionTemplateBase::read(elem);
         path = elem.find_elem(path_name)->get_cdata_str();
         command_line = elem.find_elem(command_line_name)->get_cdata_str();
         if(elem.has_attribute(show_mode_name))
         {
            StrUni temp(elem.get_attr_wstr(show_mode_name));
            if(temp == show_normal)
               show_mode = SW_SHOWNORMAL;
            else if(temp == show_minimised)
               show_mode = SW_SHOWMINIMIZED;
            else if(temp == show_maximised)
               show_mode = SW_SHOWMAXIMIZED;
         }
         else
            show_mode = SW_SHOWNORMAL;
      } // read


      void ActionExecTemplate::write(Xml::Element &elem)
      {
         ActionTemplateBase::write(elem);
         elem.add_element(path_name)->set_cdata_str(path);
         elem.add_element(command_line_name)->set_cdata_str(command_line);
         if(show_mode == SW_SHOWNORMAL)
            elem.set_attr_wstr(show_normal, show_mode_name);
         else if(show_mode == SW_SHOWMINIMIZED)
            elem.set_attr_wstr(show_minimised, show_mode_name);
         else if(show_mode == SW_SHOWMAXIMIZED)
            elem.set_attr_wstr(show_maximised, show_mode_name);
      } // write


      void ActionExecTemplate::perform_action()
      {
         condition->get_alarm()->add_action(
            new ActionExec(this, path, command_line, show_mode));
      } // perform_action
   };
};


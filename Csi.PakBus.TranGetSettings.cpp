/* Csi.PakBus.TranGetSettings.cpp

   Copyright (C) 2002, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 08 May 2002
   Last Change: Friday 30 November 2012
   Last Commit: $Date: 2016-07-28 08:37:14 -0600 (Thu, 28 Jul 2016) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.PakBus.TranGetSettings.h"
#include "Csi.PakBus.Defs.h"
#include "Csi.PakBus.PakCtrlMessage.h"
#include "Csi.StrAscStream.h"
#include "Csi.PakBus.Router.h"


namespace Csi
{
   namespace PakBus
   {
      ////////////////////////////////////////////////////////////
      // class TranGetSettings definitions
      ////////////////////////////////////////////////////////////
      TranGetSettings::TranGetSettings(
         Router *router,
         timer_handle &timer,
         priority_type priority,
         uint2 destination_address,
         client_type *client_,
         StrAsc const &setting_names_):
         PakBusTran(router,timer,priority,destination_address),
         client(client_),
         setting_names(setting_names_),
         retry_count(0)
      { }

      
      
      TranGetSettings::~TranGetSettings()
      { }


      void TranGetSettings::start()
      {
         OStrAscStream temp;
         temp << "PakBus get string settings: " << get_destination_address();
         report_id = router->add_report(temp.str(), priority, "requesting focus");
         request_focus();
      } // start

      
      void TranGetSettings::on_focus_start()
      {
         if(client_type::is_valid_instance(client))
         {
            router->set_report_state(report_id, "sending command");
            send_command();
         }
         else
         {
            router->set_report_state(report_id, "aborted");
            post_close_event();
         }
      } // on_focus_start

      
      void TranGetSettings::on_failure(failure_type failure)
      {
         PakBusTran::on_failure(failure);
         if(failure == PakCtrl::DeliveryFailure::timed_out_or_resource_error &&
            client_type::is_valid_instance(client) &&
            ++retry_count <= 3)
         {
            router->set_report_state(report_id, "retrying");
            send_command();
         }
         else
         {
            client_type::outcome_type outcome;
            router->set_report_state(report_id, "failed");
            switch(failure)
            {
            case PakCtrl::DeliveryFailure::unreachable_destination:
               outcome = client_type::outcome_unreachable;
               break;
               
            case PakCtrl::DeliveryFailure::unsupported_message_type:
               outcome = client_type::outcome_unsupported;
               break;
               
            default:
               outcome = client_type::outcome_communication_failed;
               break;
            }
            if(client_type::is_valid_instance(client))
               client->on_complete(this,outcome,0);
            post_close_event();
         }
      } // on_failure

      
      void TranGetSettings::on_pakctrl_message(pakctrl_message_handle &message)
      {
         StrAsc settings;

         message->readAsciiZ(settings);
         if(client_type::is_valid_instance(client))
            client->on_complete(this,client_type::outcome_success,settings);
         post_close_event();
      } // on_pakctrl_message


      void TranGetSettings::parse_settings(settings_type &settings, StrAsc const &s)
      {
         StrAsc setting_name, temp;
         bool reading_name(true);

         for(size_t i = 0; i < s.length(); ++i)
         {
            if(s[i] == '=')
            {
               setting_name = temp;
               temp.cut(0);
               reading_name = false;
            }
            else if(s[i] == ';')
            {
               reading_name = true;
               settings.push_back(setting_type(setting_name, temp));
               temp.cut(0);
            }
            else
               temp.append(s[i]);
         }
         if(!reading_name)
            settings.push_back(setting_type(setting_name, temp));
      } // parse_settings
      
      
      void TranGetSettings::send_command()
      {
         pakctrl_message_handle cmd(new PakCtrlMessage);
         cmd->set_message_type(PakCtrl::Messages::get_settings_cmd);
         cmd->set_expect_more(ExpectMoreCodes::expect_more);
         cmd->addAsciiZ(setting_names.c_str());
         set_time_out(1000);
         send_pakctrl_message(cmd);
      } // send_command 
   };
};

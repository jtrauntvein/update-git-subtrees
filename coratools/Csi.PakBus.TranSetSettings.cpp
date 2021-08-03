/* Csi.PakBus.TranSetSettings.cpp

   Copyright (C) 2002, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 07 May 2002
   Last Change: Friday 29 April 2016
   Last Commit: $Date: 2016-05-03 16:18:03 -0600 (Tue, 03 May 2016) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.PakBus.TranSetSettings.h"
#include "Csi.PakBus.Defs.h"
#include "Csi.PakBus.PakCtrlMessage.h"
#include "Csi.PakBus.Router.h"
#include "Csi.StrAscStream.h"


namespace Csi
{
   namespace PakBus
   {
      ////////////////////////////////////////////////////////////
      // class TranSetSettings definitions
      ////////////////////////////////////////////////////////////
      TranSetSettings::TranSetSettings(
         Router *router,
         timer_handle &timer,
         priority_type priority,
         uint2 destination_address,
         TranSetSettingsClient *client_,
         StrAsc const &settings_):
         PakBusTran(router,timer,priority,destination_address),
         client(client_),
         settings(settings_),
         retry_count(0)
      { }
      
         
      TranSetSettings::~TranSetSettings()
      { }

      
      void TranSetSettings::start()
      {
         OStrAscStream temp;
         temp << "PakBus set settings: " << get_destination_address();
         report_id = router->add_report(temp.str(), priority, "requesting focus");
         request_focus();
      }

      void TranSetSettings::on_focus_start()
      {
         if(client_type::is_valid_instance(client))
         {
            router->set_report_state(report_id, "sending message");
            send_command();
         }
         else
            post_close_event();
      } // on_focus_start

      
      void TranSetSettings::on_failure(failure_type failure)
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

      
      void TranSetSettings::on_pakctrl_message(pakctrl_message_handle &message)
      {
         try
         {
            byte resp_code = message->readByte();
            uint4 error_offset = (uint4)settings.length();
            
            if(resp_code != 0)
               error_offset = message->readUInt2();
            if(client_type::is_valid_instance(client))
               client->on_complete(
                  this,
                  static_cast<client_type::outcome_type>(resp_code),
                  error_offset);
            post_close_event();
         }
         catch(std::exception &)
         {
            if(client_type::is_valid_instance(client))
               client->on_complete(
                  this,
                  client_type::outcome_communication_failed,
                  0);
            post_close_event();
         }
      } // on_pakctrl_message


      void TranSetSettings::send_command()
      {
         pakctrl_message_handle cmd(new PakCtrlMessage);

         cmd->set_message_type(PakCtrl::Messages::set_settings_cmd);
         cmd->set_expect_more(ExpectMoreCodes::expect_more);
         cmd->addAsciiZ(settings.c_str());
         set_time_out(1000);
         send_pakctrl_message(cmd);
      } // send_command
   };
};

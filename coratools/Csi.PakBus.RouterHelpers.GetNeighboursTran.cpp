/* Csi.PakBus.RouterHelpers.GetNeighboursTran.cpp

   Copyright (C) 2002, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 01 April 2002
   Last Change: Thursday 29 November 2012
   Last Commit: $Date: 2012-11-30 13:08:23 -0600 (Fri, 30 Nov 2012) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.PakBus.RouterHelpers.GetNeighboursTran.h"
#include "Csi.PakBus.RouterHelpers.h"
#include "Csi.PakBus.PakCtrlMessage.h"
#include "Csi.PakBus.Router.h"
#include "Csi.Utils.h"
#include "Csi.StrAscStream.h"


namespace Csi
{
   namespace PakBus
   {
      namespace RouterHelpers
      {
         ////////////////////////////////////////////////////////////
         // class GetNeighboursTran definitions
         ////////////////////////////////////////////////////////////
         GetNeighboursTran::GetNeighboursTran(
            Router *router,
            Csi::SharedPtr<OneShot> &timer,
            Csi::SharedPtr<router_type> &router_entry_):
            router_entry(router_entry_),
            PakBusTran(router,timer,Priorities::extra_high,router_entry_->router_id)
         { }

         
         void GetNeighboursTran::start()
         {
            OStrAscStream temp;
            temp << "PakBus get neighbours: " << get_destination_address();
            report_id = router->add_report(temp.str(), priority, "requesting focus");
            request_focus();
         }


         void GetNeighboursTran::on_focus_start()
         {
            // if the port is dialed and off-line, we do not want to dial the link in order to send
            // the neighbour lists.
            route_type *route = router->find_route(get_destination_address());
            if(route &&
               (!route->port->link_is_dialed() ||
                route->port->link_is_active()))
            {
               // the message has no parameters except for the normal pakctrl header.
               pakctrl_message_handle get_cmd(new PakCtrlMessage);
               get_cmd->set_message_type(PakCtrl::Messages::get_neighbour_list_cmd);
               get_cmd->set_expect_more(ExpectMoreCodes::expect_more);
               set_time_out(1000);
               router->set_report_state(report_id, "sending message");
               send_pakctrl_message(get_cmd);
            }
            else
            {
               router_entry->send_delay_base = counter(0);
               post_close_event();
            }
         } // on_focus_start


         void GetNeighboursTran::get_transaction_description(std::ostream &out)
         {
            out << "PakCtrl::GetNeighbours\",\""
                << router_entry->router_id;
         } // get_transaction_description

         
         void GetNeighboursTran::on_failure(failure_type failure)
         {
            PakBusTran::on_failure(failure);
            router_entry->send_delay_base = counter(0);
            router->set_report_state(report_id, "failed");
            post_close_event();
         } // on_failure

         
         void GetNeighboursTran::on_pakctrl_message(pakctrl_message_handle &message)
         {
            router_entry->get_all = false;
            router_entry->send_delay_base = 0;
            router->on_router_neighbour_list(
               router_entry->router_id,
               message,
               router->neighbour_list_update_complete);
            router->set_report_state(report_id, "complete");
            post_close_event();
         } // on_pakctrl_message 
      };
   };
};


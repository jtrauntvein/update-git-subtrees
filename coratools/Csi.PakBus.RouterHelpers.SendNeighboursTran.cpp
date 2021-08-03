/* Csi.PakBus.RouterHelpers.SendNeighboursTran.cpp

   Copyright (C) 2002, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 29 March 2002
   Last Change: Thursday 29 November 2012
   Last Commit: $Date: 2012-11-30 13:08:23 -0600 (Fri, 30 Nov 2012) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.PakBus.RouterHelpers.SendNeighboursTran.h"
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
         // class SendNeighboursTran definitions
         ////////////////////////////////////////////////////////////
         SendNeighboursTran::SendNeighboursTran(
            Router *router,
            Csi::SharedPtr<OneShot> &timer,
            Csi::SharedPtr<router_type> &router_entry_):
            router_entry(router_entry_),
            PakBusTran(router,timer,Priorities::extra_high,router_entry_->router_id)
         { }


         void SendNeighboursTran::start()
         {
            OStrAscStream temp;
            temp << "PakBus send neighbours: " << get_destination_address();
            report_id = router->add_report(temp.str(), priority, "requesting focus");
            request_focus();
         } // start
         
         
         void SendNeighboursTran::on_focus_start()
         {
            // if the port is dialed and off-line, we do not want to dial the link in order to send
            // the neighbour lists.
            route_type *route = router->find_route(get_destination_address());
            if(route &&
               (!route->port->link_is_dialed() ||
                route->port->link_is_active()))
            {
               // form the header of the command message.  We will wait until just before the
               // message is sent to fill in the details of the message.  That way, the most current
               // information will be sent.
               pakctrl_message_handle send_cmd(new PakCtrlMessage);
               send_cmd->set_message_type(PakCtrl::Messages::send_neighbour_list_cmd);
               send_cmd->set_expect_more(ExpectMoreCodes::expect_more);
               set_time_out(1000);
               router->set_report_state(report_id, "sending message");
               send_pakctrl_message(send_cmd);
            }
            else
            {
               router_entry->send_delay_base = counter(0);
               router->set_report_state(report_id, "skipped");
               post_close_event();
            }
         } // on_focus_start


         void SendNeighboursTran::get_transaction_description(std::ostream &out)
         {
            out << "PakCtrl::SendNeighbours\",\"" << router_entry->router_id;
         } // get_transaction_description

         
         void SendNeighboursTran::on_failure(failure_type failure)
         {
            PakBusTran::on_failure(failure);
            router_entry->send_delay_base = counter(0);
            router->set_report_state(report_id, "failed");
            post_close_event();
         } // on_failure

         
         void SendNeighboursTran::on_pakctrl_message(pakctrl_message_handle &message)
         {
            router_entry->send_all = false;
            router_entry->send_change = false;
            router_entry->send_delay_base = 0;
            router->set_report_state(report_id, "complete");
            post_close_event();
         } // on_pakctrl_message


         void SendNeighboursTran::on_sending_message(message_handle &message)
         {
            if(router_entry->send_all)
            {
               message->addByte(0); // send full list
               message->addByte(router->neighbour_list_version);
               for(Router::neighbours_type::iterator ni = router->neighbours.begin();
                   ni != router->neighbours.end();
                   ++ni)
               {
                  if(!ni->second->needs_hello_info)
                  {
                     message->addUInt2(
                        ni->second->pack_list_entry(),
                        !is_big_endian());
                  }
               }
            }
            else
            {
               switch(router->last_neighbour_change.change)
               {
               case Router::link_change_added:
               case Router::link_change_changed:
                  message->addByte(1);
                  break;
                  
               case Router::link_change_deleted:
                  message->addByte(2);
                  break;
               }
               message->addByte(router->neighbour_list_version);
               message->addUInt2(
                  router->last_neighbour_change.neighbour->pack_list_entry(),
                  !is_big_endian());
            }
            PakBusTran::on_sending_message(message);
         } // on_sending_message
      };
   };
};


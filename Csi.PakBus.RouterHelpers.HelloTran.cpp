/* Csi.PakBus.RouterHelpers.HelloTran.cpp

   Copyright (C) 2002, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Saturday 23 March 2002
   Last Change: Monday 23 December 2019
   Last Commit: $Date: 2019-12-23 13:22:53 -0600 (Mon, 23 Dec 2019) $ 
   Last Changed by: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.PakBus.RouterHelpers.HelloTran.h"
#include "Csi.PakBus.PakCtrlMessage.h"
#include "Csi.PakBus.PortBase.h"
#include "Csi.PakBus.Router.h"
#include "Csi.ByteOrder.h"
#include "Csi.StrAscStream.h"


namespace Csi
{
   namespace PakBus
   {
      namespace RouterHelpers
      {
         ////////////////////////////////////////////////////////////
         // class HelloTran definitions
         ////////////////////////////////////////////////////////////
         HelloTran::HelloTran(
            Router *router,
            Csi::SharedPtr<OneShot> &timer,
            Csi::SharedPtr<neighbour_type> &neighbour_,
            bool send_if_dialed_,
            PakBusTran *replaced_other_):
            neighbour(neighbour_),
            PakBusTran(
               router,
               timer,
               replaced_other_ ? replaced_other_->get_priority() : Priorities::extra_high,
               neighbour_->physical_address),
            send_if_dialed(send_if_dialed_),
            replaced_other(replaced_other_)
         { }


         HelloTran::~HelloTran()
         { }


         void HelloTran::start()
         {
            if(!send_if_dialed)
            {
               OStrAscStream report_name;
               report_name << "PakBus Hello: " << neighbour->physical_address;
               report_id = router->add_report(report_name.str(), priority, "requesting focus");
               request_focus();
            }
         }
            
         
         void HelloTran::on_focus_start()
         {
            if(PortBase::is_valid_instance(neighbour->port))
            {
               // if the port is dialed and not active, we don't want to send the hello command
               // because it would force the port into an active state.  Instead, we will reset the
               // timer on the neighbour record so that the hello transaction can be re-evaluated
               // down the road.
               PortBase *port = neighbour->port;
               if(!port->must_close_link() &&
                  (!port->link_is_dialed() ||
                  (port->link_is_active() ||
                   send_if_dialed)))
               {
                  pakctrl_message_handle hello_cmd(new PakCtrlMessage);
                  hello_cmd->set_message_type(PakCtrl::Messages::hello_cmd);
                  hello_cmd->set_expect_more(ExpectMoreCodes::expect_more);
                  hello_cmd->set_physical_destination(neighbour->physical_address);
                  hello_cmd->set_port(neighbour->port);
                  hello_cmd->set_use_own_route(true);
                  hello_cmd->addByte(router->get_is_leaf_node() ? 0 : 1); // is_router
                  hello_cmd->addByte(neighbour->port->get_hop_metric());
                  hello_cmd->addUInt2(
                     neighbour->port->get_verify_interval(),
                     !is_big_endian());
                  set_time_out(1000);
                  if(report_id != -1)
                     router->set_report_state(report_id, "sending command");
                  send_pakctrl_message(hello_cmd);
               }
               else
               {
                  // if this is the first attempt, we'll reset the hello tries so that the
                  // transaction won't be endlessly repeated. 
                  neighbour->hello_tries = 0;
                  neighbour->time_since_last_beacon = Csi::counter(0);
                  if(report_id != -1)
                     router->set_report_state(report_id, "skipped");
                  post_close_event();
               }
            } 
            else
               post_close_event();
         } // on_focus_start


         void HelloTran::get_transaction_description(std::ostream &out)
         {
            out << "PakCtrl::Hello\",\""
                << neighbour->physical_address;
         } // get_transaction_description


         void HelloTran::on_failure(failure_type failure)
         {
            if(router)
            {
               PakBusTran::on_failure(failure);
               neighbour->hello_tries += 1;
               neighbour->send_hello_delay = 1000;
               neighbour->send_hello_delay_base = counter(0);
               router->set_report_state(report_id, "failed");
               if(neighbour->hello_tries >= 4 ||
                  failure == PakCtrl::DeliveryFailure::unreachable_destination ||
                  neighbour->needs_hello_info)
               {
                  router->on_neighbour_lost(
                     neighbour->port,
                     neighbour->physical_address);
               }
               post_close_event();
            }
         } // on_failure


         void HelloTran::on_pakctrl_message(pakctrl_message_handle &message)
         {
            try
            {
               if(PortBase::is_valid_instance(neighbour->port))
               {
                  if(message->get_message_type() == PakCtrl::Messages::hello_ack)
                  {
                     byte is_router = message->readByte();
                     HopMetric hop_metric = message->readByte();
                     uint2 beacon_interval = message->readUInt2(!is_big_endian());
                  
                     neighbour->hello_tries = 0;
                     router->on_neighbour_info(
                        neighbour->port,
                        neighbour->physical_address,
                        is_router ? true : false,
                        hop_metric,
                        beacon_interval);
                     router->set_report_state(report_id, "complete");
                     post_close_event();
                  }
               }
               else
                  post_close_event();
            }
            catch(std::exception &e)
            {
               router->log_debug("Csi::PakBus::RouterHelpers::HelloTran",e.what());
               post_close_event();
            }
         } // on_pakctrl_message 
      };
   };
};


/* Csi.PakBus.TranEcho.cpp

   Copyright (C) 2003, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 24 January 2003
   Last Change: Friday 30 November 2012
   Last Commit: $Date: 2012-11-30 13:08:23 -0600 (Fri, 30 Nov 2012) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.PakBus.TranEcho.h"
#include "Csi.PakBus.PakCtrlMessage.h"
#include "Csi.PakBus.Router.h"
#include "Csi.Utils.h"
#include "Csi.StrAscStream.h"


namespace Csi
{
   namespace PakBus
   {
      ////////////////////////////////////////////////////////////
      // class TranEcho definitions
      ////////////////////////////////////////////////////////////
      TranEcho::TranEcho(
         Router *router,
         timer_handle &timer,
         priority_type priority,
         uint2 destination_address,
         client_type *client_,
         uint2 packet_size_,
         uint2 remote_client_,
         byte remote_tran_no_,
         uint2 ack_physical_dest_,
         PortBase *ack_port_):
         PakBusTran(router,timer,priority,destination_address),
         client(client_),
         packet_size(packet_size_),
         packet_size_used(packet_size_),
         remote_client(remote_client_),
         remote_tran_no(remote_tran_no_),
         ack_physical_dest(ack_physical_dest_),
         ack_port(ack_port_)
      { }

      
      TranEcho::~TranEcho()
      { }


      void TranEcho::start()
      {
         OStrAscStream temp;
         temp << "PakBus Ping: " << destination_address;
         report_id = router->add_report(temp.str(), priority, "requesting focus");
         request_focus();
      }
      
         
      void TranEcho::on_focus_start()
      {
         if(remote_client || client_type::is_valid_instance(client))
         {
            // create the message
            pakctrl_message_handle cmd(new PakCtrlMessage);

            cmd->set_message_type(PakCtrl::Messages::echo_cmd);
            cmd->set_expect_more(ExpectMoreCodes::expect_more);
            cmd->addInt8(0);    // placeholder for the return timestamp

            // we want to add random filler to the message to make it at least the specified
            // length.  we will add a signature nullifier at the end of the block to give internal
            // validation.
            uint2 current_sig = 0xAAAA;
            while(cmd->length() + 2 < cmd->max_body_len && cmd->length() + 2 < packet_size)
            {
               byte next_char = static_cast<byte>(rand() % 255);
               current_sig = calcSigFor(&next_char,1,current_sig);
               cmd->addByte(next_char); 
            }
            cmd->addUInt2(calcSigNullifier(current_sig),!is_big_endian());
            packet_size_used = static_cast<uint2>(cmd->length());
            set_time_out(1000);
            start_time = end_time = LgrDate::system();
            router->set_report_state(report_id, "sending command");
            send_pakctrl_message(cmd);
         }
         else
            post_close_event();
      } // on_focus_start

      
      void TranEcho::on_failure(failure_type failure)
      {
         PakBusTran::on_failure(failure);
         end_time = LgrDate::system();
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
         on_complete(outcome);
      } // on_failure


      void TranEcho::on_sending_message(message_handle &message)
      {
         start_time = end_time = LgrDate::system();
         PakBusTran::on_sending_message(message);
      } // on_sending_message

      
      void TranEcho::on_pakctrl_message(pakctrl_message_handle &message)
      {
         try
         {
            // read the response time from the message
            uint4 resp_seconds = message->readUInt4(!is_big_endian());
            uint4 resp_nano_seconds = message->readUInt4(!is_big_endian());
            response_time = LgrDate::nsecPerSec * resp_seconds;
            response_time.setNSec(resp_nano_seconds);
            end_time = LgrDate::system();

            // we want to verify that the response contains the content that was sent.  In order to
            // do this, we will verify that the signature of the remaining bytes is zero (we put a
            // signature nullifier at the end of the packet we sent)
            uint2 signature = 0xAAAA;
            while(message->whatsLeft() > 0)
            {
               byte ch = message->readByte();
               signature = calcSigFor(&ch,1,signature);
            }

            // we can now report the outcome to the client
            client_type::outcome_type outcome = client_type::outcome_success;
            if(signature != 0 && signature != 0xAAAA)
               outcome = client_type::outcome_corrupted_echo;
            router->set_report_state(report_id, "complete");
            on_complete(outcome);
         }
         catch(std::exception &)
         { on_complete(client_type::outcome_communication_failed); }
      } // on_pakctrl_message


      void TranEcho::on_complete(client_type::outcome_type outcome)
      {
         // if there is a remote client, we need to send the ack message
         if(remote_client)
         {
            pakctrl_message_handle ack(new PakCtrlMessage);
            ack->set_message_type(PakCtrl::Messages::remote_echo_ack);
            ack->set_transaction_no(remote_tran_no);
            ack->set_destination(remote_client);
            ack->set_source(router->get_this_node_address());
            ack->set_priority(get_priority());
            switch(outcome)
            {
            case client_type::outcome_success:
               ack->addByte(0);
               ack->addUInt4(
                  static_cast<uint4>(
                     (end_time - start_time).get_millSec()),
                  !is_big_endian());
               break;
               
            case client_type::outcome_unreachable:
               ack->addByte(1);
               break;
               
            default:
               ack->addByte(2);
               break; 
            }
            if(ack_physical_dest != 0 && ack_port != 0)
            {
               ack->set_physical_destination(ack_physical_dest);
               ack->set_port(ack_port);
               ack->set_use_own_route(true);
            }
            router->send_message_from_app(ack.get_handle());
         }
         if(client_type::is_valid_instance(client))
            client->on_complete(this,outcome);
         post_close_event();
      } // on_complete
   };
};

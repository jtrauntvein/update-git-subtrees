/* Cora.PbRouter.Pinger.cpp

   Copyright (C) 2003, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 24 January 2003
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.PbRouter.Pinger.h"
#include "Csi.ByteOrder.h"


namespace Cora
{
   namespace PbRouter
   {
      namespace
      {
         ////////////////////////////////////////////////////////////
         // class event_complete
         ////////////////////////////////////////////////////////////
         class event_complete: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // client
            ////////////////////////////////////////////////////////////
            typedef PingerClient client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // pinger
            ////////////////////////////////////////////////////////////
            Pinger *pinger;

            ////////////////////////////////////////////////////////////
            // outcome
            ////////////////////////////////////////////////////////////
            typedef client_type::outcome_type outcome_type;
            outcome_type outcome;

            ////////////////////////////////////////////////////////////
            // response_time
            ////////////////////////////////////////////////////////////
            uint4 response_time_msec;

            ////////////////////////////////////////////////////////////
            // packet_size_used
            ////////////////////////////////////////////////////////////
            uint2 packet_size_used;

            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               Pinger *pinger,
               client_type *client,
               outcome_type outcome,
               uint4 response_time_msec = 0,
               uint2 packet_size_used = 0)
            {
               event_complete *event = new event_complete(
                  pinger,client,outcome,response_time_msec,packet_size_used);
               event->post();
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_complete(
               Pinger *pinger_,
               client_type *client_,
               outcome_type outcome_,
               uint4 response_time_msec_,
               uint2 packet_size_used_):
               Event(event_id,pinger_),
               client(client_),
               pinger(pinger_),
               outcome(outcome_),
               response_time_msec(response_time_msec_),
               packet_size_used(packet_size_used_)
            { }
         };


         uint4 const event_complete::event_id =
         Csi::Event::registerType("Cora::PbRouter::Pinger::event_complete");
      };


      ////////////////////////////////////////////////////////////
      // class Pinger definitions
      ////////////////////////////////////////////////////////////
      Pinger::Pinger():
         client(0),
         state(state_standby),
         pakbus_address(0),
         packet_size(1000),
         ping_from_address(0)
      { }

      
      Pinger::~Pinger()
      { finish(); }

      
      void Pinger::set_pakbus_address(uint2 pakbus_address_)
      {
         if(state == state_standby)
            pakbus_address = pakbus_address_;
         else
            throw exc_invalid_state();
      } // set_pakbus_address

      
      void Pinger::set_packet_size(uint2 packet_size_)
      {
         if(state == state_standby)
            packet_size = packet_size_;
         else
            throw exc_invalid_state();
      } // set_packet_size


      void Pinger::set_ping_from_address(uint2 ping_from_address_)
      {
         if(state != state_standby)
            throw exc_invalid_state();
         ping_from_address = ping_from_address_;
      } // set_ping_from_address

      
      void Pinger::start(
         client_type *client_,
         router_handle &router)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
            {
               client = client_;
               if(ping_from_address == 0)
               {
                  state = state_delegate;
                  PbRouterBase::start(router);
               }
               else
               {
                  init_sender();
                  sender->start(this,router);
                  state = state_active; 
               }
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state(); 
      } // start

      
      void Pinger::start(
         client_type *client_,
         ClientBase *other_component)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
            {
               client = client_;
               if(ping_from_address == 0)
               {
                  state = state_delegate;
                  PbRouterBase::start(other_component);
               }
               else
               {
                  init_sender();
                  sender->start(this,other_component);
                  state = state_active; 
               }
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state(); 
      } // start

      
      void Pinger::finish()
      {
         client = 0;
         state = state_standby;
         PbRouterBase::finish();
      } // finish


      void Pinger::on_complete(
         PakctrlMessageSender *sender,
         PakctrlMessageSenderClient::outcome_type outcome_,
         response_type &response,
         uint4 round_trip_time)
      {
         if(outcome_ == outcome_response_received)
         {
            byte reported_outcome = response->readByte();
            if(reported_outcome == 0)
            {
               uint4 response_time = response->readUInt4(!Csi::is_big_endian());
               event_complete::cpost(
                  this,
                  client,
                  PingerClient::outcome_success,
                  response_time,
                  packet_size);
            }
            else
            {
               PingerClient::outcome_type outcome;
               switch(reported_outcome)
               {
               case 1:
                  outcome = PingerClient::outcome_unreachable;
                  break;
                  
               default:
                  outcome = PingerClient::outcome_communication_failed;
                  break;
               }
               event_complete::cpost(this,client,outcome);
            }
         }
         else
         {
            PingerClient::outcome_type outcome;
            switch(outcome_)
            {
            case outcome_invalid_logon:
               outcome = PingerClient::outcome_invalid_logon;
               break;
               
            case outcome_invalid_router_id:
               outcome = PingerClient::outcome_invalid_router_id;
               break;
               
            case outcome_server_permission_denied:
               outcome = PingerClient::outcome_server_permission_denied;
               break;
               
            case outcome_server_session_failed:
               outcome = PingerClient::outcome_server_session_failed;
               break;
               
            case outcome_address_unreachable:
               outcome = PingerClient::outcome_unreachable;
               break;
               
            case outcome_command_unsupported:
            case outcome_unsupported:
               outcome = PingerClient::outcome_unsupported;
               break;

            case outcome_timed_out:
            default:
               outcome = PingerClient::outcome_communication_failed;
               break;
            }
            event_complete::cpost(this,client,outcome);
         }
      } // on_complete
      
      
      void Pinger::on_pbrouterbase_ready()
      {
         Csi::Messaging::Message cmd(pbrouter_session,Messages::ping_cmd);
         cmd.addUInt4(++last_tran_no);
         cmd.addUInt2(pakbus_address);
         cmd.addUInt2(packet_size);
         state = state_active;
         router->sendMessage(&cmd);
      } // on_pbrouterbase_ready

      
      void Pinger::on_pbrouterbase_failure(pbrouterbase_failure_type failure)
      {
         client_type::outcome_type outcome;
         switch(failure)
         {
         case pbrouterbase_failure_logon:
            outcome = client_type::outcome_invalid_logon;
            break;
            
         case pbrouterbase_failure_session:
            outcome = client_type::outcome_server_session_failed;
            break;
            
         case pbrouterbase_failure_invalid_router_id:
            outcome = client_type::outcome_invalid_router_id;
            break;
            
         case pbrouterbase_failure_unsupported:
            outcome = client_type::outcome_unsupported;
            break;
            
         case pbrouterbase_failure_security:
            outcome = client_type::outcome_server_permission_denied;
            break;
            
         default:
            outcome = client_type::outcome_unknown;
            break;   
         }
         event_complete::cpost(this,client,outcome);
      } // on_pbrouterbase_failure

      
      void Pinger::onNetMessage(
         Csi::Messaging::Router *router, 
         Csi::Messaging::Message *message)
      {
         if(state == state_active)
         {
            if(message->getMsgType() == Messages::ping_ack)
            {
               // parse the message
               uint4 tran_no;
               uint4 server_outcome;
               int8 time_started;
               int8 time_ended;
               int8 resp_time = 0;
               uint2 packet_size_used = 0;
               message->readUInt4(tran_no);
               message->readUInt4(server_outcome);
               if(interface_version >= Csi::VersionNumber("1.3.4.3"))
                  message->readUInt2(packet_size_used);
               message->readInt8(time_started);
               message->readInt8(time_ended);
               if(server_outcome == 1)
                  message->readInt8(resp_time);

               // map the outcome
               client_type::outcome_type outcome;
               switch(server_outcome)
               {
               case 1:
                  outcome = client_type::outcome_success;
                  break;

               case 3:
                  outcome = client_type::outcome_communication_failed;
                  break;

               case 5:
                  outcome = client_type::outcome_invalid_pakbus_address;
                  break;

               case 6:
                  outcome = client_type::outcome_corrupt_echo;
                  break;

               default:
                  outcome = client_type::outcome_unknown;
                  break;
               }
               event_complete::cpost(
                  this,
                  client,
                  outcome,
                  static_cast<uint4>(
                     (time_ended - time_started) / Csi::LgrDate::nsecPerMSec),
                  packet_size_used);
            }
            else
               PbRouterBase::onNetMessage(router,message);
         }
         else
            PbRouterBase::onNetMessage(router,message);
      } // onNetMessage

      
      void Pinger::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         if(ev->getType() == event_complete::event_id)
         {
            event_complete *event = static_cast<event_complete *>(ev.get_rep());
            client_type *client = this->client;
            finish();
            if(event->client == client && client_type::is_valid_instance(client))
               client->on_complete(
                  this,
                  event->outcome,
                  event->response_time_msec,
                  event->packet_size_used);
         }
      } // receive


      void Pinger::init_sender()
      {
         using namespace Csi::PakBus;
         Csi::SharedPtr<PakCtrlMessage> cmd(new PakCtrlMessage);
         sender.bind(new PakctrlMessageSender);
         cmd->set_message_type(PakCtrl::Messages::remote_echo_cmd);
         cmd->set_destination(ping_from_address);
         cmd->set_priority(Priorities::high);
         cmd->addUInt2(pakbus_address,!Csi::is_big_endian());
         cmd->addUInt2(packet_size,!Csi::is_big_endian());
         sender->set_command(cmd);
         sender->set_max_retries(0);
         sender->set_extra_timeout(10000);
         sender->set_pakbus_router_name(get_pakbus_router_name());
      } // init_sender
   };
};


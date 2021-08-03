/* Cora.PbRouter.PakctrlMessageSender.cpp

   Copyright (C) 2004, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 09 January 2004
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Last Changed by: $Author: jon $

*/

#pragma warning(disable:4786) // disable overlong symbols warning
#pragma hdrstop               // stop creation of precompiled header
#include "Cora.PbRouter.PakctrlMessageSender.h"


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
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // client
            ////////////////////////////////////////////////////////////
            typedef PakctrlMessageSenderClient client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // outcome
            ////////////////////////////////////////////////////////////
            typedef client_type::outcome_type outcome_type;
            outcome_type outcome;

            ////////////////////////////////////////////////////////////
            // response
            ////////////////////////////////////////////////////////////
            typedef client_type::response_type response_type;
            response_type response;

            ////////////////////////////////////////////////////////////
            // round_trip_time
            ////////////////////////////////////////////////////////////
            uint4 round_trip_time;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               PakctrlMessageSender *sender,
               client_type *client,
               outcome_type outcome,
               response_type response = 0,
               uint4 round_trip_time = 0)
            {
               try
               {
                  (new event_complete(
                     sender,
                     client,
                     outcome,
                     response,
                     round_trip_time))->post();
               }
               catch(Csi::Event::BadPost &)
               { }
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_complete(
               PakctrlMessageSender *sender,
               client_type *client_,
               outcome_type outcome_,
               response_type &response_,
               uint4 round_trip_time_):
               Event(event_id,sender),
               client(client_),
               outcome(outcome_),
               response(response_),
               round_trip_time(round_trip_time_)
            { }
         };


         uint4 const event_complete::event_id =
         Csi::Event::registerType("Cora::PbRouter::PakctrlMessageSender::event_complete");
      };

      
      ////////////////////////////////////////////////////////////
      // class PakctrlMessageSender definitions
      ////////////////////////////////////////////////////////////
      void PakctrlMessageSender::start(
         client_type *client_,
         router_handle &router)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
            {
               if(command != 0)
               {
                  client = client_;
                  state = state_delegate;
                  PbRouterBase::start(router);
               }
               else
                  throw std::invalid_argument("invalid command pointer");
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start

      
      void PakctrlMessageSender::start(
         client_type *client_,
         ClientBase *other_component)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
            {
               if(command != 0)
               {
                  client = client_;
                  state = state_delegate;
                  PbRouterBase::start(other_component);
               }
               else
                  throw std::invalid_argument("Invalid command pointer");
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start

      
      void PakctrlMessageSender::finish()
      {
         state = state_standby;
         client = 0;
         PbRouterBase::finish();
      } // finish

      
      void PakctrlMessageSender::receive(Csi::SharedPtr<Csi::Event> &ev)
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
                  event->response,
                  event->round_trip_time);
         }
      } // receive

      
      void PakctrlMessageSender::on_pbrouterbase_ready()
      {
         Csi::Messaging::Message cmd(
            pbrouter_session,
            Messages::send_pakctrl_message_cmd);
         state = state_active;
         cmd.addUInt4(++last_tran_no);
         cmd.addByte(command->get_message_type());
         cmd.addByte(command->get_priority());
         cmd.addBool(response_expected);
         cmd.addUInt2(command->get_destination());
         cmd.addUInt4(extra_timeout);
         cmd.addUInt4(max_retries);
         command->reset();
         cmd.addBytes(
            command->objAtReadIdx(),
            command->whatsLeft());
         router->sendMessage(&cmd);
      } // on_pbrouterbase_ready

      
      void PakctrlMessageSender::on_pbrouterbase_failure(
         pbrouterbase_failure_type failure)
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

      
      void PakctrlMessageSender::onNetMessage(
         Csi::Messaging::Router *router,
         Csi::Messaging::Message *message)
      {
         if(state == state_active &&
            message->getMsgType() == Messages::send_pakctrl_message_ack)
         {
            uint4 tran_no;
            uint4 server_outcome;

            message->readUInt4(tran_no);
            message->readUInt4(server_outcome);
            if(server_outcome == 1)
            {
               using namespace Csi::PakBus::PakCtrl;
               byte response_type;
               byte pakbus_tran_no;
               uint4 body_len;
               command_type response(new Csi::PakBus::PakCtrlMessage);
               uint4 round_trip_time;
               
               message->readByte(response_type);
               message->readByte(pakbus_tran_no);
               message->readUInt4(body_len);
               response->set_message_type(
                  static_cast<Csi::PakBus::PakCtrl::Messages::message_type>(
                     response_type));
               response->set_transaction_no(pakbus_tran_no);
               response->set_source(command->get_destination());
               response->set_destination(pbrouter_address);
               response->set_priority(command->get_priority());
               message->readBlock(*response,body_len);
               message->readUInt4(round_trip_time);
               event_complete::cpost(
                  this,
                  client,
                  client_type::outcome_response_received,
                  response,
                  round_trip_time);
            }
            else
            {
               client_type::outcome_type outcome;
               switch(server_outcome)
               {
               case 2:
                  outcome = client_type::outcome_no_response_expected;
                  break;
                  
               case 3:
                  outcome = client_type::outcome_address_unreachable;
                  break;

               case 4:
                  outcome = client_type::outcome_timed_out;
                  break;

               case 5:
                  outcome = client_type::outcome_command_malformed;
                  break;

               case 6:
                  outcome = client_type::outcome_command_unsupported;
                  break;
                  
               default:
                  outcome = client_type::outcome_unknown;
                  break;
               }
               event_complete::cpost(this,client,outcome);
            }
         }
         else
            PbRouterBase::onNetMessage(router,message);
      } // onNetMessage 
   };
};

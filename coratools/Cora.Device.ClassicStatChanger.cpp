/* Cora.Device.ClassicStatChanger.cpp

   Copyright (C) 2006, 2010 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 17 November 2006
   Last Change: Monday 19 July 2010
   Last Commit: $Date: 2010-07-19 10:21:56 -0600 (Mon, 19 Jul 2010) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.ClassicStatChanger.h"


namespace Cora
{
   namespace Device
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
            typedef ClassicStatChangerClient client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // outcome
            ////////////////////////////////////////////////////////////
            typedef client_type::outcome_type outcome_type;
            outcome_type const outcome;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               ClassicStatChanger *dumper,
               client_type *client,
               outcome_type outcome)
            { (new event_complete(dumper,client,outcome))->post(); }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_complete(
               ClassicStatChanger *dumper,
               client_type *client_,
               outcome_type outcome_):
               Event(event_id,dumper),
               client(client_),
               outcome(outcome_)
            { } 
         };

         
         uint4 const event_complete::event_id =
         Csi::Event::registerType("Cora::Device::ClassicStatChanger::event_complete");
      };
      
      
      ////////////////////////////////////////////////////////////
      // class ClassicStatChanger definitions
      ////////////////////////////////////////////////////////////
      void ClassicStatChanger::start(
         client_type *client_,
         router_handle &router)
      {
         if(state != state_standby)
            throw exc_invalid_state();
         if(!client_type::is_valid_instance(client_))
            throw std::invalid_argument("Invalid client reference");
         client = client_;
         state = state_delegate;
         DeviceBase::start(router);
      } // start
      
      
      void ClassicStatChanger::start(
         client_type *client_,
         ClientBase *other_component)
      {
         if(state != state_standby)
            throw exc_invalid_state();
         if(!client_type::is_valid_instance(client_))
            throw std::invalid_argument("Invalid client reference");
         client = client_;
         state = state_delegate;
         DeviceBase::start(other_component);
      } // start
      
      
      void ClassicStatChanger::finish()
      {
         state = state_standby;
         client = 0;
         DeviceBase::finish();
      } // finish
      
      
      void ClassicStatChanger::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         if(ev->getType() == event_complete::event_id)
         {
            event_complete *event = static_cast<event_complete *>(ev.get_rep());
            finish();
            event->client->on_complete(this,event->outcome);
         }
      } // receive
      
      
      void ClassicStatChanger::on_devicebase_ready()
      {
         Csi::Messaging::Message cmd(
            device_session,
            Cora::Device::Messages::change_classic_logger_stat_cmd);
         cmd.addUInt4(++last_tran_no);
         cmd.addByte(star_mode);
         cmd.addUInt4(window);
         cmd.addStr(window_value);
         state = state_active;
         router->sendMessage(&cmd);
      } // on_devicebase_ready
      
      
      void ClassicStatChanger::on_devicebase_failure(devicebase_failure_type failure)
      {
         client_type::outcome_type outcome;
         switch(failure)
         {
         case devicebase_failure_logon:
            outcome = client_type::outcome_invalid_logon;
            break;
            
         case devicebase_failure_session:
            outcome = client_type::outcome_session_failure;
            break;
            
         case devicebase_failure_invalid_device_name:
            outcome = client_type::outcome_invalid_device_name;
            break;
            
         case devicebase_failure_unsupported:
            outcome = client_type::outcome_unsupported;
            break;
            
         case devicebase_failure_security:
            outcome = client_type::outcome_blocked_by_server;
            break;
            
         default:
            outcome = client_type::outcome_unknown;
            break;   
         }
         event_complete::cpost(this,client,outcome);
      } // on_devicebase_failure
      
      
      void ClassicStatChanger::onNetMessage(
         Csi::Messaging::Router *router,
         Csi::Messaging::Message *message)
      {
         if(state == state_active)
         {
            if(message->getMsgType() == Cora::Device::Messages::change_classic_logger_stat_ack)
            {
               uint4 tran_no;
               uint4 resp_code;
               client_type::outcome_type outcome;
               
               message->readUInt4(tran_no);
               message->readUInt4(resp_code);
               switch(resp_code)
               {
               case 1:
                  outcome = client_type::outcome_success;
                  break;
                  
               case 2:
                  outcome = client_type::outcome_comm_failed;
                  break;
                  
               case 3:
                  outcome = client_type::outcome_blocked_by_logger;
                  break;
                  
               case 5:
                  outcome = client_type::outcome_comm_disabled;
                  break;

               case 6:
                  outcome = client_type::outcome_no_response_after;
                  break;

               case 7:
                  outcome = client_type::outcome_invalid_window;
                  break;
                  
               default:
                  outcome = client_type::outcome_unknown;
                  break;
               }
               event_complete::cpost(this,client,outcome);
            }
            else
               DeviceBase::onNetMessage(router, message);
         }
         else
            DeviceBase::onNetMessage(router,message);
      } // onNetMessage
   };
};


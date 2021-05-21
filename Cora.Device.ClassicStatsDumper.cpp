/* Cora.Device.ClassicStatsDumper.cpp

   Copyright (C) 2006, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 16 November 2006
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.ClassicStatsDumper.h"


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
            typedef ClassicStatsDumperClient client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // outcome
            ////////////////////////////////////////////////////////////
            typedef client_type::outcome_type outcome_type;
            outcome_type const outcome;

            ////////////////////////////////////////////////////////////
            // results
            ////////////////////////////////////////////////////////////
            typedef client_type::results_type results_type;
            results_type const results;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               ClassicStatsDumper *dumper,
               client_type *client,
               outcome_type outcome,
               results_type const &results = results_type())
            { (new event_complete(dumper,client,outcome,results))->post(); }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_complete(
               ClassicStatsDumper *dumper,
               client_type *client_,
               outcome_type outcome_,
               results_type const &results_):
               Event(event_id,dumper),
               client(client_),
               outcome(outcome_),
               results(results_)
            { } 
         };


         uint4 const event_complete::event_id =
         Csi::Event::registerType("Cora::Device::ClassicStatsDumper::event_complete");
      };

      
      ////////////////////////////////////////////////////////////
      // class ClassicStatsDumper definitions
      ////////////////////////////////////////////////////////////
      void ClassicStatsDumper::start(
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

      
      void ClassicStatsDumper::start(
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

      
      void ClassicStatsDumper::finish()
      {
         state = state_standby;
         client = 0;
         DeviceBase::finish();
      } // finish


      void ClassicStatsDumper::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         if(ev->getType() == event_complete::event_id)
         {
            event_complete *event = static_cast<event_complete *>(ev.get_rep());
            finish();
            event->client->on_complete(this,event->outcome,event->results);
         }
      } // receive

      
      void ClassicStatsDumper::on_devicebase_ready()
      {
         Csi::Messaging::Message cmd(
            device_session,
            Cora::Device::Messages::dump_classic_logger_stats_cmd);
         cmd.addUInt4(++last_tran_no);
         cmd.addByte(star_mode);
         state = state_active;
         router->sendMessage(&cmd);
      } // on_devicebase_ready

      
      void ClassicStatsDumper::on_devicebase_failure(devicebase_failure_type failure)
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

      
      void ClassicStatsDumper::onNetMessage(
         Csi::Messaging::Router *router,
         Csi::Messaging::Message *message)
      {
         if(state == state_active)
         {
            if(message->getMsgType() == Cora::Device::Messages::dump_classic_logger_stats_ack)
            {
               uint4 tran_no;
               uint4 resp_code;
               message->readUInt4(tran_no);
               message->readUInt4(resp_code);
               if(resp_code == 1)
               {
                  uint4 count;
                  client_type::results_type results;
                  StrAsc temp;
                  message->readUInt4(count);
                  for(uint4 i = 0; i < count; ++i)
                  {
                     message->readStr(temp);
                     results.push_back(temp);
                  }
                  event_complete::cpost(this,client,client_type::outcome_success,results);
               }
               else
               {
                  client_type::outcome_type outcome;
                  switch(resp_code)
                  {
                  case 2:
                     outcome = client_type::outcome_comm_failed;
                     break;
                     
                  case 3:
                     outcome = client_type::outcome_blocked_by_logger;
                     break;
                     
                  case 5:
                     outcome = client_type::outcome_comm_disabled;
                     break;
                     
                  default:
                     outcome = client_type::outcome_unknown;
                     break;
                  }
                  event_complete::cpost(this,client,outcome);
               }
            }
            else
               DeviceBase::onNetMessage(router,message);
         }
         else
            DeviceBase::onNetMessage(router,message);
      } // onNetMessage
   };
};


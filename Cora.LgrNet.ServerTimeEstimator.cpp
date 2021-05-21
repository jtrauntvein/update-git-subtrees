/* Cora.LgrNet.ServerTimeEstimator.cpp

   Copyright (C) 2001, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 17 April 2001
   Last Change: Thursday 31 December 2020
   Last Commit: $Date: 2020-12-31 12:54:32 -0600 (Thu, 31 Dec 2020) $ 
   Committed by: $Author: jon $
   
*/


#pragma hdrstop               // stop creation of precompiled header
#include "Cora.LgrNet.ServerTimeEstimator.h"
#include "Csi.Utils.h"
#include "Cora.LgrNet.Defs.h"


namespace Cora
{
   namespace LgrNet
   {
      namespace ServerTimeEstimatorHelpers
      {
         class event_started: public Csi::Event
         {
         public:
            ServerTimeEstimatorClient *client;
            static uint4 const event_id;

         protected:
            event_started(
               uint4 event_id,
               ServerTimeEstimator *estimator,
               ServerTimeEstimatorClient *client_):
               Event(event_id,estimator),
               client(client_)
            { }

         public:
            static void create_and_post(
               ServerTimeEstimator *estimator,
               ServerTimeEstimatorClient *client)
            {
               try { (new event_started(event_id,estimator,client))->post(); }
               catch(Csi::Event::BadPost &) { }
            }
         };


         uint4 const event_started::event_id =
         Csi::Event::registerType("Cora::Lgrnet::ServerTimeEstimator::event_started");


         class event_failure: public event_started
         {
         public:
            typedef ServerTimeEstimatorClient::failure_type failure_type;
            failure_type failure;
            static uint4 const event_id;
            
         private:
            event_failure(
               ServerTimeEstimator *estimator,
               ServerTimeEstimatorClient *client,
               failure_type failure_):
               event_started(event_id,estimator,client),
               failure(failure_)
            { }


         public:
            static void create_and_post(
               ServerTimeEstimator *estimator,
               ServerTimeEstimatorClient *client,
               failure_type failure)
            {
               try { (new event_failure(estimator,client,failure))->post(); }
               catch(Csi::Event::BadPost &) { }
            }
         };


         uint4 const event_failure::event_id =
         Csi::Event::registerType("Cora::LgrNet::ServerTimeEstimator::event_failure");
      };


      ServerTimeEstimator::ServerTimeEstimator():
         client(0),
         state(state_standby),
         last_server_time_base(0)
      { }
      
      ServerTimeEstimator::~ServerTimeEstimator()
      { finish(); }

      void ServerTimeEstimator::set_check_clock_interval(uint4 check_clock_interval_)
      {
         if(state == state_standby)
            check_clock_interval = check_clock_interval_;
         else
            throw exc_invalid_state();
      } // set_check_clock_interval
      
      void ServerTimeEstimator::start(
         client_type *client_,
         router_handle &router)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
            {
               state = state_delegate;
               client = client_;
               ClientBase::start(router);
            }
            else
               throw std::invalid_argument("Invalid client specified");
         }
         else
            throw exc_invalid_state();
      } // start

      void ServerTimeEstimator::start(
         client_type *client_,
         ClientBase *other_component,
         bool use_own_logon)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
            {
               state = state_delegate;
               client = client_;
               ClientBase::start(other_component, use_own_logon);
            }
            else
               throw std::invalid_argument("Invalid client specified");
         }
         else
            throw exc_invalid_state();
      } // start

      Csi::LgrDate ServerTimeEstimator::get_server_time()
      {
         // if enough time has gone by since the last read, we will need to do another read of the
         // server
         if(state != state_active)
            throw exc_invalid_state();
         if(Csi::counter(last_server_time_base) >= check_clock_interval)
         {
            Csi::Messaging::Message cmd(
               net_session,
               Messages::get_server_clock_cmd);
            cmd.addUInt4(++last_tran_no);
            router->sendMessage(&cmd);
         }

         // calculate the return value.
         return last_server_time + Csi::counter(last_server_time_base)*Csi::LgrDate::nsecPerMSec;
      } // get_time

      void ServerTimeEstimator::finish()
      {
         state = state_standby;
         client = 0;
         ClientBase::finish();
      } // finish

      void ServerTimeEstimator::format_failure(
         std::ostream &out, client_type::failure_type failure)
      {
         switch(failure)
         {
         default:
            ClientBase::describe_failure(out, corabase_failure_unknown); 
            break;
            
         case client_type::failure_invalid_logon:
            ClientBase::describe_failure(out, corabase_failure_logon);
            break;
            
         case client_type::failure_session_broken:
            ClientBase::describe_failure(out, corabase_failure_session);
            break;
            
         case client_type::failure_unsupported:
            ClientBase::describe_failure(out, corabase_failure_unsupported);
            break;
            
         case client_type::failure_server_security_blocked:
            ClientBase::describe_failure(out, corabase_failure_security);
            break;

         case client_type::failure_access_token_invalid:
            ClientBase::describe_failure(out, corabase_failure_invalid_access);
            break;
            
         case client_type::failure_access_token_expired:
            ClientBase::describe_failure(out, corabase_failure_access_expired);
            break;
         }
      } // format_failure

      void ServerTimeEstimator::onNetMessage(
         Csi::Messaging::Router *rtr, Csi::Messaging::Message *message)
      {
         if(state == state_active || state == state_before_active)
         {
            switch(message->getMsgType())
            {
            case Messages::settings_advise_not:
               on_server_settings_not(message);
               break;
               
            case Messages::get_server_clock_ack:
               on_get_server_time_ack(message);
               break;

            default:
               ClientBase::onNetMessage(rtr,message);
               break;
            }
         }
         else
            ClientBase::onNetMessage(rtr,message);
      } // onNetMessage

      void ServerTimeEstimator::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace ServerTimeEstimatorHelpers;
         if(ev->getType() == event_failure::event_id)
         {
            event_failure *event = static_cast<event_failure *>(ev.get_rep());
            finish();
            if(client_type::is_valid_instance(event->client))
               event->client->on_failure(this,event->failure);
         }
         else if(ev->getType() == event_started::event_id)
         {
            event_started *event = static_cast<event_started *>(ev.get_rep());
            if(client_type::is_valid_instance(event->client))
               event->client->on_started(this);
            else
               finish();
         }
      } // receive

      void ServerTimeEstimator::on_corabase_ready()
      {
         Csi::Messaging::Message get_settings_command(
            net_session,
            Messages::get_settings_cmd);
         router->sendMessage(&get_settings_command);
         state = state_before_active;
      } // on_corabase_ready

      void ServerTimeEstimator::on_corabase_failure(corabase_failure_type failure)
      {
         using namespace ServerTimeEstimatorHelpers;
         client_type::failure_type client_failure;

         switch(failure)
         {
         case corabase_failure_logon:
            client_failure = client_type::failure_invalid_logon;
            break;

         case corabase_failure_session:
            client_failure = client_type::failure_session_broken;
            break;

         case corabase_failure_unsupported:
            client_failure = client_type::failure_unsupported;
            break;
            
         case corabase_failure_security:
            client_failure = client_type::failure_server_security_blocked;
            break;

         case corabase_failure_invalid_access:
            client_failure = client_type::failure_access_token_invalid;
            break;

         case corabase_failure_access_expired:
            client_failure = client_type::failure_access_token_expired;
            break;
            
         default:
            client_failure = client_type::failure_unknown;
            break;
         }
         event_failure::create_and_post(this,client,client_failure); 
      } // on_corabase_failure

      void ServerTimeEstimator::on_corabase_session_failure()
      {
         using namespace ServerTimeEstimatorHelpers;
         event_failure::create_and_post(this,client,client_type::failure_session_broken);
      } // on_corabase_session_failure

      void ServerTimeEstimator::on_get_server_time_ack(Csi::Messaging::Message *message)
      {
         // process the message contents
         uint4 tran_no;
         int8 server_time;
         
         message->readUInt4(tran_no);
         message->readInt8(server_time);
         last_server_time = server_time;
         last_server_time_base = Csi::counter(0);

         // make sure that the state machine is correct
         if(state == state_before_active)
         {
            using namespace ServerTimeEstimatorHelpers;
            event_started::create_and_post(this,client);
            state = state_active;
         }  
      } // on_get_server_time_ack
      
      void ServerTimeEstimator::on_server_settings_not(Csi::Messaging::Message *message)
      {
         uint4 settings_count;
         uint4 setting_id;
         uint4 setting_value_len;
         
         message->readUInt4(settings_count);
         for(uint4 i = 0; i < settings_count; ++i)
         {
            message->readUInt4(setting_id);
            message->readUInt4(setting_value_len);
            message->movePast(setting_value_len);
            if(setting_id == Settings::system_clock_specifier)
            {
               // we will initiate a get server clock transaction
               Csi::Messaging::Message cmd(
                  net_session,
                  Messages::get_server_clock_cmd);
               cmd.addUInt4(++last_tran_no);
               router->sendMessage(&cmd);
            }
         }
      } // on_server_settings_not
   };
};

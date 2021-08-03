/* Cora.LgrNet.SettingSetter.cpp

   Copyright (C) 2000, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 13 September 2000
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Committed by: $Author: jon $
   
*/

#pragma warning(disable:4786) // disable overlong symbols warning
#pragma hdrstop               // stop creation of precompiled header
#include "Cora.LgrNet.SettingSetter.h"
#include "Cora.LgrNet.Defs.h"
#include <assert.h>

namespace Cora
{
   namespace LgrNet
   {
      namespace SettingSetterHelpers
      {
         ////////////////////////////////////////////////////////////
         // class event_complete
         ////////////////////////////////////////////////////////////
         class event_complete: public Csi::Event
         {
         public:
            static uint4 const event_id;

            typedef SettingSetterClient::outcome_type outcome_type;
            static void create_and_post(
               SettingSetter *setter,
               SettingSetterClient *client,
               outcome_type outcome);

            void notify()
            { client->on_complete(setter,outcome); }
            
         private:
            SettingSetter *setter;
            SettingSetterClient *client;
            outcome_type outcome;

            event_complete(
               SettingSetter *setter_,
               SettingSetterClient *client_,
               outcome_type outcome_):
               Event(event_id,setter_),
               setter(setter_),
               client(client_),
               outcome(outcome_)
            { }

            friend class Cora::LgrNet::SettingSetter;
         };


         uint4 const event_complete::event_id =
         Csi::Event::registerType("Cora::LgrNet::SettingSetter::event_complete");


         void event_complete::create_and_post(
            SettingSetter *setter,
            SettingSetterClient *client,
            outcome_type outcome)
         {
            try { (new event_complete(setter,client,outcome))->post(); }
            catch(Event::BadPost &) { }
         } // create_and_post
      };


      ////////////////////////////////////////////////////////////
      // class SettingSetter definitions
      ////////////////////////////////////////////////////////////
      
      SettingSetter::SettingSetter():
         state(state_standby),
         client(0)
      { }

      
      SettingSetter::~SettingSetter()
      { finish(); }

      
      void SettingSetter::set_the_setting(setting_handle &the_setting_)
      {
         if(state == state_standby)
            the_setting = the_setting_;
         else
            throw exc_invalid_state();
      } // set_the_setting

      
      void SettingSetter::start(
         SettingSetterClient *client_,
         router_handle &router)
      {
         if(state == state_standby)
         {
            if(the_setting.get_rep() != 0)
            {
               if(SettingSetterClient::is_valid_instance(client_))
               {
                  client = client_;
                  state = state_delegate;
                  ClientBase::start(router);
               }
               else
                  throw std::invalid_argument("Invalid client pointer");
            }
            else
               throw std::invalid_argument("Invalid seting object");
         }
         else
            throw exc_invalid_state();
      } // start


      void SettingSetter::start(
         SettingSetterClient *client_,
         ClientBase *other_component)
      {
         if(state == state_standby)
         {
            if(the_setting.get_rep() != 0)
            {
               if(SettingSetterClient::is_valid_instance(client_))
               {
                  client = client_;
                  state = state_delegate;
                  ClientBase::start(other_component);
               }
               else
                  throw std::invalid_argument("Invalid client pointer");
            }
            else
               throw std::invalid_argument("Invalid seting object");
         }
         else
            throw exc_invalid_state();
      } // start

      
      void SettingSetter::finish()
      {
         state = state_standby;
         client = 0;
         ClientBase::finish();
      } // finish

      
      void SettingSetter::on_corabase_ready()
      {
         Csi::Messaging::Message cmd(
            net_session,
            Messages::set_settings_cmd);
         uint4 setting_pos;
         uint4 setting_len_pos;
         
         cmd.addUInt4(++last_tran_no);
         cmd.addUInt4(1);
         cmd.addUInt4(the_setting->get_identifier());
         setting_len_pos = cmd.getBodyLen();
         cmd.addUInt4(0);       // place holder for setting length
         setting_pos = cmd.getBodyLen();
         the_setting->write(&cmd);
         cmd.replaceUInt4(cmd.getBodyLen() - setting_pos,setting_len_pos);
         router->sendMessage(&cmd);
         state = state_active; 
      } // on_corabase_ready

      
      void SettingSetter::on_corabase_failure(corabase_failure_type failure)
      {
         using namespace SettingSetterHelpers;
         client_type::outcome_type outcome;
         switch(failure)
         {
         case corabase_failure_logon:
            outcome = client_type::outcome_invalid_logon;
            break;
            
         case corabase_failure_session:
            outcome = client_type::outcome_session_failed;
            break;
            
         case corabase_failure_unsupported:
            outcome = client_type::outcome_unsupported_transaction;
            break;
            
         case corabase_failure_security:
            outcome = client_type::outcome_server_security_blocked;
            break;
            
         default:
            outcome = client_type::outcome_unknown;
            break;
         }
         event_complete::create_and_post(this,client,outcome);
      } // on_corabase_failure

      
      void SettingSetter::on_corabase_session_failure()
      {
         using namespace SettingSetterHelpers;
         event_complete::create_and_post(this,client,client_type::outcome_session_failed);
      } // on_corabase_session_failure


      void SettingSetter::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(state == state_active)
         {
            if(msg->getMsgType() == Messages::set_settings_ack)
            {
               using namespace SettingSetterHelpers;
               uint4 tran_no;
               uint4 num_responses;
               uint4 setting_id;
               uint4 resp_code;
               client_type::outcome_type outcome;
               
               msg->readUInt4(tran_no);
               msg->readUInt4(num_responses);
               assert(num_responses == 1);
               msg->readUInt4(setting_id);
               assert(setting_id == the_setting->get_identifier());
               msg->readUInt4(resp_code);
               switch(resp_code)
               {
               case 0:
                  outcome = client_type::outcome_success;
                  the_setting->set_set_outcome(Setting::outcome_set); 
                  break;
                  
               case 1:
                  outcome = client_type::outcome_unsupported_setting;
                  the_setting->set_set_outcome(Setting::outcome_unsupported);
                  break;
                  
               case 2:
                  outcome = client_type::outcome_invalid_value;
                  the_setting->set_set_outcome(Setting::outcome_invalid_value); 
                  break;
                  
               case 3:
                  outcome = client_type::outcome_read_only;
                  the_setting->set_set_outcome(Setting::outcome_read_only);
                  break;

               case 4:
                  outcome = client_type::outcome_network_locked;
                  the_setting->set_set_outcome(Setting::outcome_network_locked);
                  break;
                  
               default:
                  outcome = client_type::outcome_unknown;
                  the_setting->set_set_outcome(Setting::outcome_no_attempt_made);
                  break;
               }
               event_complete::create_and_post(this,client,outcome); 
            }
            else
               ClientBase::onNetMessage(rtr,msg);
         }
         else
            ClientBase::onNetMessage(rtr,msg);
      } // onNetMessage

      
      void SettingSetter::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace SettingSetterHelpers;

         if(ev->getType() == event_complete::event_id)
         {
            event_complete *event = static_cast<event_complete *>(ev.get_rep());
            
            finish();
            if(SettingSetterClient::is_valid_instance(event->client))
               event->notify(); 
         }
         else
            assert(false);
      } // receive 
   };
};

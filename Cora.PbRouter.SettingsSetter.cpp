/* Cora.PbRouter.SettingsSetter.cpp

Copyright (C) 2002, 2016 Campbell Scientific, Inc.

Written by: Jon Trauntvein
Date Begun: Friday 10 May 2002
Last Change: Friday 29 April 2016
Last Commit: $Date: 2016-05-03 16:18:03 -0600 (Tue, 03 May 2016) $ 
Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.PbRouter.SettingsSetter.h"


namespace Cora
{
   namespace PbRouter
   {
      namespace SettingsSetterHelpers
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
            typedef SettingsSetterClient client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // outcome
            ////////////////////////////////////////////////////////////
            typedef client_type::outcome_type outcome_type;
            outcome_type outcome;

            ////////////////////////////////////////////////////////////
            // settings_applied
            ////////////////////////////////////////////////////////////
            uint4 settings_applied;

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_complete(
               SettingsSetter *setter,
               client_type *client_,
               outcome_type outcome_,
               uint4 settings_applied_):
               Event(event_id,setter),
               client(client_),
               outcome(outcome_),
               settings_applied(settings_applied_)
            { }

         public:
            ////////////////////////////////////////////////////////////
            // create_and_post
            ////////////////////////////////////////////////////////////
            static void create_and_post(
               SettingsSetter *setter,
               client_type *client,
               outcome_type outcome,
               uint4 settings_applied)
            {
               try
               {
                  event_complete *event = new event_complete(
                     setter,client,outcome,settings_applied);
                  event->post();
               }
               catch(Event::BadPost &)
               { }
            }
         };


         uint4 const event_complete::event_id =
         Csi::Event::registerType("Cora::PbRouter::SettingsSetter::event_complete");
      };


      ////////////////////////////////////////////////////////////
      // class SettingsSetter definitions
      ////////////////////////////////////////////////////////////
      SettingsSetter::SettingsSetter():
         client(0),
         state(state_standby)
      { }

      
      SettingsSetter::~SettingsSetter()
      { finish(); }

      
      void SettingsSetter::set_pakbus_address(uint2 pakbus_address_)
      {
         if(state == state_standby)
            pakbus_address = pakbus_address_;
         else
            throw exc_invalid_state();
      } // set_pakbus_address

      
      void SettingsSetter::add_setting(
         StrAsc const &setting_name,
         StrAsc const &setting_value)
      {
         if(state == state_standby)
         {
            settings.push_back(setting_type(setting_name,setting_value));
            if(setting_value.length() > 0 && setting_value.last() == '\n')
            {
               StrAsc &val = settings.back().second;
               val.cut(val.length() - 1);
            }
         }
         else
            throw exc_invalid_state();
      } // add_setting

      
      void SettingsSetter::clear_settings()
      {
         if(state == state_standby)
            settings.clear();
         else
            throw exc_invalid_state();
      } // clear_settings

      
      void SettingsSetter::start(
         client_type *client_,
         router_handle &router)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
            {
               client = client_;
               state = state_delegate;
               PbRouterBase::start(router);
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start

      
      void SettingsSetter::start(
         client_type *client_,
         ClientBase *other_component)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
            {
               client = client_;
               state = state_delegate;
               PbRouterBase::start(other_component);
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start

      
      void SettingsSetter::finish()
      {
         client = 0;
         state = state_standby;
         PbRouterBase::finish();
      } // finish

      
      void SettingsSetter::on_pbrouterbase_ready()
      {
         Csi::Messaging::Message cmd(pbrouter_session,Messages::set_settings_cmd);
         cmd.addUInt4(++last_tran_no);
         cmd.addUInt2(pakbus_address);
         cmd.addUInt4((uint4)settings.size());
         for(settings_type::iterator si = settings.begin();
             si != settings.end();
             ++si)
         {
            cmd.addStr(si->first);
            cmd.addStr(si->second);
         }
         state = state_active;
         router->sendMessage(&cmd);
      } // on_pbrouterbase_ready

      
      void SettingsSetter::on_pbrouterbase_failure(pbrouterbase_failure_type failure)
      {
         using namespace SettingsSetterHelpers;
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
         event_complete::create_and_post(this,client,outcome,0);
      } // on_pbrouterbase_failure

      
      void SettingsSetter::onNetMessage(
         Csi::Messaging::Router *router,
         Csi::Messaging::Message *message)
      {
         if(state == state_active)
         {
            if(message->getMsgType() == Messages::set_settings_ack)
            {
               using namespace SettingsSetterHelpers;
               uint4 tran_no;
               uint4 resp_code;
               uint4 applied_count;
               client_type::outcome_type outcome;
               
               message->readUInt4(tran_no);
               message->readUInt4(resp_code);
               message->readUInt4(applied_count);
               switch(resp_code)
               {
               case 1:
                  outcome = client_type::outcome_success;
                  break;
                  
               case 2:
                  outcome = client_type::outcome_communication_disabled;
                  break;
                  
               case 3:
                  outcome = client_type::outcome_communication_failed;
                  break;
                  
               case 4:
                  outcome = client_type::outcome_unreachable;
                  break;
                  
               case 5:
                  outcome = client_type::outcome_setting_read_only;
                  break;
                  
               case 6:
                  outcome = client_type::outcome_not_enough_space;
                  break;
                  
               case 7:
                  outcome = client_type::outcome_invalid_name_or_value;
                  break;
                  
               case 8:
                  outcome = client_type::outcome_node_permission_denied;
                  break;
               }
               event_complete::create_and_post(this,client,outcome,applied_count);
            }
            else
               PbRouterBase::onNetMessage(router,message);
         }
         else
            PbRouterBase::onNetMessage(router,message);
      } // onNetMessage

      
      void SettingsSetter::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace SettingsSetterHelpers;
         
         if(ev->getType() == event_complete::event_id)
         {
            event_complete *event = static_cast<event_complete *>(ev.get_rep());
            finish();
            if(client_type::is_valid_instance(event->client))
               event->client->on_complete(this,event->outcome,event->settings_applied);
         }
      } // receive 
   };
};

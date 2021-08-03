/* Cora.PbRouter.PakbusSettingsGetter.cpp

Copyright (C) 2002, 2016 Campbell Scientific, Inc.

Written by: Jon Trauntvein
Date Begun: Wednesday 08 May 2002
Last Change: Friday 29 April 2016
Last Commit: $Date: 2016-05-03 16:18:03 -0600 (Tue, 03 May 2016) $ 
Last Changed by: $Author: jon $

*/

#pragma warning(disable:4786) // disable overlong symbols warning
#pragma hdrstop               // stop creation of precompiled header
#include "Cora.PbRouter.SettingsGetter.h"


namespace Cora
{
   namespace PbRouter
   {
      namespace SettingsGetterHelpers
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
            typedef SettingsGetterClient client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // outcome
            ////////////////////////////////////////////////////////////
            typedef client_type::outcome_type outcome_type;
            outcome_type outcome;

            ////////////////////////////////////////////////////////////
            // settings
            ////////////////////////////////////////////////////////////
            typedef client_type::settings_type settings_type;
            settings_type settings;

            ////////////////////////////////////////////////////////////
            // create
            ////////////////////////////////////////////////////////////
            static event_complete *create(
               SettingsGetter *getter,
               client_type *client,
               outcome_type outcome)
            { return new event_complete(getter,client,outcome); }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_complete(
               SettingsGetter *getter,
               client_type *client_,
               outcome_type outcome_):
               Event(event_id,getter),
               client(client_),
               outcome(outcome_)
            { } 
         };


         uint4 const event_complete::event_id =
         Csi::Event::registerType("Cora::PbRouter::SettingsGetter::event_complete"); 
      };


      ////////////////////////////////////////////////////////////
      // class SettingsGetter definitions
      ////////////////////////////////////////////////////////////
      SettingsGetter::SettingsGetter():
         client(0),
         state(state_standby),
         pakbus_address(0)
      { }


      SettingsGetter::~SettingsGetter()
      { finish(); }

      

      void SettingsGetter::set_pakbus_address(uint2 pakbus_address_)
      {
         if(state == state_standby)
            pakbus_address = pakbus_address_;
         else
            throw exc_invalid_state();
      } // set_pakbus_address


      void SettingsGetter::add_setting_name(StrAsc const &setting_name)
      {
         if(state == state_standby)
            setting_names.push_back(setting_name);
         else
            throw exc_invalid_state();
      } // add_setting_name

      
      void SettingsGetter::clear_setting_names()
      {
         if(state == state_standby)
            setting_names.clear();
         else
            throw exc_invalid_state();
      } // clear_setting_names

      
      void SettingsGetter::start(
         client_type *client_,
         router_handle &router)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
            {
               client= client_;
               state = state_delegate;
               PbRouterBase::start(router);
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start

      
      void SettingsGetter::start(
         client_type *client_,
         ClientBase *other_component)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
            {
                client= client_;
                state = state_delegate;
                PbRouterBase::start(other_component);
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start

      
      void SettingsGetter::finish()
      {
         client = 0;
         state = state_standby;
         PbRouterBase::finish();
      } // finish

      
      void SettingsGetter::on_pbrouterbase_ready()
      {
         Csi::Messaging::Message command(pbrouter_session,Messages::get_settings_cmd);
         command.addUInt4(++last_tran_no);
         command.addUInt2(pakbus_address);
         command.addUInt4((uint4)setting_names.size());
         for(setting_names_type::iterator si = setting_names.begin();
             si != setting_names.end();
             ++si)
            command.addStr(*si);
         state = state_active;
         router->sendMessage(&command);
      } // on_pbrouterbase_ready

      
      void SettingsGetter::on_pbrouterbase_failure(pbrouterbase_failure_type failure)
      {
         using namespace SettingsGetterHelpers;
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
         event_complete *event = event_complete::create(this,client,outcome);
         event->post();
      } // on_pbrouterbase_failure

      
      void SettingsGetter::onNetMessage(
         Csi::Messaging::Router *router,
         Csi::Messaging::Message *message)
      {
         if(state == state_active)
         {
            if(message->getMsgType() == Messages::get_settings_ack)
            {
               // read the message header and map the outcome
               using namespace SettingsGetterHelpers;
               uint4 tran_no;
               uint4 resp_code;
               uint4 settings_count;
               client_type::outcome_type outcome;
               
               message->readUInt4(tran_no);
               message->readUInt4(resp_code);
               message->readUInt4(settings_count);
               switch(resp_code)
               {
               case 1:
                  outcome = client_type::outcome_success;
                  break;
                  
               case 3:
                  outcome = client_type::outcome_communication_failed;
                  break;
                  
               case 4:
                  outcome = client_type::outcome_communication_disabled;
                  break;
                  
               case 5:
                  outcome = client_type::outcome_unreachable;
                  break;
                  
               default:
                  outcome = client_type::outcome_unknown;
                  break;
               }

               // read the setting values
               StrAsc setting_name, setting_value;
               event_complete *event = event_complete::create(this,client,outcome);
               
               for(uint4 i = 0; i < settings_count; ++i)
               {
                  message->readStr(setting_name);
                  message->readStr(setting_value);
                  event->settings.push_back(
                     client_type::setting_type(setting_name,setting_value));
               }
               event->post();
            }
            else
               PbRouterBase::onNetMessage(router,message);
         }
         else
            PbRouterBase::onNetMessage(router,message);
      } // onNetMessage

      
      void SettingsGetter::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace SettingsGetterHelpers;
         if(ev->getType() == event_complete::event_id)
         {
            event_complete *event = static_cast<event_complete *>(ev.get_rep());
            finish();
            if(client_type::is_valid_instance(event->client))
               event->client->on_complete(this,event->outcome,event->settings);
         }
      } // receive 
   };
};

/* Cora.LgrNet.DeviceDefaultSettingsLister.cpp

   Copyright (C) 2003, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 25 April 2003
   Last Change: Friday 29 April 2016
   Last Commit: $Date: 2016-05-03 16:18:03 -0600 (Tue, 03 May 2016) $ 
   Last Changed by: $Author: jon $

*/

#pragma warning(disable:4786) // disable overlong symbols warning
#pragma hdrstop               // stop creation of precompiled header
#include "Cora.LgrNet.DeviceDefaultSettingsLister.h"
#include "Cora.Device.DeviceSettingFactory.h"
#include "Cora.LgrNet.Defs.h"


namespace Cora
{
   namespace LgrNet
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
            // lister
            ////////////////////////////////////////////////////////////
            typedef DeviceDefaultSettingsLister lister_type;
            lister_type *lister;

            ////////////////////////////////////////////////////////////
            // client
            ////////////////////////////////////////////////////////////
            typedef lister_type::client_type client_type;
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

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_complete(
               lister_type *lister_,
               client_type *client_,
               outcome_type outcome_,
               settings_type settings_):
               Event(event_id,lister_),
               lister(lister_),
               client(client_),
               outcome(outcome_),
               settings(settings_)
            { }

         public:
            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               lister_type *lister,
               client_type *client,
               outcome_type outcome,
               settings_type settings = settings_type())
            {
               try{(new event_complete(lister,client,outcome,settings))->post();}
               catch(Csi::Event::BadPost &) { }
            }
         };


         uint4 const event_complete::event_id =
         Csi::Event::registerType("Cora::LgrNet::DeviceDefaultSettingsLister::event_complete");
      };

      
      ////////////////////////////////////////////////////////////
      // class DeviceDefaultSettingsLister definitions
      ////////////////////////////////////////////////////////////
      DeviceDefaultSettingsLister::DeviceDefaultSettingsLister():
         client(0),
         state(state_standby)
      { factory.bind(new Device::DeviceSettingFactory); }

      
      DeviceDefaultSettingsLister::~DeviceDefaultSettingsLister()
      { finish(); }

      
      void DeviceDefaultSettingsLister::start(
         client_type *client_,
         router_handle &router)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
            {
               if(factory != 0)
               {
                  client = client_;
                  state = state_delegate;
                  ClientBase::start(router);
               }
               else
                  throw std::invalid_argument("Invalid setting factory");
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start

      
      void DeviceDefaultSettingsLister::start(
         client_type *client_,
         ClientBase *other_component)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
            {
               if(factory != 0)
               {
                  client = client_;
                  state = state_delegate;
                  ClientBase::start(other_component);
               }
               else
                  throw std::invalid_argument("Invalid setting factory");
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start

      
      void DeviceDefaultSettingsLister::finish()
      {
         client = 0;
         state = state_standby;
         ClientBase::finish();
      } // finish

      
      void DeviceDefaultSettingsLister::on_corabase_ready()
      {
         Csi::Messaging::Message cmd(
            net_session,
            Messages::list_device_default_settings_cmd);
         cmd.addUInt4(++last_tran_no);
         cmd.addUInt4((uint4)context.size());
         for(context_type::iterator ci = context.begin();
             ci != context.end();
             ++ci)
            cmd.addUInt4(*ci);
         state = state_active;
         router->sendMessage(&cmd);
      } // on_corabase_ready

      
      void DeviceDefaultSettingsLister::on_corabase_failure(corabase_failure_type failure)
      {
         client_type::outcome_type outcome;
         switch(failure)
         {
         case corabase_failure_logon:
            outcome = client_type::outcome_invalid_logon;
            break;

         case corabase_failure_session:
            outcome = client_type::outcome_session_broken;
            break;

         case corabase_failure_unsupported:
            outcome = client_type::outcome_unsupported;
            break;

         case corabase_failure_security:
            outcome = client_type::outcome_server_security_blocked;
            break;

         default:
            outcome = client_type::outcome_unknown;
            break;
         }
         event_complete::cpost(this,client,outcome);
      } // on_corabase_failure

      
      void DeviceDefaultSettingsLister::on_corabase_session_failure()
      {
         event_complete::cpost(this,client,client_type::outcome_session_broken);
      } // on_corabase_session_failure

      
      void DeviceDefaultSettingsLister::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         if(ev->getType() == event_complete::event_id)
         {
            event_complete *event = static_cast<event_complete *>(ev.get_rep());
            client_type *client = this->client;
            finish();
            if(client == event->client && client_type::is_valid_instance(client))
               client->on_complete(this,event->outcome,event->settings);
         }
      } // receive

      
      void DeviceDefaultSettingsLister::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(state == state_active)
         {
            if(msg->getMsgType() == Messages::list_device_default_settings_ack)
            {
               // read the message header
               uint4 tran_no;
               uint4 server_outcome;
               uint4 settings_count;
               msg->readUInt4(tran_no);
               msg->readUInt4(server_outcome);
               msg->readUInt4(settings_count);

               if(server_outcome == 1)
               {
                  client_type::settings_type settings;

                  for(uint4 i = 0; i < settings_count; ++i)
                  {
                     client_type::setting_status_type setting;
                     uint4 setting_len;
                     uint4 default_code;
                     
                     msg->readUInt4(setting.setting_id);
                     msg->readUInt4(default_code);
                     if(default_code == client_type::setting_status_type::setting_known)
                        msg->readUInt4(setting_len);
                     setting.status =
                        static_cast<client_type::setting_status_type::status_type>(
                           default_code);
                     setting.setting = factory->make_setting(setting.setting_id);
                     if(setting.setting != 0)
                     {
                        if(default_code == client_type::setting_status_type::setting_known)
                           setting.setting->read(msg);
                        else
                           setting.setting.clear();
                        settings.push_back(setting);
                     }

                     if(setting.setting == 0 &&
                        setting.status == client_type::setting_status_type::setting_known)
                        msg->movePast(setting_len);
                  }

                  // we've now processed the complete response.  we can send the completion event to
                  // the client
                  event_complete::cpost(this,client,client_type::outcome_success,settings);
               }
               else
               {
                  if(server_outcome == 2)
                     event_complete::cpost(this,client,client_type::outcome_invalid_device_type);
                  else
                     event_complete::cpost(this,client,client_type::outcome_unknown);
               }
            }
            else
               ClientBase::onNetMessage(rtr,msg);
         }
         else
            ClientBase::onNetMessage(rtr,msg);
      } // onNetMessage 
   };
};


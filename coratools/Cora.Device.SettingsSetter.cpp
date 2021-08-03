/* Cora.Device.SettingsSetter.cpp

   Copyright (C) 2002, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 19 December 2002
   Last Change: Friday 27 April 2018
   Last Commit: $Date: 2018-04-28 09:41:03 -0600 (Sat, 28 Apr 2018) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.SettingsSetter.h"
#include "coratools.strings.h"
#include <algorithm>
#include <iostream>


namespace Cora
{
   namespace Device
   {
      namespace
      {
         class event_complete: public Csi::Event
         {
         public:
            static uint4 const event_id;
            SettingsSetterClient *client;
            typedef SettingsSetterClient::outcome_type outcome_type;
            outcome_type outcome;

            static void create_and_post(
               SettingsSetter *setter,
               SettingsSetterClient *client,
               outcome_type outcome)
            {
               try{ (new event_complete(setter,client,outcome))->post(); }
               catch(Csi::Event::BadPost &) { }
            }
               
         private:
            event_complete(SettingsSetter *setter,
                           SettingsSetterClient *client_,
                           outcome_type outcome_):
               Event(event_id,setter),
               client(client_),
               outcome(outcome_)
            { }
         };


         uint4 const event_complete::event_id =
         Csi::Event::registerType("Cora::Device::SettingsSetter::event_complete");
      };


      SettingsSetter::SettingsSetter():
         state(state_standby),
         client(0)
      { }


      SettingsSetter::~SettingsSetter()
      { finish(); }


      namespace
      {
         struct setting_has_id
         {
            uint4 const id;
            setting_has_id(uint4 id_): id(id_) { }
            bool operator ()(Csi::SharedPtr<Setting> &setting)
            { return setting->get_identifier() == id; }
         };
      };

      
      void SettingsSetter::add_setting(setting_handle setting)
      {
         if(state == state_standby)
         {
            settings_type::iterator si = std::find_if(
               settings.begin(), settings.end(), setting_has_id(setting->get_identifier()));
            if(si == settings.end())
               settings.push_back(setting);
            else if(*si != setting)
               *si = setting;
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
            if(!settings.empty())
            {
               if(client_type::is_valid_instance(client_))
               {
                  client = client_;
                  state = state_delegate;
                  DeviceBase::start(router);
               }
            }
            else
               throw std::invalid_argument("no settings specified");
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
            if(!settings.empty())
            {
               if(client_type::is_valid_instance(client_))
               {
                  client = client_;
                  state = state_delegate;
                  DeviceBase::start(other_component);
               }
            }
            else
               throw std::invalid_argument("no settings specified");
         }
         else
            throw exc_invalid_state();
      } // start


      void SettingsSetter::finish()
      {
         client = 0;
         state = state_standby;
         DeviceBase::finish();
      } // finish


      void SettingsSetter::describe_outcome(std::ostream &out, client_type::outcome_type outcome)
      {
         switch(outcome)
         {
         case client_type::outcome_unknown:
         default:
            format_devicebase_failure(out, devicebase_failure_unknown);
            break;
            
         case client_type::outcome_success:
            out << common_strings[common_success];
            break;
            
         case client_type::outcome_some_errors:
            out << "some setting errors";
            break;
            
         case client_type::outcome_invalid_logon:
            format_devicebase_failure(out, devicebase_failure_logon);
            break;
            
         case client_type::outcome_invalid_device_name:
            format_devicebase_failure(out, devicebase_failure_invalid_device_name);
            break;
            
         case client_type::outcome_session_failed:
            format_devicebase_failure(out, devicebase_failure_session);
            break;
            
         case client_type::outcome_security_blocked:
            format_devicebase_failure(out, devicebase_failure_security);
            break;
            
         case client_type::outcome_unsupported:
            format_devicebase_failure(out, devicebase_failure_unsupported);
            break;
            
         case client_type::outcome_network_locked:
            out << "network locked";
            break;
         }
      } // describe_outcome


      void SettingsSetter::on_devicebase_ready()
      {
         Csi::Messaging::Message command(device_session,Messages::settings_set_cmd);

         command.addUInt4(++last_tran_no);
         command.addUInt4((uint4)settings.size());
         for(settings_type::iterator si = settings.begin();
             si != settings.end();
             ++si)
         {
            uint4 setting_len_pos;
            uint4 setting_pos;
            setting_handle &setting = *si;
            
            command.addUInt4(setting->get_identifier());
            setting_len_pos = command.getBodyLen();
            command.addUInt4(0);
            setting_pos = command.getBodyLen();
            setting->write(&command);
            command.replaceUInt4(command.getBodyLen() - setting_pos,setting_len_pos); 
         }
         router->sendMessage(&command);
         state = state_active;
      } // on_devicebase_ready


      void SettingsSetter::on_devicebase_failure(devicebase_failure_type failure)
      {
         client_type::outcome_type outcome;
         switch(failure)
         {
         case devicebase_failure_logon:
            outcome = client_type::outcome_invalid_logon;
            break;
            
         case devicebase_failure_session:
            outcome = client_type::outcome_session_failed;
            break;
            
         case devicebase_failure_invalid_device_name:
            outcome = client_type::outcome_invalid_device_name;
            break;
            
         case devicebase_failure_unsupported:
            outcome = client_type::outcome_unsupported;
            break;
            
         case devicebase_failure_security:
            outcome = client_type::outcome_security_blocked;
            break;
            
         default:
            outcome = client_type::outcome_unknown;
            break;
         }
         event_complete::create_and_post(this,client,outcome);
      } // on_devicebase_failure


      void SettingsSetter::on_devicebase_session_failure()
      { event_complete::create_and_post(this,client,client_type::outcome_session_failed); }


      void SettingsSetter::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(state == state_active)
         {
            if(msg->getMsgType() == Messages::settings_set_ack)
            {
               uint4 tran_no;
               uint4 count;
               settings_type::iterator si = settings.begin();

               msg->readUInt4(tran_no);
               msg->readUInt4(count);
               if(count == settings.size())
               {
                  client_type::outcome_type outcome = client_type::outcome_success;
                  
                  for(settings_type::iterator si = settings.begin();
                      si != settings.end();
                      ++si)
                  {
                     uint4 setting_id;
                     uint4 resp_code;
                     setting_handle &setting = *si;
                     
                     msg->readUInt4(setting_id);
                     msg->readUInt4(resp_code);
                     if(setting->get_identifier() == setting_id)
                     {
                        if(resp_code != 0)
                        {
                           outcome = client_type::outcome_some_errors;
                           if(resp_code == 4)
                              outcome = client_type::outcome_network_locked;
                        }
                        switch(resp_code)
                        {
                        case 0:
                           setting->set_set_outcome(Setting::outcome_set);
                           break;

                        case 1:
                           setting->set_set_outcome(Setting::outcome_unsupported);
                           break;

                        case 2:
                           setting->set_set_outcome(Setting::outcome_invalid_value);
                           break;

                        case 3:
                           setting->set_set_outcome(Setting::outcome_read_only);
                           break;

                        case 4:
                           setting->set_set_outcome(Setting::outcome_network_locked);
                           break;

                        default:
                           setting->set_set_outcome(Setting::outcome_no_attempt_made);
                           break;
                        }
                     }
                     else
                     {
                        event_complete::create_and_post(
                           this,client,client_type::outcome_unknown);
                        break;
                     }
                  }
                  event_complete::create_and_post(this,client,outcome);
               }
               else
               {
                  event_complete::create_and_post(
                     this,client,client_type::outcome_unknown);
               }
            }
            else
               DeviceBase::onNetMessage(rtr,msg);
         }
         else
            DeviceBase::onNetMessage(rtr,msg);
      } // onNetMessage


      void SettingsSetter::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         if(ev->getType() == event_complete::event_id)
         {
            event_complete *event = static_cast<event_complete *>(ev.get_rep());
            if(event->client == client && client_type::is_valid_instance(client))
            {
               finish();
               event->client->on_complete(this,event->outcome);
            }
            else
               finish();
         }
      } // receive
   };
};

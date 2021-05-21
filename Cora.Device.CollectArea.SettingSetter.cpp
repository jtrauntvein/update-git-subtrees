/* Cora.Device.CollectArea.SettingSetter.cpp

   Copyright (C) 2000, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 02 November 2000
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Committed by: $Author: jon $
   
*/

#pragma warning(disable:4786) // disable overlong symbols warning
#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.CollectArea.SettingSetter.h"
#include <assert.h>

namespace Cora
{
   namespace Device
   {
      namespace CollectArea
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
               SettingSetterClient *client;
               typedef SettingSetterClient::outcome_type outcome_type;
               outcome_type outcome;
               
            private:
               event_complete(
                  SettingSetter *setter,
                  SettingSetterClient *client_,
                  outcome_type outcome_):
                  Event(event_id,setter),
                  client(client_),
                  outcome(outcome_)
               { }

            public:
               static void create_and_post(
                  SettingSetter *setter,
                  SettingSetterClient *client,
                  outcome_type outcome)
               {
                  try {(new event_complete(setter,client,outcome))->post(); }
                  catch(Event::BadPost &) { }
               }
            };


            uint4 const event_complete::event_id =
            Csi::Event::registerType("Cora::Device::CollectArea::SettingSetter::event_complete");
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

         
         void SettingSetter::set_collect_area_name(StrUni const &collect_area_name_)
         {
            if(state == state_standby)
               collect_area_name = collect_area_name_;
            else
               throw exc_invalid_state();
         } // set_collect_area_name

         
         void SettingSetter::set_the_setting(Csi::SharedPtr<Setting> the_setting_)
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
               if(SettingSetterClient::is_valid_instance(client_))
               {
                  if(collect_area_name.length())
                  {
                     if(the_setting.get_rep())
                     {
                        state = state_delegate;
                        client = client_;
                        DeviceBase::start(router);
                     }
                     else
                        throw std::invalid_argument("invalid setting");
                  }
                  else
                     throw std::invalid_argument("Invalid collect area name");
               }
               else
                  throw std::invalid_argument("Invalid client pointer");
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
               if(SettingSetterClient::is_valid_instance(client_))
               {
                  if(collect_area_name.length())
                  {
                     if(the_setting.get_rep())
                     {
                        state = state_delegate;
                        client = client_;
                        DeviceBase::start(other_component);
                     }
                     else
                        throw std::invalid_argument("invalid setting");
                  }
                  else
                     throw std::invalid_argument("Invalid collect area name");
               }
               else
                  throw std::invalid_argument("Invalid client pointer");
            }
            else
               throw exc_invalid_state();
         } // start

         
         void SettingSetter::finish()
         {
            client = 0;
            state = state_standby;
            DeviceBase::finish();
         } // finish

         
         void SettingSetter::on_devicebase_ready()
         {
            Csi::Messaging::Message command(
               device_session,
               Messages::collect_area_settings_set_cmd);
            uint4 setting_len_pos;
            uint4 setting_pos;
            
            command.addUInt4(++last_tran_no);
            command.addWStr(collect_area_name);
            command.addUInt4(1); // we will set only one setting
            command.addUInt4(the_setting->get_identifier());
            setting_len_pos = command.getBodyLen();
            command.addUInt4(0); // placeholder for length
            setting_pos = command.getBodyLen();
            the_setting->write(&command);
            command.replaceUInt4(command.getBodyLen() - setting_pos,setting_len_pos);
            state = state_active;
            router->sendMessage(&command);
         } // on_devicebase_ready

         
         void SettingSetter::on_devicebase_failure(devicebase_failure_type failure)
         {
            using namespace SettingSetterHelpers;
            SettingSetterClient::outcome_type outcome;
            switch(failure)
            {
            case devicebase_failure_logon:
               outcome = SettingSetterClient::outcome_invalid_logon;
               break;
               
            case devicebase_failure_session:
               outcome = SettingSetterClient::outcome_session_failed;
               break;
               
            case devicebase_failure_invalid_device_name:
               outcome = SettingSetterClient::outcome_invalid_device_name;
               break;
               
            case devicebase_failure_unsupported:
               outcome = SettingSetterClient::outcome_unsupported;
               break;
               
            case devicebase_failure_security:
               outcome = SettingSetterClient::outcome_security_blocked;
               break;
               
            default:
               outcome = SettingSetterClient::outcome_unknown;
               break;
            }
            event_complete::create_and_post(this,client,outcome);            
         } // on_devicebase_failure

         
         void SettingSetter::on_devicebase_session_failure()
         {
            using namespace SettingSetterHelpers;
            event_complete::create_and_post(
               this,
               client,
               SettingSetterClient::outcome_session_failed);
         } // on_devicebase_session_failure

         
         void SettingSetter::onNetMessage(
            Csi::Messaging::Router *rtr,
            Csi::Messaging::Message *msg)
         {
            if(state == state_active)
            {
               if(msg->getMsgType() == Messages::collect_area_settings_set_ack)
               {
                  using namespace SettingSetterHelpers;
                  uint4 tran_no;
                  uint4 resp_code;
                  uint4 count;
                  SettingSetterClient::outcome_type outcome;
                  
                  msg->readUInt4(tran_no);
                  msg->readUInt4(resp_code);
                  msg->readUInt4(count);
                  if(resp_code == 1)
                  {
                     uint4 setting_id;
                     uint4 result;

                     assert(count == 1);
                     msg->readUInt4(setting_id);
                     msg->readUInt4(result);
                     assert(setting_id == the_setting->get_identifier());
                     switch(result)
                     {
                     case 1:
                        outcome = SettingSetterClient::outcome_success;
                        the_setting->set_set_outcome(Setting::outcome_set);
                        break;

                     case 2:
                        outcome = SettingSetterClient::outcome_unsupported_setting;
                        the_setting->set_set_outcome(Setting::outcome_unsupported);
                        break;

                     case 3:
                        outcome = SettingSetterClient::outcome_invalid_setting_value;
                        the_setting->set_set_outcome(Setting::outcome_invalid_value);
                        break;

                     case 4:
                        outcome = SettingSetterClient::outcome_setting_read_only;
                        the_setting->set_set_outcome(Setting::outcome_read_only);
                        break;

                     default:
                        outcome = SettingSetterClient::outcome_unknown;
                        the_setting->set_set_outcome(Setting::outcome_no_attempt_made);
                        break;
                     }
                  }
                  else if(resp_code == 2)
                     outcome = SettingSetterClient::outcome_invalid_collect_area_name;
                  else if(resp_code == 4)
                     outcome = SettingSetterClient::outcome_network_locked;
                  else
                     outcome = SettingSetterClient::outcome_unknown;
                  event_complete::create_and_post(this,client,outcome);
               }
               else
                  DeviceBase::onNetMessage(rtr,msg);
            }
            else
               DeviceBase::onNetMessage(rtr,msg);
         } // onNetMessage

         
         void SettingSetter::receive(Csi::SharedPtr<Csi::Event> &ev)
         {
            using namespace SettingSetterHelpers;
            if(ev->getType() == event_complete::event_id)
            {
               event_complete *event = static_cast<event_complete *>(ev.get_rep());
               finish();
               if(SettingSetterClient::is_valid_instance(event->client))
                  event->client->on_complete(this,event->outcome);
            }
         } // receive 
      };
   };
};

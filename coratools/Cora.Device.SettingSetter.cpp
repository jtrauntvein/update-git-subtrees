/* Cora.Device.SettingSetter.cpp

   Copyright (C) 2000, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 01 August 2000
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Committed by: $Author: jon $
   
*/

#pragma warning(disable:4786) // disable overlong symbols warning
#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.SettingSetter.h"
#include <assert.h>


namespace Cora
{
   namespace Device
   {
      namespace SettingSetterHelpers
      {
         ////////////////////////////////////////////////////////////
         // class event_complete
         ////////////////////////////////////////////////////////////
         class event_complete: public Csi::Event
         {
         public:
            ////////// event_id
            static uint4 const event_id;

            ////////// client
            SettingSetterClient *client;

            ////////// resp_code
            typedef SettingSetterClient::resp_code_type resp_code_type;
            resp_code_type resp_code;

            ////////// create_and_post
            static void create_and_post(SettingSetter *setter,
                                        SettingSetterClient *client,
                                        resp_code_type resp_code);
               
         private:
            event_complete(SettingSetter *setter,
                           SettingSetterClient *client_,
                           resp_code_type resp_code_):
               Event(event_id,setter),
               client(client_),
               resp_code(resp_code_)
            { }
         };


         uint4 const event_complete::event_id =
         Csi::Event::registerType("Cora::Device::SettingSetter::event_complete");


         void event_complete::create_and_post(SettingSetter *setter,
                                              SettingSetterClient *client,
                                              resp_code_type resp_code)
         {
            try { (new event_complete(setter,client,resp_code))->post(); }
            catch(Csi::Event::BadPost &) { }
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
            if(the_setting.get_rep() != 0)
            {
               if(SettingSetterClient::is_valid_instance(client_))
               {
                  client = client_;
                  state = state_delegate;
                  DeviceBase::start(router);
               }
               else
                  throw std::invalid_argument("Invalid client pointer");
            }
            else
               throw std::invalid_argument("Invalid setting handle");
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
                  DeviceBase::start(other_component);
               }
               else
                  throw std::invalid_argument("Invalid client pointer");
            }
            else
               throw std::invalid_argument("Invalid setting handle");
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
         Csi::Messaging::Message command(device_session,Messages::settings_set_cmd);
         uint4 setting_len_pos;
         uint4 setting_pos;

         command.addUInt4(++last_tran_no);
         command.addUInt4(1);   // we will send one setting
         command.addUInt4(the_setting->get_identifier());
         setting_len_pos = command.getBodyLen();
         command.addUInt4(0);   // placeholder for the length
         setting_pos = command.getBodyLen();
         the_setting->write(&command);
         command.replaceUInt4(command.getBodyLen() - setting_pos,setting_len_pos);
         router->sendMessage(&command);
         state = state_active;
      } // on_devicebase_ready

      
      void SettingSetter::on_devicebase_failure(devicebase_failure_type failure)
      {
         using namespace SettingSetterHelpers;
         SettingSetterClient::resp_code_type resp_code;
         switch(failure)
         {
         case devicebase_failure_logon:
            resp_code = SettingSetterClient::resp_invalid_logon;
            break;
            
         case devicebase_failure_session:
            resp_code = SettingSetterClient::resp_session_failed;
            break;
            
         case devicebase_failure_invalid_device_name:
            resp_code = SettingSetterClient::resp_invalid_device_name;
            break;
            
         case devicebase_failure_unsupported:
            resp_code = SettingSetterClient::resp_unsupported;
            break;
            
         case devicebase_failure_security:
            resp_code = SettingSetterClient::resp_security_blocked;
            break;
            
         default:
            resp_code = SettingSetterClient::resp_unknown;
            break;
         }
         event_complete::create_and_post(this,client,resp_code);
      } // on_devicebase_failure

      
      void SettingSetter::on_devicebase_session_failure()
      {
         using namespace SettingSetterHelpers;
         event_complete::create_and_post(this,client,SettingSetterClient::resp_session_failed);
      } // on_devicebase_session_failure

      
      void SettingSetter::onNetMessage(Csi::Messaging::Router *rtr,
                                       Csi::Messaging::Message *msg)
      {
         if(state == state_active)
         {
            if(msg->getMsgType() == Messages::settings_set_ack)
            {
               // read the message contents
               uint4 tran_no;
               uint4 count;
               uint4 setting_id;
               uint4 resp_code;

               msg->readUInt4(tran_no);
               msg->readUInt4(count);
               assert(count == 1);
               msg->readUInt4(setting_id);
               assert(setting_id == the_setting->get_identifier());
               msg->readUInt4(resp_code);

               // interpret the results
               using namespace SettingSetterHelpers;
               SettingSetterClient::resp_code_type client_resp_code;
               switch(resp_code)
               {
               case 0:
                  client_resp_code = SettingSetterClient::resp_success;
                  the_setting->set_set_outcome(Setting::outcome_set); 
                  break;
                  
               case 1:
                  client_resp_code = SettingSetterClient::resp_unsupported_setting;
                  the_setting->set_set_outcome(Setting::outcome_unsupported);
                  break;
                  
               case 2:
                  client_resp_code = SettingSetterClient::resp_invalid_setting_value;
                  the_setting->set_set_outcome(Setting::outcome_invalid_value);
                  break;
                  
               case 3:
                  client_resp_code = SettingSetterClient::resp_setting_read_only;
                  the_setting->set_set_outcome(Setting::outcome_read_only);
                  break;

               case 4:
                  client_resp_code = SettingSetterClient::resp_network_locked;
                  the_setting->set_set_outcome(Setting::outcome_network_locked);
                  break;
                  
               default:
                  client_resp_code = SettingSetterClient::resp_unknown;
                  the_setting->set_set_outcome(Setting::outcome_no_attempt_made);
                  break;
               }
               event_complete::create_and_post(this,client,client_resp_code);
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
         event_complete *event = dynamic_cast<event_complete *>(ev.get_rep());
         assert(event != 0);
         finish();
         if(SettingSetterClient::is_valid_instance(event->client))
            event->client->on_complete(this,event->resp_code);
      } // receive
   };
};

/* Cora.Device.CollectArea.SettingsEnumerator.cpp

   Copyright (C) 2000, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 31 October 2000
   Last Change: Friday 18 October 2019
   Last Commit: $Date: 2019-10-18 17:46:39 -0600 (Fri, 18 Oct 2019) $ 
   Committed by : $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.CollectArea.SettingsEnumerator.h"
#include "Cora.Device.CollectArea.CollectAreaSettingFactory.h"
#include "coratools.strings.h"


namespace Cora
{
   namespace Device
   {
      namespace CollectArea
      {
         namespace SettingsEnumeratorHelpers
         {
            class event_base: public Csi::Event
            {
            public:
               SettingsEnumeratorClient *client;
               SettingsEnumerator *receiver;
               virtual void send() = 0;
               
            protected:
               event_base(
                  uint4 event_id,
                  SettingsEnumeratorClient *client_,
                  SettingsEnumerator *receiver_):
                  Event(event_id,receiver_),
                  client(client_),
                  receiver(receiver_)
               { }
            };


            class event_started: public event_base
            {
            public:
               static uint4 const event_id;
               
            private:
               event_started(
                  SettingsEnumeratorClient *client,
                  SettingsEnumerator *receiver):
                  event_base(event_id,client,receiver)
               { }

            public:
               virtual void send()
               { client->on_started(receiver); }
               
               static void create_and_post(
                  SettingsEnumeratorClient *client,
                  SettingsEnumerator *receiver)
               {
                  try { (new event_started(client,receiver))->post(); }
                  catch(Event::BadPost &) { }
               } 
            };


            uint4 const event_started::event_id =
            Csi::Event::registerType("Cora::Device::SettingsEnumerator::event_started");


            class event_failure: public event_base
            {
            public:
               static uint4 const event_id;
               typedef SettingsEnumeratorClient::failure_type failure_type;
               failure_type failure;

            private:
               event_failure(
                  SettingsEnumeratorClient *client,
                  SettingsEnumerator *receiver,
                  failure_type failure_):
                  event_base(event_id,client,receiver),
                  failure(failure_)
               { }

            public:
               virtual void send()
               { client->on_failure(receiver,failure); }

               static void create_and_post(
                  SettingsEnumeratorClient *client,
                  SettingsEnumerator *receiver,
                  failure_type failure)
               {
                  try { (new event_failure(client,receiver,failure))->post(); }
                  catch(Event::BadPost &) { }
               }
            };


            uint4 const event_failure::event_id =
            Csi::Event::registerType("Cora::Device::CollectArea::SettingsEnumerator::event_failure");


            class event_setting_changed: public event_base
            {
            public:
               static uint4 const event_id;
               Csi::SharedPtr<Setting> setting;

               virtual void send()
               { client->on_setting_changed(receiver,setting); }

            private:
               event_setting_changed(
                  SettingsEnumeratorClient *client,
                  SettingsEnumerator *receiver,
                  Csi::SharedPtr<Setting> &setting_):
                  event_base(event_id,client,receiver),
                  setting(setting_)
               { }

            public:
               static void create_and_post(
                  SettingsEnumeratorClient *client,
                  SettingsEnumerator *receiver,
                  Csi::SharedPtr<Setting> &setting)
               {
                  try {(new event_setting_changed(client,receiver,setting))->post(); }
                  catch(Event::BadPost &) { }
               }
            };


            uint4 const event_setting_changed::event_id =
            Csi::Event::registerType(
               "Cora::Device::CollectArea::SettingsEnumerator::event_setting_changed");

         };

         
         SettingsEnumerator::SettingsEnumerator():
            state(state_standby),
            client(0)
         { setting_factory.bind(new CollectAreaSettingFactory); }

         
         SettingsEnumerator::~SettingsEnumerator()
         { finish(); }

         
         void SettingsEnumerator::set_collect_area_name(StrUni const &collect_area_name_)
         {
            if(state == state_standby)
               collect_area_name = collect_area_name_;
            else
               throw exc_invalid_state();
         } // set_collect_area_name

         
         void SettingsEnumerator::set_setting_factory(
            Csi::SharedPtr<SettingFactory> setting_factory_)
         {
            if(state == state_standby)
               setting_factory = setting_factory_;
            else
               throw exc_invalid_state();
         } // set_setting_factory

         
         void SettingsEnumerator::start(
            SettingsEnumeratorClient *client_,
            router_handle &router)
         {
            if(state == state_standby)
            {
               if(SettingsEnumeratorClient::is_valid_instance(client_))
               {
                  if(collect_area_name.length())
                  {
                     if(setting_factory.get_rep() != 0)
                     {
                        client = client_;
                        state = state_delegate;
                        DeviceBase::start(router);
                     }
                     else
                        throw std::invalid_argument("Invalid setting factory");
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


         void SettingsEnumerator::start(
            SettingsEnumeratorClient *client_,
            ClientBase *other_component)
         {
            if(state == state_standby)
            {
               if(SettingsEnumeratorClient::is_valid_instance(client_))
               {
                  if(collect_area_name.length())
                  {
                     if(setting_factory.get_rep() != 0)
                     {
                        client = client_;
                        state = state_delegate;
                        DeviceBase::start(other_component);
                     }
                     else
                        throw std::invalid_argument("Invalid setting factory");
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

         
         void SettingsEnumerator::finish()
         {
            client = 0;
            state = state_standby;
            DeviceBase::finish();
         } // finish


         void SettingsEnumerator::describe_failure(std::ostream &out, client_type::failure_type failure)
         {
            switch(failure)
            {
            case client_type::failure_connection_failed:
               format_devicebase_failure(out, devicebase_failure_session);
               break;
               
            case client_type::failure_invalid_logon:
               format_devicebase_failure(out, devicebase_failure_logon);
               break;
               
            case client_type::failure_server_security_blocked:
               format_devicebase_failure(out, devicebase_failure_security);
               break;
               
            case client_type::failure_device_name_invalid:
               format_devicebase_failure(out, devicebase_failure_invalid_device_name);
               break;
               
            case client_type::failure_collect_area_name_invalid:
               out << common_strings[common_invalid_collect_area_name];
               break;
               
            default:
               format_devicebase_failure(out, devicebase_failure_unknown);
               break;
            }
         } // describe_failure

         
         void SettingsEnumerator::receive(Csi::SharedPtr<Csi::Event> &ev)
         {
            using namespace SettingsEnumeratorHelpers;
            event_base *event = static_cast<event_base *>(ev.get_rep());
            if(event->getType() == event_failure::event_id)
               finish();
            if(SettingsEnumeratorClient::is_valid_instance(event->client))
               event->send();
            else
               finish();
         } // receive

         
         void SettingsEnumerator::onNetMessage(
            Csi::Messaging::Router *rtr,
            Csi::Messaging::Message *msg)
         {
            if(state == state_active || state == state_before_active)
            {
               switch(msg->getMsgType())
               {
               case Cora::Device::Messages::collect_area_settings_enum_not:
                  on_enum_not(msg);
                  break;
                  
               case Cora::Device::Messages::collect_area_settings_enum_stopped_not:
                  on_stopped_not(msg);
                  break;
                  
               default:
                  DeviceBase::onNetMessage(rtr,msg);
                  break;
               }
            }
            else
               DeviceBase::onNetMessage(rtr,msg);
         } // onNetMessage

         
         void SettingsEnumerator::on_devicebase_failure(
            devicebase_failure_type failure)
         {
            SettingsEnumeratorClient::failure_type client_failure;
         
            switch(failure)
            {
            case devicebase_failure_logon:
               client_failure = SettingsEnumeratorClient::failure_invalid_logon;
               break;
               
            case devicebase_failure_session:
               client_failure = SettingsEnumeratorClient::failure_connection_failed;
               break;
               
            case devicebase_failure_invalid_device_name:
               client_failure = SettingsEnumeratorClient::failure_device_name_invalid;
               break;

            default:
               client_failure = SettingsEnumeratorClient::failure_unknown;
               break;
            }
            SettingsEnumeratorHelpers::event_failure::create_and_post(client,this,client_failure);
         } // on_devicebase_failure

         
         void SettingsEnumerator::on_devicebase_ready()
         {
            state = state_before_active;
            Csi::Messaging::Message command(
               device_session,
               Cora::Device::Messages::collect_area_settings_enum_start_cmd);

            command.addUInt4(++last_tran_no);
            command.addWStr(collect_area_name); 
            router->sendMessage(&command);
         } // on_devicebase_ready

         
         void SettingsEnumerator::on_setting_read(
            Csi::SharedPtr<Setting> &setting,
            uint4 context_token)
         {
            using namespace SettingsEnumeratorHelpers;
            event_setting_changed::create_and_post(client,this,setting);
         } // on_setting_read

         
         void SettingsEnumerator::on_enum_not(Csi::Messaging::Message *message)
         {
            uint4 tran_no;
            message->readUInt4(tran_no);
            read_settings(message,setting_factory.get_rep(),0);
            if(state == state_before_active)
            {
               SettingsEnumeratorHelpers::event_started::create_and_post(client,this);
               state = state_active;
            }
         } // on_enum_not

         
         void SettingsEnumerator::on_stopped_not(Csi::Messaging::Message *message)
         {
            uint4 tran_no;
            uint4 reason;
            SettingsEnumeratorClient::failure_type failure;

            message->readUInt4(tran_no);
            message->readUInt4(reason);
            if(reason == 3)
               failure = SettingsEnumeratorClient::failure_collect_area_name_invalid;
            else
               failure = SettingsEnumeratorClient::failure_unknown;
            SettingsEnumeratorHelpers::event_failure::create_and_post(client,this,failure);
         } // on_stopped_not
      };
   };
};

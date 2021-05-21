/* Cora.Device.SettingsEnumerator.cpp

   Copyright (C) 2000, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 21 July 2000
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Committed by: $Author: jon $
   
*/

#pragma warning(disable:4786) // disable overlong symbols warning
#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.SettingsEnumerator.h"
#include "Cora.Device.DeviceSettingFactory.h"

namespace Cora
{
   namespace Device
   {
      namespace SettingsEnumeratorHelpers
      {
         ////////////////////////////////////////////////////////////
         // class event_started declaration and definitions
         ////////////////////////////////////////////////////////////

         class event_base: public Csi::Event
         {
         public:
            ////////// client
            SettingsEnumeratorClient *client;

            ////////// receiver
            SettingsEnumerator *receiver;
            
            ////////// constructor
            event_base(uint4 event_id,
                       SettingsEnumerator *receiver_,
                       SettingsEnumeratorClient *client_):
               Event(event_id,receiver_),
               client(client_),
               receiver(receiver_)
            { }

            ////////// send_notification
            // Sends the appropriate notification to the client
            virtual void send_notification() = 0;
         };

         class event_started: public event_base
         {
         public:
            static uint4 const event_id;
            
            static void create_and_post(SettingsEnumerator *receiver,
                                        SettingsEnumeratorClient *client);

            virtual void send_notification()
            { client->on_started(receiver); }

         private:
            event_started(SettingsEnumerator *receiver,
                          SettingsEnumeratorClient *client):
               event_base(event_id,receiver,client)
            { }
         };

         uint4 const event_started::event_id  =
         Csi::Event::registerType("Cora::Device::SettingsEnumerator::event_started");


         void event_started::create_and_post(SettingsEnumerator *receiver,
                                             SettingsEnumeratorClient *client)
         {
            try { (new event_started(receiver,client))->post(); }
            catch(Csi::Event::BadPost &) { }
         } // create_and_post

         
         ////////////////////////////////////////////////////////////
         // class event_failure declaration and definitions
         ////////////////////////////////////////////////////////////

         class event_failure: public event_base
         {
         public:
            static uint4 const event_id;
            SettingsEnumeratorClient::failure_type failure;

            static void create_and_post(SettingsEnumerator *receiver,
                                        SettingsEnumeratorClient *client,
                                        SettingsEnumeratorClient::failure_type failure);

            virtual void send_notification()
            { client->on_failure(receiver,failure); }

         private:
            event_failure(SettingsEnumerator *receiver,
                          SettingsEnumeratorClient *client,
                          SettingsEnumeratorClient::failure_type failure_):
               event_base(event_id,receiver,client),
               failure(failure_)
            { }
         };


         uint4 const event_failure::event_id =
         Csi::Event::registerType("Cora::Device::SettingsEnumerator::event_failure");


         void event_failure::create_and_post(SettingsEnumerator *receiver,
                                             SettingsEnumeratorClient *client,
                                             SettingsEnumeratorClient::failure_type failure)
         {
            try { (new event_failure(receiver,client,failure))->post(); }
            catch(Csi::Event::BadPost &) { }
         } // create_and_post


         ////////////////////////////////////////////////////////////
         // class event_setting_changed declaration and definitions
         ////////////////////////////////////////////////////////////

         class event_setting_changed: public event_base
         {
         public:
            static uint4 const event_id;
            Csi::SharedPtr<Setting> setting;

            static void create_and_post(SettingsEnumerator *receiver,
                                        SettingsEnumeratorClient *client,
                                        Csi::SharedPtr<Setting> &setting);

            virtual void send_notification()
            { client->on_setting_changed(receiver,setting); }

         private:
            event_setting_changed(SettingsEnumerator *receiver,
                                  SettingsEnumeratorClient *client,
                                  Csi::SharedPtr<Setting> &setting_):
               event_base(event_id,receiver,client),
               setting(setting_)
            { }
         };


         uint4 const event_setting_changed::event_id =
         Csi::Event::registerType("Cora::Device::SettingsEnumerator::event_setting_changed");


         void event_setting_changed::create_and_post(SettingsEnumerator *receiver,
                                                     SettingsEnumeratorClient *client,
                                                     Csi::SharedPtr<Setting> &setting)
         {
            try { (new event_setting_changed(receiver,client,setting))->post(); }
            catch(Csi::Event::BadPost &) { }
         } // create_and_post


         ////////////////////////////////////////////////////////////
         // class event_setting_overridden declarations and definition
         ////////////////////////////////////////////////////////////

         class event_setting_overridden: public event_base
         {
         public:
            static uint4 const event_id;
            Csi::SharedPtr<Setting> setting;

            static void create_and_post(SettingsEnumerator *receiver,
                                        SettingsEnumeratorClient *client,
                                        Csi::SharedPtr<Setting> &setting);

            virtual void send_notification()
            { client->on_setting_overridden(receiver,setting); }
            
         private:
            event_setting_overridden(SettingsEnumerator *receiver,
                                     SettingsEnumeratorClient *client,
                                     Csi::SharedPtr<Setting> &setting_):
               event_base(event_id,receiver,client),
               setting(setting_)
            { }
         };


         uint4 const event_setting_overridden::event_id =
         Csi::Event::registerType("Cora::Device::SettingsEnumerator::event_setting_overridden");


         void event_setting_overridden::create_and_post(SettingsEnumerator *receiver,
                                                        SettingsEnumeratorClient *client,
                                                        Csi::SharedPtr<Setting> &setting)
         {
            try { (new event_setting_overridden(receiver,client,setting))->post(); }
            catch(Csi::Event::BadPost &) { }
         } // create_and_post


         ////////////////////////////////////////////////////////////
         // class event_override_stopped declaration and definitions
         ////////////////////////////////////////////////////////////

         class event_override_stopped: public event_base
         {
         public:
            static uint4 const event_id;

            static void create_and_post(SettingsEnumerator *receiver,
                                        SettingsEnumeratorClient *client);

            virtual void send_notification()
            { client->on_override_stopped(receiver); }

         private:
            event_override_stopped(SettingsEnumerator *receiver,
                                   SettingsEnumeratorClient *client):
               event_base(event_id,receiver,client)
            { }
         };


         uint4 const event_override_stopped::event_id =
         Csi::Event::registerType("Cora::Device::SettingsEnumerator::event_override_stopped");


         void event_override_stopped::create_and_post(SettingsEnumerator *receiver,
                                                      SettingsEnumeratorClient *client)
         {
            try { (new event_override_stopped(receiver,client))->post(); }
            catch(Csi::Event::BadPost &) { }
         } // create_and_post
      };

      ////////////////////////////////////////////////////////////
      // class SettingsEnumerator definitions
      ////////////////////////////////////////////////////////////

      SettingsEnumerator::SettingsEnumerator():
         state(state_standby),
         client(0)
      { setting_factory.bind(new DeviceSettingFactory); }

      
      SettingsEnumerator::~SettingsEnumerator()
      { }

      
      void SettingsEnumerator::set_setting_factory(Csi::SharedPtr<SettingFactory> factory)
      {
         if(state == state_standby)
            setting_factory = factory;
         else
            throw exc_invalid_state();
      } // set_setting_factory


      void SettingsEnumerator::set_setting_factory(SettingFactory *factory)
      { set_setting_factory(Csi::SharedPtr<SettingFactory>(factory)); }

      
      void SettingsEnumerator::start(
         SettingsEnumeratorClient *client_,
         router_handle &router)
      {
         if(state == state_standby)
         {
            if(SettingsEnumeratorClient::is_valid_instance(client_))
            {
               state = state_delegate;
               client = client_;
               DeviceBase::start(router);
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
               state = state_delegate;
               client = client_;
               DeviceBase::start(other_component);
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start

      
      void SettingsEnumerator::finish()
      {
         ignored_settings.clear();
         state = state_standby;
         client = 0;
         DeviceBase::finish();
      } // finish


      bool SettingsEnumerator::is_setting_ignored(uint4 setting_id) const
      {
         bool rtn = false;
         if(state == state_active || state == state_before_active)
         {
            rtn = ignored_settings.find(setting_id) != ignored_settings.end();
         }
         else
            throw exc_invalid_state();
         return rtn;
      } // is_setting_ignored


      void SettingsEnumerator::format_failure(
         std::ostream &out, client_type::failure_type failure)
      {
         switch(failure)
         {
         case client_type::failure_unknown:
            ClientBase::describe_failure(out, corabase_failure_unknown);
            break;
            
         case client_type::failure_connection_failed:
            ClientBase::describe_failure(out, corabase_failure_session);
            break;
            
         case client_type::failure_invalid_logon:
            ClientBase::describe_failure(out, corabase_failure_logon);
            break;
            
         case client_type::failure_server_security_blocked:
            ClientBase::describe_failure(out, corabase_failure_security);
            break;
            
         case client_type::failure_device_name_invalid:
            DeviceBase::format_devicebase_failure(out, devicebase_failure_invalid_device_name); 
            break;
         }
      } // format_failure

      
      void SettingsEnumerator::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace SettingsEnumeratorHelpers;
         if(ev->getType() == event_failure::event_id)
            finish();
         event_base *event = dynamic_cast<event_base *>(ev.get_rep());
         if(event && SettingsEnumeratorClient::is_valid_instance(event->client))
            event->send_notification();
         else
            finish();
      } // receive

      
      void SettingsEnumerator::onNetMessage(Csi::Messaging::Router *rtr,
                                            Csi::Messaging::Message *msg)
      {
         if(state == state_active || state == state_before_active)
         {
            switch(msg->getMsgType())
            {
            case Cora::Device::Messages::settings_enum_status_not:
               on_settings_enum_status_not(msg);
               break;
               
            case Cora::Device::Messages::settings_enum_not:
               on_settings_enum_not(msg);
               break;
               
            case Cora::Device::Messages::settings_enum_override_not:
               on_settings_enum_override_not(msg);
               break;
               
            case Cora::Device::Messages::settings_enum_override_stop_not:
               on_settings_enum_override_stop_not(msg);
               break;

            case Cora::Device::Messages::settings_enum_stopped_not:
               on_settings_enum_stopped_not(msg);
               break;
               
            case Cora::Device::Messages::settings_get_not:
               on_settings_get_not(msg);
               break;

            default:
               DeviceBase::onNetMessage(rtr,msg);
               break;
            }
         }
         else
            DeviceBase::onNetMessage(rtr,msg);
      } // onNetMessage

      
      void SettingsEnumerator::on_devicebase_failure(devicebase_failure_type failure)
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
         SettingsEnumeratorHelpers::event_failure::create_and_post(this,client,client_failure);
      } // on_devicebase_failure

      
      void SettingsEnumerator::on_devicebase_session_failure()
      {
         SettingsEnumeratorHelpers::event_failure::create_and_post(
            this,
            client,
            SettingsEnumeratorClient::failure_connection_failed);
      } // on_devicebase_session_failure

      
      void SettingsEnumerator::on_devicebase_ready()
      {
         state = state_before_active;
         if(interface_version >= Csi::VersionNumber("1.2.1"))
         {
            Csi::Messaging::Message start_command(device_session,
                                                  Messages::settings_enum_start_cmd);
            start_command.addUInt4(++last_tran_no);
            start_command.addBool(true); // send overrides
            start_command.addBool(true); // send status notification
            router->sendMessage(&start_command);
         }
         else
         {
            Csi::Messaging::Message start_command(device_session,
                                                  Messages::settings_get_cmd);
            router->sendMessage(&start_command);
         }
      } // on_devicebase_ready

      
      void SettingsEnumerator::on_setting_read(Csi::SharedPtr<Setting> &setting,
                                               uint4 context_token)
      {
         using namespace SettingsEnumeratorHelpers;
         if(context_token == Messages::settings_enum_override_not)
            event_setting_overridden::create_and_post(this,client,setting);
         else
            event_setting_changed::create_and_post(this,client,setting);
      } // on_setting_read


      void SettingsEnumerator::on_settings_enum_status_not(
         Csi::Messaging::Message *message)
      {
         uint4 tran_no;
         uint4 count;
         
         message->readUInt4(tran_no);
         message->readUInt4(count);
         for(uint4 i = 0; i < count; ++i)
         {
            uint4 setting_id;
            uint4 status;
            message->readUInt4(setting_id);
            message->readUInt4(status);
            if(status == 1)
               ignored_settings.erase(setting_id);
            else
               ignored_settings[setting_id] = true; 
         }
      } // on_settings_enum_status_not


      void SettingsEnumerator::on_settings_enum_not(Csi::Messaging::Message *message)
      {
         uint4 tran_no;
         message->readUInt4(tran_no);
         read_settings(message,setting_factory.get_rep(),message->getMsgType());
         if(state == state_before_active)
         {
            using namespace SettingsEnumeratorHelpers;
            event_started::create_and_post(this,client);
            state = state_active;
         }
      } // on_settings_enum_not

      
      void SettingsEnumerator::on_settings_enum_override_not(Csi::Messaging::Message *message)
      {
         uint4 tran_no;
         message->readUInt4(tran_no);
         read_settings(message,setting_factory.get_rep(),message->getMsgType());
      } // on_settings_enum_override_not

      
      void SettingsEnumerator::on_settings_enum_override_stop_not(
         Csi::Messaging::Message *message)
      {
         using namespace SettingsEnumeratorHelpers;
         event_override_stopped::create_and_post(this,client);
      } // on_settings_enum_override_stop_not

      
      void SettingsEnumerator::on_settings_enum_stopped_not(Csi::Messaging::Message *message)
      {
         using namespace SettingsEnumeratorHelpers;
         event_failure::create_and_post(
            this,
            client,
            SettingsEnumeratorClient::failure_connection_failed); 
      } // on_settings_enum_stopped_not

      
      void SettingsEnumerator::on_settings_get_not(Csi::Messaging::Message *message)
      {
         read_settings(message,setting_factory.get_rep(),message->getMsgType());
         if(state == state_before_active)
         {
            using namespace SettingsEnumeratorHelpers;
            event_started::create_and_post(this,client);
            state = state_active;
         }
      } // on_settings_get_not
   };
};

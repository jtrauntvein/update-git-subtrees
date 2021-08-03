/* Cora.Device.SettingsOverrider.cpp

   Copyright (C) 2000, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 01 August 2000
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Committed by: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.SettingsOverrider.h"
#include "coratools.strings.h"
#include <assert.h>


namespace Cora
{
   namespace Device
   {
      namespace SettingsOverriderHelpers
      {
         ////////////////////////////////////////////////////////////
         // class event_base
         //
         // Defines a base class for all events used internally by class SettingsOverrider.
         ////////////////////////////////////////////////////////////
         class event_base: public Csi::Event
         {
         protected:
            SettingsOverrider *overrider;
            SettingsOverriderClient *client;
            friend class Cora::Device::SettingsOverrider;

         public:
            event_base(uint4 event_id,
                       SettingsOverrider *overrider_,
                       SettingsOverriderClient *client_):
               Event(event_id,overrider_),
               overrider(overrider_),
               client(client_)
            { }

            virtual void notify() = 0;
         };


         ////////////////////////////////////////////////////////////
         // class event_started
         ////////////////////////////////////////////////////////////
         class event_started: public event_base
         {
         public:
            static const uint4 event_id;

            static void create_and_post(SettingsOverrider *overrider,
                                        SettingsOverriderClient *client);

            virtual void notify() { client->on_started(overrider); }

         private:
            event_started(SettingsOverrider *overrider,
                          SettingsOverriderClient *client):
               event_base(event_id,overrider,client)
            { }
         };


         uint4 const event_started::event_id =
         Csi::Event::registerType("Cora::Device::SettingsOvcerridder::event_started");


         void event_started::create_and_post(SettingsOverrider *overrider,
                                             SettingsOverriderClient *client)
         {
            try { (new event_started(overrider,client))->post(); }
            catch(Csi::Event::BadPost &) { }
         } // create_and_post


         ////////////////////////////////////////////////////////////
         // class event_failure
         ////////////////////////////////////////////////////////////
         class event_failure: public event_base
         {
         public:
            static uint4 const event_id;

            typedef SettingsOverriderClient::failure_type failure_type;
            static void create_and_post(SettingsOverrider *overrider,
                                        SettingsOverriderClient *client,
                                        failure_type failure);

            virtual void notify() { client->on_failure(overrider,failure); }

         private:
            failure_type failure;

            event_failure(SettingsOverrider *overrider,
                          SettingsOverriderClient *client,
                          failure_type failure_):
               event_base(event_id,overrider,client),
               failure(failure_)
            { }
         };


         uint4 const event_failure::event_id =
         Csi::Event::registerType("Cora::Device::SettingsOvcerridder::event_failure");


         void event_failure::create_and_post(SettingsOverrider *overrider,
                                             SettingsOverriderClient *client,
                                             failure_type failure)
         {
            try { (new event_failure(overrider,client,failure))->post(); }
            catch(Csi::Event::BadPost &) { }
         } // create_and_post


         ////////////////////////////////////////////////////////////
         // class event_override_started
         ////////////////////////////////////////////////////////////
         class event_override_started: public event_base
         {
         public:
            static uint4 const event_id;

            typedef SettingsOverriderClient::resp_code_type resp_code_type;
            static void create_and_post(SettingsOverrider *overrider,
                                        SettingsOverriderClient *client,
                                        uint4 setting_id,
                                        resp_code_type resp_code);

            virtual void notify() { client->on_override_started(overrider,setting_id,resp_code); }

         private:
            uint4 setting_id;
            resp_code_type resp_code;

            event_override_started(SettingsOverrider *overrider,
                                   SettingsOverriderClient *client,
                                   uint4 setting_id_,
                                   resp_code_type resp_code_):
               event_base(event_id,overrider,client),
               setting_id(setting_id_),
               resp_code(resp_code_)
            { }
         };


         uint4 const event_override_started::event_id =
         Csi::Event::registerType("Cora::Device::SettingsOverrider::event_override_started");


         void event_override_started::create_and_post(SettingsOverrider *overrider,
                                                      SettingsOverriderClient *client,
                                                      uint4 setting_id,
                                                      resp_code_type resp_code)
         {
            try { (new event_override_started(overrider,client,setting_id,resp_code))->post(); }
            catch(Csi::Event::BadPost &) { }
         } // create_and_post
      };


      ////////////////////////////////////////////////////////////
      // class SettingsOverrider definitions
      ////////////////////////////////////////////////////////////

      SettingsOverrider::SettingsOverrider():
         client(0),
         state(state_standby),
         override_tran_no(0)
      { }

      
      SettingsOverrider::~SettingsOverrider()
      { finish(); }


      void SettingsOverrider::start(
         SettingsOverriderClient *client_,
         router_handle &router)
      {
         if(state == state_standby)
         {
            if(SettingsOverriderClient::is_valid_instance(client_))
            {
               state = state_delegate;
               client = client_;
               DeviceBase::start(router);
            }
            else
               throw std::invalid_argument("invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start


      void SettingsOverrider::start(
         SettingsOverriderClient *client_,
         ClientBase *other_component)
      {
         if(state == state_standby)
         {
            if(SettingsOverriderClient::is_valid_instance(client_))
            {
               state = state_delegate;
               client = client_;
               DeviceBase::start(other_component);
            }
            else
               throw std::invalid_argument("invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start

      
      void SettingsOverrider::override_setting(Setting const &the_setting)
      {
         if(state == state_active)
         {
            Csi::Messaging::Message command(device_session,Messages::settings_override_start_cmd);
            uint4 setting_size_pos;
            uint4 setting_pos;
            
            command.addUInt4(override_tran_no);
            command.addUInt4(1); // send one setting
            command.addUInt4(the_setting.get_identifier());
            setting_size_pos = command.getBodyLen();
            command.addUInt4(0); // placeholder for the size
            setting_pos = command.getBodyLen();
            the_setting.write(&command);
            command.replaceUInt4(command.getBodyLen() - setting_pos,setting_size_pos);
            router->sendMessage(&command);
         }
         else
            throw exc_invalid_state();
      } // override

      
      void SettingsOverrider::finish()
      {
         state = state_standby;
         client = 0;
         override_tran_no = 0;
         DeviceBase::finish();
      } // finish


      void SettingsOverrider::format_failure(
         std::ostream &out, client_type::failure_type failure)
      {
         using namespace SettingsOverriderStrings;
         switch(failure)
         {
         case client_type::failure_unknown:
            ClientBase::describe_failure(out, corabase_failure_unknown);
            break;
            
         case client_type::failure_invalid_logon:
            ClientBase::describe_failure(out, corabase_failure_logon);
            break;
            
         case client_type::failure_invalid_device_name:
            DeviceBase::format_devicebase_failure(out, devicebase_failure_invalid_device_name);
            break;
            
         case client_type::failure_security_blocked:
            ClientBase::describe_failure(out, corabase_failure_security);
            break;
            
         case client_type::failure_unsupported:
            ClientBase::describe_failure(out, corabase_failure_unsupported);
            break;
            
         case client_type::failure_session_failed:
            ClientBase::describe_failure(out, corabase_failure_session);
            break;
            
         case client_type::failure_another_in_progress:
            out << my_strings[strid_another_in_progress];
            break;
         }
      } // format_failure

      
      void SettingsOverrider::on_devicebase_ready()
      {
         Csi::Messaging::Message command(device_session,Messages::settings_override_start_cmd);

         command.addUInt4(override_tran_no = ++last_tran_no);
         command.addUInt4(0);   // send no initial overrides
         router->sendMessage(&command);
         state = state_before_active;
      } // on_devicebase_ready

      
      void SettingsOverrider::on_devicebase_failure(devicebase_failure_type failure)
      {
         using namespace SettingsOverriderHelpers;
         SettingsOverriderClient::failure_type client_failure;

         switch(failure)
         {
         default:
            client_failure = SettingsOverriderClient::failure_unknown;
            break;

         case devicebase_failure_logon:
            client_failure = SettingsOverriderClient::failure_invalid_logon;
            break;
            
         case devicebase_failure_session:
            client_failure = SettingsOverriderClient::failure_session_failed;
            break;
            
         case devicebase_failure_invalid_device_name:
            client_failure = SettingsOverriderClient::failure_invalid_device_name;
            break;
            
         case devicebase_failure_unsupported:
            client_failure = SettingsOverriderClient::failure_unsupported;
            break;

         case devicebase_failure_security:
            client_failure = SettingsOverriderClient::failure_security_blocked;
            break;
         }
         event_failure::create_and_post(this,client,client_failure);
      } // on_devicebase_failure

      
      void SettingsOverrider::on_devicebase_session_failure()
      {
         using namespace SettingsOverriderHelpers;
         event_failure::create_and_post(this,client,SettingsOverriderClient::failure_session_failed);
      } // on_devicebase_session_failure

      
      void SettingsOverrider::onNetMessage(Csi::Messaging::Router *rtr,
                                           Csi::Messaging::Message *msg)
      {
         using namespace SettingsOverriderHelpers;

         if(state == state_before_active || state == state_active)
         {
            if(msg->getMsgType() == Messages::settings_override_start_ack)
            {
               // read the message contents
               uint4 tran_no;
               uint4 resp_code;
               uint4 count;
               uint4 setting_id;
               uint4 setting_resp_code;

               msg->readUInt4(tran_no);
               msg->readUInt4(resp_code);
               msg->readUInt4(count);
               if(resp_code == 1)
               {
                  assert(count == 0 || count == 1);
                  if(count >= 1)
                  {
                     msg->readUInt4(setting_id);
                     msg->readUInt4(setting_resp_code);
                  }
                  
                  // interpret the results
                  if(state == state_before_active)
                  {
                     state = state_active;
                     assert(count == 0);
                     event_started::create_and_post(this,client);
                  }
                  else
                  {
                     event_override_started::create_and_post(
                        this,
                        client,
                        setting_id,
                        static_cast<SettingsOverriderClient::resp_code_type>(setting_resp_code));
                  }
               }
               else
                  event_failure::create_and_post(this,client,SettingsOverriderClient::failure_another_in_progress);
            }
            else
               DeviceBase::onNetMessage(rtr,msg);
         }
         else
            DeviceBase::onNetMessage(rtr,msg);
      } // onNetMessage

      
      void SettingsOverrider::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace SettingsOverriderHelpers;
         event_base *event = dynamic_cast<event_base *>(ev.get_rep());

         assert(event != 0);
         if(event->getType() == event_failure::event_id)
            finish();
         if(SettingsOverriderClient::is_valid_instance(event->client))
            event->notify();
         else
            finish();
      } // receive 
   };
};

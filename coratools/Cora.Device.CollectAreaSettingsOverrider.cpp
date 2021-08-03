/* Cora.Device.CollectAreaSettingsOverrider.cpp

   Copyright (C) 2006, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 12 May 2006
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.CollectAreaSettingsOverrider.h"


namespace Cora
{
   namespace Device
   {
      namespace
      {
         ////////////////////////////////////////////////////////////
         // class event_base
         ////////////////////////////////////////////////////////////
         class event_base: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // overrider
            ////////////////////////////////////////////////////////////
            typedef CollectAreaSettingsOverrider overrider_type;
            overrider_type *overrider;

            ////////////////////////////////////////////////////////////
            // client
            ////////////////////////////////////////////////////////////
            typedef overrider_type::client_type client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify() = 0;

         protected:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_base(
               uint4 event_id,
               overrider_type *overrider_,
               client_type *client_):
               Event(event_id,overrider_),
               overrider(overrider_),
               client(client_)
            { }
         };


         ////////////////////////////////////////////////////////////
         // class event_started
         ////////////////////////////////////////////////////////////
         class event_started: public event_base
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               overrider_type *overrider,
               client_type *client)
            { (new event_started(overrider,client))->post(); }

            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            { client->on_started(overrider); }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_started(
               overrider_type *overrider,
               client_type *client):
               event_base(event_id,overrider,client)
            { }
         };


         uint4 const event_started::event_id =
         Csi::Event::registerType("Cora::Device::CollectAreaSettingsOverrider::event_started");


         ////////////////////////////////////////////////////////////
         // class event_failure
         ////////////////////////////////////////////////////////////
         class event_failure: public event_base
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // failure
            ////////////////////////////////////////////////////////////
            typedef client_type::failure_type failure_type;
            failure_type failure;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               overrider_type *overrider,
               client_type *client,
               failure_type failure)
            { (new event_failure(overrider,client,failure))->post(); }

            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            { client->on_failure(overrider,failure); }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_failure(
               overrider_type *overrider,
               client_type *client,
               failure_type failure_):
               event_base(event_id,overrider,client),
               failure(failure_)
            { }
         };


         uint4 const event_failure::event_id =
         Csi::Event::registerType("Cora::Device::CollectAreaSettingsOverrider::event_failure");


         ////////////////////////////////////////////////////////////
         // class event_setting_overriden
         ////////////////////////////////////////////////////////////
         class event_setting_overriden: public event_base
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // setting_id
            ////////////////////////////////////////////////////////////
            uint4 setting_id;

            ////////////////////////////////////////////////////////////
            // setting_outcome
            ////////////////////////////////////////////////////////////
            typedef client_type::outcome_type outcome_type;
            outcome_type setting_outcome;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               overrider_type *overrider,
               client_type *client,
               uint4 setting_id,
               outcome_type setting_outcome)
            {
               (new event_setting_overriden(
                  overrider,
                  client,
                  setting_id,
                  setting_outcome))->post();
            }

            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            { client->on_override_started(overrider,setting_id,setting_outcome); }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_setting_overriden(
               overrider_type *overrider,
               client_type *client,
               uint4 setting_id_,
               outcome_type setting_outcome_):
               event_base(event_id,overrider,client),
               setting_id(setting_id_),
               setting_outcome(setting_outcome_)
            { }
         };


         uint4 const event_setting_overriden::event_id =
         Csi::Event::registerType("Cora::Device::CollectAreaSettingsOverrider::event_setting_overriden");
      };


      ////////////////////////////////////////////////////////////
      // class CollectAreaSettingsOverrider definitions
      ////////////////////////////////////////////////////////////
      void CollectAreaSettingsOverrider::override_setting(Setting const &the_setting)
      {
         // we have to be started to send the command
         if(state != state_active)
            throw exc_invalid_state();

         // format and send the command
         Csi::Messaging::Message cmd(
            device_session,
            Cora::Device::Messages::override_collect_area_settings_cmd);
         uint4 setting_size_pos;
         uint4 setting_pos;
         
         cmd.addUInt4(override_tran_no);
         cmd.addWStr(collect_area_name);
         cmd.addUInt4(1);       // one setting will be sent
         cmd.addUInt4(the_setting.get_identifier());
         setting_size_pos = cmd.getBodyLen();
         cmd.addUInt4(0);
         setting_pos = cmd.getBodyLen();
         the_setting.write(&cmd);
         cmd.replaceUInt4(cmd.getBodyLen() - setting_pos,setting_size_pos);
         router->sendMessage(&cmd);
      } // override

      
      void CollectAreaSettingsOverrider::finish()
      {
         state = state_standby;
         client = 0;
         override_tran_no = 0;
         DeviceBase::finish();
      } // finish

      
      void CollectAreaSettingsOverrider::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         event_base *event = dynamic_cast<event_base *>(ev.get_rep());
         if(event)
         {
            client_type *client = this->client;

            if(event->getType() == event_failure::event_id)
               finish();
            if(event->client == client && client_type::is_valid_instance(client))
               event->notify();
         }
      } // receive

      
      void CollectAreaSettingsOverrider::on_devicebase_ready()
      {
         Csi::Messaging::Message cmd(
            device_session,
            Cora::Device::Messages::override_collect_area_settings_cmd);
         cmd.addUInt4(override_tran_no = ++last_tran_no);
         cmd.addWStr(collect_area_name);
         cmd.addUInt4(0);
         state = state_before_active;
         router->sendMessage(&cmd);
      } // on_devicebase_ready

      
      void CollectAreaSettingsOverrider::on_devicebase_failure(
         devicebase_failure_type failure_)
      {
         client_type::failure_type failure;
         switch(failure_)
         {
         default:
            failure = client_type::failure_unknown;
            break;
            
         case devicebase_failure_logon:
            failure = client_type::failure_invalid_logon;
            break;
            
         case devicebase_failure_session:
            failure = client_type::failure_session_failed;
            break;
            
         case devicebase_failure_invalid_device_name:
            failure = client_type::failure_invalid_device_name;
            break;
            
         case devicebase_failure_security:
            failure = client_type::failure_security_blocked;
            break;
         }
         event_failure::cpost(this,client,failure);
      } // on_devicebase_failure

      
      void CollectAreaSettingsOverrider::on_devicebase_session_failure()
      {
         on_devicebase_failure(devicebase_failure_session);
      } // on_devicebase_session_failure

      
      void CollectAreaSettingsOverrider::onNetMessage(
         Csi::Messaging::Router *router,
         Csi::Messaging::Message *message)
      {
         if(state == state_before_active || state == state_active)
         {
            if(message->getMsgType() == Cora::Device::Messages::override_collect_area_settings_ack)
            {
               uint4 tran_no;
               uint4 outcome;
               uint4 count;
               message->readUInt4(tran_no);
               message->readUInt4(outcome);
               message->readUInt4(count);
               if(outcome == 1)
               {
                  uint4 setting_id;
                  uint4 setting_outcome;
                  
                  if(state == state_before_active)
                  {
                     state = state_active;
                     event_started::cpost(this,client);
                  }
                  for(uint4 i = 0; i < count; ++i)
                  {
                     message->readUInt4(setting_id);
                     message->readUInt4(setting_outcome);
                     event_setting_overriden::cpost(
                        this,
                        client,
                        setting_id,
                        static_cast<client_type::outcome_type>(setting_outcome));
                  }
               }
               else
               {
                  client_type::failure_type failure;
                  switch(outcome)
                  {
                  case 3:
                     failure = client_type::failure_another_in_progress;
                     break;
                     
                  case 4:
                     failure = client_type::failure_invalid_collect_area_name;
                     break;

                  default:
                     failure = client_type::failure_unknown;
                     break;
                  }
                  event_failure::cpost(this,client,failure);
               }
            }
            else if(message->getMsgType() == Cora::Device::Messages::override_collect_area_settings_stopped_not)
            {
               uint4 tran_no;
               uint4 reason;
               client_type::failure_type failure;
               
               message->readUInt4(tran_no);
               message->readUInt4(reason);
               switch(reason)
               {
               case 2:
                  failure = client_type::failure_collect_area_deleted;
                  break;

               case 3:
                  failure = client_type::failure_device_deleted;
                  break;

               default:
                  failure = client_type::failure_unknown;
                  break;
               }
               event_failure::cpost(this,client,failure);
            }
            else
               DeviceBase::onNetMessage(router,message);
         }
         else
            DeviceBase::onNetMessage(router,message);
      } // onNetMessage 
   };
};


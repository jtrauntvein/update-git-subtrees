/* Cora.LgrNet.SettingsEnumerator.cpp

   Copyright (C) 2000, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 13 September 2000
   Last Change: Tuesday 10 September 2019
   Last Commit: $Date: 2019-09-11 18:20:24 -0600 (Wed, 11 Sep 2019) $ 
   Committed by: $Author: jon $
   
*/


#pragma hdrstop               // stop creation of precompiled header
#include "Cora.LgrNet.SettingsEnumerator.h"
#include "Cora.LgrNet.Defs.h"
#include "Cora.LgrNet.LgrNetSettingFactory.h"
#include <assert.h>


namespace Cora
{
   namespace LgrNet
   {
      namespace SettingsEnumeratorHelpers
      {
         class event_base: public Csi::Event
         {
         protected:
            SettingsEnumeratorClient *client;
            SettingsEnumerator *enumerator;

            event_base(
               uint4 event_id,
               SettingsEnumeratorClient *client_,
               SettingsEnumerator *enumerator_):
               Event(event_id,enumerator_),
               client(client_),
               enumerator(enumerator_)
            { }

            virtual void notify() = 0;

            friend class Cora::LgrNet::SettingsEnumerator;
         };


         class event_started: public event_base
         {
         public:
            virtual void notify()
            { client->on_started(enumerator); }

            static uint4 const event_id;

            static void create_and_post(
               SettingsEnumeratorClient *client,
               SettingsEnumerator *enumerator);

         private:
            event_started(
               SettingsEnumeratorClient *client,
               SettingsEnumerator *enumerator):
               event_base(event_id,client,enumerator)
            { }
         };


         uint4 const event_started::event_id =
         Csi::Event::registerType("Cora::LgrNet::SettingsEnumerator::event_started");


         void event_started::create_and_post(
            SettingsEnumeratorClient *client,
            SettingsEnumerator *enumerator)
         {
            try { (new event_started(client,enumerator))->post(); }
            catch(Csi::Event::BadPost &) { }
         } // create_and_post


         class event_failure: public event_base
         {
         public:
            typedef SettingsEnumeratorClient::failure_type failure_type;
            failure_type failure;

            virtual void notify()
            { client->on_failure(enumerator,failure); }

            static uint4 const event_id;

            static void create_and_post(
               SettingsEnumeratorClient *client,
               SettingsEnumerator *enumerator,
               failure_type failure);

         private:
            event_failure(
               SettingsEnumeratorClient *client,
               SettingsEnumerator *enumerator,
               failure_type failure_):
               event_base(event_id,client,enumerator),
               failure(failure_)
            { }
         };


         uint4 const event_failure::event_id =
         Csi::Event::registerType("Cora::LgrNet::SettingsEnumerator::event_failure");


         void event_failure::create_and_post(
            SettingsEnumeratorClient *client,
            SettingsEnumerator *enumerator,
            failure_type failure)
         {
            try { (new event_failure(client,enumerator,failure))->post(); }
            catch(Csi::Event::BadPost &) { }
         } // create_and_post


         class event_setting_changed: public event_base
         {
         public:
            typedef Csi::SharedPtr<Setting> setting_handle;
            setting_handle setting;

            virtual void notify()
            { client->on_setting_changed(enumerator,setting); }

            static uint4 const event_id;

            static void create_and_post(
               SettingsEnumeratorClient *client,
               SettingsEnumerator *enumerator,
               setting_handle &setting);

         private:
            event_setting_changed(
               SettingsEnumeratorClient *client,
               SettingsEnumerator *enumerator,
               setting_handle &setting_):
               event_base(event_id,client,enumerator),
               setting(setting_)
            { }
         };


         uint4 const event_setting_changed::event_id =
         Csi::Event::registerType("Cora::LgrNet::SettingsEnumerator::event_setting_changed");


         void event_setting_changed::create_and_post(
            SettingsEnumeratorClient *client,
            SettingsEnumerator *enumerator,
            setting_handle &setting)
         {
            try { (new event_setting_changed(client,enumerator,setting))->post(); }
            catch(Csi::Event::BadPost &) { }
         } // create_and_post 
      };


      SettingsEnumerator::SettingsEnumerator():
         state(state_standby),
         client(0)
      { setting_factory.bind(new LgrNetSettingFactory); }

      
      SettingsEnumerator::~SettingsEnumerator()
      { finish(); }

      
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
               state = state_delegate;
               client = client_;
               ClientBase::start(router);
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
               ClientBase::start(other_component);
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start
      
      
      void SettingsEnumerator::finish()
      {
         state = state_standby;
         client = 0;
         ClientBase::finish();
      } // finish


      void SettingsEnumerator::format_failure(std::ostream &out, client_type::failure_type failure)
      {
         switch(failure)
         {
         case client_type::failure_session_failed:
            describe_failure(out, corabase_failure_session);
            break;
            
         case client_type::failure_invalid_logon:
            describe_failure(out, corabase_failure_logon);
            break;
            
         case client_type::failure_unsupported:
            describe_failure(out, corabase_failure_unsupported);
            break;
            
         case client_type::failure_server_security_blocked:
            describe_failure(out, corabase_failure_security);
            break;
            
         default:
            describe_failure(out, corabase_failure_unknown);
            break;
         }
      } // format_failure
      
      
      void SettingsEnumerator::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(state != state_delegate)
         {
            if(msg->getMsgType() == Messages::settings_advise_not)
            {
               read_settings(msg,setting_factory.get_rep());
               if(state == state_before_active)
               {
                  using namespace SettingsEnumeratorHelpers;
                  state = state_active;
                  event_started::create_and_post(client,this);
               }
            }
            else
               ClientBase::onNetMessage(rtr,msg);
         }
         else
            ClientBase::onNetMessage(rtr,msg);
      } // onNetMessage

      
      void SettingsEnumerator::on_corabase_ready()
      {
         Csi::Messaging::Message cmd(
            net_session,
            Messages::get_settings_cmd);
         
         state = state_before_active;
         router->sendMessage(&cmd);
      } // on_corabase_ready

      
      void SettingsEnumerator::on_corabase_failure(corabase_failure_type failure)
      {
         using namespace SettingsEnumeratorHelpers;
         client_type::failure_type client_failure;

         switch(failure)
         {
         case corabase_failure_logon:
            client_failure = client_type::failure_invalid_logon;
            break;

         case corabase_failure_session:
            client_failure = client_type::failure_session_failed;
            break;

         case corabase_failure_unsupported:
            client_failure = client_type::failure_unsupported;
            break;
            
         case corabase_failure_security:
            client_failure = client_type::failure_server_security_blocked;
            break;
            
         default:
            client_failure = client_type::failure_unknown;
            break;
         }
         event_failure::create_and_post(client,this,client_failure);
      } // on_corabase_failure

      
      void SettingsEnumerator::on_corabase_session_failure()
      {
         using namespace SettingsEnumeratorHelpers;
         event_failure::create_and_post(client,this,client_type::failure_session_failed);
      } // on_corabase_sesion_failure

      
      void SettingsEnumerator::on_setting_read(
         Csi::SharedPtr<Setting> &setting,
         uint4 context_token)
      {
         using namespace SettingsEnumeratorHelpers;
         event_setting_changed::create_and_post(client,this,setting);
      } // on_setting_read


      void SettingsEnumerator::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace SettingsEnumeratorHelpers;
         event_base *event = dynamic_cast<event_base *>(ev.get_rep());

         assert(event != 0);
         if(event->getType() == event_failure::event_id)
            finish();
         if(SettingsEnumeratorClient::is_valid_instance(event->client))
            event->notify();
         else
            finish();
      } // receive
   };
};

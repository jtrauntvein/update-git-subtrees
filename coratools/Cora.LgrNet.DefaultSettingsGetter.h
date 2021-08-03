/* Cora.LgrNet.DefaultSettingsGetter.h

   Copyright (C) 2012, Campbell Scientific, Inc

   Written by: Jon Trauntvein 
   Date Begun: Friday 07 December 2012
   Last Change: Friday 07 December 2012
   Last Commit: $Date: 2012-12-07 09:45:26 -0600 (Fri, 07 Dec 2012) $
   Last Changed by: $Author: jon $

*/

#ifndef Cora_LgrNet_DefaultSettingsGetter_h
#define Cora_LgrNet_DefaultSettingsGetter_h

#include "Cora.ClientBase.h"
#include "Cora.LgrNet.LgrNetSettingFactory.h"
#include "Cora.LgrNet.Defs.h"
#include "Csi.Events.h"


namespace Cora
{
   namespace LgrNet
   {
      ////////////////////////////////////////////////////////////
      // class DefaultSettingsGetterClient
      ////////////////////////////////////////////////////////////
      class DefaultSettingsGetter;
      class DefaultSettingsGetterClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_complete
         ////////////////////////////////////////////////////////////
         enum outcome_type
         {
            outcome_unknown = 0,
            outcome_success = 1,
            outcome_session_failed = 2,
            outcome_invalid_logon = 3,
            outcome_unsupported = 4,
            outcome_server_security_blocked = 5
         };
         typedef Csi::SharedPtr<Setting> setting_handle;
         typedef std::list<setting_handle> settings_type;
         virtual void on_complete(
            DefaultSettingsGetter *getter,
            outcome_type outcome,
            settings_type const &settings) = 0;
      };


      ////////////////////////////////////////////////////////////
      // class DefaultSettingsGetter
      //
      // Defines a component that an application can use to obtain the list of
      // default settings from the LgrNet server. 
      ////////////////////////////////////////////////////////////
      class DefaultSettingsGetter:
         public ClientBase,
         public Csi::EventReceiver
      {
      private:
         ////////////////////////////////////////////////////////////
         // client
         ////////////////////////////////////////////////////////////
         DefaultSettingsGetterClient *client;

         ////////////////////////////////////////////////////////////
         // state
         ////////////////////////////////////////////////////////////
         enum state_type
         {
            state_standby,
            state_delegate,
            state_active
         } state;

         ////////////////////////////////////////////////////////////
         // factory
         ////////////////////////////////////////////////////////////
         Csi::SharedPtr<SettingFactory> factory;

      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         DefaultSettingsGetter():
            client(0),
            state(state_standby),
            factory(new LgrNetSettingFactory)
         { }

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~DefaultSettingsGetter()
         { finish(); }

         ////////////////////////////////////////////////////////////
         // get_factory
         ////////////////////////////////////////////////////////////
         Csi::SharedPtr<SettingFactory> &get_factory()
         { return factory; }

         ////////////////////////////////////////////////////////////
         // set_factory
         ////////////////////////////////////////////////////////////
         void set_factory(Csi::SharedPtr<SettingFactory> factory_)
         {
            if(state == state_standby)
               factory = factory_;
            else
               throw exc_invalid_state();
         }

         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         typedef DefaultSettingsGetterClient client_type;
         void start(client_type *client_, router_handle &router)
         {
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("invalid client pointer");
            if(state != state_standby)
               throw exc_invalid_state();
            client = client_;
            state = state_delegate;
            ClientBase::start(router); 
         }
         void start(client_type *client_, ClientBase *other)
         {
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("invalid client pointer");
            if(state != state_standby)
               throw exc_invalid_state();
            client = client_;
            state = state_delegate;
            ClientBase::start(other);
         }

         ////////////////////////////////////////////////////////////
         // finish
         ////////////////////////////////////////////////////////////
         virtual void finish()
         {
            state = state_standby;
            client = 0;
         }

         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

         ////////////////////////////////////////////////////////////
         // format_outcome
         ////////////////////////////////////////////////////////////
         static void format_outcome(
            std::ostream &out, client_type::outcome_type outcome);
         
      protected:
         ////////////////////////////////////////////////////////////
         // on_corabase_ready
         ////////////////////////////////////////////////////////////
         virtual void on_corabase_ready()
         {
            Csi::Messaging::Message command(
               net_session, Messages::get_default_settings_cmd);
            command.addUInt4(++last_tran_no);
            state = state_active;
            router->sendMessage(&command);
         }

         ////////////////////////////////////////////////////////////
         // on_corabase_failure
         ////////////////////////////////////////////////////////////
         virtual void on_corabase_failure(corabase_failure_type failure);

         ////////////////////////////////////////////////////////////
         // onNetMessage
         ////////////////////////////////////////////////////////////
         virtual void onNetMessage(
            Csi::Messaging::Router *router, Csi::Messaging::Message *message);
      };
   }; 
};


#endif

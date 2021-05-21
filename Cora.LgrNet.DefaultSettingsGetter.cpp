/* Cora.LgrNet.DefaultSettingsGetter.cpp

   Copyright (C) 2012, Campbell Scientific, Inc

   Written by: Jon Trauntvein 
   Date Begun: Friday 07 December 2012
   Last Change: Friday 07 December 2012
   Last Commit: $Date: 2012-12-07 09:45:26 -0600 (Fri, 07 Dec 2012) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.LgrNet.DefaultSettingsGetter.h"


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
            // outcome
            ////////////////////////////////////////////////////////////
            typedef DefaultSettingsGetterClient client_type;
            client_type::outcome_type outcome;

            ////////////////////////////////////////////////////////////
            // settings
            ////////////////////////////////////////////////////////////
            typedef client_type::settings_type settings_type;
            settings_type settings;

            ////////////////////////////////////////////////////////////
            // create
            ////////////////////////////////////////////////////////////
            static event_complete *create(
               DefaultSettingsGetter *getter,
               client_type::outcome_type outcome)
            { return new event_complete(getter, outcome); }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_complete(
               DefaultSettingsGetter *getter,
               client_type::outcome_type outcome_):
               Event(event_id, getter),
               outcome(outcome_)
            { }
         };


         uint4 const event_complete::event_id(
            Csi::Event::registerType("Cora::LgrNet::DefaultSettingsGetter::event_complete"));
      };


      ////////////////////////////////////////////////////////////
      // class DefaultSettingsGetter definitions
      ////////////////////////////////////////////////////////////
      void DefaultSettingsGetter::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         if(ev->getType() == event_complete::event_id)
         {
            event_complete *event(static_cast<event_complete *>(ev.get_rep()));
            client_type *my_client(client);
            finish();
            if(client_type::is_valid_instance(my_client))
               my_client->on_complete(this, event->outcome, event->settings);
         }
      } // receive


      void DefaultSettingsGetter::format_outcome(
         std::ostream &out, client_type::outcome_type outcome)
      {
         switch(outcome)
         {
         case client_type::outcome_success:
            out << "success"; 
            break;
            
         case client_type::outcome_session_failed:
            describe_failure(out, corabase_failure_session);
            break;
            
         case client_type::outcome_invalid_logon:
            describe_failure(out, corabase_failure_logon);
            break;
            
         case client_type::outcome_unsupported:
            describe_failure(out, corabase_failure_unsupported);
            break;
            
         case client_type::outcome_server_security_blocked:
            describe_failure(out, corabase_failure_security);
            break;
            
         default:
            describe_failure(out, corabase_failure_unknown);
            break;
         }
      } // format_outcome


      void DefaultSettingsGetter::on_corabase_failure(corabase_failure_type failure)
      {
         client_type::outcome_type outcome(client_type::outcome_unknown);
         switch(failure)
         {
         case corabase_failure_logon:
            outcome = client_type::outcome_invalid_logon;
            break;
            
         case corabase_failure_session:
            outcome = client_type::outcome_session_failed;
            break;
            
         case corabase_failure_unsupported:
            outcome = client_type::outcome_unsupported;
            break;
            
         case corabase_failure_security:
            outcome = client_type::outcome_server_security_blocked;
         }
         event_complete::create(this, outcome)->post();
      } // on_corabase_failure


      void DefaultSettingsGetter::onNetMessage(
         Csi::Messaging::Router *router, Csi::Messaging::Message *message)
      {
         if(state == state_active)
         {
            if(message->getMsgType() == Messages::get_default_settings_ack)
            {
               uint4 tran_no;
               uint4 count;
               event_complete *event(event_complete::create(this, client_type::outcome_success));
               uint4 setting_id;
               uint4 setting_len;
               
               message->readUInt4(tran_no);
               message->readUInt4(count);
               for(uint4 i = 0; i < count; ++i)
               {
                  if(message->readUInt4(setting_id) && message->readUInt4(setting_len))
                  {
                     client_type::setting_handle setting(
                        factory->make_setting(setting_id));
                     if(setting != 0)
                     {
                        setting->read(message);
                        event->settings.push_back(setting);
                     }
                     else
                        message->movePast(setting_len);
                  }
                  else
                  {
                     event->outcome = client_type::outcome_unknown;
                     break;
                  }
               }
               event->post();
            }
            else
               ClientBase::onNetMessage(router, message);
         }
         else
            ClientBase::onNetMessage(router, message);
      } // onNetMessage
   };
};


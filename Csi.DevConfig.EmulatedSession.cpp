/* Csi.DevConfig.EmulatedSession.cpp

   Copyright (C) 2010, 2010 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 29 June 2010
   Last Change: Wednesday 30 June 2010
   Last Commit: $Date: 2017-12-08 18:05:03 -0600 (Fri, 08 Dec 2017) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.DevConfig.EmulatedSession.h"
#include "Csi.MaxMin.h"


namespace Csi
{
   namespace DevConfig
   {
      namespace
      {
         ////////////////////////////////////////////////////////////
         // class event_transaction_added
         ////////////////////////////////////////////////////////////
         class event_transaction_added: public Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // client
            ////////////////////////////////////////////////////////////
            TransactionClient *client;

            ////////////////////////////////////////////////////////////
            // command
            ////////////////////////////////////////////////////////////
            typedef EmulatedSession::message_handle message_handle;
            message_handle command;

            ////////////////////////////////////////////////////////////
            // tran_no
            ////////////////////////////////////////////////////////////
            byte tran_no;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               EmulatedSession *session,
               TransactionClient *client,
               message_handle &command)
            {
               event_transaction_added *event(
                  new event_transaction_added(
                     session, client, command));
               event->post();
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_transaction_added(
               EmulatedSession *session,
               TransactionClient *client_,
               message_handle &command_):
               Event(event_id, session),
               client(client_),
               command(command_)
            { } 
         };


         uint4 const event_transaction_added::event_id(
            Event::registerType("Csi::DevConfig::EmulatedSession::event_transaction_added"));


         struct compare_setting_id
         {
            int operator ()(SharedPtr<Setting> const &s1, SharedPtr<Setting> const &s2) const
            { return s1->get_identifier() < s2->get_identifier(); }
         };
      };
      
      
      ////////////////////////////////////////////////////////////
      // class EmulatedSession definitions
      ////////////////////////////////////////////////////////////
      EmulatedSession::EmulatedSession(summary_handle &summary_):
         summary(summary_),
         last_tran_no(0),
         settings(summary_->begin(), summary_->end())
      {
         std::sort(settings.begin(), settings.end(), compare_setting_id());
      }


      EmulatedSession::~EmulatedSession()
      {
         summary.clear();
      } // destructor


      void EmulatedSession::add_transaction(
         TransactionClient *client,
         message_handle command,
         uint4 max_retry_count,
         uint4 extra_timeout_interval,
         byte tran_no)
      {
         if(command != 0)
         {
            if(tran_no == 0)
            {
               if(++last_tran_no == 0)
                  last_tran_no = 1;
               tran_no = last_tran_no;
            }
            command->set_tran_no(tran_no);
            event_transaction_added::cpost(this, client, command);
         }
      } // add_transaction


      bool EmulatedSession::supports_reset()
      {
         bool rtn(false);
         if(summary != 0 && summary->get_device_desc() != 0)
            rtn = summary->get_device_desc()->get_offline_defaults();
         return rtn;
      } // supports_reset
      
      
      void EmulatedSession::receive(SharedPtr<Event> &ev)
      {
         try
         {
            if(ev->getType() == event_transaction_added::event_id)
            {
               event_transaction_added *event(static_cast<event_transaction_added *>(ev.get_rep()));
               if(TransactionClient::is_valid_instance(event->client))
               {
                  switch(event->command->get_message_type())
                  {
                  case Messages::get_settings_cmd:
                     on_get_settings_command(event->client, event->command);
                     break;

                  case Messages::get_setting_fragment_cmd:
                     on_get_setting_fragment_command(event->client, event->command);
                     break;
                     
                  case Messages::set_settings_cmd:
                     on_set_settings_command(event->client, event->command);
                     break;

                  case Messages::set_setting_fragment_cmd:
                     on_set_setting_fragment_command(event->client, event->command);
                     break;
                     
                  case Messages::control_cmd:
                     on_control_command(event->client, event->command);
                     break;
                  }
               }
            }
         }
         catch(std::exception &e)
         { trace("Csi::DevConfig::EmulatedSession -- %s", e.what()); }
      } // receive


      namespace
      {
         ////////////////////////////////////////////////////////////
         // predicate setting_has_id
         ////////////////////////////////////////////////////////////
         struct setting_has_id
         {
            uint2 const id;
            bool const start_after;
            bool found_id;
            setting_has_id(uint2 id_, bool start_after_):
               id(id_),
               start_after(start_after_),
               found_id(false)
            { }

            bool operator ()(ConfigSummary::value_type &setting)
            {
               bool rtn(false);
               if(setting->get_identifier() == id)
               {
                  if(start_after)
                     found_id = true;
                  else
                     rtn = true;
               }
               else if(found_id)
                  rtn = true;
               return rtn;
            }
         };


         ////////////////////////////////////////////////////////////
         // functor do_write_settings
         ////////////////////////////////////////////////////////////
         struct do_write_settings
         {
            SharedPtr<Message> &response;
            SharedPtr<Message> temp;
            bool more_settings;
            do_write_settings(SharedPtr<Message> &response_):
               response(response_),
               temp(new Message),
               more_settings(false)
            { }

            void operator ()(ConfigSummary::value_type &setting)
            {
               // we will only write the setting if there is space for it
               uint4 remaining(response->get_available());
               if(remaining >= 4)
               {
                  uint4 setting_len;
                  temp->clear();
                  setting->write(temp);
                  setting_len = temp->get_body_len();
                  response->addUInt2(setting->get_identifier());
                  if(setting_len > remaining - 4)
                  {
                     // there is more setting than will fit so we will just write the setting
                     // meta-data
                     setting_len = 0x8000;
                     if(setting->get_read_only())
                        setting_len |= 0x4000;
                     response->addUInt2(static_cast<uint2>(setting_len));
                  }
                  else
                  {
                     uint4 write_len(csimin(remaining - 4, temp->get_body_len()));
                     if(setting->get_read_only())
                        setting_len |= 0x4000;
                     response->addUInt2(static_cast<uint2>(setting_len));
                     response->addBytes(temp->get_body(), write_len);
                  }
               }
               else
                  more_settings = true;
            }
         };
      };
      

      void EmulatedSession::on_get_settings_command(
         TransactionClient *client, message_handle &command)
      {
         // parse the command
         uint2 security_code;
         bool start_after(false);
         uint2 begin_setting(0);
         uint2 end_setting(0xFFFF);

         security_code = command->readUInt2();
         if(command->whatsLeft() >= 2)
         {
            begin_setting = command->readUInt2();
            if((begin_setting & 0x8000) != 0)
            {
               start_after = true;
               begin_setting &= 0x7FFF;
            }
            if(command->whatsLeft() >= 2)
               end_setting = command->readUInt2();
         }

         // form the response header
         message_handle response(new Message);
         response->set_tran_no(command->get_tran_no());
         response->set_message_type(Messages::get_settings_ack);
         response->addByte(1);  // success outcome
         response->addUInt2(summary->get_device_type());
         response->addByte(summary->get_major_version());
         response->addByte(summary->get_minor_version());

         // we need to select the settings from the summary that will satisfy this command
         settings_type::iterator bi(
            std::find_if(settings.begin(), settings.end(), setting_has_id(begin_setting, start_after)));
         settings_type::iterator ei(settings.end());
         if(bi == settings.end())
            bi = settings.begin();
         else
            ei = std::find_if(bi, settings.end(), setting_has_id(end_setting, true));

         // we are now ready to write the settings that were requested
         bool more_settings = false;
         uint4 more_settings_pos = response->length();
         
         response->addBool(more_settings);
         more_settings = std::for_each(bi, ei, do_write_settings(response)).more_settings;
         if(more_settings)
            response->replaceBool(more_settings, more_settings_pos);
         client->on_complete(command, response);
      } // on_get_settings_command


      void EmulatedSession::on_get_setting_fragment_command(
         TransactionClient *client, message_handle &command)
      {
         // parse the command
         uint2 security_code(command->readUInt2());
         uint2 setting_id(command->readUInt2());
         uint4 offset(command->readUInt4());

         // we can now form the response
         message_handle response(new Message);
         settings_type::iterator si(
            std::find_if(settings.begin(), settings.end(), setting_has_id(setting_id, false)));
         
         response->set_tran_no(command->get_tran_no());
         response->set_message_type(Messages::get_setting_fragment_ack);
         if(si != settings.end())
         {
            // we'll format the setting so that we can get the fragment that is being requested
            ConfigSummary::value_type setting(*si);
            message_handle temp(new Message);

            response->addByte(1); // successful response
            setting->write(temp);

            // we now need to determine the fragment that will be written
            if(offset < temp->get_body_len())
            {
               char const *fragment = static_cast<char const *>(temp->get_body()) + offset;
               uint4 available(response->get_available() - 2);
               uint4 fragment_len(csimin(temp->get_body_len() - offset, available));
               uint2 write_len(static_cast<uint2>(fragment_len));

               if(offset + fragment_len < temp->get_body_len())
                  write_len |= 0x8000;
               response->addUInt2(write_len);
               response->addBytes(fragment, fragment_len);
            } 
            else
               response->addUInt2(0);
         }
         else
            response->addByte(2); // unsupported setting
         client->on_complete(command, response);
      } // on_get_setting_fragment_command
      

      void EmulatedSession::on_set_settings_command(
         TransactionClient *client, message_handle &command)
      {
         message_handle response(new Message);
         response->set_message_type(Messages::set_settings_ack);
         response->set_tran_no(command->get_tran_no());
         response->addByte(1);  // we'll pretend that we accepted their changes
         client->on_complete(command, response);
      } // on_set_settings_command


      void EmulatedSession::on_set_setting_fragment_command(
         TransactionClient *client, message_handle &command)
      {
         message_handle response(new Message);
         response->set_message_type(Messages::set_setting_fragment_ack);
         response->set_tran_no(command->get_tran_no());
         response->addByte(1);  // we'll pretend that we accepted their changes
         client->on_complete(command, response);
      } // on_set_setting_fragment_command


      void EmulatedSession::on_control_command(
         TransactionClient *client, message_handle &command)
      {
         // parse the command
         uint2 security_code(command->readUInt2());
         byte action(command->readByte());
         message_handle response(new Message);

         response->set_message_type(Messages::control_ack);
         response->set_tran_no(command->get_tran_no());
         switch(action)
         {
         case ControlCodes::action_commit_changes:
            response->addByte(ControlCodes::outcome_committed);
            break;
            
         case ControlCodes::action_revert_to_defaults:
            response->addByte(ControlCodes::outcome_reverted_to_defaults);
            break;
            
         case ControlCodes::action_cancel_without_reboot:
            response->addByte(ControlCodes::outcome_session_ended);
            break;
            
         case ControlCodes::action_refresh_timer:
            response->addByte(ControlCodes::outcome_session_timer_reset);
            break;
            
         case ControlCodes::action_cancel_with_reboot:
            response->addByte(ControlCodes::outcome_discarded_with_reboot);
            break;
         }
         client->on_complete(command, response);
      } // on_control_command
   };
};



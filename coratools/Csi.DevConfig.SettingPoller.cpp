/* Csi.DevConfig.SettingPoller.cpp

   Copyright (C) 2008, 2015 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 13 November 2008
   Last Change: Thursday 15 January 2015
   Last Commit: $Date: 2015-01-15 14:10:21 -0600 (Thu, 15 Jan 2015) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.DevConfig.SettingPoller.h"
#include "coratools.strings.h"


namespace Csi
{
   namespace DevConfig
   {
      ////////////////////////////////////////////////////////////
      // class SettingPoller definitions
      ////////////////////////////////////////////////////////////
      SettingPoller::SettingPoller():
         security_code(0)
      { }


      SettingPoller::~SettingPoller()
      { }


      void SettingPoller::add_setting(setting_handle &setting)
      {
         if(session == 0)
            pending_settings.push_back(setting);
      }


      void SettingPoller::clear_settings()
      {
         if(session == 0)
            pending_settings.clear();
      }


      namespace
      {
         ////////////////////////////////////////////////////////////
         // predicate setting_id_less
         ////////////////////////////////////////////////////////////
         struct setting_id_less
         {
         public:
            typedef Csi::SharedPtr<Setting> setting_handle;
            bool operator ()(setting_handle const &s1, setting_handle const &s2) const
            { return s1->get_identifier() < s2->get_identifier(); }
         };
      };

      
      void SettingPoller::start(
         SharedPtr<SessionBase> session_, uint2 security_code_)
      {
         if(session == 0)
         {
            session = session_;
            security_code = security_code_;
            pending_settings.sort(setting_id_less());
            send_next();
         }
      } // start


      namespace
      {
         ////////////////////////////////////////////////////////////
         // predicate setting_has_id
         ////////////////////////////////////////////////////////////
         struct setting_has_id
         {
            uint2 const id;
            setting_has_id(uint2 id_):
               id(id_)
            { }

            bool operator ()(SharedPtr<Setting> &setting)
            { return setting->get_identifier() == id; }
         };
      };
      

      void SettingPoller::on_complete(
         message_handle &command, message_handle &response)
      {
         switch(response->get_message_type())
         {
         case Messages::get_settings_ack:
            on_get_settings_ack(command, response);
            break;

         case Messages::get_setting_fragment_ack:
            on_get_fragment_ack(command, response);
            break;
         }
      } // on_complete


      void SettingPoller::describe_outcome(
         std::ostream &out, outcome_type outcome)
      {
         using namespace SettingPollerStrings;
         switch(outcome)
         {
         case outcome_success:
            out << my_strings[strid_outcome_success];
            break;
            
         case outcome_link_failed:
            out << my_strings[strid_outcome_link_failed];
            break;
            
         case outcome_timed_out:
            out << my_strings[strid_outcome_timed_out];
            break;
            
         case outcome_partials_not_supported:
            out << my_strings[strid_outcome_partials_not_supported];
            break;
            
         default:
            out << my_strings[strid_outcome_unknown];
            break;
         }
      } // describe_outcome


      void SettingPoller::on_get_settings_ack(
         message_handle &command, message_handle &response)
      {
         try
         {
            byte response_code = response->readByte();
            if(response_code == 1)
            {
               uint2 device_type = response->readUInt2();
               byte major_version = response->readByte();
               byte minor_version = response->readByte();
               bool more_settings = response->readBool();
               while(response->whatsLeft() > 0 && !pending_settings.empty())
               {
                  uint2 setting_id = response->readUInt2();
                  uint2 flags = response->readUInt2();
                  uint2 setting_len = flags & 0x3fff;
                  bool large_value = (flags & 0x8000) != 0;
                  pending_settings_type::iterator si = std::find_if(
                     pending_settings.begin(), pending_settings.end(), setting_has_id(setting_id));
                  if(si != pending_settings.end())
                  {
                     if(!large_value)
                     {
                        setting_handle setting(*si);
                        message_handle content(new Message);
                        
                        response->readBytes(*content, setting_len);
                        pending_settings.erase(si); 
                        setting->read(content); 
                     }
                     else 
                     {
                        message_handle content(new Message);
                        response->readBytes(*content, setting_len);
                        partials.push_back(partial_type(*si, content));
                        pending_settings.erase(si);
                     }
                  }
                  else
                     response->movePast(setting_len);
               }
               send_next();
            }
            else
               do_on_complete(outcome_link_failed);
         }
         catch(std::exception &)
         { do_on_complete(outcome_link_failed); }
      } // on_get_settings_ack


      void SettingPoller::on_get_fragment_ack(
         message_handle &command, message_handle &response)
      {
         try
         {
            byte outcome = response->readByte();
            if(!partials.empty() && outcome == 1)
            {
               partial_type &partial = partials.front();
               uint2 flags = response->readUInt2();
               bool more_fragments = (flags & 0x8000) != 0;
               uint2 fragment_len = flags & 0x7fff;

               response->readBytes(*partial.second, fragment_len);
               if(!more_fragments)
               {
                  partial.first->read(partial.second);
                  partials.pop_front();
               }
            }
            else if(!partials.empty())
               partials.pop_front();
            send_next();
         }
         catch(std::exception &)
         { do_on_complete(outcome_link_failed); }
      } // on_get_fragment_ack
      
      
      void SettingPoller::on_failure(
         message_handle &command, failure_type failure)
      { do_on_complete(outcome_link_failed); }


      void SettingPoller::send_next()
      {
         if(!pending_settings.empty())
         {
            // we will only ask for settings that have adjacent identifiers because otherwise the
            // device will respond with settings we haven't asked for.
            uint2 begin_id(pending_settings.front()->get_identifier());
            uint2 end_id(begin_id + 1);
            pending_settings_type::iterator pi = pending_settings.begin();
            for(++pi; pi != pending_settings.end(); ++pi)
            {
               setting_handle &pending(*pi);
               if(pending->get_identifier() != end_id)
                  break;
               else
                  ++end_id;
            }

            // formate the command to send to the device.
            message_handle cmd(new Message);
            cmd->set_message_type(Messages::get_settings_cmd);
            cmd->addUInt2(security_code);
            cmd->addUInt2(begin_id);
            cmd->addUInt2(end_id);
            session->add_transaction(this, cmd, 1, 3000);
         }
         else if(!partials.empty())
         {
            message_handle cmd(new Message);
            partial_type &partial = partials.front();
            cmd->set_message_type(Messages::get_setting_fragment_cmd);
            cmd->addUInt2(security_code);
            cmd->addUInt2(partial.first->get_identifier());
            cmd->addUInt4(partial.second->get_body_len());
            session->add_transaction(this, cmd, 1, 3000);
         }
         else
            do_on_complete(outcome_success);
      } // send_next


      void SettingPoller::do_on_complete(outcome_type outcome)
      {
         session.clear();
         pending_settings.clear();
         on_complete(outcome);
      } // do_on_complete
   };
};


/* Csi.DevConfig.CoraPakbusSession.cpp

   Copyright (C) 2004, 2013 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Saturday 10 January 2004
   Last Change: Tuesday 05 November 2013
   Last Commit: $Date: 2013-11-06 10:41:13 -0600 (Wed, 06 Nov 2013) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.DevConfig.CoraPakbusSession.h"
#include "Csi.StrAscStream.h"
#include <algorithm>


namespace Csi
{
   namespace DevConfig
   {
      namespace
      {
         ////////////////////////////////////////////////////////////
         // class CachedSettingLess
         ////////////////////////////////////////////////////////////
         class CachedSettingLess
         {
         public:
            ////////////////////////////////////////////////////////////
            // comparator
            ////////////////////////////////////////////////////////////
            typedef std::pair<bool, SharedPtr<Setting> > setting_type;
            bool operator ()(setting_type const &s1, setting_type const &s2)
            {
               return s1.second->get_identifier() < s2.second->get_identifier();
            }
         };


         ////////////////////////////////////////////////////////////
         // class CachedSettingHasId
         ////////////////////////////////////////////////////////////
         class CachedSettingHasId
         {
         public:
            ////////////////////////////////////////////////////////////
            // id
            ////////////////////////////////////////////////////////////
            uint2 id;

            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            CachedSettingHasId(uint2 id_):
               id(id_)
            { }

            ////////////////////////////////////////////////////////////
            // evaluatator
            ////////////////////////////////////////////////////////////
            typedef std::pair<bool, SharedPtr<Setting> > setting_type;
            bool operator ()(setting_type const &setting)
            { return setting.second->get_identifier() == id; }
         };

         
         ////////////////////////////////////////////////////////////
         // event_next
         ////////////////////////////////////////////////////////////
         class event_next: public Event
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
               CoraPakbusSession *session)
            {
               try{(new event_next(session))->post();}
               catch(Event::BadPost &) { }
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_next(
               CoraPakbusSession *session):
               Event(event_id,session)
            { }
         };

         
         uint4 const event_next::event_id =
         Event::registerType("Csi::DevConfig::CoraPakbusSession::event_next");


         ////////////////////////////////////////////////////////////
         // class event_command_complete
         ////////////////////////////////////////////////////////////
         class event_command_complete: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // transaction
            ////////////////////////////////////////////////////////////
            typedef CoraPakbusSessionHelpers::Transaction tran_type;
            typedef SharedPtr<tran_type> tran_handle;
            tran_handle transaction;

            ////////////////////////////////////////////////////////////
            // response
            ////////////////////////////////////////////////////////////
            typedef SharedPtr<Message> response_type;
            response_type response;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               CoraPakbusSession *session,
               tran_handle &transaction,
               response_type &response)
            {
               event_command_complete *event = new event_command_complete(
                  session, transaction, response);
               event->post();
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_command_complete(
               CoraPakbusSession *session,
               tran_handle &transaction_,
               response_type &response_):
               Event(event_id, session),
               transaction(transaction_),
               response(response_)
            { }
         };


         uint4 const event_command_complete::event_id =
         Event::registerType("Csi::DevConfig::CoraPakbusSession::event_command_complete");
      };

   
      ////////////////////////////////////////////////////////////
      // class CoraPakbusSession definitions
      ////////////////////////////////////////////////////////////
      CoraPakbusSession::CoraPakbusSession(
         SharedPtr<LibraryManager> &library_,
         SharedPtr<Cora::ClientBase> &base_component_,
         StrUni const &pakbus_router_name,
         uint2 node_address):
         use_binary_transactions(false),
         library(library_),
         state(state_standby),
         base_component(base_component_)
      {
         comm_manager.bind(new comm_manager_type);
         comm_manager->set_pakbus_router_name(pakbus_router_name);
         comm_manager->set_pakbus_address(node_address);
      } // constructor

      
      CoraPakbusSession::~CoraPakbusSession()
      {
         comm_manager.clear();
         strings_setter.clear();
         strings_getter.clear();
         cached_settings.clear();
         device_desc.clear();
         library.clear();
         base_component.clear();
      } // destructor

      
      void CoraPakbusSession::add_transaction(
         TransactionClient *client,
         message_handle command,
         uint4 max_retry_count,
         uint4 timeout_interval,
         byte tran_no)
      {
         // create an object to represent this transaction and push it on to the queue
         if(tran_no != 0)
            throw std::invalid_argument("Cannot specify the server transaction number");
         transactions.push_back(
            new transaction_type(
               client,
               command,
               max_retry_count,
               timeout_interval));

         // if we are in a standby state, we need to start the comm_manager and get the string
         // settings before we can begin to execute transactions.
         if(state == state_standby)
         {
            try
            {
               comm_manager->start(this,base_component.get_rep());
               state = state_loading_strings;
            }
            catch(std::exception &)
            { do_on_error(); }
         }
         else if(transactions.size() == 1)
            event_next::cpost(this);
      } // add_transaction


      bool CoraPakbusSession::supports_reset()
      {
         bool rtn = use_binary_transactions;
         for(cached_settings_type::iterator si = cached_settings.begin();
             !rtn && si != cached_settings.end();
             ++si)
         {
            if(si->second->get_desc()->get_default_value().length())
               rtn = true;
         } 
         return rtn;
      } // supports_reset


      void CoraPakbusSession::map_model_no(StrAsc &value)
      {
         if(value.find("PC:") < value.length())
            value = "coralib3.dll";
         if(value == "CR2xx" || value == "CR200X")
            value = "CR200 Series";
         if(value == "CR800")
            value = "CR800 Series";
         if(value == "VSC1XX")
            value = "VSC100 Series";
      } // map_model_no
         
      
      void CoraPakbusSession::on_started(
         comm_manager_type *manager)
      {
         if(state == state_loading_strings)
         {
            try
            {
               strings_getter.bind(new strings_getter_type);
               strings_getter->set_pakbus_router_name(
                  manager->get_pakbus_router_name());
               strings_getter->set_pakbus_address(
                  manager->get_pakbus_address());
               strings_getter->start(this,manager);
            }
            catch(std::exception &)
            { do_on_error(); }
         }
      } // on_started

      
      void CoraPakbusSession::on_failure(
         comm_manager_type *manager,
         CommResourceManagerClient::failure_type failure)
      { 
         if(state != state_standby)
            do_on_error();
      } // on_failure

      
      namespace
      {
         ////////////////////////////////////////////////////////////
         // class setting_has_name
         ////////////////////////////////////////////////////////////
         class setting_has_name
         {
         public:
            ////////////////////////////////////////////////////////////
            // name
            ////////////////////////////////////////////////////////////
            StrAsc name;

            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            setting_has_name(StrAsc const &name_):
               name(name_)
            { }

            ////////////////////////////////////////////////////////////
            // evaluator
            ////////////////////////////////////////////////////////////
            bool operator ()(std::pair<StrAsc, StrAsc> const &pair) const
            { return pair.first == name; }
         };
      };
      
      
      void CoraPakbusSession::on_complete(
         strings_getter_type *getter,
         SettingsGetterClient::outcome_type outcome,
         SettingsGetterClient::settings_type const &settings)
      {
         if(state == state_loading_strings)
         {
            try
            {
               if(outcome == SettingsGetterClient::outcome_success)
               {
                  // make a copy of the setting string values and store them in the cache.  If the
                  // PakCtrlCodes setting is encountered, we will look at the value to determine if
                  // the new settings transactions are supported.
                  cached_settings.clear();
                  device_desc.clear();
                  use_binary_transactions = false;
                  for(settings_type::const_iterator si = settings.begin();
                      si != settings.end();
                      ++si)
                  {
                     StrAsc const &name = si->first;
                     StrAsc value = si->second;
                     if(name == "PakCtrlCodes" &&
                        value.find(" 15 ") < value.length())
                        use_binary_transactions = true;
                     if(name == "Model" && device_desc == 0)
                     {
                        map_model_no(value);
                        LibraryManager::iterator li = library->get_device(value);
                        if(li != library->end())
                           device_desc = *li;
                     }
                  }
                  
                  // now that all of the values and names have been copied, we need to create the
                  // settings objects for each value if the binary transactions are not to be used.
                  if(!use_binary_transactions && device_desc != 0)
                  {
                     DeviceDesc::iterator cat_it = device_desc->find_catalog(0);
                     if(cat_it != device_desc->end())
                     {
                        SharedPtr<SettingCatalog> &catalog = *cat_it;
                        for(settings_type::const_iterator si = settings.begin();
                            si != settings.end();
                            ++si)
                        {
                           // look up the description for the setting in the catalog.  if the
                           // description is not present, it will be created using the device's name. 
                           SettingCatalog::iterator setting_desc = catalog->get_setting(
                              si->first,
                              true);
                           if(setting_desc != catalog->end())
                           {
                              IBuffStream temp(si->second.c_str(), si->second.length());
                              cached_settings.push_back(
                                 setting_type(
                                    false,
                                    new Setting(*setting_desc)));
                              cached_settings.back().second->read_formatted(temp);
                              cached_settings.back().second->clear_has_changed();
                           }
                        }
                        
                        // we will now iterate the catalog to see if there are any settings
                        // specified there that were not sent by the device.  Since many devices
                        // have the ability to create settings, we will add those as cached settings
                        // as well
                        for(SettingCatalog::iterator ci = catalog->begin();
                            ci != catalog->end();
                            ++ci)
                        {
                           SettingCatalog::value_type &setting = *ci;
                           settings_type::const_iterator si = std::find_if(
                              settings.begin(),
                              settings.end(),
                              setting_has_name(setting->get_name()));
                           if(si == settings.end() && !setting->get_ignore_not_present())
                           {
                              cached_settings.push_back(
                                 setting_type(
                                    false,
                                    new Setting(setting)));
                              cached_settings.back().second->clear_has_changed();
                           }
                        }
                        cached_settings.sort(CachedSettingLess());
                     }
                     else
                     {
                        do_on_error();
                        return;
                     }
                  }
                  else if(!use_binary_transactions)
                  {
                     do_on_error();
                     return;
                  }
                  
                  // if we have made it to this point, we are ready to start processing the client
                  // transactions.
                  state = state_strings_loaded;
                  event_next::cpost(this);
               }
               else
                  do_on_error();
            }
            catch(std::exception &)
            { do_on_error(); } 
         }
      } // on_complete

      
      void CoraPakbusSession::on_complete(
         strings_setter_type *setter,
         SettingsSetterClient::outcome_type outcome,
         uint4 settings_applied)
      {
         if(outcome == SettingsSetterClient::outcome_success)
         {
            if(!transactions.empty())
            {
               tran_handle transaction(transactions.front());
               transactions.pop_front();
               do_close_session(transaction,1);
            }
         }
         else
            do_on_error();
      } // on_complete

      
      void CoraPakbusSession::on_complete(
         binary_sender_type *sender,
         PakctrlMessageSenderClient::outcome_type outcome,
         PakctrlMessageSenderClient::response_type &response_,
         uint4 round_trip_time)
      {
         if(outcome == PakctrlMessageSenderClient::outcome_response_received)
         {
            if(!transactions.empty())
            {
               tran_handle tran = transactions.front();
               message_handle response(new Message);

               transactions.pop_front();
               response->set_message_type(
                  static_cast<Messages::message_id_type>(
                     response_->get_message_type()));
               response->set_tran_no(tran->command->get_tran_no());
               response->addBytes(
                  response_->objAtReadIdx(),
                  response_->whatsLeft());
               event_command_complete::cpost(this, tran, response);
            }
            do_next_transaction();
         }
         else
            do_on_error();
      } // on_complete


      void CoraPakbusSession::receive(SharedPtr<Event> &ev)
      {
         if(ev->getType() == event_next::event_id)
         {
            if(state == state_strings_loaded && !transactions.empty())
               do_next_transaction();
         }
         if(ev->getType() == event_command_complete::event_id)
         {
            event_command_complete *event = static_cast<event_command_complete *>(ev.get_rep());
            if(TransactionClient::is_valid_instance(event->transaction->client))
            {
               event->transaction->client->on_complete(
                  event->transaction->command,
                  event->response);
            } 
         }
      } // receive
      
      
      void CoraPakbusSession::do_next_transaction()
      {
         if(state == state_strings_loaded && !transactions.empty())
         {
            using namespace CoraPakbusSessionHelpers;
            if(transactions.front()->started)
               return; 
            transactions.front()->started = true;
            if(use_binary_transactions)
            {
               tran_handle &tran = transactions.front(); 
               try
               {
                  SharedPtr<Csi::PakBus::PakCtrlMessage> command(
                     new Csi::PakBus::PakCtrlMessage);

                  command->set_message_type(
                     static_cast<Csi::PakBus::PakCtrl::Messages::message_type>(
                        tran->command->get_message_type()));
                  command->set_destination(comm_manager->get_pakbus_address());
                  command->set_priority(Csi::PakBus::Priorities::high);
                  tran->command->reset();
                  command->addBytes(
                     tran->command->get_body(),
                     tran->command->get_body_len()); 
                  tran->sender.bind(new Cora::PbRouter::PakctrlMessageSender);
                  tran->sender->set_pakbus_router_name(
                     comm_manager->get_pakbus_router_name());
                  tran->sender->set_command(command);
                  tran->sender->start(this,comm_manager.get_rep());
               }
               catch(std::exception &)
               { do_on_error(); }
            }
            else
            {
               tran_handle tran(transactions.front());
               transactions.pop_front();
               switch(tran->command->get_message_type())
               {
               case Messages::get_settings_cmd:
                  do_get_settings(tran);
                  break;
                  
               case Messages::set_settings_cmd:
                  do_set_settings(tran);
                  break;

               case Messages::control_cmd:
                  do_start_control(tran);
                  break;
                  
               case Messages::get_setting_fragment_cmd:
                  do_get_setting_fragment(tran);
                  break;
                  
               case Messages::set_setting_fragment_cmd:
                  do_set_setting_fragment(tran);
                  break;
               }
               event_next::cpost(this);
            }
         }
      } // do_next_transaction

      
      void CoraPakbusSession::do_get_settings(tran_handle &transaction)
      {
         // read the command parameters
         uint2 begin_id = 0;
         uint2 end_id = UInt2_Max;
         uint2 security_code;
         SharedPtr<Message> &command = transaction->command;
         
         security_code = command->readUInt2();
         if(command->whatsLeft() >= 2)
            begin_id = command->readUInt2();
         if(command->whatsLeft() >= 2)
            end_id = command->readUInt2();

         // form the response message
         SharedPtr<Message> response(new Message);
         uint4 more_settings_pos;
         
         response->set_message_type(Messages::get_settings_ack);
         response->set_tran_no(command->get_tran_no());
         response->addByte(1);  // outcome
         response->addUInt2(device_desc->get_device_type());
         response->addByte(0);  // major version number
         response->addByte(0);  // minor version number
         more_settings_pos = response->length();
         response->addBool(false);
         
         // we need to position the setting iterator to the first value with an identifier greater
         // than or equal to the specified beginning
         cached_settings_type::iterator si = cached_settings.begin();
         while(si != cached_settings.end())
         {
            if(si->second->get_identifier() >= begin_id)
               break;
            ++si;
         }
         while(si != cached_settings.end() &&
               si->second->get_identifier() < end_id)
         {
            SharedPtr<Setting> &setting = si->second;
            SharedPtr<Message> temp(new Message);
            
            response->addUInt2(setting->get_identifier());
            setting->write(temp);
            response->addUInt2(static_cast<uint2>(temp->get_body_len()));
            response->addBytes(temp->get_body(),temp->get_body_len());
            ++si;
         }
         if(si != cached_settings.end())
            response->replaceByte(1,more_settings_pos);
         transaction->client->on_complete(
            transaction->command,
            response);
      } // do_get_settings

      
      void CoraPakbusSession::do_set_settings(tran_handle &transaction)
      {
         SharedPtr<Message> &command = transaction->command;
         SharedPtr<Message> ack(new Message);
         
         uint2 security_code = command->readUInt2();
         ack->set_message_type(Messages::set_settings_ack);
         ack->set_tran_no(command->get_tran_no());
         ack->addByte(1);       // outcome == success
         while(command->whatsLeft() > 4)
         {
            uint2 setting_id = command->readUInt2();
            uint2 setting_len = command->readUInt2();
            SharedPtr<Message> body(new Message);
            body->addBytes(command->objAtReadIdx(),setting_len);
            command->movePast(setting_len);
            cached_settings_type::iterator si = std::find_if(
               cached_settings.begin(),
               cached_settings.end(),
               CachedSettingHasId(setting_id));

            ack->addUInt2(setting_id);
            if(si != cached_settings.end())
            {
               ack->addByte(1);
               si->second->read(body);
               si->first = true;
            }
            else
               ack->addByte(2); 
         }
         transaction->client->on_complete(command,ack);
      } // do_set_settings

      
      void CoraPakbusSession::do_start_control(tran_handle &transaction)
      {
         // read the command
         SharedPtr<Message> &command = transaction->command;
         uint2 security_code = command->readUInt2();
         byte action = command->readByte();

         switch(action)
         {
         case 1:
            do_start_commit(transaction);
            break;

         case 2:
         case 5:
            do_close_session(transaction,4);
            break;
            
         case 3:
            do_revert(transaction);
            break;

         case 4:
            do_refresh_session(transaction);
            break;

         default:
            do_close_session(transaction,7);
            break;
         }
      } // do_start_control


      void CoraPakbusSession::do_get_setting_fragment(tran_handle &transaction)
      {
         SharedPtr<Message> ack(new Message);
         ack->set_message_type(Messages::get_setting_fragment_ack);
         ack->set_tran_no(transaction->command->get_tran_no());
         ack->addByte(3);
         transaction->client->on_complete(transaction->command,ack);
      } // do_get_setting_fragment


      void CoraPakbusSession::do_set_setting_fragment(tran_handle &transaction)
      {
         SharedPtr<Message> ack(new Message);
         ack->set_message_type(Messages::set_setting_fragment_ack);
         ack->set_tran_no(transaction->command->get_tran_no());
         ack->addByte(3);    // not supported code
         transaction->client->on_complete(transaction->command,ack);
      } // do_set_setting_fragment
      

      void CoraPakbusSession::do_on_error()
      {
         // cancel all of the transactions
         transactions_type temp(transactions);
         transactions.clear();
         comm_manager->finish();
         strings_getter.clear();
         strings_setter.clear(); 
         while(!temp.empty())
         {
            tran_handle transaction(temp.front());
            temp.pop_front();
            if(TransactionClient::is_valid_instance(transaction->client))
            {
               transaction->client->on_failure(
                  transaction->command,
                  TransactionClient::failure_link_failed);
            }
         }
      } // do_on_error


      void CoraPakbusSession::do_start_commit(tran_handle &transaction)
      {
         // we need to set up the new strings to send to the device.
         uint4 save_count = 0;
         
         strings_setter.bind(new strings_setter_type);
         strings_setter->set_pakbus_router_name(
            comm_manager->get_pakbus_router_name());
         strings_setter->set_pakbus_address(
            comm_manager->get_pakbus_address());
         for(cached_settings_type::iterator si = cached_settings.begin();
             si != cached_settings.end();
             ++si)
         {
            if(si->first && !si->second->get_read_only())
            {
               // format the setting as a string
               OStrAscStream temp;
               temp.imbue(std::locale::classic());
               si->second->write_formatted(temp);
               strings_setter->add_setting(
                  si->second->get_name(),
                  temp.str());
               save_count++;
            }
         }
         
         // if there were any settings to save, we can now start the setter, otherwise, the
         // transaction is complete
         if(save_count)
         {
            transactions.push_front(transaction);
            strings_setter->start(this,comm_manager.get_rep());
         }
         else
            do_close_session(transaction,3);
      } // do_start_commit


      void CoraPakbusSession::do_revert(
         tran_handle &transaction)
      {
         // we will call the revert() method for each cached setting
         for(cached_settings_type::iterator si = cached_settings.begin();
             si != cached_settings.end();
             ++si)
         {
            if(si->second->revert_to_default())
               si->first = true;
         }
         
         // form the response to send to the client
         SharedPtr<Message> response(new Message);
         response->set_message_type(Messages::control_ack);
         response->set_tran_no(transaction->command->get_tran_no());
         response->addByte(5);
         transaction->client->on_complete(transaction->command,response);
      } // do_revert
      
      
      void CoraPakbusSession::do_close_session(
         tran_handle &transaction,
         byte outcome)
      {
         state = state_standby;
         strings_getter.clear();
         strings_setter.clear();
         transactions.clear();
         cached_settings.clear();
         comm_manager->finish();
         device_desc.clear();
         if(TransactionClient::is_valid_instance(transaction->client))
         {
            SharedPtr<Message> ack(new Message);
            ack->set_message_type(Messages::control_ack);
            ack->set_tran_no(transaction->command->get_tran_no());
            ack->addByte(outcome);
            transaction->client->on_complete(transaction->command,ack);
         }
      } // do_close_session


      void CoraPakbusSession::do_refresh_session(tran_handle &transaction)
      {
         if(TransactionClient::is_valid_instance(transaction->client))
         {
            SharedPtr<Message> ack(new Message);
            ack->set_message_type(Messages::control_ack);
            ack->set_tran_no(transaction->command->get_tran_no());
            ack->addByte(6);
            transaction->client->on_complete(transaction->command,ack);
         }
      } // do_refresh_session
   };
};


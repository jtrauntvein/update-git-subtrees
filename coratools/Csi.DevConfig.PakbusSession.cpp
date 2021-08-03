/* Csi.DevConfig.PakbusSession.cpp

   Copyright (C) 2012, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Monday 07 May 2012
   Last Change: Wednesday 24 January 2018
   Last Commit: $Date: 2018-01-25 10:51:49 -0600 (Thu, 25 Jan 2018) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.DevConfig.PakbusSession.h"
#include "Csi.PakBus.PakBusTran.h"
#include "Csi.StrAscStream.h"


namespace Csi
{
   namespace DevConfig
   {
      namespace
      {
         /**
          * Defines a predicate the compares setting objects by their identifiers
          */
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


         /**
          * Defines a predicate that tests whether a cached setting object has a specified
          * identifier.
          */
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


         /**
          * Defines a predicate that tests whether a given setting has the specified name.
          */
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

         
         /**
          * Represents the event that is posted to process the next transaction.
          */
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
               PakbusSession *session)
            {
               try{(new event_next(session))->post();}
               catch(Event::BadPost &) { }
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_next(
               PakbusSession *session):
               Event(event_id,session)
            { }
         };

         
         uint4 const event_next::event_id =
         Event::registerType("Csi::DevConfig::PakbusSession::event_next");


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
            typedef PakbusSessionHelpers::Transaction tran_type;
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
               PakbusSession *session,
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
               PakbusSession *session,
               tran_handle &transaction_,
               response_type &response_):
               Event(event_id, session),
               transaction(transaction_),
               response(response_)
            { }
         };


         uint4 const event_command_complete::event_id =
            Event::registerType("Csi::DevConfig::PakbusSession::event_command_complete");
         

         /**
          * Defines a PakBus transaction object that will carry out a devconfig transaction
          * by transmitting the commands over PakBus protocol.
          */
         class DevconfigTran: public PakBus::PakBusTran
         {
         public:
            /**
             * construct from a session transaction.
             *
             * @param session_  The session that started this transaction.
             * @param transaction_  The state of the transaction.
             */
            typedef PakbusSession::tran_handle tran_handle;
            DevconfigTran(PakbusSession *session_, tran_handle &transaction_):
               PakBusTran(
                  session_->get_router().get_rep(),
                  session_->get_router()->get_timer(),
                  PakBus::Priorities::high,
                  session_->get_node_address()),
               session(session_),
               transaction(transaction_),
               retry_count(0)
            { }

            /**
             * destructor
             */
            virtual ~DevconfigTran()
            { }
            
            /**
             * Overloads the base class' version to request focus
             */
            virtual void start()
            { request_focus(); }

            /**
             * Overloads the base class' version to send the message
             */
            virtual void on_focus_start()
            {
               pakctrl_message_handle command(new PakBus::PakCtrlMessage);
               command->set_message_type(
                  static_cast<PakBus::PakCtrl::Messages::message_type>(
                     transaction->command->get_message_type()));
               transaction->command->reset();
               command->addBytes(
                  transaction->command->get_body(), transaction->command->get_body_len());
               send_pakctrl_message(command);
            }

            /**
             * Handle a failure for the transaction
             */
            virtual void on_failure(failure_type failure)
            {
               using namespace PakBus::PakCtrl::DeliveryFailure;
               if(failure == timed_out_or_resource_error && ++retry_count < 4)
                  on_focus_start();
               else
               {
                  post_close_event();
                  session->do_on_error();
               }
            } 

            /**
             * Handle an incoming PakCtrl message
             */
            virtual void on_pakctrl_message(pakctrl_message_handle &message)
            {
               SharedPtr<Message> response(new Message);
               response->set_message_type(
                  static_cast<Messages::message_id_type>(message->get_message_type()));
               response->set_tran_no(transaction->command->get_tran_no());
               response->addBytes(
                  message->objAtReadIdx(), message->whatsLeft());
               event_command_complete::cpost(session, transaction, response);
               post_close_event();
            } // on_pakctrl_message

            /**
             * Overloads the base class' version to format the type for this transaction
             */
            virtual void get_transaction_description(std::ostream &out)
            { out << "devconfig"; }

         private:
            tran_handle transaction;
            PakbusSession *session;
            uint4 retry_count;
         };


         /**
          * Defines a transaction that will maintain the link with the device for an extended
          * period of time.
          */
         class CommResourceTran: public PakBus::PakBusTran
         {
         public:
            CommResourceTran(PakbusSession *session):
               PakBusTran(
                  session->get_router().get_rep(),
                  session->get_router()->get_timer(),
                  PakBus::Priorities::low,
                  session->get_node_address())
            { first_message_sent = true; }

            virtual void get_transaction_description(std::ostream &out)
            { out << "devconfig comm resource"; }
         };
      };

      
      PakbusSession::PakbusSession(
         SharedPtr<LibraryManager> &library_,
         SharedPtr<PakBus::Router> &router_,
         uint2 node_address_):
         library(library_),
         router(router_),
         node_address(node_address_),
         state(state_standby),
         use_binary_transactions(false)
      {
         
      } // constructor


      PakbusSession::~PakbusSession()
      {
      } // destructor


      void PakbusSession::add_transaction(
         TransactionClient *client,
         message_handle command,
         uint4 max_retry_count,
         uint4 timeout_interval,
         byte tran_no)
      {
         // create an object to represent this transaction  and push it on the queue
         if(tran_no != 0)
            throw std::invalid_argument("Cannot specify the PakBus transaction number.");
         transactions.push_back(
            new transaction_type(client, command, max_retry_count, timeout_interval));

         // if we are in a standby state, we will to start getting the strings from the datalogger
         if(state == state_standby)
         {
            state = state_loading_strings;
            comm_manager = new CommResourceTran(this);
            router->open_transaction(comm_manager);
            router->open_transaction(
               new PakBus::TranGetSettings(
                  router.get_rep(),
                  router->get_timer(),
                  PakBus::Priorities::high,
                  node_address,
                  this,
                  ""));
         }
         else if(transactions.size() == 1)
            event_next::cpost(this);
      } // add_transaction


      bool PakbusSession::supports_reset()
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
      }


      void PakbusSession::map_model_no(StrAsc &value)
      {
         if(value.find("PC:") < value.length())
            value = "coralib3.dll";
         if(value == "CR2xx" || value == "CR200X")
            value = "CR200 Series";
         if(value == "CR800")
            value = "CR800 Series";
         if(value == "VSC1xx")
            value = "VSC100 Series";
      } // map_model_no


      void PakbusSession::receive(SharedPtr<Event> &ev)
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
            event_next::cpost(this);
         }
      } // receive


      void PakbusSession::on_complete(
         PakBus::TranGetSettings *getter,
         TranGetSettingsClient::outcome_type outcome,
         StrAsc const &content)
      {
         if(state == state_loading_strings)
         {
            try
            {
               if(outcome == TranGetSettingsClient::outcome_success)
               {
                  // we need to parse the content.  Once we have done that, we can start rebuilding
                  // our cached setting set.
                  typedef PakBus::TranGetSettings::settings_type settings_type;
                  settings_type settings;
                  cached_settings.clear();
                  device_desc.clear();
                  use_binary_transactions = false;
                  PakBus::TranGetSettings::parse_settings(settings, content);
                  for(settings_type::const_iterator si = settings.begin();
                      si != settings.end();
                      ++si)
                  {
                     StrAsc const &name(si->first);
                     StrAsc value(si->second);
                     if(name == "PakCtrlCodes" && value.find(" 15 ") < value.length())
                        use_binary_transactions = true;
                     if(name == "Model" && device_desc == 0 && library != 0)
                     {
                        LibraryManager::iterator li;
                        
                        map_model_no(value);
                        li = library->get_device(value);
                        if(li != library->end())
                           device_desc = *li;
                     }
                  }

                  // now that we have determined the model and whether binary transactions should be
                  // used, we need to attempt to create settings objects for each value if binary
                  // transactions are not to be used.
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

                  // if we made it to this point, we are ready to start processsing the client
                  // requests
                  state = state_strings_loaded;
                  event_next::cpost(this);
               }
               else
                  do_on_error();
            }
            catch(std::exception &)
            {
               do_on_error();
            }
         }
      } // on_complete (get string settings)


      void PakbusSession::on_complete(
         PakBus::TranSetSettings *setter,
         TranSetSettingsClient::outcome_type outcome,
         uint4 failed_offset)
      {
         if(outcome == PakBus::TranSetSettingsClient::outcome_success)
         {
            if(!transactions.empty())
            {
               tran_handle tran(transactions.front());
               transactions.pop_front();
               do_close_session(tran, 1);
            }
         }
         else
            do_on_error();
      } // on_complete

      
      void PakbusSession::do_next_transaction()
      {
         if(state == state_strings_loaded && !transactions.empty())
         {
            using namespace PakbusSessionHelpers;
            tran_handle tran(transactions.front());
            if(!tran->started)
            {
               if(use_binary_transactions)
               {
                  transactions.pop_front();
                  router->open_transaction(new DevconfigTran(this, tran));
               }
               else
               {
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
         }
      } // do_next_transaction


      void PakbusSession::do_get_settings(tran_handle &transaction)
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


      void PakbusSession::do_set_settings(tran_handle &transaction)
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
         transaction->client->on_complete(command, ack);
      } // do_set_settings


      void PakbusSession::do_start_control(tran_handle &transaction)
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
            do_close_session(transaction, 4);
            break;
            
         case 3:
            do_revert(transaction);
            break;

         case 4:
            do_refresh_session(transaction);
            break;

         default:
            do_close_session(transaction, 7);
            break;
         }
      } // do_start_control


      void PakbusSession::do_get_setting_fragment(tran_handle &transaction)
      {
         SharedPtr<Message> ack(new Message);
         ack->set_message_type(Messages::get_setting_fragment_ack);
         ack->set_tran_no(transaction->command->get_tran_no());
         ack->addByte(3);
         transaction->client->on_complete(transaction->command, ack);
      } // do_get_setting_fragment


      void PakbusSession::do_set_setting_fragment(tran_handle &transaction)
      {
         SharedPtr<Message> ack(new Message);
         ack->set_message_type(Messages::set_setting_fragment_ack);
         ack->set_tran_no(transaction->command->get_tran_no());
         ack->addByte(3);    // not supported code
         transaction->client->on_complete(transaction->command, ack);
      } // do_set_setting_fragment


      void PakbusSession::do_on_error()
      {
         // cancel all pending transactions
         transactions_type temp(transactions);
         transactions.clear();
         while(!temp.empty())
         {
            tran_handle tran(temp.front());
            temp.pop_front();
            if(TransactionClient::is_valid_instance(tran->client))
               tran->client->on_failure(
                  tran->command, TransactionClient::failure_link_failed);
         }
      } // do_on_error


      void PakbusSession::do_start_commit(tran_handle &transaction)
      {
         // we need to format the collection of settings that need to be saved
         uint4 save_count = 0;
         Csi::OStrAscStream temp;

         temp.imbue(std::locale::classic());
         for(cached_settings_type::iterator si = cached_settings.begin();
             si != cached_settings.end();
             ++si)
         {
            if(si->first && !si->second->get_read_only())
            {
               if(save_count > 0)
                  temp << ";";
               temp << si->second->get_name() << "=";
               si->second->write_formatted(temp, false);
               ++save_count;
            } 
         }

         // if there were changes that were formatted, we can now initiate the set transaction.
         if(save_count)
         {
            transactions.push_front(transaction);
            router->open_transaction(
               new PakBus::TranSetSettings(
                  router.get_rep(),
                  router->get_timer(),
                  PakBus::Priorities::high,
                  node_address,
                  this,
                  temp.str()));
         }
         else
            do_close_session(transaction, 3);
      } // do_start_commit


      void PakbusSession::do_revert(tran_handle &transaction)
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


      void PakbusSession::do_close_session(
         tran_handle &transaction, byte outcome)
      {
         state = state_standby;
         transactions.clear();
         cached_settings.clear();
         device_desc.clear();
         comm_manager->on_application_finished();
         comm_manager.clear();
         if(TransactionClient::is_valid_instance(transaction->client))
         {
            SharedPtr<Message> ack(new Message);
            ack->set_message_type(Messages::control_ack);
            ack->set_tran_no(transaction->command->get_tran_no());
            ack->addByte(outcome);
            transaction->client->on_complete(transaction->command, ack);
         }
      } // do_close_session


      void PakbusSession::do_refresh_session(tran_handle &transaction)
      {
         if(TransactionClient::is_valid_instance(transaction->client))
         {
            SharedPtr<Message> ack(new Message);
            ack->set_message_type(Messages::control_ack);
            ack->set_tran_no(transaction->command->get_tran_no());
            ack->addByte(6);
            transaction->client->on_complete(transaction->command, ack);
         }
      } // do_refresh_session
   };
};



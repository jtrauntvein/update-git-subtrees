/* Csi.DevConfig.AtCommands.cpp

   Copyright (C) 2013, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Wednesday 19 June 2013
   Last Change: Friday 29 April 2016
   Last Commit: $Date: 2016-05-03 16:18:03 -0600 (Tue, 03 May 2016) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.DevConfig.AtCommands.h"
#include "Csi.fstream.h"
#include "Csi.OsException.h"
#include <regex>


namespace Csi
{
   namespace DevConfig
   {
      namespace
      {
         ////////////////////////////////////////////////////////////
         // class event_tran_complete
         ////////////////////////////////////////////////////////////
         class event_tran_complete: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // response
            ////////////////////////////////////////////////////////////
            typedef AtCommandSession::message_handle message_handle;
            message_handle response;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               AtCommandSession *session, message_handle &response)
            {
               event_tran_complete *event(
                  new event_tran_complete(session, response));
               event->post();
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_tran_complete(
               AtCommandSession *session, message_handle &response_):
               Event(event_id, session),
               response(response_)
            { }
         };


         uint4 const event_tran_complete::event_id(
            Event::registerType("Csi::DevConfig::AtCommands::event_tran_complete"));

         
         ////////////////////////////////////////////////////////////
         // token_factory
         ////////////////////////////////////////////////////////////
         Expression::TokenFactory token_factory;
      };

      
      ////////////////////////////////////////////////////////////
      // class AtCommandSession definition
      ////////////////////////////////////////////////////////////
      AtCommandSession::AtCommandSession(
         SharedPtr<SessionDriverBase> driver,
         SharedPtr<OneShot> timer_,
         device_desc_handle &device_desc_):
         Session(driver, timer_),
         timer(timer_),
         device_desc(device_desc_)
      {
         Csi::ifstream map_input(device_desc->get_at_command_map().c_str(), std::ios::binary);
         device_desc->check_loaded();
         if(map_input)
         {
            // we will parse the map input as XML
            StrUni const at_command_map_name(L"at-command-map");
            Xml::Element map_xml(at_command_map_name);
            map_xml.input(map_input);
            if(map_xml.get_name() != at_command_map_name)
               throw std::invalid_argument("invalid at command map structure");

            // we can now construct the set of translators from the command map
            DeviceDesc::value_type catalog(device_desc->front());
            for(Xml::Element::iterator mi = map_xml.begin(); mi != map_xml.end(); ++mi)
            {
               Xml::Element::value_type child(*mi);
               if(child->get_name() == L"setting")
               {
                  uint2 setting_id = child->get_attr_uint2(L"id");
                  SettingCatalog::iterator di(catalog->get_setting(setting_id));
                  if(di != catalog->end())
                  {
                     translator_handle translator(new translator_type(*child, *di));
                     translators.push_back(translator);
                  }
               }
               else if(child->get_name() == L"init")
                  init_commands.push_back(child->get_cdata_str());
               else if(child->get_name() == L"save")
                  save_commands.push_back(child->get_cdata_str());
            }
         }
         else
            throw OsException("failed to open the setting at command map file");
      } // constructor

      
      AtCommandSession::~AtCommandSession()
      {
         translators.clear();
         pending_gets.clear();
      } // destructor

      
      void AtCommandSession::add_transaction(
         TransactionClient *client,
         message_handle command,
         uint4 max_retry_count,
         uint4 timeout_interval,
         byte tran_no)
      {
         if(command != 0)
         {
            transactions.push_back(transaction_type(command, client));
            if(current_command == 0)
               do_next_transaction();
         }
      } // add_transaction

      
      void AtCommandSession::suspend()
      {
      } // suspend

      
      void AtCommandSession::on_driver_open()
      {
         // Since this is the first time that the port has been opened, we will want to insert
         // commands for all of the init strings.
         if(!transactions.empty())
         {
            transaction_type &tran(transactions.front());
            for(init_commands_type::reverse_iterator ii = init_commands.rbegin();
                ii != init_commands.rend();
                ++ii)
            {
               commands.push_front(
                  new command_type(
                     this, *ii, tran.first->get_tran_no(), true));
            }

            // we will want the attention sequence to go out first
            commands.push_front(
               new command_type(
                  this, "", tran.first->get_tran_no(), true));
         }
         do_next_command();
      } // on_driver_open

      
      void AtCommandSession::on_driver_data(void const *buff, uint4 buff_len)
      {
         if(current_command != 0)
            current_command->on_data(buff, buff_len);
         else if(terminal != 0)
         {
            using namespace SessionHelpers;
            if(TerminalBase::is_valid_instance(terminal))
               terminal->receive_data(buff, buff_len);
         }
      } // on_driver_data

      
      void AtCommandSession::on_driver_failure()
      {
         transactions_type temp(transactions);
         transactions.clear();
         commands.clear();
         if(terminal && SessionHelpers::TerminalBase::is_valid_instance(terminal))
            terminal->on_driver_failure();
         terminal = 0;
         while(!temp.empty())
         {
            transaction_type tran(temp.front());
            temp.pop_front();
            if(TransactionClient::is_valid_instance(tran.second))
               tran.second->on_failure(tran.first, TransactionClient::failure_link_failed);
         }
      } // on_driver_failure

      
      void AtCommandSession::send_data(void const *buff, uint4 buff_len)
      {
         driver->send(this, buff, buff_len);
      } // send_data

      
      void AtCommandSession::receive(SharedPtr<Event> &ev)
      {
         if(ev->getType() == event_tran_complete::event_id)
         {
            event_tran_complete *event(static_cast<event_tran_complete *>(ev.get_rep()));
            if(!transactions.empty())
            {
               transaction_type tran(transactions.front());
               transactions.pop_front();
               do_next_transaction();
               if(TransactionClient::is_valid_instance(tran.second))
                  tran.second->on_complete(tran.first, event->response);
            }
         }
         else
            Session::receive(ev);
      } // receive


      void AtCommandSession::on_command_complete(command_type *command)
      {
         if(current_command == command)
         {
            if(commands.empty())
            {
               if(!transactions.empty())
               {
                  if(command->outcome == command_type::outcome_success ||
                     command->outcome == command_type::outcome_error)
                  {
                     transaction_type &tran(transactions.front());
                     switch(tran.first->get_message_type())
                     {
                     case Messages::get_settings_cmd:
                        send_get_settings_ack();
                        break;
                        
                     case Messages::control_cmd:
                        send_control_ack();
                        break;
                     }
                     current_command.clear();
                  }
               }
               else
                  on_driver_failure();
            }
            else
            {
               if(command->outcome == command_type::outcome_synch_failed)
                  on_driver_failure();
               else
               {
                  current_command.clear();
                  do_next_command();
               }
            }
         }
      } // on_command_complete


      void AtCommandSession::do_next_transaction()
      {
         if(!transactions.empty() && current_command == 0)
         {
            message_handle &message(transactions.front().first);
            switch(message->get_message_type())
            {
            case Messages::get_settings_cmd:
               on_get_settings_cmd(message);
               break;

            case Messages::set_settings_cmd:
               on_set_settings_cmd(message);
               break;

            case Messages::get_setting_fragment_cmd:
               on_get_setting_fragment_cmd(message);
               break;

            case Messages::set_setting_fragment_cmd:
               on_set_setting_fragment_cmd(message);
               break;

            case Messages::control_cmd:
               on_control_cmd(message);
               break;
            }
         }
      } // do_next_transaction


      void AtCommandSession::do_next_command()
      {
         if(current_command == 0 && !commands.empty())
         {
            if(driver->is_open(this))
            {
               // get the first command from the queue
               current_command = commands.front();
               commands.pop_front();
               current_command->start();
            }
            else
               driver->start_open(this);
         }
      } // do_next_command


      void AtCommandSession::on_get_settings_cmd(message_handle &message)
      {
         try
         {
            // we need to parse the message parameters
            uint2 security_code(message->readUInt2());
            uint2 begin_id(0);
            uint2 end_id(0xFFFF);
            if(message->whatsLeft() >= 2)
            {
               begin_id = message->readUInt2();
               if((begin_id & 0x8000) != 0)
               {
                  begin_id &= 0x7FFF;
                  ++begin_id;
               }
               if(message->whatsLeft() >= 2)
                  end_id = (message->readUInt2() & 0x7FFF);
            }
            if(begin_id > end_id)
            {
               uint2 temp(begin_id);
               begin_id = end_id;
               end_id = temp;
            }

            // the next step is to select all of the translators that match the specified
            // identifiers
            pending_gets.clear();
            for(translators_type::iterator ti = translators.begin(); ti != translators.end(); ++ti)
            {
               translator_handle &translator(*ti);
               if(translator->get_id() >= begin_id &&
                  translator->get_id() <= end_id &&
                  translator->use_last_response)
                  pending_gets.push_back(translator);
            }

            // we can now generate commands for all of the pending translators.
            typedef std::map<StrAsc, command_handle> commands_map_type;
            commands_map_type new_commands;
            for(translators_type::iterator ti = pending_gets.begin();
                ti != pending_gets.end();
                ++ti)
            {
               // we need to add any dependencies for the translator
               translator_handle &translator(*ti);
               translators_type::iterator psi(
                  std::find(pending_saves.begin(), pending_saves.end(), translator));
               if(psi == pending_saves.end())
               {
                  typedef translator_type::dependencies_type dependencies_type;
                  for(dependencies_type::iterator di = translator->dependencies.begin();
                      di != translator->dependencies.end();
                      ++di)
                  {
                     commands_map_type::iterator dpi(new_commands.find(di->first));
                     if(dpi == new_commands.end())
                     {
                        command_handle command(
                           new command_type(
                              this, di->first, message->get_tran_no()));
                        new_commands[di->first] = command;
                        commands.push_back(command);
                        command->translators.push_back(translator);
                     }
                     else
                        dpi->second->translators.push_back(translator);
                  }
                  
                  // add the read command for the translator
                  commands_map_type::iterator pi(new_commands.find(translator->read_command));
                  command_handle command;
                  if(pi == new_commands.end() && translator->use_last_response)
                  {
                     command.bind(
                        new command_type(
                           this,
                           translator->read_command,
                           message->get_tran_no()));
                     commands.push_back(command);
                     new_commands[translator->read_command] = command;
                  }
                  else
                     command = pi->second;
                  command->translators.push_back(translator);
               }
            }
            if(commands.empty())
               commands.push_back(new command_type(this, "", message->get_tran_no()));
            do_next_command();
         }
         catch(std::exception &)
         { }
      } // on_get_settings_cmd


      void AtCommandSession::send_get_settings_ack()
      {
         transaction_type tran(transactions.front());
         message_handle ack(new Message);
         ack->set_message_type(Messages::get_settings_ack);
         ack->set_tran_no(tran.first->get_tran_no());
         ack->addByte(1);       // response code
         ack->addUInt2(device_desc->get_device_type());
         ack->addByte(1);       // major version
         ack->addByte(1);       // minor version
         ack->addBool(false);   // more settings
         for(translators_type::iterator ti = pending_gets.begin();
             ti != pending_gets.end();
             ++ti)
         {
            translator_handle &translator(*ti);
            if(translator->use_last_response)
               translator->write(ack);
         }
         event_tran_complete::cpost(this, ack);
      } // send_get_settings_ack


      void AtCommandSession::on_set_settings_cmd(message_handle &message)
      {
         try
         {
            uint2 security_code(message->readUInt2());
            uint2 setting_id;
            uint2 flags;
            uint2 len;
            message_handle ack(new Message);
            message_handle value(new Message);

            ack->set_tran_no(message->get_tran_no());
            ack->set_message_type(Messages::set_settings_ack);
            ack->addByte(1);    // response code
            while(message->whatsLeft() >= 4)
            {
               // read the information for this setting
               setting_id = message->readUInt2();
               flags = message->readUInt2();
               len = flags & 0x7FFF;
               value->clear();
               value->addBytes(message->objAtReadIdx(), len);
               message->movePast(len);

               // we now need to locate the setting in the list of translators
               byte setting_outcome(2);
               for(translators_type::iterator ti = translators.begin();
                   ti != translators.end();
                   ++ti)
               {
                  translator_handle &translator(*ti);
                  if(translator->get_id() == setting_id &&
                     translator->use_last_response &&
                     translator->setting != 0)
                  {
                     try
                     {
                        translators_type::iterator pi(
                           std::find(pending_saves.begin(), pending_saves.end(), translator));
                        translator->setting->read(value);
                        if(pi == pending_saves.end())
                           pending_saves.push_back(translator);
                        setting_outcome = 1;
                     }
                     catch(std::exception &)
                     { setting_outcome = 3; }
                  }
               }
               ack->addUInt2(setting_id);
               ack->addByte(setting_outcome);
            }
            event_tran_complete::cpost(this, ack);
         }
         catch(std::exception &)
         { }
      } // on_set_settings_cmd


      void AtCommandSession::on_get_setting_fragment_cmd(message_handle &message)
      {
      } // on_get_setting_fragment_cmd


      void AtCommandSession::on_set_setting_fragment_cmd(message_handle &message)
      {
      } // on_set_setting_fragment_cmd


      void AtCommandSession::on_control_cmd(message_handle &message)
      {
         try
         {
            uint2 security_code(message->readUInt2());
            current_control_action = message->readByte();
            if(current_control_action == ControlCodes::action_commit_changes)
            {
               // format all of the pending saves and post commands to execute them
               OStrAscStream command_buff;
               command_buff.imbue(std::locale::classic());
               for(translators_type::iterator pi = pending_saves.begin();
                   pi != pending_saves.end();
                   ++pi)
               {
                  // format the write command
                  translator_handle &translator(*pi);
                  command_buff.str("");
                  command_buff << translator->write_command;
                  translator->setting->write_formatted(command_buff, false);
                  command_buff << translator->write_postfix;

                  // post a command to change the setting
                  commands.push_back(
                     new command_type(this, command_buff.str(), message->get_tran_no()));
               }

               // add the final save commands
               for(save_commands_type::iterator si = save_commands.begin();
                   si != save_commands.end();
                   ++si)
               {
                  commands.push_back(
                     new command_type(
                        this,
                        *si,
                        message->get_tran_no(),
                        si + 1 == save_commands.end()));
               }
               do_next_command();
            }
            else if(current_control_action == ControlCodes::action_cancel_without_reboot ||
                    current_control_action == ControlCodes::action_cancel_with_reboot)
            {
               message_handle ack(new Message);
               ack->set_tran_no(message->get_tran_no());
               ack->set_message_type(Messages::control_ack);
               ack->addByte(ControlCodes::outcome_session_ended);
               event_tran_complete::cpost(this, ack);
            }
            else if(current_control_action == ControlCodes::action_revert_to_defaults)
            {
               // apply the defaults for all settings that are loaded
               for(translators_type::iterator ti = translators.begin();
                   ti != translators.end();
                   ++ti)
               {
                  translator_handle &translator(*ti);
                  if(translator->use_last_response &&
                     translator->setting != 0 &&
                     translator->setting->revert_to_default())
                  {
                     translators_type::iterator pi(
                        std::find(pending_saves.begin(), pending_saves.end(), translator));
                     if(pi == pending_saves.end())
                        pending_saves.push_back(translator);
                  }
               }

               // send the acknowledgement
               message_handle ack(new Message);
               ack->set_tran_no(message->get_tran_no());
               ack->set_message_type(Messages::control_ack);
               ack->addByte(ControlCodes::outcome_reverted_to_defaults);
               event_tran_complete::cpost(this, ack);
            }
            else if(current_control_action == ControlCodes::action_refresh_timer)
            {
               message_handle ack(new Message);
               ack->set_tran_no(message->get_tran_no());
               ack->set_message_type(Messages::control_ack);
               ack->addByte(ControlCodes::outcome_session_timer_reset);
               event_tran_complete::cpost(this, ack);
            }
         }
         catch(std::exception &)
         { }
      } // on_control_cmd


      void AtCommandSession::send_control_ack()
      {
         try
         {
            transaction_type tran(transactions.front());
            message_handle ack(new Message);
            ack->set_message_type(Messages::control_ack);
            ack->set_tran_no(tran.first->get_tran_no());
            ack->addByte(ControlCodes::outcome_committed);
            event_tran_complete::cpost(this, ack);
         }
         catch(std::exception &)
         { }
      } // send_control_ack


      namespace AtCommandsHelpers
      {
         ////////////////////////////////////////////////////////////
         // class Translator definitions
         ////////////////////////////////////////////////////////////
         Translator::Translator(
            Xml::Element &elem, setting_desc_handle &desc_):
            desc(desc_),
            use_last_response(true)
         {
            read_command = elem.get_attr_str(L"read");
            write_command = elem.get_attr_str(L"write");
            write_postfix = elem.get_attr_str(L"write-postfix");
            for(Xml::Element::iterator ei = elem.begin();
                ei != elem.end();
                ++ei)
            {
               Xml::Element::value_type &child(*ei);
               if(child->get_name() == L"depends")
               {
                  try
                  {
                     StrUni pattern(child->get_cdata_wstr());
                     if(pattern.length())
                     {
                        expression_handle expression(new Expression::ExpressionHandler(&token_factory));
                        expression->tokenise(pattern);
                        dependencies.push_back(
                           std::make_pair(child->get_attr_str(L"command"), expression));
                     }
                  }
                  catch(std::exception &)
                  { }
               }
            }
         } // constructor


         Translator::~Translator()
         {
            desc.clear();
         } // destructor


         void Translator::on_command_complete(Command *command)
         {
            if(command->outcome == Command::outcome_success)
            {
               for(dependencies_type::iterator di = dependencies.begin();
                   di != dependencies.end() && use_last_response;
                   ++di)
               {
                  if(command->name == di->first)
                  {
                     try
                     {
                        using namespace Expression;
                        expression_handle &expression(di->second);
                        ExpressionHandler::operand_handle result;
                        for(ExpressionHandler::iterator vi = expression->begin();
                            vi != expression->end();
                            ++vi)
                           vi->second->set_val(command->response, Csi::LgrDate::system());
                        result = expression->eval();
                        if(result->get_val_int() == 0)
                           use_last_response = false;
                     }
                     catch(std::exception &)
                     { }
                  }
               }
               if(command->name == read_command && use_last_response)
               {
                  try
                  {
                     if(setting == 0)
                        setting.bind(new Setting(desc));
                     setting->read_formatted(command->response, false);
                  }
                  catch(std::exception &)
                  { use_last_response = false; }
               }
            }
            else
               use_last_response = false;
         } // on_command_complete


         void Translator::write(SharedPtr<Message> &message)
         {
            if(setting != 0)
            {
               uint4 len_pos;
               message->addUInt2(setting->get_identifier());
               len_pos = message->length();
               message->addUInt2(0);
               setting->write(message);
               message->replaceUInt2(
                  static_cast<uint2>(message->length() - len_pos - 2),
                  len_pos,
                  !is_big_endian());
            }
         } // write


         ////////////////////////////////////////////////////////////
         // class Command definitions
         ////////////////////////////////////////////////////////////
         Command::Command(
            AtCommandSession *session_,
            StrAsc const &name_,
            byte tran_no_,
            bool for_init_):
            session(session_),
            name(name_),
            tran_no(tran_no_),
            timer(session_->get_timer()),
            timer_id(0),
            command(name_ + "\r"),
            state(state_before_start),
            for_init(for_init_),
            retry_count(0)
         { }


         Command::~Command()
         {
            if(timer != 0 && timer_id)
               timer->disarm(timer_id);
            timer.clear();
         } // destructor


         void Command::start()
         {
            if(state == state_before_start)
            {
               if(name.length() > 0)
               {
                  state = state_send_command;
                  session->send_data(command.c_str(), (uint4)command.length());
                  timer_id = timer->arm(this, for_init ? 500 : 5000);
               }
               else if(for_init)
               {
                  state = state_synch_plus1;
                  session->send_data("+", 1);
                  timer_id = timer->arm(this, 250);
               }
               else
               {
                  state = state_no_op;
                  timer_id = timer->arm(this, 10);
               }
            }
         } // start


         void Command::on_data(void const *buff_, uint4 buff_len)
         {
            char const *buff(static_cast<char const *>(buff_));
            if(buff_len > 0)
               response.append(buff, buff_len);
            if(state == state_send_command)
            {
               // we are waiting for the echo
               size_t echo_pos;
               StrAsc pattern(command + "\r\n");
               echo_pos = response.find(pattern.c_str());
               if(echo_pos < response.length())
               {
                  response.cut(0, echo_pos + pattern.length());
                  state = state_wait_response;
                  if(response.length() > 0)
                     on_data(buff, 0);
               }
            }
            else if(state == state_wait_response)
            {
               StrAsc const success_tail("OK");
               StrAsc const error_tail("ERROR");
               size_t success_pos(response.find(success_tail.c_str()));
               if(success_pos < response.length())
               {
                  response.cut(success_pos);
                  on_complete(outcome_success);
               }
               else
               {
                  size_t error_pos(response.find(error_tail.c_str()));
                  if(error_pos < response.length())
                     on_complete(outcome_error);
               }
            }
         } // on_data


         void Command::onOneShotFired(uint4 id)
         {
            if(id == timer_id)
            {
               timer_id = 0;
               if(state == state_send_command || state == state_wait_response)
               {
                  if(for_init)
                     on_complete(outcome_success);
                  else if(++retry_count >= 3)
                     on_complete(outcome_synch_failed);
                  else
                  {
                     state = state_synch_plus1;
                     session->send_data("+", 1);
                     timer_id = timer->arm(this, 250);
                  }
               }
               else if(state == state_synch_plus1)
               {
                  state = state_synch_plus2;
                  session->send_data("+", 1);
                  timer_id = timer->arm(this, 250);
               }
               else if(state == state_synch_plus2)
               {
                  state = state_synch_plus3;
                  session->send_data("+", 1);
                  timer_id = timer->arm(this, 250);
               }
               else if(state == state_synch_plus3)
               {
                  if(name.length() > 0)
                  {
                     response.cut(0);
                     state = state_send_command;
                     session->send_data(command.c_str(), (uint4)command.length());
                     timer_id = timer->arm(this, 5000);
                  }
                  else
                     on_complete(outcome_success);
               }
               else if(state == state_no_op)
               {
                  on_complete(outcome_success);
               }
            }
         } // onOneShotFired


         void Command::on_complete(outcome_type outcome_)
         {
            outcome = outcome_;
            for(translators_type::iterator ti = translators.begin();
                ti != translators.end() && outcome == outcome_success;
                ++ti)
               (*ti)->on_command_complete(this);
            session->on_command_complete(this);
         } // on_complete
      };
   };
};



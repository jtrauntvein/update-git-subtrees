/* Csi.DevConfig.SettingsManager.cpp

   Copyright (C) 2003, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 18 December 2003
   Last Change: Tuesday 20 March 2018
   Last Commit: $Date: 2018-03-21 11:26:54 -0600 (Wed, 21 Mar 2018) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.DevConfig.SettingsManager.h"
#include "Csi.MaxMin.h"
#include "Csi.MsgExcept.h"
#include "Csi.StrAscStream.h"
#include "coratools.strings.h"
#include "Csi.Utils.h"
#include "boost/format.hpp"
#include <algorithm>
#include <iterator>


namespace Csi
{
   namespace DevConfig
   {
      namespace
      {
         void set_comp_operand(
            Expression::Operand *operand,
            SettingComp::CompBase *component)
         {
            using namespace Csi::Expression;
            switch(component->get_component_type())
            {
            case Components::comp_int1:
            case Components::comp_int2:
            case Components::comp_int4:
            case Components::comp_enum:
            case Components::comp_enumi2:
            case Components::comp_enumi4:
               operand->set_val(
                  static_cast<int8>(component->get_val_int4()), Csi::LgrDate::system());
               break;
               
            case Components::comp_uint1:
            case Components::comp_uint2:
            case Components::comp_uint4:
               operand->set_val(
                  static_cast<int8>(component->get_val_uint4()), Csi::LgrDate::system());
               break;
               
            case Components::comp_double:
            case Components::comp_float:
               operand->set_val(component->get_val_double(), Csi::LgrDate::system());
               break;
               
            default:
               operand->set_val(component->get_val_str(), Csi::LgrDate::system());
               break;
            }
         } // set_comp_operand


         class SettingValFunction: public Expression::Function
         {
         private:
            ////////////////////////////////////////////////////////////
            // manager
            ////////////////////////////////////////////////////////////
            SettingsManager *manager;

         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            SettingValFunction(SettingsManager *manager_):
               manager(manager_)
            { }

            ////////////////////////////////////////////////////////////
            // eval
            ////////////////////////////////////////////////////////////
            virtual void eval(
               token_stack_type &stack, Expression::ExpressionHandler *expression)
            {
               using namespace Expression;
               typedef LightPolySharedPtr<Token, Operand> operand_handle;
               if(stack.size() >= 1)
               {
                  operand_handle setting_id(stack.back()); stack.pop_back();
                  SettingsManager::value_type setting(
                     manager->get_setting(static_cast<uint2>(setting_id->get_val_int())));
                  OStrAscStream temp;
                  operand_handle rtn(new Operand);

                  temp.imbue(std::locale::classic());
                  setting->write_formatted(temp, false);
                  rtn->set_val(temp.str(), LgrDate::system());
                  stack.push_back(rtn.get_handle());
               }
               else
                  throw std::invalid_argument("expected the setting ID");
            }

            ////////////////////////////////////////////////////////////
            // clone
            ////////////////////////////////////////////////////////////
            virtual SettingValFunction *clone() const
            { return new SettingValFunction(manager); }

            ////////////////////////////////////////////////////////////
            // format
            ////////////////////////////////////////////////////////////
            virtual void format(std::ostream &out) const
            { out << "SettingVal"; }

            ////////////////////////////////////////////////////////////
            // get_min_arguments
            ////////////////////////////////////////////////////////////
            virtual uint4 get_min_arguments()
            { return 1; }

            ////////////////////////////////////////////////////////////
            // get_argument_name
            ////////////////////////////////////////////////////////////
            virtual wchar_t const *get_argument_name(uint4 index)
            { return L"settingId"; }
         };


         ////////////////////////////////////////////////////////////
         // class CompValFunction
         ////////////////////////////////////////////////////////////
         class CompValFunction: public Expression::Function
         {
         private:
            ////////////////////////////////////////////////////////////
            // manager
            ////////////////////////////////////////////////////////////
            SettingsManager *manager;

         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            CompValFunction(SettingsManager *manager_):
               manager(manager_)
            { }

            ////////////////////////////////////////////////////////////
            // eval
            ////////////////////////////////////////////////////////////
            virtual void eval(
               token_stack_type &stack, Expression::ExpressionHandler *expression)
            {
               using namespace Expression;
               typedef LightPolySharedPtr<Token, Operand> operand_handle;
               if(stack.size() >= 2)
               {
                  operand_handle comp_no(stack.back()); stack.pop_back();
                  operand_handle setting_id(stack.back()); stack.pop_back();
                  operand_handle rtn(new Operand);
                  SettingsManager::value_type setting(
                     manager->get_setting(static_cast<uint2>(setting_id->get_val_int())));
                  Setting::value_type component(
                     (*setting)[static_cast<uint4>(comp_no->get_val_int())]);
                  set_comp_operand(rtn.get_rep(), component.get_rep());
                  stack.push_back(rtn.get_handle());
               }
               else
                  throw std::invalid_argument("CompValue(settingId, compNo) requires two operands");
            }

            ////////////////////////////////////////////////////////////
            // clone
            ////////////////////////////////////////////////////////////
            virtual CompValFunction *clone() const
            { return new CompValFunction(manager); }

            ////////////////////////////////////////////////////////////
            // format
            ////////////////////////////////////////////////////////////
            virtual void format(std::ostream &out) const
            { out << "CompValue"; }

            ////////////////////////////////////////////////////////////
            // get_min_arguments
            ////////////////////////////////////////////////////////////
            virtual uint4 get_min_arguments()
            { return 2; }
            
            ////////////////////////////////////////////////////////////
            // get_argument_name
            ////////////////////////////////////////////////////////////
            virtual wchar_t const *get_argument_name(uint4 index)
            {
               wchar_t const *rtn(0);
               switch(index)
               {
               case 0:
                  rtn = L"setting_id";
                  break;
                  
               case 1:
                  rtn = L"comp_index";
                  break;
                  
               default:
                  throw std::invalid_argument("invalid argument index");
                  break;
               }
               return rtn;
            }
         };


         ////////////////////////////////////////////////////////////
         // class ValidWepPasswordFunction
         ////////////////////////////////////////////////////////////
         class ValidWepFunction: public Expression::Function
         {
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            ValidWepFunction()
            { }

            ////////////////////////////////////////////////////////////
            // eval
            ////////////////////////////////////////////////////////////
            virtual void eval(
               token_stack_type &stack, Expression::ExpressionHandler *expression)
            {
               using namespace Expression;
               if(stack.size() >= 1)
               {
                  typedef Csi::LightPolySharedPtr<Token, Operand> operand_handle;
                  operand_handle password(stack.back());
                  bool rtn;
                  stack.pop_back();
                  rtn = Csi::valid_wep_password(password->get_val_str());
                  stack.push_back(new Operand(rtn ? -1 : 0, password->get_timestamp()));
               }
               else
                  throw std::invalid_argument("ValidWepPassword(password) requires one operand");
            }


            ////////////////////////////////////////////////////////////
            // clone
            ////////////////////////////////////////////////////////////
            virtual ValidWepFunction *clone() const
            { return new ValidWepFunction(); }

            ////////////////////////////////////////////////////////////
            // format
            ////////////////////////////////////////////////////////////
            virtual void format(std::ostream &out) const
            {  out << "ValidWepPassword"; }

            ////////////////////////////////////////////////////////////
            // get_min_arguments
            ////////////////////////////////////////////////////////////
            virtual uint4 get_min_arguments()
            { return 1; }

            ////////////////////////////////////////////////////////////
            // get_argument_name
            ////////////////////////////////////////////////////////////
            virtual wchar_t const *get_argument_name(uint4 index)
            { return L"password"; }
         };
         
         
         ////////////////////////////////////////////////////////////
         // class ValidateFactory
         ////////////////////////////////////////////////////////////
         class ValidateFactory: public Expression::TokenFactory
         {
         private:
            ////////////////////////////////////////////////////////////
            // manager
            ////////////////////////////////////////////////////////////
            SettingsManager *manager;
            
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            ValidateFactory(SettingsManager *manager_):
               manager(manager_)
            { }

            ////////////////////////////////////////////////////////////
            // make_token
            ////////////////////////////////////////////////////////////
            virtual Expression::token_handle make_token(
               Expression::token_handle &prev_token,
               StrUni const &token_str,
               size_t token_start)
            {
               Expression::token_handle rtn;
               if(token_str == L"SettingVal" || token_str == L"SettingValue")
                  rtn.bind(new SettingValFunction(manager));
               else if(token_str == L"CompValue" || token_str == L"CompVal")
                  rtn.bind(new CompValFunction(manager));
               else if(token_str == L"ValidWepPassword")
                  rtn.bind(new ValidWepFunction());
               else
                  rtn = TokenFactory::make_token(prev_token, token_str, token_start);
               return rtn;
            }
         };
      };

      
      ////////////////////////////////////////////////////////////
      // class SettingsManagerClient definitions
      ////////////////////////////////////////////////////////////
      void SettingsManagerClient::describe_load_failure(
         std::ostream &out, load_failure_type failure)
      {
         using namespace SettingsManagerStrings;
         switch(failure)
         {
         case load_unknown_device_type:
            out << my_strings[strid_load_unknown_device_type];
            break;
            
         case load_unknown_catalog:
            out << my_strings[strid_load_unknown_catalog];
            break;
            
         case load_comms_failure:
            out << my_strings[strid_load_comms];
            break;
            
         case load_security_failure:
            out << my_strings[strid_load_security];
            break;
            
         case load_locked_failure:
            out << my_strings[strid_load_locked];
            break;

         case load_invalid_device_desc_failure:
            out << my_strings[strid_load_invalid_device_desc];
            break;
            
         default:
            out << my_strings[strid_load_unknown];
            break;
         }
      } // describe_load_failure


      void SettingsManagerClient::describe_commit_failure(
         std::ostream &out, commit_failure_type failure)
      {
         using namespace SettingsManagerStrings;
         switch(failure)
         {
         case commit_security_failure:
            out << my_strings[strid_commit_security];
            break;
            
         case commit_comms_failure:
            out << my_strings[strid_commit_comms];
            break;
            
         case commit_locked_failure:
            out << my_strings[strid_commit_locked];
            break;

         case commit_setting_not_recognised:
            out << my_strings[strid_commit_setting_not_recognised];
            break;
            
         case commit_setting_malformed:
            out << my_strings[strid_commit_setting_malformed];
            break;
            
         case commit_setting_read_only:
            out << my_strings[strid_commit_setting_read_only];
            break;
            
         case commit_out_of_memory:
            out << my_strings[strid_commit_out_of_memory];
            break;

         case commit_system_error:
            out << my_strings[strid_commit_system_error];
            break;
            
         default:
            out << my_strings[strid_load_unknown];
            break;
         }
      } // describe_commit_failure

      
      ////////////////////////////////////////////////////////////
      // class SettingsManager definitions
      ////////////////////////////////////////////////////////////
      SettingsManager::SettingsManager(
         SharedPtr<LibraryManager> library_, bool for_factory_):
         library(library_),
         state(state_standby),
         marked_as_changed(false),
         security_code(0),
         for_factory(for_factory_)
      { }


      SettingsManager::~SettingsManager()
      { }


      void SettingsManager::add_client(SettingsManagerClient *client)
      {
         if(SettingsManagerClient::is_valid_instance(client))
         {
            clients.push_back(client);
            if(state != state_standby && state != state_loading_settings)
               client->on_started(this);
         }
         else
            throw MsgExcept("Invalid client pointer");
      } // add_client


      void SettingsManager::remove_client(SettingsManagerClient *client)
      {
         clients_type::iterator ci = std::find(
            clients.begin(),
            clients.end(),
            client);
         if(ci != clients.end())
            clients.erase(ci);
      } // remove_client
      

      void SettingsManager::start(
         SharedPtr<SessionBase> session_,
         uint2 security_code_)
      {
         if(state == state_standby)
         {
            // initialise the state variables
            session = session_;
            security_code = security_code_;
            state = state_loading_settings;
            marked_as_changed = false;
            
            // we need to send the first command message
            message_handle cmd(new Message);
            cmd->set_message_type(Messages::get_settings_cmd);
            cmd->addUInt2(security_code);
            received_identifiers.clear();
            session->add_transaction(this,cmd,3,3000);
         }
         else
            throw MsgExcept("SettingsManager already started");
      } // start


      bool SettingsManager::needs_to_commit()
      {
         bool is_busy(
            state == state_cancelling ||
            state == state_reboot_commit ||
            state == state_reboot_rollback);
         bool rtn( marked_as_changed && !is_busy);
         for(settings_type::iterator si = settings.begin();
             !rtn && !is_busy && si != settings.end();
             ++si)
            rtn = (*si)->get_has_changed();
         return rtn;
      } // needs_to_commit


      void SettingsManager::set_needs_to_commit()
      { marked_as_changed = true; }

      
      void SettingsManager::start_commit_changes()
      {
         if(state == state_active)
         {
            // all of the settings that have been changed need to have their contents streamed and
            // put in the commit_queue.  At that point, we can start sending the changed settings to
            // the device.
            commit_warnings.clear();
            for(settings_type::iterator si = settings.begin();
                si != settings.end();
                ++si)
            {
               value_type &setting = *si;
               if(setting->get_has_changed())
               {
                  message_handle msg(new Message);
                  setting->write(msg);
                  commit_queue.push_back(
                     queue_element(
                        setting->get_desc(),
                        msg)); 
               }
            }
            state = state_saving_settings;
            do_next_commit();
         }
         else
            throw MsgExcept("Commit started from an invalid state");
      } // start_commit_changes


      bool SettingsManager::can_revert_to_defaults()
      { return session->supports_reset() && device_desc->get_supports_factory_defaults(); }


      void SettingsManager::start_revert_to_defaults()
      {
         if(state == state_active)
         {
            message_handle revert_cmd(new Message);
            revert_cmd->set_message_type(Messages::control_cmd);
            revert_cmd->addUInt2(security_code);
            revert_cmd->addByte(ControlCodes::action_revert_to_defaults);
            received_identifiers.clear();
            session->add_transaction(this,revert_cmd,3,3000);
            state = state_reverting_to_defaults;
         }
         else
            throw MsgExcept("Revert started in an invalid state");
      } // start_revert_to_defaults


      void SettingsManager::start_cancel_changes(bool should_reboot)
      {
         if(state == state_active)
         {
            message_handle cancel_cmd(new Message);
            cancel_cmd->set_message_type(Messages::control_cmd);
            cancel_cmd->addUInt2(security_code);
            if(!should_reboot) 
               cancel_cmd->addByte(ControlCodes::action_cancel_without_reboot);
            else
               cancel_cmd->addByte(ControlCodes::action_cancel_with_reboot);
            state = state_cancelling;
            session->add_transaction(this,cancel_cmd,3,3000);
         }
         else
            throw MsgExcept("Cancel started in an invalid state");
      } // start_cancel_changes


      void SettingsManager::start_refresh()
      {
         if(state == state_active)
         {
            // set the state and start the load all over again
            message_handle cmd(new Message);
            cmd->set_message_type(Messages::get_settings_cmd);
            cmd->addUInt2(security_code);
            state = state_loading_settings;
            marked_as_changed = false;
            settings.clear();
            received_identifiers.clear();
            session->add_transaction(this,cmd,3,3000); 
         }
         else
            throw MsgExcept("Refresh started in an invalid state");
      } // start_refresh


      void SettingsManager::start_scan_wifi()
      {
         if(state == state_active)
         {
            message_handle cmd(new Message);
            cmd->set_message_type(Messages::control_cmd);
            cmd->addUInt2(security_code);
            cmd->addByte(ControlCodes::action_scan_wifi);
            session->add_transaction(this, cmd, 3, 3000);
         }
         else
            throw MsgExcept("Wifi scan started in an invalid state");
      } // start_scan_wifi


      StrAsc const &SettingsManager::get_model_no(bool use_presented) const
      {
         if(device_desc == 0)
            throw MsgExcept("Invalid device description");
         return device_desc->get_model_no(use_presented);
      } // get_model_no


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
            bool operator ()(SharedPtr<Setting> const &setting)
            { return setting->get_name() == name; }
         };


         ////////////////////////////////////////////////////////////
         // class setting_has_id
         ////////////////////////////////////////////////////////////
         class setting_has_id
         {
         private:
            uint2 setting_id;

         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            setting_has_id(uint2 setting_id_):
               setting_id(setting_id_)
            { }

            ////////////////////////////////////////////////////////////
            // evaluator
            ////////////////////////////////////////////////////////////
            bool operator ()(SharedPtr<Setting> const &setting)
            { return setting->get_identifier() == setting_id; }
         };


         ////////////////////////////////////////////////////////////
         // predicate setting_has_common_name
         ////////////////////////////////////////////////////////////
         class setting_has_common_name
         {
         public:
            StrAsc const common_name;
            setting_has_common_name(StrAsc const &name):
               common_name(name)
            { }

            bool operator ()(SharedPtr<Setting> const &setting) const
            { return setting->get_common_name() == common_name; }
         };
      };
      

      SettingsManager::iterator SettingsManager::find_setting(
         StrAsc const &setting_name)
      {
         return std::find_if(
            settings.begin(),
            settings.end(),
            setting_has_name(setting_name));
      } // find_setting


      SettingsManager::iterator SettingsManager::find_setting_common(
         StrAsc const &common_name)
      {
         return std::find_if(
            settings.begin(),
            settings.end(),
            setting_has_common_name(common_name));
      } // find_setting_common


      SettingsManager::iterator SettingsManager::find_setting(
         uint2 setting_id)
      {
         return std::find_if(
            settings.begin(),
            settings.end(),
            setting_has_id(setting_id));
      }


      Setting *SettingsManager::clone_setting(StrAsc const &setting_name)
      {
         iterator it = find_setting(setting_name);
         
         if(it == settings.end())
            throw std::invalid_argument("Invalid setting name");
         return (*it)->clone();
      } // clone_setting


      SettingsManager::value_type SettingsManager::get_setting(uint2 id)
      {
         iterator it = find_setting(id);
         if(it == settings.end())
            throw std::invalid_argument("Invalid setting identifier");
         return *it;
      } // get_setting


      Setting *SettingsManager::clone_setting(uint2 setting_id)
      {
         iterator it = find_setting(setting_id);
         if(it == settings.end())
            throw std::invalid_argument("Invalid setting identifier");
         return (*it)->clone();
      } // clone_setting


      void SettingsManager::copy_setting(Setting *setting)
      {
         iterator si = find_setting(setting->get_identifier());
         if(si != settings.end())
         {
            (*si)->copy(setting);
            marked_as_changed = true;
         }
         else
            throw std::invalid_argument("Invalid setting name");
      } // copy_setting


      StrAsc const &SettingsManager::get_component_desc_text(
         StrAsc const &setting_name,
         StrAsc const &comp_name)
      {
         iterator si = find_setting(setting_name);
         if(si == settings.end())
            throw std::invalid_argument("Invalid setting name");
         return (*si)->get_description_text(comp_name);
      } // get_component_desc_text


      bool SettingsManager::is_loaded() const
      {
         bool rtn;
         switch(state)
         {
         case state_standby:
         case state_loading_settings:
            rtn = false;
            break;
            
         default:
            rtn = true;
            break;
         }
         return rtn;
      } // is_loaded


      bool SettingsManager::should_use_tabs() const
      {
         bool rtn(false);
         for(const_iterator si = begin(); !rtn && si != end(); ++si)
         {
            value_type const &setting(*si);
            StrAsc const &tab_name(setting->get_tab_name());
            if(tab_name.length() > 0)
               rtn = true;
         }
         return rtn;
      } // should_use_tabs


      bool SettingsManager::supports_factory_defaults() const
      { return device_desc->get_supports_factory_defaults(); }


      bool SettingsManager::validate(validate_errors_type &errors)
      {
         errors.clear();
         if(validate_token_factory == 0)
            validate_token_factory.bind(new ValidateFactory(this));
         for(iterator si = begin(); si != end(); ++si)
         {
            value_type &setting(*si);
            for(Setting::iterator ci = setting->begin(); ci != setting->end(); ++ci)
            {
               try
               {
                  // we need to start by checking to see if we need to generate the validate
                  // expressions for the token.
                  using namespace SettingComp;
                  Setting::value_type &component(*ci);
                  CompBase::validate_rules_type &rules(component->get_validate_rules());
                  
                  if(rules.empty() && !component->get_rule_sources().empty())
                  {
                     CompBase::rule_sources_type sources(component->get_rule_sources());
                     while(!sources.empty())
                     {
                        CompBase::rule_sources_type::value_type source(sources.front());
                        CompBase::expression_handle expression(
                           new Expression::ExpressionHandler(validate_token_factory.get_rep()));
                        expression->tokenise(source.first);
                        rules.push_back(CompBase::validate_rule(expression, source.second));
                        sources.pop_front();
                     }
                  }

                  // we now need to evaluate each of the component's rules
                  for(CompBase::validate_rules_type::iterator ri = rules.begin();
                      ri != rules.end();
                      ++ri)
                  {
                     // we need to initialise any known variables in the expression
                     using namespace Csi::Expression;
                     CompBase::validate_rule &rule(*ri);
                     CompBase::expression_handle &expression(rule.first);
                     
                     for(ExpressionHandler::iterator vi = expression->begin();
                         vi != expression->end();
                         ++vi)
                     {
                        if(vi->first == L"value")
                           set_comp_operand(vi->second.get_rep(), component.get_rep());
                     }

                     // we are now ready to evaluate the expression
                     ExpressionHandler::operand_handle result(expression->eval());
                     if(result->get_val_int())
                     {
#ifdef _DEBUG
                        OStrAscStream msg;
                        msg << "validation error for setting " << setting->get_identifier() << "\n";
                        expression->format_postfix(msg);
                        trace(msg.c_str());
#endif
                        errors.push_back(rule.second);
                     }
                  }
               }
               catch(std::exception &e)
               {
                  trace("Error with settings validation: %s", e.what());
               }
            }
         }
         return errors.empty();
      } // validate


      bool SettingsManager::get_cancel_pending() const
      { return state == state_cancelling; }

      
      void SettingsManager::on_complete(
         message_handle &command,
         message_handle &response)
      {
         switch(response->get_message_type())
         {
         case Messages::get_settings_ack:
            on_get_settings_ack(command,response);
            break;
            
         case Messages::get_setting_fragment_ack:
            on_get_setting_fragment_ack(command,response);
            break;
            
         case Messages::set_settings_ack:
            on_set_settings_ack(command,response);
            break; 
               
         case Messages::set_setting_fragment_ack:
            on_set_setting_fragment_ack(command,response);
            break;
            
         case Messages::control_ack:
            on_control_ack(command,response);
            break;
         }
      } // on_complete


      void SettingsManager::on_failure(
         message_handle &command,
         failure_type failure)
      {
         switch(state)
         {
         case state_loading_settings:
         case state_reverting_to_defaults:
         case state_cancelling:
            do_load_error(SettingsManagerClient::load_comms_failure);
            break;

         case state_saving_settings:
            do_commit_failure(SettingsManagerClient::commit_comms_failure);
            break;
         }
      } // on_failure


      void SettingsManager::on_sending_command(
         message_handle &command)
      {
      } // on_sending_command


      void SettingsManager::on_get_settings_ack(
         message_handle &command,
         message_handle &response)
      {
         if(state == state_loading_settings || state == state_reverting_to_defaults)
         {
            try
            {
               byte outcome = response->readByte();
               if(outcome == 1)
               {
                  // read the header of the response
                  device_type = response->readUInt2();
                  major_version = response->readByte();
                  minor_version = response->readByte();
                  bool more_settings = response->readBool();

                  // we need to look up the catalog for this device
                  bool have_catalog = (catalog != 0);
                  if(!have_catalog)
                  {
                     LibraryManager::iterator device_it = library->get_device(device_type);
                     if(device_it != library->end())
                     {
                        // we need to ensure that the device description is fully loaded before
                        // attempting to locate the catalogue.
                        try
                        {
                           device_desc = *device_it;
                           device_desc->check_loaded();
                        }
                        catch(std::exception &)
                        {
                           do_load_error(SettingsManagerClient::load_invalid_device_desc_failure);
                           return;
                        }

                        // now we can try to locate the setting catalogue
                        DeviceDesc::iterator catalog_it = device_desc->find_catalog(major_version);
                        if(catalog_it != device_desc->end())
                        {
                           catalog = *catalog_it;
                           have_catalog = true;
                        }
                        else
                           do_load_error(SettingsManagerClient::load_unknown_catalog);
                     }
                     else
                        do_load_error(SettingsManagerClient::load_unknown_device_type); 
                  }
                  if(have_catalog)
                  {
                     uint2 last_setting_id = 0;
                     while(response->whatsLeft() >= 2)
                     {
                        // read the setting information
                        uint2 setting_id = response->readUInt2();
                        bool read_only;
                        bool large_value;
                        uint2 setting_len = response->readUInt2();
                        message_handle setting_content(new Message);
                        
                        last_setting_id = setting_id;
                        large_value = (setting_len & 0x8000) != 0;
                        read_only = (setting_len & 0x4000) != 0;
                        setting_len &= 0x3FFF;
                        response->readBytes(*setting_content,setting_len);
                        
                        // we now need to create a setting object
                        SettingCatalog::iterator setting_it = catalog->get_setting(setting_id);
                        if(setting_it != catalog->end() && received_identifiers.find(setting_id) == received_identifiers.end())
                        {
                           SharedPtr<SettingDesc> &setting_desc(*setting_it);
                           if(for_factory || !setting_desc->get_factory_only())
                           {
                              value_type setting(new Setting(setting_desc));
                              if(!setting->get_read_only() && read_only)
                                 setting->set_read_only(read_only);
                              if(setting->get_read_only() && for_factory && setting_desc->get_factory_write())
                                 setting->set_read_only(false);
                              setting->set_version(major_version, minor_version);
                              settings.push_back(setting);
                              received_identifiers.insert(setting_id);
                              if(!large_value)
                                 setting->read(setting_content);
                              else
                                 get_queue.push_back(
                                    queue_element(
                                       *setting_it,
                                       setting_content)); 
                           }
                        }
                     }
                     
                     // we've processed the contents of this message. we now need to determine if
                     // the load process is complete
                     if(more_settings)
                     {
                        message_handle next_command(new Message);
                        next_command->set_message_type(Messages::get_settings_cmd);
                        next_command->addUInt2(security_code);
                        next_command->addUInt2(0x8000 | last_setting_id);
                        session->add_transaction(
                           this,
                           next_command,
                           3,
                           3000);
                     }
                     else if(!get_queue.empty())
                        do_next_load_fragment(); 
                     else
                        do_load_complete();
                  }
               }
               else if(outcome == 2)
                  do_load_error(SettingsManagerClient::load_security_failure);
               else
                  do_load_error(SettingsManagerClient::load_comms_failure);
            }
            catch(std::exception &)
            { do_load_error(SettingsManagerClient::load_comms_failure); }
         }
      } // on_get_settings_ack

      
      void SettingsManager::on_get_setting_fragment_ack(
         message_handle &command,
         message_handle &response)
      {
         if((state == state_loading_settings || state == state_reverting_to_defaults)
            && !get_queue.empty())
         {
            byte outcome = response->readByte();
            if(outcome == 1)
            {
               try
               {
                  uint2 fragment_size = response->readUInt2();
                  bool more_fragments = (fragment_size & 0x8000) != 0;
                  fragment_size &= 0x0fff;
                  queue_type::value_type &element = get_queue.front();
                  
                  response->readBytes(*element.second, fragment_size);
                  if(more_fragments)
                     do_next_load_fragment();
                  else
                  {
                     iterator si = find_setting(element.first->get_identifier());
                     if(si != settings.end())
                     {
                        value_type &setting = *si;
                        setting->read(element.second);
                     }
                     get_queue.pop_front();
                     if(!get_queue.empty())
                        do_next_load_fragment();
                     else
                        do_load_complete();
                  }
               }
               catch(std::exception &)
               { do_load_error(SettingsManagerClient::load_comms_failure); }
            }
            else
               do_load_error(SettingsManagerClient::load_security_failure);
         }
      } // on_get_setting_fragment_ack
      
      
      void SettingsManager::on_set_settings_ack(
         message_handle &command,
         message_handle &response)
      {
         using namespace SettingsManagerStrings;
         iterator error_setting;
         if(state == state_saving_settings)
         {
            byte outcome = response->readByte();
            if(outcome == 1)
            {
               // we need to check to see if all of the settings sent were accepted.
               bool settings_accepted = true;
               SettingsManagerClient::commit_failure_type failure = SettingsManagerClient::commit_unknown_error;
               while(settings_accepted && response->whatsLeft() >= 3)
               {
                  uint2 setting_id = response->readUInt2();
                  byte setting_outcome = response->readByte();
                  switch(setting_outcome)
                  {
                  case 2:
                     error_setting = find_setting(setting_id);
                     if(error_setting != end())
                     {
                        Csi::OStrAscStream msg;
                        value_type &setting(*error_setting);
                        msg << boost::format(my_strings[strid_commit_setting_not_recognised].c_str()) %
                           setting_id % setting->get_name();
                        commit_warnings.push_back(msg.str());
                     }
                     break;
                     
                  case 3:
                     error_setting = find_setting(setting_id);
                     if(error_setting != end())
                     {
                        Csi::OStrAscStream msg;
                        value_type &setting(*error_setting);
                        msg << boost::format(my_strings[strid_commit_setting_malformed].c_str()) %
                           setting_id % setting->get_name();
                        commit_warnings.push_back(msg.str());
                     }
                     break;
                     
                  case 4:
                     // Both the CS450 and the CR1000 will, at times, report settings that are
                     // otherwise treated as read/write as read only.  Instead of treating this as
                     // an error, we will instead add this as a commit warning and let the
                     // application deal with it after the settings are committed.
                     error_setting = find_setting(setting_id);
                     if(error_setting != end())
                     {
                        Csi::OStrAscStream msg;
                        value_type &setting(*error_setting);
                        msg << boost::format(my_strings[strid_commit_setting_read_only].c_str()) %
                           setting_id % setting->get_name();
                        commit_warnings.push_back(msg.str());
                     }
                     break;
                     
                  case 5:
                     settings_accepted = false;
                     failure = SettingsManagerClient::commit_out_of_memory;
                     break;
                  }
               }
               if(settings_accepted)
                  do_next_commit();
               else
                  do_commit_failure(failure);
            }
            else
            {
               switch(outcome)
               {
               case 2:
                  do_commit_failure(SettingsManagerClient::commit_security_failure);
                  break;
                  
               case 3:
                  do_commit_failure(SettingsManagerClient::commit_locked_failure);
                  break;
                  
               default:
                  do_commit_failure(SettingsManagerClient::commit_unknown_error);
                  break;
               }
            }
         }
      } // on_set_settings_ack
      
      
      void SettingsManager::on_set_setting_fragment_ack(
         message_handle &command,
         message_handle &response)
      {
         if(state == state_saving_settings)
         {
            byte outcome = response->readByte();
            switch(outcome)
            {
            case 1:
               do_next_commit();
               break;
               
            default:
               do_commit_failure(SettingsManagerClient::commit_unknown_error);
               break;
            }
         }
      } // on_set_setting_fragment_ack
      
      
      void SettingsManager::on_control_ack(
         message_handle &command,
         message_handle &response)
      {
         response->reset();
         byte outcome = response->readByte();
         switch(state)
         {
         case state_saving_settings:
            switch(outcome)
            {
            case 1:
            case 3:
               do_commit_complete(false);
               break;

            case 2:
               do_commit_failure(SettingsManagerClient::commit_security_failure);
               break;

            case 7:
               do_commit_failure(SettingsManagerClient::commit_locked_failure);
               break;

            case 8:
               do_commit_failure(SettingsManagerClient::commit_system_error);
               break;
               
            default:
               do_commit_failure(SettingsManagerClient::commit_unknown_error);
               break;
            }
            break;

         case state_reverting_to_defaults:
            switch(outcome)
            {
            case 2:
               do_load_error(SettingsManagerClient::load_security_failure);
               break;
               
            case 5:
            {
               message_handle get_cmd(new Message);
               get_cmd->set_message_type(Messages::get_settings_cmd);
               get_cmd->addUInt2(security_code);
               settings.clear();
               session->add_transaction(this,get_cmd,3,3000);
               break;
            }
            
            case 7:
               do_load_error(SettingsManagerClient::load_locked_failure);
               break;
               
            default:
               do_load_error(SettingsManagerClient::load_unknown_error);
               break;
            }
            break;
            
         case state_cancelling:
            switch(outcome)
            {
            case 2:
               do_commit_failure(SettingsManagerClient::commit_security_failure);
               break;

            case 1:
            case 3:
            case 4:
               do_commit_complete(true);
               break;

            case 6:
               do_rollback_for_reboot();
               break;

            default:
               do_commit_failure(SettingsManagerClient::commit_unknown_error);
               break;
            }
            break;

         case state_reboot_rollback:
            switch(outcome)
            {
            case 1:
               do_commit_complete(true);
               break;
               
            case 3:
            case 4:
               do_reboot();
               break;
               
            default:
               do_commit_failure(SettingsManagerClient::commit_unknown_error);
               break;
            }
            break;
            
         case state_reboot_commit:
            switch(outcome)
            {
            case 1:
            case 3:
            case 4:
               do_commit_complete(true);
               break;
               
            default:
               do_commit_failure(SettingsManagerClient::commit_unknown_error);
               break; 
            }
            break;

         case state_active:
            switch(outcome)
            {
            case ControlCodes::outcome_wifi_scan_started:
               do_report_wifi_scan();
               break;
            }
         }
      } // on_control_ack


      namespace
      {
         ////////////////////////////////////////////////////////////
         // class call_on_started
         ////////////////////////////////////////////////////////////
         class call_on_started
         {
         public:
            ////////////////////////////////////////////////////////////
            // manager
            ////////////////////////////////////////////////////////////
            SettingsManager *manager;

            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            call_on_started(SettingsManager *manager_):
               manager(manager_)
            { }

            ////////////////////////////////////////////////////////////
            // evaluation method
            ////////////////////////////////////////////////////////////
            void operator ()(SettingsManagerClient *client)
            {
               if(SettingsManagerClient::is_valid_instance(client))
                  client->on_started(manager);
            }
         };


         ////////////////////////////////////////////////////////////
         // class call_on_settings_reverted
         ////////////////////////////////////////////////////////////
         class call_on_settings_reverted
         {
         public:
            ////////////////////////////////////////////////////////////
            // manager
            ////////////////////////////////////////////////////////////
            SettingsManager *manager;

            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            call_on_settings_reverted(SettingsManager *manager_):
               manager(manager_)
            { }

            ////////////////////////////////////////////////////////////
            // evaluator
            ////////////////////////////////////////////////////////////
            void operator ()(SettingsManagerClient *client)
            {
               if(SettingsManagerClient::is_valid_instance(client))
                  client->on_settings_reverted(manager);
            }
         };


         ////////////////////////////////////////////////////////////
         // class clear_has_changed
         ////////////////////////////////////////////////////////////
         class clear_has_changed
         {
         public:
            ////////////////////////////////////////////////////////////
            // evaluator
            ////////////////////////////////////////////////////////////
            void operator ()(Csi::SharedPtr<Setting> &setting)
            { setting->clear_has_changed(); }
         };
      };


      namespace
      {
         ////////////////////////////////////////////////////////////
         // predicate sort_order_less
         ////////////////////////////////////////////////////////////
         struct sort_order_less
         {
            bool operator ()(SettingsManager::value_type const &s1, SettingsManager::value_type const &s2)
            { return s1->get_sort_order() < s2->get_sort_order(); }
         };
      };
      
      
      void SettingsManager::do_load_complete()
      {
         state_type old_state = state;
         clients_type temp(clients);
         state = state_active;
         switch(old_state)
         {
         case state_loading_settings:
            settings.sort(sort_order_less());
            std::for_each(
               settings.begin(),
               settings.end(),
               clear_has_changed());
            std::for_each(
               temp.begin(),
               temp.end(),
               call_on_started(this));
            break;
            
         case state_reverting_to_defaults:
            std::for_each(
               settings.begin(),
               settings.end(),
               clear_has_changed());
            marked_as_changed = true;
            std::for_each(
               temp.begin(),
               temp.end(),
               call_on_settings_reverted(this));
            break;
         }
      } // do_load_complete


      void SettingsManager::do_next_load_fragment()
      {
         message_handle next_command(new Message);
         next_command->set_message_type(Messages::get_setting_fragment_cmd);
         next_command->addUInt2(security_code);
         next_command->addUInt2(
            get_queue.front().first->get_identifier());
         next_command->addUInt4(
            get_queue.front().second->get_body_len());
         session->add_transaction(
            this,
            next_command,
            3,
            3000);
      } // do_next_load_fragment


      namespace
      {
         ////////////////////////////////////////////////////////////
         // class call_on_load_failure
         ////////////////////////////////////////////////////////////
         class call_on_load_failure
         {
         public:
            ////////////////////////////////////////////////////////////
            // manager
            ////////////////////////////////////////////////////////////
            SettingsManager *manager;

            ////////////////////////////////////////////////////////////
            // failure
            ////////////////////////////////////////////////////////////
            SettingsManagerClient::load_failure_type failure;

            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            call_on_load_failure(
               SettingsManager *manager_,
               SettingsManagerClient::load_failure_type failure_):
               manager(manager_),
               failure(failure_)
            { }

            ////////////////////////////////////////////////////////////
            // evaluator
            ////////////////////////////////////////////////////////////
            void operator ()(SettingsManagerClient *client)
            {
               if(SettingsManagerClient::is_valid_instance(client))
                  client->on_load_failure(manager,failure);
            }
         };
      };

      
      void SettingsManager::do_load_error(load_failure_type failure)
      {
         clients_type temp(clients);
         clients.clear();
         state = state_standby;
         commit_queue.clear();
         get_queue.clear();
         session.clear();
         std::for_each(
            temp.begin(),
            temp.end(),
            call_on_load_failure(this,failure));
      } // do_load_error


      void SettingsManager::do_next_commit()
      {
         // we need to determine the next command that should be sent to the device.  If the set
         // queue is not empty, the next command will be something to get the settings out,
         // otherwise, we will send a control command with a commit option
         uint4 const setting_header_len(8);
         uint4 const max_body_len(988);
         uint4 const max_fragment_len(max_body_len - setting_header_len);
         if(!commit_queue.empty())
         {
            // there are, apparently, more setting contents to send.  We will use the set fragment
            // transaction if the first element is larger than the max size or if that setting has
            // already been sent as a fragment
            message_handle send_cmd(new Message);
            message_handle first = commit_queue.front().second;

            send_cmd->addUInt2(security_code);
            if(first->getReadIdx() > first->get_headerLen() || first->whatsLeft() > max_fragment_len)
            {
               uint2 frag_len = static_cast<uint2>(csimin<uint4>(first->whatsLeft(), max_fragment_len));
               uint2 flags = frag_len;
               
               if(first->whatsLeft() > max_fragment_len)
                  flags |= 0x8000;
               send_cmd->addUInt2(commit_queue.front().first->get_identifier());
               if(first->getReadIdx() > first->get_headerLen())
               {
                  send_cmd->set_message_type(Messages::set_setting_fragment_cmd);
                  send_cmd->addUInt4(first->getReadIdx() - first->get_headerLen());
               }
               else
                  send_cmd->set_message_type(Messages::set_settings_cmd);
               send_cmd->addUInt2(flags);
               first->readBytes(*send_cmd, frag_len);
               if((flags & 0x8000) == 0)
                  commit_queue.pop_front();
            }
            else
            {
               // we are here because the first setting is small enough to go in a regular set
               // command.  We want to pack as many settings as possible into this command.
               uint2 bytes_written = 0;
               
               send_cmd->set_message_type(Messages::set_settings_cmd);
               while(!commit_queue.empty() && bytes_written + first->whatsLeft() <= max_body_len)
               {
                  bytes_written += static_cast<uint2>(first->whatsLeft() + setting_header_len);
                  send_cmd->addUInt2(commit_queue.front().first->get_identifier());
                  send_cmd->addUInt2(static_cast<uint2>(first->whatsLeft()));
                  first->readBytes(*send_cmd,first->whatsLeft());
                  commit_queue.pop_front();
                  if(!commit_queue.empty())
                     first = commit_queue.front().second;
               }
            }

            // the command (set settings or set setting fragment) has now been formed. it needs to
            // be sent
            session->add_transaction(
               this,
               send_cmd,
               3,
               3000); 
         }
         else
         {
            message_handle commit_cmd(new Message);
            commit_cmd->set_message_type(Messages::control_cmd);
            commit_cmd->addUInt2(security_code); // zero security code
            commit_cmd->addByte(ControlCodes::action_commit_changes);
            session->add_transaction(
               this,
               commit_cmd,
               3,
               5000);
         }
      } // do_next_commit


      namespace
      {
         ////////////////////////////////////////////////////////////
         // class call_on_commit_complete
         ////////////////////////////////////////////////////////////
         class call_on_commit_complete
         {
         public:
            ////////////////////////////////////////////////////////////
            // manager
            ////////////////////////////////////////////////////////////
            SettingsManager *manager;

            ////////////////////////////////////////////////////////////
            // cancelled
            ////////////////////////////////////////////////////////////
            bool cancelled;

            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            call_on_commit_complete(
               SettingsManager *manager_,
               bool cancelled_):
               manager(manager_),
               cancelled(cancelled_)
            { }

            ////////////////////////////////////////////////////////////
            // evaluator
            ////////////////////////////////////////////////////////////
            void operator ()(SettingsManagerClient *client)
            {
               if(SettingsManagerClient::is_valid_instance(client))
                  client->on_settings_committed(manager,cancelled);
            }
         };
      };


      void SettingsManager::do_commit_complete(bool cancelled)
      {
         clients_type temp(clients);

         if(session != 0)
         {
            std::copy(
               session->get_commit_warnings().begin(),
               session->get_commit_warnings().end(),
               std::back_inserter(commit_warnings));
         }
         state = state_standby;
         session.clear();
         clients.clear();
         settings.clear();
         get_queue.clear();
         commit_queue.clear();
         marked_as_changed = false;
         std::for_each(
            temp.begin(),
            temp.end(),
            call_on_commit_complete(this,cancelled));
      } // do_commit_complete


      namespace
      {
         ////////////////////////////////////////////////////////////
         // class call_on_commit_failure
         ////////////////////////////////////////////////////////////
         class call_on_commit_failure
         {
         public:
            ////////////////////////////////////////////////////////////
            // manager
            ////////////////////////////////////////////////////////////
            SettingsManager *manager;

            ////////////////////////////////////////////////////////////
            // failure
            ////////////////////////////////////////////////////////////
            typedef SettingsManagerClient::commit_failure_type failure_type;
            failure_type failure;

            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            call_on_commit_failure(SettingsManager *manager_, failure_type failure_):
               manager(manager_),
               failure(failure_)
            { }

            ////////////////////////////////////////////////////////////
            // evaluator
            ////////////////////////////////////////////////////////////
            void operator ()(SettingsManagerClient *client)
            {
               if(SettingsManagerClient::is_valid_instance(client))
                  client->on_commit_failure(manager,failure);
            }
         };
      };


      void SettingsManager::do_commit_failure(commit_failure_type failure)
      {
         clients_type temp(clients);
         
         state = state_standby;
         session.clear();
         clients.clear();
         settings.clear();
         get_queue.clear();
         commit_queue.clear();
         std::for_each(
            temp.begin(),
            temp.end(),
            call_on_commit_failure(this,failure));
      } // do_commit_failure


      void SettingsManager::do_report_wifi_scan()
      {
         clients_type temp(clients);
         for(clients_type::iterator ci = temp.begin(); ci != temp.end(); ++ci)
            (*ci)->on_wifi_scan_complete(this);
      } // do_report_wifi_scan


      void SettingsManager::do_rollback_for_reboot()
      {
         message_handle rollback(new Message);
         rollback->set_message_type(Messages::control_cmd);
         rollback->addUInt2(security_code);
         rollback->addByte(ControlCodes::action_cancel_without_reboot);
         state = state_reboot_rollback;
         session->add_transaction(this,rollback,3,3000); 
      } // do_rollback_for_reboot


      void SettingsManager::do_reboot()
      {
         message_handle rollback(new Message);
         rollback->set_message_type(Messages::control_cmd);
         rollback->addUInt2(security_code);
         rollback->addByte(ControlCodes::action_commit_changes);
         state = state_reboot_commit;
         session->add_transaction(this,rollback,3,3000);
      } // do_reboot
   };
};

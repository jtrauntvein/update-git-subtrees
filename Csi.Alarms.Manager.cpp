/* Csi.Alarms.Manager.cpp

   Copyright (C) 2012, 2014 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Tuesday 25 September 2012
   Last Change: Thursday 06 November 2014
   Last Commit: $Date: 2016-11-17 14:43:10 -0600 (Thu, 17 Nov 2016) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Alarms.Manager.h"


namespace Csi
{
   namespace Alarms
   {
      ////////////////////////////////////////////////////////////
      // class Manager definitions
      ////////////////////////////////////////////////////////////
      Manager::Manager(sources_handle &sources_):
         sources(sources_),
         exec_actions_allowed(true),
         was_started(false)
      { token_factory.bind(new Expression::TokenFactory); }


      Manager::~Manager()
      {
         alarms.clear();
         sources.clear();
         token_factory.clear();
      } // destructor


      namespace
      {
         StrUni const alarm_name(L"alarm");
         StrUni const profiles_name(L"EmailProfiles");
         StrUni const profile_name(L"EmailProfile");
         StrUni const log_name(L"log");
         StrUni const log_dir_name(L"directory");
         StrUni const log_file_name(L"base-file-name");
         StrUni const log_bale_count_name(L"count");
         StrUni const log_bale_size_name(L"size");
         StrUni const log_bale_interval_name(L"interval");
         StrUni const log_enabled_name(L"enabled");
      };

      
      void Manager::read(
         Xml::Element &elem,
         read_errors_type &errors,
         AlarmClient *client)
      {
         using namespace Xml;
         alarms.clear();
         profiles.clear();
         log.clear();
         for(Element::iterator ei = elem.begin();
             ei != elem.end();
             ++ei)
         {
            Element::value_type &child(*ei);
            if(child->get_name() == alarm_name)
            {
               try
               {
                  value_type alarm(new Alarm(this));
                  alarm->read(*child);
                  if(client)
                     alarm->set_client(client);
                  alarms.push_back(alarm);
               }
               catch(std::exception &e)
               {
                  Csi::OStrAscStream msg;
                  msg << "alarm read error\",\"" << e.what();
                  errors.push_back(msg.str());
               }
            }
            else if(child->get_name() == profiles_name)
            {
               for(Element::iterator pi = child->begin();
                   pi != child->end();
                   ++pi)
               {
                  Element::value_type &profile_xml(*pi);
                  profile_handle profile(new EmailProfile);
                  profile->read(*profile_xml);
                  profiles[profile->get_unique_id()] = profile;
               }
            }
            else if(child->get_name() == log_name)
               set_log(*child);
         }
      } // read


      void Manager::write(Xml::Element &elem)
      {
         using namespace Xml;
         Element::value_type profiles_xml(elem.add_element(profiles_name));
         
         for(alarms_type::iterator ai = alarms.begin();
             ai != alarms.end();
             ++ai)
         {
            value_type &alarm(*ai);
            Element::value_type alarm_xml(elem.add_element(alarm_name));
            alarm->write(*alarm_xml);
         }
         for(profiles_type::iterator pi = profiles.begin();
             pi != profiles.end();
             ++pi)
         {
            Element::value_type profile_xml(profiles_xml->add_element(profile_name));
            pi->second->write(*profile_xml);
         }
         if(log != 0)
         {
            Element::value_type log_xml(elem.add_element(log_name));
            get_log(*log_xml);
         }
      } // write


      void Manager::start()
      {
         was_started = true;
         for(alarms_type::iterator ai = alarms.begin();
             ai != alarms.end();
             ++ai)
         {
            value_type &alarm(*ai);
            alarm->start();
         }
      } // start


      void Manager::stop()
      {
         was_started = false;
         for(alarms_type::iterator ai = alarms.begin();
             ai != alarms.end();
             ++ai)
         {
            value_type &alarm(*ai);
            alarm->stop();
         }
      } // stop


      void Manager::add_alarm(value_type alarm)
      {
         alarms.push_back(alarm);
         if(was_started)
            alarm->start();
      } // add_alarm


      Manager::value_type Manager::clone_alarm(Alarm *alarm)
      {
         value_type rtn;
         iterator ai(
            std::find_if(begin(), end(), HasSharedPtr<Alarm>(alarm)));
         if(ai != alarms.end())
         {
            // we will create a new alarm object but we will first want to get the properties for
            // the alarm to be cloned.  We will also will remove the ID property so that the new
            // alarm can use its own identifier.
            Xml::Element properties(L"properties");
            Xml::Element::value_type conditions;
            alarm->write(properties);
            conditions = properties.find_elem(L"conditions", 0, true);
            for(Xml::Element::iterator ci = conditions->begin(); ci != conditions->end(); ++ci)
            {
               Xml::Element::value_type &condition(*ci);
               Xml::Element::value_type actions(condition->find_elem(L"actions", 0, true));
               actions->clear();
            }
            properties.remove_attribute(L"id");

            // with all of the actions stripped from the alarm configuration, we can now generate a
            // new alarm
            rtn.bind(new Alarm(this));
            alarms.push_back(rtn);
            rtn->read(properties);
            if(was_started)
               rtn->start();
         }
         return rtn;
      } // clone_alarm


      void Manager::remove_alarm(Alarm *alarm_)
      {
         iterator ai(
            std::find_if(begin(), end(), HasSharedPtr<Alarm>(alarm_)));
         if(ai != alarms.end())
         {
            value_type alarm(*ai);
            actions_type::iterator aci(actions.begin());
            alarms.erase(ai);
            if(was_started)
               alarm->stop();
         }
      } // remove_alarm


      Manager::profile_handle Manager::create_profile()
      {
         profile_handle rtn(new EmailProfile);
         OStrUniStream name;
         
         name << L"EmailProfile_" << rtn->get_unique_id();
         rtn->set_name(name.str());
         profiles[rtn->get_unique_id()] = rtn;
         return rtn;
      } // create_profile


      void Manager::add_profile(profile_handle profile)
      {
         profiles[profile->get_unique_id()] = profile;
      }


      Manager::profile_handle Manager::find_profile_id(StrUni const &id)
      {
         profiles_type::iterator pi(profiles.find(id));
         profile_handle rtn;
         if(pi != profiles.end())
            rtn = pi->second;
         return rtn;
      }

      
      Manager::profile_handle Manager::find_profile_name(StrUni const &name)
      {
         profile_handle rtn;
         for(profiles_type::iterator pi = profiles.begin();
             rtn == 0 && pi != profiles.end();
             ++pi)
         {
            if(pi->second->get_name() == name)
               rtn = pi->second;
         }               
         return rtn;
      } // find_profile


      void Manager::remove_profile(StrUni const &id)
      {
         profiles_type::iterator pi(profiles.find(id));
         if(pi != profiles.end())
            profiles.erase(pi);
      } // remove_profile


      void Manager::add_action(action_handle action)
      {
         actions.push_back(action);
         if(current_action == 0)
            do_next_action();
      } // add_action


      void Manager::stop_actions_for_alarm(Alarm *alarm)
      {
         actions_type::iterator ai(actions.begin());
         while(ai != actions.end())
         {
            action_handle &candidate(*ai);
            if(candidate->get_alarm() == alarm)
            {
               actions_type::iterator dai(ai++);
               actions.erase(dai);
            }
            else
               ++ai;
         }
         if(current_action != 0 && current_action->get_alarm() == alarm)
            current_action.clear();
      } // stop_actions_for_alarm


      void Manager::on_action_complete(ActionBase *action)
      {
         if(current_action == action)
         {  
            current_action.clear();
            do_next_action();
         }
         else
         {
            actions_type::iterator ai(
               std::find_if(
                  actions.begin(), actions.end(), HasSharedPtr<ActionBase>(action)));
            if(ai != actions.end())
               actions.erase(ai);
         }
      } // on_action_complete


      uint4 Manager::pending_actions_for_alarm(Alarm *alarm)
      {
         uint4 rtn(0);
         if(alarm == 0)
         {
            rtn = (uint4)actions.size();
            if(current_action != 0)
               ++rtn;
         }
         else
         {
            if(current_action != 0 && current_action->get_alarm() == alarm)
               ++rtn;
            for(actions_type::iterator ai = actions.begin();
                ai != actions.end();
                ++ai)
            {
               action_handle &action(*ai);
               if(action->get_alarm() == alarm)
                  ++rtn;
            }
         }
         return rtn;
      } // pending_actions_for_alarm


      void Manager::remove_actions_for_alarm(Alarm *alarm)
      {
         actions_type::iterator ai(actions.begin());
         if(current_action != 0 && current_action->get_alarm() == alarm)
            current_action.clear();
         while(ai != actions.end())
         {
            action_handle action(*ai);
            if(action->get_alarm() == alarm)
            {
               actions_type::iterator dai(ai++);
               actions.erase(dai);
            }
            else
               ++ai;
         }
      } // remove_actions_for_alarm


      Manager::action_handle Manager::find_action(ActionBase *action)
      {
         action_handle rtn;
         if(current_action == action)
            rtn = current_action;
         else
         {
            actions_type::iterator ai(
               std::find_if(actions.begin(), actions.end(), HasSharedPtr<ActionBase>(action)));
            if(ai != actions.end())
               rtn = *ai;
         }
         return rtn;
      } // find_action
      

      void Manager::add_log(Xml::Element &elem)
      {
         if(log != 0)
         {
            elem.set_attr_lgrdate(LgrDate::system(), L"date");
            AlarmLogEvent event(elem);
            log->wr(event);
         }
      } // add_log


      void Manager::get_log(Xml::Element &elem)
      {
         if(log != 0)
         {
            elem.set_attr_str(log->get_path(), log_dir_name);
            elem.set_attr_str(log->get_file_name(), log_file_name);
            elem.set_attr_uint4(log->get_baleSize(), log_bale_size_name);
            elem.set_attr_uint4(log->get_baleCnt(), log_bale_count_name);
            if(log->get_time_based_baling())
               elem.set_attr_int8(log->get_baling_interval(), log_bale_interval_name);
            elem.set_attr_bool(log->isEnabled(), log_enabled_name);
         }
         else
         {
            elem.set_attr_uint4(1048576, log_bale_size_name);
            elem.set_attr_uint4(4, log_bale_count_name);
            elem.set_attr_bool(false, log_enabled_name);
         }
      } // set_log
      

      void Manager::set_log(Xml::Element &elem)
      {
         StrAsc dir(elem.get_attr_str(log_dir_name));
         StrAsc file_name(elem.get_attr_str(log_file_name));
         uint4 count(elem.get_attr_uint4(log_bale_count_name));
         uint4 size(1048576);

         if(alarm_log_dir.length() > 0)
            dir = alarm_log_dir;
         log.bind(new AlarmLogger(this, dir.c_str(), file_name.c_str()));
         if(elem.has_attribute(log_bale_interval_name))
         {
            int8 interval(elem.get_attr_int8(log_bale_interval_name));
            log->setBaleParams(size, count);
            log->set_time_based_baling(true, interval, get_timer());
         }
         else if(elem.has_attribute(log_bale_size_name))
         {
            size = elem.get_attr_uint4(log_bale_size_name);
            log->setBaleParams(size, count);
         }
         else
            throw std::invalid_argument("invalid log baling parameters");
         if(elem.has_attribute(log_enabled_name))
            log->setEnable(elem.get_attr_bool(log_enabled_name));
         else
            log->setEnable(false);
      } // set_log
      
      
      void Manager::do_next_action()
      {
         if(!actions.empty() && current_action == 0)
         {
            current_action = actions.front();
            actions.pop_front();
            current_action->execute();
         }
      } // do_next_action 
   };
};



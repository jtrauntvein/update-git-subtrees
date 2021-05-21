/* Csi.Alarms.Alarm.cpp

   Copyright (C) 2012, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Monday 24 September 2012
   Last Change: Tuesday 27 November 2018
   Last Commit: $Date: 2018-11-29 13:29:49 -0600 (Thu, 29 Nov 2018) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#define _SCL_SECURE_NO_WARNINGS
#include "Csi.Alarms.Alarm.h"
#include "Csi.Alarms.Manager.h"
#include "Csi.Utils.h"


namespace Csi
{
   namespace Alarms
   {
      namespace
      {
         ////////////////////////////////////////////////////////////
         // class event_action_complete
         ////////////////////////////////////////////////////////////
         class event_action_complete: public Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // action
            ////////////////////////////////////////////////////////////
            ActionBase *action;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(Alarm *alarm, ActionBase *action)
            {
               event_action_complete *event(new event_action_complete(alarm, action));
               event->post();
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_action_complete(Alarm *alarm, ActionBase *action_):
               Event(event_id, alarm),
               action(action_)
            { }
         };


         uint4 const event_action_complete::event_id(
            Event::registerType("Csi::Alarms::Alarm::event_action_complete"));
      };

      
      ////////////////////////////////////////////////////////////
      // class Alarm definitions
      ////////////////////////////////////////////////////////////
      Alarm::Alarm(Manager *manager_):
         manager(manager_),
         for_table(false),
         latch_alarms(false),
         acknowledged(false),
         client(0),
         actions_enabled(true)
      { id = make_guid(); }


      Alarm::~Alarm()
      {
         if(manager != 0 && manager->get_sources() != 0)
         {
            if(Csi::InstanceValidator::is_valid_instance<Csi::EvReceiver>(manager->get_sources().get_rep()))
               manager->get_sources()->remove_requests(this);
         }
         conditions.clear();
      } // destructor


      Csi::SharedPtr<OneShot> &Alarm::get_timer()
      { return manager->get_timer(); }


      StrUni const Alarm::alarm_name_name(L"name");
      StrUni const Alarm::id_name(L"id");
      StrUni const Alarm::conditions_name(L"conditions");
      StrUni const Alarm::condition_name(L"condition");
      StrUni const Alarm::source_expression_name(L"source");
      StrUni const Alarm::latched_name(L"latched");
      

      void Alarm::read(Xml::Element &elem)
      {
         using namespace Xml;
         Element::value_type conditions_xml(elem.find_elem(conditions_name));
         Element::value_type source_expression_xml(elem.find_elem(source_expression_name));
         
         name = elem.get_attr_wstr(alarm_name_name);
         if(elem.has_attribute(latched_name))
            latch_alarms = elem.get_attr_bool(latched_name);
         else
            latch_alarms = false;
         if(elem.has_attribute(id_name))
            id = elem.get_attr_wstr(id_name);
         if(manager != 0)
            manager->remove_actions_for_alarm(this);
         conditions.clear();
         try
         {
            for(Element::iterator ci = conditions_xml->begin();
                ci != conditions_xml->end();
                ++ci)
            {
               Element::value_type &condition_xml(*ci);
               if(condition_xml->get_name() == condition_name)
               {
                  value_type condition(new Condition(this));
                  condition->read(*condition_xml);
                  conditions.push_back(condition);
               }
            }
            source_expression_str = source_expression_xml->get_cdata_wstr();
            requests.clear();
            manager->get_sources()->remove_requests(this);
            source_expression = Expression::TokenFactory::make_expression(
               this,
               source_expression_str,
               requests,
               L"",
               manager->get_token_factory().get_rep(),
               Cora::DataSources::Request::order_logged_without_holes);
         }
         catch(std::exception &e)
         {
            Csi::OStrAscStream msg;
            msg << id << "\",\"" << name << "\",\"" << e.what();
            throw std::invalid_argument(msg.c_str());
         }
      } // read


      void Alarm::write(Xml::Element &elem)
      {
         using namespace Xml;
         Element::value_type conditions_xml(elem.add_element(conditions_name));
         Element::value_type source_expression_xml(elem.add_element(source_expression_name));
         source_expression_xml->set_cdata_wstr(source_expression_str);
         elem.set_attr_wstr(name, alarm_name_name);
         elem.set_attr_wstr(id, id_name);
         elem.set_attr_bool(latch_alarms, latched_name);
         for(conditions_type::iterator ci = conditions.begin();
             ci != conditions.end();
             ++ci)
         {
            value_type &condition(*ci);
            Element::value_type condition_xml(conditions_xml->add_element(condition_name));
            condition->write(*condition_xml);
         }
      } // write

      
      Alarm::state_type Alarm::get_state() const
      {
         state_type rtn(state_off);
         if(triggered_condition != 0 && !acknowledged)
            rtn = state_on;
         else if(triggered_condition != 0 && acknowledged)
            rtn = state_acked;
         return rtn;
      } // get_state


      bool Alarm::has_on_condition() const
      {
         bool rtn(triggered_condition != 0);
         for(conditions_type::const_iterator ci = conditions.begin();
             !rtn && ci != conditions.end();
             ++ci)
         {
            value_type const &condition(*ci);
            rtn = condition->get_test()->has_on_condition();
         }
         return rtn;
      } // has_on_condition


      void Alarm::on_sink_ready(
         sources_type *sources,
         request_handle &request,
         record_handle &record)
      {
         using namespace Cora::DataSources;
         SourceBase *source(request->get_source());
         SourceBase::symbols_type symbols;
         source->breakdown_uri(symbols, request->get_uri());
         if(symbols.back().second == SymbolBase::type_table)
            for_table = true;
         else
            for_table = false;
         last_error.cut(0);
         if(AlarmClient::is_valid_instance(client))
            client->on_alarm_change(this);
         else
            client = 0;
      } // on_sink_ready
      

      void Alarm::on_sink_failure(
         sources_type *sources,
         request_handle &request,
         sink_failure_type failure)
      {
         Csi::OStrAscStream temp;
         temp << "request failure for \"" << request->get_uri() << "\": ";
         format_sink_failure(temp, failure);
         last_error = temp.str();
         if(AlarmClient::is_valid_instance(client))
            client->on_alarm_change(this);
         else
            client = 0;
      } // on_sink_failure


      void Alarm::on_sink_records(
         sources_type *sources,
         requests_type &requests,
         records_type const &records)
      {
         try
         {
            last_error.cut(0);
            for(records_type::const_iterator ri = records.begin();
                ri != records.end();
                ++ri)
            {
               // assign all of the values for all of the requests that might use this record
               records_type::value_type const &record(*ri);
               bool ignored(false);
               for(requests_type::iterator qi = requests.begin();
                   qi != requests.end();
                   ++qi)
               {
                  requests_type::value_type request(*qi);
                  requests_type::iterator iri(
                     std::find(ignore_requests.begin(), ignore_requests.end(), request));
                  if(!for_table)
                     source_expression->assign_request_variables(*record, *request);
                  if(iri != ignore_requests.end())
                  {
                     ignored = true;
                     ignore_requests.erase(iri);
                  }
               }
               if(!ignored)
               {
                  if(!for_table)
                  {
                     try
                     {
                        last_value = source_expression->eval();
                        if(AlarmClient::is_valid_instance(client))
                           client->on_last_value_changed(this, last_value);
                     }
                     catch(std::exception &)
                     { }
                  }
                  process_record(record);
               }
            }
         }
         catch(std::exception &e)
         {
            OStrAscStream temp;
            temp << "error processing records: \"" << e.what() << "\"";
            last_error = temp.str();
         }
      } // on_sink_records


      void Alarm::ignore_next_record(StrUni const &uri)
      {
         StrUni table_uri(manager->get_sources()->make_table_uri(uri));
         StrUni request_table_uri;
         for(requests_type::iterator ri = requests.begin();
             ri != requests.end();
             ++ri)
         {
            requests_type::value_type &request(*ri);
            request_table_uri = manager->get_sources()->make_table_uri(request->get_uri());
            if(request_table_uri == table_uri)
               ignore_requests.push_back(request);
         }
      } // ignore_next_record
      

      void Alarm::process_record(record_handle const &record)
      {
         // we need to evaluate whether there are any conditions that are currently triggered.
         conditions_type triggered_now;
         if(!for_table)
         {
            for(conditions_type::iterator ci = conditions.begin(); ci != conditions.end(); ++ci)
            {
               value_type &condition(*ci);
               if(last_value.get_rep() != 0)
               {
                  if(condition->on_value(last_value))
                     triggered_now.push_back(condition);
               }
               else //Look for no data trigger with no data yet - Issue #33846
               {
                  record_handle nothing;
                  if(nothing == record)
                  {
                     triggered_now.push_back(condition);
                  }
               }
            }
         }
         else
         {
            for(conditions_type::iterator ci = conditions.begin(); ci != conditions.end(); ++ci)
            {
               value_type &condition(*ci);
               if(condition->on_record(record))
                  triggered_now.push_back(condition);
            }
         }

         // we need to evaluate whether the triggered condition has changed
         if(triggered_condition != 0)
         {
            conditions_type::iterator ci(
               std::find(triggered_now.begin(), triggered_now.end(), triggered_condition));
            bool notify(true);
            if(ci == triggered_now.end() && !(latch_alarms && !acknowledged))
            {
               on_alarm_off();
               if(!triggered_now.empty())
               {
                  triggered_condition = triggered_now.front();
                  on_alarm_on();
                  notify = false;
               }
            }
            if(notify && AlarmClient::is_valid_instance(client))
               client->on_alarm_change(this);
         }
         else if(!triggered_now.empty())
         {
            triggered_condition = triggered_now.front();
            on_alarm_on();
         }
         else if(AlarmClient::is_valid_instance(client))
            client->on_alarm_change(this);

         // some alarms may use a const expression in their off expressions to allow the alarm to
         // immediately return to an off state.  Value forwarders particularly will do this.  In
         // order to support this, we will re-evaluate whether the alarm is triggered.  We do this
         // here in order to allow the client to see the alarm in a triggered state as well as in an
         // off state.
         if(triggered_condition != 0 &&
            !triggered_condition->is_triggered() &&
            (!latch_alarms || acknowledged))
            on_alarm_off();
      } // process_record

      
      void Alarm::acknowledge(StrUni const &comments)
      {
         if(triggered_condition != 0 && !acknowledged)
         {
            Xml::Element log_xml(L"alarm-acknowledged");
            Xml::Element::value_type condition_xml(log_xml.add_element(L"condition"));
            OStrAscStream exit;
            acknowledged = true;
            log_xml.add_element(L"source")->set_cdata_wstr(annotate_source_expression());
            log_xml.add_element(L"comments")->set_cdata_wstr(comments);
            condition_xml->set_attr_wstr(triggered_condition->get_name(), L"name");
            triggered_condition->get_test()->format_exit(exit);
            condition_xml->add_element(L"exit")->set_cdata_str(exit.str());
            add_log(log_xml);
            triggered_condition->on_alarm_off();
            if(!triggered_condition->is_triggered())
            {
               acknowledged = false;
               on_alarm_off();
            }
         }
      } // acknowledge


      SharedPtr<Expression::TokenFactory> &Alarm::get_token_factory()
      { return manager->get_token_factory(); }


      void Alarm::start()
      {
         if(source_expression != 0)
         {
            typedef Expression::token_stack_type tokens_type;
            tokens_type &tokens(source_expression->get_postfix_stack());
            for(tokens_type::iterator ti = tokens.begin(); ti != tokens.end(); ++ti)
            {
               tokens_type::value_type &token(*ti);
               if(token->has_state())
                  token->reset_state();
            }
         }
         last_value.clear();
         last_error = "waiting for requests";
         manager->get_sources()->remove_requests(this);
         for_table = false;
         for(conditions_type::iterator ci = conditions.begin();
             ci != conditions.end();
             ++ci)
         {
            value_type &condition(*ci);
            condition->on_started();
         }
         for(requests_type::iterator ri = requests.begin();
             ri != requests.end();
             ++ri)
         {
            manager->get_sources()->add_request(*ri, false);
         }
         manager->get_sources()->activate_requests();
      } // start


      void Alarm::stop()
      {
         last_value.clear();
         last_error.cut(0);
         for_table = false;
         manager->get_sources()->remove_requests(this);
         manager->stop_actions_for_alarm(this);
         for(conditions_type::iterator ci = conditions.begin();
             ci != conditions.end();
             ++ci)
         {
            value_type &condition(*ci);
            condition->on_stopped();
         }
         triggered_condition.clear();
         acknowledged = false;
      } // stop


      StrUni Alarm::annotate_source_expression()
      {
         StrUni rtn(source_expression_str);
         if(source_expression != 0)
            source_expression->annotate_source(rtn);
         return rtn;
      } // annotate_source_expression


      uint4 Alarm::get_pending_actions()
      { return manager->pending_actions_for_alarm(this); }


      void Alarm::add_action(action_handle action)
      {
         if(actions_enabled)
         {
            Xml::Element log_xml(L"action-started");
            Xml::Element::value_type condition_xml(log_xml.add_element(L"condition"));
            Xml::Element::value_type action_xml(log_xml.add_element(L"action"));
            OStrAscStream entrance;
            
            condition_xml->set_attr_wstr(action->get_condition()->get_name(), L"name");
            action->get_condition()->get_test()->format_entrance(entrance);
            condition_xml->add_element(L"entrance")->set_cdata_str(entrance.str());
            action_xml->set_attr_wstr(action->get_template()->get_type_name(), L"type");
            action->describe_log(*action_xml);
            add_log(log_xml);
            manager->add_action(action);
         }
      } // add_action


      void Alarm::on_action_complete(ActionBase *action)
      {
         Xml::Element log_xml(L"action-complete");
         Xml::Element::value_type condition_xml(log_xml.add_element(L"condition"));
         Xml::Element::value_type action_xml(log_xml.add_element(L"action"));
         OStrAscStream entrance;
         
         condition_xml->set_attr_wstr(action->get_condition()->get_name(), L"name");
         action->get_condition()->get_test()->format_entrance(entrance);
         condition_xml->add_element(L"entrance")->set_cdata_str(entrance.str());
         action_xml->set_attr_wstr(action->get_template()->get_type_name(), L"type");
         action->describe_log(*action_xml);
         add_log(log_xml);
         event_action_complete::cpost(this, action);
      } // on_action_complete


      void Alarm::enable_actions(bool enabled)
      {
         actions_enabled = enabled;
         if(!actions_enabled)
            manager->stop_actions_for_alarm(this);
      } // enable_actions


      void Alarm::receive(SharedPtr<Event> &ev)
      {
         if(ev->getType() == event_action_complete::event_id)
         {
            event_action_complete *event(static_cast<event_action_complete *>(ev.get_rep()));
            Manager::action_handle action(manager->find_action(event->action));
            if(action != 0)
            {
               last_action_error = action->get_last_error();
               manager->on_action_complete(event->action);
               if(AlarmClient::is_valid_instance(client))
                  client->on_alarm_change(this);
               else
                  client = 0;
            }
         }
      } // receive


      void Alarm::add_log(Xml::Element &elem)
      {
         elem.set_attr_wstr(name, alarm_name_name);
         manager->add_log(elem);
      } // add_log


      void Alarm::format_value(std::ostream &out)
      {
         if(last_value == 0)
            out << "NAN";
         else
         {
            if(!AlarmClient::is_valid_instance(client) || !client->format_last_value(out, last_value))
               out << last_value->get_val_str();
         }
      } // format_value


      void Alarm::format_value_units(std::ostream &out)
      {
         if(AlarmClient::is_valid_instance(client))
            client->format_last_value_units(out);
      } // format_value_units


      void Alarm::format_value_time(std::ostream &out)
      {
         if(last_value != 0)
            last_value->get_timestamp().format(out, "%c");
      } // format_value_tme


      void Alarm::format_value_type(std::ostream &out)
      {
         if(last_value == 0)
            out << "xsd:double";
         else
         {
            switch(last_value->get_value().type)
            {
            case Expression::value_double:
            case Expression::value_int:
               out << "xsd:double";
               break;
               
            default:
               out << "xsd:string";
               break;
            }
         }
      } // format_value_type
      

      StrUni Alarm::get_triggered_condition_name()
      {
         StrUni rtn;
         if(triggered_condition != 0)
            rtn = triggered_condition->get_name();
         return rtn;
      }


      void Alarm::format_json(Csi::Json::Object &message)
      {
         using namespace Csi::Json;
         Csi::OStrAscStream temp;
         message.set_property_str("name", name.to_utf8(), false);
         message.set_property_str("id", id.to_utf8(), false);
         try
         {
            format_value(temp);
         }
         catch(std::exception &)
         { }
         message.set_property_str("value", temp.str(), false);
         temp.str("");
         try
         {
            format_value_type(temp);
         }
         catch(std::exception &)
         { }
         message.set_property_str("value_type", temp.str(), false);
         switch(get_state())
         {
         case state_on:
            message.set_property_str("state", "on", false);
            break;

         case state_off:
            message.set_property_str("state", "off", false);
            break;

         case state_acked:
            message.set_property_str("state", "acknowledged", false);
            break;
         }
         message.set_property_str("last_error", get_last_error(), false);
         message.set_property_str("triggered_condition_name", get_triggered_condition_name().to_utf8(), false);
         message.set_property_number("actions_pending", get_pending_actions(), false);
         message.set_property_str("last_action_error", get_last_action_error(), false);
      } // format_json

      
      void Alarm::on_alarm_on()
      {
         if(triggered_condition != 0)
         {
            Xml::Element log_event(L"alarm-triggered");
            Xml::Element::value_type condition_xml(log_event.add_element(L"condition"));
            OStrAscStream entrance;
            
            acknowledged = false;
            condition_xml->set_attr_wstr(triggered_condition->get_name(), L"name");
            log_event.add_element(L"source")->set_cdata_wstr(annotate_source_expression());
            triggered_condition->get_test()->format_entrance(entrance);
            condition_xml->add_element(L"entrance")->set_cdata_str(entrance.str());
            add_log(log_event);
            triggered_condition->on_alarm_on();
            if(AlarmClient::is_valid_instance(client))
               client->on_alarm_change(this);
         }
      } // on_alarm_on
      
      
      void Alarm::on_alarm_off()
      {
         if(triggered_condition != 0)
         {
            Xml::Element log_event(L"alarm-off");
            Xml::Element::value_type condition_xml(log_event.add_element(L"condition"));
            OStrAscStream exit;
            
            acknowledged = false;
            log_event.add_element(L"source")->set_cdata_wstr(annotate_source_expression());
            condition_xml->set_attr_wstr(triggered_condition->get_name(), L"name");
            triggered_condition->get_test()->format_exit(exit);
            condition_xml->add_element(L"exit")->set_cdata_str(exit.str());
            add_log(log_event);
            triggered_condition->on_alarm_off();
            triggered_condition.clear();
            if(AlarmClient::is_valid_instance(client))
               client->on_alarm_change(this);
         }
      } // on_alarm_off
   };
};



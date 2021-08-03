/* Csi.Alarms.Condition.cpp

   Copyright (C) 2012, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Monday 24 September 2012
   Last Change: Wednesday 30 November 2016
   Last Commit: $Date: 2018-11-29 13:29:49 -0600 (Thu, 29 Nov 2018) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Alarms.Condition.h"
#include "Csi.Alarms.TestNoData.h"
#include "Csi.Alarms.TestData.h"
#include "Csi.Alarms.Alarm.h"


namespace Csi
{
   namespace Alarms
   {
      StrUni const Condition::condition_name_name(L"name");
      StrUni const Condition::test_name(L"test");
      StrUni const Condition::test_type_name(L"type");
      StrUni const Condition::test_type_no_data(L"no-data");
      StrUni const Condition::test_type_data("data");
      StrUni const Condition::actions_name(L"actions");
      StrUni const Condition::action_name(L"action");
      StrUni const Condition::action_type_name(L"type");

      
      ////////////////////////////////////////////////////////////
      // class Condition definitions
      ////////////////////////////////////////////////////////////
      void Condition::read(Xml::Element &elem)
      {
         using namespace Xml;
         Element::value_type test_xml(elem.find_elem(test_name));
         Element::value_type actions_xml(elem.find_elem(actions_name));
         
         StrUni const test_type(test_xml->get_attr_wstr(test_type_name));
         name = elem.get_attr_wstr(condition_name_name);
         if(test_type == test_type_no_data)
            test.bind(new TestNoData(alarm, alarm->get_timer()));
         else if(test_type == test_type_data)
            test.bind(new TestData(alarm));
         else
            throw std::invalid_argument("invalid test type specified");
         test->read(*test_xml);
         actions.clear();
         for(Element::iterator ai = actions_xml->begin();
             ai != actions_xml->end();
             ++ai)
         {
            Element::value_type &action_xml(*ai);
            action_handle action(
               ActionTemplateBase::make_template(
                  this, action_xml->get_attr_wstr(action_type_name)));
            action->read(*action_xml);
            actions.push_back(action);
         }
      } // read


      void Condition::write(Xml::Element &elem)
      {
         using namespace Xml;
         Element::value_type test_xml(elem.add_element(test_name));
         Element::value_type actions_xml(elem.add_element(actions_name));
         test->write(*test_xml);
         elem.set_attr_wstr(name, condition_name_name);
         for(actions_type::iterator ai = actions.begin();
             ai != actions.end();
             ++ai)
         {
            action_handle &action(*ai);
            Element::value_type action_xml(actions_xml->add_element(action_name));
            action->write(*action_xml);
         }
      } // write


      bool Condition::on_value(operand_handle &value)
      {
         return test->on_value(value);
      } // on_value


      bool Condition::on_record(record_handle const &record)
      {
         return test->on_record(record);
      }


      void Condition::on_alarm_on()
      {
         for(actions_type::iterator ai = actions.begin();
             ai != actions.end();
             ++ai)
         {
            (*ai)->on_alarm_on();
         }
      } // on_alarm_on


      void Condition::on_alarm_off()
      {
         for(actions_type::iterator ai = actions.begin();
             ai != actions.end();
             ++ai)
         {
            (*ai)->on_alarm_off();
         }
      } // on_alarm_on


      Manager *Condition::get_manager()
      { return alarm->get_manager(); }


      void Condition::format_desc(std::ostream &out, StrAsc const &fmt)
      {
         size_t i(0);
         
         while(i < fmt.length())
         {
            char ch(fmt[i]);
            switch(ch)
            {
            case '%':
               if(i + 1 < fmt.length())
               {
                  size_t increment(2);
                  switch(fmt[i + 1])
                  {
                  case '%':
                     out << '%';
                     break;

                  case 'n':
                  case 'N':
                     out << alarm->get_name();
                     break;

                  case 's':
                  case 'S':
                     out << alarm->annotate_source_expression();
                     break;

                  case 'v':
                  case 'V':
                     alarm->format_value(out);
                     break;

                  case 'u':
                  case 'U':
                     alarm->format_value_units(out);
                     break;

                  case 't':
                  case 'T':
                     alarm->format_value_time(out);
                     break;

                  case 'e':
                  case 'E':
                     test->format_entrance(out);
                     break;

                  case 'x':
                  case 'X':
                     test->format_exit(out);
                     break;

                  case 'c':
                  case 'C':
                     out << name;
                     break;

                  default:
                     if(InstanceValidator::is_valid_instance(alarm->get_client()))
                     {
                        increment = alarm->get_client()->expand_format(out, fmt, i);
                        if(increment == 0)
                        {
                           out << ch << fmt[i + 1];
                           increment = 2;
                        }
                     }
                     else
                        out << ch << fmt[i + 1];
                     break;
                  }
                  i += increment;
               }
               else
               {
                  out << ch;
                  ++i;
               }
               break;

            default:
               out << ch;
               ++i;
               break;
            }
         }
      } // format_desc
   };
};

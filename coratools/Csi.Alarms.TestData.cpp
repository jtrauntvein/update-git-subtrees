/* Csi.Alarms.TestData.cpp

   Copyright (C) 2012, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Tuesday 25 September 2012
   Last Change: Thursday 17 November 2016
   Last Commit: $Date: 2016-11-17 14:43:10 -0600 (Thu, 17 Nov 2016) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Alarms.TestData.h"
#include "Csi.Alarms.TestNoData.h"
#include "Csi.Alarms.Alarm.h"


namespace Csi
{
   namespace Alarms
   {
      TestData::TestData(Alarm *alarm_):
         was_triggered(false),
         alarm(alarm_)
      { }


      TestData::~TestData()
      {
         last_off_value.clear();
         last_on_value.clear();
         on_expression.clear();
         off_expression.clear();
      } // destructor


      bool TestData::on_value(operand_handle &value)
      {
         using namespace Expression;
         if(on_expression != 0)
         {
            for(ExpressionHandler::iterator vi = on_expression->begin();
                vi != on_expression->end();
                ++vi)
            {
               ExpressionHandler::value_type &variable(*vi);
               variable.second->set_val(value->get_value(), value->get_timestamp());
            }
            try
            {
               last_on_value = on_expression->eval();
            }
            catch(std::exception &)
            { }
         }
         if(off_expression != 0)
         {
            for(ExpressionHandler::iterator vi = off_expression->begin();
                vi != off_expression->end();
                ++vi)
            {
               ExpressionHandler::value_type &variable(*vi);
               variable.second->set_val(value->get_value(), value->get_timestamp());
            }
            try
            {
               last_off_value = off_expression->eval();
            }
            catch(std::exception &)
            { }
         }
         return is_triggered();
      } // on_value
      
      
      bool TestData::is_triggered()
      {
         bool rtn(false);
         if(last_on_value != 0)
         {
            if(!was_triggered)
            {
               if(last_on_value->get_val_int() != 0)
               {
                  was_triggered = true;
                  rtn = true;
               }
            }
            else
            {
               if(last_off_value != 0)
               {
                  if(last_off_value->get_val_int() != 0)
                     was_triggered = false;
                  else
                     rtn = true;
               }
               else
               {
                  if(last_on_value->get_val_int() == 0)
                     was_triggered = false;
                  else
                     rtn = true;
               }
            }
         }
         return rtn;
      } // is_triggered


      bool TestData::has_on_condition() const
      {
         bool rtn(false);
         if(last_on_value != 0 && last_on_value->get_val_int())
            rtn = true;
         return rtn;
      } // has_on_condition


      StrUni const TestData::on_expression_name(L"on-expr");
      StrUni const TestData::off_expression_name(L"off-expr");
      StrUni const TestNoData::interval_name(L"interval");
      

      void TestData::read(Xml::Element &elem)
      {
         using namespace Xml;
         Element::value_type on_expression_xml(elem.find_elem(on_expression_name));
         Element::iterator off_expression_it(elem.find(off_expression_name));

         on_expression.bind(new expression_type(alarm->get_token_factory().get_rep()));
         on_expression_str = on_expression_xml->get_cdata_wstr();
         on_expression->tokenise(on_expression_str);
         if(off_expression_it != elem.end())
         {
            Element::value_type &off_expression_xml(*off_expression_it);
            off_expression_str = off_expression_xml->get_cdata_wstr();
            if(off_expression_str.length() > 0)
            {
               off_expression.bind(new expression_type(alarm->get_token_factory().get_rep()));
               off_expression->tokenise(off_expression_str); 
            }
            else
               off_expression.clear();
         }
         else
            off_expression.clear();
      } // read


      void TestData::write(Xml::Element &elem)
      {
         using namespace Xml;
         Element::value_type on_expression_xml(elem.add_element(on_expression_name));
         elem.set_attr_wstr(L"data", L"type");
         on_expression_xml->set_cdata_wstr(on_expression_str);
         if(off_expression_str.length() > 0)
         {
            Element::value_type off_expression_xml(elem.add_element(off_expression_name));
            off_expression_xml->set_cdata_wstr(off_expression_str);
         }
      } // write


      void TestData::format_entrance(std::ostream &out)
      {
         StrUni temp(on_expression_str);
         on_expression->annotate_source(temp);
         out << temp;
      } // format_entrance


      void TestData::format_exit(std::ostream &out)
      {
         if(off_expression != 0)
         {
            StrUni temp(off_expression_str);
            off_expression->annotate_source(temp);
            out << temp;
         }
         else
         {
            out << "NOT(";
            format_entrance(out);
            out << ")";
         }
         if(alarm->get_latched())
            out << ", latched";
      } // format_exit


      void TestData::format_value(std::ostream &out)
      {
         if(last_on_value != 0)
         {
            Expression::value_type const &value(last_on_value->get_value());
            switch(value.type)
            {
            case Expression::value_double:
               csiFloatToStream(out, value.vdouble);
               break;
               
            case Expression::value_int:
               out << value.vint;
               break;
               
            case Expression::value_string:
               out << value.vstring;
               break;
               
            case Expression::value_date:
               LgrDate(value.vint).format(out, "%Y-%m-%d %H:%M:%S%x");
               break;
            }
         }
      } // format_value


      void TestData::format_value_time(std::ostream &out)
      {
         if(last_on_value != 0)
         {
            Csi::LgrDate const &timestamp(last_on_value->get_timestamp());
            timestamp.format(out, "%c");
         }
      } // format_value_time


      void TestData::on_started()
      {
         typedef Expression::token_stack_type tokens_type;
         if(on_expression != 0)
         {
            tokens_type &tokens(on_expression->get_postfix_stack());
            for(tokens_type::iterator ti = tokens.begin(); ti != tokens.end(); ++ti)
            {
               tokens_type::value_type &token(*ti);
               if(token->has_state())
                  token->reset_state();
            }
         }
         if(off_expression != 0)
         {
            tokens_type &tokens(off_expression->get_postfix_stack());
            for(tokens_type::iterator ti = tokens.begin(); ti != tokens.end(); ++ti)
            {
               tokens_type::value_type &token(*ti);
               if(token->has_state())
                  token->reset_state();
            }
         }
         last_on_value.clear();
         last_off_value.clear();
      } // on_started
   };
};


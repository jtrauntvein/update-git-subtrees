/* Csi.Alarms.ActionForward.cpp

   Copyright (C) 2012, 2014 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Monday 01 October 2012
   Last Change: Thursday 31 July 2014
   Last Commit: $Date: 2014-07-31 15:21:39 -0600 (Thu, 31 Jul 2014) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Alarms.ActionForward.h"
#include "Csi.Alarms.Manager.h"


namespace Csi
{
   namespace Alarms
   {
      ////////////////////////////////////////////////////////////
      // class ActionForward
      ////////////////////////////////////////////////////////////
      class ActionForward:
         public ActionBase,
         public Cora::DataSources::SinkBase
      {
      private:
         ////////////////////////////////////////////////////////////
         // value
         ////////////////////////////////////////////////////////////
         typedef Expression::value_type value_type;
         value_type value;
         
         ////////////////////////////////////////////////////////////
         // uri
         ////////////////////////////////////////////////////////////
         StrAsc uri;

         ////////////////////////////////////////////////////////////
         // last_error
         ////////////////////////////////////////////////////////////
         StrAsc last_error;

         ////////////////////////////////////////////////////////////
         // complete
         ////////////////////////////////////////////////////////////
         bool complete;
         
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         ActionForward(
            ActionForwardTemplate *action_template_,
            StrAsc const &uri_,
            value_type const &value_):
            ActionBase(action_template_),
            uri(uri_),
            value(value_),
            complete(false)
         { }

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~ActionForward()
         { }

         ////////////////////////////////////////////////////////////
         // execute
         ////////////////////////////////////////////////////////////
         virtual void execute()
         {
            try
            {
               using namespace Cora::DataSources;
               SharedPtr<ValueSetter> setter;
               Manager *manager(
                  action_template->get_condition()->get_alarm()->get_manager());
               sources_type *sources(manager->get_sources().get_rep());
               
               switch(value.type)
               {
               case Expression::value_double:
                  setter.bind(new ValueSetter(value.vdouble));
                  break;
                  
               case Expression::value_int:
               case Expression::value_date:
                  setter.bind(new ValueSetter(static_cast<int4>(value.vint)));
                  break;
                  
               case Expression::value_string:
                  setter.bind(new ValueSetter(value.vstring));
                  break;
               }
               if(!sources->start_set_value(this, uri, *setter))
               {
                  last_error = "invalid destination URI";
                  report_complete();
               }
            }
            catch(std::exception &e)
            {
               last_error = e.what();
               complete = true;
               report_complete();
            }
         }

         ////////////////////////////////////////////////////////////
         // describe_log
         ////////////////////////////////////////////////////////////
         virtual void describe_log(Xml::Element &elem)
         {
            elem.set_attr_str(uri, L"uri");
            switch(value.type)
            {
            case Expression::value_double:
               elem.set_attr_double(value.vdouble, L"value");
               break;
               
            case Expression::value_int:
               elem.set_attr_int8(value.vint, L"value");
               break;
               
            case Expression::value_date:
               elem.set_attr_lgrdate(LgrDate(value.vint), L"value");
               break;

            case Expression::value_string:
               elem.set_attr_str(value.vstring, L"value");
               break;
            }
            if(complete)
            {
               Xml::Element::value_type outcome_xml(elem.add_element(L"outcome"));
               if(last_error.length() != 0)
                  outcome_xml->set_cdata_wstr(L"success");
               else
                  outcome_xml->set_cdata_str(last_error);
            }
         } // describe_log

         ////////////////////////////////////////////////////////////
         // get_last_error
         ////////////////////////////////////////////////////////////
         virtual StrAsc const &get_last_error()
         { return last_error; }

         ////////////////////////////////////////////////////////////
         // on_sink_failure
         ////////////////////////////////////////////////////////////
         typedef Cora::DataSources::Manager sources_type;
         virtual void on_sink_failure(
            sources_type *sources,
            request_handle &request,
            sink_failure_type failure)
         { }

         ////////////////////////////////////////////////////////////
         // on_sink_records
         ////////////////////////////////////////////////////////////
         virtual void on_sink_records(
            sources_type *sources,
            requests_type &requests,
            records_type const &records)
         { }

         ////////////////////////////////////////////////////////////
         // on_set_complete
         ////////////////////////////////////////////////////////////
         virtual void on_set_complete(
            sources_type *sources,
            StrUni const &uri,
            set_outcome_type outcome)
         {
            if(outcome != set_outcome_succeeded)
            {
               OStrAscStream temp;
               format_set_outcome(temp, outcome);
               last_error = temp.str();
            }
            else
            {
               // since LoggerNet polls the table after setting the variable, we will need to ignore
               // the next record from that table.
               action_template->get_condition()->get_alarm()->ignore_next_record(uri);
               last_error.cut(0);
            }
            complete = true;
            report_complete();
         }
      };
         
      ////////////////////////////////////////////////////////////
      // class ActionForwardTemplate definitions
      ////////////////////////////////////////////////////////////
      ActionForwardTemplate::ActionForwardTemplate(Condition *condition):
         ActionTemplateBase(condition)
      { }


      ActionForwardTemplate::~ActionForwardTemplate()
      { }


      namespace
      {
         StrUni const forward_expression_name(L"forward-expression");
         StrUni const dest_uri_name(L"dest-uri");
      };
      

      void ActionForwardTemplate::read(Xml::Element &elem)
      {
         ActionTemplateBase::read(elem);
         forward_expression_str = elem.find_elem(forward_expression_name)->get_cdata_str();
         dest_uri = elem.find_elem(dest_uri_name)->get_cdata_str();
         forward_expression.bind(
            new Expression::ExpressionHandler(
               condition->get_alarm()->get_token_factory().get_rep()));
         forward_expression->tokenize(forward_expression_str.c_str());
      } // read


      void ActionForwardTemplate::write(Xml::Element &elem)
      {
         ActionTemplateBase::write(elem);
         elem.add_element(forward_expression_name)->set_cdata_str(forward_expression_str);
         elem.add_element(dest_uri_name)->set_cdata_str(dest_uri);
      } // write


      void ActionForwardTemplate::perform_action()
      {
         try
         {
            // we will need to evaluate the forward expression before we generate the value setter.
            using namespace Expression;
            Alarm::operand_handle const &value(condition->get_alarm()->get_last_value());
            Alarm::operand_handle forward_value;
            
            for(ExpressionHandler::iterator vi = forward_expression->begin();
                vi != forward_expression->end();
                ++vi)
            {
               vi->second->set_val(value->get_value(), value->get_timestamp());
            }
            forward_value = forward_expression->eval();
            condition->get_alarm()->add_action(
               new ActionForward(this, dest_uri, forward_value->get_value()));
         }
         catch(std::exception &)
         { }
      } // perform_action
   };
};


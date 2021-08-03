/* Csi.Expression.JsGenerator.cpp

   Copyright (C) 2010, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 28 July 2010
   Last Change: Friday 29 April 2016
   Last Commit: $Date: 2020-07-14 15:47:08 -0600 (Tue, 14 Jul 2020) $
   Last Changed by: $Author: jbritt $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Expression.JsGenerator.h"
#include "Csi.Json.h"


namespace Csi
{
   namespace Expression
   {
      ////////////////////////////////////////////////////////////
      // class JsGenerator definitions
      ////////////////////////////////////////////////////////////
      JsGenerator::JsGenerator(
         sources_handle &sources_, resources_handle &resources_):
         sources(sources_),
         resources(resources_),
         resource_generator(resources_.get_rep())
      { }


      JsGenerator::~JsGenerator()
      {
         sources.clear();
         resources.clear();
         expressions.clear();
         variables.clear();
      } // destructor


      namespace
      {
         ////////////////////////////////////////////////////////////
         // predicate query_is_compatible
         ////////////////////////////////////////////////////////////
         struct query_is_compatible
         {
            typedef SharedPtr<Cora::DataSources::Request> request_handle;
            request_handle &request;
            query_is_compatible(request_handle &request_):
               request(request_)
            { }

            bool operator ()(JsGenerator::value_type &query)
            {
               bool rtn = false;
               request_handle &query_request(query->get_request());
               if(query_request->get_uri() == request->get_uri() &&
                  query_request->is_compatible(*request) &&
                  query_request->get_js_name() == request->get_js_name())
                  rtn = true;
               return rtn;
            }
         };
      };
      

      void JsGenerator::add_expression(expression_handle &expression)
      {
         // we need to add the expression and then add all of the variables associated with that
         // expression.
         expression->set_js_index((uint4)expressions.size());
         expressions.push_back(expression);
         for(ExpressionHandler::iterator vi = expression->begin(); vi != expression->end(); ++vi)
         {
            // add the variable
            variable_handle var = vi->second;
            var->set_js_index((uint4)variables.size());
            variables.push_back(vi->second);

            // we also need to create or maintain a web query to service this variable.
            query_type::request_handle request(var->get_request());
            if(request != 0)
            {
               StrUni table_uri(sources->make_table_uri(request->get_uri()));
               table_uri.replace(L"\\", L"\\\\");
               if(table_uri.length() > 0)
               {
                  // we will form a request for this table using the variable request parameters.
                  typedef Cora::DataSources::Request request_type;
                  query_type::request_handle table_request(new request_type(0, table_uri));
                  table_request->copy_start_options(*request);

                  // we must now search for a web query that is compatible with this request
                  queries_type::iterator qi = std::find_if(
                     queries.begin(), queries.end(), query_is_compatible(table_request));
                  if(qi != queries.end())
                  {
                     value_type &query(*qi);
                     query->add_variable(var);
                  }
                  else
                  {
                     value_type query(new query_type(table_request));
                     query->add_variable(var);
                     queries.push_back(query);
                  }
               }
            }
         }
      } // add_expression


      namespace
      {
         ////////////////////////////////////////////////////////////
         // functor generate_token_code
         ////////////////////////////////////////////////////////////
         struct generate_token_code
         {
            std::ostream &out;
            ResourceGenerator &resources;
            StrAsc resource_name;
            generate_token_code(std::ostream &out_, ResourceGenerator &resources_):
               resources(resources_),
               out(out_)
            { }

            void operator ()(token_stack_type::value_type &token)
            {
               resource_name = token->get_js_resource();
               if(resource_name.length() > 0)
                  resources.generate(out, resource_name);
            }
         };

         
         ////////////////////////////////////////////////////////////
         // functor generate_expression_code
         ////////////////////////////////////////////////////////////
         struct generate_expression_code
         {
            ResourceGenerator &resources;
            std::ostream &out;
            generate_expression_code(std::ostream &out_, ResourceGenerator &resources_):
               resources(resources_),
               out(out_)
            { }

            void operator ()(JsGenerator::expression_handle &expression)
            {
               std::for_each(
                  expression->get_postfix_stack().begin(),
                  expression->get_postfix_stack().end(),
                  generate_token_code(out, resources));
            }
         };


         ////////////////////////////////////////////////////////////
         // functor generate_token
         ////////////////////////////////////////////////////////////
         struct generate_token
         {
            std::ostream &out;
            int count;
            int indent_level;
            generate_token(std::ostream &out_, int indent_level_):
               out(out_),
               count(0),
               indent_level(indent_level_)
            { }

            void operator ()(token_stack_type::value_type &token)
            {
               if(count++ != 0)
                  out << ",";
               out << "\n";
               for(int i = 0; i < indent_level; ++i)
                  out << "  ";
               token->format_js(out);
            }
         };
         
         
         ////////////////////////////////////////////////////////////
         // functor generate_expression
         ////////////////////////////////////////////////////////////
         struct generate_expression
         {
            std::ostream &out;
            int count;
            int indent_level;
            generate_expression(std::ostream &out_, int indent_level_):
               out(out_),
               count(0),
               indent_level(indent_level_)
            { }

            void operator ()(SharedPtr<ExpressionHandler> &expression)
            {
               token_stack_type &tokens(expression->get_postfix_stack()); 
               if(count++ != 0)
                  out << ",\n";
               else
                  out << "\n";
               for(int i = 0; i < indent_level; ++i)
                  out << "  ";
               out << "new CsiExpression([";
               std::for_each(
                  tokens.begin(), tokens.end(), generate_token(out, indent_level + 1));
               out << "\n";
               for(int i = 0; i < indent_level; ++i)
                  out << "  ";
               out << "])";
            }
         };


         struct generate_queries
         {
            std::ostream &out;
            int count;
            int indent_level;
            generate_queries(std::ostream &out_, int indent_level_):
               out(out_),
               count(0),
               indent_level(indent_level_)
            { }

            void operator ()(JsGenerator::value_type &query)
            {
               if(count++ != 0)
                  out << ",\n";
               for(int i = 0; i < indent_level; ++i)
                  out << "  ";
               query->generate(out);
            }
         };
      };


      void JsGenerator::generate_dependencies(std::ostream &out)
      {
         resource_generator.generate(out, "CsiDataManager.js");
         std::for_each(
            expressions.begin(),
            expressions.end(),
            generate_expression_code(out, resource_generator));
      } // generate_dependencies


      void JsGenerator::generate_other_dependencies(std::ostream &out, StrAsc const &key)
      { resource_generator.generate(out, key); }

      
      bool JsGenerator::has_dependency(StrAsc const &key)
      {
         return resource_generator.is_key_used(key);
      } // has_dependency


      namespace
      {
         ////////////////////////////////////////////////////////////
         // predicate real_time_queries_first
         ////////////////////////////////////////////////////////////
         struct real_time_queries_first
         {
            typedef JsGenerator::value_type value_type;
            typedef Cora::DataSources::Request request_type;
            typedef JsGeneratorHelpers::WebQuery WebQuery;
            bool operator ()(value_type &q1, value_type &q2)
            {
               bool rtn(false);
               WebQuery::request_handle &r1(q1->get_request());
               WebQuery::request_handle &r2(q2->get_request());
               request_type::start_option_type r1_start(r1->get_start_option());
               request_type::start_option_type r2_start(r2->get_start_option());
               request_type::order_option_type r1_order(r1->get_order_option());
               request_type::order_option_type r2_order(r2->get_order_option());
               bool r1_at_newest(
                  r1_start == request_type::start_at_newest ||
                  r1_start == request_type::start_after_newest ||
                  r1_order == request_type::order_real_time);
               bool r2_at_newest(
                  r2_start == request_type::start_at_newest ||
                  r2_start == request_type::start_after_newest ||
                  r2_order == request_type::order_real_time);
               
               if(r1_at_newest && !r2_at_newest)
                  rtn = true;

               else if (r1_at_newest == r2_at_newest && r1_start == r2_start)
               {
                  if(r1_start == request_type::start_relative_to_newest)
                  {
                     rtn = (r1->get_backfill_interval() > r2->get_backfill_interval());
                  }
                  else if(r1_start == request_type::start_at_offset_from_newest)
                  {
                     rtn = (r1->get_start_record_offset() < r2->get_start_record_offset());
                  }
               }
               return rtn;
            }
         };
      };


      void JsGenerator::generate(
         std::ostream &out,
         int indent_level,
         uint4 alarms_poll_interval,
         bool use_web_sockets)
      {
         // generate the variables section
         for(int i = 0; i < indent_level; ++i)
            out << "  ";
         out << "var variables = [";
         for(variables_type::iterator vi = variables.begin(); vi != variables.end(); ++vi)
         {
            // we need to extract the variable name from the request uri
            using namespace Cora::DataSources;
            variables_type::value_type &variable(*vi);

            if(vi != variables.begin())
               out << ",";
            out << "\n";
            for(int i = 0; i < indent_level + 1; ++i)
               out << "  ";
            variable->generate_constructor_js(sources, out);
         }
         out << "\n";
         for(int i = 0; i < indent_level; ++i)
            out << "  ";
         out << "];\n\n";

         // generate the expressions section
         for(int i = 0; i < indent_level; ++i)
            out << "  ";
         out << "var expressions = [";
         std::for_each(expressions.begin(), expressions.end(), generate_expression(out, indent_level + 1));
         out << "\n";
         for(int i = 0; i < indent_level; ++i)
            out << "  ";
         out << "];\n\n";

         // generate the web queries
         queries.sort(real_time_queries_first());
         for(int i = 0; i < indent_level; ++i)
            out << "  ";
         out << "var web_queries = [\n";
         std::for_each(queries.begin(), queries.end(), generate_queries(out, indent_level + 1));
         out << "\n";
         for(int i = 0; i < indent_level; ++i)
            out << "  ";
         out << "];\n\n";

         // generate the alarms manager
         if(alarms_poll_interval > 0)
         {
            for(int i = 0; i < indent_level; ++i)
               out << "  ";
            out << "theAlarmsManager = new CsiAlarmsManager(" << alarms_poll_interval << ");\n";
         }

         // generate the data manager
         for(int i = 0; i < indent_level; ++i)
            out << "  ";
         out << "dataManager = new CsiDataManager(web_queries, "
             << (use_web_sockets ? "true" : "false") << ");\n";
      } // generate


      namespace JsGeneratorHelpers
      {
         ////////////////////////////////////////////////////////////
         // class WebQuery definitions
         ////////////////////////////////////////////////////////////
         WebQuery::WebQuery(request_handle request_):
            request(request_),
            refresh_interval(10000),
            override_interval(0xFFFFFFFF),
            override_at_refresh(false)
         { }


         WebQuery::~WebQuery()
         { }


         void WebQuery::generate(std::ostream &out)
         {
            using Cora::DataSources::Request;
            out << "new CsiWebQuery(\"" << request->get_uri() << "\", ";
            switch(request->get_start_option())
            {
            case Request::start_at_record:
               out << "\"since-record\", \"" << request->get_record_no() << "\",\"\", ";
               break;
               
            case Request::start_at_time:
               out << "\"since-time\", \"";
               request->get_start_time().format(out, "%Y-%m-%dT%H:%M:%S%x");
               out << "\", \"\", ";
               break;
               
            case Request::start_date_query:
               out << "\"date-range\", \"";
               request->get_start_time().format(out, "%Y-%m-%dT%H:%M:%S%x");
               out << "\", \"";
               request->get_end_time().format(out, "%Y-%m-%dT%H:%M:%S%x");
               out << "\", ";
               break;
               
            case Request::start_relative_to_newest:
               out << "\"backfill\", \"" << (-request->get_backfill_interval() / LgrDate::nsecPerSec)
                   << "\", \"\", ";
               break;
               
            case Request::start_at_offset_from_newest:
               out << "\"most-recent\", \"" << request->get_start_record_offset()
                   << "\", \"\", ";
               break;
               
            default:
               out << "\"most-recent\", \"1\", \"\", ";
               break;
            }

            // output the order option
            switch(request->get_order_option())
            {
            case Request::order_logged_with_holes:
               out << "\"logged-with-holes\", ";
               break;
               
            case Request::order_logged_without_holes:
               out << "\"logged-without-holes\", ";
               break;
               
            case Request::order_real_time:
               out << "\"real-time\", ";
               break;
               
            default:
               out << "\"collected\", ";
               break;
            }
            out << refresh_interval << ", ";
            if(override_interval != 0xFFFFFFFF)
               out << override_interval << ", [";
            else if(override_at_refresh)
               out << refresh_interval << ", [";
            else
               out << "-1, [";
            for(variables_type::iterator vi = variables.begin(); vi != variables.end(); ++vi)
            {
               variables_type::value_type &var(*vi);
               if(vi != variables.begin())
                  out << ", ";
               out << "variables[" << var->get_js_index() << "]";
            }
            out << " ], ";
            out << "\"" << request->get_js_name() << "\","
                << (request->get_report_offset() / LgrDate::nsecPerMSec) << ")";
         } // generate
      };
   };
};


/* Csi.Expression.JsGenerator.h

   Copyright (C) 2010, 2014 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 28 July 2010
   Last Change: Wednesday 17 December 2014
   Last Commit: $Date: 2014-12-18 12:23:14 -0600 (Thu, 18 Dec 2014) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Csi_Expression_JsGenerator_h
#define Csi_Expression_JsGenerator_h

#include "Csi.ResourceManager.h"
#include "Csi.Expression.ExpressionHandler.h"
#include "Cora.DataSources.Manager.h"
#include "Csi.SharedPtr.h"


namespace Csi
{
   namespace Expression
   {
      namespace JsGeneratorHelpers
      {
         ////////////////////////////////////////////////////////////
         // class WebQuery
         ////////////////////////////////////////////////////////////
         class WebQuery
         {
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            typedef Cora::DataSources::Request request_type;
            typedef SharedPtr<request_type> request_handle;
            WebQuery(request_handle request_); 

            ////////////////////////////////////////////////////////////
            // destructor
            ////////////////////////////////////////////////////////////
            ~WebQuery();

            ////////////////////////////////////////////////////////////
            // add_variable
            ////////////////////////////////////////////////////////////
            typedef LightPolySharedPtr<Token, Variable> variable_handle;
            void add_variable(variable_handle &variable)
            { variables.push_back(variable); }

            ////////////////////////////////////////////////////////////
            // generate
            ////////////////////////////////////////////////////////////
            void generate(std::ostream &out);

            ////////////////////////////////////////////////////////////
            // get_request
            ////////////////////////////////////////////////////////////
            request_handle &get_request()
            { return request; }

            ////////////////////////////////////////////////////////////
            // set_refresh_interval
            ////////////////////////////////////////////////////////////
            void set_refresh_interval(uint4 refresh_interval_)
            { refresh_interval = refresh_interval_; }

            ////////////////////////////////////////////////////////////
            // get_refresh_interval
            ////////////////////////////////////////////////////////////
            uint4 get_refresh_interval() const
            { return refresh_interval; }

            ////////////////////////////////////////////////////////////
            // set_override_interval
            ////////////////////////////////////////////////////////////
            void set_override_interval(uint4 override_interval_)
            { override_interval = override_interval_; }

            ////////////////////////////////////////////////////////////
            // get_override_interval
            ////////////////////////////////////////////////////////////
            uint4 get_override_interval() const
            { return override_interval; }

            ////////////////////////////////////////////////////////////
            // get_override_at_refresh
            ////////////////////////////////////////////////////////////
            bool get_override_at_refresh() const
            { return override_at_refresh; }

            ////////////////////////////////////////////////////////////
            // set_override_at_refresh
            ////////////////////////////////////////////////////////////
            void set_override_at_refresh(bool override)
            { override_at_refresh = override; }

         private:
            ////////////////////////////////////////////////////////////
            // request
            ////////////////////////////////////////////////////////////
            request_handle request;

            ////////////////////////////////////////////////////////////
            // variables
            ////////////////////////////////////////////////////////////
            typedef std::list<variable_handle> variables_type;
            variables_type variables;

            ////////////////////////////////////////////////////////////
            // refresh_interval
            ////////////////////////////////////////////////////////////
            uint4 refresh_interval;

            ////////////////////////////////////////////////////////////
            // override_interval
            ////////////////////////////////////////////////////////////
            uint4 override_interval;

            ////////////////////////////////////////////////////////////
            // override_at_refresh
            ////////////////////////////////////////////////////////////
            bool override_at_refresh;
         };
      };

      
      ////////////////////////////////////////////////////////////
      // class JsGenerator
      //
      // Defines an object that, given a data source manager, a resource
      // manager, and a collection of expression handlers, will be able to
      // generate the JavaScript code that will represent these objects.  
      ////////////////////////////////////////////////////////////
      class JsGenerator
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         typedef SharedPtr<ResourceManager> resources_handle;
         typedef SharedPtr<Cora::DataSources::Manager> sources_handle;
         JsGenerator(
            sources_handle &sources_,
            resources_handle &resources_);

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~JsGenerator();

         ////////////////////////////////////////////////////////////
         // add_expression
         ////////////////////////////////////////////////////////////
         typedef SharedPtr<ExpressionHandler> expression_handle;
         void add_expression(expression_handle &expression);

         ////////////////////////////////////////////////////////////
         // generate_dependencies
         //
         // Generates the dependencies of all of the registered expressions. 
         ////////////////////////////////////////////////////////////
         void generate_dependencies(std::ostream &out);
         
         ////////////////////////////////////////////////////////////
         // generate_other_dependencies
         //
         // Generates the dependencies associated with the specified key.
         ////////////////////////////////////////////////////////////
         void generate_other_dependencies(std::ostream &out, StrAsc const &key);

         ///////////////////////////////////////////////////////////
         // has_dependency
         //
         // Checks to see if the dependency specified by the key exists
         ///////////////////////////////////////////////////////////
         bool has_dependency(StrAsc const &key);
            
         ////////////////////////////////////////////////////////////
         // generate
         //
         // Generates the variables, expressions, and web queries based upon
         // the current set of expressions.
         ////////////////////////////////////////////////////////////
         void generate(
            std::ostream &out,
            int indent_level = 1,
            uint4 alarms_poll_interval = 10000,
            bool use_web_sockets = false);
         
         // @group: web queries container definitions
         
         ////////////////////////////////////////////////////////////
         // begin
         ////////////////////////////////////////////////////////////
         typedef JsGeneratorHelpers::WebQuery query_type;
         typedef SharedPtr<query_type> value_type;
         typedef std::list<value_type> queries_type;
         typedef queries_type::iterator iterator;
         typedef queries_type::const_iterator const_iterator;
         iterator begin()
         { return queries.begin(); }
         const_iterator begin() const
         { return queries.begin(); }
         
         ////////////////////////////////////////////////////////////
         // end
         ////////////////////////////////////////////////////////////
         iterator end()
         { return queries.end(); }
         const_iterator end() const
         { return queries.end(); }
         
         ////////////////////////////////////////////////////////////
         // size
         ////////////////////////////////////////////////////////////
         typedef queries_type::size_type size_type;
         size_type size() const
         { return queries.size(); }
         
         ////////////////////////////////////////////////////////////
         // empty
         ////////////////////////////////////////////////////////////
         bool empty() const
         { return queries.empty(); }
         
         // @endgroup
         
      private:
         ////////////////////////////////////////////////////////////
         // sources
         ////////////////////////////////////////////////////////////
         sources_handle sources;

         ////////////////////////////////////////////////////////////
         // resources
         ////////////////////////////////////////////////////////////
         resources_handle resources;

         ////////////////////////////////////////////////////////////
         // expressions
         ////////////////////////////////////////////////////////////
         typedef std::list<expression_handle> expressions_type;
         expressions_type expressions;

         ////////////////////////////////////////////////////////////
         // variables
         ////////////////////////////////////////////////////////////
         typedef ExpressionHandler::variable_handle variable_handle;
         typedef std::list<variable_handle> variables_type;
         variables_type variables;

         ////////////////////////////////////////////////////////////
         // queries
         ////////////////////////////////////////////////////////////
         queries_type queries;

         ////////////////////////////////////////////////////////////
         // resource_generator
         ////////////////////////////////////////////////////////////
         ResourceGenerator resource_generator;
      };
   };
};


#endif

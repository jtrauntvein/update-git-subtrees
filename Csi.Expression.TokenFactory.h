/* Csi.Expression.TokenFactory.h

   Copyright (C) 2002, 2020 Campbell Scientific, Inc.

   Written by: Tyler Mecham
   Date Begun: 9 january 2002
   Last Change: Wednesday 15 April 2020
   Last Commit: $Date: 2020-04-15 11:51:44 -0600 (Wed, 15 Apr 2020) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Csi_Expression_TokenFactory_h
#define Csi_Expression_TokenFactory_h

#include "Csi.Expression.Token.h"
#include "Cora.DataSources.Request.h"
#include <map>


namespace Csi
{
   namespace Expression
   {
      /**
       * Defines an object that is able to generate expression tokens given a string that represents
       * that token.
       */
      class TokenFactory
      {
      public:
         /**
          * Constructor
          *
          * @param variables_quoted_ Set to true if variable names are allowed to be quoted.
          */
         TokenFactory(bool variables_quoted_ = true);
         
         /**
          * Destructor
          */
         virtual ~TokenFactory();

         /**
          * Defines the type of exception that is thrown when a token string is not supported by
          * this factor.
          */
         class ExcUnsupportedTokenType: public std::exception
         {
         public:
            char const *what() const throw ()
            { return "Csi::Expression::TokenFactory: Unsupported token type specified"; }
         };

         /**
          * @return Returns the token associated with the specified token string.
          *
          * @param prev_token Specifies the last token generated (this is needed to distinguish
          * between unary and binary operators).
          *
          * @param token_string Specifies the string for the token.
          *
          * @param token_start Specifies the offset for the token string in the expression.
          */
         virtual token_handle make_token(
            token_handle &prev_token,
            StrUni const &token_string,
            size_t token_start = 0);

         /**
          * Allows the application to register a function for the specified key.
          * 
          * @param key Specifies the name of the token.
          *
          * @param func Specifies the associated function.
          */
         void register_function(StrUni const &key, token_handle func);

         /**
          * @return Returns an expression object that resolves all variables as data source
          * variables.
          *
          * @param sink Specifies the application object that will receive data source
          * notifications.
          *
          * @param expression_string Specifies the source for the expression.
          *
          * @param requests Specifies the collection where generated requests will be added.
          *
          * @param default_source_name Specifies a data source name if the expression does not
          * include the source name in variables.
          *
          * @param factory Specifies the token factory.
          *
          * @param default_order_option Specifies the order option that should be used.
          */
         typedef Csi::SharedPtr<Csi::Expression::ExpressionHandler> expression_handle;
         typedef Csi::SharedPtr<Cora::DataSources::Request> request_handle;
         typedef std::list<request_handle> requests_type;
         static expression_handle make_expression(
            Cora::DataSources::SinkBase *sink,
            StrUni &expression_string,
            requests_type &requests,
            StrUni const default_source_name = L"",
            TokenFactory *factory = 0,
            Cora::DataSources::Request::order_option_type default_order_option = Cora::DataSources::Request::order_real_time);

         /**
          * @return Returns the iterator to the first function.
          */
         typedef std::map<StrUni, token_handle> functions_type;
         typedef functions_type::iterator iterator;
         typedef functions_type::const_iterator const_iterator;
         iterator begin()
         { return functions.begin(); }
         const_iterator begin() const
         { return functions.begin(); }

         /**
          * @return Returns the iterator beyond the end of the last function.
          */
         iterator end()
         { return functions.end(); }
         const_iterator end() const
         { return functions.end(); }

         /**
          * @return Returns the number of functions available.
          */
         typedef functions_type::size_type size_type;
         size_type size() const
         { return functions.size(); }

         /**
          * @return Returns true if there are no functions available.
          */
         bool empty() const
         { return functions.empty(); }
         
      private:
         /**
          * @return Returns true if the specified string represents a number.
          *
          * @param val Specifies the token to evaluate.
          */
         bool is_number(StrUni const &val);

         /**
          * Specifies the collection of functions that are available.
          */
         functions_type functions;

         /**
          * Set to true if variable names are allowed to be quoted.  This controls how quoted tokens
          * are interpreted.  If false, quoted tokens are assumed to be strings.
          */
         bool variables_quoted;
      };
   };
};

#endif

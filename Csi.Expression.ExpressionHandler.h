/* Csi.Expression.ExpressionHandler.h

   Copyright (C) 2002, 2020 Campbell Scientific, Inc.

   Written by: Tyler Mecham
   Date Begun: 8 January 2002 
   Last Change: Wednesday 15 April 2020
   Last Commit: $Date: 2020-04-15 11:51:44 -0600 (Wed, 15 Apr 2020) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Csi_Expression_ExpressionHandler_h
#define Csi_Expression_ExpressionHandler_h

#include <list>
#include <map>
#include "Csi.Expression.TokenFactory.h"
#include "Csi.Expression.Token.h"
#include "Csi.SharedPtr.h"
#include "Csi.PolySharedPtr.h"
#include "Csi.LightSharedPtr.h"


namespace Csi
{
   namespace Expression
   {
      /**
       * Declares the type of exception thrown when the expression  handler does not have an
       * expression to tokenise or if no expression has been tokenised when eval() has been called.
       */
      class ExcInvalidState: public std::exception
      {
      public:
         char const *what() const throw ()
         { return "Csi::Expression::ExpressionHandler: Invalid state"; }
      };


      /**
       * Defines a class of expression that is thrown while attemping to parse an invalid
       * expression.
       */
      class ExcParseError: public std::exception
      {
      public:
         /**
          * Specifies the offset into the expression source where the error was found.
          */
         size_t const offset;

         /**
          * Specifies the reason for the error.
          */
         StrAsc const reason;

         /**
          * Constructor
          *
          * @param offset_ Specifies the offset where the error was found.
          *
          * @param reason_ Specifies the reason for the exception.
          */
         ExcParseError(size_t offset_, StrAsc const &reason_):
            offset(offset_),
            reason(reason_)
         { }

         /**
          * Copy Constructor
          */
         ExcParseError(ExcParseError const &other):
            offset(other.offset),
            reason(other.reason)
         { }

         /**
          * Destructor
          */
         virtual ~ExcParseError() throw ()
         { }

         /**
          * @return Overrides the base class version to return the reason for the failure.
          */
         virtual char const *what() const throw() override
         { return reason.c_str(); }
      };


      /**
       * Defines an object that is able to parse a CRBasic like expression and to evaluate that
       * expression based upon application supplied variable values.
       */
      class ExpressionHandler
      {
      public:
         /**
          * Constructor
          *
          * @param token_factory_ Specifies the object that will create tokens for this expression.
          */
         ExpressionHandler(TokenFactory *token_factory_ = 0);

         /**
          * Destructor
          */
         virtual ~ExpressionHandler();

         /**
          * Parses an expression string to produce the token stack that this handler will use to
          * evaluate the expression.
          *
          * @param expression Specifies the expression string.
          */
         void tokenise(StrUni const &expression);
         void tokenise(char const *expression)
         { tokenise(StrUni(expression != 0 ? expression : "", true)); }
         void tokenize(char const *expression)
         { tokenise(expression); }
         void tokenize(StrUni const &expression)
         { tokenise(expression); }

         /**
          * Parses an expression string into a collection of token source strings and their
          * positions within the source.
          *
          * @param tokens Specifies the container into which the token source strings will be
          * written.
          *
          * @param expression Specifies the expression source.
          */
         typedef std::pair<StrUni, size_t> string_token_type;
         typedef std::list<string_token_type> string_tokens_type;
         void make_string_tokens(string_tokens_type &tokens, StrUni const &expression);

         /**
          * Converts a list of string token sources into a list of token objects along with their
          * associated position.
          *
          * @param tokens Specifies the list of tokens that were parsed.
          *
          * @param string_tokens Specifies the list of token source strings that were parsed.
          *
          * @param expression Specifies the string source for the expression.
          */
         typedef LightSharedPtr<Token> token_handle;
         struct parsed_token_type
         {
            token_handle token;
            size_t begin_pos;
            size_t length;
            
            parsed_token_type():
               begin_pos(0),
               length(0)
            { }
            
            parsed_token_type(
               token_handle &token_,
               string_token_type const &string_token):
               token(token_),
               begin_pos(string_token.second),
               length(string_token.first.length())
            { }

            parsed_token_type(parsed_token_type const &other):
               token(other.token),
               begin_pos(other.begin_pos),
               length(other.length)
            { }

            parsed_token_type &operator =(parsed_token_type const &other)
            {
               token = other.token;
               begin_pos = other.begin_pos;
               length = other.length;
               return *this;
            }
         };
         typedef std::list<parsed_token_type> parsed_tokens_type;
         void make_tokens(parsed_tokens_type &tokens, string_tokens_type const &string_tokens);
         void make_tokens(parsed_tokens_type &tokens, StrUni const &expression)
         {
            string_tokens_type string_tokens;
            make_string_tokens(string_tokens, expression);
            make_tokens(tokens, string_tokens);
         }

         /**
          * @return Evaluates the expression and returns the operand at the bottom of the stack
          * following that evaluation.
          */
         typedef LightPolySharedPtr<Token, Operand> operand_handle;
         operand_handle eval();

         /**
          * @return Evaluates the expression and returns the last operand as a double value.
          */
         double evaluate()
         {
            operand_handle last_op(eval());
            return last_op->get_val();
         }

         /**
          * @return Returns the token factory.
          */
         TokenFactory *get_token_factory()
         { return token_factory; }

         /**
          * @return Returns the number of tokens in the post-fix stack.
          */
         token_stack_type::size_type get_stack_size() const
         { return postfix_stack.size(); }

         /**
          * @return Returns the postfix stack container reference.
          */
         token_stack_type &get_postfix_stack()
         { return postfix_stack; }

         /**
          * Formats the postfiox stack to the specified stream.
          */
         void format_postfix(std::ostream &out) const;

         /**
          * @return Returns the iterator at the beginning of the variables list.
          */
         typedef LightPolySharedPtr<Token, Variable> variable_handle;
         typedef std::map<StrUni, variable_handle > variables_type;
         typedef variables_type::value_type value_type;
         typedef variables_type::iterator iterator;
         typedef variables_type::const_iterator const_iterator;
         iterator begin() { return variables.begin(); }
         const_iterator begin() const { return variables.begin(); }

         /**
          * @return Returns the iterator beyond the end of the variables list.
          */
         iterator end() { return variables.end(); }
         const_iterator end() const { return variables.end(); }

         /**
          * @return Returns the reverse iterator at the end of the list.
          */
         typedef variables_type::reverse_iterator reverse_iterator;
         typedef variables_type::const_reverse_iterator const_reverse_iterator;
         reverse_iterator rbegin() { return variables.rbegin(); }
         const_reverse_iterator rbegin() const { return variables.rbegin(); }

         /**
          * @return Returns the reverse iterator beyond the beginning of the variables list.
          */
         const_reverse_iterator rend() const { return variables.rend(); }
         reverse_iterator rend() { return variables.rend(); }

         /**
          * @return Returns the iterator associated with the specified variable name.
          *
          * @param key Specifies the variable name.
          */
         iterator find(StrUni const &key)
         { return variables.find(key); }
         const_iterator find(StrUni const &key) const
         { return variables.find(key); }

         /**
          * @return Returns the number of variables in the expression.
          */
         variables_type::size_type size() const { return variables.size(); }

         /**
          * @return Returns true if there are no variablkes in the expression.
          */
         bool empty() const { return variables.empty(); }

         /**
          * Removes the specified variable name from the expression.
          *
          * @param var_name Specifies the variable to erase.
          */
         void erase(StrUni const &var_name)
         { variables.erase(var_name); }
         
         /**
          * @return Returns true if the expression has tokens that would abort execution.
          */
         bool has_aborting_tokens();

         /**
          * @return Returns the javascript index that was assigned to this expression when it was
          * added to the generator.
          */
         int get_js_index() const
         { return js_index; }

         /**
          * @param index Specifies the javascript index assigned to this expression by the
          * generator.
          */
         void set_js_index(int index)
         { js_index = index; }

         /**
          * Specifies that any data source requests generated for variables in this expression
          * should start at a given record number and file mark.
          *
          * @param file_mark_no_ Specifies the begin file mark.
          *
          * @param recod_no_ Specifies the begin record number.
          */
         typedef Cora::DataSources::Request request_type;
         typedef request_type::start_option_type start_option_type;
         void set_start_at_record(uint4 file_mark_no_, uint4 record_no_)
         {
            start_option = request_type::start_at_record;
            file_mark_no = file_mark_no_;
            record_no = record_no_;
            start_option_set = true;
         }

         /**
          * Specifies that any data source requests generated from variables in this expression
          * should start at a specified date and time.
          *
          * @param start_time_ Specifies the start time.
          */
         void set_start_at_time(LgrDate const &start_time_)
         {
            start_option = request_type::start_at_time;
            start_time = start_time_;
            start_option_set = true;
         }

         /**
          * Specifies that any data source requests generated from variables in this expression
          * should start at the newest record.
          */
         void set_start_at_newest()
         {
            start_option = request_type::start_at_newest;
            start_option_set = true;
         }

         /**
          * Specifies that any data source requests generated from variabkes in this expression
          * should after the newest record.
          */
         void set_start_after_newest()
         {
            start_option = request_type::start_after_newest;
            start_option_set = true;
         }

         /**
          * Specifies that any data source requests generated from variables in this expression
          * should start at a time relative to the newest time stamp.
          *
          * @param backfill_interval_ Specifies the postive interval, in nanoseconds, that should be
          * subtracted from the newest record time stamp.
          */
         void set_start_relative_to_newest(int8 backfill_interval_)
         {
            start_option = request_type::start_relative_to_newest;
            backfill_interval = backfill_interval_;
            start_option_set = true;
         }

         /**
          * Specifies that any data source requests generated from variables in this expression
          * should start at a position relative to the newest record.
          *
          * @param start_record_offset_ Specifies the offset from the newest record.
          */
         void set_start_at_offset_from_newest(uint4 start_record_offset_)
         {
            start_option = request_type::start_at_offset_from_newest;
            start_record_offset = start_record_offset_;
            start_option_set = true;
         }

         /**
          * Specifies that any data source requests generated from variables in this expression
          * should be a query between two time stamps.
          *
          * @param begin Specifies the start of the time range.
          *
          * @param end Specifies the end of the time range.
          */
         void set_query_times(LgrDate const &begin, LgrDate const &end)
         {
            start_option = request_type::start_date_query;
            start_time = begin;
            end_time = end;
            start_option_set = true;
         }

         /**
          * @param order_option_ Specifies the order option that should be applied to any requests
          * generated from variables for this query.
          */
         typedef request_type::order_option_type order_option_type;
         void set_order_option(int8 order_option_);

         /**
          * Configures the specified request.
          */
         void configure_request(request_type &request);

         /**
          * Sets the report offset for requests generated for this query.
          */
         void set_report_offset(int8 offset)
         {
            report_offset = offset;
            start_option_set = true;
         }

         /**
          * @return Returns the report offset.
          */
         int8 get_report_offset() const
         { return report_offset; }

         /**
          * Assigns the values for variables in this expression based upon the given record and
          * request.
          *
          * @param record Specifies the record that has the values to assign.
          *
          * @param request Specifies the request for the vsriables.
          */
         void assign_request_variables(
            Cora::Broker::Record const &record, request_type &request);

         /**
          * Sets the value of the specified variable object from the specified record value.
          *
          * @param variable Specifies the variable that should be initialised.
          *
          * @param value Specifies the record value that will be copied.
          *
          * @param stamp Specifies the time stamp for the variable.
          */
         static void assign_request_variable(
            variable_handle &variable,
            Cora::Broker::Record::value_type const &value,
            LgrDate const &stamp);

         /**
          * Modifies the specified string that should represent the original source and replaces all
          * references to variables in this expression with the actual values of those variables.
          *
          * @param source Specifies the string to annotate.
          */
         void annotate_source(StrUni &source);

         typedef Csi::Expression::ExcInvalidState ExcInvalidState;

      protected:
         /**
          * Converts the specified list of tokens in infix form to the post-fix notation used on the
          * stack.
          *
          * @param token_list Specifies the infix token list.
          */
         void infix_to_postfix_convert(parsed_tokens_type &token_list);

         /**
          * Specifies the postfix tokens for this expression.
          */
         token_stack_type postfix_stack;

         /**
          * Specifies the token factory used for this expression.
          */
         TokenFactory *token_factory;
         bool owns_factory;

         /**
          * Specifies the list of variables extracted from the postfix stack.
          */
         variables_type variables;

         /**
          * Specifies the javascript index assigned to this expression by the generator.
          */
         int js_index;

         /**
          * Set to true if request start options have been set.
          */
         bool start_option_set;
         
         /**
          * Specifies the request start option.
          */
         start_option_type start_option;

         /**
          * Specifies the request order option.
          */
         order_option_type order_option;

         /**
          * Specifies the file mark and record nuimber for starting request at records.
          */
         uint4 file_mark_no;
         uint4 record_no;

         /**
          * Specifies the start time for start at time or date range.
          */
         LgrDate start_time;

         /**
          * Specifis the end time for date range.
          */
         LgrDate end_time;

         /**
          * Specifies the backfill interval.
          */
         int8 backfill_interval;

         /**
          * Specifies the start record offset.
          */
         uint4 start_record_offset;

         /**
          * Specifies the offset that should be applied to requests from this expression for
          * reports.
          */
         int8 report_offset;
      };
   };
};

#endif

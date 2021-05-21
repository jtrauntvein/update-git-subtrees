/* Csi.Expression.Token.h

   Copyright (C) 2002, 2020 Campbell Scientific, Inc.

   Written by: Tyler Mecham
   Date Begun: 9 January, 2002
   Last Change: Wednesday 15 April 2020
   Last Commit: $Date: 2020-07-14 15:47:08 -0600 (Tue, 14 Jul 2020) $
   Last Changed by: $Author: jbritt $

*/

#pragma once
#ifndef Csi_Expression_Token_h
#define Csi_Expression_Token_h

#include "Csi.SharedPtr.h"
#include "Csi.LightSharedPtr.h"
#include "CsiTypes.h"
#include "Csi.FloatUtils.h"
#include "Cora.DataSources.Request.h"
#include <list>
#include <iosfwd>

namespace Cora
{
   namespace DataSources
   {
      class Manager;
      class SourceBase;
      class SymbolBase;
   };
};


namespace Csi
{
   namespace Expression
   {
      ////////////////////////////////////////////////////////////
      // class Token
      //
      // Defines a base class for token objects that can appear in an
      // expression.  These token objects can include variables, functions,
      // operators, or syntax markers such as parentheses and commas.
      ////////////////////////////////////////////////////////////
      class ExpressionHandler;
      class Token
      {
      public:
         //@group typedefs
         typedef Csi::LightSharedPtr<Token> token_handle ;
         typedef std::list< token_handle > token_stack_type;
         //@endgroup

         ////////////////////////////////////////////////////////////
         // Default Constructor
         //////////////////////////////////////////////////////////// 
         Token()
         { }

         ////////////////////////////////////////////////////////////
         // Destructor
         //////////////////////////////////////////////////////////// 
         virtual ~Token()
         { }

         ////////////////////////////////////////////////////////////
         // eval
         //
         // Pure virtual function that takes the stack object and 
         // evaluates itself against it.  So a binary operator would
         // pop 2 tokens off the stack a unary operator would pop 1.
         //////////////////////////////////////////////////////////// 
         virtual void eval(token_stack_type &stack, ExpressionHandler *expression) = 0;

         ////////////////////////////////////////////////////////////
         // get_priority
         //
         // Return the priority of the token
         //////////////////////////////////////////////////////////// 
         virtual uint2 get_priority() const = 0;

         ////////////////////////////////////////////////////////////
         // is_lparen
         //
         // return true if this token is a left parenthesis
         //////////////////////////////////////////////////////////// 
         virtual bool is_lparen() const
         { return false; }

         ////////////////////////////////////////////////////////////
         // is_rparen
         //
         // returns true if this token is a right parenthesis
         //////////////////////////////////////////////////////////// 
         virtual bool is_rparen() const
         { return false; }

         ////////////////////////////////////////////////////////////
         // is_operator
         //
         // returns true if this token is an operator
         //////////////////////////////////////////////////////////// 
         virtual bool is_operator() const
         { return false; }

         ////////////////////////////////////////////////////////////
         // is_variable
         // returns true if this token is a variable
         //////////////////////////////////////////////////////////// 
         virtual bool is_variable() const
         { return false; }

         ////////////////////////////////////////////////////////////
         // is_operand
         //
         // return true if this token is an operand
         //////////////////////////////////////////////////////////// 
         virtual bool is_operand() const
         { return false; }

         ////////////////////////////////////////////////////////////
         // is_comma
         //
         // return true if this token is a comma
         //////////////////////////////////////////////////////////// 
         virtual bool is_comma() const
         { return false; }

         ////////////////////////////////////////////////////////////
         // is_function
         ////////////////////////////////////////////////////////////
         virtual bool is_function() const
         { return false; }

         ////////////////////////////////////////////////////////////
         // is_semi_colon
         ////////////////////////////////////////////////////////////
         virtual bool is_semi_colon() const
         { return false; }

         ////////////////////////////////////////////////////////////
         // format
         ////////////////////////////////////////////////////////////
         virtual void format(std::ostream &out) const = 0;

         ////////////////////////////////////////////////////////////
         // has_state
         //
         // Can be overloaded to indicate that this token has "state".  This
         // method is used by the factory to determine whether a "function"
         // needs to be cloned or can be used directly. 
         ////////////////////////////////////////////////////////////
         virtual bool has_state() const
         { return false; }

         ////////////////////////////////////////////////////////////
         // reset_state
         //
         // Can be overloaded to reset the "state" of this token.
         ////////////////////////////////////////////////////////////
         virtual void reset_state()
         { }

         ////////////////////////////////////////////////////////////
         // clone
         //
         // Must be overloaded to produce an exact duplicate of this token. 
         ////////////////////////////////////////////////////////////
         virtual Token *clone() const = 0;

         ////////////////////////////////////////////////////////////
         // abort_after_eval
         //
         // Can be overloaded by "special" token types that may modify the
         // conditions of the expression but can also interfere with the
         // expression.   The expression handler will invoke this method after
         // each evaluation and, if this returns true, will abort the
         // evaluation and remove this and all prior tokens from the evaluation
         // stack. 
         ////////////////////////////////////////////////////////////
         virtual bool abort_after_eval()
         { return false; }

         ////////////////////////////////////////////////////////////
         // get_js_resource
         //
         // Can be overloaded to return the name of the JavaScript resource
         // that will support the token subclass.
         ////////////////////////////////////////////////////////////
         virtual StrAsc get_js_resource() const
         { return ""; }

         ////////////////////////////////////////////////////////////
         // format_js
         ////////////////////////////////////////////////////////////
         virtual void format_js(std::ostream &out);

         ////////////////////////////////////////////////////////////
         // clear_args_count
         //
         // Can be overloaded by a derived token class that needs to keep track
         // of the number of arguments on  which to operate.
         ////////////////////////////////////////////////////////////
         virtual void clear_args_count()
         { }

         ////////////////////////////////////////////////////////////
         // increment_args_count
         ////////////////////////////////////////////////////////////
         virtual void increment_args_count()
         { }

         ////////////////////////////////////////////////////////////
         // get_category
         //
         // Can be overloaded to specify the category for this token.
         ////////////////////////////////////////////////////////////
         enum category_type
         {
            category_unknown = 0,
            category_operator = 1,
            category_math_operator = 2,
            category_math_constant = 3,
            category_time_constant = 4,
            category_alias = 5,
            category_start_option = 6,
            category_order_constant = 7,
            category_math_function = 8,
            category_logical_operator = 9,
            category_string_function = 10,
            category_conversion_function = 11,
            category_time_function = 12,
            category_statistics_function = 13,
            category_reset_constant = 14
         };
         virtual category_type get_category() const
         { return category_unknown; }
      };

      //@group typedefs
      typedef Token::token_handle token_handle ;
      typedef Token::token_stack_type token_stack_type;
      //@endgroup 


      ////////////////////////////////////////////////////////////
      // value_type
      ////////////////////////////////////////////////////////////
      enum value_type_identifier
      {
         value_double,
         value_int,
         value_string,
         value_date
      };

      enum precedence_id
      {
         prec_default = 0xFFFF,
         prec_semi_colon = 11,
         prec_paren = 10,
         prec_comma = 9,
         prec_max_operator = 8,
         prec_function = 7,
         prec_negation = 6,
         prec_expon = 5,
         prec_mult_div_mod = 4,
         prec_add_subtr = 3,
         prec_comparator = 2,
         prec_logic_op = 1,
         prec_bit_op = 0,
      };

      class value_type
      {
      public:
         ////////////////////////////////////////////////////////////
         // type
         ////////////////////////////////////////////////////////////
         value_type_identifier type;

         ////////////////////////////////////////////////////////////
         // vdouble
         ////////////////////////////////////////////////////////////
         double vdouble;

         ////////////////////////////////////////////////////////////
         // vint
         ////////////////////////////////////////////////////////////
         int8 vint;

         ////////////////////////////////////////////////////////////
         // vstring
         ////////////////////////////////////////////////////////////
         StrAsc vstring;

         ////////////////////////////////////////////////////////////
         // contructor
         ////////////////////////////////////////////////////////////
         value_type():
            type(value_int),
            vint(0),
            vdouble(0.0)
         { }

         ////////////////////////////////////////////////////////////
         // copy constructor
         ////////////////////////////////////////////////////////////
         value_type(value_type const &other):
            type(other.type),
            vint(other.vint),
            vdouble(other.vdouble),
            vstring(other.vstring)
         { }

         ////////////////////////////////////////////////////////////
         // copy operator
         ////////////////////////////////////////////////////////////
         value_type &operator =(value_type const &other)
         {
            type = other.type;
            vint = other.vint;
            vdouble = other.vdouble;
            vstring = other.vstring;
            return *this;
         } 
      };

      
      ////////////////////////////////////////////////////////////
      // class Operand
      //////////////////////////////////////////////////////////// 
      class Operand : public Token
      {
      protected:
         ////////////////////////////////////////////////////////////
         // value
         ////////////////////////////////////////////////////////////
         value_type value;

         ////////////////////////////////////////////////////////////
         // timestamp
         ////////////////////////////////////////////////////////////
         LgrDate timestamp;
         
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         Operand(double value_ = -1.0, LgrDate const &timestamp_ = 0):
            timestamp(timestamp_)
         {
            value.type = value_double;
            value.vdouble = value_;
         } 

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~Operand()
         {}

         ////////////////////////////////////////////////////////////
         // get_value
         ////////////////////////////////////////////////////////////
         virtual value_type const &get_value() const
         { return value; }

         ////////////////////////////////////////////////////////////
         // reset_state
         ////////////////////////////////////////////////////////////
         virtual void reset_state()
         {
            Token::reset_state();
            value.type = value_double;
            value.vdouble = -1.0;
            timestamp = 0;
         }

         ////////////////////////////////////////////////////////////
         // get_timestamp
         ////////////////////////////////////////////////////////////
         LgrDate get_timestamp() const
         { return timestamp; }

         ////////////////////////////////////////////////////////////
         // set_timestamp
         ////////////////////////////////////////////////////////////
         void set_timestamp(LgrDate const &timestamp_)
         { timestamp = timestamp_; }
         
         ////////////////////////////////////////////////////////////
         // get_val
         ////////////////////////////////////////////////////////////
         virtual double get_val() const
         {
            double rtn = 0.0;
            switch(value.type)
            {
            case value_double:
               rtn = value.vdouble;
               break;

            case value_int:
            case value_date:
               rtn = static_cast<double>(value.vint);
               break;

            case value_string:
               rtn = csiStringToFloat(value.vstring.c_str(), std::locale::classic());
               break;
            }
            return rtn;
         } // get_val

         ////////////////////////////////////////////////////////////
         // get_val_int
         ////////////////////////////////////////////////////////////
         virtual int8 get_val_int() const
         {
            int8 rtn = -1;
            switch(value.type)
            {
            case value_double:
               if(is_finite(value.vdouble))
                  rtn = static_cast<int8>(value.vdouble);
               break;
               
            case value_int:
            case value_date:
               rtn = value.vint;
               break;
               
            case value_string:
               // in order to honour things like exponential notation, we will first convert the
               // value to a double and then to an integer.
               rtn = static_cast<int8>(get_val());
               break;
            }
            return rtn;
         }

         ////////////////////////////////////////////////////////////
         // get_val_str
         ////////////////////////////////////////////////////////////
         virtual StrAsc get_val_str() const
         {
            StrAsc rtn;
            char temp[25];
            
            switch(value.type)
            {
            case value_string:
               rtn = value.vstring;
               break;

            case value_int:
#ifdef _WIN32
#pragma warning(disable: 4996)
               sprintf(temp,"%I64d",value.vint);
#pragma warning(default: 4996)
#else
               sprintf(temp,"%Ld",value.vint);
#endif
               rtn = temp;
               break;

            case value_date:
               Csi::LgrDate(value.vint).format(rtn, "%Y-%m-%d %H:%M:%S%x");
               break;
               
            case value_double:
               csiFloatToString(rtn,value.vdouble);
               break;
            }
            return rtn;
         }

         ////////////////////////////////////////////////////////////
         // get_val_date
         ////////////////////////////////////////////////////////////
         virtual int8 get_val_date() const
         {
            int8 rtn = 0;
            switch(value.type)
            {
            case value_string:
               rtn = Csi::LgrDate::fromStr(value.vstring.c_str()).get_nanoSec();
               break;
               
            case value_double:
               rtn = static_cast<int8>(value.vdouble);
               break;
               
            case value_int:
            case value_date:
               rtn = value.vint;
               break;
            }
            return rtn;
         } 

         ////////////////////////////////////////////////////////////
         // set_val
         ////////////////////////////////////////////////////////////
         virtual void set_val(double nval, LgrDate const &timestamp_)
         {
            value.type = value_double;
            value.vdouble = nval;
            timestamp = timestamp_;
         }

         ////////////////////////////////////////////////////////////
         // set_val
         ////////////////////////////////////////////////////////////
         virtual void set_val(int8 nval, LgrDate const &timestamp_)
         {
            value.type = value_int;
            value.vint = nval;
            timestamp = timestamp_;
         }

         ////////////////////////////////////////////////////////////
         // set_val
         ////////////////////////////////////////////////////////////
         virtual void set_val(StrAsc const &val, LgrDate const &timestamp_)
         {
            value.type = value_string;
            value.vstring = val;
            timestamp = timestamp_;
         }

         ////////////////////////////////////////////////////////////
         // set_val_date
         ////////////////////////////////////////////////////////////
         virtual void set_val_date(int8 val, LgrDate const &timestamp_)
         {
            value.type = value_date;
            value.vint = val;
            timestamp = timestamp_;
         }

         ////////////////////////////////////////////////////////////
         // set_val
         ////////////////////////////////////////////////////////////
         virtual void set_val(value_type const &value_, LgrDate const &timestamp_)
         {
            value = value_;
            timestamp = timestamp_;
         }

         ////////////////////////////////////////////////////////////
         // get_type
         ////////////////////////////////////////////////////////////
         value_type_identifier get_type() const
         { return value.type; } 

         //@group Token virtual function implementations
         ////////////////////////////////////////////////////////////
         // eval
         //////////////////////////////////////////////////////////// 
         virtual void eval(token_stack_type &stack, ExpressionHandler *expression)
         { }

         ////////////////////////////////////////////////////////////
         // get_priority
         ////////////////////////////////////////////////////////////
         virtual uint2 get_priority() const
         { return prec_default; }

         ////////////////////////////////////////////////////////////
         // is_operand
         ////////////////////////////////////////////////////////////
         virtual bool is_operand() const
         { return true; }

         ///////////////////////////////////////////////////////////
         // is_constant
         ///////////////////////////////////////////////////////////
         virtual bool is_constant() const
         { return false; }

         ////////////////////////////////////////////////////////////
         // format
         ////////////////////////////////////////////////////////////
         virtual void format(std::ostream &out) const
         {
            switch(value.type)
            {
            case value_string:
               out << "\"" << value.vstring << "\"";
               break;
               
            case value_int:
               out << value.vint;
               break;

            case value_date:
               Csi::LgrDate(value.vint).format(out, "\"%Y-%m-%d %H:%M:%S%x\"");
               break;
               
            case value_double:
               csiFloatToStream(out, value.vdouble);
               break; 
            }
         } 
         //@endgroup

         ////////////////////////////////////////////////////////////
         // clone
         ////////////////////////////////////////////////////////////
         virtual Operand *clone() const
         {
            Operand *rtn = new Operand;
            rtn->value = value;
            return rtn;
         } 
      };


      ////////////////////////////////////////////////////////////
      // Class Constant
      //
      // Represents a constant value in an expression
      //////////////////////////////////////////////////////////// 
      class Constant: public Operand
      {
      private:
         ////////////////////////////////////////////////////////////
         // name
         ////////////////////////////////////////////////////////////
         StrUni name;

         ////////////////////////////////////////////////////////////
         // category
         ////////////////////////////////////////////////////////////
         category_type category;
         
      public:
         ////////////////////////////////////////////////////////////
         // constructor (from double)
         ////////////////////////////////////////////////////////////
         Constant(
            double val = -1.0,
            StrUni const &name_ = L"",
            category_type category_ = category_unknown):
            name(name_),
            category(category_)
         {
            value.type = value_double;
            value.vdouble = val;
         }
         
         ////////////////////////////////////////////////////////////
         // constructor (from int8)
         ////////////////////////////////////////////////////////////
         Constant(
            int8 val,
            StrUni const &name_,
            category_type category_ = category_unknown):
            name(name_),
            category(category_)
         {
            value.type = value_int;
            value.vint = val;
         }

         /**
          * String constructor (construct string, double, or integer token type).
          *
          * @param val Specifies the string to be parsed to initialise the value.
          *
          * @param variables_quoted Set to true to indicate that the beginning of a string should be
          * marked with a dollar sign.
          */
         Constant(StrAsc const &val, bool variables_quoted = true);
         Constant(StrUni const &val, bool variables_quoted = true);

         ////////////////////////////////////////////////////////////
         // Destructor
         //////////////////////////////////////////////////////////// 
         virtual ~Constant()
         {}

         ///////////////////////////////////////////////////////////
         // is_constant
         ///////////////////////////////////////////////////////////
         virtual bool is_constant() const
         { return true; }

         ////////////////////////////////////////////////////////////
         // get_js_resource
         ////////////////////////////////////////////////////////////
         virtual StrAsc get_js_resource() const
         { return "CsiConstant.js"; }

         ////////////////////////////////////////////////////////////
         // format_js
         ////////////////////////////////////////////////////////////
         virtual void format_js(std::ostream &out);

         ////////////////////////////////////////////////////////////
         // format
         ////////////////////////////////////////////////////////////
         virtual void format(std::ostream &out) const
         {
            if(name.length())
               out << name;
            else
               Operand::format(out);
         }

         ////////////////////////////////////////////////////////////
         // get_category
         ////////////////////////////////////////////////////////////
         virtual category_type get_category() const
         { return category; }
      };

      
      ////////////////////////////////////////////////////////////
      // Class Variable
      //
      // Represents a string variable in an expression
      //////////////////////////////////////////////////////////// 
      class Variable: public Operand
      {
      public:
         ////////////////////////////////////////////////////////////
         // Constructor
         //////////////////////////////////////////////////////////// 
         Variable(StrUni const &name):
            var_name(name),
            has_been_set(false),
            record_index(0xFFFFFFFF),
            js_index(0)
         { }

         ////////////////////////////////////////////////////////////
         // Destructor
         //////////////////////////////////////////////////////////// 
         virtual ~Variable(){}

         ////////////////////////////////////////////////////////////
         // reset_state
         ////////////////////////////////////////////////////////////
         virtual void reset_state()
         {
            Operand::reset_state();
            has_been_set = false;
            record_index = 0xFFFFFFFF;
         }

         //@group Token virtual function implementations
         ////////////////////////////////////////////////////////////
         // is_variable
         ////////////////////////////////////////////////////////////
         virtual bool is_variable() const
         { return true; }
         //@endgroup

         ////////////////////////////////////////////////////////////
         // get_var_name
         ////////////////////////////////////////////////////////////
         StrUni const &get_var_name() const
         { return var_name; }

         /**
          * @return Overloaded to return the value to check to see of the variable has been set.
          */
         virtual value_type const &get_value() const
         {
            check_set();
            return Operand::get_value();
         }
         
         ////////////////////////////////////////////////////////////
         // get_val
         ////////////////////////////////////////////////////////////
         virtual double get_val() const
         {
            check_set();
            return Operand::get_val();
         }

         ////////////////////////////////////////////////////////////
         // get_val_int
         ////////////////////////////////////////////////////////////
         virtual int8 get_val_int() const
         {
            check_set();
            return Operand::get_val_int();
         }

         ////////////////////////////////////////////////////////////
         // get_val_str
         ////////////////////////////////////////////////////////////
         virtual StrAsc get_val_str() const
         {
            check_set();
            return Operand::get_val_str();
         }

         ////////////////////////////////////////////////////////////
         // get_val_date
         ////////////////////////////////////////////////////////////
         virtual int8 get_val_date() const
         {
            check_set();
            return Operand::get_val_date();
         }

         ////////////////////////////////////////////////////////////
         // set_val
         ////////////////////////////////////////////////////////////
         virtual void set_val(double nval, Csi::LgrDate const &timestamp)
         {
            Operand::set_val(nval, timestamp);
            has_been_set = true;
         }

         ////////////////////////////////////////////////////////////
         // set_val (int version)
         ////////////////////////////////////////////////////////////
         virtual void set_val(int8 nval, Csi::LgrDate const &timestamp)
         {
            Operand::set_val(nval, timestamp);
            has_been_set = true;
         }

         ////////////////////////////////////////////////////////////
         // set_val (string version)
         ////////////////////////////////////////////////////////////
         virtual void set_val(StrAsc const &nval, Csi::LgrDate const &timestamp)
         {
            Operand::set_val(nval, timestamp);
            has_been_set = true;
         }

         ////////////////////////////////////////////////////////////
         // set_val
         ////////////////////////////////////////////////////////////
         virtual void set_val_date(Csi::LgrDate const &val, Csi::LgrDate const &timestamp)
         {
            Operand::set_val_date(val.get_nanoSec(), timestamp);
            has_been_set = true;
         }

         ////////////////////////////////////////////////////////////
         // set_val
         ////////////////////////////////////////////////////////////
         virtual void set_val(value_type const &value_, Csi::LgrDate const &timestamp)
         {
            Operand::set_val(value_, timestamp);
            has_been_set = true;
         }

         ////////////////////////////////////////////////////////////
         // format
         ////////////////////////////////////////////////////////////
         virtual void format(std::ostream &out) const
         {
            out << "(" << var_name.to_utf8() << "=";
            Operand::format(out);
            out << ")";
         }

         ////////////////////////////////////////////////////////////
         // get_js_resource
         ////////////////////////////////////////////////////////////
         virtual StrAsc get_js_resource() const
         { return "CsiVariable.js"; }

         /*
          * generate_constructor_js
          *
          * @param sources - the Datasources manager
          * @param out - The output stream for generating the javascript
          */
         virtual void generate_constructor_js(SharedPtr<Cora::DataSources::Manager> &sources, std::ostream &out);
         
         ////////////////////////////////////////////////////////////
         // format_js
         ////////////////////////////////////////////////////////////
         virtual void format_js(std::ostream &out)
         { out << "variables[" << js_index << "]"; }

         ///////////////////////////////////////////////////////////
         // set_record_index
         ///////////////////////////////////////////////////////////
         void set_record_index(uint4 record_index_)
         { record_index = record_index_; }

         ///////////////////////////////////////////////////////////
         // get_record_index
         ///////////////////////////////////////////////////////////
         uint4 get_record_index() const
         { return record_index; }

         ////////////////////////////////////////////////////////////
         // get_has_been_set
         ////////////////////////////////////////////////////////////
         bool get_has_been_set() const
         { return has_been_set; }

         ////////////////////////////////////////////////////////////
         // get_js_index
         ////////////////////////////////////////////////////////////
         int get_js_index() const
         { return js_index; }

         ////////////////////////////////////////////////////////////
         // set_js_index
         ////////////////////////////////////////////////////////////
         void set_js_index(int index)
         { js_index = index; }

         ////////////////////////////////////////////////////////////
         // set_request
         ////////////////////////////////////////////////////////////
         typedef Cora::DataSources::Request request_type;
         void set_request(SharedPtr<request_type> &request_)
         { request = request_; }

         ////////////////////////////////////////////////////////////
         // get_request
         ////////////////////////////////////////////////////////////
         SharedPtr<request_type> get_request()
         { return request; }
         
      private:
         ///////////////////////////////////////////////////////////
         // record_index
         ///////////////////////////////////////////////////////////
         uint4 record_index;

         ////////////////////////////////////////////////////////////
         // var_name
         ////////////////////////////////////////////////////////////
         StrUni var_name;

         ////////////////////////////////////////////////////////////
         // has_been_set
         ////////////////////////////////////////////////////////////
         bool has_been_set;

         ////////////////////////////////////////////////////////////
         // check_set
         ////////////////////////////////////////////////////////////
         void check_set() const
         {
            if(!has_been_set)
               throw std::invalid_argument("variable value not set");
         }

         ////////////////////////////////////////////////////////////
         // js_index
         //
         // The index number assigned to this variable by the javascript generator. 
         ////////////////////////////////////////////////////////////
         int js_index;

         ////////////////////////////////////////////////////////////
         // request
         //
         // Stores the request that was associated with this variable when the expression was
         // parsed. 
         ////////////////////////////////////////////////////////////
         SharedPtr<request_type> request;
      };


      ////////////////////////////////////////////////////////////
      // class Function
      //
      // Defines a base class that overloads the appropriate methods for a function.
      ////////////////////////////////////////////////////////////
      class Function: public Token
      {
      public:
         ////////////////////////////////////////////////////////////
         // get_priority
         ////////////////////////////////////////////////////////////
         virtual uint2 get_priority() const
         { return prec_function; }

         ////////////////////////////////////////////////////////////
         // is_operator
         ////////////////////////////////////////////////////////////
         virtual bool is_operator() const
         { return true; }

         ////////////////////////////////////////////////////////////
         // is_operand
         ////////////////////////////////////////////////////////////
         virtual bool is_function() const
         { return true; }

         ////////////////////////////////////////////////////////////
         // get_min_arguments
         //
         // Must be overloaded to return the minimum number of parameters that
         // this function will pull off the stack.
         ////////////////////////////////////////////////////////////
         virtual uint4 get_min_arguments() = 0;

         ////////////////////////////////////////////////////////////
         // get_max_arguments
         //
         // Can be overloaded to return the maximum number of arguments that
         // this function will pull off the stack.
         ////////////////////////////////////////////////////////////
         virtual uint4 get_max_arguments()
         { return get_min_arguments(); }

         ////////////////////////////////////////////////////////////
         // get_argument_name
         //
         // Must be overloaded to return the name of the specified argument
         // index. 
         ////////////////////////////////////////////////////////////
         virtual wchar_t const *get_argument_name(uint4 index) = 0;
         
         ////////////////////////////////////////////////////////////
         // get_argument_type
         ////////////////////////////////////////////////////////////
         enum argument_type_code
         {
            argument_type_unknown = 0,
            argument_type_date = 1,
            argument_type_number = 2,
            argument_type_order_option = 3
         };
         virtual argument_type_code get_argument_type(uint4 index)
         { return argument_type_unknown; }

         ////////////////////////////////////////////////////////////
         // is_start_condition
         //
         // Can be overloaded to indicate that this function sets start
         // conditions for the expression requests.
         ////////////////////////////////////////////////////////////
         virtual bool is_start_condition() const
         { return false; }

         ////////////////////////////////////////////////////////////
         // is_alias
         //
         // Can be overloaded to indicate that this function creates a variable
         // alias. 
         ////////////////////////////////////////////////////////////
         virtual bool is_alias() const
         { return false; }

         ////////////////////////////////////////////////////////////
         // is_value_synch
         //
         // Can be overloaded to indicate that this function creates a variable
         // value synch. 
         ////////////////////////////////////////////////////////////
         virtual bool is_value_synch() const
         { return false; }
      };


      ////////////////////////////////////////////////////////////
      // class ExcSynchingValues
      //
      // Defines a class of exception that is thrown by a variable generated by
      // ValueSynch() when it has no value for the current time.  By looking
      // for this class of exception, the application can distinguish this
      // particular condition.
      ////////////////////////////////////////////////////////////
      class ExcSynchValues: public Csi::MsgExcept
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         ExcSynchValues():
            MsgExcept("No matching values for ValueSynch()")
         { }
      };
   };
};

#endif //csi_expression_token_h

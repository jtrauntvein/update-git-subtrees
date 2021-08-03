/* Csi.Expression.TokenFactory.cpp

   Copyright (C) 2002, 2020 Campbell Scientific, Inc.

   Written by: Tyler Mecham
   Date Begun: 1/9/2002 09:55:33
   Last Change: Wednesday 13 May 2020
   Last Commit: $Date: 2020-05-13 13:50:49 -0600 (Wed, 13 May 2020) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header

#include "Csi.Expression.TokenFactory.h"
#include "Csi.Expression.TokenTypes.h"
#include "Csi.Expression.ExpressionHandler.h"
#include "Csi.LgrDate.h"
#include "Cora.DataSources.Manager.h"


namespace Csi
{
   namespace Expression
   {
      TokenFactory::TokenFactory(bool variables_quoted_):
         variables_quoted(variables_quoted_)
      {
         //Define basic operators
         functions[L"("] = token_handle(new LeftParen);
         functions[L")"] = token_handle(new RightParen);
         functions[L";"] = token_handle(new SemiColon); 
         functions[L","] = token_handle(new Comma);
            
         //Define basic constants
         functions[L"NOPLOT"] = token_handle(
            new Constant(
               std::numeric_limits<double>::quiet_NaN(),
               L"NOPLOT",
               Token::category_math_constant));
         functions[L"NAN"] = token_handle(
            new Constant(
               std::numeric_limits<double>::quiet_NaN(),
               L"NAN",
               Token::category_math_constant));
         functions[L"INF"] = token_handle(
            new Constant(
               std::numeric_limits<double>::infinity(),
               L"INF",
               Token::category_math_constant));
         functions[L"TRUE"] = token_handle(
            new Constant(-1.0, L"TRUE", Token::category_math_constant));
         functions[L"FALSE"] = token_handle(
            new Constant(0.0, L"FALSE", Token::category_math_constant));
         functions[L"PI"] = token_handle(
            new Constant(3.14159265359, L"PI", Token::category_math_constant));
         functions[L"e"] = token_handle(
            new Constant(2.718282, L"e", Token::category_math_constant));
         functions[L"nsecPerUSec"] = token_handle(
            new Constant(
               Csi::LgrDate::nsecPerUSec, L"nsecPerUSec", Token::category_time_constant));
         functions[L"nsecPerMSec"] = token_handle(
            new Constant(
               Csi::LgrDate::nsecPerMSec, L"nsecPerMSec", Token::category_time_constant));
         functions[L"nsecPerSec"] = token_handle(
            new Constant(
               Csi::LgrDate::nsecPerSec, L"nsecPerSec", Token::category_time_constant));
         functions[L"nsecPerMin"] = token_handle(
            new Constant(
               Csi::LgrDate::nsecPerMin, L"nsecPerMin", Token::category_time_constant));
         functions[L"nsecPerHour"] = token_handle(
            new Constant(
               Csi::LgrDate::nsecPerHour, L"nsecPerHour", Token::category_time_constant));
         functions[L"nsecPerDay"] = token_handle(
            new Constant(
               Csi::LgrDate::nsecPerDay, L"nsecPerDay", Token::category_time_constant));
         functions[L"nsecPerWeek"] = token_handle(
            new Constant(
               Csi::LgrDate::nsecPerWeek, L"nsecPerWeek", Token::category_time_constant));
         functions[L"RESET_HOURLY"] = token_handle(
            new Constant(
               static_cast<int8>(1), L"RESET_HOURLY", Token::category_reset_constant));
         functions[L"RESET_DAILY"] = token_handle(
            new Constant(
               static_cast<int8>(2), L"RESET_DAILY", Token::category_reset_constant));
         functions[L"RESET_WEEKLY"] = token_handle(
            new Constant(
               static_cast<int8>(5), L"RESET_WEEKLY", Token::category_reset_constant));
         functions[L"RESET_MONTHLY"] = token_handle(
            new Constant(
               static_cast<int8>(3), L"RESET_MONTHLY", Token::category_reset_constant));
         functions[L"RESET_YEARLY"] = token_handle(
            new Constant(
               static_cast<int8>(4), L"RESET_YEARLY", Token::category_reset_constant));
         functions[L"RESET_CUSTOM"] = token_handle(
            new Constant(
               static_cast<int8>(6), L"RESET_CUSTOM", Token::category_reset_constant));

         //Math Operands
         functions[L"^"] = token_handle(new Exponentiation);
         functions[L"*"] = token_handle(new Multiplication);
         functions[L"/"] = token_handle(new Division);
         functions[L"+"] = token_handle(new Addition);
         functions[L"-"] = token_handle(new Subtraction);
         functions[L"(-)"] = token_handle(new Negation(L"(-)"));
         
         //Logic Operands
         functions[L"="] = token_handle(new IsEqual);
         functions[L"<>"] = token_handle(new NotEqual);
         functions[L">"] = token_handle(new Greater);
         functions[L"<"] = token_handle(new Less);
         functions[L">="] = token_handle(new GreaterEqual);
         functions[L"<="] = token_handle(new LessEqual);
         functions[L"SelectSwitch"] = token_handle(new Switch);

         //Math Functions
         functions[L"ABS"] = token_handle(new AbsoluteValue);
         functions[L"ACOS"] = token_handle(new ArcCosine);
         functions[L"AND"] = token_handle(new AndOperator);
         functions[L"ASIN"] = token_handle(new ArcSine);
         functions[L"ATN"] = token_handle(new ArcTangent);
         functions[L"ATN2"] = token_handle(new ArcTangent2);
         functions[L"COS"] = token_handle(new Cosine);
         functions[L"COSH"] = token_handle(new HyperbolicCosine);
         functions[L"CSGN"] = token_handle(new CsgnFunction);
         functions[L"EQV"] = token_handle(new Equivalence);
         functions[L"EXP"] = token_handle(new EtoPower);
         functions[L"FIX"] = token_handle(new Fix);
         functions[L"FRAC"] = token_handle(new FractionalPart);
         functions[L"IIF"] = token_handle(new IIF);
         functions[L"IMP"] = token_handle(new Implication);
         functions[L"INT"] = token_handle(new Int);
         functions[L"LN"] = token_handle(new NaturalLog(L"LN"));
         functions[L"LOG"] = token_handle(new NaturalLog(L"LOG"));
         functions[L"LOG10"] = token_handle(new LogBase10);
         functions[L"MOD"] = token_handle(new Modulo);
         functions[L"NOT"] = token_handle(new NotOperator);
         functions[L"OR"] = token_handle(new OrOperator);
         functions[L"PWR"] = token_handle(new PwrFunction);
         functions[L"RND"] = token_handle(new Random);
         functions[L"SGN"] = token_handle(new Sign);
         functions[L"SIN"] = token_handle(new Sine);
         functions[L"SINH"] = token_handle(new HyperbolicSine);
         functions[L"SQR"] = token_handle(new SquareRoot);
         functions[L"TAN"] = token_handle(new Tangent);
         functions[L"TANH"] = token_handle(new HyperbolicTangent);
         functions[L"XOR"] = token_handle(new XOrOperator);
         functions[L"FormatFloat"] = token_handle(new FormatFloat);
         functions[L"FormatFloatL"] = token_handle(new FormatFloatL);
         functions[L"IsFinite"] = token_handle(new IsFinite);
         functions[L"Ceiling"] = token_handle(new Ceiling);
         functions[L"Floor"] = token_handle(new Floor);
         functions[L"Round"] = token_handle(new Round);
         
         //String Functions
         functions[L"InStr"] = token_handle(new InStr);
         functions[L"InStrRev"] = token_handle(new InStrRev);
         functions[L"Left"] = token_handle(new Left);
         functions[L"Len"] = token_handle(new Len);
         functions[L"LTrim"] = token_handle(new LTrim);
         functions[L"Mid"] = token_handle(new Mid);
         functions[L"Replace"] = token_handle(new Replace);
         functions[L"Right"] = token_handle(new Right);
         functions[L"RTrim"] = token_handle(new RTrim);
         functions[L"Space"] = token_handle(new Space);
         functions[L"StrComp"] = token_handle(new StrComp);
         functions[L"StrReverse"] = token_handle(new StrReverse);
         functions[L"Trim"] = token_handle(new Trim);
         functions[L"Hex"] = token_handle(new Hex);
         functions[L"HexToDec"] = token_handle(new HexToDec);
         
         //Functions With State
         functions[L"Alias"] = token_handle(new Alias);
         functions[L"AvgRun"] = token_handle(new AvgRun);
         functions[L"AvgRunOverTime"] = token_handle(new AvgRunOverTime);
         functions[L"AvgRunOverTimeWithReset"] = token_handle(new AvgRunOverTimeWithReset);
         functions[L"AvgSpa"] = token_handle(new AvgSpa);
         functions[L"Last"] = token_handle(new Last);
         functions[L"MaxRun"] = token_handle(new MaxRun);
         functions[L"MaxRunOverTime"] = token_handle(new MaxRunOverTime);
         functions[L"MaxRunOverTimeWithReset"] = token_handle(new MaxRunOverTimeWithReset);
         functions[L"MaxSpa"] = token_handle(new MaxSpa);
         functions[L"MedianRun"] = token_handle(new MedianRun);
         functions[L"MedianRunOverTime"] = token_handle(new MedianRunOverTime);
         functions[L"MinRun"] = token_handle(new MinRun);
         functions[L"MinRunOverTime"] = token_handle(new MinRunOverTime);
         functions[L"MinRunOverTimeWithReset"] = token_handle(new MinRunOverTimeWithReset);
         functions[L"MinSpa"] = token_handle(new MinSpa);
         functions[L"ValueAtTime"] = token_handle(new ValueAtTime);
         functions[L"Total"] = token_handle(new Total);
         functions[L"TotalOverTime"] = token_handle(new TotalOverTime);
         functions[L"TotalOverTimeWithReset"] = token_handle(new TotalOverTimeWithReset);
         functions[L"StdDev"] = token_handle(new StdDev);
         functions[L"StdDevOverTime"] = token_handle(new StdDevOverTime);
         functions[L"StdDevOverTimeWithReset"] = token_handle(new StdDevOverTimeWithReset);
         functions[L"ValueSynch"] = token_handle(new ValueSynch);
         
         // functions associated with time/date
         functions[L"FormatTime"] = token_handle(new FormatTime);
         functions[L"SystemTime"] = token_handle(new SystemTime);
         functions[L"SystemTimeGmt"] = token_handle(new SystemTimeGmt);
         functions[L"Timestamp"] = token_handle(new Timestamp);
         functions[L"SetTimestamp"] = token_handle(new SetTimestamp);
         functions[L"LocalToGmt"] = token_handle(new LocalToGmt);
         functions[L"GmtToLocal"] = token_handle(new GmtToLocal);
         
         // explicit type conversion functions
         functions[L"ToDate"] = token_handle(new ToDate);
         functions[L"ToFloat"] = token_handle(new ToFloat);
         functions[L"ToInt"] = token_handle(new ToInt);

         // start condition functions
         functions[L"StartAtRecord"] = token_handle(new StartAtRecordFunction);
         functions[L"StartAtTime"] = token_handle(new StartAtTimeFunction);
         functions[L"StartAtNewest"] = token_handle(new StartAtNewestFunction);
         functions[L"StartAfterNewest"] = token_handle(new StartAfterNewestFunction);
         functions[L"StartRelativeToNewest"] = token_handle(new StartRelativeToNewestFunction);
         functions[L"StartAtOffsetFromNewest"] = token_handle(new StartAtOffsetFromNewestFunction);
         functions[L"ReportOffset"] = token_handle(new ReportOffsetFunction);
         functions[L"OrderCollected"] = token_handle(
            new Constant(
               static_cast<int8>(ExpressionHandler::request_type::order_collected),
               L"OrderCollected",
               Token::category_order_constant));
         functions[L"OrderLoggedWithHoles"] = token_handle(
            new Constant(
               static_cast<int8>(ExpressionHandler::request_type::order_logged_with_holes),
               L"OrderLoggedWithHoles",
               Token::category_order_constant));
         functions[L"OrderLoggedWithoutHoles"] = token_handle(
            new Constant(
               static_cast<int8>(ExpressionHandler::request_type::order_logged_without_holes),
               L"OrderLoggedWithoutHoles",
               Token::category_order_constant));
         functions[L"OrderRealTime"] = token_handle(
            new Constant(
               static_cast<int8>(ExpressionHandler::request_type::order_real_time),
               L"OrderRealTime",
               Token::category_order_constant));
      }


      TokenFactory::~TokenFactory()
      { }


      void TokenFactory::register_function(StrUni const &key, token_handle func)
      {
         functions[key] = func;
      }


      token_handle TokenFactory::make_token(
         token_handle &prev_token,
         StrUni const &token_string,
         size_t start_pos)
      {
         token_handle rtn;
         functions_type::iterator it = functions.find(token_string);
         if( it != functions.end() )
         {
            if(!it->second->has_state())
               rtn = it->second;
            else
               rtn.bind(it->second->clone());
         }
         else
         {
            //We need to see if this is a variable or a constant
            if(is_number(token_string))
               rtn.bind(new Constant(token_string));
            else if(variables_quoted && token_string.first() == L'$')
            {
               if(token_string.length() >= 3 &&
                    token_string[1] == L'\"' &&
                    token_string.last() == L'\"')
                  rtn.bind(new Constant(token_string, true));
            }
            else if(!variables_quoted && token_string.first() == L'\"' && token_string.last() == '\"')
               rtn.bind(new Constant(token_string, false));
            else if(token_string.first() == L'&')
               rtn.bind(new Constant(token_string));
            else
               rtn.bind(new Variable(token_string));
         }
            
         //Check to see if we have a unary operator
         if(prev_token == 0 ||
            (prev_token->is_operator() && !prev_token->is_function()) ||
            prev_token->is_lparen() ||
            prev_token->is_semi_colon())
         {
            if(token_string == L"+")
            {
               //We ignore unary "+" so clear out rtn that has been set to addition
               rtn.clear();
            }
            else if(token_string == L"-")
            {
               functions_type::iterator nit = functions.find(L"(-)");
               if(nit != functions.end())
                  rtn = nit->second;
               else
                  rtn.bind(new Negation(L"-"));
            }
         }

         // Apply Expression Rules Here
         if(prev_token.get_rep() && prev_token->is_operand() && rtn->is_operand())
            throw ExcParseError(start_pos, "Syntax Error: operand followed by an operand");
         return rtn;
      } // make_token


      namespace
      {
         class SourceTimeVariable: public Variable
         {
         public:
            SourceTimeVariable(Cora::DataSources::SinkBase *sink_, StrUni const &source_name):
               Variable(source_name),
               sink(sink_)
            {
               value.type = Csi::Expression::value_date;
            }

            /*
             * get_js_resource
             *
             * @return string that represents the name of the javascript file that needs to be included as a resource
             */
            virtual StrAsc get_js_resource() const
            { return "CsiSourceTimeVariable.js"; }

            /*
             * generate_constructor_js
             *
             * @param sources - the Datasources manager
             * @param out - The output stream for generating the javascript
             */
            virtual void generate_constructor_js(SharedPtr<Cora::DataSources::Manager> &sources, std::ostream &out)
            {
               out << "new CsiSourceTimeVariable(" << get_var_name() << ")";
            }

            virtual value_type const &get_value() const
            {
               auto source = sink->get_source(get_var_name());
               Csi::LgrDate source_time;
               if(source)
               {
                  source_time = source->get_source_time();
               }
               time_value.type = value_date;
               time_value.vint = source_time.get_nanoSec();
               return time_value;
            }

            virtual double get_val() const
            {
               auto val(get_value());
               return (double)val.vint;
            }

            virtual int8 get_val_int() const
            {
               auto val(get_value());
               return val.vint;
            }

            virtual int8 get_val_date() const
            {
               auto val(get_value());
               return val.vint;
            }

            virtual StrAsc get_val_str() const
            {
               auto val(get_value());
               StrAsc rtn;
               Csi::LgrDate(val.vint).format(rtn, "%Y-%m-%d %H:%M:%S%x");
               return rtn;
            }

            virtual SourceTimeVariable *clone() const
            {
               return new SourceTimeVariable(sink, get_var_name());
            }

         private:
            Cora::DataSources::SinkBase *sink;

            mutable value_type time_value;
         };
      };


      TokenFactory::expression_handle TokenFactory::make_expression(
         Cora::DataSources::SinkBase *sink,
         StrUni &expression_str,
         requests_type &requests,
         StrUni const default_source_name,
         TokenFactory *factory,
         Cora::DataSources::Request::order_option_type default_order_option)
      {
         // generate the expression object and replace variables as needed.
         expression_handle rtn(new ExpressionHandler(factory));
         rtn->tokenise(expression_str);
         for(auto vi = rtn->begin(); vi != rtn->end(); ++vi)
         {
            // insert any default source name that is specified
            StrUni uri(vi->first);
            if(default_source_name.length() > 0)
            {
               uri.insert(default_source_name.c_str(), 0);
               expression_str.replace(vi->first.c_str(), uri.c_str());
            }

            // we need to replace any variable that references only a source with a variable that
            // will evaluate the source time.
            if(uri.find(L":") >= uri.length())
            {
               auto var(vi->second.get_handle());
               auto &postfix_stack(rtn->get_postfix_stack());
               vi->second.bind(new SourceTimeVariable(sink, uri));
               for(auto pi = postfix_stack.begin(); pi != postfix_stack.end(); ++pi)
               {
                  if(*pi == var)
                     *pi = vi->second.get_handle();
               }
            }
         }

         // evaluate any precondition expressions.
         while(rtn->has_aborting_tokens())
            ExpressionHandler::operand_handle op(rtn->eval());

         // we can now configure the requests for this expression
         requests.clear();
         for(ExpressionHandler::iterator vi = rtn->begin(); vi != rtn->end(); ++vi)
         {
            requests_type::value_type request(new Cora::DataSources::Request(sink, vi->first));
            request->set_order_option(default_order_option);
            rtn->configure_request(*request);
            vi->second->set_request(request);
            requests.push_back(request);
         }
         return rtn;
      } // make_expression


      bool TokenFactory::is_number(StrUni const &val)
      {
         size_t len = val.length();
         int cVal = 0;
         bool found_decimal = false;
         bool found_E = false;
         bool found_sign = false;
         wchar_t c = val.charAt(0);
         //Make sure it starts with a number or decimal
         if(c != L'.' && (c < L'0' || c > L'9'))
            return false;

         if(c == L'.')
            found_decimal = true;

         for(size_t i = 1; i < len; i++)
         {
            c = val.charAt(i);
            if(c == L'.')
            {
               if(found_decimal || found_E)
                  return false;
               else
                  found_decimal = true;
            }
            else if(c == L'E' || c == L'e')
            {
               if(found_E)
                  return false;
               else
                  found_E = true;
            }
            else if(c == L'+' || c == L'-')
            {
               if(found_sign || !found_E)
                  return false;
               else
               {
                  //sign can only follow e directly
                  wchar_t prevC = val.charAt(i - 1);
                  if(prevC != L'e' && prevC != L'E')
                     return false;
                  else                           
                     found_sign = true;
               }
            }
            else
            {
               cVal = static_cast<int>(c);
               if(cVal < L'0' || cVal > L'9')
                  return false;
            }
         }
         return true;
      }
   };
};

/* Csi.Expression.ExpressionHandler.cpp

   Copyright (C) 2002, 2020 Campbell Scientific, Inc.

   Written by: Tyler Mecham
   Date Begun: 9 January 2002
   Last Change: Wednesday 15 April 2020
   Last Commit: $Date: 2020-04-15 11:51:44 -0600 (Wed, 15 Apr 2020) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header

#include "Csi.Expression.ExpressionHandler.h"
#include "Csi.Expression.TokenTypes.h"
#include "Cora.Broker.ValueTypes.h"
#include "StrAsc.h"
#include "trace.h"
#include <algorithm>


namespace Csi
{
   namespace Expression
   {
      ExpressionHandler::ExpressionHandler(TokenFactory *token_factory_):
         token_factory(token_factory_),
         owns_factory(false),
         start_option_set(false),
         start_option(request_type::start_at_newest),
         order_option(request_type::order_real_time),
         file_mark_no(0),
         record_no(0),
         backfill_interval(0),
         start_record_offset(0),
         report_offset(0)
      {
         if(token_factory == 0)
         {
            token_factory = new TokenFactory;
            owns_factory = true;
         }
      } // constructor


      ExpressionHandler::~ExpressionHandler()
      {
         if(owns_factory)
            delete token_factory;
      } // destructor


      void ExpressionHandler::format_postfix(std::ostream &out) const
      {
         for(token_stack_type::const_iterator ti = postfix_stack.begin();
             ti != postfix_stack.end();
             ++ti)
         {
            (*ti)->format(out);
            out << " ";
         }
      } // format_postfix


      void ExpressionHandler::make_string_tokens(
         string_tokens_type &strings,
         StrUni const &expression)
      {
         enum state_type {
            state_between_tokens,
            state_in_name,
            state_quoted,
            state_after_number,
            state_after_decimal,
            state_after_exp,
            state_after_exp_sign,
            state_after_amp,
            state_after_amp_hex,
            state_after_amp_bin,
            state_after_lt,
            state_after_gt,
            state_after_dollar,
            state_in_string
         } state = state_between_tokens;
         StrUni cur_word;
         StrUni const operators(L"+-*/()^,=<>;");
         size_t i;
         for(i = 0; i < expression.length(); ++i)
         {
            wchar_t ch = expression[i];
            wchar_t ch_str[] = { ch, 0 };
            switch(state)
            {
            case state_between_tokens:
               if(ch >= L'0' && ch <= L'9')
               {
                  cur_word.append(ch);
                  state = state_after_number;
               }
               else if(ch == L'.')
               {
                  cur_word.append(ch);
                  state = state_after_decimal;
               }
               else if(ch == L'&')
               {
                  cur_word.append(ch);
                  state = state_after_amp;
               }
               else if(ch == L'>')
               {
                  cur_word.append(ch);
                  state = state_after_gt;
               }
               else if(ch == L'<')
               {
                  cur_word.append(ch);
                  state = state_after_lt;
               }
               else if(operators.find(ch_str) < operators.length())
               {
                  if(cur_word.length())
                     strings.push_back(std::make_pair(cur_word, i - cur_word.length()));
                  cur_word.cut(0);
                  cur_word.append(ch);
                  strings.push_back(std::make_pair(cur_word, i));
                  cur_word.cut(0); 
               }
               else if(ch == L'\"')
               {
                  cur_word.append(ch);
                  state = state_quoted;
               }
               else if(ch == L'$')
               {
                  if(cur_word.length())
                     strings.push_back(std::make_pair(cur_word, i - cur_word.length()));
                  cur_word.cut(0);
                  state = state_after_dollar;
                  cur_word.append(ch);
               } 
               else if(!iswspace(ch))
               {
                  cur_word.append(ch);
                  state = state_in_name;
               }
               break;
               
            case state_in_name:
               if(iswspace(ch) || operators.find(ch_str) < operators.length())
               {
                  strings.push_back(std::make_pair(cur_word, i - cur_word.length()));
                  cur_word.cut(0);
                  if(!iswspace(ch))
                     --i;
                  state = state_between_tokens;
               }
               else if(ch == L'\"')
               {
                  cur_word.append(ch);
                  state = state_quoted;
               }
               else
               {
                  cur_word.append(ch);
                  break;
               }
               break;
               
            case state_quoted:
               if(ch == L'\"')
               {
                  if(cur_word.length())
                  {
                     cur_word.append(ch);
                     state = state_in_name;
                  }
                  else
                     state = state_between_tokens;
               }
               else
                  cur_word.append(ch);
               break;
               
            case state_after_number:
               if(ch >= L'0' && ch <= L'9')
                  cur_word.append(ch);
               else if(ch == L'.')
               {
                  cur_word.append(ch);
                  state = state_after_decimal;
               }
               else if(ch == L'e' || ch == L'E')
               {
                  cur_word.append(ch);
                  state = state_after_exp; 
               }
               else if(operators.find(ch_str) < operators.length() || iswspace(ch))
               {
                  strings.push_back(std::make_pair(cur_word, i - cur_word.length()));
                  cur_word.cut(0);
                  if(!iswspace(ch))
                     --i;
                  state = state_between_tokens;
               }
               else
                  throw ExcParseError(i, "Unexpected character in number constant");
               break;
               
            case state_after_decimal:
               if(ch >= L'0' && ch <= L'9')
                  cur_word.append(ch);
               else if(ch == 'e' || ch == 'E')
               {
                  cur_word.append(ch);
                  state = state_after_exp;
               }
               else if(operators.find(ch_str) < operators.length() || iswspace(ch))
               {
                  strings.push_back(std::make_pair(cur_word, i - cur_word.length()));
                  cur_word.cut(0);
                  if(!isspace(ch))
                     --i;
                  state = state_between_tokens;
               }
               else
                  throw ExcParseError(i, "Unexpected character in number constant");
               break;
               
            case state_after_exp:
               if(ch == L'+' || ch == L'-' || (ch >= L'0' && ch <= L'9'))
               {
                  cur_word.append(ch);
                  state = state_after_exp_sign;
               }
               else
                  throw ExcParseError(i, "Unexpected character in number constant");
               break;
               
            case state_after_exp_sign:
               if(ch >= L'0' && ch <= L'9')
                  cur_word.append(ch);
               else if(iswspace(ch) || operators.find(ch_str) < operators.length())
               {
                  strings.push_back(std::make_pair(cur_word, i - cur_word.length()));
                  cur_word.cut(0);
                  if(!isspace(ch))
                     --i;
                  state = state_between_tokens;
               }
               else
                  throw ExcParseError(i, "Unexpected character in number constant");
               break;
               
            case state_after_amp:
               if(ch == L'h' || ch == L'H')
               {
                  cur_word.append(ch);
                  state = state_after_amp_hex;
               }
               else if(ch == L'b' || ch == L'B')
               {
                  cur_word.append(ch);
                  state = state_after_amp_bin;
               }
               else
                  throw ExcParseError(i, "Invalid ampersand expression");
               break;
               
            case state_after_amp_hex:
               if((ch >= L'0' && ch <= L'9') ||
                  (ch >= L'a' && ch <= L'f') ||
                  (ch >= L'A' && ch <= L'F'))
                  cur_word.append(ch);
               else if(iswspace(ch) || operators.find(ch_str) < operators.length())
               {
                  strings.push_back(std::make_pair(cur_word, i - cur_word.length()));
                  cur_word.cut(0);
                  if(!isspace(ch))
                     --i;
                  state = state_between_tokens;
               }
               else
                  throw ExcParseError(i, "Invalid hexadecimal sequence");
               break;
               
            case state_after_amp_bin:
               if(ch == L'0' || ch == L'1')
                  cur_word.append(ch);
               else if(iswspace(ch) || operators.find(ch_str) < operators.length())
               {
                  strings.push_back(std::make_pair(cur_word, i - cur_word.length()));
                  cur_word.cut(0);
                  if(!iswspace(ch))
                     --i;
                  state = state_between_tokens;
               }
               else
                  throw ExcParseError(i, "Invalid binary sequence");
               break;
               
            case state_after_lt:
               if(ch == L'>' || ch == L'=')
                  cur_word.append(ch);
               else
                  --i;
               strings.push_back(std::make_pair(cur_word, i - cur_word.length()));
               cur_word.cut(0);
               state = state_between_tokens;
               break;
               
            case state_after_gt:
               if(ch == L'=')
                  cur_word.append(ch);
               else
                  --i;
               strings.push_back(std::make_pair(cur_word, i - cur_word.length()));
               cur_word.cut(0);
               state = state_between_tokens;
               break;
               
            case state_after_dollar:
               if(ch == L'\"')
               {
                  cur_word.append(ch);
                  state = state_in_string;
               }
               else
                  throw ExcParseError(i, "Double quotes expected after an unquoted dollar sign");
               break;
               
            case state_in_string:
               cur_word.append(ch);
               if(ch == L'\"')
               {
                  strings.push_back(std::make_pair(cur_word, i - cur_word.length() + 1));
                  cur_word.cut(0);
                  state = state_between_tokens;
               }
               break;
            }
         }
         if(cur_word.length())
            strings.push_back(std::make_pair(cur_word, i - cur_word.length()));
         if(state == state_quoted)
            throw ExcParseError(i, "Unbalanced quotes in expression");
      } // make_string_tokens


      void ExpressionHandler::make_tokens(
         parsed_tokens_type &tokens, string_tokens_type const &string_tokens)
      {
         token_handle prev_token, cur_token;
         tokens.clear();
         variables.clear();
         for(string_tokens_type::const_iterator si = string_tokens.begin();
             si != string_tokens.end();
             ++si)
         {
            string_token_type const &current_str(*si);
            variables_type::iterator vi(variables.find(current_str.first));
            if(vi != variables.end())
            {
               cur_token = vi->second.get_handle();
               tokens.push_back(parsed_token_type(cur_token, current_str));
            }
            else
            {
               cur_token = token_factory->make_token(
                  prev_token, current_str.first, current_str.second);
               if(cur_token != 0)
               {
                  if(cur_token->is_variable())
                     variables[current_str.first] = cur_token;
                  tokens.push_back(parsed_token_type(cur_token, current_str));
               }
               else
                  throw ExcParseError(current_str.second, "unrecognised token name");
            }
            prev_token = cur_token;
         }
      } // make_tokens
      
         
      void ExpressionHandler::tokenise(StrUni const &expression)
      {
         // we will call an auxiliary function to break the string into a sequence of string tokens
         string_tokens_type string_list;
         parsed_tokens_type tokens;
         make_string_tokens(string_list, expression);
         make_tokens(tokens, string_list);
         infix_to_postfix_convert(tokens);
      } // tokenise


      void ExpressionHandler::assign_request_variables(
         Cora::Broker::Record const &record, request_type &request)
      {
         using namespace Cora::Broker;
         if(request.get_begin_index() < request.get_end_index() &&
            request.get_begin_index() < record.size() &&
            request.get_end_index() - 1 < record.size())
         {
            Record::const_iterator vi(record.begin() + request.get_begin_index());
            Record::value_type const &value(*vi);
            iterator ei(find(request.get_uri()));
            if(ei != end())
            {
               variable_handle &variable(ei->second);
               assign_request_variable(variable, value, record.get_stamp());
            }
         }
      } // assign_request_variables


      void ExpressionHandler::assign_request_variable(
         variable_handle &variable,
         Cora::Broker::Record::value_type const &value,
         LgrDate const &stamp)
      {
         using namespace Cora::Broker;
         CsiDbTypeCode value_type = value->get_type();
         if(value_type == CsiAscii || value_type == CsiAsciiZ)
         {
            StrBin temp(value->get_pointer(), value->get_pointer_len());
            temp.append(0);
            variable->set_val(temp.getContents(), stamp);
         }
         else if(value_type == CsiInt8 || value_type == CsiInt8Lsf)
         {
            Csi::PolySharedPtr<Value, ValueTypes::VInt8> temp(value);
            variable->set_val(temp->get_value(), stamp);
         }
         else if(value_type == CsiLgrDate ||
                 value_type == CsiLgrDateLsf ||
                 value_type == CsiNSec ||
                 value_type == CsiNSecLsf ||
                 value_type == CsiSec ||
                 value_type == CsiUSec)
         {
            Csi::PolySharedPtr<Value, ValueTypes::VStamp> temp(value);
            variable->set_val_date(temp->get_value(), stamp);
         }
         else
         {
            double temp(0);
            if(value->to_float(temp))
               variable->set_val(temp, stamp);
         }
      } // assign_request_variable


      void ExpressionHandler::annotate_source(StrUni &source)
      {
         OStrAscStream temp;
         StrUni expanded;
         for(iterator vi = begin(); vi != end(); ++vi)
         {
            temp.str("");
            vi->second->format(temp);
            expanded = temp.str();
            source.replace(vi->first.c_str(), expanded.c_str());
         }
      } // annotate_source
      

      void ExpressionHandler::infix_to_postfix_convert(parsed_tokens_type &token_list)
      {
         parsed_tokens_type op_stack;
         parsed_tokens_type::iterator it(token_list.begin());
         parsed_token_type popped_token;
         postfix_stack.clear();
         while(it != token_list.end())
         {
            parsed_token_type current(*it);
            if(current.token->is_comma()) //COMMA
            {
               if(!op_stack.empty())
               {
                  //need to pop off op stack everything with lesser priority
                  popped_token = op_stack.back();
                  while(!op_stack.empty() && 
                        popped_token.token->get_priority() < current.token->get_priority() &&
                        !popped_token.token->is_lparen())
                  {
                     op_stack.pop_back();
                     postfix_stack.push_back(popped_token.token);
                     if(!op_stack.empty())
                        popped_token = op_stack.back();
                  }
                  if(!popped_token.token->is_lparen())
                     throw ExcParseError(
                        popped_token.begin_pos,
                        "Commas must appear within parentheses");

                  // we need to increment the argument, if any, of the token in front of the left
                  // parenthese 
                  parsed_token_type paren_token(popped_token);
                  op_stack.pop_back();
                  if(!op_stack.empty())
                  {
                     popped_token = op_stack.back();
                     popped_token.token->increment_args_count();
                  }
                  op_stack.push_back(paren_token);
               }
            }
            else if(current.token->is_rparen()) //RIGHT PARENTHESIS
            {
               if(!op_stack.empty())
               {
                  // need to pop off op stack and put on postfix stack until we find matching lparen
                  popped_token = op_stack.back();
                  while(!op_stack.empty() && !popped_token.token->is_lparen())
                  {
                     op_stack.pop_back();
                     postfix_stack.push_back(popped_token.token);
                     if(!op_stack.empty())
                        popped_token = op_stack.back();
                  }
                  if(!popped_token.token->is_lparen())
                     throw ExcParseError(popped_token.begin_pos, "Mismatched parenthesis");
                  else
                     op_stack.pop_back();
               }
            }
            else if(current.token->is_lparen()) //LEFT PARENTHESIS
            {
               //Always put left parens on op stack
               if(!op_stack.empty())
                  op_stack.back().token->clear_args_count();
               op_stack.push_back(current);
            }
            else if(current.token->is_operator()) //OPERATOR
            {
               if(op_stack.empty())
                  op_stack.push_back(current);
               else
               {
                  bool cont = true;
                  while(cont)
                  {
                     if(op_stack.empty())
                        cont = false;
                     else
                     {
                        popped_token = op_stack.back();
                        if(popped_token.token->is_lparen() ||
                           popped_token.token->get_priority() < current.token->get_priority())
                           cont = false;
                        else if(popped_token.token->get_priority() == current.token->get_priority() &&
                                current.token->get_priority() >= prec_max_operator)
                           cont = false;
                     }
                     if(cont)
                     {
                        popped_token = op_stack.back();
                        op_stack.pop_back();
                        postfix_stack.push_back(popped_token.token);
                     }
                  }
                  op_stack.push_back(*it);
               }
            }
            else if(current.token->is_semi_colon())
            {
               while(!op_stack.empty())
               {
                  popped_token = op_stack.back();
                  if(!popped_token.token->is_lparen())
                     postfix_stack.push_back(popped_token.token);
                  op_stack.pop_back();
               }
            }
            else //OPERAND
               postfix_stack.push_back(current.token);
            ++it;
         }
         
         // We have finished going through the original stack, so let's pop off any remaining
         // operators on the op stack
         while(!op_stack.empty())
         {
            popped_token = op_stack.back();
            if(popped_token.token->is_lparen() || popped_token.token->is_rparen())
               throw ExcParseError(popped_token.begin_pos, "Mismatched parenthesis");
            postfix_stack.push_back(popped_token.token);
            op_stack.pop_back();
         }
      } // infix_to_postfix_convert

        
      ExpressionHandler::operand_handle ExpressionHandler::eval()
      {
         token_stack_type result_stack;
         bool aborted = false;
         if(postfix_stack.empty())
            throw ExcInvalidState();

         // Let's evaluate the postfix stack
         for(token_stack_type::iterator it = postfix_stack.begin();
             it != postfix_stack.end();
             ++it)
         {
            token_handle &th = *it;
            if(th->is_operator())
            {
               th->eval(result_stack, this);
               if(th->abort_after_eval())
               {
                  if(!result_stack.empty())
                     throw ExcInvalidState();
                  postfix_stack.erase(postfix_stack.begin(), ++it);
                  aborted = true;
                  break;
               }
            }
            else
               result_stack.push_back(th);
         }

         //At this point only the result should be on the stack
         operand_handle rtn;
         if(result_stack.size() != 1 && !aborted)
            throw ExcInvalidState();
         else if(!result_stack.empty())
            rtn = result_stack.back();
         return rtn;
      } // eval


      namespace
      {
         ////////////////////////////////////////////////////////////
         // predicate token_will_abort
         ////////////////////////////////////////////////////////////
         struct token_will_abort
         {
            bool operator ()(LightSharedPtr<Token> &token)
            { return token->abort_after_eval(); }
         };
      };

      
      bool ExpressionHandler::has_aborting_tokens()
      {
         bool rtn = false;
         token_stack_type::iterator ti = std::find_if(
            postfix_stack.begin(), postfix_stack.end(), token_will_abort());
         if(ti != postfix_stack.end())
            rtn = true;
         return rtn;
      } // has_aborting_tokens


      void ExpressionHandler::set_order_option(int8 order_option_)
      {
         order_option_type option(static_cast<order_option_type>(order_option_));
         if(request_type::is_valid_order_option(option))
         {
            order_option = option;
            start_option_set = true;
         }
         else
            throw std::invalid_argument("Invalid order option specified");
      } // set_order_option

     
      void ExpressionHandler::configure_request(request_type &request)
      {
         if(start_option_set && !request.is_frozen())
         {
            // we can now create and configure the request
            switch(start_option)
            {
            case request_type::start_at_record:
               request.set_start_at_record(file_mark_no, record_no);
               break;
               
            case request_type::start_at_time:
               request.set_start_at_time(start_time);
               break;
            
            case request_type::start_at_newest:
               request.set_start_at_newest();
               break;
               
            case request_type::start_after_newest:
               request.set_start_after_newest();
               break;
               
            case request_type::start_relative_to_newest:
               request.set_start_relative_to_newest(-backfill_interval);
               break;
            
            case request_type::start_at_offset_from_newest:
               request.set_start_at_offset_from_newest(start_record_offset);
               break;
            }
            request.set_report_offset(report_offset);
            request.set_order_option(order_option);
            request.freeze();
         }
      } // configure_request
   };
};

/* Csi.Expression.TokenTypes.cpp

   Copyright (C) 2007, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 09 February 2007
   Last Change: Thursday 19 November 2020
   Last Commit: $Date: 2020-11-19 14:49:16 -0600 (Thu, 19 Nov 2020) $
   Last Changed by: $Author: jon $

*/

#pragma warning(disable: 4996)
#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Expression.TokenTypes.h"
#include "Csi.Expression.ExpressionHandler.h"
#include "Csi.StrAscStream.h"
#include "Csi.StringLoader.h"
#include "Csi.BuffStream.h"
#include "Csi.MaxMin.h"
#include "Csi.Json.h"
#include "boost/format.hpp"
#ifdef min
#undef min
#undef max
#endif


namespace Csi
{
   namespace Expression
   {
      void Token::format_js(std::ostream &out)
      {
         OStrAscStream message;
         message << "unable to format \"";
         format(message);
         message << "\" as Javascript";
         throw std::invalid_argument(message.c_str());
      } // format_js

      
      Constant::Constant(StrAsc const &val, bool variables_quoted)
      {
         if(val.length())
         {
            if(val[0] == '&' && val.length() > 2)
            {
               if(val[1] == 'h' || val[1] == 'H')
               {
                  value.type = value_int;
                  value.vint = strtoul(val.c_str() + 2,0,16);
               }
               else if(val[1] == 'b' || val[1] == 'B')
               {
                  value.type = value_int;
                  value.vint = strtoul(val.c_str() + 2,0,2);
               }
            }
            else if(variables_quoted &&
                    val[0] == '$' &&
                    val.length() >= 3 &&
                    val[1] == '\"' &&
                    val.last() == '\"')
            {
               value.type = value_string;
               val.sub(value.vstring,2,val.length() - 3);
            }
            else if(!variables_quoted && val.first() == '\"' && val.last() == '\"')
            {
               value.type = value_string;
               val.sub(value.vstring, 1, val.length() - 2);
            }
            else
            {
               // If the value is encoded without decimal points, we will assume that the string
               // represents an integer.
               size_t decimal_pos = val.find(".");
               size_t exponent_pos = val.find("E");
               if(decimal_pos < val.length() || exponent_pos < val.length())
               {
                  value.type = value_double;
                  value.vdouble = csiStringToFloat(val.c_str(), StringLoader::make_locale(0), true);
               }
               else
               {
                  value.type = value_int;
                  value.vint = strtol(val.c_str(), 0, 10);
               }
            }
         }
      } // constructor


      Constant::Constant(StrUni const &val, bool variables_quoted)
      {
         if(val.length())
         {
            if(val[0] == L'&' && val.length() > 2)
            {
               if(val[1] == L'h' || val[1] == L'H')
               {
                  value.type = value_int;
                  value.vint = wcstoul(val.c_str() + 2,0,16);
               }
               else if(val[1] == L'b' || val[1] == L'B')
               {
                  value.type = value_int;
                  value.vint = wcstoul(val.c_str() + 2,0,2);
               }
            }
            else if(variables_quoted &&
                    val[0] == L'$' &&
                    val.length() >= 3 &&
                    val[1] == L'\"' &&
                    val.last() == L'\"')
            {
               StrUni temp;
               val.sub(temp, 2, val.length() - 3);
               value.type = value_string;
               value.vstring = temp.to_utf8();
            }
            else if(!variables_quoted && val.first() == L'\"' && val.last() == L'\"')
            {
               StrUni temp;
               val.sub(temp, 1, val.length() - 2);
               value.type = value_string;
               value.vstring = temp.to_utf8();
            }
            else
            {
               // if there is a decimal point, we will interpret the value as a floating point.
               size_t decimal_pos(val.find(L"."));
               size_t exp_pos(val.find(L"E"));
               if(decimal_pos < val.length() || exp_pos < val.length())
               {
                  value.type = value_double;
                  value.vdouble = csiStringToFloat(val.c_str(), StringLoader::make_locale(0), true);
               }
               else
               {
                  value.type = value_int;
                  value.vint = wcstol(val.c_str(), 0, 10);
               }
            }
         }
      } // StrUni constructor


      void Constant::format_js(std::ostream &out)
      {
         out << "new CsiConstant(";
         if(value.type == value_double)
         {
            if(is_signalling_nan(value.vdouble))
                  out << "Number.NaN";
            else if(is_pos_inf(value.vdouble))
               out << "Number.POSITIVE_INFINITY";
            else if(is_neg_inf(value.vdouble))
               out << "Number.NEGATIVE_INFINITY";
            else
            {
               double intpart, fracpart;
               fracpart = modf(value.vdouble, &intpart);
               if(fracpart == 0)
                  out << static_cast<int8>(intpart);
               else
                  out << value.vdouble;
            }
         }
         else if(value.type == value_int)
         {
            out << value.vint;
         }
         else //(value.type == value_string)
         {
            out << "\"" << Csi::Json::format_string(value.vstring, true) << "\"";
         }
         out << ")";
      } // format_js
      

      ////////////////////////////////////////////////////////////
      // class Addition definitions
      ////////////////////////////////////////////////////////////
      void Addition::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         if( stack.size() < 2 )
            throw MsgExcept("Unable to perform addition, stack size must be >= 2 to perform binary operations");
         
         token_handle val2 = stack.back();
         stack.pop_back();
         token_handle val1 = stack.back();
         stack.pop_back();
         
         if( val1->is_operand() && val2->is_operand() )
         {
            Operand *op1 = static_cast<Operand*>(val1.get_rep());
            Operand *op2 = static_cast<Operand*>(val2.get_rep());
            LightPolySharedPtr<Token, Constant> result(new Constant);

            if(op1->get_type() == value_string || op2->get_type() == value_string)
            {
               result->set_val(
                  op1->get_val_str() + op2->get_val_str(),
                  csimax(op1->get_timestamp(), op2->get_timestamp()));
            }
            else if(op1->get_type() == value_double || op2->get_type() == value_double)
            {
               result->set_val(
                  op1->get_val() + op2->get_val(),
                  csimax(op1->get_timestamp(), op2->get_timestamp()));
            }
            else if(op1->get_type() == value_date || op2->get_type() == value_date)
            {
               result->set_val_date(
                  op1->get_val_date() + op2->get_val_date(),
                  csimax(op1->get_timestamp(), op2->get_timestamp()));
            }
            else if(op1->get_type() == value_int || op2->get_type() == value_int)
            {
               result->set_val(
                  op1->get_val_int() + op2->get_val_int(),
                  csimax(op1->get_timestamp(), op2->get_timestamp()));
            }
            else
               throw std::invalid_argument("unsupported addition argument types");
            stack.push_back(result.get_handle());
         }
         else
            throw MsgExcept("Expression syntax error - unable to perform addition");
      } // eval


      ////////////////////////////////////////////////////////////
      // class Subtraction definitions
      ////////////////////////////////////////////////////////////
      void Subtraction::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         if( stack.size() < 2 )
            throw MsgExcept("Unable to perform subtraction, stack size must be >= 2 to perform binary operations");
         token_handle val2 = stack.back();
         stack.pop_back();
         token_handle val1 = stack.back();
         stack.pop_back();
         
         if( val1->is_operand() && val2->is_operand() )
         {
            Operand *op1 = static_cast<Operand*>(val1.get_rep());
            Operand *op2 = static_cast<Operand*>(val2.get_rep());
            LightPolySharedPtr<Token, Constant> result(new Constant);

            if(op1->get_type() == value_double || op2->get_type() == value_double)
            {
               result->set_val(
                  op1->get_val() - op2->get_val(),
                  csimax(op1->get_timestamp(), op2->get_timestamp()));
            }
            else if(op1->get_type() == value_date || op2->get_type() == value_date)
            {
               result->set_val_date(
                  op1->get_val_date() - op2->get_val_date(),
                  csimax(op1->get_timestamp(), op2->get_timestamp()));
            }
            else if(op1->get_type() == value_int || op2->get_type() == value_int)
            {
               result->set_val(
                  op1->get_val_int() - op2->get_val_int(),
                  csimax(op1->get_timestamp(), op2->get_timestamp()));
            }
            else
               throw std::invalid_argument("invalid subtraction operand types");
            stack.push_back(result.get_handle());
         }
         else
            throw MsgExcept("Expression syntax error - unable to perform subtraction");
      } // eval


      void Multiplication::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         if( stack.size() < 2 )
            throw MsgExcept("Unable to perform multiplication, stack size must be >= 2 to perform binary operations");
         token_handle val2 = stack.back();
         stack.pop_back();
         token_handle val1 = stack.back();
         stack.pop_back();
            
         if(val1->is_operand() && val2->is_operand())
         {
            Operand *op1 = static_cast<Operand*>(val1.get_rep());
            Operand *op2 = static_cast<Operand*>(val2.get_rep());
            LightPolySharedPtr<Token, Constant> result(new Constant);

            if(op1->get_type() == value_double || op2->get_type() == value_double)
            {
               result->set_val(
                  op1->get_val() * op2->get_val(),
                  csimax(op1->get_timestamp(), op2->get_timestamp()));
            }
            else if(op1->get_type() == value_date || op2->get_type() == value_date)
            {
               result->set_val_date(
                  op1->get_val_date() * op2->get_val_date(),
                  csimax(op1->get_timestamp(), op2->get_timestamp()));
            }
            else if(op1->get_type() == value_int || op2->get_type() == value_int)
            {
               result->set_val(
                  op1->get_val_int() * op2->get_val_int(),
                  csimax(op1->get_timestamp(), op2->get_timestamp()));
            }
            else if(op1->get_type() == value_string || op2->get_type() == value_string)
            {
               result->set_val(
                  op1->get_val() * op2->get_val(),
                  csimax(op1->get_timestamp(), op2->get_timestamp()));
            }
            else
               throw std::invalid_argument("invalid multiply operand types");
            stack.push_back(result.get_handle());
         }
         else
            throw MsgExcept("Expression syntax error - unable to perform multiplication");
      } // eval


      void Division::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         if( stack.size() < 2 )
            throw MsgExcept("Unable to perform division, stack size must be >= 2 to perform binary operations");
         token_handle val1 = stack.back();
         stack.pop_back();
         token_handle val2 = stack.back();
         stack.pop_back();

         if(val1->is_operand() && val2->is_operand())
         {
            Operand *op1 = static_cast<Operand*>(val2.get_rep());
            Operand *op2 = static_cast<Operand*>(val1.get_rep());
            LightPolySharedPtr<Token, Constant> result(new Constant);

            if(op1->get_type() == value_double || op2->get_type() == value_double)
            {
               if(op2->get_val() != 0.0)
               {
                  result->set_val(
                     op1->get_val() / op2->get_val(),
                     csimax(op1->get_timestamp(), op2->get_timestamp()));
               }
               else
               {
                  double rtn = std::numeric_limits<double>::infinity();
                  if(op1->get_val() == 0)
                     rtn = std::numeric_limits<double>::quiet_NaN();
                  result->set_val(
                     rtn, 
                     csimax(op1->get_timestamp(), op2->get_timestamp()));
               }
            }
            else if(op1->get_type() == value_date || op2->get_type() == value_date)
            {
               if(op2->get_val_int() != 0)
               {
                  result->set_val(
                     op1->get_val_date() / op2->get_val_date(),
                     csimax(op1->get_timestamp(), op2->get_timestamp()));
               }
               else
               {
                  double rtn = std::numeric_limits<double>::infinity();
                  if(op1->get_val_int() == 0)
                     rtn = std::numeric_limits<double>::quiet_NaN();
                  result->set_val(
                     rtn, 
                     csimax(op1->get_timestamp(), op2->get_timestamp()));
               }
            }
            else if(op1->get_type() == value_int || op2->get_type() == value_int)
            {
               if(op2->get_val_int() != 0)
               {
                  result->set_val(
                     op1->get_val() / op2->get_val(),
                     csimax(op1->get_timestamp(), op2->get_timestamp()));
               }
               else
               {
                  double rtn = std::numeric_limits<double>::infinity();
                  if(op1->get_val_int() == 0)
                     rtn = std::numeric_limits<double>::quiet_NaN();
                  result->set_val(
                     rtn, 
                     csimax(op1->get_timestamp(), op2->get_timestamp()));
               }
            }
            else if(op1->get_type() == value_string || op2->get_type() == value_string)
            {
               if(op2->get_val() != 0.0)
               {
                  result->set_val(
                     op1->get_val() / op2->get_val(),
                     csimax(op1->get_timestamp(), op2->get_timestamp()));
               }
               else
               {
                  double rtn = std::numeric_limits<double>::infinity();
                  if(op1->get_val() == 0)
                     rtn = std::numeric_limits<double>::quiet_NaN();
                  result->set_val(
                     rtn, 
                     csimax(op1->get_timestamp(), op2->get_timestamp()));
               }
            }
            else
               throw std::invalid_argument("invalid divide operand types");
            stack.push_back(result.get_handle());
         }
         else
            throw MsgExcept("Expression syntax error - unable to perform division");
      } // eval


      void Exponentiation::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         if( stack.size() < 2 )
            throw MsgExcept("Unable to perform Exponentiation, stack size must be >= 2 to perform binary operations");
         LightPolySharedPtr<Token, Operand> op2(stack.back()); stack.pop_back();
         LightPolySharedPtr<Token, Operand> op1(stack.back()); stack.pop_back();
         LightPolySharedPtr<Token, Operand> rtn(new Operand);
         rtn->set_val(
            pow(op1->get_val(), op2->get_val()),
            csimax(op1->get_timestamp(), op2->get_timestamp()));
         stack.push_back(rtn.get_handle());
      } // eval


      void PwrFunction::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         if( stack.size() < 2 )
            throw MsgExcept("Unable to perform Exponentiation, stack size must be >= 2 to perform binary operations");
         LightPolySharedPtr<Token, Operand> op2(stack.back()); stack.pop_back();
         LightPolySharedPtr<Token, Operand> op1(stack.back()); stack.pop_back();
         LightPolySharedPtr<Token, Operand> rtn(new Operand);
         rtn->set_val(
            pow(op1->get_val(), op2->get_val()),
            csimax(op1->get_timestamp(), op2->get_timestamp()));
         stack.push_back(rtn.get_handle());
      } // eval


      void Negation::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         if( stack.size() < 1 )
            throw MsgExcept("Unable to perform negation, stack size must be >= 1 to perform binary operations");
         token_handle val1 = stack.back();
         stack.pop_back();

         if(val1->is_operand())
         {
            Operand *op1 = static_cast<Operand*>(val1.get_rep());
            LightPolySharedPtr<Token, Operand> result(new Constant);

            if(op1->get_type() == value_double)
               result->set_val(-1.0 * op1->get_val(),op1->get_timestamp());
            else if(op1->get_type() == value_int)
               result->set_val(-1 * op1->get_val_int(), op1->get_timestamp());
            else if(op1->get_type() == value_date)
               result->set_val_date(-1 * op1->get_val_date(), op1->get_timestamp());
            else
               throw MsgExcept("Invalid negation operand");
            stack.push_back(result.get_handle());
         }
         else
            throw MsgExcept("Expression syntax error - unable to perform negation");
      } // eval


      void CsgnFunction::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         if( stack.size() < 1 )
            throw MsgExcept("Unable to perform negation, stack size must be >= 1 to perform binary operations");
         token_handle val1 = stack.back();
         stack.pop_back();

         if( val1->is_operand()  )
         {
            Operand *op1 = static_cast<Operand*>(val1.get_rep());
            LightPolySharedPtr<Token, Operand> result(new Constant);

            if(op1->get_type() == value_double)
               result->set_val(-1.0 * op1->get_val(),op1->get_timestamp());
            else if(op1->get_type() == value_int)
               result->set_val(-1 * op1->get_val_int(), op1->get_timestamp());
            else if(op1->get_type() == value_date)
               result->set_val_date(-1 * op1->get_val_date(), op1->get_timestamp());
            else
               throw MsgExcept("Invalid negation operand");
            stack.push_back(result.get_handle());
         }
         else
            throw MsgExcept("Expression syntax error - unable to perform negation");
      } // eval


      void ToDate::eval(token_stack_type &stack, ExpressionHandler *expr)
      {
         token_handle tk1;
         if(stack.size() < 1)
            throw std::invalid_argument("ToDate() requires one parameter");
         tk1 = stack.back();
         stack.pop_back();
         if(tk1->is_operand())
         {
            LightPolySharedPtr<Token, Operand> op1(tk1);
            LightPolySharedPtr<Token, Operand> rtn(new Constant);
            rtn->set_val_date(op1->get_val_date(), op1->get_timestamp());
            stack.push_back(rtn.get_handle());
         }
         else
            throw std::invalid_argument("Expression syntax error - unable to perform ToDate");
      } // eval


      void ToFloat::eval(token_stack_type &stack, ExpressionHandler *expr)
      {
         token_handle tk1;
         if(stack.size() < 1)
            throw std::invalid_argument("ToFloat() requires one parameter");
         tk1 = stack.back();
         stack.pop_back();
         if(tk1->is_operand())
         {
            LightPolySharedPtr<Token, Operand> op1(tk1);
            LightPolySharedPtr<Token, Operand> rtn(new Constant);
            rtn->set_val(op1->get_val(), op1->get_timestamp());
            stack.push_back(rtn.get_handle());
         }
         else
            throw std::invalid_argument("Expression syntax error - unable to perform ToFloat");
      } // eval


      void ToInt::eval(token_stack_type &stack, ExpressionHandler *expr)
      {
         token_handle tk1;
         if(stack.size() < 1)
            throw std::invalid_argument("ToInt() requires one parameter");
         tk1 = stack.back();
         stack.pop_back();
         if(tk1->is_operand())
         {
            LightPolySharedPtr<Token, Operand> op1(tk1);
            LightPolySharedPtr<Token, Operand> rtn(new Constant);
            double val(op1->get_val());
            if(is_finite(val))
               rtn->set_val(static_cast<int8>(val), op1->get_timestamp());
            else
               rtn->set_val(static_cast<int8>(0), op1->get_timestamp());
            stack.push_back(rtn.get_handle());
         }
         else
            throw std::invalid_argument("Expression syntax error - unable to perform ToInt");
      } // eval


      void AbsoluteValue::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         if( stack.size() < 1 )
            throw MsgExcept("Unable to perform ABS, stack size must be >= 1 to perform unary operations");
         token_handle val1 = stack.back();
         stack.pop_back();

         if( val1->is_operand() )
         {
            Operand *op1 = static_cast<Operand*>(val1.get_rep());
            LightPolySharedPtr<Token, Operand> result(new Constant);

            if(op1->get_type() == value_double)
               result->set_val(fabs(op1->get_val()), op1->get_timestamp());
            else if(op1->get_type() == value_int || op1->get_type() == value_date)
            {
               int8 val = op1->get_val_int();
               if(val < 0)
                  val *= -1;
               result->set_val(val, op1->get_timestamp());
            }
            else
               throw MsgExcept("Incompatible abs operand type");
            stack.push_back(result.get_handle());
         }
         else
            throw MsgExcept("Expression syntax error - unable to perform ABS");
      } // eval


      void ArcCosine::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         if( stack.size() < 1 )
            throw MsgExcept("Unable to perform ACOS, stack size must be >= 1 to perform unary operations");
         token_handle val1 = stack.back();
         stack.pop_back();

         if( val1->is_operand() )
         {
            Operand *op1 = static_cast<Operand*>(val1.get_rep());
            if(op1->get_type() == value_string)
               throw MsgExcept("Incompatible acos operand");
            token_handle result(new Operand(acos(op1->get_val()), op1->get_timestamp()));
            stack.push_back(result);
         }
         else
            throw MsgExcept("Expression syntax error - unable to perform ACOS");
      } // eval


      void AndOperator::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         if( stack.size() < 2 )
            throw MsgExcept("Unable to perform AND, stack size must be >= 2 to perform binary operations");
         token_handle val1 = stack.back();
         stack.pop_back();
         token_handle val2 = stack.back();
         stack.pop_back();

         if( val1->is_operand() && val2->is_operand() )
         {
            Operand *op1 = static_cast<Operand*>(val1.get_rep());
            Operand *op2 = static_cast<Operand*>(val2.get_rep());
            LightPolySharedPtr<Token, Constant> result(new Constant);

            result->set_val(
               op1->get_val_int() & op2->get_val_int(),
               csimax(op1->get_timestamp(), op2->get_timestamp()));
            stack.push_back(result.get_handle());
         }
         else
            throw MsgExcept("Expression syntax error - unable to perform AND");
      } // eval


      void ArcSine::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         if( stack.size() < 1 )
            throw MsgExcept("Unable to perform ASIN, stack size must be >= 1 to perform unary operations");
         token_handle val1 = stack.back();
         stack.pop_back();

         if( val1->is_operand() )
         {
            Operand *op1 = static_cast<Operand*>(val1.get_rep());
            if(op1->get_type() == value_string)
               throw MsgExcept("Incompatible asin operand type");
            token_handle result(new Operand(asin(op1->get_val()), op1->get_timestamp()));
            stack.push_back(result);
         }
         else
            throw MsgExcept("Expression syntax error - unable to perform ASIN");
      } // eval


      void ArcTangent::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         if( stack.size() < 1 )
            throw MsgExcept("Unable to perform ATN, stack size must be >= 1 to perform binary operations");
         token_handle val1 = stack.back();
         stack.pop_back();

         if( val1->is_operand() )
         {
            Operand *op1 = static_cast<Operand*>(val1.get_rep());
            token_handle result(new Operand(atan(op1->get_val()), op1->get_timestamp()));
            stack.push_back(result);
         }
         else
            throw MsgExcept("Expression syntax error - unable to perform ATAN");
      } // eval

      
      void ArcTangent2::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         if( stack.size() < 2 )
            throw MsgExcept("Unable to perform ATN2, stack size must be >= 2 to perform binary operations");
         token_handle val1 = stack.back();
         stack.pop_back();
         token_handle val2 = stack.back();
         stack.pop_back();

         if( val1->is_operand() )
         {
            Operand *op1 = static_cast<Operand*>(val1.get_rep());
            Operand *op2 = static_cast<Operand*>(val2.get_rep());
            token_handle result(
               new Operand(
                  atan2(op2->get_val(),op1->get_val()),
                  csimax(op1->get_timestamp(), op2->get_timestamp())));
            stack.push_back(result);
         }
         else
            throw MsgExcept("Expression syntax error - unable to perform ATAN2");
      } // eval


      void Cosine::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         if( stack.size() < 1 )
            throw MsgExcept("Unable to perform COS, stack size must be >= 1 to perform unary operations");
         token_handle tk1 = stack.back(); stack.pop_back();
         LightPolySharedPtr<Token, Operand> op1(tk1);
         LightPolySharedPtr<Token, Operand> rtn(new Operand);
         rtn->set_val(cos(op1->get_val()), op1->get_timestamp());
         stack.push_back(rtn.get_handle());
      } // eval


      void HyperbolicCosine::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         if( stack.size() < 1 )
            throw MsgExcept("Unable to perform COSH, stack size must be >= 1 to perform unary operations");
         token_handle tk1 = stack.back(); stack.pop_back();
         LightPolySharedPtr<Token, Operand> op1(tk1);
         LightPolySharedPtr<Token, Operand> rtn(new Operand);

         rtn->set_val(cosh(op1->get_val()), op1->get_timestamp());
         stack.push_back(rtn.get_handle());
      } // eval


      void Equivalence::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         if(stack.size() < 2)
            throw MsgExcept("Unable to perform EQV, stack size must be >= 2 to perform binary operations");
         token_handle val1 = stack.back();
         stack.pop_back();
         token_handle val2 = stack.back();
         stack.pop_back();

         if( val1->is_operand() && val2->is_operand() )
         {
            Operand *op1 = static_cast<Operand*>(val1.get_rep());
            Operand *op2 = static_cast<Operand*>(val2.get_rep());
            LightPolySharedPtr<Token, Constant> result(new Constant);
            int8 a(op1->get_val_int());
            int8 b(op2->get_val_int());
            result->set_val(
               (a & b) | (~a & ~b),
               csimax(op1->get_timestamp(), op2->get_timestamp()));
            stack.push_back(result.get_handle());
         }
         else
            throw MsgExcept("Expression syntax error - unable to perform EQV");
      } // eval


      void EtoPower::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         if( stack.size() < 1 )
            throw MsgExcept("Unable to perform EXP, stack size must be >= 1 to perform unary operations");
         LightPolySharedPtr<Token, Operand> op1(stack.back()); stack.pop_back();
         LightPolySharedPtr<Token, Operand> rtn(new Operand);
         rtn->set_val(exp(op1->get_val()), op1->get_timestamp());
         stack.push_back(rtn.get_handle());
      } // eval


      void Fix::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         if( stack.size() < 1 )
            throw MsgExcept("Unable to perform FIX, stack size must be >= 1 to perform unary operations");
         LightPolySharedPtr<Token, Operand> op1(stack.back()); stack.pop_back();
         LightPolySharedPtr<Token, Operand> rtn(new Operand);
         rtn->set_val(op1->get_val_int(), op1->get_timestamp());
         stack.push_back(rtn.get_handle());
      } // eval


      void IsFinite::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         if( stack.size() < 1 )
            throw MsgExcept("Unable to perform IsFinite, stack size must be >= 1 to perform unary operations");
         token_handle val1 = stack.back();
         stack.pop_back();

         if( val1->is_operand() )
         {
            Operand *op1 = static_cast<Operand*>(val1.get_rep());
            LightPolySharedPtr<Token, Constant> result(new Constant);
            switch(op1->get_type())
            {
            case value_double:
            case value_int:
            case value_date:
               result->set_val(Csi::is_finite(op1->get_val()) ? -1.0 : 0.0, op1->get_timestamp());
               break;
                  
            case value_string:
               throw MsgExcept("Invalid IsFinite operand type");
               break;
            }
            stack.push_back(result.get_handle());
         }
         else
            throw MsgExcept("Expression syntax error - unable to perform FIX");
      } // eval


      void FractionalPart::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         if( stack.size() < 1 )
            throw MsgExcept("Unable to perform FRAC, stack size must be >= 1 to perform unary operations");
         token_handle val1 = stack.back();
         stack.pop_back();

         if( val1->is_operand() )
         {
            Operand *op1 = static_cast<Operand*>(val1.get_rep());
            LightPolySharedPtr<Token, Constant> result(new Constant);
            double ipart;
               
            switch(op1->get_type())
            {
            case value_double:
               result->set_val(modf(op1->get_val(),&ipart), op1->get_timestamp());
               break;
                  
            case value_int:
            case value_date:
               result->set_val(int8(0), op1->get_timestamp());
               break;
                  
            case value_string:
               throw MsgExcept("Invalid FRAC operand type");
               break;
            }
            stack.push_back(result.get_handle());
         }
         else
            throw MsgExcept("Expression syntax error - unable to perform FRAC");
      } // eval



      void IIF::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         if( stack.size() < 3 )
            throw MsgExcept("Unable to perform IIF, stack size must be >= 3 to perform tertiary operations");
         token_handle tk3(stack.back()); stack.pop_back();
         token_handle tk2(stack.back()); stack.pop_back();
         token_handle tk1(stack.back()); stack.pop_back();
         LightPolySharedPtr<Token, Operand> op1(tk1);
         LightPolySharedPtr<Token, Operand> op2(tk2);
         LightPolySharedPtr<Token, Operand> op3(tk3);
         LightPolySharedPtr<Token, Operand> rtn(new Operand);
         Csi::LgrDate rtn_time(
            csimax(
               op1->get_timestamp(),
               csimax(op2->get_timestamp(), op3->get_timestamp())));
         if(op1->get_val_int() != 0)
            rtn->set_val(op2->get_value(), op2->get_timestamp() != 0 ? op2->get_timestamp() : rtn_time);
         else
            rtn->set_val(op3->get_value(), op3->get_timestamp() != 0 ? op3->get_timestamp() : rtn_time);
         stack.push_back(rtn.get_handle());
      } // eval


      void Switch::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         // check the stack size
         if(stack.size() < args_count)
         {
            OStrAscStream temp;
            temp << "Switch() requires at least " << args_count << " arguments";
            throw std::invalid_argument(temp.c_str());
         }
         if(args_count % 2 == 0)
            throw std::invalid_argument("Switch(): missing default value");
         if(args_count < 3)
            throw std::invalid_argument("Switch() requires at least one predicate, value, and default value");

         // we now need to extract the default value as well as the list of alternatives
         token_handle default_value(stack.back()); stack.pop_back();
         typedef LightPolySharedPtr<Token, Operand> operand_handle;
         typedef std::pair<operand_handle, operand_handle> case_type;
         typedef std::list<case_type> cases_type;
         cases_type cases;
         for(uint4 i = 0; i < args_count - 1; i += 2)
         {
            operand_handle value(stack.back()); stack.pop_back();
            operand_handle pred(stack.back()); stack.pop_back();
            cases.push_front(case_type(pred, value));
         }

         // now that we have extracted all values from the stack, we can iterate the list of cases
         // to find the first predicate value that is true.
         operand_handle rtn;
         while(!cases.empty() && rtn == 0)
         {
            case_type candidate(cases.front());
            cases.pop_front();
            if(candidate.first->get_val_int() != 0)
               rtn = candidate.second;
         }
         if(rtn == 0)
            rtn = default_value;
         stack.push_back(rtn.get_handle());
      } // eval


      void Implication::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         if( stack.size() < 2 )
            throw MsgExcept("Unable to perform IMP, stack size must be >= 2 to perform binary operations");
         token_handle tk2 = stack.back(); stack.pop_back();
         token_handle tk1 = stack.back(); stack.pop_back();
         LightPolySharedPtr<Token, Operand> op1(tk1);
         LightPolySharedPtr<Token, Operand> op2(tk2);
         int8 val1 = op1->get_val_int();
         int8 val2 = op2->get_val_int();
         LightPolySharedPtr<Token, Operand> rtn(new Operand);
         rtn->set_val(
            (~val1 | val2),
            csimax(op1->get_timestamp(), op2->get_timestamp()));
         stack.push_back(rtn.get_handle());
      } // eval


      void Int::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         if( stack.size() < 1 )
            throw MsgExcept("Unable to perform INT, stack size must be >= 1 to perform unary operations");
         LightPolySharedPtr<Token, Operand> op1(stack.back()); stack.pop_back();
         LightPolySharedPtr<Token, Operand> rtn(new Operand);
         double int_part = 0.0;
         double frac_part = modf(op1->get_val(), &int_part);
         if(int_part < 0 && frac_part != 0.0)
            int_part -= 1.0;
         rtn->set_val(static_cast<int8>(int_part), op1->get_timestamp());
         stack.push_back(rtn.get_handle());
      }


      void NaturalLog::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         if( stack.size() < 1 )
            throw MsgExcept("Unable to perform LOG, stack size must be >= 1 to perform unary operations");
         LightPolySharedPtr<Token, Operand> op1(stack.back()); stack.pop_back();
         LightPolySharedPtr<Token, Operand> rtn(new Operand);
         rtn->set_val(log(op1->get_val()), op1->get_timestamp());
         stack.push_back(rtn.get_handle());
      } // eval


      void LogBase10::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         if( stack.size() < 1 )
            throw MsgExcept("Unable to perform LOG10, stack size must be >= 1 to perform unary operations");
         LightPolySharedPtr<Token, Operand> op1(stack.back()); stack.pop_back();
         LightPolySharedPtr<Token, Operand> rtn(new Operand);
         rtn->set_val(log10(op1->get_val()), op1->get_timestamp());
         stack.push_back(rtn.get_handle());
      } // eval


      void Modulo::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         if( stack.size() < 2 )
            throw MsgExcept("Unable to perform modulo, stack size must be >= 2 to perform binary operations");
         token_handle val1 = stack.back();
         stack.pop_back();
         token_handle val2 = stack.back();
         stack.pop_back();

         if( val1->is_operand() && val2->is_operand() )
         {
            Operand *op1 = static_cast<Operand*>(val1.get_rep());
            Operand *op2 = static_cast<Operand*>(val2.get_rep());
            LightPolySharedPtr<Token,Operand> result(new Operand);
               
            switch(op1->get_type())
            {
            case value_double:
               switch(op2->get_type())
               {
               case value_double:
               case value_int:
               case value_date:
                  result->set_val(
                     fmod(op2->get_val(),op1->get_val()),
                     csimax(op1->get_timestamp(), op2->get_timestamp()));
                  break;
                     
               default:
                  throw MsgExcept("Unsupported modulo operand type");
                  break;
               }
               break;
                  
            case value_int:
               switch(op2->get_type())
               {
               case value_double:
                  result->set_val(
                     fmod(op2->get_val(),op1->get_val()),
                     csimax(op1->get_timestamp(), op2->get_timestamp()));
                  break;
                     
               case value_int:
               case value_date:
                  result->set_val(
                     op2->get_val_int() % op1->get_val_int(),
                     csimax(op1->get_timestamp(), op2->get_timestamp()));
                  break;
                     
               default:
                  throw MsgExcept("Unsupported modulo operand type");
                  break;
               }
               break;
                  
            default:
               throw MsgExcept("Unsupported modulo operand type");
               break;
            }
            stack.push_back(result.get_handle());
         }
         else
            throw MsgExcept("Expression syntax error - unable to perform modulo");
      } // eval


      void NotOperator::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         if( stack.size() < 1 )
            throw MsgExcept("Unable to perform NOT, stack size must be >= 1 to perform binary operations");
         token_handle val1 = stack.back(); stack.pop_back();
         LightPolySharedPtr<Token, Operand> op1(val1);
         LightPolySharedPtr<Token, Operand> result(new Operand);
         
         switch(op1->get_type())
         {
         case value_double:
            if(is_finite(op1->get_val()))
               result->set_val(static_cast<double>(~op1->get_val_int()), op1->get_timestamp());
            else
               result->set_val(op1->get_val(), op1->get_timestamp());
            break;
            
         case value_int:
         case value_date:
         case value_string:
            result->set_val(~op1->get_val_int(), op1->get_timestamp());
            break;
            
         default:
            throw MsgExcept("Incompatible NOT operand type");
         }
         stack.push_back(result.get_handle());
      } // eval


      void OrOperator::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         if( stack.size() < 2 )
            throw MsgExcept("Unable to perform OR, stack size must be >= 2 to perform binary operations");
         token_handle val1 = stack.back();
         stack.pop_back();
         token_handle val2 = stack.back();
         stack.pop_back();

         if( val1->is_operand() && val2->is_operand() )
         {
            Operand *op1 = static_cast<Operand*>(val1.get_rep());
            Operand *op2 = static_cast<Operand*>(val2.get_rep());
            LightPolySharedPtr<Token,Constant> result(new Constant);
            result->set_val(
               op1->get_val_int() | op2->get_val_int(),
               csimax(op1->get_timestamp(), op2->get_timestamp()));
            stack.push_back(result.get_handle());
         }
         else
            throw MsgExcept("Expression syntax error - unable to perform OR");
      } // eval


      void Random::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         double val = rand();
         stack.push_back(
            new Constant(val / (static_cast<double>(RAND_MAX) - 1.0)));
      } // eval


      void Sign::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         if( stack.size() < 1 )
            throw MsgExcept("Unable to perform SIGN, stack size must be >= 1 to perform unary operations");
         token_handle val1 = stack.back();
         stack.pop_back();

         if( val1->is_operand() )
         {
            Operand *op1 = static_cast<Operand*>(val1.get_rep());
            LightPolySharedPtr<Token, Constant> result(new Constant);
            switch(op1->get_type())
            {
            case value_double:
               if(op1->get_val() > 0.0)
                  result->set_val(1.0, op1->get_timestamp());
               else if(op1->get_val() < 0.0)
                  result->set_val(-1.0, op1->get_timestamp());
               else
                  result->set_val(0.0, op1->get_timestamp());
               break;
                  
            case value_int:
            case value_date:
               if(op1->get_val() > 0)
                  result->set_val(int8(1), op1->get_timestamp());
               else if(op1->get_val() < 0)
                  result->set_val(int8(-1), op1->get_timestamp());
               else
                  result->set_val(int8(0), op1->get_timestamp());
               break;
                  
            default:
               throw MsgExcept("Incompatible SIGN operand type");
               break;
            }
            stack.push_back(result.get_handle());
         }
         else
            throw MsgExcept("Expression syntax error - unable to perform LOG10");
      } // eval


      void Sine::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         if( stack.size() < 1 )
            throw MsgExcept("Unable to perform SIN, stack size must be >= 1 to perform unary operations");
         LightPolySharedPtr<Token, Operand> op1(stack.back()); stack.pop_back();
         LightPolySharedPtr<Token, Operand> rtn(new Operand);

         rtn->set_val(sin(op1->get_val()), op1->get_timestamp());
         stack.push_back(rtn.get_handle());
      } // eval


      void HyperbolicSine::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         if( stack.size() < 1 )
            throw MsgExcept("Unable to perform SINH, stack size must be >= 1 to perform unary operations");
         LightPolySharedPtr<Token, Operand> op1(stack.back()); stack.pop_back();
         LightPolySharedPtr<Token, Operand> rtn(new Operand);
         rtn->set_val(sinh(op1->get_val()), op1->get_timestamp());
         stack.push_back(rtn.get_handle());
      } // eval


      void SquareRoot::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         if( stack.size() < 1 )
            throw MsgExcept("Unable to perform SQR, stack size must be >= 1 to perform unary operations");
         LightPolySharedPtr<Token, Operand> op1(stack.back()); stack.pop_back();
         LightPolySharedPtr<Token, Operand> rtn(new Operand);
         rtn->set_val(sqrt(op1->get_val()), op1->get_timestamp());
         stack.push_back(rtn.get_handle());
      } // eval


      void Tangent::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         if( stack.size() < 1 )
            throw MsgExcept("Unable to perform TAN, stack size must be >= 1 to perform unary operations");
         LightPolySharedPtr<Token, Operand> op1(stack.back()); stack.pop_back();
         LightPolySharedPtr<Token, Operand> rtn(new Operand);
         rtn->set_val(tan(op1->get_val()), op1->get_timestamp());
         stack.push_back(rtn.get_handle());
      } // eval


      void HyperbolicTangent::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         if( stack.size() < 1 )
            throw MsgExcept("Unable to perform TANH, stack size must be >= 1 to perform unary operations");
         LightPolySharedPtr<Token, Operand> op1(stack.back()); stack.pop_back();
         LightPolySharedPtr<Token, Operand> rtn(new Operand);
         rtn->set_val(tanh(op1->get_val()), op1->get_timestamp());
         stack.push_back(rtn.get_handle());
      } // eval


      void XOrOperator::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         if( stack.size() < 2 )
            throw MsgExcept("Unable to perform XOR, stack size must be >= 2 to perform binary operations");
         token_handle val1 = stack.back();
         stack.pop_back();
         token_handle val2 = stack.back();
         stack.pop_back();

         if( val1->is_operand() && val2->is_operand() )
         {
            Operand *op1 = static_cast<Operand*>(val1.get_rep());
            Operand *op2 = static_cast<Operand*>(val2.get_rep());
            LightPolySharedPtr<Token, Constant> result(new Constant);
               
            if(op1->get_type() == value_string || op2->get_type() == value_string)
               throw MsgExcept("Incompatible XOR operand types");
            result->set_val(
               op1->get_val_int() ^ op2->get_val_int(),
               csimax(op1->get_timestamp(), op2->get_timestamp()));
            stack.push_back(result.get_handle());
         }
         else
            throw MsgExcept("Expression syntax error - unable to perform XOR");
      } // eval


      void IsEqual::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         if( stack.size() < 2 )
            throw MsgExcept("Unable to perform = operation, stack size must be >= 2 to perform binary operations");
         token_handle val1 = stack.back();
         stack.pop_back();
         token_handle val2 = stack.back();
         stack.pop_back();

         if( val1->is_operand() && val2->is_operand() )
         {
            Operand *op1 = static_cast<Operand*>(val1.get_rep());
            Operand *op2 = static_cast<Operand*>(val2.get_rep());
            LightPolySharedPtr<Token, Operand> result(new Constant);

            if(op1->get_type() == value_string || op2->get_type() == value_string)
            {
               result->set_val(
                  op1->get_val_str() == op2->get_val_str() ? int8(-1) : int8(0),
                  csimax(op1->get_timestamp(), op2->get_timestamp()));
            }
            else if(op1->get_type() == value_double || op2->get_type() == value_double)
            {
               double op1_value(op1->get_val());
               double op2_value(op2->get_val());
               bool incomparable = false;
               int8 rtn(compare_doubles(op1_value, op2_value, 7, incomparable));
               result->set_val(
                  static_cast<int8>(rtn == 0 && !incomparable ? -1 : 0),
                  csimax(op1->get_timestamp(), op2->get_timestamp()));
            }
            else if(op1->get_type() == value_date || op2->get_type() == value_date)
            {
               result->set_val(
                  op1->get_val_date() == op2->get_val_date() ? int8(-1) : int8(0),
                  csimax(op1->get_timestamp(), op2->get_timestamp()));
            }
            else if(op1->get_type() == value_int || op2->get_type() == value_int)
            {
               result->set_val(
                  op1->get_val_int() == op2->get_val_int() ? int8(-1) : int8(0),
                  csimax(op1->get_timestamp(), op2->get_timestamp()));
            }
            else
               throw std::invalid_argument("unsupported comparison types");
            stack.push_back(result.get_handle());
         }
         else
            throw MsgExcept("Expression syntax error - unable to perform =");
      } // eval


      void NotEqual::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         if( stack.size() < 2 )
            throw MsgExcept("Unable to perform <> operation, stack size must be >= 2 to perform binary operations");
         token_handle val1 = stack.back();
         stack.pop_back();
         token_handle val2 = stack.back();
         stack.pop_back();

         if( val1->is_operand() && val2->is_operand() )
         {
            Operand *op1 = static_cast<Operand*>(val1.get_rep());
            Operand *op2 = static_cast<Operand*>(val2.get_rep());
            LightPolySharedPtr<Token, Constant> result(new Constant);

            if(op1->get_type() == value_string || op2->get_type() == value_string)
            {
               result->set_val(
                  op1->get_val_str() != op2->get_val_str() ? int8(-1) : int8(0),
                  csimax(op1->get_timestamp(), op2->get_timestamp()));
            }
            else if(op1->get_type() == value_double || op2->get_type() == value_double)
            {
               double op1_val(op1->get_val());
               double op2_val(op2->get_val());
               bool incomparable = false;
               int8 rtn(compare_doubles(op1_val, op2_val, 7, incomparable));
               result->set_val(
                  static_cast<int8>(rtn != 0 && !incomparable ? -1 : 0),
                  csimax(op1->get_timestamp(), op2->get_timestamp()));
            }
            else if(op1->get_type() == value_date || op2->get_type() == value_date)
            {
               result->set_val(
                  op1->get_val_date() != op2->get_val_date() ? int8(-1) : int8(0),
                  csimax(op1->get_timestamp(), op2->get_timestamp()));
            }
            else if(op1->get_type() == value_int || op2->get_type() == value_int)
            {
               result->set_val(
                  op1->get_val_int() != op2->get_val_int() ? int8(-1) : int8(0),
                  csimax(op1->get_timestamp(), op2->get_timestamp()));
            }
            else
               throw std::invalid_argument("unsupported comparison operand types");
            stack.push_back(result.get_handle());
         }
         else
            throw MsgExcept("Expression syntax error - unable to perform <>");
      } // eval


      void Greater::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         if( stack.size() < 2 )
            throw MsgExcept("Unable to perform > operation, stack size must be >= 2 to perform binary operations");
         token_handle val1 = stack.back();
         stack.pop_back();
         token_handle val2 = stack.back();
         stack.pop_back();

         if( val1->is_operand() && val2->is_operand() )
         {
            Operand *op1 = static_cast<Operand*>(val1.get_rep());
            Operand *op2 = static_cast<Operand*>(val2.get_rep());
            LightPolySharedPtr<Token, Constant> result(new Constant);
            bool incomparable = false;
            int8 rtn;
            if(op1->get_type() == value_string || op2->get_type() == value_string)
            {
               result->set_val(
                  op2->get_val_str() > op1->get_val_str() ? int8(-1) : int8(0),
                  csimax(op1->get_timestamp(), op2->get_timestamp()));
            }
            else if(op1->get_type() == value_double || op2->get_type() == value_double)
            {
               rtn = compare_doubles(op2->get_val(), op1->get_val(), 7, incomparable);
               result->set_val(
                  rtn > 0 && !incomparable ? -1.0 : 0.0,
                  csimax(op1->get_timestamp(), op2->get_timestamp()));
            }
            else if(op1->get_type() == value_date || op2->get_type() == value_date)
            {
               result->set_val(
                  op2->get_val_date() > op1->get_val_date() ? int8(-1) : int8(0),
                  csimax(op1->get_timestamp(), op2->get_timestamp()));
            }
            else if(op1->get_type() == value_int || op2->get_type() == value_int)
            {
               result->set_val(
                  op2->get_val_int() > op1->get_val_int() ? int8(-1) : int8(0),
                  csimax(op1->get_timestamp(), op2->get_timestamp()));
            }
            else
               throw std::invalid_argument("unsupported comparison operand types");
            stack.push_back(result.get_handle());
         }
         else
            throw MsgExcept("Expression syntax error - unable to perform >");
      } // eval


      void Less::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         if( stack.size() < 2 )
            throw MsgExcept("Unable to perform < operation, stack size must be >= 2 to perform binary operations");
         token_handle val1 = stack.back();
         stack.pop_back();
         token_handle val2 = stack.back();
         stack.pop_back();

         if( val1->is_operand() && val2->is_operand() )
         {
            Operand *op1 = static_cast<Operand*>(val1.get_rep());
            Operand *op2 = static_cast<Operand*>(val2.get_rep());
            LightPolySharedPtr<Token, Constant> result(new Constant);
            int8 rtn(0);
            bool incomparable(false);

            if(op1->get_type() == value_string || op2->get_type() == value_string)
            {
               result->set_val(
                  op2->get_val_str() < op1->get_val_str() ? int8(-1) : int8(0),
                  csimax(op1->get_timestamp(), op2->get_timestamp()));
            }
            else if(op1->get_type() == value_double || op2->get_type() == value_double)
            {
               rtn = compare_doubles(op2->get_val(), op1->get_val(), 7, incomparable);
               result->set_val(
                  rtn < 0 && !incomparable ? -1.0 : 0.0,
                  csimax(op1->get_timestamp(), op2->get_timestamp()));
            }
            else if(op1->get_type() == value_date || op2->get_type() == value_date)
            {
               result->set_val(
                  op2->get_val_date() < op1->get_val_date() ? int8(-1) : int8(0),
                  csimax(op1->get_timestamp(), op2->get_timestamp()));
            }
            else if(op1->get_type() == value_int || op2->get_type() == value_int)
            {
               result->set_val(
                  op2->get_val_int() < op1->get_val_int() ? int8(-1) : int8(0),
                  csimax(op1->get_timestamp(), op2->get_timestamp()));
            }
            else
               throw std::invalid_argument("unsupported less than argument types");
            stack.push_back(result.get_handle());
         }
         else
            throw MsgExcept("Expression syntax error - unable to perform <");
      } // eval


      void GreaterEqual::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         if( stack.size() < 2 )
            throw MsgExcept("Unable to perform >= operation, stack size must be >= 2 to perform binary operations");
         token_handle val1 = stack.back();
         stack.pop_back();
         token_handle val2 = stack.back();
         stack.pop_back();

         if( val1->is_operand() && val2->is_operand() )
         {
            Operand *op1 = static_cast<Operand*>(val1.get_rep());
            Operand *op2 = static_cast<Operand*>(val2.get_rep());
            LightPolySharedPtr<Token, Constant> result(new Constant);
            int8 rtn(0);
            bool incomparable(false);

            if(op1->get_type() == value_string || op2->get_type() == value_string)
            {
               result->set_val(
                  op2->get_val_str() >= op1->get_val_str() ? int8(-1) : int8(0),
                  csimax(op1->get_timestamp(), op2->get_timestamp()));
            }
            else if(op1->get_type() == value_double || op2->get_type() == value_double)
            {
               rtn = compare_doubles(op2->get_val(), op1->get_val(), 7, incomparable);
               result->set_val(
                  rtn >= 0 && !incomparable ? -1.0 : 0.0,
                  csimax(op1->get_timestamp(), op2->get_timestamp()));
            }
            else if(op1->get_type() == value_date || op2->get_type() == value_date)
            {
               result->set_val(
                  op2->get_val_date() >= op1->get_val_date() ? int8(-1) : int8(0),
                  csimax(op1->get_timestamp(), op2->get_timestamp()));
            }
            else if(op1->get_type() == value_int || op2->get_type() == value_int)
            {
               result->set_val(
                  op2->get_val_int() >= op1->get_val_int() ? int8(-1) : int8(0),
                  csimax(op1->get_timestamp(), op2->get_timestamp()));
            }
            else
               throw std::invalid_argument("unsupported comparison operand types");
            stack.push_back(result.get_handle());
         }
         else
            throw MsgExcept("Expression syntax error - unable to perform >=");
      } // eval


      void LessEqual::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         if( stack.size() < 2 )
            throw MsgExcept("Unable to perform <= operation, stack size must be >= 2 to perform binary operations");
         token_handle val1 = stack.back();
         stack.pop_back();
         token_handle val2 = stack.back();
         stack.pop_back();
            
         if( val1->is_operand() && val2->is_operand() )
         {
            Operand *op1 = static_cast<Operand*>(val1.get_rep());
            Operand *op2 = static_cast<Operand*>(val2.get_rep());
            LightPolySharedPtr<Token, Constant> result(new Constant);
            int8 rtn(0);
            bool incomparable(false);

            if(op1->get_type() == value_string || op2->get_type() == value_string)
            {
               result->set_val(
                  op2->get_val_str() <= op1->get_val_str() ? int8(-1) : int8(0),
                  csimax(op1->get_timestamp(), op2->get_timestamp()));
            }
            else if(op1->get_type() == value_double || op2->get_type() == value_double)
            {
               rtn = compare_doubles(op2->get_val(), op1->get_val(), 7, incomparable);
               result->set_val(
                  rtn <= 0 && !incomparable ? -1.0 : 0.0,
                  csimax(op1->get_timestamp(), op2->get_timestamp()));
            }
            else if(op1->get_type() == value_date || op2->get_type() == value_date)
            {
               result->set_val(
                  op2->get_val_date() <= op1->get_val_date() ? int8(-1) : int8(0),
                  csimax(op1->get_timestamp(), op2->get_timestamp()));
            }
            else if(op1->get_type() == value_int || op2->get_type() == value_int)
            {
               result->set_val(
                  op2->get_val_int() <= op1->get_val_int() ? int8(-1) : int8(0),
                  csimax(op1->get_timestamp(), op2->get_timestamp()));
            }
            else
               throw std::invalid_argument("unsupported comparison operand types");
            stack.push_back(result.get_handle());
         }
         else
            throw MsgExcept("Expression syntax error - unable to perform <=");
      } // eval


      void StrComp::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         // get the arguments off of the stack
         token_handle tk1, tk2;
            
         if(stack.size() < 2)
            throw MsgExcept("two operands required for StrComp()");
         tk2 = stack.back();
         stack.pop_back();
         tk1 = stack.back();
         stack.pop_back(); 

         // we can now perform the operation
         LightPolySharedPtr<Token, Operand> op1(tk1);
         LightPolySharedPtr<Token, Operand> op2(tk2);
         StrAsc str1(op1->get_val_str());
         StrAsc str2(op2->get_val_str());
         LightPolySharedPtr<Token, Constant> result(new Constant);

         result->set_val(
            static_cast<int8>(str1.compare(str2, false)),
            csimax(op1->get_timestamp(), op2->get_timestamp()));
         stack.push_back(result.get_handle());
      } // eval


      void InStr::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         // extract the parameters from the stack
         token_handle tk1, tk2, tk3;
         if(stack.size() < 3)
            throw MsgExcept("InStr requires three operands");
         tk3 = stack.back();
         stack.pop_back();
         tk2 = stack.back();
         stack.pop_back();
         tk1 = stack.back();
         stack.pop_back();
         
         // these tokens need to be operands
         LightPolySharedPtr<Token, Operand> op1(tk1), op2(tk2), op3(tk3);
         LightPolySharedPtr<Token, Constant> result(new Constant);
         int8 start = op1->get_val_int();
         StrAsc search(op2->get_val_str());
         StrAsc pattern(op3->get_val_str());
         size_t find_result;
         
         if(start <= 0)
            start = 1;
         find_result = search.find(
            pattern.c_str(),
            static_cast<size_t>(start - 1));
         if(find_result >= search.length())
            result->set_val(int8(0), op1->get_timestamp());
         else
            result->set_val(static_cast<int8>(find_result+1), op1->get_timestamp());
         stack.push_back(result.get_handle());
      } // eval
      

      void InStrRev::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         // get the arguments from the stack
         token_handle tk1, tk2, tk3;
         if(stack.size() < 3)
            throw MsgExcept("InStrRev(offset, search, pattern) requires three operands");
         tk3 = stack.back();
         stack.pop_back();
         tk2 = stack.back();
         stack.pop_back();
         tk1 = stack.back();
         stack.pop_back();

         // now perform the operation
         LightPolySharedPtr<Token, Operand> op1(tk1), op2(tk2), op3(tk3);
         int8 start(op1->get_val_int());
         StrAsc search(op2->get_val_str());
         StrAsc pattern(op3->get_val_str());
         LightPolySharedPtr<Token, Constant> result(new Constant);
         size_t rfind_result;
            
         if(start <= 0)
            start = search.length() + 1;
         rfind_result = search.rfind(pattern.c_str(),static_cast<size_t>(start - 1));
         if(rfind_result >= search.length())
            result->set_val(int8(0), op1->get_timestamp());
         else
            result->set_val(static_cast<int8>(rfind_result + 1), op1->get_timestamp());
         stack.push_back(result.get_handle());
      } // eval


      void StrReverse::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         // get the function parameter
         token_handle tk1;
         if(stack.size() < 1)
            throw MsgExcept("StrReverse() requires one parameter");
         tk1 = stack.back();
         stack.pop_back();

         // perform the operation
         LightPolySharedPtr<Token, Operand> op1(tk1);
         LightPolySharedPtr<Token, Constant> result(new Constant);
         StrAsc str(op1->get_val_str());
         str.reverse();
         result->set_val(str, op1->get_timestamp());
         stack.push_back(result.get_handle());
      } // eval


      void Space::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         // get the parameters
         token_handle tk1;

         if(stack.empty())
            throw MsgExcept("Space() requires one parameter");
         tk1 = stack.back();
         stack.pop_back();

         // perform the function
         LightPolySharedPtr<Token, Operand> op1(tk1);
         LightPolySharedPtr<Token, Constant> result(new Constant);
         StrAsc temp;
         int8 count = op1->get_val_int();

         if(count < 0 || count >= get_available_virtual_memory())
            count = 0;
         temp.fill(' ',static_cast<size_t>(count));
         result->set_val(temp, op1->get_timestamp());
         stack.push_back(result.get_handle());
      } // eval


      void Len::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         // get the argument
         token_handle tk1;
         if(stack.empty())
            throw MsgExcept("Len() requires one parameter");
         tk1 = stack.back();
         stack.pop_back();

         // perform the function
         LightPolySharedPtr<Token, Operand> op1(tk1);
         LightPolySharedPtr<Token, Constant> result(new Constant);

         result->set_val(static_cast<int8>(op1->get_val_str().length()), op1->get_timestamp());
         stack.push_back(result.get_handle());
      } // eval


      void Left::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         // extract the parameters from the stack
         token_handle tk1, tk2;
         if(stack.size() < 2)
            throw MsgExcept("Left() requires two parameters");
         tk2 = stack.back();
         stack.pop_back();
         tk1 = stack.back();
         stack.pop_back();

         // perform the operation
         LightPolySharedPtr<Token, Operand> op1(tk1), op2(tk2);
         LightPolySharedPtr<Token, Constant> result(new Constant);
         StrAsc str(op1->get_val_str());
         int8 count(op2->get_val_int());
            
         str.cut(static_cast<size_t>(count));
         result->set_val(str, op1->get_timestamp());
         stack.push_back(result.get_handle());
      } // eval


      void Right::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         // extract the parameters
         token_handle tk1, tk2;
         if(stack.size() < 2)
            throw MsgExcept("Right() requires two paremeters");
         tk2 = stack.back();
         stack.pop_back();
         tk1 = stack.back();
         stack.pop_back();

         // evaluate the function
         LightPolySharedPtr<Token, Operand> op1(tk1), op2(tk2);
         LightPolySharedPtr<Token, Constant> result(new Constant);
         StrAsc str(op1->get_val_str());
         int8 count(op2->get_val_int());
         StrAsc rtn;

         if(count > (int8)str.length())
            count = (int8)str.length();
         str.sub(
            rtn,
            str.length() - static_cast<size_t>(count),
            static_cast<size_t>(count));
         result->set_val(rtn, op1->get_timestamp());
         stack.push_back(result.get_handle());
      } // eval


      void LTrim::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         // extract the arguments
         token_handle tk1;
         if(stack.empty())
            throw MsgExcept("LTrim() requires one parameter");
         tk1 = stack.back();
         stack.pop_back();

         // perform the function
         LightPolySharedPtr<Token, Operand> op1(tk1);
         LightPolySharedPtr<Token, Constant> result(new Constant);
         StrAsc str(op1->get_val_str());
         size_t space_count = 0;

         while(space_count < str.length() && isspace(str[space_count]))
            ++space_count;
         if(space_count > 0)
            str.cut(0,space_count);
         result->set_val(str, op1->get_timestamp());
         stack.push_back(result.get_handle());
      } // eval


      void RTrim::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         // extract the parameter
         token_handle tk1;
         if(stack.empty())
            throw MsgExcept("RTrim requires one parameter");
         tk1 = stack.back();
         stack.pop_back();

         // perform the function
         LightPolySharedPtr<Token, Operand> op1(tk1);
         LightPolySharedPtr<Token, Constant> result(new Constant);
         StrAsc str(op1->get_val_str());
         size_t space_count = 0;

         while(space_count < str.length() &&
               isspace(str[str.length() - space_count - 1]))
            ++space_count;
         str.cut(str.length() - space_count);
         result->set_val(str, op1->get_timestamp());
         stack.push_back(result.get_handle());
      } // eval


      void Trim::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         // extract the parameter
         token_handle tk1;
         if(stack.empty())
            throw MsgExcept("Trim() requires one parameter");
         tk1 = stack.back();
         stack.pop_back();

         // perform the function
         LightPolySharedPtr<Token, Operand> op1(tk1);
         LightPolySharedPtr<Token, Constant> result(new Constant);
         StrAsc str(op1->get_val_str());
         size_t space_count = 0;

         while(space_count < str.length() && isspace(str[space_count]))
            ++space_count;
         str.cut(0,space_count);
         space_count = 0;
         while(space_count < str.length() &&
               isspace(str[str.length() - space_count - 1]))
            ++space_count;
         str.cut(str.length() - space_count);
         result->set_val(str, op1->get_timestamp());
         stack.push_back(result.get_handle());
      } // eval


      void Mid::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         // extract the parameters
         token_handle tk1, tk2, tk3;
         if(stack.size() < 3)
            throw MsgExcept("Mid() requires three parameters");
         tk3 = stack.back();  stack.pop_back();
         tk2 = stack.back(); stack.pop_back();
         tk1 = stack.back(); stack.pop_back();

         // perform the function
         LightPolySharedPtr<Token, Operand> op1(tk1), op2(tk2), op3(tk3);
         LightPolySharedPtr<Token, Constant> result(new Constant);
         StrAsc str(op1->get_val_str());
         int8 start(op2->get_val_int());
         int8 count(op3->get_val_int());
         StrAsc rtn;
            
         if(start <= 0)
            start = 1;
         if(count > (uint4)str.length())
            count = (uint4)str.length();
         str.sub(
            rtn,
            static_cast<size_t>(start - 1),
            static_cast<size_t>(count));
         result->set_val(rtn, op1->get_timestamp());
         stack.push_back(result.get_handle());
      } // eval


      void Replace::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         // extract the parameters
         token_handle tk1, tk2, tk3, tk4, tk5;
         if(stack.size() < 5)
            throw MsgExcept("Replace requires five parameters");
         tk5 = stack.back(); stack.pop_back();
         tk4 = stack.back(); stack.pop_back();
         tk3 = stack.back(); stack.pop_back();
         tk2 = stack.back(); stack.pop_back();
         tk1 = stack.back(); stack.pop_back();

         // evaluate the function
         LightPolySharedPtr<Token, Operand> op1(tk1), op2(tk2), op3(tk3), op4(tk4), op5(tk5);
         LightPolySharedPtr<Token, Constant> result(new Constant);
         StrAsc str(op1->get_val_str());
         StrAsc pattern(op2->get_val_str());
         StrAsc replacement(op3->get_val_str());
         int8 start(op4->get_val_int());
         int8 count(op5->get_val_int());

         if(start < 1)
            start = 1;
         if(count <= 0)
            count = Int8_Max;
         str.replace(
            pattern.c_str(),
            replacement.c_str(),
            static_cast<size_t>(start - 1),
            static_cast<size_t>(count));
         result->set_val(str, op1->get_timestamp());
         stack.push_back(result.get_handle());
      } // eval


      void Last::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         // extract the parameter
         if(stack.size() < 1)
            throw MsgExcept("Last requires one parameter");
         token_handle tk1 = stack.back();
         stack.pop_back();

         LightPolySharedPtr<Token, Operand> op1(tk1);
         LightPolySharedPtr<Token, Operand> result(new Operand);
         result->set_val(previous_value, previous_time);
         previous_value = op1->get_value();
         previous_time = op1->get_timestamp();
         if(!was_set)
         {
            result->set_val(op1->get_value(), op1->get_timestamp());
            was_set = true;
         }
         stack.push_back(result.get_handle());
      } // eval


      void FormatFloat::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         // extract the parameters
         token_handle tk1, tk2;
         if(stack.size() < 2)
            throw MsgExcept("FormatFloat() requires two parameters");
         tk2 = stack.back(); stack.pop_back();
         tk1 = stack.back(); stack.pop_back();

         // perform the function
         LightPolySharedPtr<Token, Operand> op1(tk1);
         LightPolySharedPtr<Token, Constant> op2(tk2);
         double float_val(op1->get_val());
         StrAsc format_str(op2->get_val_str());
         
         Csi::OStrAscStream rtn;
         if(is_finite(float_val))
            rtn << "$\"" << boost::format(format_str.c_str()) % float_val << "\"";
         else if(is_neg_inf(float_val))
            rtn << "$\"-INF\"";
         else if(is_pos_inf(float_val))
            rtn << "$\"INF\"";
         else
            rtn << "$\"NAN\"";
         LightPolySharedPtr<Token, Constant> result(new Constant(rtn.str()));
         result->set_timestamp(op1->get_timestamp());
         stack.push_back(result.get_handle());
      } // eval


      void FormatFloatL::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         // extract the parameters
         token_handle tk1, tk2;
         if(stack.size() < 2)
            throw MsgExcept("FormatFloat() requires two parameters");
         tk2 = stack.back(); stack.pop_back();
         tk1 = stack.back(); stack.pop_back();

         // perform the function
         LightPolySharedPtr<Token, Operand> op1(tk1);
         LightPolySharedPtr<Token, Constant> op2(tk2);
         double float_val(op1->get_val());
         StrAsc format_str(op2->get_val_str());
         Csi::OStrAscStream rtn;
         if(is_finite(float_val))
            rtn << "$\"" << boost::format(format_str.c_str(),Csi::StringLoader::make_locale()) % float_val << "\"";
         else if(is_pos_inf(float_val))
            rtn << "$\"INF\"";
         else if(is_neg_inf(float_val))
            rtn << "$\"-INF\"";
         else
            rtn << "$\"NAN\"";
         LightPolySharedPtr<Token, Constant> result(new Constant(rtn.str()));
         result->set_timestamp(op1->get_timestamp());
         stack.push_back(result.get_handle());
      } // eval


      namespace
      {
         ////////////////////////////////////////////////////////////
         // functor calc_avg
         ////////////////////////////////////////////////////////////
         struct calc_avg
         {
            uint4 count;
            double sum;
            calc_avg():
               count(0),
               sum(std::numeric_limits<double>::quiet_NaN())
            { }

            void operator ()(double val)
            {
               if(is_finite(val))
               {
                  ++count;
                  if(!is_finite(sum))
                     sum = val;
                  else
                     sum += val;
               }
            }

            double avg() const
            {
               double rtn = std::numeric_limits<double>::quiet_NaN();
               if(count > 0)
                  rtn = sum / count;
               return rtn;
            }
         };
      };
      

      ////////////////////////////////////////////////////////////
      // class AvgRun definitions
      ////////////////////////////////////////////////////////////
      AvgRun::AvgRun():
         total(std::numeric_limits<double>::quiet_NaN()),
         insert_pos(0)
      { }
      
         
      void AvgRun::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         // get the stack parameters
         LightPolySharedPtr<Token, Operand> op1, op2;
         if(stack.size() < 2)
            throw MsgExcept("AvgRun() requires two parameters");
         op2 = stack.back();
         stack.pop_back();
         op1 = stack.back();
         stack.pop_back();

         // we need to convert these parameters to numeric values.  We will process the specified
         // value provided it is finite.
         double value = op1->get_val();
         uint4 count = static_cast<uint4>(op2->get_val_int());

         if(is_finite(value))
         {
            if(is_finite(total))
               total += value;
            else
               total = value;
            if(last_values.size() >= count)
            {
               total -= last_values[insert_pos];
               last_values[insert_pos] = value;
               if(++insert_pos >= count)
                  insert_pos = 0;
            }
            else
               last_values.push_back(value);
         }

         // we now need to recalculate the average and place it back on the stack as the return
         // value
         double rtn(std::numeric_limits<double>::quiet_NaN());
         if(!last_values.empty())
            rtn = total / last_values.size();
         stack.push_back(new Operand(rtn, op1->get_timestamp()));
      } // eval


      ////////////////////////////////////////////////////////////
      // class AvgSpa definitions
      ////////////////////////////////////////////////////////////
      void AvgSpa::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         // pop the arguments from the stack
         std::list<double> arguments;
         LgrDate stamp;
         if(stack.size() < args_count)
         {
            OStrAscStream temp;
            temp << "AvgSpa() requires at least " << args_count << " arguments";
            throw std::invalid_argument(temp.c_str());
         }
         for(uint4 i = 0; i < args_count; ++i)
         {
            LightPolySharedPtr<Token, Operand> operand(stack.back());
            stack.pop_back();
            if(operand->get_timestamp() > stamp)
               stamp = operand->get_timestamp();
            arguments.push_back(operand->get_val());
         }

         // we can now calculate the return value
         double rtn(
            std::for_each(arguments.begin(), arguments.end(), calc_avg()).avg());
         stack.push_back(new Operand(rtn, stamp));
      } // eval


      ////////////////////////////////////////////////////////////
      // class MaxRun definitions
      ////////////////////////////////////////////////////////////
      void MaxRun::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         // get the parameters from the stack
         token_handle tk1;
         if(stack.size() < 1)
            throw MsgExcept("MaxRun() requires one parameter");
         tk1 = stack.back();
         stack.pop_back();

         // we will expect the token we just popped to be an Operand
         LightPolySharedPtr<Token, Operand> op1(tk1);
         switch(op1->get_type())
         {
         case value_double:
            if(is_finite(op1->get_val()))
            {
               switch(max_value.type)
               {
               case value_double:
                  if(!is_finite(max_value.vdouble) || op1->get_val() > max_value.vdouble)
                  {
                     max_value = op1->get_value();
                     max_timestamp = op1->get_timestamp();
                  }
                  break;
                  
               case value_int:
               case value_string:
               case value_date:
                  break;
               }
            }
            break;
            
         case value_int:
            switch(max_value.type)
            {
            case value_double:
               if(!is_finite(max_value.vdouble) || max_value.vdouble < op1->get_val())
               {
                  max_value = op1->get_value();
                  max_timestamp = op1->get_timestamp();
               }
               break;
               
            case value_int:
            case value_date:
               if(max_value.vint < op1->get_val_int())
               {
                  max_value = op1->get_value();
                  max_timestamp = op1->get_timestamp();
               }
               break;
               
            case value_string:
               if(max_value.vstring < op1->get_val_str())
               {
                  max_value = op1->get_value();
                  max_timestamp = op1->get_timestamp();
               }
               break;
            }
            break;
            
         case value_string:
            switch(max_value.type)
            {
            case value_double: 
            case value_int:
            case value_date:
               max_value = op1->get_value();
               max_timestamp = op1->get_timestamp();
               break;
               
            case value_string:
               if(max_value.vstring < op1->get_val_str())
               {
                  max_value = op1->get_value();
                  max_timestamp = op1->get_timestamp();
               }
               break;
            }
            break;
         }

         // push on the return value
         LightPolySharedPtr<Token, Operand> rtn(new Operand);
         rtn->set_val(max_value, max_timestamp);
         stack.push_back(rtn.get_handle());
      } // eval


      ////////////////////////////////////////////////////////////
      // class MinRun definitions
      ////////////////////////////////////////////////////////////
      void MinRun::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         // get the parameters from the stack
         token_handle tk1;
         if(stack.size() < 1)
            throw MsgExcept("MinRun() requires one parameter");
         tk1 = stack.back();
         stack.pop_back();

         // we will expect the token we just popped to be an Operand
         LightPolySharedPtr<Token, Operand> var1(tk1);
         switch(var1->get_type())
         {
         case value_double:
            if(is_finite(var1->get_val()))
            {
               double value = var1->get_val();
               switch(min_value.type)
               {
               case value_double:
                  if(!is_finite(min_value.vdouble) || min_value.vdouble > value)
                  {
                     min_value = var1->get_value();
                     min_timestamp = var1->get_timestamp();
                  }
                  break;
                  
               case value_int:
               case value_string:
               case value_date:
                  break;
               }
            }
            break;
            
         case value_int:
            switch(min_value.type)
            {
            case value_double:
               if(!is_finite(min_value.vdouble) || min_value.vdouble > var1->get_val())
               {
                  min_value = var1->get_value();
                  min_timestamp = var1->get_timestamp();
               }
               break;
               
            case value_int:
            case value_date:
               if(min_value.vint > var1->get_val_int())
               {
                  min_value = var1->get_value();
                  min_timestamp = var1->get_timestamp();
               }
               break;
               
            case value_string:
               if(min_value.vstring > var1->get_val_str())
               {
                  min_value = var1->get_value();
                  min_timestamp = var1->get_timestamp();
               }
               break;
            }
            break;
            
         case value_string:
            switch(min_value.type)
            {
            case value_double: 
            case value_int:
            case value_date:
               min_value = var1->get_value();
               min_timestamp = var1->get_timestamp();
               break;
               
            case value_string:
               if(min_value.vstring > var1->get_val_str())
               {
                  min_value = var1->get_value();
                  min_timestamp = var1->get_timestamp();
               }
               break;
            }
            break;
         }

         // push on the return value
         LightPolySharedPtr<Token, Operand> rtn(new Operand);
         rtn->set_val(min_value, min_timestamp);
         stack.push_back(rtn.get_handle());
      } // eval


      namespace
      {
         ////////////////////////////////////////////////////////////
         // predicate value_type_less
         ////////////////////////////////////////////////////////////
         struct value_type_less
         {
            bool operator ()(value_type const &v1, value_type const &v2)
            {
               bool rtn = false;
               if(v1.type == v2.type)
               {
                  switch(v1.type)
                  {
                  case value_double:
                     rtn = v1.vdouble < v2.vdouble;
                     break;
                     
                  case value_int:
                  case value_date:
                     rtn = v1.vint < v2.vint;
                     break;
                     
                  case value_string:
                     rtn = v1.vstring < v2.vstring;
                     break;
                  }
               }
               return rtn;
            }
         };
      };


      ////////////////////////////////////////////////////////////
      // class MedianRun definitions
      ////////////////////////////////////////////////////////////
      void MedianRun::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         // get the stack parameters
         token_handle tk1, tk2;
         if(stack.size() < 2)
            throw MsgExcept("MedianRun() requires two parameters");
         tk2 = stack.back();
         stack.pop_back();
         tk1 = stack.back();
         stack.pop_back();

         // we need to convert these parameters to numeric values.  We will process the specified
         // value provided it is finite.
         LightPolySharedPtr<Token, Operand> op1(tk1);
         LightPolySharedPtr<Token, Operand> op2(tk2);
         int8 count = op2->get_val_int();
         double value(op1->get_val());

         if(is_finite(value))
         {
            while((int8)last_values.size() >= count)
               last_values.pop_front();
            last_values.push_back(value);
         }

         // we can now calculate the return value
         LightPolySharedPtr<Token, Operand> rtn(new Operand);
         rtn->set_val(std::numeric_limits<double>::quiet_NaN(), op1->get_timestamp());
         if(!last_values.empty())
         {
            last_values_type sorted_values(last_values);
            last_values_type::size_type middle(last_values.size() / 2);
            
            std::sort(sorted_values.begin(), sorted_values.end());
            if(last_values.size() % 2 == 0)
            {
               double left(sorted_values[middle]);
               double right(sorted_values[middle - 1]);
               rtn->set_val((left + right) / 2, op1->get_timestamp());
            }
            else
               rtn->set_val(sorted_values[middle], op1->get_timestamp());
         }
         stack.push_back(rtn.get_handle());
      } // eval
      

      ////////////////////////////////////////////////////////////
      // class ValueAtTime definitions
      ////////////////////////////////////////////////////////////
      void ValueAtTime::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         // get the stack parameters
         token_handle tk1, tk2, tk3, tk4;
         if(stack.size() < 4)
            throw MsgExcept("ValueAtTime(value, stamp_nsec, time_range_nsec, default_value) requires four parameters");
         
         tk4 = stack.back();
         stack.pop_back();
         tk3 = stack.back();
         stack.pop_back();
         tk2 = stack.back();
         stack.pop_back();
         tk1 = stack.back();
         stack.pop_back();

         // we need to convert these parameters to numeric values.  We will process the specified
         // value provided it is finite.
         LightPolySharedPtr<Token, Operand> op1(tk1);
         LightPolySharedPtr<Token, Operand> op2(tk2);
         LightPolySharedPtr<Token, Operand> op3(tk3);
         LightPolySharedPtr<Token, Operand> op4(tk4);
         
         value_type value = op1->get_value();
         int8 timestamp = op2->get_val_int();
         int8 time_range = op3->get_val_int();
         value_type default_value = op4->get_value();

         last_values[timestamp] = value;
         
         //Clean out any values outside the specified range
         Csi::LgrDate newest_timestamp = last_values.rbegin()->first;
         last_values_type::iterator val_it = last_values.begin();
         while(val_it != last_values.end())
         {
            int8 difference = (newest_timestamp - val_it->first).get_nanoSec();
            if(difference > time_range)
            {
               report_default = false;
               last_values_type::iterator del_it = val_it++;
               last_values.erase(del_it);
            }
            else if(difference == time_range)
            {
               report_default = false;
               break;
            }
            else
               break;
         }

         // we can now calculate the return value
         LightPolySharedPtr<Token, Operand> rtn(new Operand);
         if(report_default)
         {
            rtn->set_val(default_value, op1->get_timestamp());
         }
         else
         {
            rtn->set_val(std::numeric_limits<double>::quiet_NaN(), op1->get_timestamp());
            if(!last_values.empty())
            {
               rtn->set_val(last_values.begin()->second,last_values.begin()->first);
            }
         }
         stack.push_back(rtn.get_handle());
      } // eval


      void Timestamp::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         token_handle tk1;
         if(stack.size() < 1)
            throw MsgExcept("Timestamp requires one parameter");
         
         tk1 = stack.back();
         stack.pop_back();

         LightPolySharedPtr<Token, Operand> op1(tk1);
         LightPolySharedPtr<Token, Operand> rtn(new Operand);
         rtn->set_val_date(op1->get_timestamp().get_nanoSec(), op1->get_timestamp());
         stack.push_back(rtn.get_handle());
      } //eval


      ////////////////////////////////////////////////////////////
      // class SetTimestamp definitions
      ////////////////////////////////////////////////////////////
      void SetTimestamp::eval(token_stack_type &stack, ExpressionHandler *handler)
      {
         // get the stack parameters
         token_handle tk1, tk2;
         if(stack.size() < 2)
            throw std::invalid_argument("SetTimestamp(value, timestamp) requires two values");
         tk2 = stack.back();
         stack.pop_back();
         tk1 = stack.back();
         stack.pop_back();

         // we need to convert these to operands and generate the return value
         LightPolySharedPtr<Token, Operand> op1(tk1);
         LightPolySharedPtr<Token, Operand> op2(tk2);
         LightPolySharedPtr<Token, Operand> rtn(new Operand);
         rtn->set_val(op1->get_value(), op2->get_val_date());
         stack.push_back(rtn.get_handle());
      } // eval


      ////////////////////////////////////////////////////////////
      // class AvgRunOverTime definitions
      ////////////////////////////////////////////////////////////
      void AvgRunOverTime::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         // get the stack parameters
         token_handle tk1, tk2, tk3;
         if(stack.size() < 3)
            throw MsgExcept("AvgRunOverTime(value, stamp_nsec, time_range_nsec) requires three parameters");
         
         tk3 = stack.back();
         stack.pop_back();
         tk2 = stack.back();
         stack.pop_back();
         tk1 = stack.back();
         stack.pop_back();

         // we need to convert these parameters to numeric values.  We will process the specified
         // value provided it is finite.
         LightPolySharedPtr<Token, Operand> op1(tk1);
         LightPolySharedPtr<Token, Operand> op2(tk2);
         LightPolySharedPtr<Token, Operand> op3(tk3);
         
         value_type value = op1->get_value();
         int8 timestamp = op2->get_val_int();
         int8 time_range = op3->get_val_int();

         if(!Csi::is_signalling_nan(op1->get_val()))
         {
            last_values.insert(last_values_type::value_type(timestamp, value));
            
            // Clean out any values outside the specified range
            Csi::LgrDate newest_timestamp = last_values.rbegin()->first;
            last_values_type::iterator val_it = last_values.begin();
            while(val_it != last_values.end())
            {
               if((newest_timestamp - val_it->first) >= time_range)
               {
                  last_values_type::iterator del_it = val_it++;
                  last_values.erase(del_it);
               }
               else //If we get here, everything else is in the range
                  break;
            }
         }

         // we can now calculate the return value
         LightPolySharedPtr<Token, Operand> rtn(new Operand);
         rtn->set_val(std::numeric_limits<double>::quiet_NaN(), timestamp);
         if(!last_values.empty())
         {
            double sum = 0;
            last_values_type::iterator it = last_values.begin();
            while(it != last_values.end())
            {
               value = it->second;
               double val = 0.0;
               switch(it->second.type)
               {
               case value_double:
                  val = value.vdouble;
                  break;

               case value_int:
                  val = static_cast<double>(value.vint);
                  break;

               case value_string:
                  val = csiStringToFloat(value.vstring.c_str(), std::locale::classic());
                  break;
               }

               sum += val;
               ++it;
            }
            double avg = sum/static_cast<double>(last_values.size());
            rtn->set_val(avg, timestamp);
         }
         stack.push_back(rtn.get_handle());
      } // eval


      ////////////////////////////////////////////////////////////
      // class MaxRunOverTime definitions
      ////////////////////////////////////////////////////////////
      void MaxRunOverTime::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         if(stack.size() < 3)
            throw MsgExcept("MaxRunOverTime(value, stamp_nsec, time_range_nsec) requires three parameters");
         
         // get the stack parameters
         token_handle tk1, tk2, tk3;
         tk3 = stack.back();
         stack.pop_back();
         tk2 = stack.back();
         stack.pop_back();
         tk1 = stack.back();
         stack.pop_back();

         // we need to convert these parameters to numeric values.  We will process the specified
         // value provided it is finite.
         LightPolySharedPtr<Token, Operand> op1(tk1);
         LightPolySharedPtr<Token, Operand> op2(tk2);
         LightPolySharedPtr<Token, Operand> op3(tk3);
         
         value_type value = op1->get_value();
         int8 timestamp = op2->get_val_int();
         int8 time_range = op3->get_val_int();

         if(is_finite(op1->get_val()))
         {
            last_values.insert(last_values_type::value_type(timestamp, value));
            Csi::LgrDate newest_timestamp = last_values.rbegin()->first;
            last_values_type::iterator val_it = last_values.begin();
            while(val_it != last_values.end())
            {
               if((newest_timestamp - val_it->first) >= time_range)
               {
                  last_values_type::iterator del_it = val_it++;
                  last_values.erase(del_it);
               }
               else //If we get here, everything else is in the range
                  break;
            }
         }

         // we can now calculate the return value
         LightPolySharedPtr<Token, Operand> rtn(new Operand);
         rtn->set_val(std::numeric_limits<double>::quiet_NaN(), timestamp);
         if(!last_values.empty())
         {
            double max_value = -std::numeric_limits<double>::max();
            last_values_type::iterator it = last_values.begin();
            while(it != last_values.end())
            {
               value = it->second;
               double val = 0;
               switch(value.type)
               {
               case value_double:
                  val = value.vdouble;
                  break;
                  
               case value_int:
               case value_date:
                  val = static_cast<double>(value.vint);
                  break;
                  
               case value_string:
                  val = csiStringToFloat(value.vstring.c_str(), std::locale::classic());
                  break;
               }
               
               if(val > max_value)
               {
                  max_value = val;
                  rtn->set_val(max_value, it->first);
               }
               ++it;
            }
         }
         stack.push_back(rtn.get_handle());
      } // eval


      ////////////////////////////////////////////////////////////
      // class MinRunOverTime definitions
      ////////////////////////////////////////////////////////////
      void MinRunOverTime::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         // get the stack parameters
         token_handle tk1, tk2, tk3;
         if(stack.size() < 3)
            throw MsgExcept("MinRunOverTime(value, stamp_nsec, time_range_nsec) requires three parameters");
         
         tk3 = stack.back();
         stack.pop_back();
         tk2 = stack.back();
         stack.pop_back();
         tk1 = stack.back();
         stack.pop_back();

         // we need to convert these parameters to numeric values.  We will process the specified
         // value provided it is finite.
         LightPolySharedPtr<Token, Operand> op1(tk1);
         LightPolySharedPtr<Token, Operand> op2(tk2);
         LightPolySharedPtr<Token, Operand> op3(tk3);
         
         value_type value = op1->get_value();
         int8 timestamp = op2->get_val_int();
         int8 time_range = op3->get_val_int();

         last_values.insert(last_values_type::value_type(timestamp, value));

         //Clean out any values outside the specified range
         Csi::LgrDate newest_timestamp = last_values.rbegin()->first;
         last_values_type::iterator val_it = last_values.begin();
         while(val_it != last_values.end())
         {
            if((newest_timestamp - val_it->first) >= time_range)
            {
               last_values_type::iterator del_it = val_it++;
               last_values.erase(del_it);
            }
            else //If we get here, everything else is in the range
               break;
         }

         // we can now calculate the return value
         LightPolySharedPtr<Token, Operand> rtn(new Operand);
         rtn->set_val(std::numeric_limits<double>::quiet_NaN(), op1->get_timestamp());
         if(!last_values.empty())
         {
            double min_value = std::numeric_limits<double>::max();
            last_values_type::iterator it = last_values.begin();
            while(it != last_values.end())
            {
               value = it->second;
               double val = 0;
               switch(value.type)
               {
               case value_double:
                  val = value.vdouble;
                  break;
                  
               case value_int:
               case value_date:
                  val = static_cast<double>(value.vint);
                  break;
                  
               case value_string:
                  val = csiStringToFloat(value.vstring.c_str(), std::locale::classic());
                  break;
               }
               if(val < min_value)
               {
                  min_value = val;
                  rtn->set_val(min_value,it->first);
               }
               ++it;
            }
         }
         stack.push_back(rtn.get_handle());
      } // eval


      ////////////////////////////////////////////////////////////
      // class MedianRunOverTime definitions
      ////////////////////////////////////////////////////////////
      void MedianRunOverTime::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         // get the stack parameters
         token_handle tk1, tk2, tk3;
         if(stack.size() < 3)
            throw MsgExcept("MedianRunOverTime(value, stamp_nsec, time_range_nsec) requires three parameters");
         
         tk3 = stack.back();
         stack.pop_back();
         tk2 = stack.back();
         stack.pop_back();
         tk1 = stack.back();
         stack.pop_back();

         // we need to convert these parameters to numeric values.  We will process the specified
         // value provided it is finite.
         LightPolySharedPtr<Token, Operand> op1(tk1);
         LightPolySharedPtr<Token, Operand> op2(tk2);
         LightPolySharedPtr<Token, Operand> op3(tk3);
         double value = op1->get_val();
         int8 timestamp = op2->get_val_int();
         int8 time_range = op3->get_val_int();

         if(is_finite(value))
         {
            last_values.insert(last_values_type::value_type(timestamp, value));
            
            //Clean out any values outside the specified range
            Csi::LgrDate newest_timestamp = last_values.rbegin()->first;
            last_values_type::iterator val_it = last_values.begin();
            while(val_it != last_values.end())
            {
               if((newest_timestamp - val_it->first) >= time_range)
               {
                  last_values_type::iterator del_it = val_it++;
                  last_values.erase(del_it);
               }
               else //If we get here, everything else is in the range
                  break;
            }
         }

         // we can now calculate the return value
         LightPolySharedPtr<Token, Operand> rtn(new Operand);
         rtn->set_val(std::numeric_limits<double>::quiet_NaN(), op1->get_timestamp());
         if(!last_values.empty())
         {
            typedef std::vector<double> sorted_values_type;
            sorted_values_type sorted_values;
            sorted_values_type::size_type middle = last_values.size() / 2;

            for(last_values_type::iterator vi = last_values.begin(); vi != last_values.end(); ++vi)
               sorted_values.push_back(vi->second);
            std::sort(sorted_values.begin(), sorted_values.end());
            if(last_values.size() % 2 == 0)
            {
               double right(sorted_values[middle]);
               double left(sorted_values[middle - 1]);
               rtn->set_val((left + right) / 2, op1->get_timestamp());
            }
            else
               rtn->set_val(sorted_values[sorted_values.size() / 2], op1->get_timestamp());
         }
         stack.push_back(rtn.get_handle());
      } // eval


      ////////////////////////////////////////////////////////////
      // class FormatTime definitions
      ////////////////////////////////////////////////////////////
      void FormatTime::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         // get the parameters off of the stack
         token_handle tk1, tk2;
         if(stack.size() < 2)
            throw std::invalid_argument("FormatTime() requires two parameters");
         tk2 = stack.back();
         stack.pop_back();
         tk1 = stack.back();
         stack.pop_back();

         // prepare the return value
         LightPolySharedPtr<Token, Operand> op1(tk1), op2(tk2);
         LightPolySharedPtr<Token, Operand> result(new Operand);
         Csi::LgrDate date(op1->get_val_date());
         Csi::OStrAscStream temp;

         temp.imbue(Csi::StringLoader::make_locale());
         date.format(temp, op2->get_val_str().c_str());
         result->set_val(temp.str(), op1->get_timestamp());
         stack.push_back(result.get_handle());
      } // eval


      ////////////////////////////////////////////////////////////
      // class Total definitions
      ////////////////////////////////////////////////////////////
      void Total::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         LightPolySharedPtr<Token, Operand> op;
         LightPolySharedPtr<Token, Constant> rtn(
            new Constant(std::numeric_limits<double>::quiet_NaN()));
         double val;
         
         if(stack.empty())
            throw MsgExcept("Total(value) requires one parameter");
         op = stack.back(); 
         stack.pop_back();
         val = op->get_val();
         if(is_finite(val))
         {
            if(!is_finite(sum))
               sum = 0.0;
            sum += val;
         }
         rtn->set_val(sum, op->get_timestamp());
         stack.push_back(rtn.get_handle());
      } // eval

      
      ////////////////////////////////////////////////////////////
      // class TotalOverTime definitions
      ////////////////////////////////////////////////////////////
      void TotalOverTime::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         // get the stack parameters
         token_handle tk1, tk2, tk3;
         if(stack.size() < 3)
            throw MsgExcept("TotalOverTime(value, stamp_nsec, time_range_nsec) requires three parameters");
         
         tk3 = stack.back();
         stack.pop_back();
         tk2 = stack.back();
         stack.pop_back();
         tk1 = stack.back();
         stack.pop_back();

         // we need to convert these parameters to numeric values.  We will process the specified
         // value provided it is finite.
         LightPolySharedPtr<Token, Operand> op1(tk1);
         LightPolySharedPtr<Token, Operand> op2(tk2);
         LightPolySharedPtr<Token, Operand> op3(tk3);
         
         value_type value = op1->get_value();
         int8 timestamp = op2->get_val_int();
         int8 time_range = op3->get_val_int();

         if(!Csi::is_signalling_nan(op1->get_val()))
         {
            last_values.insert(last_values_type::value_type(timestamp, value));
            
            // Clean out any values outside the specified range
            Csi::LgrDate newest_timestamp = last_values.rbegin()->first;
            last_values_type::iterator val_it = last_values.begin();
            while(val_it != last_values.end())
            {
               if((newest_timestamp - val_it->first) >= time_range) //Not inclusive on the oldest value
               {
                  last_values_type::iterator del_it = val_it++;
                  last_values.erase(del_it);
               }
               else //If we get here, everything else is in the range
                  break;
            }
         }

         // we can now calculate the return value
         LightPolySharedPtr<Token, Operand> rtn(
            new Operand(std::numeric_limits<double>::quiet_NaN(), timestamp));
         if(!last_values.empty())
         {
            double sum = 0;
            last_values_type::iterator it = last_values.begin();
            while(it != last_values.end())
            {
               value = it->second;
               double val = 0.0;
               switch(it->second.type)
               {
               case value_double:
                  val = value.vdouble;
                  break;

               case value_int:
                  val = static_cast<double>(value.vint);
                  break;

               case value_string:
                  val = csiStringToFloat(value.vstring.c_str(), std::locale::classic());
                  break;
               }

               sum += val;
               ++it;
            }
            rtn->set_val(sum, timestamp);
         }
         stack.push_back(rtn.get_handle());
      } // eval


      ////////////////////////////////////////////////////////////
      // class ResettableBase definitions
      ////////////////////////////////////////////////////////////
      void ResettableBase::add_value(double value, LgrDate const &time, int8 reset_option, int8 reset_arg)
      {
         LgrDate this_time(time - LgrDate::nsecPerSec);
         if(!is_signalling_nan(value))
         {
            if(last_values.empty())
               last_values.insert(last_values_type::value_type(this_time, value));
            else
            {
               LgrDate newest_time(last_values.rbegin()->first);
               switch(reset_option)
               {
               case 1:          // reset hourly
                  if(newest_time.year() == this_time.year() &&
                     newest_time.month() == this_time.month() &&
                     newest_time.day() == this_time.day() &&
                     newest_time.hour() == this_time.hour())
                  {
                     last_values.insert(last_values_type::value_type(this_time, value));
                  }
                  else if(this_time > newest_time)
                  {
                     last_values.clear();
                     last_values.insert(last_values_type::value_type(this_time, value));
                  }
                  break;

               case 2:          // reset daily
                  if(this_time.year() == newest_time.year() &&
                     this_time.month() == newest_time.month() &&
                     this_time.day() == newest_time.day())
                  {
                     last_values.insert(last_values_type::value_type(this_time, value));
                  }
                  else if(this_time > newest_time)
                  {
                     last_values.clear();
                     last_values.insert(last_values_type::value_type(this_time, value));
                  } 
                  break;

               case 5:          // reset weekly
                  if(this_time.dayOfWeek() >= newest_time.dayOfWeek() &&
                     abs((this_time.get_nanoSec() - newest_time.get_nanoSec()) / LgrDate::nsecPerDay) <= 7)
                  {
                     last_values.insert(last_values_type::value_type(this_time, value));
                  }
                  else
                  {
                     last_values.clear();
                     last_values.insert(last_values_type::value_type(this_time, value));
                  }
                  break;

               case 3:          // reset monthly
                  if(this_time.year() == newest_time.year() &&
                     this_time.month() == newest_time.month())
                  {
                     last_values.insert(last_values_type::value_type(this_time, value));
                  }
                  else if(this_time > newest_time)
                  {
                     last_values.clear();
                     last_values.insert(last_values_type::value_type(this_time, value));
                  }
                  break;

               case 4:          // reset yearly
                  if(this_time.year() == newest_time.year())
                     last_values.insert(last_values_type::value_type(this_time, value));
                  else if(this_time > newest_time)
                  {
                     last_values.clear();
                     last_values.insert(last_values_type::value_type(this_time, value));
                  }
                  break;

               case 6:          // reset custom
                  if(!reset_arg)
                     last_values.insert(last_values_type::value_type(this_time, value));
                  else
                  {
                     last_values.clear();
                     last_values.insert(last_values_type::value_type(this_time, value));
                  }
                  break;
               }
            }
         }
      } // add_value
      

      ////////////////////////////////////////////////////////////
      // class TotalOverTimeWithReset definitions
      ////////////////////////////////////////////////////////////
      void TotalOverTimeWithReset::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         // check the arguments count
         if(stack.size() < args_count ||
            (args_count != 3 && args_count != 4))
            throw std::invalid_argument("TotalOverTimeWithReset insufficent arguments");
         
         // get the stack parameters
         typedef LightPolySharedPtr<Token, Operand> operand_handle;
         operand_handle op1, op2, op3, op4;
         int8 do_reset(0);
         LightPolySharedPtr<Token, Constant> rtn(
            new Constant(std::numeric_limits<double>::quiet_NaN()));

         if(args_count == 4)
         {
            op4 = stack.back();
            stack.pop_back();
            do_reset = op4->get_val_int();
         }
         op3 = stack.back();
         stack.pop_back();
         op2 = stack.back();
         stack.pop_back();
         op1 = stack.back();
         stack.pop_back();
         add_value(op1->get_val(), op2->get_val_date(), op3->get_val_int(), do_reset);
         if(!last_values.empty())
         {
            double sum = 0;
            for(last_values_type::iterator it = last_values.begin(); it != last_values.end(); ++it)
               sum += it->second;
            rtn->set_val(sum, op2->get_val_date());
         }
         stack.push_back(rtn.get_handle());
      } // eval


      ////////////////////////////////////////////////////////////
      // class AvgRunOverTimeWithReset
      ////////////////////////////////////////////////////////////
      void AvgRunOverTimeWithReset::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         // check the arguments count
         if(stack.size() < args_count ||
            (args_count != 3 && args_count != 4))
            throw std::invalid_argument("AvgRunOverTimeWithReset: invalid number of arguments");
         
         // get the stack parameters
         typedef LightPolySharedPtr<Token, Operand> operand_handle;
         operand_handle op1, op2, op3, op4;
         int8 do_reset(0);
         
         if(args_count == 4)
         {
            op4 = stack.back();
            stack.pop_back();
            do_reset = op4->get_val_int();
         }
         op3 = stack.back();
         stack.pop_back();
         op2 = stack.back();
         stack.pop_back();
         op1 = stack.back();
         stack.pop_back();

         // we can now calculate the return value
         LightPolySharedPtr<Token, Constant> rtn(
            new Constant(std::numeric_limits<double>::quiet_NaN()));
         add_value(op1->get_val(), op2->get_val_date(), op3->get_val_int(), do_reset);
         if(!last_values.empty())
         {
            double total = 0;
            for(last_values_type::iterator it = last_values.begin(); it != last_values.end(); ++it)
               total += it->second;
            rtn->set_val(total / last_values.size(), op2->get_val_date());
         }
         stack.push_back(rtn.get_handle());
      } // eval


      ////////////////////////////////////////////////////////////
      // class MaxRunOverTimeWithReset
      ////////////////////////////////////////////////////////////
      void MaxRunOverTimeWithReset::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         // check the arguments count
         if(stack.size() < args_count ||
            (args_count != 3 && args_count != 4))
            throw std::invalid_argument("MaxRunOverTimeWithReset: invalid number of arguments");

         // get the stack parameters
         typedef LightPolySharedPtr<Token, Operand> operand_handle;
         operand_handle op1, op2, op3, op4;
         int8 do_reset(0);
         
         if(args_count == 4)
         {
            op4 = stack.back();
            stack.pop_back();
            do_reset = op4->get_val_int();
         }
         op3 = stack.back();
         stack.pop_back();
         op2 = stack.back();
         stack.pop_back();
         op1 = stack.back();
         stack.pop_back();
         
         // we can now calculate the return value
         LightPolySharedPtr<Token, Constant> rtn(
            new Constant(std::numeric_limits<double>::quiet_NaN()));
         add_value(op1->get_val(), op2->get_val_date(), op3->get_val_int(), do_reset);
         if(!last_values.empty())
         {
            double maximum = -std::numeric_limits<double>::max();
            LgrDate max_time;
            for(last_values_type::iterator it = last_values.begin(); it != last_values.end(); ++it)
            {
               double val(it->second);
               if(val > maximum)
               {
                  maximum = val;
                  max_time = it->first + 1;
               }
            }
            rtn->set_val(maximum, max_time);
         }
         stack.push_back(rtn.get_handle());
      } // eval


      ////////////////////////////////////////////////////////////
      // class MinRunOverTimeWithReset
      ////////////////////////////////////////////////////////////
      void MinRunOverTimeWithReset::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         // check the arguments count
         if(stack.size() < args_count ||
            (args_count != 3 && args_count != 4))
            throw std::invalid_argument("MinRunOverTimeWithReset: invalid number of arguments");

         // get the stack parameters
         typedef LightPolySharedPtr<Token, Operand> operand_handle;
         operand_handle op1, op2, op3, op4;
         int8 do_reset(0);
         
         if(args_count == 4)
         {
            op4 = stack.back();
            stack.pop_back();
            do_reset = op4->get_val_int();
         }
         op3 = stack.back();
         stack.pop_back();
         op2 = stack.back();
         stack.pop_back();
         op1 = stack.back();
         stack.pop_back();

         // we can now calculate the return value
         LightPolySharedPtr<Token, Constant> rtn(
            new Constant(std::numeric_limits<double>::quiet_NaN()));
         add_value(op1->get_val(), op2->get_val_date(), op3->get_val_int(), do_reset);
         if(!last_values.empty())
         {
            double minimum = std::numeric_limits<double>::max();
            LgrDate min_date;
            for(last_values_type::iterator it = last_values.begin(); it != last_values.end(); ++it)
            {
               double val(it->second);
               if(val < minimum)
               {
                  minimum = val;
                  min_date = it->first + 1;
               }
            }
            rtn->set_val(minimum, min_date);
         }
         stack.push_back(rtn.get_handle());
      } // eval


      ////////////////////////////////////////////////////////////
      // class MaxSpa definitions
      ////////////////////////////////////////////////////////////
      void MaxSpa::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         // pop the arguments from the stack
         LightPolySharedPtr<Token, Operand> rtn;
         if(stack.size() < args_count)
         {
            OStrAscStream temp;
            temp << "MaxSpa() requires at least " << args_count << " arguments";
            throw std::invalid_argument(temp.c_str());
         }
         for(uint4 i = 0; i < args_count; ++i)
         {
            LightPolySharedPtr<Token, Operand> operand(stack.back());
            stack.pop_back();
            if(rtn == 0 && is_finite(operand->get_val()))
               rtn = operand;
            else if(rtn != 0 &&
                    is_finite(operand->get_val()) &&
                    rtn->get_val() < operand->get_val())
               rtn = operand;
         }
         if(rtn == 0)
            rtn.bind(new Operand(std::numeric_limits<double>::quiet_NaN(), LgrDate::system()));
         stack.push_back(rtn.get_handle());
      } // eval


      ////////////////////////////////////////////////////////////
      // class MinSpa definitions
      ////////////////////////////////////////////////////////////
      void MinSpa::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         // pop the arguments from the stack
         LightPolySharedPtr<Token, Operand> rtn;
         if(stack.size() < args_count)
         {
            OStrAscStream temp;
            temp << "MinSpa() requires at least " << args_count << " arguments";
            throw std::invalid_argument(temp.c_str());
         }
         for(uint4 i = 0; i < args_count; ++i)
         {
            LightPolySharedPtr<Token, Operand> operand(stack.back());
            stack.pop_back();
            if(rtn == 0 && is_finite(operand->get_val()))
               rtn = operand;
            else if(rtn != 0 &&
                    is_finite(operand->get_val()) &&
                    rtn->get_val() > operand->get_val())
               rtn = operand;
         }
         if(rtn == 0)
            rtn.bind(new Operand(std::numeric_limits<double>::quiet_NaN(), LgrDate::system()));
         stack.push_back(rtn.get_handle());
      } // eval
      

      ////////////////////////////////////////////////////////////
      // class Alias definitions
      ////////////////////////////////////////////////////////////
      void Alias::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         // this function will operate on two variables.
         if(stack.size() < 2)
            throw MsgExcept("Alias(alias_name, var_name) requires two variable names");
         LightPolySharedPtr<Token, Variable> var(stack.back());
         stack.pop_back();
         LightPolySharedPtr<Token, Variable> alias(stack.back());
         stack.pop_back();

         // we now need to replace any references to the alias name with the variable name in the
         // expression's postfix stack.
         token_stack_type &tokens = expression->get_postfix_stack();
         for(token_stack_type::iterator ei = tokens.begin(); ei != tokens.end(); ++ei)
         {
            token_stack_type::value_type &token = *ei;
            if(token->is_variable())
            {
               LightPolySharedPtr<Token, Variable> ref(token);
               if(ref != var && ref->get_var_name() == alias->get_var_name())
               {
                  *ei = var.get_handle();
                  expression->erase(alias->get_var_name());
               }
            }
         }
      } // eval


      ////////////////////////////////////////////////////////////
      // class Ceiling definitions
      ////////////////////////////////////////////////////////////
      void Ceiling::eval(token_stack_type &stack, ExpressionHandler *expr)
      {
         if(stack.size() < 1)
            throw std::invalid_argument("Ceiling() requires one argument");
         LightPolySharedPtr<Token, Operand> op(stack.back());
         LightPolySharedPtr<Token, Constant> rtn(new Constant);
         stack.pop_back();
         rtn->set_val(static_cast<int8>(ceil(op->get_val())), op->get_timestamp());
         stack.push_back(rtn.get_handle());
      } // eval


      ////////////////////////////////////////////////////////////
      // class Floor definitions
      ////////////////////////////////////////////////////////////
      void Floor::eval(token_stack_type &stack, ExpressionHandler *expr)
      {
         if(stack.size() < 1)
            throw std::invalid_argument("Floor() requires one argument");
         LightPolySharedPtr<Token, Operand> op(stack.back());
         LightPolySharedPtr<Token, Constant> rtn(new Constant);
         stack.pop_back();
         rtn->set_val(static_cast<int8>(floor(op->get_val())), op->get_timestamp());
         stack.push_back(rtn.get_handle());
      } // eval


      ////////////////////////////////////////////////////////////
      // class Hex definitions
      ////////////////////////////////////////////////////////////
      void Hex::eval(token_stack_type &stack, ExpressionHandler *expr)
      {
         if(stack.size() < 1)
            throw std::invalid_argument("Hex(int) requires one integer argument");
         LightPolySharedPtr<Token, Operand> op(stack.back());
         LightPolySharedPtr<Token, Constant> rtn(new Constant);
         OStrAscStream temp;
         
         stack.pop_back();
         temp << std::hex << (op->get_val_int() & 0xFFFFFFFF);
         rtn->set_val(temp.str(), op->get_timestamp());
         stack.push_back(rtn.get_handle());
      } // eval


      ////////////////////////////////////////////////////////////
      // class HexToDec definitions
      ////////////////////////////////////////////////////////////
      void HexToDec::eval(token_stack_type &stack, ExpressionHandler *expr)
      {
         if(stack.size() < 1)
            throw std::invalid_argument("HexToDec(str) requires one string argument");
         LightPolySharedPtr<Token, Operand> op(stack.back());
         LightPolySharedPtr<Token, Constant> rtn(new Constant);
         StrAsc val(op->get_val_str());
         uint4 temp(strtoul(val.c_str(), 0, 16));
         rtn->set_val(static_cast<int8>(temp) & 0xFFFFFFFF, op->get_timestamp());
         stack.pop_back();
         stack.push_back(rtn.get_handle());
      } // eval


      ////////////////////////////////////////////////////////////
      // class Round definitions
      ////////////////////////////////////////////////////////////
      void Round::eval(token_stack_type &stack, ExpressionHandler *expr)
      {
         if(stack.size() < 2)
            throw std::invalid_argument("Round(val, deci) requires two arguments");
         LightPolySharedPtr<Token, Operand> op2(stack.back());
         stack.pop_back();
         LightPolySharedPtr<Token, Operand> op1(stack.back());
         stack.pop_back();
         LightPolySharedPtr<Token, Constant> rtn(new Constant);
         double value(op1->get_val());
         int deci(static_cast<int>(op2->get_val_int()));

         rtn->set_val(Csi::round(value, deci), op1->get_timestamp());
         stack.push_back(rtn.get_handle());
      } // eval


      namespace
      {
         ////////////////////////////////////////////////////////////
         // functor calc_std_dev
         ////////////////////////////////////////////////////////////
         struct calc_std_dev
         {
            double s1;
            double s2;
            uint4 count;
            calc_std_dev():
               s1(0.0),
               s2(0.0),
               count(0)
            { }

            ////////////////////////////////////////////////////////////
            // operator
            ////////////////////////////////////////////////////////////
            typedef std::pair<LgrDate const, double> value_type;
            void operator ()(value_type const &value)
            {
               ++count;
               s1 += value.second * value.second;
               s2 += value.second;
            }
            void operator ()(double const &value)
            {
               ++count;
               s1 += value * value;
               s2 += value;
            }

            ////////////////////////////////////////////////////////////
            // get_std_dev
            ////////////////////////////////////////////////////////////
            double get_std_dev() const
            {
               double rtn(std::numeric_limits<double>::quiet_NaN());
               if(count > 0) 
                  rtn =  sqrt((s1 - (s2 * s2) / count) / count);
               return rtn;
            }
         };
      };


      ////////////////////////////////////////////////////////////
      // class StdDev definitions
      ////////////////////////////////////////////////////////////
      void StdDev::eval(token_stack_type &stack, ExpressionHandler *expr)
      {
         if(stack.size() < 2)
            throw std::invalid_argument("StdDev(val, range) requires two argments");
         LightPolySharedPtr<Token, Operand> op2(stack.back()); stack.pop_back();
         LightPolySharedPtr<Token, Operand> op1(stack.back()); stack.pop_back();
         double value(op1->get_val());
         int8 count(op2->get_val_int());
         LightPolySharedPtr<Token, Constant> rtn(
            new Constant(std::numeric_limits<double>::quiet_NaN()));
         
         if(is_finite(value))
         {
            values.push_back(value);
            while((int8)values.size() > count)
               values.pop_front();
         }
         rtn->set_val(
            std::for_each(values.begin(), values.end(), calc_std_dev()).get_std_dev(),
            op1->get_timestamp());
         stack.push_back(rtn.get_handle());
      } // eval
      

      ////////////////////////////////////////////////////////////
      // class StdDevOverTime definitions
      ////////////////////////////////////////////////////////////
      void StdDevOverTime::eval(token_stack_type &stack, ExpressionHandler *expr)
      {
         // we need to add this value to the stack
         if(stack.size() < 3)
            throw std::invalid_argument("StdDevOverTime(val, time, range) requires three arguments");
         LightPolySharedPtr<Token, Operand> op3(stack.back()); stack.pop_back();
         LightPolySharedPtr<Token, Operand> op2(stack.back()); stack.pop_back();
         LightPolySharedPtr<Token, Operand> op1(stack.back()); stack.pop_back();
         double val(op1->get_val());
         LgrDate time(op2->get_val_date());
         int8 range(op3->get_val_int());

         if(is_finite(val))
         {
            values.insert(values_type::value_type(time, val));
            Csi::LgrDate newest_time(values.rbegin()->first);
            while(!values.empty())
            {
               values_type::iterator vi(values.begin());
               LgrDate val_time(vi->first);
               if(newest_time - val_time > range)
                  values.erase(vi);
               else
                  break;
            }
         }

         // we are now ready to calculate the return value
         LightPolySharedPtr<Token, Constant> rtn(
            new Constant(std::numeric_limits<double>::quiet_NaN()));
         rtn->set_val(
            std::for_each(
               values.begin(), values.end(), calc_std_dev()).get_std_dev(),
            time);
         stack.push_back(rtn.get_handle());
      } // eval


      ////////////////////////////////////////////////////////////
      // class StdDevOverTimeWithReset definitions
      ////////////////////////////////////////////////////////////
      void StdDevOverTimeWithReset::eval(token_stack_type &stack, ExpressionHandler *expr)
      {
         // check the arguments count
         if(stack.size() < args_count ||
            (args_count != 3 && args_count != 4))
            throw std::invalid_argument("StdDevOverTimeWithReset insufficent arguments");

         // get the stack parameters
         typedef LightPolySharedPtr<Token, Operand> operand_handle;
         operand_handle op1, op2, op3, op4;
         int8 do_reset(0);
         LightPolySharedPtr<Token, Constant> rtn(
            new Constant(std::numeric_limits<double>::quiet_NaN()));

         if(args_count == 4)
         {
            op4 = stack.back();
            stack.pop_back();
            do_reset = op4->get_val_int();
         }
         op3 = stack.back();
         stack.pop_back();
         op2 = stack.back();
         stack.pop_back();
         op1 = stack.back();
         stack.pop_back();

         // we can now interpret the operands
         double val(op1->get_val());
         LgrDate time(op2->get_val_date());
         int8 reset_mode(op3->get_val_int());
         add_value(val, time, reset_mode, do_reset);
         rtn->set_val(
            std::for_each(
               last_values.begin(), last_values.end(), calc_std_dev()).get_std_dev(),
            time);
         stack.push_back(rtn.get_handle());
      } // eval


      ////////////////////////////////////////////////////////////
      // class SynchVariable
      ////////////////////////////////////////////////////////////
      class SynchVariable: public Function
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         typedef LightPolySharedPtr<Token, Variable> variable_handle;
         SynchVariable(variable_handle &variable_):
            variable(variable_)
         { }
         
         ////////////////////////////////////////////////////////////
         // format
         ////////////////////////////////////////////////////////////
         virtual void format(std::ostream &out) const
         { out << "SynchVariable(" << variable->get_var_name() << ")"; }

         ////////////////////////////////////////////////////////////
         // clone
         ////////////////////////////////////////////////////////////
         virtual SynchVariable *clone() const
         { throw std::invalid_argument("cloning of SynchVariable not allowed"); }

         ////////////////////////////////////////////////////////////
         // eval
         ////////////////////////////////////////////////////////////
         virtual void eval(token_stack_type &stack, ExpressionHandler *expr);

         ////////////////////////////////////////////////////////////
         // reset_state
         ////////////////////////////////////////////////////////////
         virtual void reset_state()
         { values.clear(); }

         ////////////////////////////////////////////////////////////
         // update
         ////////////////////////////////////////////////////////////
         void update();

         ////////////////////////////////////////////////////////////
         // trim
         ////////////////////////////////////////////////////////////
         void trim(LgrDate const &oldest);

         ////////////////////////////////////////////////////////////
         // get_js_resource
         ////////////////////////////////////////////////////////////
         virtual StrAsc get_js_resource() const
         { return "CsiSynchVariable.js"; }

         ////////////////////////////////////////////////////////////
         // format_js
         ////////////////////////////////////////////////////////////
         virtual void format_js(std::ostream &out)
         { out << "new CsiSynchVariable(variables[" << variable->get_js_index() << "])"; }
         

         ////////////////////////////////////////////////////////////
         // get_min_arguments
         ////////////////////////////////////////////////////////////
         virtual uint4 get_min_arguments()
         { return 0; }

         ////////////////////////////////////////////////////////////
         // get_argument_name
         ////////////////////////////////////////////////////////////
         virtual wchar_t const *get_argument_name(uint4 index)
         { throw std::invalid_argument("invalid argument index"); }

      private:
         ////////////////////////////////////////////////////////////
         // variable
         ////////////////////////////////////////////////////////////
         variable_handle variable;

         ////////////////////////////////////////////////////////////
         // values
         ////////////////////////////////////////////////////////////
         typedef std::map<LgrDate, value_type> values_type;
         values_type values;
      };


      void SynchVariable::eval(
         token_stack_type &stack, ExpressionHandler *expr)
      {
         // We need to update the values of each of the SynchVariables in the stack.  While doing
         // this, we will also determine the "oldest newest" time stamps for all of the synched
         // variables. 
         token_stack_type &tokens(expr->get_postfix_stack());
         LgrDate oldest_newest(std::numeric_limits<int8>::max());
         bool trim_after(false);
         for(token_stack_type::iterator ei = tokens.begin(); ei != tokens.end(); ++ei)
         {
            SynchVariable *other = dynamic_cast<SynchVariable *>(ei->get_rep());
            if(other != 0)
            {
               other->update();
               if(!other->values.empty())
               {
                  LgrDate other_newest(other->values.rbegin()->first);
                  if(other_newest < oldest_newest)
                     oldest_newest = other_newest;
               }
               else
                  oldest_newest = 0;
               if(other == this)
                  trim_after = true;
               else
                  trim_after = false;
            }
         }
         variable->reset_state();
         
         // We can now examine our own list of values.  We want to select the value in our list
         // whose stamp is closest to the oldest_newest and less than or equal to that time
         bool found_value(false);
         for(values_type::iterator vi = values.begin(); vi != values.end() && !found_value; ++vi)
         {
            if(vi->first == oldest_newest)
            {
               LightPolySharedPtr<Token, Operand> rtn(new Operand);
               rtn->set_val(vi->second, vi->first);
               stack.push_back(rtn.get_handle());
               found_value = true;
            }
         }

         // we need to clean up any stored values that are older than the selected value.  We must
         // only do this if this variable is the last synched variable that is evaluated in the
         // expression.
         for(token_stack_type::iterator ei = tokens.begin(); trim_after && ei != tokens.end(); ++ei)
         {
            SynchVariable *other = dynamic_cast<SynchVariable *>(ei->get_rep());
            if(other != 0)
               other->trim(oldest_newest);
         }

         // finally, if no value was found, we will need to throw an exception to abort the
         // evaluation of the expression
         if(!found_value)
            throw ExcSynchValues();
      } // eval


      void SynchVariable::update()
      {
         if(variable->get_has_been_set())
         {
            // we will add the value with its time stamp.  However, in order to avoid problems that
            // can come from trying to synch values with sub-second resolution, we will simply strip
            // off any subsecond resolution for the stored time
            LgrDate stamp(variable->get_timestamp());
            stamp -= stamp.nsec();
            values[stamp] = variable->get_value();

            // there may be cases where one or more variables may stop updating.  In these cases,
            // those variables that continue to update can accumulate a large number of values.  In
            // order to prevent a memory leak, we will limit the number of variables that can be
            // buffered to 100000 values.
            while(values.size() > 100000)
               values.erase(values.begin());
         }
      } // update


      void SynchVariable::trim(LgrDate const &oldest)
      {
         while(!values.empty())
         {
            values_type::iterator vi(values.begin());
            if(vi->first <= oldest)
               values.erase(vi);
            else
               break;
         }
      } // trim
      

      ////////////////////////////////////////////////////////////
      // class ValueSynch definitions
      ////////////////////////////////////////////////////////////
      void ValueSynch::eval(
         token_stack_type &stack, ExpressionHandler *expr)
      {
         // get the parameters off of the stack
         if(stack.size() < 2)
            throw std::invalid_argument("ValueSynch(alias_name, variable) requires two arguments");
         LightPolySharedPtr<Token, Variable> variable(stack.back()); stack.pop_back();
         LightPolySharedPtr<Token, Variable> alias(stack.back()); stack.pop_back();

         // wwe will now replace any references to the alias_name with an encapsulated variable
         // object
         token_stack_type &tokens(expr->get_postfix_stack());
         for(token_stack_type::iterator ei = tokens.begin(); ei != tokens.end(); ++ei)
         {
            token_stack_type::value_type &token(*ei);
            if(token->is_variable())
            {
               LightPolySharedPtr<Token, Variable> ref(token);
               if(ref != variable && ref->get_var_name() == alias->get_var_name())
               {
                  ei->bind(new SynchVariable(variable));
                  expr->erase(alias->get_var_name());
               }
            }
         }
      } // eval


      ////////////////////////////////////////////////////////////
      // class StartAtRecordFunction definitions
      ////////////////////////////////////////////////////////////
      void StartAtRecordFunction::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         // retrieve the tokens from the stack
         if(stack.size() < 3)
            throw std::invalid_argument("StartAtRecord() requires at least three operands");
         token_handle tk3(stack.back());
         stack.pop_back();
         token_handle tk2(stack.back());
         stack.pop_back();
         token_handle tk1(stack.back());
         stack.pop_back();
         
         if(tk1->is_operand() && tk2->is_operand() && tk3->is_operand())
         {
            // we need to apply these start options to all variables in the expression token stack
            Operand *op1 = static_cast<Operand *>(tk1.get_rep());
            Operand *op2 = static_cast<Operand *>(tk2.get_rep());
            Operand *op3 = static_cast<Operand *>(tk3.get_rep());
            expression->set_start_at_record(
               static_cast<uint4>(op1->get_val_int()),
               static_cast<uint4>(op2->get_val_int()));
            expression->set_order_option(op3->get_val_int());
         }
         else
            throw std::invalid_argument("invalid operands for StartAtRecord");
      } // eval


      ////////////////////////////////////////////////////////////
      // class StartAtTimeFunction definitions
      ////////////////////////////////////////////////////////////
      void StartAtTimeFunction::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         // retrieve the tokens from the stack
         if(stack.size() < 2)
            throw std::invalid_argument("StartAtTime() requires at least two operands");
         token_handle tk2(stack.back());
         stack.pop_back();
         token_handle tk1(stack.back());
         stack.pop_back();
         
         if(tk1->is_operand() && tk2->is_operand())
         {
            Operand *op1 = static_cast<Operand *>(tk1.get_rep());
            Operand *op2 = static_cast<Operand *>(tk2.get_rep());
            expression->set_start_at_time(op1->get_val_date());
            expression->set_order_option(op2->get_val_int());
         }
         else
            throw std::invalid_argument("invalid operands for StartAtTime");
      } // eval


      ////////////////////////////////////////////////////////////
      // class StartAtNewestFunction definitions
      ////////////////////////////////////////////////////////////
      void StartAtNewestFunction::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         // retrieve the tokens from the stack
         if(stack.size() < 1)
            throw std::invalid_argument("StartAtNewest() requires at least one operand");
         token_handle tk1(stack.back());
         stack.pop_back();
         
         if(tk1->is_operand())
         {
            Operand *op1 = static_cast<Operand *>(tk1.get_rep());
            expression->set_start_at_newest();
            expression->set_order_option(op1->get_val_int());
         }
         else
            throw std::invalid_argument("invalid operands for StartAtNewest");
      }


      ////////////////////////////////////////////////////////////
      // class StartAfterNewestFunction
      ////////////////////////////////////////////////////////////
      void StartAfterNewestFunction::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         // retrieve the tokens from the stack
         if(stack.size() < 1)
            throw std::invalid_argument("StartAfterNewest() requires at least one operand");
         token_handle tk1(stack.back());
         stack.pop_back();
         
         if(tk1->is_operand())
         {
            Operand *op1 = static_cast<Operand *>(tk1.get_rep());
            expression->set_start_after_newest();
            expression->set_order_option(op1->get_val_int());
         }
         else
            throw std::invalid_argument("invalid operands for StartAfterNewest");
      } // eval


      ////////////////////////////////////////////////////////////
      // class StartRelativeToNewestFunction definitions
      ////////////////////////////////////////////////////////////
      void StartRelativeToNewestFunction::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         // retrieve the tokens from the stack
         if(stack.size() < 2)
            throw std::invalid_argument("StartRelativeToNewest() requires at least two operands");
         token_handle tk2(stack.back());
         stack.pop_back();
         token_handle tk1(stack.back());
         stack.pop_back();
         
         if(tk1->is_operand() && tk2->is_operand())
         {
            Operand *op1 = static_cast<Operand *>(tk1.get_rep());
            Operand *op2 = static_cast<Operand *>(tk2.get_rep());
            expression->set_start_relative_to_newest(op1->get_val_int());
            expression->set_order_option(op2->get_val_int());
         }
         else
            throw std::invalid_argument("invalid operands for StartRelativeToNewest");
      } // eval


      ////////////////////////////////////////////////////////////
      // class StartAtOffsetFromNewestFunction definitions
      ////////////////////////////////////////////////////////////
      void StartAtOffsetFromNewestFunction::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         // retrieve the tokens from the stack
         if(stack.size() < 2)
            throw std::invalid_argument("StartRelativeToNewest() requires at least two operands");
         token_handle tk2(stack.back());
         stack.pop_back();
         token_handle tk1(stack.back());
         stack.pop_back();
         
         if(tk1->is_operand() && tk2->is_operand())
         {
            Operand *op1 = static_cast<Operand *>(tk1.get_rep());
            Operand *op2 = static_cast<Operand *>(tk2.get_rep());
            expression->set_start_at_offset_from_newest(static_cast<uint4>(op1->get_val_int()));
            expression->set_order_option(op2->get_val_int());
         }
         else
            throw std::invalid_argument("invalid operands for StartAtOffsetFromNewest()");
      } // eval


      ////////////////////////////////////////////////////////////
      // class ReportOffsetFunction definitions
      ////////////////////////////////////////////////////////////
      void ReportOffsetFunction::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         if(stack.size() < 1)
            throw std::invalid_argument("ReportOffset() requires one operand");
         token_handle token(stack.back());
         stack.pop_back();
         if(token->is_operand())
         {
            Operand *op(static_cast<Operand *>(token.get_rep()));
            expression->set_report_offset(op->get_val_int());
         }
         else
            throw std::invalid_argument("invalid operand type for ReportOffset()");
      } // eval


      ////////////////////////////////////////////////////////////
      // class LocalToGmt definitions
      ////////////////////////////////////////////////////////////
      void LocalToGmt::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         LightPolySharedPtr<Token, Operand> op;
         LightPolySharedPtr<Token, Operand> rtn(new Operand);
         LgrDate date;
         
         if(stack.size() < 1)
            throw std::invalid_argument("LocalToGmt() requires one operand");
         op = stack.back();
         stack.pop_back();
         date = op->get_val_date();
         rtn->set_val_date(date.get_nanoSec() - date.gmt_offset(), date);
         stack.push_back(rtn.get_handle());
      } // eval


      ////////////////////////////////////////////////////////////
      // class GmtToLocal definitions
      ////////////////////////////////////////////////////////////
      void GmtToLocal::eval(token_stack_type &stack, ExpressionHandler *expression)
      {
         LightPolySharedPtr<Token, Operand> op;
         LightPolySharedPtr<Token, Operand> rtn(new Operand);
         LgrDate date;
         
         if(stack.size() < 1)
            throw std::invalid_argument("GmtToLocal() requires one operand");
         op = stack.back();
         stack.pop_back();
         date = op->get_val_date();
         rtn->set_val_date(date.get_nanoSec() + date.gmt_offset(), date);
         stack.push_back(rtn.get_handle());
      } // eval
   };
};


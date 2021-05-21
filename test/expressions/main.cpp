/* main.cpp

   Copyright (C) 2002, 2011 Campbell Scientific, Inc.

   Written by: Tyler Mecham
   Date Begun: 16 January, 2002
   Last Change: Thursday 14 July 2011
   Last Commit: $Date: 2006/03/17 23:54:44 $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header 
#include "../../Csi.Expression.ExpressionHandler.h"
#include "../../Csi.Expression.TokenTypes.h"
#include "../../CsiTypes.h"
#include <iostream>
#include <deque>
#include <string>


int main(int argc, char* argv[])
{
   using namespace Csi::Expression;
   ExpressionHandler expression;
   std::string str;
   
   std::cout << "\nPlease Enter Expression:";
   while(std::getline(std::cin,str))
   {
      try
      {
         if( str != "" )
         {
            expression.tokenize(str.c_str());
            
            for(ExpressionHandler::iterator vi = expression.begin();
                vi != expression.end();
                ++vi)
            {
               Variable *var = static_cast<Variable*>(vi->second.get_rep());
               std::cout << "Set " << var->get_var_name() << " = ";
               double val;
               std::cin >> val;
               var->set_val(val, Csi::LgrDate::system());
            }
            
            // print the expression's postfix representation
            expression.format_postfix(std::cout);
            std::cout << "\n";
            
            try
            {
               ExpressionHandler::operand_handle result(expression.eval());
               std::cout << "Result = ";
               
               switch(result->get_type())
               {
               case value_double:
                  csiFloatToStream(std::cout, result->get_val());
                  break;
                  
               case value_int:
                  std::cout << result->get_val_int();
                  break;
                  
               case value_string:
                  std::cout << "\"" << result->get_val_str() << "\"";
                  break;

               case value_date:
                  std::cout <<Csi::LgrDate(result->get_val_date());
                  break;
               }
               std::cout << std::endl;
            }
            catch(std::exception &e)
            {
               std::cout << "\nUnable to evaluate expression\n" << std::endl;
               std::cout << e.what() << std::endl;
            }
         }
      }
      catch(std::exception &e)
      {
         std::cerr << "Expression parse error: " << e.what() << "\n";
      }
      std::cout << "\n\nPlease Enter Expression: ";
   }
   return 0;
} // main


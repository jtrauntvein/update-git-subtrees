/* CrBasic.Constants.cpp

   Copyright (C) 2013, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Wednesday 20 March 2013
   Last Change: Friday 29 April 2016
   Last Commit: $Date: 2016-05-03 16:18:03 -0600 (Tue, 03 May 2016) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "CrBasic.Constants.h"
#include <cctype>
#include <iostream>


namespace CrBasic
{
   void parse_constants(constants_type &constants, std::istream &in)
   {
      enum state_type
      {
         state_between_constants,
         state_finish_line,
         state_const_c,
         state_const_o,
         state_const_n,
         state_const_s,
         state_const_t,
         state_before_name,
         state_in_name,
         state_before_equal,
         state_before_value,
         state_in_value,
         state_in_value_quote
      } state(state_between_constants);
      StrAsc name;
      StrAsc value;
      char ch;
      int8 pos(0);
      int8 start_pos;
      
      while(in.get(ch))
      {
         switch(state)
         {
         case state_between_constants:
            if(ch == 'c' || ch == 'C')
            {
               start_pos = pos;
               state = state_const_c;
            }
            else if(!isspace(ch))
               state = state_finish_line;
            break;
            
         case state_finish_line:
            if(ch == '\n' || ch == '\r')
               state = state_between_constants;
            break;
            
         case state_const_c:
            if(ch == 'o' || ch == 'O')
               state = state_const_o;
            else
               state = state_finish_line;
            break;
            
         case state_const_o:
            if(ch == 'n' || ch == 'N')
               state = state_const_n;
            else
               state = state_finish_line;
            break;

         case state_const_n:
            if(ch == 's' || ch == 'S')
               state = state_const_s;
            else
               state = state_finish_line;
            break;
            
         case state_const_s:
            if(ch == 't' || ch == 'T')
               state = state_const_t;
            else
               state = state_finish_line;
            break;
            
         case state_const_t:
            if(ch == ' ' || ch == '\t')
               state = state_before_name;
            else
               state = state_finish_line;
            break;
            
         case state_before_name:
            if(ch != ' ' && ch != '\t')
            {
               name.cut(0);
               name.append(ch);
               state = state_in_name;
            }
            break;
            
         case state_in_name:
            if(ch != ' ' && ch != '\t' && ch != '=')
               name.append(ch);
            else if(ch == '=')
               state = state_before_value;
            else if(ch == '\r' || ch == '\n')
               state = state_between_constants;
            else if(ch == ' ' || ch == '\t')
               state = state_before_equal;
            break;
            
         case state_before_equal:
            if(ch == '=')
               state = state_before_value;
            else if(ch != ' ' && ch != '\t')
               state = state_finish_line;
            break;
            
         case state_before_value:
            if(ch != ' ' && ch != '\t')
            {
               value.cut(0);
               value.append(ch);
               if(ch == '\"')
                  state = state_in_value_quote;
               else
                  state = state_in_value;
            }
            break;
               
         case state_in_value:
            if(!isspace(ch))
               value.append(ch);
            else
            {
               constants.push_back(Constant(name, value, start_pos, pos - start_pos));
               if(ch == '\r' || ch == '\n')
                  state = state_between_constants;
               else
                  state = state_finish_line;
            }
            if(ch == '\"')
               state = state_in_value_quote;
            break;

         case state_in_value_quote:
            value.append(ch);
            if(ch == '\"')
               state = state_in_value;
            else if(ch == '\r' || ch == '\n')
               state = state_between_constants;
            break;
         }
         ++pos;
      }
   } // parse_constants


   void output_constants(
      std::ostream &out,
      std::istream &in,
      constants_type const &constants)
   {
      constants_type::const_iterator ci(constants.begin());
      uint4 pos(0);
      char ch;
      while(in.get(ch))
      {
         if(ci != constants.end())
         {
            if(pos < ci->start_pos)
               out << ch;
            else if(pos == ci->start_pos)
               out << "Const " << ci->name << " = " << ci->value;
            else if(pos >= ci->start_pos + ci->length)
            {
               out << ch;
               ++ci;
            }
         }
         else
            out << ch;
         ++pos;
      }
   } // output_constants
};


/* Csi.CommandLine.cpp

   Copyright (C) 2000, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 13 January 2000
   Last Change: Thursday 29 November 2012
   Lasts Commit: $Date: 2013-04-06 07:00:13 -0600 (Sat, 06 Apr 2013) $ 
   Committed by: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.CommandLine.h"
#include "CsiTypeDefs.h"
#include <wctype.h>
#ifndef WIN32
#include <wctype.h>
#endif


namespace Csi
{
   ////////////////////////////////////////////////////////////
   // class CommandLine definitions
   ////////////////////////////////////////////////////////////
   CommandLine::CommandLine()
   { }


   CommandLine::~CommandLine()
   { }


   void CommandLine::add_expected_option(StrAsc const &option_name)
   { expected_options.insert(option_name); }


   void CommandLine::parse_command_line(wchar_t const *command_line)
   {
      // initialise the options and arguments containers
      options.clear();
      arguments.clear();

      // work through the command line string
      StrUni token;
      bool skip_increment = true;
      bool in_quote = false;
      uint4 pos = 0;
      enum state_type
      {
         state_between_tokens,
         state_in_arg,
         state_found_optmark_1,
         state_in_option_name,
         state_in_option_value,
         state_in_comment
      } state = state_between_tokens;
      StrAsc option_name;
      uint4 nested_brace_count = 0;

      while(command_line[pos] != 0)
      {
         wchar_t this_char = command_line[pos];

         skip_increment = false;
         if(state == state_between_tokens)
         {
            if(this_char == L'-')
               state = state_found_optmark_1;
            else if(this_char == L'#')
               state = state_in_comment;
            else if(!iswspace(this_char))
            { skip_increment = true; state = state_in_arg; }
         }
         else if(state == state_in_comment)
         {
            if(this_char == L'\n')
               state = state_between_tokens;
         }
         else if(state == state_in_arg)
         {
            // process the character
            if(this_char == L'{' && !in_quote)
            {
               if(++nested_brace_count > 1)
                  token += this_char;
            }
            else if(this_char == L'}' && !in_quote)
            {
               if(nested_brace_count == 0)
                  throw MsgExcept("Unmatched braces in input");
               if(--nested_brace_count > 0)
                  token += this_char;
            }
            else if(this_char == L'\"' && nested_brace_count == 0)
               in_quote = !in_quote;
            else if(in_quote || nested_brace_count > 0)
               token += this_char;
            else if(!iswspace(this_char))
               token += this_char;
            else
            {
               state = state_between_tokens;
               arguments.push_back(token.to_utf8());
               token.cut(0);
            }
         }
         else if(state == state_found_optmark_1)
         {
            if(this_char == L'-')
               state = state_in_option_name;
            else
            { state = state_in_arg; token += L'-'; skip_increment = true; }
         }
         else if(state == state_in_option_name)
         {
            if(iswspace(this_char) || this_char == L'=' || this_char == L':')
            {
               // check to see if the token is a recognised option name
               StrAsc temp(token.to_utf8());
               expected_options_type::iterator ei = expected_options.find(temp);
               if(ei != expected_options.end())
               {
                  option_name = temp;
                  token.cut(0);
                  if(iswspace(this_char))
                  {
                     options[option_name] = StrAsc();
                     state = state_between_tokens;
                  }
                  else
                     state = state_in_option_value;
               }
               else
                  throw ExcUnknownOption(token.to_utf8());
            }
            else
               token += this_char;
         }
         else if(state == state_in_option_value)
         {
            // process the character
            if(!in_quote && this_char == L'{')
            {
               if(++nested_brace_count > 1)
                  token += this_char;
            }
            else if(!in_quote && this_char == L'}')
            {
               if(nested_brace_count == 0)
                  throw MsgExcept("Unmatched braces in input");
               if(--nested_brace_count > 0)
                  token += this_char;
            }
            else if(this_char == L'\"' && nested_brace_count == 0)
               in_quote = !in_quote;
            else if(in_quote || nested_brace_count > 0)
               token += this_char;
            else if(!iswspace(this_char))
               token += this_char;
            else
            {
               options[option_name] = token.to_utf8();
               token.cut(0);
               state = state_between_tokens;
            }
         }

         // proceed to the next character
         if(!skip_increment)
            pos++;
      }

      // process whatever was left at the end of the loop
      if(in_quote)
         throw MsgExcept("Unmatched quotation marks in input");
      if(nested_brace_count > 0)
         throw MsgExcept("Unmatched braces in input");
      if(state == state_in_arg)
         arguments.push_back(token.to_utf8());
      else if(state == state_in_option_name && token.length())
      {
         StrAsc temp(token.to_utf8());
         expected_options_type::const_iterator ei = expected_options.find(temp);
         if(ei != expected_options.end())
            options[temp] = StrAsc();
         else
            throw ExcUnknownOption(temp);
      }
      else if(state == state_in_option_value)
         options[option_name] = token.to_utf8();
   } // parse_command_line


   bool CommandLine::get_option_value(StrAsc const &option_name, StrAsc &value_buffer) const
   {
      options_type::const_iterator oi = options.find(option_name);
      bool rtn = false;

      if(oi != options.end())
      {
         rtn = true;
         value_buffer = oi->second;
      }
      return rtn;
   } // get_option_value


   bool CommandLine::get_option_value(StrAsc const &option_name, StrUni &value_buffer) const
   {
      options_type::const_iterator oi = options.find(option_name);
      bool rtn = false;

      if(oi != options.end())
      {
         rtn = true;
         value_buffer = oi->second;
      }
      return rtn;
   } // get_option_value


   bool CommandLine::get_argument(StrAsc &buffer, arguments_type::size_type pos)
   {
      bool rtn;
      if(pos < arguments.size())
      {
         buffer = arguments[pos];
         rtn = true;
      }
      else
         rtn = false;
      return rtn;
   } // get_argument


   bool CommandLine::get_argument(StrUni &buffer, arguments_type::size_type pos)
   {
      bool rtn;
      if(pos < arguments.size())
      {
         buffer = arguments[pos];
         rtn = true;
      }
      else
         rtn = false;
      return rtn;
   } // get_argument
};

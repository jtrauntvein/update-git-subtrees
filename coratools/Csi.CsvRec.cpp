/* Csi.CsvRec.cpp

   Copyright (C) 2000, 2017 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 16 November 2000
   Last Change: Thursday 07 September 2017
   Last Commit: $Date: 2017-09-07 16:55:39 -0600 (Thu, 07 Sep 2017) $ 
   Committed by: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header

#include <iostream>
#include <algorithm>
#include "Csi.CsvRec.h"
#include "Csi.Utils.h"


namespace Csi
{
   namespace
   {
      /**
       * @return Returns a utf-8 encoding of a parsed JSON format string.
       *
       * @param s Specifies the string to parse.
       */
      StrAsc parse_json_string(StrAsc const &s)
      {
         enum state_type
         {
            state_in_content,
            state_solidus,
            state_unicode
         } state(state_in_content);
         StrAsc rtn;
         StrAsc unicode_buff;

         rtn.reserve(s.length());
         for(size_t i = 0; i < s.length(); ++i)
         {
            char ch(s[i]);
            switch(state)
            {
            case state_in_content:
               if(ch == '\\')
                  state = state_solidus;
               else
                  rtn.append(ch);
               break;

            case state_solidus:
               switch(ch)
               {
               case '\"':
               case '\\':
               case '/':
               case '\'':
                  state = state_in_content;
                  rtn.append(ch);
                  break;

               case 'b':
                  state = state_in_content;
                  rtn.append('\b');
                  break;

               case 'f':
                  state = state_in_content;
                  rtn.append('\f');
                  break;

               case 'n':
                  state = state_in_content;
                  rtn.append('\n');
                  break;

               case 'r':
                  state = state_in_content;
                  rtn.append('\r');
                  break;

               case 't':
                  state = state_in_content;
                  rtn.append('\t');
                  break;

               case 'u':
                  state = state_unicode;
                  unicode_buff.cut(0);
                  break;

               default:
                  throw std::invalid_argument("invalid escape sequence in string");
                  break;
               }
               break;

            case state_unicode:
               if(isxdigit(ch))
               {
                  if(unicode_buff.length() < 4)
                  {
                     unicode_buff.append(ch);
                     if(unicode_buff.length() == 4)
                     {
                        uint4 code_point(strtoul(unicode_buff.c_str(), 0, 16));
                        unicode_to_utf8(rtn, code_point);
                        state = state_in_content;
                     }
                  }
               }
               else
                  throw std::invalid_argument("invalid unicode sequence in string");
               break;
            }
               
         }
         return rtn;
      } // parse_json_string
   };

   
   CsvRec::CsvRec(bool json_strings_):
      last_character(0),
      within_quotes(false),
      passed_comma(false),
      value_was_quoted(false),
      json_strings(json_strings_)
   { }


   CsvRec::CsvRec(CsvRec const &other):
      std::vector<StrAsc>(other),
      quoted_positions(other.quoted_positions),
      values_count(other.values_count),
      last_character(other.last_character),
      within_quotes(other.within_quotes),
      passed_comma(other.passed_comma),
      value_was_quoted(other.value_was_quoted),
      json_strings(other.json_strings)
   { }


   CsvRec::~CsvRec()
   { }


   int CsvRec::read(std::istream &in, int limit)
   {
      // initialise the state machine to read the values
      last_character = '\0';
      passed_comma = within_quotes = value_was_quoted = false;
      current_value.cut(0);
      quoted_positions.clear();
      values_count = 0;
      clear();

      // start the read loop
      bool done = false;
      char ch;

      while(!done && in.get(ch))
         done = process_next_character(ch, limit);
      if(passed_comma || size() == 0)
      {
         ++values_count;
         if(limit < 0 || values_count <= limit)
         {
            if(value_was_quoted && json_strings)
               push_back(parse_json_string(current_value));
            else
               push_back(current_value);
            if(value_was_quoted)
               quoted_positions.push_back(end() - 1);
         }
      }
      return values_count;
   } // read


   int CsvRec::read(FILE *in, int limit)
   {
      // initialise the state machine to read the values
      last_character = '\0';
      passed_comma = within_quotes = value_was_quoted = false;
      current_value.cut(0);
      quoted_positions.clear();
      values_count = 0;
      clear();

      // enter the read loop
      char ch;
      bool done = false;

      while(!done && fread(&ch,sizeof(ch),1,in))
         done = process_next_character(ch, limit);
      if(passed_comma || size() == 0)
      {
         ++values_count;
         if(limit < 0 || values_count <= limit)
         {
            if(value_was_quoted && json_strings)
               push_back(parse_json_string(current_value));
            else
               push_back(current_value);
            if(value_was_quoted)
               quoted_positions.push_back(end() - 1);
         }
      }
      return values_count;
   } // read

   
   bool CsvRec::process_next_character(char ch, int limit)
   {
      bool rtn = false;
      if(last_character != '\\')
      {
         switch(ch)
         {
         case '\"':             // start quotation
            within_quotes = !within_quotes;
            if(within_quotes)
               value_was_quoted = true;
            break;
            
         case ',': 
            if(!within_quotes)
            {
               ++values_count;
               if(limit < 0 || values_count <= limit)
               {
                  if(value_was_quoted && json_strings)
                     push_back(parse_json_string(current_value));
                  else
                     push_back(current_value);
                  if(value_was_quoted)
                  {
                     quoted_positions.push_back(end() - 1);
                     value_was_quoted = false;
                  }
               }
               current_value.cut(0);
               passed_comma = true;
            }
            else
               current_value += ch;
            break;
            
         case '\\':             // escape the next character
            break;

         case '\r':             // ignore carraige returns (they can come from binary files)
            break;
            
         case '\n':
            rtn = true;
            break;
            
         default:
            current_value += ch;
            break;
         }
      }
      else
         current_value += ch;
      last_character = ch;
      return rtn;
   }


   bool CsvRec::was_quoted(const_iterator pos)
   {
      return std::binary_search(
         quoted_positions.begin(),
         quoted_positions.end(),
         pos);
   }


   CsvRecUni::CsvRecUni(bool json_strings_):
      last_character(0),
      within_quotes(false),
      passed_comma(false),
      value_was_quoted(false),
      json_strings(json_strings_)
   { }


   CsvRecUni::CsvRecUni(CsvRecUni const &other):
      std::vector<StrUni>(other),
      quoted_positions(other.quoted_positions),
      values_count(other.values_count),
      last_character(other.last_character),
      within_quotes(other.within_quotes),
      passed_comma(other.passed_comma),
      value_was_quoted(other.value_was_quoted),
      json_strings(other.json_strings)
   { }


   CsvRecUni::~CsvRecUni()
   { }


   int CsvRecUni::read(std::wistream &in, int limit)
   {
      // initialise the state machine to read the values
      last_character = 0;
      passed_comma = within_quotes = value_was_quoted = false;
      current_value.cut(0);
      quoted_positions.clear();
      values_count = 0;
      clear();

      // start the read loop
      bool done = false;
      wchar_t ch;

      while(!done && in.get(ch))
         done = process_next_character(ch, limit);
      if(passed_comma || size() == 0)
      {
         ++values_count;
         if(limit < 0 || values_count <= limit)
         {
            if(value_was_quoted && json_strings)
               push_back(parse_json_string(current_value.to_utf8()));
            else
               push_back(current_value);
            if(value_was_quoted)
               quoted_positions.push_back(end() - 1);
         }
      }
      return values_count;
   } // read


   bool CsvRecUni::process_next_character(wchar_t ch, int limit)
   {
      bool rtn = false;
      if(last_character != L'\\')
      {
         switch(ch)
         {
         case L'\"':             // start quotation
            within_quotes = !within_quotes;
            if(within_quotes)
               value_was_quoted = true;
            break;
            
         case L',': 
            if(!within_quotes)
            {
               ++values_count;
               if(limit < 0 || values_count <= limit)
               {
                  if(value_was_quoted && json_strings)
                     push_back(StrUni(parse_json_string(current_value.to_utf8())));
                  else
                     push_back(current_value);
                  if(value_was_quoted)
                  {
                     quoted_positions.push_back(end() - 1);
                     value_was_quoted = false;
                  }
               }
               current_value.cut(0);
               passed_comma = true;
            }
            else
               current_value += ch;
            break;
            
         case L'\\':             // escape the next character
            break;

         case L'\r':             // ignore carraige returns (they can come from binary files)
            break;
            
         case L'\n':
            rtn = true;
            break;
            
         default:
            current_value += ch;
            break;
         }
      }
      else
         current_value += ch;
      last_character = ch;
      return rtn;
   }


   bool CsvRecUni::was_quoted(const_iterator pos)
   {
      return std::binary_search(
         quoted_positions.begin(),
         quoted_positions.end(),
         pos);
   }
};

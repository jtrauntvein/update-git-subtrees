/* Csi.IniFile.cpp

   Copyright (C) 2009, 2013 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 14 September 2009
   Last Change: Wednesday 10 April 2013
   Last Commit: $Date: 2013-04-10 12:50:33 -0600 (Wed, 10 Apr 2013) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.IniFile.h"
#include "Csi.BuffStream.h"


namespace Csi
{
   ////////////////////////////////////////////////////////////
   // class IniFile definitions
   ////////////////////////////////////////////////////////////
   void IniFile::parse(std::istream &in)
   {
      StrAsc section_name;
      StrAsc key;
      enum state_type
      {
         state_begin,
         state_in_section,
         state_before_key,
         state_in_key,
         state_in_value,
         state_complete
      } state = state_begin;

      sections.clear();
      while(in && state != state_complete)
      {
         char ch;
         in.get(ch);
         if(!in)
            state = state_complete;
         switch(state)
         {
         case state_begin:
            if(ch == '[')
            {
               section_name.cut(0);
               state = state_in_section;
            }
            break;
            
         case state_in_section:
            if(ch == ']')
            {
               sections.push_back(value_type(section_name, keys_type()));
               state = state_before_key;
               section_name.cut(0);
            }
            else
               section_name.append(ch);
            break;
            
         case state_before_key:
            if(ch == '\n')
               state = state_in_key;
            break;
            
         case state_in_key:
            switch(ch)
            {
            case '\r':
            case '\n':
               if(key.length() > 0 && !sections.empty())
               {
                  key.cut(0);
                  sections.back().second.push_back(key_type(key, StrAsc()));
                  state = state_in_key;
               }
               break;

            case '=':
               if(key.length() > 0 && !sections.empty())
               {
                  sections.back().second.push_back(key_type(key, StrAsc()));
                  state = state_in_value;
                  key.cut(0);
               }
               else
                  state = state_begin; 
               break;

            case '[':
               state = state_in_section;
               break;
               
            default:
               key.append(ch);
               break;
            }
            break;
               
         case state_in_value:
            switch(ch)
            {
            case '\n':
            case '\r':
               state = state_in_key;
               break;

            default:
               sections.back().second.back().second.append(ch);
               break;
            }
            break;
            
         default:
            state = state_complete;
            break;
         }
      }
   } // parse


   void IniFile::parse(void const *buff, size_t buff_len)
   {
      IBuffStream in(static_cast<char const *>(buff), buff_len);
      parse(in);
   } // parse


   StrAsc IniFile::get_key_value(StrAsc const &section, StrAsc const &key)
   {
      StrAsc rtn;
      for(sections_type::iterator si = sections.begin(); si != sections.end(); ++si)
      {
         if(si->first == section)
         {
            for(keys_type::iterator ki = si->second.begin(); ki != si->second.end(); ++ki)
            {
               if(ki->first == key)
               {
                  rtn = ki->second;
                  break;
               }
            }
            break;
         }
      }
      return rtn;
   }
};


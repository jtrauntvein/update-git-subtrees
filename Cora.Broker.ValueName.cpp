/* Cora.Broker.ValueName.cpp

   Copyright (C) 2003, 2014 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 08 January 2003
   Last Change: Friday 14 March 2014
   Last Commit: $Date: 2014-03-14 11:42:00 -0600 (Fri, 14 Mar 2014) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Broker.ValueName.h"
#include "Csi.MaxMin.h"
#include "Csi.StrAscStream.h"
#include <wctype.h>


namespace Cora
{
   namespace Broker
   {
      ////////////////////////////////////////////////////////////
      // class ValueName definitions
      ////////////////////////////////////////////////////////////
      ValueName::ValueName()
      { }

      
      ValueName::ValueName(char const *s)
      { parse(StrUni(s, true).c_str()); }
      
         
      ValueName::ValueName(wchar_t const *s)
      { parse(s); }


      ValueName::ValueName(StrUni const &s)
      { parse(s.c_str()); }

      
      ValueName::ValueName(ValueName const &other):
         full_name(other.full_name),
         station_name(other.station_name),
         table_name(other.table_name),
         full_column_name(other.full_column_name),
         column_name(other.column_name),
         subscripts(other.subscripts)
      { }


      ValueName &ValueName::operator =(ValueName const &other)
      {
         full_name = other.full_name;
         station_name = other.station_name;
         table_name = other.table_name;
         full_column_name = other.full_column_name;
         column_name = other.column_name;
         subscripts = other.subscripts;
         return *this;
      } // copy operator

      
      ValueName &ValueName::operator =(char const *s)
      {
         parse(StrUni(s, true).c_str());
         return *this;
      } // string copy operator


      ValueName &ValueName::operator =(wchar_t const *s)
      {
         parse(s);
         return *this;
      } // wide string copy operator


      ValueName &ValueName::operator =(StrUni const &s)
      {
         parse(s.c_str());
         return *this;
      }

      
      int ValueName::compare(ValueName const &other) const
      {
         int rtn = station_name.compare(other.station_name, false);
         if(rtn == 0)
         {
            rtn = table_name.compare(other.table_name, false);
            if(rtn == 0)
            {
               rtn = column_name.compare(other.column_name, false);
               if(rtn == 0)
               {
                  if(subscripts < other.subscripts)
                     rtn = -1;
                  else if(subscripts == other.subscripts)
                     rtn = 0;
                  else
                     rtn = 1;
               }
            }
         }
         return rtn;
      } // compare


      void ValueName::parse(wchar_t const *s)
      {
         // initialise the components
         station_name.cut(0);
         table_name.cut(0);
         full_column_name.cut(0);
         column_name.cut(0);
         subscripts.clear();
         
         // we'll make a copy of the name.  We can then use the search facilities in StrUni to break
         // the full name into its component parts.
         full_name = StrUni(s);
         size_t dot_pos = full_name.findRev(L".");
         size_t last_dot_pos = full_name.length();
         while(dot_pos > 0 && dot_pos < full_name.length())
         {
            if(column_name.length() == 0)
               full_name.sub(column_name,dot_pos + 1,last_dot_pos);
            else if(table_name.length() == 0)
               full_name.sub(table_name,dot_pos+1,last_dot_pos-dot_pos - 1);
            else if(station_name.length() == 0)
               full_name.sub(station_name,dot_pos+1,last_dot_pos-dot_pos - 1);
            else
            {
               column_name.insert(table_name.c_str(),0);
               table_name = station_name;
               station_name.cut(0);
               full_name.sub(station_name,dot_pos+1,last_dot_pos-dot_pos - 1);
            }

            last_dot_pos = dot_pos;
            dot_pos = full_name.findRev(L".", dot_pos - 1);
         }

         if(dot_pos == full_name.length())
         {
            if(column_name.length())
            {
               if(table_name.length())
               {
                  if(station_name.length())
                  {
                     column_name.insert(L".",0);
                     column_name.insert(table_name.c_str(),0);
                     table_name = station_name;
                     station_name.cut(0);
                     full_name.sub(station_name,0,last_dot_pos);
                  }
                  else
                  {
                     station_name.cut(0);
                     full_name.sub(station_name,0,last_dot_pos);
                  }
               }
               else
               {
                  table_name.cut(0);
                  full_name.sub(table_name,0,last_dot_pos);
               }
            }
            else
            {
               column_name.cut(0);
               full_name.sub(column_name,0,last_dot_pos);
            }
         }

         // remove any preceding or trailing blanks on the tokens
         while(station_name.length() && std::isspace(station_name.first()))
            station_name.cut(0,1);
         while(table_name.length() && std::isspace(table_name.first()))
            table_name.cut(0,1);
         while(column_name.length() && std::isspace(column_name.first()))
            column_name.cut(0,1);
         while(station_name.length() && std::isspace(station_name.last()))
            station_name.cut(station_name.length() - 1);
         while(table_name.length() && std::isspace(table_name.last()))
            table_name.cut(table_name.length() - 1);

         // we are now ready to break the parse the column name
         enum state_type {
            state_in_name,
            state_before_subscript,
            state_in_subscript,
            state_after_subscript,
            state_complete
         } state = state_in_name;
         bool advance_i;
         StrUni temp;
         size_t i = 0;
         size_t subscripts_begin = column_name.length();

         while(i < column_name.length() && state != state_complete)
         {
            advance_i = true;
            switch(state)
            {
            case state_in_name:
               if(column_name[i] == L'(')
               {
                  state = state_before_subscript;
                  subscripts_begin = i;
               }
               break;

            case state_before_subscript:
               switch(column_name[i])
               {
               case L'0':
               case L'1':
               case L'2':
               case L'3':
               case L'4':
               case L'5':
               case L'6':
               case L'7':
               case L'8':
               case L'9':
                  state = state_in_subscript;
                  advance_i = false;
                  temp.cut(0);
                  break;

               default:
                  if(!isspace(column_name[i]))
                     throw exc_invalid_syntax();
                  break;
               }
               break;

            case state_in_subscript:
               switch(column_name[i])
               {
               case L'0':
               case L'1':
               case L'2':
               case L'3':
               case L'4':
               case L'5':
               case L'6':
               case L'7':
               case L'8':
               case L'9':
                  temp.append(column_name[i]);
                  break;

               case L',':
                  if(temp.length())
                     subscripts.push_back(wcstoul(temp.c_str(),0,10));
                  else
                     throw exc_invalid_syntax();
                  state = state_before_subscript;
                  break;
                  
               case L')':
                  if(temp.length())
                     subscripts.push_back(wcstoul(temp.c_str(),0,10));
                  else
                     throw exc_invalid_syntax();
                  state = state_complete;
                  break;
                  
               default:
                  if(isspace(column_name[i]) && temp.length())
                  {
                     subscripts.push_back(wcstoul(temp.c_str(),0,10));
                     state = state_after_subscript;
                  }
                  else
                     throw exc_invalid_syntax();
                  break;
               }
               break;

            case state_after_subscript:
               if(column_name[i] == ',')
                  state = state_before_subscript;
               else if(!isspace(column_name[i]))
                  throw exc_invalid_syntax();
               break;
            }

            // advance the subscript
            if(advance_i)
               ++i;
         }

         // now that the subscripts have been parsed, we can trim the subscripts from the column
         // name and remove any trailing whitespace
         column_name.cut(subscripts_begin);
         while(column_name.length() && isspace(column_name.last()))
            column_name.cut(column_name.length() - 1);
         if(column_name.length())
         {
            if(station_name.length() > 0 && table_name.length() == 0)
               throw exc_invalid_syntax();
         }
         else
            throw exc_invalid_syntax();

         // we can now paste the full column name together.
         Csi::OStrAscStream str;
         str.imbue(std::locale::classic());
         str << column_name;
         if(!subscripts.empty())
         {
            str << "(";
            for(subscripts_type::iterator si = subscripts.begin(); si != subscripts.end(); ++si)
            {
               if(si != subscripts.begin())
                  str << ",";
               str << *si;
            }
            str << ")";
         }
         full_column_name = str.str();
      } // parse
   };
};


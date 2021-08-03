/* Csi.Json.cpp

   Copyright (C) 2010, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 25 March 2010
   Last Change: Thursday 13 August 2020
   Last Commit: $Date: 2020-08-13 10:27:58 -0600 (Thu, 13 Aug 2020) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Json.h"
#include "CsiTypes.h"
#include "Csi.BuffStream.h"
#include "Csi.Utils.h"
#include "Csi.Base64.h"
#include <algorithm>
#include <iostream>
#include <iomanip>


namespace Csi
{
   namespace Json
   {
      namespace
      {
         struct property_has_name
         {
            StrAsc const &name;
            property_has_name(StrAsc const &name_):
               name(name_)
            { }

            bool operator ()(Object::value_type const &attr) const
            { return attr.first == name; }
         };


         std::locale json_locale(std::locale::classic());


         void maintain_parse_pos(char ch, int *row_no, int *column_no)
         {
            if(column_no)
               ++(*column_no);
            if(row_no && ch == '\n')
            {
               if(column_no)
                  *column_no = 0;
               ++(*row_no);
            }
         }
      };


      format_string::format_string(StrAsc const &s, bool convert_from_multi):
         content(s, convert_from_multi)
      { }

      
      format_string::format_string(StrUni const &s)
      {
         content = s.to_utf8();
      } // unicode string constructor


      void format_string::operator ()(std::ostream &out) const
      {
         for(size_t i = 0; i < content.length(); ++i)
         {
            switch(content[i])
            {
            case L'\0':
               out << "\\u0000";
               break;
               
            case L'\\':
               out << "\\\\";
               break;
               
            case L'\"':
               out << "\\\"";
               break;
               
            case L'\b':
               out << "\\b";
               break;
               
            case L'\f':
               out << "\\f";
               break;
               
            case L'\n':
               out << "\\n";
               break;
               
            case L'\r':
               out << "\\r";
               break;
               
            case L'\t':
               out << "\\t";
               break;

            default:
               if(content[i] >= 0x20 && content[i] < 0x7f)
                  out << static_cast<char>(content[i]);
               else
               {
                  std::ios_base::fmtflags flags(out.flags());
                  out << "\\u" << std::setw(4) << std::setfill('0') << std::hex
                      << static_cast<uint4>(content[i]);
                  out.flags(flags);
               }
               break;
            }
         }
      } // format_string


      void format_string::operator ()(std::wostream &out) const
      {
         for(size_t i = 0; i < content.length(); ++i)
         {
            switch(content[i])
            {
            case L'\0':
               out << L"\\u0000";
               break;
               
            case L'\\':
               out << L"\\\\";
               break;
               
            case L'\"':
               out << L"\\\"";
               break;
               
            case L'\b':
               out << L"\\b";
               break;
               
            case L'\f':
               out << L"\\f";
               break;
               
            case L'\n':
               out << L"\\n";
               break;
               
            case L'\r':
               out << L"\\r";
               break;
               
            case L'\t':
               out << L"\\t";
               break;

            default:
               if(content[i] >= 0x20 && content[i] < 0x7f)
                  out << content[i];
               else
               {
                  std::ios_base::fmtflags flags(out.flags());
                  out << L"\\u" << std::setw(4) << std::setfill(L'0') << std::hex
                      << static_cast<uint4>(content[i]);
                  out.flags(flags);
               }
               break;
            }
         }
      } // format_string (wide version)


      std::ostream &operator <<(std::ostream &out, format_string const &s)
      {
         s(out);
         return out;
      } // output operator


      std::wostream &operator <<(std::wostream &out, format_string const &s)
      {
         s(out);
         return out;
      } // wide stream output operator


      parse_exception::parse_exception(StrAsc const &s, int *row_no, int *column_no)
      {
         Csi::OStrAscStream temp;
         temp << "JSON Parsing Exception";
         if(row_no && column_no) 
            temp << " at (" << (row_no + 1) << ", " << (column_no + 1) << ")";
         temp << ": " << s;
         message = temp.str();
      } // constructor

      
      Object *Object::clone() const
      {
         Object *rtn(new Object);
         for(const_iterator vi = begin(); vi != end(); ++vi)
            rtn->push_back(value_type(vi->first, vi->second->clone()));
         return rtn;
      } // clone

      
      void Object::format(
         std::ostream &out, bool do_indent, int indent_level) const
      {
         out << "{";
         for(const_iterator vi = begin(); vi != end(); ++vi)
         {
            if(do_indent)
            {
               out << "\n";
               for(int i = 0; i < indent_level + 1; ++i)
                  out << "  ";
            }
            out << "\"" << vi->first << "\": ";
            if(vi->second != 0)
               vi->second->format(out, do_indent, indent_level + 1);
            else
               out << "null";
            if(vi + 1 != end())
               out << ",";
         }
         if(do_indent)
         {
            out << "\n";
            for(int i = 0; i < indent_level; ++i)
               out << "  ";
         }
         out << "}";
      } // format


      char Object::parse(std::istream &in, char start_char, int *row_no, int *column_no)
      {
         // initialise our internal state
         enum state_type
         {
            state_before_open,
            state_before_value_name,
            state_after_value_name,
            state_before_value,
            state_after_value,
            state_end
         } state(state_before_open); 
         if(start_char == '{')
            state = state_before_value_name;
         values.clear();
         
         // we are now ready to parse the input
         char ch = 0;
         String value_name;
         value_handle value;
         while(state != state_end && in.get(ch))
         {
            maintain_parse_pos(ch, row_no, column_no);
            switch(state)
            {
            case state_before_open:
               if(ch == '{')
                  state = state_before_value_name;
               else if(!isspace(ch))
                  throw parse_exception("opening brace not found", row_no, column_no);
               break;
               
            case state_before_value_name:
               if(ch == '\"')
               {
                  try
                  {
                     value_name.parse(in, ch, row_no, column_no);
                     state = state_after_value_name;
                  }
                  catch(std::exception &)
                  { throw parse_exception("a value name was not terminated", row_no, column_no); }
               }
               else if(ch == '}')
                  state = state_end;
               else if(!isspace(ch))
                  throw parse_exception("value name not found", row_no, column_no);
               break;
               
            case state_after_value_name:
               if(ch == ':')
                  state = state_before_value;
               else if(!isspace(ch))
                  throw parse_exception(StrAsc("error before value: ") + value_name.get_value(), row_no, column_no);
               break;

            case state_before_value:
               // we will use the first character to determine the type of the value
               if(isdigit(ch) || ch == '+' || ch == '-' || ch == '.')
                  value.bind(new Number);
               else if(ch == '\"')
                  value.bind(new String);
               else if(ch == '{')
                  value.bind(new Object);
               else if(ch == '[')
                  value.bind(new Array);
               else if(ch == 'n')
                  value.bind(new Null);
               else if(ch == 't' || ch == 'f')
                  value.bind(new Boolean);
               else if(!isspace(ch))
                  throw parse_exception(StrAsc("invalid value type for: ") + value_name.get_value(), row_no, column_no);
               
               // we will now invoke that value's parse method.  After that, we will use the follow
               // on character to determine the next state
               if(value != 0)
               {
                  ch = value->parse(in, ch, row_no, column_no);
                  set_property(value_name.get_value(), value, false);
                  if(ch == ',')
                     state = state_before_value_name;
                  else if(ch == '}' && value->get_type() != value_object)
                     state = state_end;
                  else 
                     state = state_after_value;
                  value.clear();
               }
               break;

            case state_after_value:
               if(ch == ',')
                  state = state_before_value_name;
               else if(ch == '}')
                  state = state_end;
               else if(!isspace(ch))
                  throw parse_exception("invalid post-value termination", row_no, column_no);
               break;
            }
         }

         // if we made it this far but are not in the end state, we will assume that the input ran
         // out prematurely
         if(state != state_end)
            throw parse_exception("premature end of object input", row_no, column_no);
         return ch;
      } // parse


      Object::iterator Object::find(StrAsc const &name)
      {
         return std::find_if(values.begin(), values.end(), property_has_name(name));
      } // find


      Object::const_iterator Object::find(StrAsc const &name) const
      {
         return std::find_if(values.begin(), values.end(), property_has_name(name));
      } // find
      

      Object::value_handle &Object::operator [](StrAsc const &name)
      {
         values_type::iterator pi(
            std::find_if(values.begin(), values.end(), property_has_name(name)));
         if(pi == values.end())
         {
            values.push_back(value_type(name, value_handle()));
            pi = values.end() - 1;
         }
         return pi->second;
      } // associative operator


      Object::value_handle const &Object::operator [](StrAsc const &name) const
      {
         values_type::const_iterator pi(
            std::find_if(values.begin(), values.end(), property_has_name(name)));
         if(pi == values.end())
            throw std::out_of_range("property does not exist");
         return pi->second;
      } // associative operator


      Object::iterator Object::set_property(StrAsc const &name, value_handle value, bool check_existing)
      {
         values_type::iterator pi(values.end());
         if(check_existing)
            pi = std::find_if(values.begin(), values.end(), property_has_name(name)); 
         if(pi == values.end())
         {
            values.push_back(value_type(name, value));
            pi = values.end() - 1;
         }
         else
            pi->second = value;
         return pi;
      } // set_property


      void Object::remove_property(StrAsc const &name)
      {
         values_type::iterator pi(
            std::find_if(values.begin(), values.end(), property_has_name(name)));
         if(pi != values.end())
            values.erase(pi);
      } // remove_property


      double Object::get_property_number(StrAsc const &name)
      {
         double rtn(0);
         auto pi(find(name));
         if(pi != end())
         {
            if(pi->second->get_type() == value_number)
            {
               NumberHandle val(pi->second);
               rtn = val->get_value();
            }
            else if(pi->second->get_type() == value_bool)
            {
               BooleanHandle val(pi->second);
               rtn = val->get_value() ? -1 : 0;
            }
            else if(pi->second->get_type() == value_string)
            {
               StringHandle val(pi->second);
               rtn = csiStringToFloat(val->get_value().c_str(), json_locale, true);
            }
            else if(pi->second->get_type() == value_null)
               rtn = std::numeric_limits<double>::quiet_NaN();
            else
               throw std::invalid_argument("property not convertable");
         }
         else
            throw std::out_of_range("property not found");
         return rtn;
      } // get_property_number
      

      StrAsc Object::get_property_str(StrAsc const &name)
      {
         iterator pi(find(name));
         StrAsc rtn;
         if(pi != end())
         {
            if(pi->second->get_type() == value_string)
            {
               StringHandle val(pi->second);
               rtn = val->get_value();
            }
            else
            {
               Csi::OStrAscStream temp;
               temp.imbue(json_locale);
               pi->second->format(temp, false);
               rtn = temp.str();
            }
         }
         else
            throw std::out_of_range("property not found");
         return rtn;
      } // get_property_str


      LgrDate Object::get_property_date(StrAsc const &name)
      {
         LgrDate rtn;
         iterator pi(find(name));
         if(pi != end())
         {
            if(pi->second->get_type() == value_string)
            {
               String *string_val(dynamic_cast<String *>(pi->second.get_rep()));
               if(string_val != 0)
                  rtn = LgrDate::fromStr(string_val->get_value().c_str());
               else
               {
                  DateHandle date_val(pi->second);
                  rtn = date_val->get_value();
               }
            }
            else if(pi->second->get_type() == value_number)
            {
               NumberHandle val(pi->second);
               rtn = static_cast<int8>(val->get_value());
            }
            else
               throw std::invalid_argument("property not convertable");
         }
         else
            throw std::out_of_range("property not found");
         return rtn;
      } // get_property_date

      
      Array *Array::clone() const
      {
         Array *rtn(new Array);
         for(const_iterator vi = begin(); vi != end(); ++vi)
         {
            value_type value((*vi)->clone());
            rtn->push_back(value);
         }
         return rtn;
      } // clone


      void Array::format(
         std::ostream &out, bool do_indent, int indent_level) const
      {
         out << "[";
         for(const_iterator vi = values.begin(); vi != values.end(); ++vi)
         {
            value_type const &value(*vi);
            if(do_indent)
            {
               out << "\n";
               for(int i = 0; i < indent_level + 1; ++i)
                  out << "  ";
            }
            if(value != 0)
               value->format(out, do_indent, indent_level + 1);
            else
               out << "null";
            if(vi + 1 != end())
               out << ",";
         }
         if(do_indent)
         {
            out << "\n";
            for(int i = 0; i < indent_level; ++i)
               out << "  ";
         }
         out << "]";
      } // format


      char Array::parse(std::istream &in, char last_char, int *row_no, int *column_no)
      {
         // initialise our state
         enum state_type
         {
            state_before_array,
            state_before_value,
            state_after_value,
            state_complete
         } state(state_before_array);
         if(last_char == '[')
            state = state_before_value;
         else if(last_char != 0 && !isspace(last_char))
            throw parse_exception("invalid character preceding an array", row_no, column_no);

         // enter the parsing loop
         char ch;
         value_type value;

         values.clear();
         while(state != state_complete && in.get(ch))
         {
            maintain_parse_pos(ch, row_no, column_no);
            switch(state)
            {
            case state_before_array:
               if(ch == '[')
                  state = state_before_value;
               else if(!isspace(ch))
                  throw parse_exception("invalid character preceding an array", row_no, column_no);
               break;
               
            case state_before_value:
               if(ch == '{')
                  value.bind(new Object);
               else if(ch == '[')
                  value.bind(new Array);
               else if(ch == '\"')
                  value.bind(new String);
               else if(ch == 't' || ch == 'f')
                  value.bind(new Boolean);
               else if(ch == 'n')
                  value.bind(new Null);
               else if(isdigit(ch) || ch == '+' || ch == '-' || ch == '.')
                  value.bind(new Number);
               else if(ch == ']')
                  state = state_complete;
               else if(!isspace(ch))
                  throw parse_exception("invalid character between array values", row_no, column_no);

               // we need to parse the value if the state applies
               if(value != 0)
               {
                  ch = value->parse(in, ch, row_no, column_no);
                  values.push_back(value);
                  if(value->get_type() != value_array && ch == ']')
                     state = state_complete;
                  else if(ch != ',')
                     state = state_after_value;
                  value.clear();
               } 
               break;

            case state_after_value:
               if(ch == ',')
                  state = state_before_value;
               else if(ch == ']')
                  state = state_complete;
               else if(!isspace(ch))
                  throw parse_exception("invalid character following array values", row_no, column_no);
               break;
            }
         }
         if(state != state_complete)
            throw parse_exception("invalid array syntax", row_no, column_no);
         return ch;
      } // parse
      

      ////////////////////////////////////////////////////////////
      // class String definitions
      ////////////////////////////////////////////////////////////
      void String::format(
         std::ostream &out, bool do_indent, int indent_level) const
      {
         out << "\"" << format_string(value) << "\"";
      } // format


      char String::parse(std::istream &in, char last_char, int *row_no, int *column_no)
      {
         // initialise the state
         enum state_type
         {
            state_before_begin,
            state_in_content,
            state_solidus,
            state_unicode,
            state_complete
         } state(state_before_begin);
         if(last_char == '\"')
            state = state_in_content;
         else if(!isspace(last_char))
            throw parse_exception("invalid string syntax", row_no, column_no);

         // enter the parsing loop
         char ch;
         StrAsc unicode_buff;
         value.cut(0);
         
         while(state != state_complete && in.get(ch))
         {
            maintain_parse_pos(ch, row_no, column_no);
            switch(state)
            {
            case state_before_begin:
               if(ch == '\"')
                  state = state_in_content;
               else if(!isspace(ch))
                  throw parse_exception("invalid character preceding a string value.", row_no, column_no);
               break;
               
            case state_in_content:
               if(ch == '\"')
                  state = state_complete;
               else if(ch == '\\')
                  state = state_solidus;
               else
                  value.append(ch);
               break;
               
            case state_solidus:
               switch(ch)
               {
               case '\"':
                  state = state_in_content;
                  value.append(ch);
                  break;
                  
               case '\\':
                  state = state_in_content;
                  value.append(ch);
                  break;
                  
               case '/':
                  state = state_in_content;
                  value.append(ch);
                  break;
                  
               case 'b':
                  state = state_in_content;
                  value.append('\b');
                  break;
                  
               case 'f':
                  state = state_in_content;
                  value.append('\f');
                  break;
                  
               case 'n':
                  state = state_in_content;
                  value.append('\n');
                  break;
                  
               case 'r':
                  state = state_in_content;
                  value.append('\r');
                  break;
                  
               case 't':
                  state = state_in_content;
                  value.append('\t');
                  break;

               case 'u':
                  state = state_unicode;
                  unicode_buff.cut(0);
                  break;

               case '\'':
                  state = state_in_content;
                  unicode_buff.append('\'');
                  break;
                  
               default:
                  throw parse_exception("invalid escape sequence in a string", row_no, column_no);
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
                        unicode_to_utf8(value, code_point);
                        state = state_in_content;
                     }
                  }
               }
               else
                  throw parse_exception("invalid unicode sequence in a string", row_no, column_no);
               break; 
            }
         }
         if(state != state_complete)
            throw parse_exception("improperly terminated string value", row_no, column_no);
         return ch;
      } // parse


      ////////////////////////////////////////////////////////////
      // class Number definitions
      ////////////////////////////////////////////////////////////
      void Number::format(
         std::ostream &out, bool do_indent, int indent_level) const
      { csiFloatToStream(out, value, 15, true, 15); }


      char Number::parse(std::istream &in, char start_char, int *row_no, int *column_no)
      {
         // initialise the state machine
         enum state_type
         {
            state_before_value,
            state_in_sign,
            state_in_int,
            state_in_fract,
            state_before_exp,
            state_in_exp_sign,
            state_in_exp_value,
            state_complete
         } state(state_before_value);
         StrAsc buff;
         if(start_char == '-' || start_char == '+')
            state = state_in_sign;
         else if(isdigit(start_char))
            state = state_in_int;
         else if(start_char == '.')
            state = state_in_fract;
         else if(start_char != 0 && !isspace(start_char))
            throw parse_exception("invalid start of number", row_no, column_no);
         if(start_char != 0)
            buff.append(start_char);

         // we can now carry out the state machine
         char ch;
         while(state != state_complete && in.get(ch))
         {
            maintain_parse_pos(ch, row_no, column_no);
            switch(state)
            {
            case state_before_value:
               if(ch == '-' || ch == '+')
                  state = state_in_sign;
               else if(isdigit(ch))
                  state = state_in_int;
               else if(ch == '.')
                  state = state_in_fract;
               else if(!isspace(ch))
                  throw parse_exception("invalid character preceding a number", row_no, column_no);
               buff.append(ch);
               break;
               
            case state_in_sign:
               if(isdigit(ch))
                  state = state_in_int;
               else if(ch == '.')
                  state = state_in_fract;
               else
                  throw parse_exception("invalid character following the numeric sign", row_no, column_no);
               buff.append(ch);
               break;
               
            case state_in_int:
               if(ch == '.')
               {
                  buff.append(ch);
                  state = state_in_fract;
               }
               else if(ch == 'e' || ch == 'E')
               {
                  buff.append(ch);
                  state = state_before_exp;
               }
               else if(isspace(ch) || ch == ',' || ch == ']' || ch == '}')
                  state = state_complete;
               else if(isdigit(ch))
                  buff.append(ch);
               else
                  throw parse_exception("invalid character within the integer portion of a number", row_no, column_no);
               break;
               
            case state_in_fract:
               if(isdigit(ch))
                  buff.append(ch);
               else if(ch == 'e' || ch == 'E')
               {
                  buff.append(ch);
                  state = state_before_exp;
               }
               else if(isspace(ch) || ch == ',' || ch == ']' || ch == '}')
               {
                  buff.append(ch);
                  state = state_complete;
               }
               else
                  throw parse_exception("invalid character within the fractional part of a number", row_no, column_no);
               break;
               
            case state_before_exp:
               if(ch == '+' || ch == '-')
               {
                  buff.append(ch);
                  state = state_in_exp_sign;
               }
               else if(isdigit(ch))
               {
                  buff.append(ch);
                  state = state_in_exp_value;
               }
               else
                  throw parse_exception("invalid character within the exponent of a number", row_no, column_no);
               break;
               
            case state_in_exp_sign:
               if(isdigit(ch))
               {
                  buff.append(ch);
                  state = state_in_exp_value;
               }
               else
                  throw parse_exception("invalid character within a number exponent", row_no, column_no);
               break;
               
            case state_in_exp_value:
               if(isspace(ch) || ch == ',' || ch == ']' || ch == '}')
                  state = state_complete;
               else if(!isdigit(ch))
                  throw parse_exception("invalid character within a number exponent", row_no, column_no);
               else
                  buff.append(ch);
               break;
            }
            if(state == state_complete)
            {
               Csi::IBuffStream temp(buff.c_str(), buff.length());
               temp.imbue(json_locale);
               temp >> value; 
            }
         }
         if(state != state_complete)
            throw parse_exception("syntax error parsing a number", row_no, column_no);
         return ch;
      } // parse
      

      ////////////////////////////////////////////////////////////
      // class Boolean definitions
      ////////////////////////////////////////////////////////////
      void Boolean::format(
         std::ostream &out, bool do_indent, int indent_level) const
      {
         if(value)
            out << "true";
         else
            out << "false";
      } // format


      char Boolean::parse(std::istream &in, char last_char, int *row_no, int *column_no)
      {
         // initialise the state of this parser
         enum state_type
         {
            state_before_value,
            state_true_t,
            state_true_r,
            state_true_u,
            state_true_e,
            state_false_f,
            state_false_a,
            state_false_l,
            state_false_s,
            state_false_e,
            state_complete
         } state(state_before_value);
         if(last_char == 't')
            state = state_true_t;
         else if(last_char == 'f')
            state = state_false_f;
         else if(!isspace(last_char))
            throw parse_exception("invalid character before a boolean", row_no, column_no);

         // process the input
         char ch;
         while(state != state_complete && in.get(ch))
         {
            maintain_parse_pos(ch, row_no, column_no);
            switch(state)
            {
            case state_before_value:
               if(ch == 't')
                  state = state_true_t;
               else if(ch == 'f')
                  state = state_false_f;
               else
                  throw parse_exception("invalid boolean syntax", row_no, column_no);
               break;
               
            case state_true_t:
               if(ch == 'r')
                  state = state_true_r;
               else
                  throw parse_exception("invalid boolean syntax", row_no, column_no);
               break;
               
            case state_true_r:
               if(ch == 'u')
                  state = state_true_u;
               else
                  throw parse_exception("invalid boolean syntax", row_no, column_no); 
               break;
               
            case state_true_u:
               if(ch == 'e')
                  state = state_true_e;
               else
                  throw parse_exception("invalid boolean syntax", row_no, column_no);
               break;
               
            case state_true_e:
               if(isspace(ch) || ch == ',' || ch == '}' || ch == ']')
               {
                  value = true;
                  state = state_complete;
               }
               else
                  throw parse_exception("invalid boolean syntax", row_no, column_no);
               break;
               
            case state_false_f:
               if(ch == 'a')
                  state = state_false_a;
               else
                  throw parse_exception("invalid boolean syntax", row_no, column_no); 
               break;
               
            case state_false_a:
               if(ch == 'l')
                  state = state_false_l;
               else
                  throw parse_exception("invalid boolean syntax",row_no, column_no);
               break;
               
            case state_false_l:
               if(ch == 's')
                  state = state_false_s;
               else
                  throw parse_exception("invalid boolean syntax", row_no, column_no);
               break;
               
            case state_false_s:
               if(ch == 'e')
                  state = state_false_e;
               else
                  throw parse_exception("invalid boolean syntax", row_no, column_no);
               break;
               
            case state_false_e:
               if(isspace(ch) || ch == ',' || ch == ']' || ch == '}')
               {
                  value = false;
                  state = state_complete;
               }
               else
                  throw parse_exception("invalid boolean syntax", row_no, column_no);
               break;
            }
         }
         if(state != state_complete)
            throw parse_exception("invalid boolean syntax", row_no, column_no);
         return ch;
      } // parse


      ////////////////////////////////////////////////////////////
      // class Date definitions
      ////////////////////////////////////////////////////////////
      void Date::format(
         std::ostream &out, bool do_indent, int indent_level) const
      {
         date.format(out, "\"%Y-%m-%dT%H:%M:%S%x\"");
      } // format (date)


      void BlobFile::format(std::ostream &out, bool do_indent, int indent_level) const
      {
         FILE *input(Csi::open_file(file_name, "rb"));
         char input_buff[1023]; // sized as a multiple of three so we avoid padding except at the end
         out << "\"";
         if(input)
         {
            size_t rcd(fread(input_buff, 1, sizeof(input_buff), input));
            while(rcd > 0)
            {
               Base64::encode(out, input_buff, rcd, false);
               if(rcd == sizeof(input_buff))
                  rcd = fread(input_buff, 1, sizeof(input_buff), input);
               else
                  rcd = 0;
            }
            fclose(input);
         }
         out << "\"";
      } // format

      
      ////////////////////////////////////////////////////////////
      // class Null definitions
      ////////////////////////////////////////////////////////////
      void Null::format(
         std::ostream &out, bool do_indent, int indent_level) const
      { out << "null"; }


      char Null::parse(std::istream &in, char last_char, int *row_no, int *column_no)
      {
         // set up the parsing syntax
         enum state_type
         {
            state_before_value,
            state_n,
            state_u,
            state_l1,
            state_l2,
            state_complete
         } state(state_before_value);
         if(last_char == 'n')
            state = state_n;
         else if(!isspace(last_char))
            throw parse_exception("invalid null syntax", row_no, column_no);

         // parse the value
         char ch;
         while(state != state_complete && in.get(ch))
         {
            switch(state)
            {
            case state_before_value:
               if(ch == 'n')
                  state = state_n;
               else if(!isspace(ch))
                  throw parse_exception("invalid null syntax", row_no, column_no); 
               break;
               
            case state_n:
               if(ch == 'u')
                  state = state_u;
               else
                  throw parse_exception("invalid null syntax", row_no, column_no); 
               break;
               
            case state_u:
               if(ch == 'l')
                  state = state_l1;
               else
                  throw parse_exception("invalid null syntax", row_no, column_no); 
               break;
               
            case state_l1:
               if(ch == 'l')
                  state = state_l2;
               else
                  throw parse_exception("invalid null syntax", row_no, column_no);
               break;
               
            case state_l2:
               if(isspace(ch) || ch == ',' || ch == ']' || ch == '}')
                  state = state_complete;
               else
                  throw parse_exception("invalid null syntax", row_no, column_no);
               break; 
            }
         }
         if(state != state_complete)
            throw parse_exception("invalid null syntax", row_no, column_no);
         return ch;
      } // parse
   };
};


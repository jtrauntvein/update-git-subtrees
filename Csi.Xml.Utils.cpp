/* Csi.Xml.Utils.cpp

   Copyright (C) 2016, Campbell Scientific, Inc

   Written by: Jon Trauntvein 
   Date Begun: Tuesday 01 March 2016
   Last Change: Tuesday 01 March 2016
   Last Commit: $Date: 2016-03-03 15:24:10 -0600 (Thu, 03 Mar 2016) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Xml.Utils.h"
#include "Csi.Utils.h"
#include "Csi.StrAscStream.h"
#include <iostream>
#include <iomanip>


namespace Csi
{
   namespace Xml
   {
      void output_xml_data(std::ostream &out, StrUni const &data)
      {
         for(size_t i = 0; i < data.length(); ++i)
         {
            // check for ascii data
            if(data[i] >= 32 && data[i] <= 126)
            {
               switch(data[i])
               {
               case '<':
                  out << "&lt;";
                  break;
                  
               case '>':
                  out << "&gt;";
                  break;
                  
               case '&':
                  out << "&amp;";
                  break;
                  
               case '\"':
                  out << "&quot;";
                  break;

               case '\'':
                  out << "&apos;";
                  break;
                  
               default:
                  out << static_cast<char>(data[i]);
                  break;
               }
            }
            else
            {
               // XML 1.0 only supports certain valid character references.  See
               // http://en.wikipedia.org/wiki/XML#Valid_characters
               unsigned int ch(data[i]);
               if(data[i] == 0x09 || data[i] == 0x0a || data[i] == 0x0d ||
                  (data[i] >= 0x0020 && data[i] <= 0xd7ff) ||
                  (data[i] >= 0xe000 && data[i] <= 0xfffd))
               {
                  out << "&#" << std::dec << ch << ";";
               }
               else
               {
                  out << "U+" << std::setw(4) << std::setfill('0') << std::hex << ch;
               }
            }
         }
      } // output_xml_data


      void output_xml_data(std::wostream &out, StrUni const &data)
      {
         for(size_t i = 0; i < data.length(); ++i)
         {
            // check for ascii data
            if(data[i] >= 32 && data[i] <= 126)
            {
               switch(data[i])
               {
               case '<':
                  out << L"&lt;";
                  break;
                  
               case '>':
                  out << L"&gt;";
                  break;
                  
               case '&':
                  out << L"&amp;";
                  break;
                  
               case '\"':
                  out << L"&quot;";
                  break;

               case '\'':
                  out << L"&apos;";
                  break;
                  
               default:
                  out << static_cast<wchar_t>(data[i]);
                  break;
               }
            }
            else
            {
               // XML 1.0 only supports certain valid character references.  See
               // http://en.wikipedia.org/wiki/XML#Valid_characters
               unsigned int ch(data[i]);
               if(data[i] == 0x09 || data[i] == 0x0a || data[i] == 0x0d ||
                  (data[i] >= 0x0020 && data[i] <= 0xd7ff) ||
                  (data[i] >= 0xe000 && data[i] <= 0xfffd))
               {
                  out << L"&#" << std::dec << ch << L";";
               }
               else
               {
                  out << L"U+" << std::setw(4) << std::setfill(L'0') << std::hex << ch;
               }
            }
         }
      } // output_xml_data


      void input_xml_data(
         StrUni &dest, char const *buff, size_t buff_len)
      {
         size_t i(0);
         size_t last_entity_end(0);
         StrAsc temp;
         dest.reserve(buff_len);
         while(i < buff_len)
         {
            size_t amp_pos(locate_sub_string(buff + i, buff_len - i, "&", 1) + i);
            if(amp_pos > last_entity_end)
               dest.append_utf8(buff + last_entity_end, amp_pos - last_entity_end);
            if(amp_pos < buff_len)
            {
               last_entity_end = locate_sub_string(buff + amp_pos, buff_len - amp_pos, ";", 1);
               temp.cut(0);
               temp.append(buff + amp_pos + 1, last_entity_end - 1);
               if(temp == "gt")
                  dest.append(L'>');
               else if(temp == "lt")
                  dest.append(L'<');
               else if(temp == "amp")
                  dest.append(L'&');
               else if(temp == "apos")
                  dest.append(L'\'');
               else if(temp == "quot")
                  dest.append(L'\"');
               else if(temp.first() == '#')
               {
                  int radix(10);
                  int start_pos(1);
                  unsigned long val;
               
                  if(temp.first() == 'x' || temp.first() == 'X')
                  {
                     radix = 16;
                     start_pos = 2;
                  }
                  else if(temp.first() == '0')
                  {
                     radix = 8;
                     start_pos = 2;
                  }
                  val = strtoul(temp.c_str() + start_pos, 0, radix);
                  dest.append(static_cast<wchar_t>(val));
               }
               else
                  throw std::invalid_argument("invalid XML entity");
               last_entity_end += amp_pos + 1;
               i = last_entity_end;
            }
            else
               i = buff_len;
         }
      } // input_xml_data


      void input_xml_data(
         StrAsc &dest, char const *buff, size_t buff_len)
      {
         StrUni temp;
         input_xml_data(temp, buff, buff_len);
         dest += temp.to_utf8();
      } // input_xml_data


      ParsingError::ParsingError(
         char const *message,
         size_t row_no_,
         size_t column_no_):
         MsgExcept(""),
         row_no(row_no_),
         column_no(column_no_)
      {
         OStrAscStream temp;
         temp << "XML Parsing Error\",\"";
         if(message != 0 && message[0] != 0)
            temp << message << "\",\"";
         else
            temp << "\",\"";
         temp << row_no << "\",\"" << column_no << "\"";
         msg = temp.str();
      } // constructor
   };
};


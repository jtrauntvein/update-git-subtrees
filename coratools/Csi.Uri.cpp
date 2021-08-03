/* Csi.Uri.cpp

   Copyright (C) 2007, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 23 February 2007
   Last Change: Thursday 09 May 2019
   Last Commit: $Date: 2019-05-10 09:19:17 -0600 (Fri, 10 May 2019) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Uri.h"
#include "Csi.StrAscStream.h"
#include "Csi.MsgExcept.h"
#include "Csi.Utils.h"
#include <cctype>
#include <iomanip>
#include <algorithm>


namespace Csi
{
   ////////////////////////////////////////////////////////////
   // class Uri definitions
   ////////////////////////////////////////////////////////////
   Uri::Uri():
      protocol("http"),
      path("/"),
      server_port(80)
   { }


   Uri::Uri(StrAsc const &s):
      server_port(80)
   { parse(s); }


   Uri::Uri(Uri const &other):
      protocol(other.protocol),
      server_address(other.server_address),
      server_port(other.server_port),
      path(other.path),
      anchor(other.anchor),
      params(other.params)
   { }


   Uri &Uri::operator =(Uri const &other)
   {
      protocol = other.protocol;
      server_address = other.server_address;
      server_port = other.server_port;
      path = other.path;
      anchor = other.anchor;
      params = other.params;
      return *this;
   }


   Uri &Uri::operator =(StrAsc const &s)
   {
      parse(s);
      return *this;
   } // copy operator for strings


   Uri::~Uri()
   { }


   void Uri::parse(StrAsc const &s)
   {
      // initialise all components of the URI
      protocol.cut(0);
      server_address.cut(0);
      server_port = 80;
      path.cut(0);
      anchor.cut(0);
      params.clear();

      // we will look for delimiting characters in the provided string
      size_t proto_end_pos = s.find("://");
      size_t path_start_pos = 0;
      StrAsc temp;
      
      if(proto_end_pos < s.length())
      {
         // separate the address and the protocol parts of the URL.
         size_t address_end_pos = s.find("/", proto_end_pos + 3);
         StrAsc temp;
         s.sub(protocol, 0, proto_end_pos);
         s.sub(
            temp,
            proto_end_pos + 3,
            address_end_pos - proto_end_pos -3);
         path_start_pos = address_end_pos + 1;

         // we need to parse the address further
         parse_uri_address(server_address, server_port, temp);
         if(server_port == 0)
         {
            if(protocol == "https" || protocol == "wss")
               server_port = 443;
            else if(protocol == "ftp")
               server_port = 21;
            else
               server_port = 80;
         }
      }
      else
      {
         // there is no protocol defined but we should still look for the address
         size_t address_end_pos(s.find("/"));
         protocol = "http";
         if(address_end_pos >= s.length())
            address_end_pos = s.find("?");
         if(address_end_pos < s.length())
         {
            StrAsc temp;
            s.sub(temp, 0, address_end_pos);
            parse_uri_address(server_address, server_port, temp);
            if(server_port == 0)
               server_port = 80;
         }
         else
         {
            parse_uri_address(server_address, server_port, s);
            if(server_port == 0)
               server_port = 80;
         }
      }

      // we now have what we believe to be the starting position of the path.  The ending position
      // will depend on the presence of the parameters marker ('?') and the anchor marker ('#').
      // Once these are found, we can copy out the path
      size_t params_start_pos = s.find("?",path_start_pos);
      size_t anchor_start_pos = s.find("#",path_start_pos);
      size_t path_len = s.length() - path_start_pos;

      if(params_start_pos < s.length() &&
         anchor_start_pos < s.length() &&
         anchor_start_pos < params_start_pos)
         throw Csi::MsgExcept("URI syntax error");
      if(params_start_pos < s.length())
         path_len = params_start_pos - path_start_pos;
      else if(anchor_start_pos < s.length())
      {
         path_len = anchor_start_pos - path_start_pos;
         s.sub(temp, anchor_start_pos + 1, s.length());
         anchor = decode(temp);
      }
      s.sub(temp, path_start_pos, path_len);
      path = decode(temp);
      
      // our final act is to parse the parameters portion
      StrAsc param_name;
      StrAsc buff;
      bool read_hex = false;
      char hex_buff[3];
      int hex_buff_len = 0;

      memset(hex_buff,0,sizeof(hex_buff));
      while(++params_start_pos < s.length() && params_start_pos < anchor_start_pos)
      {
         char ch = s[params_start_pos];
         if(read_hex)
         {
            hex_buff[hex_buff_len++] = ch;
            if(hex_buff_len == 2)
            {
               hex_buff[hex_buff_len] = 0;
               buff.append(
                  static_cast<char>(
                     strtol(hex_buff,0,16)));
               read_hex = false;
            }
         }
         else if(ch == '&')
         {
            params.push_back(value_type(param_name,buff));
            param_name.cut(0);
            buff.cut(0);
         }
         else if(ch == '%')
         {
            hex_buff_len = 0;
            read_hex = true;
         }
         else if(ch == '=')
         {
            param_name = buff;
            buff.cut(0);
         }
         else if(ch == '+')
            buff.append(' ');
         else
            buff.append(ch);
      }
      if(param_name.length() != 0)
         params.push_back(value_type(param_name,buff));
   } // parse


   void Uri::format(std::ostream &out, bool address_only) const
   {
      bool ipv6_address(server_address.find(":") < server_address.length());
      
      out << protocol << "://";
      if(ipv6_address)
         out << "[";
      out << server_address;
      if(ipv6_address)
         out << "]";
      if((protocol == "http" && server_port != 80) || (protocol == "https" && server_port != 443))
         out << ":" << server_port;
      if(!address_only)
      {
         if(path.first() != '/')
            out << "/";
         out << path;
         if(!empty())
         {
            out << "?";
            for(const_iterator pi = begin(); pi != end(); ++pi)
            {
               if(pi != begin())
                  out << "&";
               encode(out, pi->first);
               out << "=";
               encode(out, pi->second);
            }
         }
         if(anchor.length() > 0)
         {
            out << "#";
            encode(out, anchor);
         }
      }
   } // format


   void Uri::format(std::wostream &out, bool address_only) const
   {
      bool ipv6_address(server_address.find(":") < server_address.length());
      OStrAscStream temp;
      
      out << protocol << L"://";
      if(ipv6_address)
         out << L"[";
      out << server_address;
      if(ipv6_address)
         out << L"]";
      if((protocol == "http" && server_port != 80) || (protocol == "https" && server_port != 443))
         out << L":" << server_port;
      if(!address_only)
      {
         out << path;
         if(!empty())
         {
            out << L"?";
            for(const_iterator pi = begin(); pi != end(); ++pi)
            {
               temp.str("");
               if(pi != begin())
                  temp << "&";
               encode(temp, pi->first);
               out << temp.str();
               out << L"=";
               temp.str("");
               encode(temp, pi->second);
               out << temp.str();
            }
         }
         if(anchor.length() > 0)
         {
            out << "#";
            temp.str("");
            encode(temp, anchor);
            out << temp.str();
         }
      }
   } // format
   

   namespace 
   {
      bool is_alpha_numeric(int ch)
      { return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9'); }
   };


   void Uri::encode(
      std::ostream &out,
      StrAsc const &s)
   {
      // the following information derived from RFC3986 - syntax of URI
      // reserved    = gen-delims / sub-delims
      //
      // gen-delims  = ":" / "/" / "?" / "#" / "[" / "]" / "@"
      //
      // sub-delims  = "!" / "$" / "&" / "'" / "(" / ")"
      //             / "*" / "+" / "," / ";" / "="
      // unreserved  = ALPHA / DIGIT / "-" / "." / "_" / "~"


      for(uint4 i = 0; i < s.length(); ++i)
      {
         bool needs_quote = false;
         char ch = s[i];
         switch(ch)
         {
         //General Delims
         case ':':
         case '/':
         case '?':
         case '#':
         case '[':
         case ']':
         case '@':
            needs_quote = true;
            break;

         //Sub Delims
         case '!':
         case '$':
         case '&':
         case '\'':
         case '(':
         case ')':
         case '*':
         case '+':
         case ',':
         case ';':
         case '=':
            needs_quote = true;
            break;

         //Unreserved Delims
         case '-':
         case '.':
         case '_':
         case '~':
            needs_quote = false;
            break;
            
         default:
            if(!is_alpha_numeric(ch))
               needs_quote = true;
            break;
         }
         if(needs_quote)
            out << '%' << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
                << static_cast<uint2>(ch & 0xFF);
         else
            out << ch;
      }
   } // encode


   StrAsc Uri::decode(StrAsc const &s)
   {
      StrAsc rtn;
      for(uint4 i = 0; i < s.length(); ++i)
      {
         char ch(s[i]);
         if(ch == '%' && i + 3 < s.length())
         {
            StrAsc code;
            s.sub(code, i + 1, 2);
            rtn.append(static_cast<char>(strtoul(code.c_str(), 0, 16)));
            i += 2;
         }
         else
            rtn.append(ch);
      }
      return rtn; 
   } // decode
   

   StrAsc Uri::build()
   {
      // encode the protocol and server address
      OStrAscStream rtn;
      if(protocol.length() && server_address.length())
      {
         if(protocol.length())
            rtn << protocol << "://";
         if(server_address.length())
         {
            encode(rtn,server_address);
            if(server_port != 80 && server_port != 443)
               rtn << ":" << server_port;
         }
      }

      // encode the path names individually
      std::list<StrAsc> path_names;
      get_path_names(path_names);
      if(path_names.empty())
         rtn << '/';
      while(!path_names.empty())
      {
         rtn << '/';
         encode(rtn, path_names.front());
         path_names.pop_front();
      }

      // encode the parameters
      if(!params.empty())
      {
         rtn << "?";
         for(iterator pi = begin(); pi != end(); ++pi)
         {
            if(pi != begin())
               rtn << "&";
            encode(rtn, pi->first);
            rtn << '=';
            encode(rtn, pi->second);
         }
      }

      // finally format the anchor
      if(anchor.length())
      {
         rtn << '#';
         encode(rtn, anchor);
      }
      return rtn.str();
   } // build


   namespace
   {
      struct param_has_name
      {
         StrAsc const &name;
         param_has_name(StrAsc const &name_): name(name_) { }
         bool operator ()(Uri::value_type const &value) const
         { return value.first == name; }
      };
   };


   bool Uri::has_param(StrAsc const &name, StrAsc *value) const
   {
      const_iterator pi = std::find_if(
         params.begin(), params.end(), param_has_name(name));
      bool rtn = false;
      
      if(pi != params.end())
      {
         if(value != 0)
            *value = pi->second;
         rtn = true;
      }
      else if(value != 0)
         value->cut(0);
      return rtn;
   } // has_param

   
   StrAsc Uri::get_param_value(StrAsc const &name) const
   {
      StrAsc rtn;
      const_iterator pi = std::find_if(params.begin(),params.end(),param_has_name(name));
      if(pi != end())
         rtn = pi->second;
      return rtn;
   } // get_param_value
};


/* Csi.SocketAddress.cpp

   Copyright (C) 2012, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Tuesday 19 June 2012
   Last Change: Tuesday 14 May 2019
   Last Commit: $Date: 2019-05-14 12:45:41 -0600 (Tue, 14 May 2019) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.SocketAddress.h"
#include "Csi.StrAscStream.h"
#include "Csi.OsException.h"
#include "coratools.strings.h"
#ifdef WIN32
#include <ws2tcpip.h>
#endif
#include <iostream>
#include <algorithm>
#include <cstdlib>


namespace Csi
{
   SocketAddress::SocketAddress(char const *s, uint2 port)
   {
      addresses_type addresses;
      resolve(addresses, s, port);
      if(!addresses.empty())
      {
         addresses_type::value_type &address(addresses.front());
         memcpy(&storage, &address.storage, sizeof(storage));
         address_len = address.address_len;
      }
   } // constructor

   
   StrAsc SocketAddress::get_address() const
   {
      StrAsc rtn;
      if(address_len > 0)
      {
         char address_buff[NI_MAXHOST];
         char port_buff[NI_MAXSERV];
         int rcd;
         
         memset(address_buff, 0, sizeof(address_buff));
         memset(port_buff, 0, sizeof(port_buff));
         rcd = ::getnameinfo(
            reinterpret_cast<struct sockaddr const *>(&storage), address_len,
            address_buff, sizeof(address_buff) - 1,
            port_buff, sizeof(port_buff) - 1,
            NI_NUMERICHOST | NI_NUMERICSERV);
         if(rcd == 0)
            rtn = address_buff;
      }
      return rtn;
   } // get_address


   uint2 SocketAddress::get_port() const
   {
      uint2 rtn(0);
      if(get_family() == family_ipv4)
      {
         struct sockaddr_in const *in_address(reinterpret_cast<struct sockaddr_in const *>(&storage));
         rtn = htons(in_address->sin_port);
      }
      else
      {
         struct sockaddr_in6 const *in_address(reinterpret_cast<struct sockaddr_in6 const *>(&storage));
         rtn = htons(in_address->sin6_port);
      }
      return rtn;
   } // get_port


   void SocketAddress::set_port(uint2 port)
   {
      if(get_family() == family_ipv4)
      {
         struct sockaddr_in *in_address(reinterpret_cast<struct sockaddr_in *>(&storage));
         in_address->sin_port = htons(port);
      }
      else if(get_family() == family_ipv6)
      {
         struct sockaddr_in6 *in_address(reinterpret_cast<struct sockaddr_in6 *>(&storage));
         in_address->sin6_port = htons(port);
      }
   } // set_port


   void SocketAddress::clear()
   {
      memset(&storage, 0, sizeof(storage));
   } // clear


   namespace
   {
      bool is_decimal_digit(char ch)
      { return ch >= '0' && ch <= '9'; }


      bool is_hex_only_digit(char ch)
      { return (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F'); }
      

      bool is_hex_digit(char ch)
      { return is_decimal_digit(ch) || is_hex_only_digit(ch); }
   };
   

   SocketAddress::validate_result_type SocketAddress::validate(char const *address)
   {
      int pos(0);
      enum state_type
      {
         state_error = -1,
         state_before_address,
         state_in_address_group,
         state_in_ipv4,
         state_in_ipv4_group,
         state_in_ipv6_address,
         state_in_ipv6_group,
         state_between_ipv6_groups,
         state_in_ipv6_scope
      } state(state_before_address);
      int groups_count(0);
      int empty_groups_count(0);
      bool skip_next(false);
      StrAsc group_buffer;
      validate_result_type rtn(validate_empty);

      // we will parse through the string one character at a time and drive the state machine.
      while(address[pos] != 0 && state != state_error)
      {
         char ch = address[pos];
         skip_next = false;
         if(state == state_before_address)
         {
            if(is_hex_only_digit(ch) || ch == ':')
            {
               state = state_in_ipv6_group;
               skip_next = true;
            }
            else if(is_decimal_digit(ch))
            {
               state = state_in_address_group;
               skip_next = true;
            }
            else if(!isspace(ch))
               state = state_error;
         }
         else if(state == state_in_address_group)
         {
            if(is_hex_only_digit(ch))
            {
               group_buffer.append(ch);
               state = state_in_ipv6_group;
            }
            else if(is_decimal_digit(ch))
               group_buffer.append(ch);
            else if(ch == '.')
            {
               state = state_in_ipv4_group;
               skip_next = true;
            }
            else if(ch == ':')
            {
               state = state_in_ipv6_group;
               skip_next = true;
            }
            else
               state = state_error;
         }
         else if(state == state_in_ipv4_group)
         {
            if(is_decimal_digit(ch))
               group_buffer.append(ch);
            else if(ch == '.')
            {
               if(std::strtoul(group_buffer.c_str(), 0, 10) <= 255)
               {
                  if(++groups_count >= 4)
                     state = state_error;
                  group_buffer.cut(0);
               }
               else
                  state = state_error;
            }
            else
               state = state_error;
         }
         else if(state == state_in_ipv6_group)
         {
            if(is_hex_digit(ch))
               group_buffer.append(ch);
            else if(ch == ':')
            {
               if(group_buffer.length() <= 4)
               {
                  if(++groups_count < 8)
                     state = state_between_ipv6_groups;
                  else
                     state = state_error;
               }
               else
                  state = state_error;
               group_buffer.cut(0);
            }
            else if(ch == '%')
               state = state_in_ipv6_scope;
            else if(!isspace(ch))
               state = state_error;
         }
         else if(state == state_in_ipv6_scope)
         {
            if(isspace(ch))
               break;
         }
         else if(state == state_between_ipv6_groups)
         {
            if(is_hex_digit(ch))
            {
               skip_next = true;
               state = state_in_ipv6_group;
            }
            else if(ch == ':')
            {
               if(++empty_groups_count > 1)
                  state = state_error;
            }
            else if(!isspace(ch))
               state = state_error;
         }
         if(!skip_next)
            ++pos;
      }

      // if we left the loop with a remnant, we will need to process that remnant based upon the
      // current state.
      if(state != state_error && group_buffer.length() > 0)
      {
         if(state == state_in_ipv4_group)
         {
            if(std::strtoul(group_buffer.c_str(), 0, 10) > 255)
               state = state_error;
            if(++groups_count > 4)
               state = state_error;
         }
         else if(state == state_in_ipv6_group)
         {
            if(group_buffer.length() > 4)
               state = state_error;
            if(++groups_count > 8)
               state = state_error;
         }
         else if(state != state_in_ipv6_scope)
            state = state_error;
      }

      // we can now map the state into the result code for validation.
      switch(state)
      {
      case state_before_address:
         rtn = validate_empty;
         break;
         
      case state_in_ipv4_group:
         if(groups_count == 4)
            rtn = validate_ipv4_complete;
         else
            rtn = validate_ipv4_incomplete;
         break;

      case state_in_ipv6_group:
      case state_between_ipv6_groups:
         if(groups_count == 8 || (groups_count < 8 && empty_groups_count))
            rtn = validate_ipv6_complete;
         else
            rtn = validate_ipv6_incomplete;
         break;

      default:
         rtn = validate_error;
         break;
      }
      return rtn;
   } // validate


   void SocketAddress::format_validate_result(std::ostream &out, validate_result_type result)
   {
      using namespace SocketAddressStrings;
      switch(result)
      {
      case validate_empty:
         out << my_strings[strid_validate_empty];
         break;

      case validate_ipv4_complete:
         out << my_strings[strid_validate_ipv4_complete];
         break;

      case validate_ipv4_incomplete:
         out << my_strings[strid_validate_ipv4_incomplete];
         break;

      case validate_ipv6_complete:
         out << my_strings[strid_validate_ipv6_complete];
         break;

      case validate_ipv6_incomplete:
         out << my_strings[strid_validate_ipv6_incomplete];
         break;

      default:
         out << my_strings[strid_validate_error];
         break;
      }
   } // format_validate_result
   

   bool SocketAddress::operator <(SocketAddress const &other) const
   {
      bool rtn(false);
      if(get_family() == other.get_family())
      {
         if(get_family() == family_ipv4)
         {
            sockaddr_in const *mine(reinterpret_cast<sockaddr_in const *>(&storage));
            sockaddr_in const *theirs(reinterpret_cast<sockaddr_in const *>(&other.storage));
            uint4 my_address(htonl(mine->sin_addr.s_addr));
            uint4 their_address(htonl(theirs->sin_addr.s_addr));
            rtn = (my_address < their_address);
         }
         else
         {
            sockaddr_in6 const *mine(reinterpret_cast<sockaddr_in6 const *>(&storage));
            sockaddr_in6 const *theirs(reinterpret_cast<sockaddr_in6 const *>(&other.storage));
            rtn = memcmp(mine->sin6_addr.s6_addr, theirs->sin6_addr.s6_addr, address_len) < 0;
         }
      }
      else
         rtn = (get_family() == family_ipv6);
      return rtn;
   } // less than operator


   bool SocketAddress::operator ==(SocketAddress const &other) const
   {
      bool rtn(false);
      if(get_family() == other.get_family())
      {
         if(get_family() == family_ipv4)
         {
            sockaddr_in const *mine(reinterpret_cast<sockaddr_in const *>(&storage));
            sockaddr_in const *theirs(reinterpret_cast<sockaddr_in const *>(&other.storage));
            rtn = mine->sin_addr.s_addr == theirs->sin_addr.s_addr;
         }
         else
         {
            sockaddr_in6 const *mine(reinterpret_cast<sockaddr_in6 const *>(&storage));
            sockaddr_in6 const *theirs(reinterpret_cast<sockaddr_in6 const *>(&other.storage));
            rtn = memcmp(mine->sin6_addr.s6_addr, theirs->sin6_addr.s6_addr, address_len) == 0;
         }
      }
      return rtn;
   } // equality operator
      

   void SocketAddress::resolve(
      addresses_type &addresses,
      char const *address,
      uint2 port,
      bool passive,
      bool numeric)
   {
      // we will get the list of addresses that have been resolved.
      struct addrinfo hints;
      struct addrinfo *results = 0;
      int rcd;
      Csi::OStrAscStream service;

      service.imbue(std::locale::classic());
      service << port;
      memset(&hints, 0, sizeof(hints));
      if(passive || address == 0 || address[0] == 0)
         hints.ai_flags = AI_PASSIVE;
      else if(!numeric)
         hints.ai_flags = AI_CANONNAME;
      if(numeric)
         hints.ai_flags |= AI_NUMERICHOST;
      hints.ai_family = AF_UNSPEC;
      hints.ai_socktype = SOCK_STREAM;
      rcd = ::getaddrinfo(address, service.str().c_str(), &hints, &results);
      if(rcd != 0)
         throw OsException("address resolution failed");

      // we now need to create SocketAddress instances to cover each address returned
      addresses.clear();
      for(addrinfo *info = results; info != 0; info = info->ai_next)
      {
         SocketAddress address;
         memcpy(&address.storage, info->ai_addr, info->ai_addrlen);
         address.address_len = (uint4)info->ai_addrlen;
         if(std::find(addresses.begin(), addresses.end(), address) == addresses.end())
            addresses.push_back(address);
      }
      freeaddrinfo(results);
   } // resolve


   std::ostream &operator <<(std::ostream &out, SocketAddress const &address)
   {
      if(address.get_family() == SocketAddress::family_ipv6)
         out << "[";
      out << address.get_address();
      if(address.get_family() == SocketAddress::family_ipv6)
         out << "]";
      out << ":" << address.get_port();
      return out;
   }
};



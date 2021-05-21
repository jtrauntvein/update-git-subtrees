/* Csi.SocketBase.cpp

   Copyright (C) 2010, 2017 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 23 December 2010
   Last Change: Thursday 02 November 2017
   Last Commit: $Date: 2017-11-02 16:49:33 -0600 (Thu, 02 Nov 2017) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "Csi.SocketBase.h"
#include "Csi.StrAscStream.h"
#include "Csi.SocketAddress.h"
#ifndef WIN32
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <ifaddrs.h>
#endif


namespace Csi
{
   StrAsc get_host_name()
   {
      StrAsc rtn;
      char buff[1024];
      int rcd(::gethostname(buff, sizeof(buff)));
      if(rcd == 0)
         rtn = buff;
      return rtn;
   } // get_host_name

   
   uint4 str_to_address(char const *str)
   {
      uint4 rtn = 0;
      if(str != 0 && str[0] != 0)
       rtn = htonl(inet_addr(str));
      return rtn;
   } // str_to_address

   
   StrAsc address_to_str(uint4 address)
   {
      OStrAscStream rtn;
      rtn << ((address & 0xFF000000) >> 24) << "."
          << ((address & 0x00FF0000) >> 16) << "."
          << ((address & 0x0000FF00) >>  8) << "."
          << ( address & 0x000000FF);
      return rtn.str();
   } // address_to_str

   
   void get_host_names(host_names_type &host_names, bool ipv4_only)
   {
      host_names.clear();
#ifdef WIN32
      StrAsc host_name(get_host_name());
      
      if(host_name.length() > 0)
      {
         SocketAddress::addresses_type addresses;
         OStrAscStream temp;
         
         SocketAddress::resolve(addresses, host_name.c_str(), 0, true);
         while(!addresses.empty())
         {
            SocketAddress address(addresses.front());
            addresses.pop_front();
            if(address.get_family() == SocketAddress::family_ipv4 || !ipv4_only)
            {
               temp.str("");
               temp << address.get_address();
               temp.str().cut(temp.str().find("%"));
               host_names.push_back(temp.str());
            }
         }
      }
#else
      struct ifaddrs *ifaddr, *ifa;
      if(getifaddrs(&ifaddr) != -1)
      {
         for(ifa = ifaddr; ifa != 0; ifa = ifa->ifa_next)
         {
            if(ifa->ifa_addr != 0)
            {
               int family(ifa->ifa_addr->sa_family);
               if(family == AF_INET)
               {
                  SocketAddress address(ifa->ifa_addr, sizeof(struct sockaddr_in));
                  host_names.push_back(address.get_address());
               }
               else if(family == AF_INET6 && !ipv4_only)
               {
                  SocketAddress address(ifa->ifa_addr, sizeof(struct sockaddr_in6));
                  host_names.push_back(address.get_address());
               }
            }
            
         }
         freeifaddrs(ifaddr);
      }
#endif
   } // get_host_names


   StrAsc get_domain_name()
   {
      StrAsc host_name(get_host_name());
      struct hostent *hp(gethostbyname(host_name.c_str()));
      StrAsc rtn;

      if(hp != 0)
         rtn = hp->h_name;
      return rtn;
   } // get_domain_name
};



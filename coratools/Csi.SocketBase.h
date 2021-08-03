/* Csi.SocketBase.h

   Copyright (C) 2010, 2011 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 23 December 2010
   Last Change: Tuesday 20 September 2011
   Last Commit: $Date: 2012-08-17 15:21:30 -0600 (Fri, 17 Aug 2012) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_SocketBase_h
#define Csi_SocketBase_h

#include <list>


#ifdef WIN32
#include "Csi.Win32.WinsockBase.h"
namespace Csi
{
   typedef Win32::WinSockBase SocketBase;
};
#else

#include "Csi.Posix.SocketBase.h"
namespace Csi
{
   using Posix::SocketBase;
};
#endif


namespace Csi
{
   ////////////////////////////////////////////////////////////
   // get_host_name
   //
   // Returns the name for the local host.
   ////////////////////////////////////////////////////////////
   StrAsc get_host_name();

   ////////////////////////////////////////////////////////////
   // str_to_address
   //
   // Converts the specified string into an IP address in network byte order. 
   ////////////////////////////////////////////////////////////
   uint4 str_to_address(char const *str);

   ////////////////////////////////////////////////////////////
   // address_to_str
   //
   // Converts the specified address (assumed to be in network byte order) to a
   // dotted decimal string.
   ////////////////////////////////////////////////////////////
   StrAsc address_to_str(uint4 address);

   ////////////////////////////////////////////////////////////
   // get_host_names
   //
   // Creates an array of host addresses for the local host.  These addresses
   // can optionally be translated into domain names.
   ////////////////////////////////////////////////////////////
   typedef std::list<StrAsc> host_names_type;
   void get_host_names(host_names_type &host_names, bool ipv4_only = false);
   inline host_names_type get_host_names()
   {
      host_names_type rtn;
      get_host_names(rtn);
      return rtn;
   }

   ////////////////////////////////////////////////////////////
   // get_domain_name
   //
   // Returns the domain  name for this host
   ////////////////////////////////////////////////////////////
   StrAsc get_domain_name();
};


#endif

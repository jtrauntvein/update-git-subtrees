/* Csi.DevConfig.SettingComp.CompIpAddrAny.cpp

   Copyright (C) 2019, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Friday 08 March 2019
   Last Change: Saturday 09 March 2019
   Last Commit: $Date: 2019-03-11 11:39:36 -0600 (Mon, 11 Mar 2019) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.DevConfig.SettingComp.CompIpAddrAny.h"
#include "Csi.SocketAddress.h"


namespace Csi
{
   namespace DevConfig
   {
      namespace SettingComp
      {
         CompBase *CompIpAddrAnyDesc::make_component(SharedPtr<DescBase> &desc, SharedPtr<CompBase> &previous)
         { return new CompIpAddrAny(desc); }
         
         void CompIpAddrAny::input(std::istream &in, bool translate)
         {
            StrAsc temp;
            temp.readLine(in);
            switch(SocketAddress::validate(temp.c_str()))
            {
            case SocketAddress::validate_ipv4_complete:
            case SocketAddress::validate_ipv6_complete:
            case SocketAddress::validate_empty:
               value = temp;
               has_changed = true;
               break;
               
            default:
               throw std::invalid_argument(desc->get_name().c_str());
               break;
            }
         } // input
      };
   };
};


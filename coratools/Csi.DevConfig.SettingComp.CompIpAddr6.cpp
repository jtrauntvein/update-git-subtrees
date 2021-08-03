/* Csi.DevConfig.SettingComp.CompIpAddr6.cpp

   Copyright (C) 2005, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 02 June 2005
   Last Change: Thursday 01 March 2018
   Last Commit: $Date: 2012-07-13 13:45:59 -0600 (Fri, 13 Jul 2012) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.DevConfig.SettingComp.CompIpAddr6.h"
#ifdef WIN32
#include <ws2tcpip.h>
#endif


namespace Csi
{
   namespace DevConfig
   {
      namespace SettingComp
      {
         CompBase *CompIpAddr6Desc::make_component(
            SharedPtr<DescBase> &desc,
            SharedPtr<CompBase> &previous_component)
         { return new CompIpAddr6(desc); }

         
         void CompIpAddr6::read(Csi::SharedPtr<Message> &in)
         {
            CompIpAddr6Desc *temp(static_cast<CompIpAddr6Desc *>(get_desc().get_rep()));
            if(!temp->get_as_string())
            {
               struct sockaddr_in6 storage;
               memset(&storage, 0, sizeof(storage));
               storage.sin6_family = AF_INET6;
               in->readBytes(&storage.sin6_addr, 16, false);
               value = SocketAddress(reinterpret_cast<struct sockaddr *>(&storage), sizeof(storage));
               if(temp->get_include_net_size())
                  net_size = in->readByte();
            }
            else
            {
               StrAsc str;
               in->readAsciiZ(str);
               Csi::IBuffStream val_str(str.c_str(), str.length());
               input(val_str, true);
            }
         } // read


         void CompIpAddr6::write(SharedPtr<Message> &out)
         {
            CompIpAddr6Desc *temp(static_cast<CompIpAddr6Desc *>(get_desc().get_rep()));
            if(!temp->get_as_string())
            {
               struct sockaddr_in6 *storage(
                  reinterpret_cast<struct sockaddr_in6 *>(value.get_storage()));
               out->addBytes(&storage->sin6_addr, 16, false);
               if(temp->get_include_net_size())
                  out->addByte(static_cast<byte>(net_size));
            }
            else
            {
               Csi::OStrAscStream str;
               output(str, true);
               out->addAsciiZ(str.str().c_str());
            }
         } // write


         void CompIpAddr6::write(Json::Array &json)
         {
            Csi::OStrAscStream temp;
            output(temp, true);
            json.push_back(new Json::String(temp.str()));
         } // write


         Json::Array::iterator CompIpAddr6::read(Json::Array::iterator current)
         {
            Json::StringHandle value_json(*current);
            IBuffStream temp(value_json->get_value().c_str(), value_json->get_value().length());
            input(temp, true);
            return ++current;
         } // read

         
         void CompIpAddr6::output(std::ostream &out, bool translate)
         {
            CompIpAddr6Desc *temp(static_cast<CompIpAddr6Desc *>(get_desc().get_rep()));
            out << value.get_address();
            if(temp->get_include_net_size())
               out << '/' << net_size;
         } // output


         void CompIpAddr6::output(std::wostream &out, bool translate)
         {
            CompIpAddr6Desc *temp(static_cast<CompIpAddr6Desc *>(get_desc().get_rep()));
            out << value.get_address();
            if(temp->get_include_net_size())
               out << L'/' << net_size;
         } // output


         void CompIpAddr6::input(std::istream &in, bool translate)
         {
            // skip over any initial whitespace
            CompIpAddr6Desc *temp_desc(static_cast<CompIpAddr6Desc *>(get_desc().get_rep()));
            enum state_type
            {
               state_before_address,
               state_in_address,
               state_in_size,
               state_complete,
               state_error
            } state(state_before_address);
            char ch = ' ';
            bool skip_next(false);
            StrAsc address_buff;
            StrAsc size_buff;
            
            in.get(ch);
            while(state < state_complete && in)
            {
               skip_next = false;
               switch(state)
               {
               case state_before_address:
                  if(!isspace(ch))
                  {
                     state = state_in_address;
                     skip_next = true;
                  }
                  break;

               case state_in_address:
                  if(isxdigit(ch) || ch == ':')
                     address_buff.append(ch);
                  else if(ch == '/')
                  {
                     if(temp_desc->get_include_net_size())
                        state = state_in_size;
                     else
                        state = state_error;
                  }
                  else
                     state = state_complete;
                  break;

               case state_in_size:
                  if(isdigit(ch))
                     size_buff.append(ch);
                  else
                     state = state_complete;
                  break;
               }
               if(!skip_next)
                  in.get(ch);
            }

            // we can now try to convert the address
            SocketAddress::addresses_type addresses;
            if(state == state_error)
               throw std::invalid_argument("invalid IPv6 address");
            if(address_buff.length() == 0)
               address_buff = "::0";
            SocketAddress::resolve(addresses, address_buff.c_str(), 0, false, true);
            if(addresses.empty())
               throw std::invalid_argument("invalid IPv6 address");

            // finally, we need to convert the net size
            if(temp_desc->get_include_net_size() && size_buff.length() > 0)
            {
               IBuffStream temp(size_buff.c_str(), size_buff.length());
               uint4 size_val;
               temp >> size_val;
               if(size_val > 128)
                  throw std::invalid_argument("invalid network size specified");
               net_size = size_val;
            }
            else
               net_size = 64;
            value = addresses.front();
            set_has_changed(true);
         } // input 
      };
   };
};


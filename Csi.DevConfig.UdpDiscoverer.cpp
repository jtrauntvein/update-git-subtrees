/* Csi.DevConfig.UdpDiscoverer.cpp

   Copyright (C) 2015, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Tuesday 19 May 2015
   Last Change: Monday 06 May 2019
   Last Commit: $Date: 2019-05-06 11:46:49 -0600 (Mon, 06 May 2019) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.DevConfig.UdpDiscoverer.h"
#include "Csi.SocketBase.h"
#include "Csi.SocketException.h"
#include "Csi.Utils.h"


namespace Csi
{
   namespace DevConfig
   {
      namespace UdpDiscovererHelpers
      {
         bool Device::read(Message &message, LibraryManager *library)
         {
            bool rtn(true);
            try
            {
               byte version(message.readByte());
               if(version != 1)
                  throw std::invalid_argument("unsupported response version");
               device_type = message.readUInt2();
               major_version = message.readByte();
               minor_version = message.readByte();
               config_port = message.readUInt2();
               ip_address.set_port(config_port);
               
               // we need to look up the device type in the catalog
               if(library != 0)
               {
                  LibraryManager::iterator device_desc(library->get_device(device_type));
                  if(device_desc == library->end())
                     throw std::invalid_argument("unrecognised device type");
                  model_no = (*device_desc)->get_model_no();
               }
               else
                  model_no.cut(0);
               
               // we now need to read any extra parameters that are in the message
               has_mac_address = has_serial_no = has_station_name = has_os_version = false;
               has_pakbus_address = has_pakbus_tcp_port = false;
               while(message.whatsLeft() >= 2)
               {
                  byte param_id(message.readByte());
                  byte param_len(message.readByte());
                  if(param_id == Csi::DevConfig::Discovery::param_mac_address)
                  {
                     has_mac_address = true;
                     message.readAsciiZ(mac_address);
                  }
                  else if(param_id == Csi::DevConfig::Discovery::param_station_name)
                  {
                     has_station_name = true;
                     message.readAsciiZ(station_name);
                  }
                  else if(param_id == Csi::DevConfig::Discovery::param_serial_no)
                  {
                     has_serial_no = true;
                     message.readAsciiZ(serial_no);
                  }
                  else if(param_id == Csi::DevConfig::Discovery::param_os_version)
                  {
                     has_os_version = true;
                     message.readAsciiZ(os_version);
                  }
                  else if(param_id == Csi::DevConfig::Discovery::param_encrypted)
                  {
                     encrypted = message.readBool();
                     has_encrypted = true;
                  }
                  else if(param_id == Csi::DevConfig::Discovery::param_pakbus_address)
                  {
                     pakbus_address = message.readUInt2();
                     has_pakbus_address = true;
                  }
                  else if(param_id == Csi::DevConfig::Discovery::param_pakbus_tcp_port)
                  {
                     pakbus_tcp_port = message.readUInt2();
                     has_pakbus_tcp_port = true;
                  }
                  else
                     message.movePast(param_len);
               }
            }
            catch(std::exception &)
            { rtn = false; }
            return rtn;
         } // read


         Listener::Listener(
            UdpDiscoverer *owner_,
            SocketAddress const &address_,
            library_handle &library_):
            SocketUdpServer(address_, 1024),
            owner(owner_),
            address(address_),
            last_tran_no(0),
            library(library_)
         {
            if(address.get_family() == AF_INET)
               broadcast_address = SocketAddress("255.255.255.255", address.get_port());
            else
               broadcast_address = SocketAddress("ff02::1", address.get_port());
         } // constructor


         Listener::~Listener()
         {
            SocketUdpServer::close();
         } // destructor


         void Listener::on_datagram(Packet &packet, SocketAddress const &address)
         {
            try
            {
               // we will process only datagrams that have a valid signature and minimum length.
               uint2 sig(Csi::calcSigFor(packet.getMsg(), packet.length()));
               if(sig == 0 && packet.length() > 6)
               {
                  // we will construct a devconfig message from the datagram
                  Message message(packet.getMsg(), packet.length() - 2);
                  if(message.get_message_type() == Messages::discovery_ack)
                  {
                     UdpDiscoverer::device_handle device(new Device(address));
                     if(device->read(message, library.get_rep()))
                        owner->on_device_report(device);
                  }
               }
            }
            catch(std::exception &)
            { }
         } // on_datagram


         void Listener::on_socket_error(int error_code)
         {
            SocketException error("listener failed", error_code);
            SocketUdpServer::close();
            owner->on_listener_fail(this, error.what());
         } // on_socket_error


         void Listener::transmit()
         {
            try
            {
               Message cmd;
               cmd.set_message_type(Messages::discovery_cmd);
               cmd.set_tran_no(++last_tran_no);
               cmd.addUInt2(calcSigNullifier(calcSigFor(cmd.getMsg(), cmd.length())));
               send_datagram(cmd, broadcast_address);
            }
            catch(std::exception &)
            { }
         } // transmit
      };


      UdpDiscoverer::UdpDiscoverer():
         device_mask(0xffff),
         broadcast_id(0),
         client(0),
         state(state_standby),
         discover_port(6785),
         broadcast_interval(5000)
      { }


      void UdpDiscoverer::set_device_mask(uint2 value)
      {
         if(state != state_standby)
            throw MsgExcept("cannot set device type when started");
         device_mask = value;
      } // set_device_type


      void UdpDiscoverer::set_timer(timer_handle timer_)
      {
         if(state != state_standby)
            throw MsgExcept("cannot set timer when started");
         timer = timer_;
      } // set_timer


      void UdpDiscoverer::set_discover_port(uint2 value)
      {
         if(state != state_standby)
            throw MsgExcept("cannot set the discovery port when started");
         discover_port = value;
      } // set_discover_port


      void UdpDiscoverer::set_library(library_handle library_)
      {
         if(state != state_standby)
            throw MsgExcept("cannot set the library when started");
         library= library_;
      } // set_library


      void UdpDiscoverer::start(client_type *client_)
      {
         // check the state and parameters
         if(state != state_standby)
            throw MsgExcept("discoverer already started");
         if(!client_type::is_valid_instance(client_))
            throw std::invalid_argument("invalid client pointer");
         if(discover_port == 0)
            throw std::invalid_argument("invalid discoverer port");
         
         // initialise the components.
         if(timer == 0)
            timer.bind(new OneShot);
         client = client_;
         state = state_started;
         
         
         // we will use a short delay for the first broadcast.
         broadcast_id = timer->arm(this, 10);
      } // start


      void UdpDiscoverer::finish()
      {
         state = state_standby;
         client = 0;
         listeners.clear();
         devices.clear();
         if(broadcast_id != 0 && timer != 0)
            timer->disarm(broadcast_id);
      } // finish


      void UdpDiscoverer::onOneShotFired(uint4 id)
      {
         if(id == broadcast_id)
         {
            // we need to create a listener for each interface.
            broadcast_id = 0;
            if(listeners.empty())
            {
               host_names_type host_names;
               SocketAddress::addresses_type host_addresses;
               get_host_names(host_names, false);
               while(!host_names.empty())
               {
                  SocketAddress::addresses_type addresses;
                  SocketAddress::resolve(
                     addresses, host_names.front().c_str(), discover_port, true);
                  while(!addresses.empty())
                  {
                     host_addresses.push_back(addresses.front());
                     addresses.pop_front();
                  }
                  host_names.pop_front();
               }
               while(!host_addresses.empty())
               {
                  SocketAddress host(host_addresses.front());
                  host_addresses.pop_front();
                  try
                  {
                     listeners.push_back(new listener_type(this, host, library));
                  }
                  catch(std::exception &e)
                  {
                     if(client_type::is_valid_instance(client))
                        client->on_listen_fail(this, host, e.what());
                     else
                        finish();
                  }
               }
            }

            // we can now tell each listener to send a broadcast and reschedule the timer
            for(listeners_type::iterator li = listeners.begin(); li != listeners.end(); ++li)
               (*li)->transmit();
            broadcast_id = timer->arm(this, csimax((uint4)5000, broadcast_interval));
         }
      } // onOneShotFired


      void UdpDiscoverer::on_device_report(device_handle &device)
      {
         devices_type::iterator di(devices.find(device->get_ip_address()));
         if(di == devices.end() && (device_mask == 0xffff || device_mask == device->get_device_type()))
         {
            devices[device->get_ip_address()] = device;
            if(client_type::is_valid_instance(client))
               client->on_device(this, device);
            else
               finish();
         }
      } // on_device_report


      void UdpDiscoverer::on_listener_fail(listener_type *listener, StrAsc const &message)
      {
         listeners_type::iterator li(
            std::find_if(
               listeners.begin(), listeners.end(), HasSharedPtr<listener_type>(listener)));
         if(li != listeners.end())
         {
            listener_handle found(*li);
            listeners.erase(li);
            if(client_type::is_valid_instance(client))
               client->on_listen_fail(this, found->address, message);
            else
               finish();
         }
      } // on_listener_fail
   };
};


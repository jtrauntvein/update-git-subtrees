/* Cora.LgrNet.UdpDiscoverer.cpp

   Copyright (C) 2015, Campbell Scientific, Inc

   Written by: Jon Trauntvein 
   Date Begun: Wednesday 20 May 2015
   Last Change: Wednesday 20 May 2015
   Last Commit: $Date: 2015-05-21 12:19:51 -0600 (Thu, 21 May 2015) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.LgrNet.UdpDiscoverer.h"
#include "Cora.LgrNet.Defs.h"


namespace Cora
{
   namespace LgrNet
   {
      namespace
      {
         class event_device_added: public Csi::Event
         {
         public:
            /**
             * Specifies the unique identifier for this event type.
             */
            static uint4 const event_id;

            /**
             * Specifies the device that has been added.
             */
            typedef UdpDiscovererClient::device_handle device_handle;
            device_handle device;

            /**
             * Creates and posts an event of this class.
             */
            static void cpost(UdpDiscoverer *discoverer, device_handle device)
            {
               (new event_device_added(discoverer, device))->post();
            }

         private:
            /**
             * Constructor
             */
            event_device_added(UdpDiscoverer *discoverer, device_handle &device_):
               Event(event_id, discoverer),
               device(device_)
            { }
         };

         
         uint4 const event_device_added::event_id(
            Csi::Event::registerType("Cora::LgrNet::UdpDiscoverer::device_added"));


         class event_failure: public Csi::Event
         {
         public:
            /**
             * Defines the unique identifier for this event type.
             */
            static uint4 const event_id;

            /**
             * Specifies the failure code.
             */
            typedef UdpDiscoverer::client_type::failure_type failure_type;
            failure_type failure;

            /**
             * creates and posts an event of this class.
             */
            static void cpost(UdpDiscoverer *discoverer, failure_type failure)
            { (new event_failure(discoverer, failure))->post(); }

         private:
            /**
             * constructor
             */
            event_failure(UdpDiscoverer *discoverer, failure_type failure_):
               Event(event_id, discoverer),
               failure(failure_)
            { }
         };


         uint4 const event_failure::event_id(
            Csi::Event::registerType("Cora:;LgrNet::UdpDiscoverer::event_failure"));
      };

      
      void UdpDiscoverer::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         client_type *report(client);
         if(ev->getType() == event_device_added::event_id)
         {
            event_device_added *event(static_cast<event_device_added *>(ev.get_rep()));
            if(client_type::is_valid_instance(report))
               report->on_device_added(this, event->device);
            else
               finish();
         }
         else if(ev->getType() == event_failure::event_id)
         {
            event_failure *event(static_cast<event_failure *>(ev.get_rep()));
            finish();
            if(client_type::is_valid_instance(report))
               report->on_failure(this, event->failure);
         }
      } // receive


      void UdpDiscoverer::format_failure(std::ostream &out, client_type::failure_type failure)
      {
         switch(failure)
         {
         case client_type::failure_logon:
            describe_failure(out, corabase_failure_logon);
            break;

         case client_type::failure_session:
            describe_failure(out, corabase_failure_session);
            break;

         case client_type::failure_unsupported:
            describe_failure(out, corabase_failure_unsupported);
            break;

         case client_type::failure_security:
            describe_failure(out, corabase_failure_security);
            break;

         case client_type::failure_network:
            out << "UDP socket failure";
            break;

         case client_type::failure_shut_down:
            out << "server shutting down";
            break;
            
         default:
            describe_failure(out, corabase_failure_unknown);
            break;
         }
      } // format_failure


      void UdpDiscoverer::on_corabase_ready()
      {
         Csi::Messaging::Message cmd(net_session, Messages::udp_discover_start_cmd);
         server_tran = ++last_tran_no;
         cmd.addUInt4(server_tran);
         cmd.addUInt2(discover_port);
         cmd.addUInt2(device_mask);
         state = state_active;
         router->sendMessage(&cmd);
      } // on_corabase_ready


      void UdpDiscoverer::on_corabase_failure(corabase_failure_type failure)
      {
         client_type::failure_type report;
         switch(failure)
         {
         case corabase_failure_logon:
            report = client_type::failure_logon;
            break;

         case corabase_failure_session:
            report = client_type::failure_session;
            break;

         case corabase_failure_unsupported:
            report = client_type::failure_unsupported;
            break;

         case corabase_failure_security:
            report = client_type::failure_security;
            break;

         default:
            report = client_type::failure_unknown;
            break;
         }
         event_failure::cpost(this, report);
      } // on_corabase_failure


      void UdpDiscoverer::onNetMessage(
         Csi::Messaging::Router *router, Csi::Messaging::Message *message)
      {
         if(state == state_active)
         {
            if(message->getMsgType() == Messages::udp_discover_not)
            {
               uint4 tran_no;
               if(message->readUInt4(tran_no) && tran_no == server_tran)
               {
                  client_type::device_handle device;
                  StrAsc ip_address;
                  uint2 device_type;
                  byte major_version;
                  byte minor_version;
                  uint2 config_port;
                  uint4 params_count;
                  bool encrypted;
                  uint2 pakbus_address;
                  uint2 pakbus_tcp_port;
                  
                  message->readStr(ip_address);
                  message->readUInt2(device_type);
                  message->readByte(major_version);
                  message->readByte(minor_version);
                  message->readUInt2(config_port);
                  message->readUInt4(params_count);
                  device.bind(
                     new Csi::DevConfig::UdpDiscovererHelpers::Device(ip_address.c_str()));
                  device->set_device_type(device_type);
                  device->set_major_version(major_version);
                  device->set_minor_version(minor_version);
                  device->set_config_port(config_port);
                  for(uint4 i = 0; i < params_count; ++i)
                  {
                     uint4 id;
                     uint4 len;
                     StrAsc temp;
                     message->readUInt4(id);
                     message->readUInt4(len);
                     switch(id)
                     {
                     case Csi::DevConfig::Discovery::param_mac_address:
                        message->readStr(temp);
                        device->set_mac_address(temp);
                        break;

                     case Csi::DevConfig::Discovery::param_station_name:
                        message->readStr(temp);
                        device->set_station_name(temp);
                        break;

                     case Csi::DevConfig::Discovery::param_serial_no:
                        message->readStr(temp);
                        device->set_serial_no(temp);
                        break;

                     case Csi::DevConfig::Discovery::param_os_version:
                        message->readStr(temp);
                        device->set_os_version(temp);
                        break;

                     case Csi::DevConfig::Discovery::param_encrypted:
                        message->readBool(encrypted);
                        device->set_encrypted(encrypted);
                        break;

                     case Csi::DevConfig::Discovery::param_pakbus_address:
                        message->readUInt2(pakbus_address);
                        device->set_pakbus_address(pakbus_address);
                        break;

                     case Csi::DevConfig::Discovery::param_pakbus_tcp_port:
                        message->readUInt2(pakbus_tcp_port);
                        device->set_pakbus_tcp_port(pakbus_tcp_port);
                        break;

                     default:
                        message->movePast(len);
                        break;
                     }
                  }
                  event_device_added::cpost(this, device);
               }
            }
            else if(message->getMsgType() == Messages::udp_discover_stopped_not)
            {
               uint4 tran_no;
               uint4 reason;
               if(message->readUInt4(tran_no) && message->readUInt4(reason) && tran_no == server_tran)
               {
                  client_type::failure_type failure;
                  switch(reason)
                  {
                  case 3:
                     failure = client_type::failure_network;
                     break;
                     
                  case 4:
                     failure = client_type::failure_shut_down;
                     break;

                  default:
                     failure = client_type::failure_unknown;
                     break;
                  }
                  event_failure::cpost(this, failure);
               }
            }
            else
               ClientBase::onNetMessage(router, message);
         }
         else
            ClientBase::onNetMessage(router, message);
      } // onNetMessage
   };
};


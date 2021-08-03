/* Csi.PakBus.TcpSerialPort.cpp

   Copyright (C) 2018, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Wednesday 11 April 2018
   Last Change: Tuesday 28 January 2020
   Last Commit: $Date: 2020-01-28 11:11:06 -0600 (Tue, 28 Jan 2020) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.PakBus.TcpSerialPort.h"
#include "Csi.StrAscStream.h"
#include "Csi.SocketException.h"


namespace Csi
{
   namespace PakBus
   {
      TcpSerialPort::TcpSerialPort(
         router_handle &router, StrAsc const &server_address_, uint2 server_port_):
         SerialPacketBase(router, router->get_timer()),
         server_address(server_address_),
         server_port(server_port_),
         report_open_id(0)
      {
         if(server_port == 0)
            server_port = 6785;
         on_comm_enabled_change(true);
      } // constructor


      TcpSerialPort::~TcpSerialPort()
      {
         if(report_open_id != 0 && timer != 0)
            timer->disarm(report_open_id);
         close();
         router.clear();
      } // destructor


      void TcpSerialPort::set_log(log_handle log_)
      {
         log = log_;
      } // set_log


      void TcpSerialPort::on_needs_to_dial(priority_type priority)
      {
         try
         {
            if(!get_is_connected())
               open(server_address.c_str(), server_port);
         }
         catch(std::exception &e)
         {
            trace("TcpSwerialPort open failure: %s", e.what());
            on_socket_error(0);
         }
      } // on_needs_to_dial


      void TcpSerialPort::write_data(
         void const *buff, uint4 buff_len, uint4 repeat_count, uint4 msec_between)
      {
         if(get_is_connected())
         {
            for(uint4 i = 0; i < repeat_count; ++i)
               write(buff, buff_len);
         }
      } // write_data


      void TcpSerialPort::on_hanging_up()
      {
         if(log != 0)
            log->force_break("closing socket");
         close();
      } // on_hanging_up


      StrAsc TcpSerialPort::get_port_name() const
      {
         Csi::OStrAscStream rtn;
         rtn << server_address << ":" << server_port;
         return rtn.str();
      } // get_port_name


      void TcpSerialPort::onOneShotFired(uint4 id)
      {
         if(id == report_open_id)
         {
            report_open_id = 0;
            if(log != 0)
               log->force_break("TCP connection open");
            on_link_ready();
         }
         else
            SerialPacketBase::onOneShotFired(id);
      } // onOneShotFired


      void TcpSerialPort::on_connected(SocketAddress const &connected_address)
      {
         if(report_open_id == 0)
            report_open_id = timer->arm(this, 10);
      } // on_connected


      void TcpSerialPort::on_read()
      {
         if(report_open_id == 0)
         {
            ByteQueue &read_buffer(get_read_buffer());
            uint4 rcd;
            SocketTcpSock::on_read();
            while((rcd = read_buffer.pop(low_buff, sizeof(low_buff))) > 0)
               on_data_read(low_buff, rcd);
         }
      } // on_read


      void TcpSerialPort::on_socket_error(int error_code)
      {
         SocketException e("connection lost", error_code);
         if(log != 0)
            log->force_break(e.what());
         close();
         router->log_comms(Router::comms_fault, e.what());
         SerialPacketBase::on_link_failed();
      } // on_socket_error

      
      void TcpSerialPort::on_low_level_read(void const *buff, uint4 buff_len)
      {
         if(log != 0)
            log->wr(buff, buff_len, true);
         SocketTcpSock::on_low_level_read(buff, buff_len);
      } // on_low_level_read


      void TcpSerialPort::on_low_level_write(void const *buff, uint4 buff_len)
      {
         if(log != 0)
            log->wr(buff, buff_len, false);
         SocketTcpSock::on_low_level_write(buff, buff_len);
      } // on_low_level_write
   };
};


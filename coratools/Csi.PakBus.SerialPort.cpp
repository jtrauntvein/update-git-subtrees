/* Csi.PakBus.SerialPort.cpp

   Copyright (C) 2012, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Tuesday 08 May 2012
   Last Change: Tuesday 28 January 2020
   Last Commit: $Date: 2020-01-28 11:11:06 -0600 (Tue, 28 Jan 2020) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.PakBus.SerialPort.h"


namespace Csi
{
   namespace PakBus
   {
      SerialPort::SerialPort(
         router_handle &router,
         StrAsc const &port_name_,
         uint4 baud_rate_):
         SerialPacketBase(router, router->get_timer()),
         port_name(port_name_),
         baud_rate(baud_rate_),
         report_open_id(0)
      { on_comm_enabled_change(true); }


      SerialPort::~SerialPort()
      {
         if(report_open_id && timer != 0)
            timer->disarm(report_open_id);
         close();
         router.clear();
      } // destructor


      void SerialPort::write_data(
         void const *buff, uint4 buff_len, uint4 repeat_count, uint4 msec_between)
      {
         if(is_open())
         {
            if(log != 0)
               log->wr(buff, buff_len, false);
            SerialPortBase::write_reps(buff, buff_len, repeat_count, msec_between);
         }
      } // write_data


      void SerialPort::on_hanging_up()
      {
         if(log != 0)
            log->force_break("closing serial port");
         SerialPortBase::close();
      }


      void SerialPort::on_needs_to_dial(priority_type priority)
      {
         try
         {
            if(!is_open())
               SerialPortBase::open(port_name, baud_rate, false);
         }
         catch(std::exception &e)
         {
            on_error(e.what());
         }
      } // on_needs_to_dial


      void SerialPort::onOneShotFired(uint4 id)
      {
         if(id == report_open_id)
         {
            if(log != 0)
               log->force_break("serial port open");
            report_open_id = 0;
            on_link_ready();
         }
         else
            SerialPacketBase::onOneShotFired(id);
      } // onOneShotFired


      void SerialPort::on_error(char const *message)
      {
         if(log != 0)
            log->force_break(message);
         SerialPacketBase::on_link_failed();
         router->log_comms(Router::comms_fault, message);
      } // on_error


      void SerialPort::on_read()
      {
         byte rx_buff[1024];
         uint4 rcd = read(rx_buff, sizeof(rx_buff));
         while(rcd > 0)
         {
            if(log != 0)
               log->wr(rx_buff, rcd, true);
            on_data_read(rx_buff, rcd);
            rcd = read(rx_buff, sizeof(rx_buff));
         }
      } // on_read


      void SerialPort::on_open()
      {
         report_open_id = timer->arm(this, 10);
      } // on_open
   };
};



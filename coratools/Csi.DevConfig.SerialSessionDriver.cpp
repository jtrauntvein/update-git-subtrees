/* Csi.DevConfig.SerialSessionDriver.cpp

   Copyright (C) 2008, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Saturday 24 May 2008
   Last Change: Friday 29 April 2016
   Last Commit: $Date: 2016-05-03 16:18:03 -0600 (Tue, 03 May 2016) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.DevConfig.SerialSessionDriver.h"
#include "Csi.StrAscStream.h"


namespace Csi
{
   namespace DevConfig
   {
      ////////////////////////////////////////////////////////////
      // class SerialSessionDriver definitions
      ////////////////////////////////////////////////////////////
      SerialSessionDriver::SerialSessionDriver(StrAsc const &serial_port_name_, uint4 baud_rate_):
         serial_port_name(serial_port_name_),
         baud_rate(baud_rate_)
      { }
         
      SerialSessionDriver::~SerialSessionDriver()
      { }

      
      void SerialSessionDriver::start_open(Csi::DevConfig::Session *session)
      {
         try
         {
            if(low_level_log != 0)
            {
               Csi::OStrAscStream msg;
               msg << "opening " << serial_port_name << " at " << baud_rate;
               low_level_log->force_break(msg.str().c_str());
            }
            open(serial_port_name, baud_rate);
         }
         catch(std::exception &e)
         {
            if(low_level_log != 0)
            {
               OStrAscStream msg;
               msg << "open failed: " << e.what();
               low_level_log->force_break(msg.str().c_str());
            }
            session->on_driver_failure();
         }
      } // start_open

      
      bool SerialSessionDriver::is_open(Csi::DevConfig::Session *session)
      { return SerialPortBase::is_open(); }

      
      void SerialSessionDriver::close(Csi::DevConfig::Session *session)
      {
         if(low_level_log != 0)
            low_level_log->force_break("closing");
         SerialPortBase::close();
      } // close

      
      void SerialSessionDriver::send(Csi::DevConfig::Session *session, void const *buff, uint4 buff_len)
      {
         SerialPortBase::write(buff, buff_len);
         if(low_level_log != 0)
            low_level_log->wr(buff, buff_len, false);
      } // send

      
      void SerialSessionDriver::on_error(char const *message)
      {
         session->on_driver_failure();
         if(low_level_log != 0)
         {
            OStrAscStream temp;
            temp << "low level serial failure: " << message;
            low_level_log->force_break(temp.str().c_str());
         }
      } // on_error

      
      void SerialSessionDriver::on_read()
      {
         // get the bytes from the serial port buffer and notify the session
         if(!rx_queue.empty())
         {
            read_buffer.cut(0);
            rx_queue.pop(read_buffer, rx_queue.size());
            if(low_level_log != 0)
               low_level_log->wr(read_buffer.getContents(), (uint4)read_buffer.length(), true);
         }
         session->on_driver_data(read_buffer.getContents(), (uint4)read_buffer.length());
      } // on_read

      
      void SerialSessionDriver::on_open()
      { session->on_driver_open(); }
   };
};


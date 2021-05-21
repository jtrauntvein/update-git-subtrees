/* Csi.PakBus.TcpSerialPort.h

   Copyright (C) 2018, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Wednesday 11 April 2018
   Last Change: Thursday 12 April 2018
   Last Commit: $Date: 2018-04-12 13:53:06 -0600 (Thu, 12 Apr 2018) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_PakBus_TcpSerialPort_h
#define Csi_PakBus_TcpSerialPort_h

#include "Csi.PakBus.SerialPacketBase.h"
#include "Csi.SocketTcpSock.h"
#include "Csi.LogByte.h"


namespace Csi
{
   namespace PakBus
   {
      /**
       * Defines an object that acts as a port for a PakBus router using a PakBus/TCP communication
       * services.
       */
      class TcpSerialPort: public SerialPacketBase, public SocketTcpSock
      {
      public:
         /**
          * Constructor
          *
          * @param router Specifies the router that will manage this port.
          *
          * @param server_address_ Specifies IP address or DNS name of the server.
          *
          * @param server_port_ Specifies the TCP port of the server.
          */
         TcpSerialPort(router_handle &router, StrAsc const &server_address_, uint2 server_port_);

         /**
          * Destructor
          */
         virtual ~TcpSerialPort();

         /**
          * Sets the low level logger for this port.
          *
          * @param log_ Specifies the object that will log messages sent or received on this port.
          */
         typedef SharedPtr<LogByte> log_handle;
         void set_log(log_handle log_);

         /**
          * @return Returns the low level log.
          */
         log_handle &get_log()
         { return log; }

         /**
          * Overloads the base class' method to start the connection process.
          */
         virtual void on_needs_to_dial(priority_type priority);

         /**
          * @return Overloads the base class version to return true if the socket is open.
          */
         virtual bool get_is_dialed()
         { return get_is_connected(); }

         /**
          * @return Overloads the base class version to return true if the link is dialed.
          */
         virtual bool link_is_dialed()
         { return false; }

      protected:
         /**
          * Overloads the base class version to write the data to the socket.
          */
         virtual void write_data(
            void const *buff, uint4 buff_len, uint4 repeat_count, uint4 msec_between);

         /**
          * Overloads the base class version to close the link resources.
          */
         virtual void on_hanging_up();

         /**
          * @return Overloads the base class version to return the beacon interval.
          */
         virtual uint2 get_beacon_interval()
         { return 60; }

         /**
          * @return Overloads the base class version to return a worst case response of 5 seconds.
          */
         virtual uint4 get_worst_case_response()
         { return 5000; }

         /**
          * @return Overloads the base class version to return the name of this port.
          */
         virtual StrAsc get_port_name() const;

         /**
          * Overloads the base class version to deal with the report of being open.
          */
         virtual void onOneShotFired(uint4 id);

         /**
          * Overloads the base class version to handle the connected report for the socket.
          */
         virtual void on_connected(SocketAddress const &connected_address);

         /**
          * Overloads the base class version to handle notification of invoming data.
          */
         virtual void on_read();

         /**
          * Overloads the base class to handle a socket error.
          */
         virtual void on_socket_error(int error_code);

         /**
          * Overloads the base class version to log the low level data.
          */
         virtual void on_low_level_read(void const *buff, uint4 buff_len);

         /**
          * Overloads the base class version to log the low level data.
          */
         virtual void on_low_level_write(void const *buff, uint4 buff_len);

      private:
         /**
          * Specifies the address of the server.
          */
         StrAsc server_address;

         /**
          * Specifies the TCP port of the server.
          */
         uint2 server_port;

         /**
          * Specifies the timer used to report that the connection has been opened.
          */
         uint4 report_open_id;

         /**
          * Specifies the low level log.
          */
         log_handle log;

         /**
          * Specifies the buffer used to transfer data to and from the socket buffers.
          */
         byte low_buff[1024];
      };
   };
};


#endif

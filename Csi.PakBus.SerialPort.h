/* Csi.PakBus.SerialPort.h

   Copyright (C) 2012, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Tuesday 08 May 2012
   Last Change: Thursday 12 April 2018
   Last Commit: $Date: 2018-04-12 13:53:06 -0600 (Thu, 12 Apr 2018) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_PakBus_SerialPort_h
#define Csi_PakBus_SerialPort_h

#include "Csi.PakBus.SerialPacketBase.h"
#include "Csi.SerialPortBase.h"
#include "Csi.LogByte.h"


namespace Csi
{
   namespace PakBus
   {
      /**
       * Defines an object that acts as a port for a PakBus router by using the
       * communication services provided by the Csi::SerialPortBase class.
       */
      class SerialPort:
         public SerialPacketBase,
         public SerialPortBase
      {
      public:
         /**
          * @param router  The PakBus router that this port will service.
          * @param port_name_  The name of the serial port that will be used.
          * @param baud_rate_  The baud rate that will be used when the port is opened.
          */
         SerialPort(
            router_handle &router,
            StrAsc const &port_name_,
            uint4 baud_rate_);

         /**
          * destructor
          */
         virtual ~SerialPort();

         /**
          * Sets the low level logger for this port.
          *
          * @param log_  The low level logger object
          */
         typedef SharedPtr<LogByte> log_handle;
         void set_log(log_handle log_)
         { log = log_; }

         /**
          * @return the current low level logger
          */
         SharedPtr<LogByte> &get_log()
         { return log; }

         /**
          * Overloads the base class' method to open the serial port.
          */
         virtual void on_needs_to_dial(priority_type priority);

         /**
          * @return Overloads the base class to return true if the serial port is open.
          */
         virtual bool get_is_dialed()
         { return is_open(); }

      protected:
         // @group: methods overloaded from class SerialPacketBase

         /**
          * Overloaded to write the specified data to the serial port.
          *
          * @param buff  pointer to the data to write
          * @param buff_len The amount of data to write
          * @param repeat_count  The number of times that the sequence should be repeated.
          * @param msec_between  The amount of time between repetitions.
          */
         virtual void write_data(
            void const *buff, uint4 buff_len, uint4 repeat_count, uint4 msec_between);

         /**
          * Overloads the base class' method to close the serial resources.
          */
         virtual void on_hanging_up();

         /**
          * Specifies a beacon interval of 60 seconds
          */
         virtual uint2 get_beacon_interval()
         { return 60; }

         /**
          * Specifies a worst case response of 5 seconds
          */
         virtual uint4 get_worst_case_response()
         { return 5000; }

         /**
          * Specifies that this link is dialed.
          */
         virtual bool link_is_dialed()
         { return false; }

         /**
          * Returns the name of the serial port.
          */
         virtual StrAsc get_port_name() const
         { return port_name; }

         /**
          * Overloads the base class version to deal with the report of being open.
          */
         virtual void onOneShotFired(uint4 id);
         
         // @endgroup:


         // @group: methods overloaded from class SerialPortBase

         /**
          * Overloads the base class' method to report a failure of the serial port.
          */
         virtual void on_error(char const *message);

         /**
          * Overloads the base class' method to report that data is available to be read.
          */
         virtual void on_read();

         /**
          * Overloads the base class' method to report when the port has been  successfully opened.
          */
         virtual void on_open();
         
         // @endgroup:

      private:
         ////////////////////////////////////////////////////////////
         // port_name
         ////////////////////////////////////////////////////////////
         StrAsc const port_name;

         ////////////////////////////////////////////////////////////
         // baud_rate
         ////////////////////////////////////////////////////////////
         uint4 baud_rate;

         ////////////////////////////////////////////////////////////
         // report_open_id
         ////////////////////////////////////////////////////////////
         uint4 report_open_id;

         ////////////////////////////////////////////////////////////
         // log
         ////////////////////////////////////////////////////////////
         log_handle log;
      };
   };
};


#endif


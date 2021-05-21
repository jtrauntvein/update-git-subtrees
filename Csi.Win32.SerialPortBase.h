/* Csi.Win32.SerialPortBase.h

   Copyright (C) 2005, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 21 December 2005
   Last Change: Friday 16 March 2018
   Last Commit: $Date: 2018-03-16 09:39:27 -0600 (Fri, 16 Mar 2018) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Csi_Win32_SerialPortBase_h
#define Csi_Win32_SerialPortBase_h

#include "Csi.Thread.h"
#include "Csi.CriticalSection.h"
#include "Csi.Condition.h"
#include "Csi.Events.h"
#include "Csi.ByteQueue.h"
#include "StrAsc.h"
#include <deque>

#pragma comment(lib,"setupapi.lib")


namespace Csi
{
   namespace Win32
   {
      //@group class forward declarations
      class SerialPortBase;
      //@endgroup

      
      namespace SerialPortBaseHelpers
      {
         ////////////////////////////////////////////////////////////
         // class CustomCommand
         //
         // Defines a class that can be added to the SerialPortBase queue.  The
         // execute() method will be invoked within the serial port thread.
         // These commands can be added by calling
         // SerialPortBase::add_custom_command(). 
         ////////////////////////////////////////////////////////////
         class CustomCommand
         {
         public:
            ////////////////////////////////////////////////////////////
            // execute
            ////////////////////////////////////////////////////////////
            virtual void execute(
               SerialPortBase *port,
               HANDLE port_handle,
               bool use_overlapped) = 0;
         };
      };

      
      ////////////////////////////////////////////////////////////
      // class SerialPortBase
      //
      // Defines a component that accesses a port using the windows COMM API.  
      ////////////////////////////////////////////////////////////
      class SerialPortBase:
         public Thread,
         public EventReceiver
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         //
         // Creates the port object in a non-bound state.  In order to be
         // connected, the open() function must be invoked.  
         ////////////////////////////////////////////////////////////
         SerialPortBase();

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~SerialPortBase();

         ////////////////////////////////////////////////////////////
         // list_ports
         //
         // Defines a static method that will provide a list of names of
         // potential serial port devices on this host.
         ////////////////////////////////////////////////////////////
         typedef std::deque<StrAsc> port_names_type;
         static void list_ports(port_names_type &port_names);

         ////////////////////////////////////////////////////////////
         // list_ports_friendly
         //
         // Defines a static method that provides a list of names of potential
         // serial port devices on this host along with their "friendly" names
         // where those can be obtained from the host operating system. 
         ////////////////////////////////////////////////////////////
         typedef std::pair<StrAsc, StrAsc> friendly_name_type;
         typedef std::deque<friendly_name_type> friendly_names_type;
         static void list_ports_friendly(friendly_names_type &port_names); 
         
         ////////////////////////////////////////////////////////////
         // open
         //
         // Opens the specified port using the specified baud rate. 
         ////////////////////////////////////////////////////////////
         virtual void open(
            StrAsc const &port_name_,
            uint4 baud_rate_,
            bool use_overlapped_ = true);

         ////////////////////////////////////////////////////////////
         // open (already opened handle)
         //
         // Opens the thread using a handle that has already been opened  (this
         // is useful with TAPI)
         ////////////////////////////////////////////////////////////
         virtual void open(
            HANDLE port_handle_,
            bool use_overlapped_ = true);

         ////////////////////////////////////////////////////////////
         // is_open
         ////////////////////////////////////////////////////////////
         virtual bool is_open() const;

         ////////////////////////////////////////////////////////////
         // close
         //
         // Returns the port into an unbound state (same as when constructed). 
         ////////////////////////////////////////////////////////////
         virtual void close();

         ////////////////////////////////////////////////////////////
         // write
         //
         // Adds data to the queue to be written.  The port must be in an open
         // state when this is called. 
         ////////////////////////////////////////////////////////////
         virtual void write(
            void const *buff,
            uint4 buff_len);

         ////////////////////////////////////////////////////////////
         // read
         //
         // Copies up to buff_len bytes from the read buffer to the provided
         // buffer.  The return value will be the number of bytes that were
         // copied. 
         ////////////////////////////////////////////////////////////
         virtual uint4 read(
            void *buff,
            uint4 buff_len);

         ////////////////////////////////////////////////////////////
         // set_hardware_flow_control
         ////////////////////////////////////////////////////////////
         virtual void set_hardware_flow_control(bool use_flow_control);

         ////////////////////////////////////////////////////////////
         // set_rts_state
         //
         // Sets the state of the RTS line
         ////////////////////////////////////////////////////////////
         virtual void set_rts_state(bool rts_state);

         ////////////////////////////////////////////////////////////
         // set_dtr_state
         ////////////////////////////////////////////////////////////
         virtual void set_dtr_state(bool dtr_state);

         ////////////////////////////////////////////////////////////
         // set_baud_rate
         ////////////////////////////////////////////////////////////
         virtual void set_baud_rate(uint4 baud_rate);

         ////////////////////////////////////////////////////////////
         // get_baud_rate
         ////////////////////////////////////////////////////////////
         virtual uint4 get_baud_rate() const
         { return baud_rate; }

         ////////////////////////////////////////////////////////////
         // write_reps
         ////////////////////////////////////////////////////////////
         virtual void write_reps(
            void const *buff,
            uint4 buff_len,
            uint4 repeat_count,
            uint4 delay_between_msec);

         ////////////////////////////////////////////////////////////
         // add_custom_command
         ////////////////////////////////////////////////////////////
         virtual void add_custom_command(
            SharedPtr<SerialPortBaseHelpers::CustomCommand> command);

      protected:
         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(SharedPtr<Event> &ev);
         
         ////////////////////////////////////////////////////////////
         // on_error
         //
         // Called when the port thread has encountered an error and has had to
         // abort.  
         ////////////////////////////////////////////////////////////
         virtual void on_error(char const *message) = 0;

         ////////////////////////////////////////////////////////////
         // on_read
         //
         // Notification of the receipt of a read event.  This will generally
         // mean that data can be read using the read() method.  
         ////////////////////////////////////////////////////////////
         virtual void on_read()
         { }

         ////////////////////////////////////////////////////////////
         // on_open
         //
         // Low level notification called by the thread after the port has been
         // opened.  
         ////////////////////////////////////////////////////////////
         virtual void on_open()
         { }

         ////////////////////////////////////////////////////////////
         // fill_write_buffer
         //
         // Called just before the thread enters a state where it will write
         // the data in its write buffer.  This will be called within the
         // thread and gives the application a hook to fill in details. 
         ////////////////////////////////////////////////////////////
         virtual void fill_write_buffer(ByteQueue &buffer)
         { }

         ////////////////////////////////////////////////////////////
         // test_open_port
         //
         // Called to initiate a command that will test the open serial port
         // handle in order to determine whether the handle is still valid. 
         ////////////////////////////////////////////////////////////
         virtual void test_serial_port();

      public:
         ////////////////////////////////////////////////////////////
         // on_low_level_read
         //
         // Called when data has been read by the thread.  This is a low level
         // notification and will be called from within the thread.  If the
         // application needs events synchronised with the CSI event queue, it
         // should overload on_read().
         //
         // The default version of this method will add the data to the receive
         // buffer and post an event that will cause the on_read() method to be
         // invoked.  An application can change this policy by overloading this
         // method. 
         ////////////////////////////////////////////////////////////
         virtual void on_low_level_read(
            void const *buff,
            uint4 buff_len);

         ////////////////////////////////////////////////////////////
         // on_low_level_write
         //
         // Called when data has been written by the thread.  This is a low
         // level notification and is called from within the thread. 
         ////////////////////////////////////////////////////////////
         virtual void on_low_level_write(
            void const *buff,
            uint4 buff_len)
         { }
         
      protected:
         ////////////////////////////////////////////////////////////
         // on_comm_errors
         //
         // Called from within the thread loop if communication errors are
         // detected. 
         ////////////////////////////////////////////////////////////
         virtual void on_comm_errors(uint4 errors)
         { }

         ////////////////////////////////////////////////////////////
         // on_carrier_detect_change
         //
         // Called from within the thread loop if the state of the carrier
         // detect line changes.
         ////////////////////////////////////////////////////////////
         virtual void on_carrier_detect_change(bool cd_enabled)
         { }

      private:
         ////////////////////////////////////////////////////////////
         // execute
         ////////////////////////////////////////////////////////////
         virtual void execute();

         ////////////////////////////////////////////////////////////
         // execute_simple
         ////////////////////////////////////////////////////////////
         void execute_simple();

         ////////////////////////////////////////////////////////////
         // execute_overlapped
         ////////////////////////////////////////////////////////////
         void execute_overlapped();
         
      protected:
         ////////////////////////////////////////////////////////////
         // port_handle
         ////////////////////////////////////////////////////////////
         HANDLE port_handle;

         ////////////////////////////////////////////////////////////
         // port_name
         ////////////////////////////////////////////////////////////
         StrAsc port_name;

         ////////////////////////////////////////////////////////////
         // baud_rate
         ////////////////////////////////////////////////////////////
         uint4 baud_rate;

         ////////////////////////////////////////////////////////////
         // attention
         ////////////////////////////////////////////////////////////
         Condition attention;

         ////////////////////////////////////////////////////////////
         // tx_queue
         ////////////////////////////////////////////////////////////
         ByteQueue tx_queue;

         ////////////////////////////////////////////////////////////
         // rx_queue
         ////////////////////////////////////////////////////////////
         ByteQueue rx_queue;

         ////////////////////////////////////////////////////////////
         // pending_commands
         ////////////////////////////////////////////////////////////
         typedef SharedPtr<SerialPortBaseHelpers::CustomCommand> command_handle;
         typedef std::list<command_handle> commands_type;
         CriticalSection commands_protector;
         commands_type commands;

         ////////////////////////////////////////////////////////////
         // should_close
         ////////////////////////////////////////////////////////////
         bool should_close;

         ////////////////////////////////////////////////////////////
         // use_overlapped
         //
         // Controls whether this class will attempt to use overlapped I/O
         // while open.  
         ////////////////////////////////////////////////////////////
         bool use_overlapped;
      };
   };
};


#endif

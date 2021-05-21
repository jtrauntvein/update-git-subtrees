/* Csi.Rfc2217.TerminalServerPort.h

   Copyright (C) 2011, 2012 Campbell Scientific, Inc.

   Written by: 
   Date Begun: Tuesday 22 February 2011
   Last Change: Tuesday 17 July 2012
   Last Commit: $Date: 2012-07-17 09:24:04 -0600 (Tue, 17 Jul 2012) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_Rfc2217_TerminalServerPort_h
#define Csi_Rfc2217_TerminalServerPort_h

#include "Csi.Rfc2217.Defs.h"
#include "OneShot.h"
#include "Csi.Events.h"
#include "Csi.ByteQueue.h"
#include "Csi.Utils.h"
#include <list>


namespace Csi
{
   namespace Rfc2217
   {
      // @group: class forward declarations

      class TerminalServerPort;
      
      // @endgroup:


      ////////////////////////////////////////////////////////////
      // class TerminalServerPortClient
      //
      // Defines the interface that an application that uses the
      // TerminalServerPort component  must implement.  This interface includes
      // methods that will inform the application when data has been received
      // for it to process, data is ready to transmit, or port related changes
      // have taken place.
      ////////////////////////////////////////////////////////////
      class TerminalServerPortClient: public InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_data_received
         //
         // Called when the terminal server has decoded data that must be
         // passed to the application. 
         ////////////////////////////////////////////////////////////
         virtual void on_data_received(
            TerminalServerPort *port,
            void const *buff,
            uint4 buff_len) = 0;

         ////////////////////////////////////////////////////////////
         // on_data_transmit
         //
         // Called when the terminal server has coded data that must be
         // transmitted by the application.
         ////////////////////////////////////////////////////////////
         virtual void on_data_transmit(
            TerminalServerPort *port,
            void const *buff,
            uint4 buff_len) = 0;

         ////////////////////////////////////////////////////////////
         // on_error
         //
         // Called when the terminal server has determined that the serial
         // protocol protocol has failed. 
         ////////////////////////////////////////////////////////////
         enum failure_reason_type
         {
            failure_unknown,
            failure_not_supported,
            failure_timed_out,
            failure_protocol
         };
         virtual void on_error(
            TerminalServerPort *port, failure_reason_type reason) = 0;

         ////////////////////////////////////////////////////////////
         // format_failure
         ////////////////////////////////////////////////////////////
         static void format_failure(
            std::ostream &out, failure_reason_type reason);
         
         ////////////////////////////////////////////////////////////
         // on_opened
         //
         // Called when the initial telnet negotiations have been successfully
         // completed.
         ////////////////////////////////////////////////////////////
         virtual void on_opened(
            TerminalServerPort *port)
         { }
         
         ////////////////////////////////////////////////////////////
         // on_linestate_change
         //
         // Called when a notification of the current line state has been
         // received. 
         ////////////////////////////////////////////////////////////
         virtual void on_linestate_change(
            TerminalServerPort *port,
            byte linestate)
         { }

         ////////////////////////////////////////////////////////////
         // on_modemstate_change
         ////////////////////////////////////////////////////////////
         virtual void on_modemstate_change(
            TerminalServerPort *port,
            byte modemstate)
         { } 
      };


      namespace TerminalServerPortHelpers
      {
         ////////////////////////////////////////////////////////////
         // class CommandBase
         ////////////////////////////////////////////////////////////
         class CommandBase
         {
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            CommandBase(TerminalServerPort *port_):
               port(port_),
               age_base(0)
            { }

            ////////////////////////////////////////////////////////////
            // destructor
            ////////////////////////////////////////////////////////////
            virtual ~CommandBase()
            { }

            ////////////////////////////////////////////////////////////
            // write_command
            ////////////////////////////////////////////////////////////
            virtual void write_command()
            { age_base = counter(0) - 1; }

            ////////////////////////////////////////////////////////////
            // on_telnet
            ////////////////////////////////////////////////////////////
            virtual void on_telnet(
               byte command, byte option)
            { }

            ////////////////////////////////////////////////////////////
            // on_serial_response
            ////////////////////////////////////////////////////////////
            virtual void on_serial_response(byte command)
            { }

            ////////////////////////////////////////////////////////////
            // is_complete
            ////////////////////////////////////////////////////////////
            virtual bool is_complete() = 0;

            ////////////////////////////////////////////////////////////
            // get_age
            ////////////////////////////////////////////////////////////
            virtual uint4 get_age()
            {
               uint4 rtn(0);
               if(age_base != 0)
                  rtn = counter(age_base);
               return rtn;
            }

         protected:
            ////////////////////////////////////////////////////////////
            // port
            ////////////////////////////////////////////////////////////
            TerminalServerPort *port;

            ////////////////////////////////////////////////////////////
            // age_base
            //
            // Keeps track of the amount of time elapsed since this command
            // was written.  Used to track whether the command has timed out. 
            ////////////////////////////////////////////////////////////
            uint4 age_base;
         };
      };

      
      ////////////////////////////////////////////////////////////
      // class TerminalServerPort
      //
      // This class defines an object that implements most of the details of a
      // the client side of the Serial Control Protocol as defined in RFC2217.
      // In order to use this component, an application must provide an object
      // derived from class TerminalServerPortClient.  
      ////////////////////////////////////////////////////////////
      class TerminalServerPort:
         public OneShotClient,
         public EventReceiver
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         typedef SharedPtr<OneShot> timer_handle;
         TerminalServerPort(timer_handle timer_ = 0);

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~TerminalServerPort();

         ////////////////////////////////////////////////////////////
         // start
         //
         // Called to initiate the serial control protocol using the specified
         // client.  This object must be in a finished state and the client
         // object be a valid instance.  Otherwise, a std::exception derived
         // exception will be thrown.
         ////////////////////////////////////////////////////////////
         typedef TerminalServerPortClient client_type;
         void start(
            client_type *client_,
            uint4 baud_rate_,
            datasize_option_type data_size_ = datasize_8,
            parity_option_type parity_ = parity_none,
            stopsize_option_type stop_size_ = stopsize_1);

         ////////////////////////////////////////////////////////////
         // get_client
         ////////////////////////////////////////////////////////////
         client_type *get_client()
         { return client; }
         
         ////////////////////////////////////////////////////////////
         // finish
         //
         // Called to clear out the internal state of this component.
         ////////////////////////////////////////////////////////////
         void finish();

         ////////////////////////////////////////////////////////////
         // send_data
         //
         // Called by the application to send data.  Eventually, this will call
         // the client's transmit_data() method but the original data specified
         // by the application maty be quoted in order to avoid any conflicts
         // with the control protocol.
         //
         // Depending on current negotiations and flow control state, the data
         // may not be sent immediately.
         ////////////////////////////////////////////////////////////
         void send_data(void const *buff, uint4 buff_len);

         ////////////////////////////////////////////////////////////
         // receive_data
         //
         // Called by the application when there is data for this state machine
         // to process.
         ////////////////////////////////////////////////////////////
         void receive_data(void const *buff, uint4 buff_len);

         ////////////////////////////////////////////////////////////
         // get_baud_rate
         ////////////////////////////////////////////////////////////
         uint4 get_baud_rate() const
         { return baud_rate; }

         ////////////////////////////////////////////////////////////
         // set_baud_rate
         ////////////////////////////////////////////////////////////
         void set_baud_rate(uint4 baud_rate_);

         ////////////////////////////////////////////////////////////
         // get_data_size
         ////////////////////////////////////////////////////////////
         datasize_option_type get_data_size() const
         { return data_size; }

         ////////////////////////////////////////////////////////////
         // get_parity
         ////////////////////////////////////////////////////////////
         parity_option_type get_parity() const
         { return parity; }

         ////////////////////////////////////////////////////////////
         // get_stop_size
         ////////////////////////////////////////////////////////////
         stopsize_option_type get_stop_size() const
         { return stop_size; }

         ////////////////////////////////////////////////////////////
         // get_hardware_flow_control
         ////////////////////////////////////////////////////////////
         bool get_hardware_flow_control() const
         { return hardware_flow_control; }

         ////////////////////////////////////////////////////////////
         // set_hardware_flow_control_enabled
         //
         // Sets up the terminal server so that it uses hardware flow control.
         ////////////////////////////////////////////////////////////
         void set_hardware_flow_control(bool enabled);

         ////////////////////////////////////////////////////////////
         // set_rts_on
         //
         // Sets up the terminal server to raise the RTS line.
         ////////////////////////////////////////////////////////////
         void set_rts_on(bool rts_on_);

         ////////////////////////////////////////////////////////////
         // set_dtr_on
         //
         // Sets up the terminal server to raise the DTR line.
         ////////////////////////////////////////////////////////////
         void set_dtr_on(bool dtr_on_);

         ////////////////////////////////////////////////////////////
         // get_line_state
         ////////////////////////////////////////////////////////////
         byte get_line_state() const
         { return line_state; }

         ////////////////////////////////////////////////////////////
         // get_modem_state
         ////////////////////////////////////////////////////////////
         byte get_modem_state() const
         { return modem_state; }
         
         ////////////////////////////////////////////////////////////
         // onOneShotFired
         ////////////////////////////////////////////////////////////
         virtual void onOneShotFired(uint4 id);

         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(SharedPtr<Event> &ev);

      private:
         ////////////////////////////////////////////////////////////
         // add_command
         ////////////////////////////////////////////////////////////
         typedef TerminalServerPortHelpers::CommandBase command_type;
         typedef SharedPtr<command_type> command_handle;
         void add_command(command_handle command, bool turn_after = true);
         
         ////////////////////////////////////////////////////////////
         // turn_the_crank
         //
         // Implements the code that maintains the commands.
         ////////////////////////////////////////////////////////////
         bool turn_the_crank();

         ////////////////////////////////////////////////////////////
         // decode_rx_data
         //
         // Implements the state machine for decoding received data.
         ////////////////////////////////////////////////////////////
         void decode_rx_data();

         ////////////////////////////////////////////////////////////
         // decode_started_data
         //
         // Decodes any data received while in a started state
         ////////////////////////////////////////////////////////////
         void decode_started_data();
         
      private:
         ////////////////////////////////////////////////////////////
         // client
         ////////////////////////////////////////////////////////////
         client_type *client;

         ////////////////////////////////////////////////////////////
         // timer
         ////////////////////////////////////////////////////////////
         timer_handle timer;

         ////////////////////////////////////////////////////////////
         // crank_id
         //
         // Used to 
         ////////////////////////////////////////////////////////////
         uint4 crank_id;

         ////////////////////////////////////////////////////////////
         // tx_buff
         //
         // Used to buffer data that should be sent to the terminal server.
         ////////////////////////////////////////////////////////////
         ByteQueue tx_buff;

         ////////////////////////////////////////////////////////////
         // rx_buff
         //
         // Used to buffer data that has been received from the terminal
         // server. 
         ////////////////////////////////////////////////////////////
         ByteQueue rx_buff;

         ////////////////////////////////////////////////////////////
         // transfer_buff
         ////////////////////////////////////////////////////////////
         byte transfer_buff[2048];

         ////////////////////////////////////////////////////////////
         // line_state
         ////////////////////////////////////////////////////////////
         byte line_state;

         ////////////////////////////////////////////////////////////
         // modem_state
         ////////////////////////////////////////////////////////////
         byte modem_state;

         ////////////////////////////////////////////////////////////
         // commands
         ////////////////////////////////////////////////////////////
         typedef std::list<command_handle> commands_type;
         commands_type commands;

         ////////////////////////////////////////////////////////////
         // baud_rate
         ////////////////////////////////////////////////////////////
         uint4 baud_rate;

         ////////////////////////////////////////////////////////////
         // data_size
         ////////////////////////////////////////////////////////////
         datasize_option_type data_size;

         ////////////////////////////////////////////////////////////
         // parity
         ////////////////////////////////////////////////////////////
         parity_option_type parity;

         ////////////////////////////////////////////////////////////
         // stop_size
         ////////////////////////////////////////////////////////////
         stopsize_option_type stop_size;

         ////////////////////////////////////////////////////////////
         // hardware_flow_control
         ////////////////////////////////////////////////////////////
         bool hardware_flow_control;

         ////////////////////////////////////////////////////////////
         // can_write
         ////////////////////////////////////////////////////////////
         bool can_write;

         ////////////////////////////////////////////////////////////
         // state
         ////////////////////////////////////////////////////////////
         enum state_type
         {
            state_standby,
            state_started,
            state_cr,
            state_iac,
            state_telnet_will,
            state_telnet_wont,
            state_telnet_do,
            state_telnet_dont,
            state_sb,
            state_se,
            state_serial_cmd,
            state_unrecognised,
            state_data_size,
            state_parity,
            state_stop_size,
            state_control,
            state_line_state,
            state_modem_state,
            state_baud_rate,
            state_line_state_mask,
            state_modem_state_mask,
            state_purge_data,
            state_end_serial
         } state;

         ////////////////////////////////////////////////////////////
         // quotes_cr
         //
         // Flags whether the terminal server is expected to append a null
         // character after an embedded LF.  Some devices, like Perle's IOLan
         // will do this while others, such as NetModem, do not appear to do
         // this.  We will assume that the terminal server quotes embedded CR
         // characters until we find evidence that it doesn't.
         ////////////////////////////////////////////////////////////
         bool quotes_cr;
      };
   };
};

#endif

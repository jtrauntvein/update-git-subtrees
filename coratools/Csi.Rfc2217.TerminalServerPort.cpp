/* Csi.Rfc2217.TerminalServerPort.cpp

   Copyright (C) 2011, 2016 Campbell Scientific, Inc.

   Written by: 
   Date Begun: Tuesday 22 February 2011
   Last Change: Friday 29 April 2016
   Last Commit: $Date: 2016-05-03 16:18:03 -0600 (Tue, 03 May 2016) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Rfc2217.TerminalServerPort.h"
#include "Csi.MaxMin.h"
#include <iostream>


/* This module implements the state machine described in the dot syntax that follows:

digraph rfc2217
{
   labelfloat=true;
   begin [shape=circle,style=filled,width=.25,label=""];
   begin -> standby;
   standby -> started;
   started -> started [label="!IAC"];
   started -> iac [label="IAC"];
   iac -> started [label="IAC"];

   iac -> telnet_will [label="WILL"];
   iac -> telnet_wont [label="WONT"];
   iac -> telnet_do [label="DO"];
   iac -> telnet_dont [label="DONT"];
   telnet_will -> started [label="next"];
   telnet_wont -> error [label="COM-PORT-OPTION"];
   telnet_do -> started [label="next"];
   telnet_dont -> error [label="COM-PORT-OPTION"];
   telnet_wont -> started [label="other"];
   telnet_dont -> started [label="other"];

   error -> standby [label="report error"];

   iac -> sb [label="SB"];
   sb -> unrecognised [label="other"];
   unrecognised -> unrecognised [label="!IAC"];
   unrecognised -> se [label="IAC"];
   se -> started [label="SE"];
   se -> unrecognised [label="IAC"];
   sb -> serial_cmd [label="COM-PORT-OPTION"];

   serial_cmd -> baud_rate [label="101"];
   baud_rate -> end_serial [label="baud rate value (4 bytes)"];

   serial_cmd -> data_size [label="102"];
   data_size -> end_serial [label="data size value"];
   
   serial_cmd -> parity [label="103"];
   parity -> end_serial [label="parity value"];

   serial_cmd -> stop_size [label="104"];
   stop_size -> end_serial [label="stop size value"];

   serial_cmd -> control [label="105"];
   control -> end_serial [label="control value"];

   serial_cmd -> line_state [label="106"];
   line_state -> end_serial [label="line state value"];

   serial_cmd -> modem_state [label="107"];
   modem_state -> end_serial [label="modem state value"];

   serial_cmd -> end_serial [label="108 || 109"];
   
   serial_cmd -> line_state_mask [label="110"];
   line_state_mask -> end_serial [label="mask value"];
   
   serial_cmd -> modem_state_mask [label="111"];
   modem_state_mask -> end_serial [label="mask value"];
   
   serial_cmd -> purge_data [label="112"];
   purge_data -> end_serial [label="option"];

   end_serial -> se [label="IAC"];
   end_serial -> serial_cmd [label="other"];
}

*/


namespace Csi
{
   namespace Rfc2217
   {
      namespace
      {
         ////////////////////////////////////////////////////////////
         // class command_start_option
         ////////////////////////////////////////////////////////////
         class command_start_option: public TerminalServerPortHelpers::CommandBase
         {
         private:
            ////////////////////////////////////////////////////////////
            // complete
            ////////////////////////////////////////////////////////////
            bool complete;
            
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            command_start_option(TerminalServerPort *port):
               CommandBase(port),
               complete(false)
            { }

            ////////////////////////////////////////////////////////////
            // write_command
            ////////////////////////////////////////////////////////////
            virtual void write_command()
            {
               byte const command[] = {
                  code_telnet_iac, code_telnet_will, code_serial_option
               };
               port->get_client()->on_data_transmit(port, command, sizeof(command));
               CommandBase::write_command();
            }

            ////////////////////////////////////////////////////////////
            // on_telnet
            ////////////////////////////////////////////////////////////
            virtual void on_telnet(
               byte command, byte option)
            {
               if(get_age() != 0 && option == code_serial_option)
               {
                  if(command == code_telnet_will || command == code_telnet_do)
                     complete = true;
                  else
                     throw TerminalServerPortClient::failure_not_supported;
               }
            }

            ////////////////////////////////////////////////////////////
            // is_complete
            ////////////////////////////////////////////////////////////
            virtual bool is_complete()
            { return complete; }
         };

         
         ////////////////////////////////////////////////////////////
         // class command_set_baud_rate
         ////////////////////////////////////////////////////////////
         class command_set_baud_rate: public TerminalServerPortHelpers::CommandBase
         {
         private:
            ////////////////////////////////////////////////////////////
            // baud_rate
            ////////////////////////////////////////////////////////////
            uint4 baud_rate;

            ////////////////////////////////////////////////////////////
            // complete
            ////////////////////////////////////////////////////////////
            bool complete;
            
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            command_set_baud_rate(
               TerminalServerPort *port, uint4 baud_rate_):
               CommandBase(port),
               baud_rate(baud_rate_),
               complete(false)
            {
               if(!is_big_endian())
                  reverse_byte_order(&baud_rate, sizeof(baud_rate));
            }

            ////////////////////////////////////////////////////////////
            // write_command
            ////////////////////////////////////////////////////////////
            virtual void write_command()
            {
               StrBin command;
               
               command.append(code_telnet_iac);
               command.append(code_telnet_sb);
               command.append(code_serial_option);
               command.append(code_serial_set_baud);
               command.append(&baud_rate, sizeof(baud_rate));
               command.append(code_telnet_iac);
               command.append(code_telnet_se);
               port->get_client()->on_data_transmit(
                  port, command.getContents(), (uint4)command.length());
               CommandBase::write_command();
            }

            ////////////////////////////////////////////////////////////
            // on_serial_response
            ////////////////////////////////////////////////////////////
            virtual void on_serial_response(byte command)
            {
               if(get_age() != 0 && command == code_serial_set_baud)
                  complete = true;
            }

            ////////////////////////////////////////////////////////////
            // is_complete
            ////////////////////////////////////////////////////////////
            virtual bool is_complete()
            { return complete || get_age() > 100; }
         };


         ////////////////////////////////////////////////////////////
         // class command_serial_1b
         ////////////////////////////////////////////////////////////
         class command_serial_1b: public TerminalServerPortHelpers::CommandBase
         {
         private:
            ////////////////////////////////////////////////////////////
            // command
            ////////////////////////////////////////////////////////////
            byte command;

            ////////////////////////////////////////////////////////////
            // param
            ////////////////////////////////////////////////////////////
            byte param;

            ////////////////////////////////////////////////////////////
            // complete
            ////////////////////////////////////////////////////////////
            bool complete;

         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            command_serial_1b(
               TerminalServerPort *port,
               byte command_,
               byte param_):
               CommandBase(port),
               command(command_),
               param(param_),
               complete(false)
            { }

            ////////////////////////////////////////////////////////////
            // write_command
            ////////////////////////////////////////////////////////////
            virtual void write_command()
            {
               StrBin command;
               command.append(code_telnet_iac);
               command.append(code_telnet_sb);
               command.append(code_serial_option);
               command.append(this->command);
               if(this->param == code_telnet_iac)
                  command.append(this->param);
               command.append(this->param);
               command.append(code_telnet_iac);
               command.append(code_telnet_se);
               port->get_client()->on_data_transmit(
                  port, command.getContents(), (uint4)command.length());
               CommandBase::write_command();
               
               // NetModem does not seem to acknowledge any of our commands so we will consider them
               // to be complete
               complete = true; 
            }

            ////////////////////////////////////////////////////////////
            // on_serial_response
            ////////////////////////////////////////////////////////////
            virtual void on_serial_response(byte command_)
            {
               if(get_age() != 0 && command_ == command)
                  complete = true;
            }

            ////////////////////////////////////////////////////////////
            // is_complete
            ////////////////////////////////////////////////////////////
            virtual bool is_complete()
            { return complete || get_age() > 100; }
         };


         ////////////////////////////////////////////////////////////
         // class command_report_open
         ////////////////////////////////////////////////////////////
         class command_report_open: public TerminalServerPortHelpers::CommandBase
         {
         private:
            ////////////////////////////////////////////////////////////
            // complete
            ////////////////////////////////////////////////////////////
            bool complete;
            
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            command_report_open(TerminalServerPort *port):
               CommandBase(port),
               complete(false)
            { }

            ////////////////////////////////////////////////////////////
            // write_command
            ////////////////////////////////////////////////////////////
            virtual void write_command()
            {
               CommandBase::write_command();
               port->get_client()->on_opened(port);
               complete = true;
            }

            ////////////////////////////////////////////////////////////
            // is_complete
            ////////////////////////////////////////////////////////////
            virtual bool is_complete()
            { return complete; }
         };


         ////////////////////////////////////////////////////////////
         // crank_event
         ////////////////////////////////////////////////////////////
         uint4 const crank_event(
            Event::registerType("Csi::Rfc2217::TerminalServerPort::crank"));
      };


      ////////////////////////////////////////////////////////////
      // class TerminalServerPortClient definitions
      ////////////////////////////////////////////////////////////
      void TerminalServerPortClient::format_failure(
         std::ostream &out, failure_reason_type reason)
      {
         switch(reason)
         {
         case failure_not_supported:
            out << "serial control not supported";
            break;
            
         case failure_timed_out:
            out << "serial control negotiations timed out";
            break;
         
         case failure_protocol:
            out << "invalid serial control negotiations";
            break;
            
         default:
            out << "unrecognised failure";
            break;
         }
      } // format_failure
      
      
      ////////////////////////////////////////////////////////////
      // class TerminalServerPort definitions
      ////////////////////////////////////////////////////////////
      TerminalServerPort::TerminalServerPort(timer_handle timer_):
         timer(timer_),
         client(0),
         tx_buff(sizeof(transfer_buff), true),
         rx_buff(sizeof(transfer_buff), true),
         state(state_standby),
         can_write(true),
         crank_id(0),
         quotes_cr(true)
      {
         if(timer == 0)
            timer.bind(new OneShot);
      } // constructor


      TerminalServerPort::~TerminalServerPort()
      { finish(); }


      void TerminalServerPort::start(
         client_type *client_,
         uint4 baud_rate_,
         datasize_option_type data_size_,
         parity_option_type parity_,
         stopsize_option_type stop_size_)
      {
         // check the state of this port and set the members
         if(!client_type::is_valid_instance(client_))
            throw std::invalid_argument("invalid client pointer");
         if(client != 0)
            throw std::invalid_argument("TerminalServerPort already started");
         client = client_;
         baud_rate = baud_rate_;
         data_size = data_size_;
         parity = parity_;
         stop_size = stop_size_;
         can_write = true;
         state = state_started;
         quotes_cr = true;

         // we now need to post commands to negotiate the options and set the port parameters
         add_command(new command_start_option(this), false);
         add_command(new command_set_baud_rate(this, baud_rate), false);
         add_command(new command_serial_1b(this, code_serial_set_datasize, data_size), false);
         add_command(new command_serial_1b(this, code_serial_set_parity, parity), false);
         add_command(new command_serial_1b(this, code_serial_set_stopsize, stop_size), false);
         add_command(new command_serial_1b(this, code_serial_set_linestate_mask, 0xFF), false);
         add_command(new command_serial_1b(this, code_serial_set_modemstate_mask, 0xFF), false);
         add_command(new command_report_open(this));
      } // start


      void TerminalServerPort::finish()
      {
         state = state_standby;
         commands.clear();
         client = 0;
         if(timer != 0 && crank_id != 0)
            timer->disarm(crank_id);
      } // finish


      void TerminalServerPort::send_data(void const *buff_, uint4 buff_len)
      {
         // we need to quote any IAC data in the data to be transmitted so that the terminal server
         // will not get confused.  
         byte const *buff(static_cast<byte const *>(buff_));
         byte last = 0;
         for(byte const *b = buff; static_cast<uint4>(b - buff) < buff_len; ++b)
         {
            if(*b == code_telnet_iac)
               tx_buff.push(b, 1);
            tx_buff.push(b, 1);
            if(*b == code_telnet_cr && quotes_cr)
            {
               uint4 pos = static_cast<uint4>(b - buff);
               if(pos + 1 >= buff_len || buff[pos + 1] != '\n')
               {
                  byte nul_value(0);
                  tx_buff.push(&nul_value, 1);
               }
            }
         }
         Event::create(crank_event, this)->post();
      } // send_data


      void TerminalServerPort::receive_data(void const *buff, uint4 buff_len)
      {
         Event *event(Event::create(crank_event, this));
         rx_buff.push(buff, buff_len);
         event->post();
      } // receive_data


      void TerminalServerPort::onOneShotFired(uint4 id)
      {
         if(id == crank_id)
         {
            if(turn_the_crank())
            {
               if(!commands.empty())
                  crank_id = timer->arm(this, 100);
            }
         }
      } // onOneShotFired


      void TerminalServerPort::receive(SharedPtr<Event> &ev)
      {
         if(ev->getType() == crank_event)
            turn_the_crank();
      } // receive

      void TerminalServerPort::set_baud_rate(uint4 baud_rate_)
      {
         baud_rate = baud_rate_;
         add_command(new command_set_baud_rate(this, baud_rate));
      } // set_baud_rate


      void TerminalServerPort::set_hardware_flow_control(bool enabled)
      {
         hardware_flow_control = enabled;
         if(enabled)
         {
            add_command(
               new command_serial_1b(
                  this, code_serial_set_control, control_hardware_flow_control_both));
         }
         else
         {
            add_command(
               new command_serial_1b(
                  this, code_serial_set_control, control_no_flow_control_both));
         }
      } // set_hardware_clow_control


      void TerminalServerPort::set_rts_on(bool rts_on)
      {
         if(rts_on)
            add_command(
               new command_serial_1b(
                  this, code_serial_set_control, control_set_rts_on));
         else
            add_command(
               new command_serial_1b(
                  this, code_serial_set_control,  control_set_rts_off));
      } // set_rts_on


      void TerminalServerPort::set_dtr_on(bool dtr_on)
      {
         if(dtr_on)
            add_command(
               new command_serial_1b(
                  this, code_serial_set_control, control_set_dtr_on));
         else
            add_command(
               new command_serial_1b(
                  this, code_serial_set_control, control_set_rts_on));
      } // set_dtr_on


      void TerminalServerPort::add_command(command_handle command, bool turn_after)
      {
         if(!client_type::is_valid_instance(client)) 
            throw std::invalid_argument("invalid client pointer");
         if(state == state_standby)
            throw std::invalid_argument("invalid state");
         commands.push_back(command);
         if(turn_after)
         {
            turn_the_crank();
            if(crank_id == 0)
               crank_id = timer->arm(this, 100);
         }
      } // add_command


      bool TerminalServerPort::turn_the_crank()
      {
         // we will do nothing if the client is not valid
         bool rtn(true);
         if(!client_type::is_valid_instance(client))
            return false;
         try
         {
            // process any data waiting in the rx queue
            if(rx_buff.size() > 0)
               decode_rx_data();
            
            // eliminate any commands that are complete
            commands_type::iterator ci(commands.begin());
            while(ci != commands.end())
            {
               command_handle &command(*ci);
               if(command->is_complete())
               {
                  commands_type::iterator dci(ci++);
                  commands.erase(dci);
               }
               else
                  ++ci;
            }
            
            // check to see if the first command needs to write
            if(!commands.empty() && can_write)
            {
               // we need to give the command an opportunity to write its data.  
               command_handle &first(commands.front());
               uint4 first_age(first->get_age());
               if(first_age == 0)
               {
                  first->write_command();
                  if(crank_id == 0)
                     crank_id = timer->arm(this, 10000);
               }
               if(first_age >= 10000)
                  throw client_type::failure_timed_out;
            }
            while(can_write && commands.empty() && tx_buff.size() > 0)
            {
               uint4 to_write(tx_buff.pop(transfer_buff, sizeof(transfer_buff)));
               client->on_data_transmit(this, transfer_buff, to_write); 
            }
         }
         catch(client_type::failure_reason_type reason)
         {
            client_type *client(this->client);
            finish();
            if(client_type::is_valid_instance(client))
               client->on_error(this, reason);
            rtn = false;
         }
         catch(std::exception &)
         {
            client_type *client(this->client);
            finish();
            if(client_type::is_valid_instance(client))
               client->on_error(this, client_type::failure_unknown);
            rtn = false;
         }
         return rtn;
      } // turn_the_crank


      void TerminalServerPort::decode_rx_data()
      {
         byte current;
         command_handle first;
         char const cr_value('\r');

         if(!commands.empty())
            first = commands.front();
         while(state != state_standby && rx_buff.size() > 0)
         {
            rx_buff.copy(&current, 1);
            switch(state)
            {
            case state_started:
               decode_started_data();
               break;

            case state_cr:
               rx_buff.pop(1);
               state = state_started;
               if(current == '\0' && quotes_cr)
               {
                  // the "CR NUL" sequence needs to be interpreted as a single CR (RFC 854 under
                  // section entitled "The NVT and Keyboard").  We already sent the CR to the
                  // client so there is nothing more to do here.
               }
               else
               {
                  // Ken has seen cases where we are getting a CR FF FF sequence and this is pushing
                  // the state machine into an invalid state.  The first FF was being ignored.
		  if(current == code_telnet_iac)
                     state = state_iac;
		  else
                  {
                     client->on_data_received(this, &current, 1);
                     if(current != '\n')
			quotes_cr = false;
                  }
               }
               break;
               
            case state_iac:
               rx_buff.pop(1);
               switch(current)
               {
               case code_telnet_will:
                  state = state_telnet_will;
                  break;
                  
               case code_telnet_wont:
                  state = state_telnet_wont;
                  break;
                  
               case code_telnet_do:
                  state = state_telnet_do;
                  break;
                  
               case code_telnet_dont:
                  state = state_telnet_dont;
                  break;
                  
               case code_telnet_sb:
                  state = state_sb;
                  break;

               case code_telnet_iac: // quoted IAC
                  client->on_data_received(this, &current, 1);
                  state = state_started;
                  break;

               case code_telnet_se:
                  state = state_started;
                  break;

               default:
                  throw client_type::failure_protocol;
                  break;
               }
               break;
               
            case state_telnet_will:
               rx_buff.pop(1);
               if(first != 0)
                  first->on_telnet(code_telnet_will, current);
               state = state_started;
               break;

            case state_telnet_wont:
               rx_buff.pop(1);
               if(first != 0)
                  first->on_telnet(code_telnet_wont, current);
               state = state_started; 
               break;

            case state_telnet_do:
               rx_buff.pop(1);
               if(first != 0)
                  first->on_telnet(code_telnet_do, current);
               state = state_started;
               break;

            case state_telnet_dont:
               rx_buff.pop(1);
               if(first != 0)
                  first->on_telnet(code_telnet_wont, current);
               state = state_started;
               break;

            case state_sb:
               rx_buff.pop(1);
               if(current == code_serial_option)
                  state = state_serial_cmd;
               else
                  state = state_unrecognised;
               break;
               
            case state_unrecognised:
               rx_buff.pop(1);
               if(current == code_telnet_iac)
                  state = state_se; 
               break;
               
            case state_se:
               rx_buff.pop(1);
               if(current == code_telnet_iac)
                  state = state_unrecognised;
               else if(current == code_telnet_se)
                  state = state_started;
               else
                  throw client_type::failure_protocol;
               break;
               
            case state_serial_cmd:
               rx_buff.pop(1);
               switch(current)
               {
               case code_serial_set_baud + 100:
                  state = state_baud_rate;
                  break;
                  
               case code_serial_set_datasize + 100:
                  state = state_data_size;
                  break;
                  
               case code_serial_set_parity + 100:
                  state = state_parity;
                  break;
                  
               case code_serial_set_stopsize + 100:
                  state = state_stop_size;
                  break;
                  
               case code_serial_set_control + 100:
                  state = state_control;
                  break;

               case code_serial_notify_linestate + 100:
                  state = state_line_state;
                  break;
                  
               case code_serial_notify_modemstate + 100:
                  state = state_modem_state;
                  break;
                  
               case code_serial_flowcontrol_suspend + 100:
                  can_write = false;
                  state = state_end_serial;
                  break;
                  
               case code_serial_flowcontrol_resume + 100:
                  can_write = true;
                  state = state_end_serial;
                  break;
                  
               case code_serial_set_linestate_mask + 100:
                  state = state_line_state_mask;
                  break;
                  
               case code_serial_set_modemstate_mask + 100:
                  state = state_modem_state_mask;
                  break;
                  
               case code_serial_purge_data + 100:
                  state = state_purge_data;
                  break;
                  
               default:
                  state = state_unrecognised;
                  break;
               }
               break;
               
            case state_data_size:
               rx_buff.pop(1);
               if(first != 0)
                  first->on_serial_response(code_serial_set_datasize);
               data_size = static_cast<datasize_option_type>(current);
               state = state_end_serial;
               break;
               
            case state_parity:
               rx_buff.pop(1);
               if(first != 0)
                  first->on_serial_response(code_serial_set_parity);
               parity = static_cast<parity_option_type>(current);
               state = state_end_serial;
               break;
               
            case state_stop_size:
               rx_buff.pop(1);
               if(first != 0)
                  first->on_serial_response(code_serial_set_stopsize);
               stop_size = static_cast<stopsize_option_type>(current);
               state = state_end_serial;
               break;
               
            case state_control:
               rx_buff.pop(1);
               if(first != 0)
                  first->on_serial_response(code_serial_set_control);
               state = state_end_serial;
               break;
               
            case state_line_state:
               rx_buff.pop(1);
               line_state = current;
               client->on_linestate_change(this, line_state);
               state = state_end_serial;
               break;
               
            case state_modem_state:
               rx_buff.pop(1);
               modem_state = current;
               client->on_modemstate_change(this, modem_state);
               state = state_end_serial;
               break;
               
            case state_baud_rate:
               if(rx_buff.size() >= sizeof(baud_rate))
               {
                  rx_buff.pop(&baud_rate, sizeof(baud_rate));
                  if(!is_big_endian())
                     reverse_byte_order(&baud_rate, sizeof(baud_rate));
                  if(first != 0)
                     first->on_serial_response(code_serial_set_baud);
                  state = state_end_serial;
               }
               break;
               
            case state_line_state_mask:
               rx_buff.pop(1);
               if(current == code_telnet_iac)
                  rx_buff.pop(1);
               if(first != 0)
                  first->on_serial_response(code_serial_set_linestate_mask);
               state = state_end_serial;
               break;
               
            case state_modem_state_mask:
               rx_buff.pop(1);
               if(current == code_telnet_iac)
                  rx_buff.pop(1);
               if(first != 0)
                  first->on_serial_response(code_serial_set_modemstate_mask);
               state = state_end_serial;
               break;
               
            case state_purge_data:
               rx_buff.pop(1);
               if(first != 0)
                  first->on_serial_response(code_serial_purge_data);
               state = state_end_serial;
               break;

            case state_end_serial:
               if(current == code_telnet_iac)
               {
                  rx_buff.pop(1);
                  state = state_se;
               }
               else
                  state = state_serial_cmd;
               break;

            default:
               throw client_type::failure_unknown;
               break;
            }
         }
      } // decode_rx_data


      void TerminalServerPort::decode_started_data()
      {
         // we want to process any characters preceding the IAC
         byte const iac_value(code_telnet_iac);
         byte const cr_value('\r');
         uint4 iac_pos(rx_buff.find(&iac_value, 1));
         uint4 cr_pos(rx_buff.find(&cr_value, 1));
         
         while(iac_pos > 0 && cr_pos > 0)
         {
            uint4 stop_pos(csimin(iac_pos, cr_pos));
            uint4 count(csimin<uint4>(stop_pos, sizeof(transfer_buff)));
            rx_buff.pop(transfer_buff, count);
            if(iac_pos >= count)
               iac_pos -= count;
            if(cr_pos >= count)
               cr_pos -= count;
            client->on_data_received(this, transfer_buff, count);
         }

         // if we have now found an IAC, we need to switch states
         if(iac_pos == 0 && rx_buff.size() > 0)
         {
            rx_buff.pop(1);
            state = state_iac;
         }
         if(cr_pos == 0 && rx_buff.size() > 0)
         {
            rx_buff.pop(1);
            client->on_data_received(this, &cr_value, 1);
            state = state_cr;
         }
      } // decoded_started_data
   };
};


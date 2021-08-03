/* Csi.Posix.SerialPortBase.cpp

   Copyright (C) 2005, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 21 December 2005
   Last Change: Friday 27 April 2018
   Last Commit: $Date: 2018-04-28 09:41:03 -0600 (Sat, 28 Apr 2018) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Posix.SerialPortBase.h"
#include "Csi.OsException.h"
#include "../coratools/Csi.FileSystemObject.h"
#include <sstream>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>


namespace Csi
{
   namespace Posix
   {
      namespace
      {
         ////////////////////////////////////////////////////////////
         // INVALID_HANDLE_VALUE
         ////////////////////////////////////////////////////////////
         int const INVALID_HANDLE_VALUE = -1;

         
         ////////////////////////////////////////////////////////////
         // class event_error
         ////////////////////////////////////////////////////////////
         class event_error: public Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // message
            ////////////////////////////////////////////////////////////
            StrAsc message;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               SerialPortBase *port,
               StrAsc const &message)
            {
               event_error *ev = new event_error(port,message);
               ev->post();
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_error(
               SerialPortBase *port,
               StrAsc const &message_):
               Event(event_id,port),
               message(message_)
            { }
         };


         uint4 const event_error::event_id =
         Event::registerType("Csi::Posix::SerialPortBase::event_error");


         ////////////////////////////////////////////////////////////
         // class event_read
         ////////////////////////////////////////////////////////////
         class event_read: public Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(SerialPortBase *port)
            {
               event_read *event = new event_read(port);
               event->post();
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_read(SerialPortBase *port):
               Event(event_id,port)
            { }
         };


         uint4 const event_read::event_id =
         Event::registerType("Csi::Posix::SerialPortBase::event_read");


         ////////////////////////////////////////////////////////////
         // class command_set_flow_control
         ////////////////////////////////////////////////////////////
         class command_set_flow_control:
            public SerialPortBaseHelpers::CustomCommand
         {
         private:
            ////////////////////////////////////////////////////////////
            // flow_control_enabled
            ////////////////////////////////////////////////////////////
            bool flow_control_enabled;
            
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            command_set_flow_control(bool flow_control_enabled_):
               flow_control_enabled(flow_control_enabled_)
            { }

            ////////////////////////////////////////////////////////////
            // execute
            ////////////////////////////////////////////////////////////
            virtual void execute(
               SerialPortBase *port,
               HANDLE port_handle)
            {
               struct termios options;
               tcgetattr(port_handle,&options);
#ifdef CNEW_RTSCTS
               if(flow_control_enabled)
                  options.c_cflag |= CNEW_RTSCTS;
               else
                  options.c_cflag &= ~CNEW_RTSCTS;
#else
               if(flow_control_enabled)
                  options.c_cflag |= CRTSCTS;
               else
                  options.c_cflag &= ~CRTSCTS;
#endif
               tcsetattr(port_handle,TCSANOW,&options);
            }
         };


         ////////////////////////////////////////////////////////////
         // class command_set_control_line
         ////////////////////////////////////////////////////////////
         class command_set_control_line:
            public SerialPortBaseHelpers::CustomCommand
         {
         public:
            ////////////////////////////////////////////////////////////
            // line_id
            ////////////////////////////////////////////////////////////
            enum line_id_type
            {
               line_rts,
               line_dtr
            } line_id;

            ////////////////////////////////////////////////////////////
            // state
            ////////////////////////////////////////////////////////////
            bool state;

            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            command_set_control_line(line_id_type line_id_, bool state_):
               line_id(line_id_),
               state(state_)
            { }

            ////////////////////////////////////////////////////////////
            // execute
            ////////////////////////////////////////////////////////////
            virtual void execute(
               SerialPortBase *port,
               HANDLE port_handle)
            {
               int status;
               ioctl(port_handle,TIOCMGET,&status);
               switch(line_id)
               {
               case line_rts:
                  if(state)
                     status |= TIOCM_RTS;
                  else
                     status &= ~TIOCM_RTS;
                  break;
                  
               case line_dtr:
                  if(state)
                     status |= TIOCM_DTR;
                  else
                     status &= ~TIOCM_DTR;
                  break;
               }
               ioctl(port_handle,TIOCMSET,&status);
            }
         };


         ////////////////////////////////////////////////////////////
         // make_baud_rate
         ////////////////////////////////////////////////////////////
         speed_t make_baud_rate(uint4 baud_rate)
         {
            speed_t rtn;
            if(baud_rate <= 300)
               rtn = B300;
            else if(baud_rate <= 1200)
               rtn = B1200;
            else if(baud_rate <= 2400)
               rtn = B2400;
            else if(baud_rate <= 4800)
               rtn = B4800;
            else if(baud_rate <= 9600)
               rtn = B9600;
            else if(baud_rate <= 19200)
               rtn = B19200;
            else if(baud_rate <= 38400)
               rtn = B38400;
            else if(baud_rate <= 57600)
               rtn = B57600;
            else if(baud_rate <= 115200)
               rtn = B115200;
            else
               rtn = B230400;
            return rtn;
         } // make_baud_rate
      
      
         ////////////////////////////////////////////////////////////
         // class command_set_baud_rate
         ////////////////////////////////////////////////////////////
         class command_set_baud_rate: public SerialPortBaseHelpers::CustomCommand
         {
         public:
            ////////////////////////////////////////////////////////////
            // baud_rate
            ////////////////////////////////////////////////////////////
            uint4 baud_rate;

            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            command_set_baud_rate(uint4 baud_rate_):
               baud_rate(baud_rate_)
            { }

            ////////////////////////////////////////////////////////////
            // execute
            ////////////////////////////////////////////////////////////
            virtual void execute(
               SerialPortBase *port,
               HANDLE port_handle)
            {
               struct termios options;
               tcgetattr(port_handle,&options);
               cfsetispeed(&options,make_baud_rate(baud_rate));
               cfsetospeed(&options,make_baud_rate(baud_rate));
               tcsetattr(port_handle,TCSANOW,&options);
            }
         };


         ////////////////////////////////////////////////////////////
         // class command_write_reps
         ////////////////////////////////////////////////////////////
         class command_write_reps: public SerialPortBaseHelpers::CustomCommand
         {
         public:
            ////////////////////////////////////////////////////////////
            // buff
            ////////////////////////////////////////////////////////////
            StrBin buff;

            ////////////////////////////////////////////////////////////
            // repeat_count
            ////////////////////////////////////////////////////////////
            uint4 repeat_count;

            ////////////////////////////////////////////////////////////
            // delay_between_msec
            ////////////////////////////////////////////////////////////
            uint4 delay_between_msec;

            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            command_write_reps(
               void const *buff_,
               uint4 buff_len,
               uint4 repeat_count_,
               uint4 delay_between_msec_):
               buff(buff_,buff_len),
               repeat_count(repeat_count_),
               delay_between_msec(delay_between_msec_)
            { }

            ////////////////////////////////////////////////////////////
            // execute
            ////////////////////////////////////////////////////////////
            virtual void execute(
               SerialPortBase *port,
               HANDLE port_handle)
            {
               struct timespec sleep_time;
               uint4 i = 0;
               
               sleep_time.tv_sec = delay_between_msec / 1000;
               sleep_time.tv_nsec = (delay_between_msec - sleep_time.tv_sec) * 1000000;
               while(i < repeat_count)
               {
                  ssize_t rcd = ::write(
                     port_handle,
                     buff.getContents(),
                     buff.length());
                  if(rcd == buff.length())
                  {
                     port->on_low_level_write(
                        buff.getContents(),
                        buff.length()); 
                  }
                  else if(rcd == -1 && errno == EINTR)
                     continue;
                  else
                     throw OsException("write repeat failure");
                  if(i + 1 < repeat_count && delay_between_msec > 0)
                     ::nanosleep(&sleep_time,0);
                  ++i;
               }
            } // execute
         };
      };

      
      ////////////////////////////////////////////////////////////
      // class SerialPortBase definitions
      ////////////////////////////////////////////////////////////
      SerialPortBase::SerialPortBase():
         port_handle(INVALID_HANDLE_VALUE),
         tx_queue(2048, true),
         rx_queue(2048, true)
      {  } // constructor

      
      SerialPortBase::~SerialPortBase()
      { close(); }

      
      void SerialPortBase::open(
         StrAsc const &port_name_,
         uint4 baud_rate_,
         bool use_overlapped)
      {
         if(is_open())
            close();
         port_name = port_name_;
         baud_rate = baud_rate_;
         should_close = false;
         start();
      } // open


      void SerialPortBase::open(
         HANDLE port_handle_,
         bool use_overlapped)
      {
         if(is_open())
            close();
         port_handle = port_handle_;
         should_close = false;
         start();
      } // open

      
      bool SerialPortBase::is_open() const
      {
         bool rtn = false;
         if(port_handle != INVALID_HANDLE_VALUE)
            rtn = true;
         return rtn;
      } // is_open

      
      void SerialPortBase::close()
      {
         should_close = true;
         wait_for_end();
         tx_queue.pop(tx_queue.size());
         rx_queue.pop(rx_queue.size());
         commands_protector.lock();
         commands.clear();
         commands_protector.unlock();
      } // close

      
      void SerialPortBase::write(
         void const *buff,
         uint4 buff_len)
      {
         tx_queue.push(buff, buff_len);
         signal_urgent();
      } // write


      void SerialPortBase::write_reps(
         void const *buff,
         uint4 buff_len,
         uint4 repeat_count,
         uint4 delay_between_msec)
      {
         if(buff_len > 0)
         {
            add_custom_command(
               new command_write_reps(
                  buff,
                  buff_len,
                  repeat_count,
                  delay_between_msec));
         }
      } // write_reps

      
      uint4 SerialPortBase::read(
         void *buff,
         uint4 buff_len)
      {
         uint4 rtn = rx_queue.pop(buff,buff_len);
         return rtn;
      } // read

      
      void SerialPortBase::set_hardware_flow_control(bool use_flow_control)
      {
         add_custom_command(
            new command_set_flow_control(
               use_flow_control));
      } // set_hardware_flow_control

      
      void SerialPortBase::set_rts_state(bool rts_state)
      {
         add_custom_command(
            new command_set_control_line(
               command_set_control_line::line_rts,
               rts_state)); 
      } // set_rts_state

      
      void SerialPortBase::set_dtr_state(bool dtr_state)
      {
         add_custom_command(
            new command_set_control_line(
               command_set_control_line::line_dtr,
               dtr_state));
      } // set_dtr_state


      void SerialPortBase::set_baud_rate(uint4 baud_rate_)
      {
         if(baud_rate != baud_rate_)
         {
            baud_rate = baud_rate_;
            add_custom_command(
               new command_set_baud_rate(baud_rate));
         }
      } // set_baud_rate

      
      void SerialPortBase::add_custom_command(
         SharedPtr<SerialPortBaseHelpers::CustomCommand> command)
      {
         commands_protector.lock();
         commands.push_back(command);
         commands_protector.unlock();
         signal_urgent();
      } // add_custom_command


      void SerialPortBase::list_ports(port_names_type &port_names)
      {
         // the following code searches for serial ports under linux.  We will search the /dev
         // directory for files that match patterns for ttyS* and ttyUSB*.  Once we have that list,
         // we will make system calls to verify each one as a valid device name.
         Csi::FileSystemObject device_dir("/dev");
         if(device_dir.get_is_valid())
         {
            // search for all files matching ttyS*
            typedef Csi::FileSystemObject::children_type children_type;
            children_type children;
            device_dir.get_children(children,"ttyS*");
            for(children_type::iterator ci = children.begin();
                ci != children.end();
                ++ci)
               port_names.push_back(ci->get_name());

            // search for all files matching ttyUSB*
            children.clear();
            device_dir.get_children(children,"ttyUSB*");
            for(children_type::iterator ci = children.begin();
                ci != children.end();
                ++ci)
               port_names.push_back(ci->get_name());

            // there is no way of verifying the port names short of opening them.  Unfortunately,
            // there is no way of distinguishing between a port that is already open and a port that
            // is not supported.  We will simply send all of the possible names.
            std::sort(port_names.begin(), port_names.end());
         }
      } // list_ports


      void SerialPortBase::list_ports_friendly(friendly_names_type &port_names)
      {
         // we will first get the list of names.  This will ensure that, at the very least, we get
         // the same list of names as we would have otherwise obtained. 
         port_names_type simple_list;
         list_ports(simple_list);
         port_names.clear();
         for(port_names_type::iterator pi = simple_list.begin(); pi != simple_list.end(); ++pi)
            port_names.push_back(friendly_name_type(*pi, *pi));
      } // list_ports_friendly 
      

      void SerialPortBase::receive(SharedPtr<Event> &ev)
      {
         if(ev->getType() == event_error::event_id)
         {
            event_error *event = static_cast<event_error *>(ev.get_rep());
            on_error(event->message.c_str());
         }
         else if(ev->getType() == event_read::event_id)
         {
            on_read();
         }
      } // receive
            


      void SerialPortBase::on_low_level_read(
         void const *buff,
         uint4 buff_len)
      {
         rx_queue.push(buff,buff_len);
         event_read::cpost(this);
      } // on_low_level_read


      void SerialPortBase::execute()
      {
         char rx_buff[1024];
         char tx_buff[1024];
         try
         {
            // if the port handle is not opened already, we will need to open it
            if(port_handle == INVALID_HANDLE_VALUE)
            {
               // first, we will make sure that the path to the device is properly formed
               std::ostringstream port_path;
               if(port_name.first() != '/')
                  port_path << "/dev/";
               port_path << port_name;

               // open the handle to the device
               port_handle = ::open(
                  port_path.str().c_str(),
                  O_RDWR | O_NOCTTY | O_NDELAY);
               if(port_handle == INVALID_HANDLE_VALUE)
                  throw OsException("serial port open failed");

               // we now need to set the serial attributes for the port
               struct termios port_options;

               tcgetattr(port_handle,&port_options);
               cfsetispeed(&port_options,make_baud_rate(baud_rate));
               cfsetospeed(&port_options,make_baud_rate(baud_rate));
               port_options.c_cflag &= ~PARENB; // no parity (next two as well)
               port_options.c_cflag &= ~CSTOPB;
               port_options.c_cflag &= ~CSIZE;
               port_options.c_cflag |= CS8; // eight data bits
               cfmakeraw(&port_options);
               tcsetattr(port_handle,TCSANOW,&port_options);
            }

            // in order to detect changes, we need to get the initial state of the CD line
            bool carrier_detect_state = false;
            int modem_status;

            on_open();
            ioctl(port_handle,TIOCMGET,&modem_status);
            carrier_detect_state  = (modem_status & TIOCM_CD) ? true : false;
            
            // we will now start the work loop for this thread.  In this loop, we will do the
            // following operations in this order:
            //
            //  1 - check the state of CD
            //  2 - check to execute any custom commands
            //  3 - write any data that is pending in the tx queue
            //  4 - wait for data to become available to be read and read it
            //
            // We will use the select() call to wait for the port to be flagged as readable or
            // writable.  This call will be timed so that the thread can execute.  We will also use
            // a signal to interrupt the select() call and force the thread to cycle.
            bool can_send = true;
            while(!should_close)
            {
               // check the state of the carrier detect
               ioctl(port_handle,TIOCMGET,&modem_status);
               bool new_cd_state = (modem_status & TIOCM_CD) ? true : false;
               if(new_cd_state != carrier_detect_state)
               {
                  carrier_detect_state = new_cd_state;
                  on_carrier_detect_change(carrier_detect_state);
               }
               
               // check to see if there are custom commands to execute
               commands_protector.lock();
               while(!commands.empty() && !should_close)
               {
                  command_handle command(commands.front());
                  commands.pop_front();
                  commands_protector.unlock();
                  
                  command->execute(this,port_handle);
                  commands_protector.lock();
               }
               commands_protector.unlock();

               // now check to see if there is anything that must be written
               fill_write_buffer(tx_queue);
               while(!tx_queue.empty() && can_send && !should_close)
               {
                  uint4 bytes_avail = tx_queue.pop(tx_buff, sizeof(tx_buff));
                  ssize_t bytes_written = ::write(
                     port_handle,
                     tx_buff,
                     bytes_avail);
                  if(bytes_written >= 0)
                  {
                     on_low_level_write(tx_buff,bytes_written);
                     tx_queue.pop(bytes_written);
                     if(bytes_written < bytes_avail)
                        can_send = false;
                  }
                  else if(errno == EINTR)
                     continue;
                  else if(errno == EAGAIN)
                     can_send = false;
                  else
                     throw OsException("write failed");
               }

               // the final thing to do is to wait for data to become available.  We will use select
               // to do this
               fd_set read_set;
               fd_set write_set;
               fd_set error_set;
               struct timeval timeout;
               int select_rcd;
               
               FD_ZERO(&read_set);
               FD_ZERO(&write_set);
               FD_SET(port_handle,&error_set);
               FD_SET(port_handle,&read_set);
               if(!can_send)
                  FD_SET(port_handle,&write_set);
               FD_SET(port_handle,&error_set);
               timeout.tv_sec = 0;
               timeout.tv_usec = 100000; // 100 msec
               select_rcd = ::select(
                  port_handle + 1,
                  &read_set,
                  &write_set,
                  &error_set,
                  &timeout);
               if(select_rcd > 0)
               {
                  if(FD_ISSET(port_handle,&read_set))
                  {
                     ssize_t bytes_read;
                     ssize_t total_bytes_read = 0;
                     int bytes_remaining = 0;
                     do
                     {
                        bytes_read = ::read(
                           port_handle,
                           rx_buff,
                           sizeof(rx_buff));
                        if(bytes_read >= 0)
                        {
                           total_bytes_read += bytes_read;
                           on_low_level_read(rx_buff,bytes_read);
                        }
                        else if(errno != EINTR)
                           throw OsException("serial read failure");
                        ioctl(
                           port_handle,
                           FIONREAD,
                           &bytes_remaining);
                     }
                     while(bytes_remaining != 0);
                     if(total_bytes_read > 0)
                        event_read::cpost(this);
                  }
                  if(FD_ISSET(port_handle,&write_set))
                     can_send = true;
               } 
               else if(select_rcd == -1 && errno != EINTR && errno != EAGAIN)
                  throw OsException("select error");
            }
         }
         catch(std::exception &e)
         { event_error::cpost(this,e.what()); }

         // make sure that the port is shut down
         if(port_handle != INVALID_HANDLE_VALUE)
         {
            ::close(port_handle);
            port_handle = INVALID_HANDLE_VALUE;
         }

         // make sure that the transmit buffer is empty
         tx_queue.pop(tx_queue.size());
      } // execute
   };
};



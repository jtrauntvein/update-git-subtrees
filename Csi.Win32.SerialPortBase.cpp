/* Csi.Win32.SerialPortBase.cpp

   Copyright (C) 2005, 2017 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 21 December 2005
   Last Change: Tuesday 19 December 2017
   Last Commit: $Date: 2017-12-19 16:29:39 -0600 (Tue, 19 Dec 2017) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Win32.SerialPortBase.h"
#include "Csi.OsException.h"
#include "Csi.Utils.h"
#include "Csi.StrAscStream.h"
#include "coratools.strings.h"
#include <sstream>
#include <algorithm>
#include <setupapi.h>


namespace Csi
{
   namespace Win32
   {
      using namespace SerialPortBaseStrings;

      
      namespace
      {
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
         Event::registerType("Csi::Win32::SerialPortBase::event_error");


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
         Event::registerType("Csi::Win32::SerialPortBase::event_read");


         ////////////////////////////////////////////////////////////
         // init_dcb
         ////////////////////////////////////////////////////////////
         void init_dcb(DCB *dcb)
         {
            memset(dcb, 0, sizeof(dcb));
            dcb->DCBlength = sizeof(dcb);
         } // init_dcb


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
               HANDLE port_handle,
               bool use_overlapped)
            {
               DCB dcb;
               init_dcb(&dcb);
               if(!GetCommState(port_handle,&dcb))
                  throw OsException(my_strings[strid_get_comm_state_failed].c_str());
               if(flow_control_enabled)
               {
                  dcb.fOutxCtsFlow = TRUE;
                  dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
               }
               else
               {
                  dcb.fOutxCtsFlow = FALSE;
                  dcb.fRtsControl = RTS_CONTROL_ENABLE;
               }
               if(!SetCommState(port_handle,&dcb))
                  throw OsException(my_strings[strid_set_comm_state_failed].c_str());
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
               HANDLE port_handle,
               bool use_overlapped)
            {
               DCB dcb;
               init_dcb(&dcb);
               if(!GetCommState(port_handle,&dcb))
                  throw OsException(my_strings[strid_get_comm_state_failed].c_str());
               switch(line_id)
               {
               case line_dtr:
                  if(state)
                     dcb.fDtrControl = DTR_CONTROL_ENABLE;
                  else
                     dcb.fDtrControl = DTR_CONTROL_DISABLE;
                  break;
                  
               case line_rts:
                  if(state)
                     dcb.fRtsControl = RTS_CONTROL_ENABLE;
                  else
                     dcb.fRtsControl = RTS_CONTROL_DISABLE;
                  break;
               }
               if(!SetCommState(port_handle,&dcb))
                  throw OsException(my_strings[strid_set_comm_state_failed].c_str());
            }
         };


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
               HANDLE port_handle,
               bool use_overlapped)
            {
               DCB dcb;
               init_dcb(&dcb);
               if(!GetCommState(port_handle,&dcb))
                  throw OsException(my_strings[strid_get_comm_state_failed].c_str()); 
               dcb.BaudRate = baud_rate;
               if(!SetCommState(port_handle,&dcb))
                  throw OsException(my_strings[strid_set_comm_state_failed].c_str());
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
               HANDLE port_handle,
               bool use_overlapped)
            {
               if(use_overlapped)
               {
                  Condition timer, io_event;
                  OVERLAPPED io_control;

                  memset(&io_control, 0, sizeof(io_control));
                  io_control.hEvent = io_event.get_handle();
                  for(uint4 reps = 0; reps < repeat_count; ++reps)
                  {
                     uint4 bytes_written;
                     DWORD rcd = WriteFile(
                        port_handle, buff.getContents(), (DWORD)buff.length(), &bytes_written, &io_control);
                     uint4 last_error = GetLastError();
                     uint4 write_time_base = counter(0);
                     while(!rcd &&
                           counter(write_time_base) < 10000 &&
                           (last_error == ERROR_IO_PENDING ||
                            last_error == ERROR_IO_INCOMPLETE))
                     {
                        rcd = GetOverlappedResult(
                           port_handle, &io_control, &bytes_written, false);
                        last_error = GetLastError();
                        if(!rcd && last_error != ERROR_IO_INCOMPLETE)
                           throw OsException(my_strings[strid_overlapped_write_failed].c_str());
                        else if(!rcd)
                           Sleep(1);
                     }
                     if(!rcd || bytes_written < buff.length())
                        throw MsgExcept(my_strings[strid_write_failed].c_str());
                     if(rcd)
                     {
                        SetLastError(0);
                        port->on_low_level_write(
                           buff.getContents(), (uint4)buff.length());
                     }
                     timer.wait(delay_between_msec);
                  }
               }
               else
               {
                  Condition timer;
                  OVERLAPPED io_control;
                  
                  memset(&io_control, 0, sizeof(io_control));
                  for(uint4 reps = 0; reps < repeat_count; ++reps)
                  {
                     uint4 bytes_written;
                     DWORD rcd = WriteFile(
                        port_handle, buff.getContents(), (DWORD)buff.length(), &bytes_written, &io_control);
                     uint4 write_time_base = counter(0);
                     uint4 last_error = GetLastError();

                     if(!rcd || bytes_written < buff.length())
                        throw MsgExcept(my_strings[strid_write_failed].c_str());
                     if(rcd)
                     {
                        SetLastError(0);
                        port->on_low_level_write(
                           buff.getContents(),
                           bytes_written);
                     }
                     timer.wait(delay_between_msec);
                  }
               }
            } // execute
         };


         ////////////////////////////////////////////////////////////
         // class command_test_port
         ////////////////////////////////////////////////////////////
         class command_test_port: public SerialPortBaseHelpers::CustomCommand
         {
         public:
            ////////////////////////////////////////////////////////////
            // execute
            ////////////////////////////////////////////////////////////
            virtual void execute(
               SerialPortBase *port,
               HANDLE port_handle,
               bool use_overlapped)
            {
               DCB dcb;
               init_dcb(&dcb);
               if(!GetCommState(port_handle, &dcb))
                  throw OsException(my_strings[strid_get_modem_status_failed].c_str());
               if(!SetCommState(port_handle, &dcb))
                  throw OsException(my_strings[strid_get_modem_status_failed].c_str());
            }
         };


         ////////////////////////////////////////////////////////////
         // predicate port_name_less
         ////////////////////////////////////////////////////////////
         struct port_name_less
         {
            typedef std::pair<StrAsc const, bool> port_name_type;
            bool operator ()(port_name_type const &p1, port_name_type const &p2) const
            { return p1.first < p2.first; }
         };


         DEFINE_GUID(GUID_CLASS_COMPORT, 0x4d36e978, 0xe325, 0x11ce, 0xbf, 0xc1, 0x08, 0x00, 0x2b, 0xe1, 0x03, 0x18);
      };

      
      ////////////////////////////////////////////////////////////
      // class SerialPortBase definitions
      ////////////////////////////////////////////////////////////
      SerialPortBase::SerialPortBase():
         port_handle(INVALID_HANDLE_VALUE),
         attention(0,false,false),
         tx_queue(2048, true),
         rx_queue(2048, true)
      {  } // constructor

      
      SerialPortBase::~SerialPortBase()
      { close(); }


      void SerialPortBase::list_ports(port_names_type &port_names)
      {
         // there are issues of virtual com ports not showing up after
         // installation and also of USB serial ports not appearing without a
         // reboot.  We keep on getting getting the "Hyperterm does it"
         // refrain.  We will enumerate the ports given in the registry to see
         // if there are any names we don't yet know.
         wchar_t const *key_name = L"HARDWARE\\DEVICEMAP\\SERIALCOMM";
         StrUni port_name;
         StrUni port_value_name;
         HKEY reg_key;
         long rcd = RegOpenKeyExW(
            HKEY_LOCAL_MACHINE,
            key_name,
            0,               // reserved value
            KEY_READ,
            &reg_key);
         typedef std::pair<StrAsc const, bool> port_pair_type;
         typedef std::list<port_pair_type> port_pairs_type;
         port_pairs_type pairs;
         bool bluetooth_port;
         
         if(rcd == ERROR_SUCCESS)
         {
            wchar_t val[MAX_PATH];
            wchar_t val_name[MAX_PATH];
            uint4 val_len = sizeof(val);
            uint4 val_name_len = sizeof(val_name);
            uint4 val_index = 0;
            
            while(RegEnumValueW(
                     reg_key,
                     val_index,
                     val_name,
                     &val_name_len,
                     0,      // reserved
                     0,      // type, not required
                     reinterpret_cast<byte *>(val),
                     &val_len) == ERROR_SUCCESS)
            {
               // add the port name
               if(val_len > 0)
                  port_name.setContents(val, val_len);
               if(val_name_len > 0)
               {
                  port_value_name.setContents(val_name, val_name_len);
                  if(port_value_name.find(L"\\Device\\BthModem") == 0)
                     bluetooth_port = true;
                  else
                     bluetooth_port = false;
               }
               pairs.push_back(port_pair_type(port_name.to_utf8(), bluetooth_port));
               
               // get ready for the next enum
               val_len = sizeof(val);
               val_name_len = sizeof(val_name);
               ++val_index;
            } 
            RegCloseKey(reg_key);
         }
         pairs.sort(port_name_less());
         
         // we will now take all of these names and query the operating system
         // configuration details for them.  If the details look like a match,
         // we will include the name in the list
         StrBin config_buffer;
         config_buffer.fill(0,sizeof(COMMCONFIG));
         while(!pairs.empty())
         {
            port_pair_type pair(pairs.front());
            pairs.pop_front();
            if(!pair.second)
            {
               // we need to get the default configuration for the port.  This may
               // require some fudging on the buffer size.  That is why two calls
               // are being made.
               uint4 config_size = (uint4)config_buffer.length();
               StrUni temp(pair.first);
               COMMCONFIG *config(reinterpret_cast<COMMCONFIG *>(config_buffer.getContents_writable()));
               config_buffer.fill(0, config_size);
               config->dwSize = sizeof(COMMCONFIG);
               rcd = GetDefaultCommConfigW(
                  temp.c_str(), config, &config_size);
               if(!rcd && config_buffer.length() < config_size)
               {
                  config_buffer.fill(0, config_size);
                  config = reinterpret_cast<COMMCONFIG *>(config_buffer.getContents_writable());
                  config->dwSize = sizeof(COMMCONFIG);
                  rcd = GetDefaultCommConfigW(
                     temp.c_str(), config, &config_size);
               }
               
               // if the call succeeded, we can go ahead and look at the
               // configuration structure.
               if(rcd)
               {
                  if(config->dwProviderSubType == PST_RS232)
                     port_names.push_back(pair.first);
               }
               else
               {
                  OsException error("GetDefaultCommConfig Failed");
                  trace("\"%s\"", error.what());
               }
            }
            else
               port_names.push_back(pair.first);
         }

         // windows 10 does not appear to support the GetDefaultCommConfig() function properly for
         // devices that use the usbser.sys driver.  In order to pick these up, we will use the
         // SetupDi... functions to augment the list of names that we have already resolved.
         GUID *serial_port_guid = const_cast<GUID *>(&GUID_CLASS_COMPORT);
         HDEVINFO device_info = SetupDiGetClassDevs(
            serial_port_guid, 0, 0, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
         if(device_info != INVALID_HANDLE_VALUE)
         {
            int rcd;
            bool more_interfaces = true;
            SP_DEVINFO_DATA ifc_data;

            memset(&ifc_data, 0, sizeof(ifc_data));
            ifc_data.cbSize = sizeof(ifc_data);
            for(uint4 index = 0; more_interfaces; ++index)
            {
               rcd = SetupDiEnumDeviceInfo(device_info, index, &ifc_data);
               if(rcd)
               {
                  HKEY registry = SetupDiOpenDevRegKey(device_info, &ifc_data, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);
                  if(registry)
                  {
                     // extract the port name from the registyr
                     wchar_t value[MAX_PATH];
                     uint4 value_len(sizeof(value));
                     RegQueryValueExW(registry, L"PortName", 0, 0, (LPBYTE)value, &value_len);
                     port_name.setContents(value, value_len);

                     // we now need to determine whether the name is already in the list.
                     StrAsc port_name_mb(port_name.to_utf8());
                     port_names_type::iterator pi(
                        std::find(port_names.begin(), port_names.end(), port_name_mb));
                     if(pi == port_names.end())
                        port_names.push_back(port_name_mb);
                     RegCloseKey(registry);
                  }
               }
               else
                  more_interfaces = false;
            }
            SetupDiDestroyDeviceInfoList(device_info);
         }
      } // list_ports


      namespace
      {
         ////////////////////////////////////////////////////////////
         // functor port_has_name
         ////////////////////////////////////////////////////////////
         struct port_has_name
         {
            StrAsc const &name;
            Csi::OStrAscStream temp;
            port_has_name(StrAsc const &name_):
               name(name_)
            { }
            bool operator ()(std::pair<StrAsc, StrAsc> &port) const
            { return port.second.length() == 0 && port.first == name; }
         }; 
      };


      void SerialPortBase::list_ports_friendly(friendly_names_type &port_names)
      {
         // we will first get the list of names.  This will ensure that, at the
         // very least, we get the same list of names as we would have
         // otherwise obtained.
         port_names_type simple_list;
         list_ports(simple_list);
         port_names.clear();
         for(port_names_type::iterator pi = simple_list.begin(); pi != simple_list.end(); ++pi)
            port_names.push_back(friendly_name_type(*pi, ""));

         // we will now need to enumerate the subkeys of the Enum registry
         // key. We will need to consider many levels of the registry key
         // structure in doing this so we will use a list of key handles as a
         // stack.
         HKEY enum_key ;
         wchar_t const enum_key_name[] = L"SYSTEM\\CurrentControlSet\\Enum";
         StrUni const com_port_guid("{4d36e978-e325-11ce-bfc1-08002be10318}");
         wchar_t const class_guid_name[] = L"ClassGUID";
         wchar_t const friendly_name_name[] = L"FriendlyName";
         wchar_t const device_parameters_name[] = L"Device Parameters";
         wchar_t const port_name_name[] = L"PortName";
         long rcd = ::RegOpenKeyExW(
            HKEY_LOCAL_MACHINE, enum_key_name, 0, KEY_READ, &enum_key);
         wchar_t value_buff[MAX_PATH];
         StrUni port_name, friendly_name;
         
         if(!port_names.empty() && rcd == ERROR_SUCCESS)
         {
            std::list<HKEY> key_stack;
            key_stack.push_back(enum_key);
            while(!key_stack.empty())
            {
               // we need to determine whether this key has a "ClassGUID" value
               HKEY current = key_stack.front();
               uint4 value_buff_len = sizeof(value_buff);
               key_stack.pop_front();
               rcd = ::RegQueryValueExW(
                  current,
                  class_guid_name,
                  0,
                  0,
                  reinterpret_cast<byte *>(value_buff),
                  &value_buff_len);
               if(rcd == ERROR_SUCCESS)
               {
                  // we will only consider devices that match the com port GUID
                  if(com_port_guid == value_buff)
                  {
                     // this key appears to identify a com port.  We will need
                     // to get the friendly name and try to get the 'PortName'
                     // from the 'Device Parameters' subkey.  Once we have
                     // those things, we can update the friendly name in our
                     // original list
                     value_buff_len = sizeof(value_buff);
                     rcd = ::RegQueryValueExW(
                        current,
                        friendly_name_name,
                        0,
                        0,
                        reinterpret_cast<byte *>(value_buff),
                        &value_buff_len);
                     if(rcd == ERROR_SUCCESS)
                     {
                        HKEY device_parameters_key;
                        rcd = ::RegOpenKeyExW(
                           current,
                           device_parameters_name,
                           0,
                           KEY_READ,
                           &device_parameters_key);
                        if(rcd == ERROR_SUCCESS)
                        {
                           friendly_name = value_buff;
                           value_buff_len = sizeof(value_buff);
                           rcd = ::RegQueryValueExW(
                              device_parameters_key,
                              port_name_name,
                              0,
                              0,
                              reinterpret_cast<byte *>(value_buff),
                              &value_buff_len);
                           if(rcd == ERROR_SUCCESS)
                           {
                              friendly_names_type::iterator fi;
                              port_name = value_buff;
                              fi = std::find_if(
                                 port_names.begin(),
                                 port_names.end(),
                                 port_has_name(port_name.to_utf8()));
                              if(fi != port_names.end())
                                 fi->second = friendly_name.to_utf8();
                           }
                           ::RegCloseKey(device_parameters_key);
                        }
                     }
                  }
               }
               else
               {
                  // since this key did not have what we expected, we will need
                  // to check its children
                  uint4 index = 0;

                  rcd = ERROR_SUCCESS;
                  while(rcd == ERROR_SUCCESS)
                  {
                     value_buff_len = sizeof(value_buff);
                     rcd = ::RegEnumKeyExW(
                        current, index, value_buff, &value_buff_len, 0, 0, 0, 0);
                     if(rcd == ERROR_SUCCESS)
                     {
                        HKEY child;
                        rcd = ::RegOpenKeyExW(
                           current, value_buff, 0, KEY_READ, &child);
                        if(rcd == ERROR_SUCCESS)
                           key_stack.push_back(child);
                     }
                     ++index;
                  }
               }
               ::RegCloseKey(current);
            }
         }

         // if there were any friendly names that weren't resolved, we will assign the port name as
         // the "friendly" name.
         for(friendly_names_type::iterator fi = port_names.begin();
             fi != port_names.end();
             ++fi)
         {
            if(fi->second.length() == 0)
               fi->second = fi->first;
         }
      } // list_ports_friendly

      
      void SerialPortBase::open(
         StrAsc const &port_name_,
         uint4 baud_rate_,
         bool use_overlapped_)
      {
         if(is_open())
            close();
         port_name = port_name_;
         baud_rate = baud_rate_;
         use_overlapped = use_overlapped_;
         should_close = false;
         start();
      } // open


      void SerialPortBase::open(
         HANDLE port_handle_, bool use_overlapped_)
      {
         if(is_open())
            close();
         port_handle = port_handle_;
         use_overlapped = use_overlapped_;
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
         attention.set();
         wait_for_end();
      } // close

      
      void SerialPortBase::write(
         void const *buff, uint4 buff_len)
      {
         tx_queue.push(buff, buff_len);
         attention.set();
      } // write


      void SerialPortBase::write_reps(
         void const *buff,
         uint4 buff_len,
         uint4 repeat_count,
         uint4 delay_between_msec)
      {
         if(repeat_count <= 1)
            write(buff, buff_len);
         else 
            add_custom_command(
               new command_write_reps(
                  buff,
                  buff_len,
                  repeat_count,
                  delay_between_msec));
      } // write_reps

      
      uint4 SerialPortBase::read(void *buff, uint4 buff_len)
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
         attention.set();
      } // add_custom_command


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
            


      void SerialPortBase::test_serial_port()
      {
         add_custom_command(new command_test_port);
      } // test_serial_port
      

      void SerialPortBase::on_low_level_read(
         void const *buff, uint4 buff_len)
      {
         rx_queue.push(buff,buff_len);
         event_read::cpost(this);
      } // on_low_level_read


      void SerialPortBase::execute()
      {
         if(use_overlapped)
            execute_overlapped();
         else
            execute_simple();
      } // execute

      
      void SerialPortBase::execute_simple()
      {
         char rx_buff[4096];
         char tx_buff[4096];
         bool thread_owns_handle = false;
         BOOL rcd;
         
         try
         {
            // open the port handle
            if(port_handle == INVALID_HANDLE_VALUE)
            {
               std::ostringstream port_path;
               if(port_name.first() != '\\')
                  port_path << "\\\\.\\";
               port_path << port_name;
               port_handle = ::CreateFileA(
                  port_path.str().c_str(),
                  GENERIC_READ | GENERIC_WRITE,
                  0,                  // exclusive access
                  0,                  // no security attributes
                  OPEN_EXISTING,
                  0,                  // no flags
                  0);                 // no template
               if(port_handle == INVALID_HANDLE_VALUE)
                  throw OsException(my_strings[strid_serial_open_failed].c_str());
               
               // initialise the port settings
               DCB dcb;
               init_dcb(&dcb);
               if(!GetCommState(port_handle,&dcb))
                  throw OsException(my_strings[strid_get_comm_state_failed].c_str());
               dcb.BaudRate = baud_rate;
               dcb.ByteSize = 8;
               dcb.Parity = NOPARITY;
               dcb.StopBits = ONESTOPBIT;
               dcb.fDtrControl = 1;
               dcb.fRtsControl = 1;
               dcb.fBinary = 1;
               dcb.fOutxCtsFlow = 0;
               dcb.fOutxDsrFlow = 0;
               dcb.fDsrSensitivity = 0;
               dcb.fTXContinueOnXoff = 0;
               dcb.fOutX = 0;
               dcb.fInX = 0;
               dcb.fErrorChar = 0;
               dcb.fNull = 0;
               dcb.fAbortOnError = 0;
               if(!SetCommState(port_handle, &dcb))
                  throw OsException(my_strings[strid_set_comm_state_failed].c_str());
               thread_owns_handle = true;
            }

            // set up the communications timeouts
            COMMTIMEOUTS timeouts;
            if(!GetCommTimeouts(port_handle, &timeouts))
               throw OsException(my_strings[strid_get_comm_timeouts_failed].c_str());
            timeouts.ReadIntervalTimeout = MAXDWORD;
            timeouts.ReadTotalTimeoutMultiplier = MAXDWORD;
            timeouts.ReadTotalTimeoutConstant = 10;
            timeouts.WriteTotalTimeoutMultiplier = 0;
            timeouts.WriteTotalTimeoutConstant = 10000;
            if(!SetCommTimeouts(port_handle, &timeouts))
               throw OsException(my_strings[strid_set_comm_timeouts_failed].c_str());
            on_open();

            // we need to set a base for the carrier detect signal.  This will be used to determine
            // when the signal "changes" while the loop executes
            bool carrier_detect_set = false;
            uint4 modem_status = 0;
            
            if(!GetCommModemStatus(port_handle, &modem_status))
               throw OsException(my_strings[strid_get_modem_status_failed].c_str());
            if(modem_status & MS_RLSD_ON)
               carrier_detect_set = true;
            
            // we are now ready to enter the main service loop for this thread.
            OVERLAPPED io_control;
            memset(&io_control, 0, sizeof(io_control));
            while(!should_close)
            {
               // we need to check to see if any comm errors have occurred
               uint4 comm_errors = 0;
               if(!ClearCommError(port_handle, &comm_errors, 0))
                  throw OsException(my_strings[strid_clear_comm_errors_failed].c_str());
               if(comm_errors != 0)
                  on_comm_errors(comm_errors);

               // we also need to determine if the carrier detect line has changed
               modem_status = 0;
               if(!GetCommModemStatus(port_handle, &modem_status))
                  throw OsException(my_strings[strid_get_modem_status_failed].c_str());
               if(carrier_detect_set && (modem_status & MS_RLSD_ON) == 0)
                  on_carrier_detect_change(false);
               else if(!carrier_detect_set && (modem_status & MS_RLSD_ON) != 0)
                  on_carrier_detect_change(true);
               
               // we will now execute any command that might be waiting
               command_handle command;
               commands_protector.lock();
               while(!commands.empty())
               {
                  command = commands.front();
                  commands.pop_front();
                  commands_protector.unlock();
                  command->execute(this, port_handle, false);
                  commands_protector.lock();
               }
               commands_protector.unlock();

               // now we will try to write anything that is pending in the write queue
               fill_write_buffer(tx_queue);
               while(!tx_queue.empty() && !should_close)
               {
                  uint4 bytes_avail = tx_queue.copy(tx_buff, sizeof(tx_buff));
                  uint4 bytes_written = 0;

                  rcd = WriteFile(
                     port_handle, tx_buff, bytes_avail, &bytes_written, &io_control);
                  if(!rcd || bytes_written == 0)
                     throw Csi::OsException(my_strings[strid_write_failed].c_str());
                  if(rcd)
                  {
                     SetLastError(0);
                     if(bytes_written)
                     {
                        tx_queue.pop(bytes_written);
                        on_low_level_write(tx_buff, bytes_written);
                     }
                     if(bytes_written < bytes_avail)
                        throw OsException(my_strings[strid_write_timed_out].c_str());
                  }
               }

               // we will now poll to see if there is any data available to be written
               uint4 bytes_read = 0;
               
               rcd = ReadFile(
                  port_handle, rx_buff, sizeof(rx_buff), &bytes_read, &io_control);
               if(rcd && bytes_read)
                  on_low_level_read(rx_buff, bytes_read);
               else if(!rcd)
                  throw OsException(my_strings[strid_read_failed].c_str());
            }
         }
         catch(std::exception &e)
         { event_error::cpost(this,e.what()); }

         // make sure that the port is shut down
         if(port_handle != INVALID_HANDLE_VALUE)
         {
            CancelIo(port_handle);
            ::CloseHandle(port_handle);
            port_handle = INVALID_HANDLE_VALUE;
         }

         // make sure that the transmit buffer is empty
         if(!tx_queue.empty())
            tx_queue.pop(tx_queue.size());
      } // execute_simple


      void SerialPortBase::execute_overlapped()
      {
         char rx_buff[4096], tx_buff[4096];
         OVERLAPPED event_control, io_control;
         Condition io_event, comms_event;
         HANDLE waiting_events[2];
         uint4 wait_rcd;
         uint4 event_mask;
         bool comm_event_ready = false;
         bool waiting_for_comm_event = false;
         bool thread_owns_handle = false;
         BOOL rcd;
         uint4 last_error;
         
         try
         {
            // initialise the local thread variables
            memset(&event_control, 0, sizeof(event_control));
            event_control.hEvent = comms_event.get_handle();
            memset(&io_control, 0, sizeof(io_control));
            io_control.hEvent = io_event.get_handle();
            waiting_events[0] = attention.get_handle();
            waiting_events[1] = comms_event.get_handle();

            // open the port handle if needed
            if(port_handle == INVALID_HANDLE_VALUE)
            {
               OStrAscStream port_path;
               if(port_name.first() != '\\')
                  port_path << "\\\\.\\";
               port_path << port_name;
               port_handle = ::CreateFileA(
                  port_path.str().c_str(),
                  GENERIC_READ | GENERIC_WRITE,
                  0,            // exclusive access
                  0,            // no security attribs
                  OPEN_EXISTING,
                  FILE_FLAG_OVERLAPPED,
                  0);           // no template
               if(port_handle == INVALID_HANDLE_VALUE)
                  throw OsException(my_strings[strid_serial_open_failed].c_str());

               // initialise the port settings
               DCB dcb;
               init_dcb(&dcb);
               GetCommState(port_handle,&dcb);
               dcb.BaudRate = baud_rate;
               dcb.ByteSize = 8;
               dcb.Parity = NOPARITY;
               dcb.StopBits = ONESTOPBIT;
               dcb.fDtrControl = 1;
               dcb.fRtsControl = 1;
               dcb.fBinary = 1;
               dcb.fOutxCtsFlow = 0;
               dcb.fOutxDsrFlow = 0;
               dcb.fDsrSensitivity = 0;
               dcb.fTXContinueOnXoff = 0;
               dcb.fOutX = 0;
               dcb.fInX = 0;
               dcb.fErrorChar = 0;
               dcb.fNull = 0;
               dcb.fRtsControl = 1;
               dcb.fAbortOnError = 0;
               if(!SetCommState(port_handle, &dcb))
                  throw OsException(my_strings[strid_set_comm_state_failed].c_str());
               thread_owns_handle = true;
            }

            // we also need to set up the timeouts for this port
            COMMTIMEOUTS timeouts;
            if(!GetCommTimeouts(port_handle, &timeouts))
               throw OsException(my_strings[strid_get_comm_timeouts_failed].c_str());
            timeouts.ReadIntervalTimeout = MAXDWORD;
            timeouts.ReadTotalTimeoutMultiplier = 0;
            timeouts.ReadTotalTimeoutConstant = 0;
            timeouts.WriteTotalTimeoutMultiplier = 1;
            timeouts.WriteTotalTimeoutConstant = 8000;
            if(!SetCommTimeouts(port_handle, &timeouts))
               throw OsException(my_strings[strid_set_comm_timeouts_failed].c_str());
            on_open();

            // we can now enter the main service loop
            while(!should_close)
            {
               // we will first execute any waiting commands
               command_handle command;
               commands_protector.lock();
               while(!commands.empty())
               {
                  command = commands.front();
                  commands.pop_front();
                  commands_protector.unlock();
                  command->execute(this, port_handle, true);
                  commands_protector.lock(); 
               }
               commands_protector.unlock();

               // now we will try to write anything that is pending in the
               // write queue
               fill_write_buffer(tx_queue);
               while(!tx_queue.empty() && !should_close)
               {
                  uint4 bytes_avail = 0;
                  uint4 bytes_written = 0;
                  uint4 write_time_base = counter(0);
                  
                  bytes_avail = tx_queue.copy(tx_buff, sizeof(tx_buff));
                  rcd = WriteFile(
                     port_handle, tx_buff, bytes_avail, &bytes_written, &io_control);
                  last_error = GetLastError();
                  while(!should_close && !rcd &&
                        counter(write_time_base) < 10000 &&
                        (last_error == ERROR_IO_PENDING ||
                         last_error == ERROR_IO_INCOMPLETE))
                  {
                     rcd = GetOverlappedResult(
                        port_handle, &io_control, &bytes_written, false);
                     last_error = GetLastError();
                     if(!rcd && last_error != ERROR_IO_INCOMPLETE)
                        throw OsException(my_strings[strid_overlapped_write_failed].c_str());
                     else if(!rcd)
                        Sleep(1);
                  }
                  if(!rcd && last_error == ERROR_IO_INCOMPLETE)
                     throw MsgExcept(my_strings[strid_write_failed].c_str());
                  if(rcd)
                  {
                     SetLastError(0);
                     if(bytes_written)
                     {
                        tx_queue.pop(bytes_written);
                        on_low_level_write(tx_buff, bytes_written);
                     }
                     if(bytes_written < bytes_avail)
                        throw MsgExcept(my_strings[strid_write_timed_out].c_str());
                  }
               }

               // we now need to wait for events to come in on the serial port
               if(!waiting_for_comm_event)
               {
                  uint4 comm_errors(0);
                  COMSTAT comm_status;

                  memset(&comm_status, 0, sizeof(comm_status));
                  comm_event_ready = false;
                  rcd = ClearCommError(port_handle, &comm_errors, &comm_status);
                  if(rcd == 0)
                     throw OsException("ClearCommError failed");
                  if(comm_errors != 0)
                     on_comm_errors(comm_errors);
                  if(comm_status.cbInQue > 0)
                  {
                     comm_event_ready = true;
                     event_mask = EV_RXCHAR;
                  }
                  else
                  {
                     if(!SetCommMask(port_handle, EV_RXCHAR | EV_ERR | EV_RLSD))
                        throw OsException(my_strings[strid_set_comm_mask_failed].c_str());
                     rcd = WaitCommEvent(port_handle, &event_mask, &event_control);
                     last_error = GetLastError();
                     if(rcd && event_mask != 0)
                        comm_event_ready = true;
                     else if(last_error != ERROR_IO_PENDING)
                        throw OsException(my_strings[strid_wait_comm_event_failed].c_str());
                     else
                        waiting_for_comm_event = true;
                  }
               }

               // if there is no event already pending, we need to halt and wait for the event to be
               // signaled
               if(!comm_event_ready)
                  wait_rcd = WaitForMultipleObjects(2, waiting_events, FALSE, INFINITE);
               if(comm_event_ready ||
                  wait_rcd == WAIT_OBJECT_0 ||
                  wait_rcd == WAIT_OBJECT_0 + 1)
               {
                  // if a comm event was signalled, we need to determine what
                  // happened
                  if(comm_event_ready || wait_rcd == WAIT_OBJECT_0 + 1)
                  {
                     // if characters were received, we will process as many as
                     // we can
                     waiting_for_comm_event = false;
                     if((event_mask & EV_RXCHAR) != 0)
                     {
                        uint4 bytes_read = 0;
                        bool done_reading = false;
                        while(!done_reading)
                        {
                           rcd = ReadFile(
                              port_handle, rx_buff, sizeof(rx_buff), &bytes_read, &io_control);
                           last_error = GetLastError();
                           if(!rcd && last_error == ERROR_IO_PENDING)
                           {
                              rcd = GetOverlappedResult(
                                 port_handle, &io_control, &bytes_read, true);
                              last_error = GetLastError();
                              if(!rcd)
                                 throw OsException(my_strings[strid_overlapped_read_failed].c_str());
                           }
                           else if(!rcd)
                              throw MsgExcept(my_strings[strid_read_failed].c_str());
                           if(rcd && bytes_read)
                              on_low_level_read(rx_buff, bytes_read);
                           else
                              done_reading = true;
                        }
                     }
                     if((event_mask & EV_ERR) != 0)
                     {
                        uint4 comm_errors = 0;
                        ClearCommError(port_handle, &comm_errors, 0);
                        on_comm_errors(comm_errors);
                     }
                     if((event_mask & EV_RLSD) != 0)
                     {
                        uint4 modem_status = 0;
                        if(GetCommModemStatus(port_handle, &modem_status))
                        {
                           if((modem_status  & MS_RLSD_ON) != 0)
                              on_carrier_detect_change(true);
                           else
                              on_carrier_detect_change(false);
                        }
                     }
                  }
               }
               else if(!should_close)
                  throw OsException(my_strings[strid_wait_failed].c_str());
            }
         }
         catch(std::exception &e)
         {
            event_error::cpost(this, e.what());
         }

         // make sure that the port handle was shut down
         if(port_handle != INVALID_HANDLE_VALUE)
         {
            CancelIo(port_handle);
            if(waiting_for_comm_event)
               SetCommMask(port_handle, 0);
            ::CloseHandle(port_handle);
            port_handle = INVALID_HANDLE_VALUE;
         }

         // make sure that the trasmit buffer is empty
         if(!tx_queue.empty())
            tx_queue.pop(tx_queue.size());
      } // execute_overlapped
   };
};



/* Csi.OsLoader.LargeSRecordLoader.cpp

   Copyright (C) 2008, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 02 June 2008
   Last Change: Friday 29 April 2016
   Last Commit: $Date: 2016-05-03 16:18:03 -0600 (Tue, 03 May 2016) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.OsLoader.LargeSRecordLoader.h"
#include "Csi.OsLoader.SRecordUtils.h"
#include "Csi.OsException.h"
#include "Csi.Utils.h"
#include "Csi.StrAscStream.h"
#include "Csi.DevConfig.LibraryManager.h"
#include "Csi.DevConfig.Defs.h"
#include "Csi.DevConfig.Message.h"
#include "coratools.strings.h"
#include "boost/format.hpp"
#include <iomanip>


namespace Csi
{
   namespace OsLoader
   {
      using namespace LargeSRecordLoaderStrings;

      
      namespace
      {
         ////////////////////////////////////////////////////////////
         // default_device_types
         ////////////////////////////////////////////////////////////
         DeviceTypeMap default_device_types[] =
         {
            { 0x1080, "RF400", 0 },
            { 0x1080, "RF430", 0 },
            { 0x1080, "MD485", 0 },
            { 0x1080, "SC105", 0 },
            { 0x1080, "CD295", 0 },
            { 0x1080, "CR200", 0xFFDA },
            { 0x1080, "CS450", 0xFFDA },
            { 0x1080, "CS650", 0xFFDA },
            { 0x1080, "PS200", 0 },
            { 0x1080, "CH200", 0 },
            { 0x1080, "RF450", 0 },
            { 0x1080, "COM220", 0 },
            { 12544, "CR20X", 121850 },
            { 0x5C00, "CWS900", 0x1FFF0 },
            { 0x5C00, "CWS650", 0x1FFF0 },
            { 0x5C00, "CWS220", 0x1FFF0 },
            { 0x1080, "SIO1", 0 },
            { 0, "", 0 }
         };


         ////////////////////////////////////////////////////////////
         // predicate device_type_known
         ////////////////////////////////////////////////////////////
         struct device_type_known
         {
            uint4 address;
            StrAsc const &type;
            device_type_known(uint4 address_, StrAsc const &type_):
               address(address_),
               type(type_)
            { }

            bool operator ()(DeviceTypeMap const &map)
            { return map.address == address && map.device_type == type; }
         };


         ////////////////////////////////////////////////////////////
         // predicate device_map_has_type
         ////////////////////////////////////////////////////////////
         struct device_map_has_type
         {
            StrAsc const &type;
            device_map_has_type(StrAsc const &type_):
               type(type_)
            { }

            bool operator()(DeviceTypeMap const &map)
            { return map.device_type == type; }
         };
      };

      
      ////////////////////////////////////////////////////////////
      // class LargeSRecordLoader definitions
      ////////////////////////////////////////////////////////////
      LargeSRecordLoader::LargeSRecordLoader(
         timer_handle &timer_,
         StrAsc const &device_type_,
         Csi::DevConfig::LibraryManager *manager):
         file_size_bytes(0),
         bytes_sent(0),
         timer(timer_),
         state(state_standby),
         send_devconfig_reset(false),
         needs_flash_reset(true),
         device_type(device_type_),
         confirm_device_type(device_type_.length() == 0),
         retry_count(0)
      {
         // we need to initialise the device types from the static table
         for(int i = 0; default_device_types[i].address != 0; ++i)
            device_types.push_back(default_device_types[i]);

         // we need to supplement this with any device type information given in the device
         // descriptions 
         if(manager != 0)
         {
            for(Csi::DevConfig::LibraryManager::iterator li = manager->begin();
                li != manager->end();
                ++li)
            {
               Csi::DevConfig::LibraryManager::value_type const &desc(*li);
               if(desc->get_has_srecord_config() && desc->get_srecord_should_confirm())
               {
                  device_types_type::iterator dti = std::find_if(
                     device_types.begin(),
                     device_types.end(),
                     device_map_has_type(desc->get_srecord_model_no()));
                  if(dti == device_types.end())
                  {
                     DeviceTypeMap map;
                     map.address = desc->get_srecord_model_address();
                     map.device_type = desc->get_srecord_model_no();
                     map.sig_address = desc->get_srecord_os_sig_address();
                     device_types.push_back(map);
                  }
                  else
                  {
                     DeviceTypeMap &map(*dti);
                     map.address = desc->get_srecord_model_address();
                     map.sig_address = desc->get_srecord_os_sig_address();
                  }
               }
            }
         }
      } // constructor

      
      LargeSRecordLoader::~LargeSRecordLoader()
      {
         if(get_device_type_id)
            timer->disarm(get_device_type_id);
         if(get_ack_id)
            timer->disarm(get_ack_id);
         timer.clear();
      } // destructor


      void LargeSRecordLoader::open_and_validate(
         char const *os_file_name)
      {
         // open the file
#pragma warning(disable: 4996)
         FILE *in = Csi::open_file(os_file_name,"rb");
#pragma warning(default: 4996)
         
         if(state != state_standby)
            throw MsgExcept(my_strings[strid_invalid_open_state].c_str());
         if(in == 0)
            throw OsException(my_strings[strid_unable_to_open_file].c_str());

         // we will read the file character at a time (the run time library should do buffering to
         // make it efficient).  These characters will be deposited into a line buffer.  When an end
         // of line character is encountered ("\n"), the line will be validated and placed into the
         // records container.
         StrAsc line;
         int next_ch;
         uint4 segment_base_address = 0;
         
         file_sig = 0xAAAA;
         file_size_bytes = 0;
         if(confirm_device_type)
            device_type.cut(0);
         device_sig_address = 0;
         records.clear();
         while((next_ch = fgetc(in)) != EOF)
         {
            // the signature will be maintained on the raw content of the file
            char ch = static_cast<char>(next_ch);
            file_sig = calcSigFor(&ch, 1, file_sig);
            switch(ch)
            {
            case '\n':
               if(is_valid_srecord(line.c_str(), (uint4)line.length()))
               {
                  records.push_back(line);
                  file_size_bytes += (uint4)line.length() + 2;
                  if(device_type.length() == 0)
                  {
                     uint2 record_type = hex_to_byte(line.c_str() + 7);
                     
                     switch(record_type)
                     {
                     case 0x00: // data record
                        validate_device_type(
                           segment_base_address + hex_to_uint2(line.c_str() + 3),
                           line.c_str() + 9,
                           hex_to_byte(line.c_str() + 1));
                        break;
                        
                     case 0x02: // extended segment address
                        segment_base_address = hex_to_uint2(line.c_str() + 9);
                        segment_base_address <<= 4;
                        break;
                        
                     case 0x04: // extended linear address
                        segment_base_address = hex_to_uint2(line.c_str() + 9);
                        segment_base_address <<= 16;
                        break;
                     }
                  }
                  line.cut(0);
               }
               else
               {
                  Csi::OStrAscStream msg;
                  msg << my_strings[strid_invalid_srecord] << (records.size() + 1);
                  throw MsgExcept(msg.str().c_str());
               }
               break;
               
            case '\r':
               break;           // ignore
               
            default:
               if(ch == ':' || isxdigit(ch))
                  line += ch;
               break;
            }
         }
         fclose(in);

         // the records buffer must not be empty.  It must also be properly terminated
         if(records.empty())
            throw MsgExcept(my_strings[strid_no_valid_srecords].c_str());
         if(records.back() != ":00000001FF")
            throw MsgExcept(my_strings[strid_invalid_termination].c_str());
         if(device_type.length() == 0)
            throw MsgExcept(my_strings[strid_no_device_type].c_str());
         
         // we need to format a line that contains the file signature and insert it before the last
         // record.
         if(device_sig_address)
         {
            // we need to write the extended segment address as well as the srecord that specifies
            // the signature.  
            Csi::OStrAscStream sig_line;
            records_type::iterator insert_pos = records.end();
            uint4 sig_segment = (device_sig_address & 0xFFFF0000) >> 4;
            uint4 sig_offset = (device_sig_address & 0x0000FFFF);
            
            sig_line.imbue(std::locale::classic());
            sig_line << ":02000002" << std::uppercase << std::hex << std::setw(4) << std::setfill('0') << sig_segment
                     << std::uppercase << std::hex << std::setw(2) << std::setfill('0');
            sig_line << srecord_checksum(sig_line.str().c_str(), (uint4)sig_line.str().length());
            records.insert(--insert_pos, sig_line.str().c_str());
            sig_line.str("");
            insert_pos = records.end();
            sig_line << ":02" << std::uppercase << std::hex << std::setw(4) << std::setfill('0')
                     << sig_offset << "00"
                     << std::hex << std::setw(4) << std::setfill('0') << file_sig;
            sig_line << std::uppercase << std::hex << std::setw(2) << std::setfill('0')
                     << srecord_checksum(sig_line.str().c_str(), (uint4)sig_line.str().length());
            records.insert(
               --insert_pos, sig_line.str().c_str());
            file_size_bytes += (uint4)sig_line.str().length() + 2;
         }
         state = state_open;
         this->os_file_name = os_file_name;
      } // open_and_validate


      namespace
      {
         uint4 const get_device_type_timeout = 100;
         uint4 const reset_timeout = 500;
         uint4 const wait_for_flash_erase_timeout = 30000;
      };

      
      void LargeSRecordLoader::start_send(
         driver_handle driver,
         EventReceiver *client)
      {
         Csi::OStrAscStream msg;
         
         if(state != state_open)
            throw MsgExcept(my_strings[strid_invalid_start_state].c_str());
         OsLoaderBase::start_send(driver,client);
         retry_count = 0;
         if(send_devconfig_reset)
            send_reset_command();
         else
            start_send_device_type();
      } // start_send


      void LargeSRecordLoader::start_send_device_type()
      {
         OStrAscStream msg;
         driver->send(device_type.c_str(), (uint4)device_type.length());
         retry_count = records_sent = 0;
         get_device_type_id = timer->arm(this,get_device_type_timeout);
         state = state_getting_device_type;
         if(!send_devconfig_reset)
            msg << boost::format(my_strings[strid_waiting_for_type].c_str()) % device_type;
         else
            msg << boost::format(my_strings[strid_usb_waiting_for_type].c_str()) % device_type;
         send_status(msg.str().c_str());
      } // start_send_device_type

      
      void LargeSRecordLoader::cancel_send()
      {
         if(state > state_standby && state != state_error)
            on_error(my_strings[strid_cancelled].c_str());
         else
            throw MsgExcept(my_strings[strid_already_ended].c_str());
      } // cancel_send

      
      void LargeSRecordLoader::on_receive(
         OsLoaderDriver *driver,
         void const *buff,
         uint4 buff_len)
      {
         receive_buffer.push(buff,buff_len);
         switch(state)
         {
         case state_send_reset:
            check_for_reset_ack(static_cast<byte const *>(buff), buff_len);
            break;

         case state_getting_device_type:
            check_for_device_type();
            break;

         case state_wait_for_flash_erase:
            check_for_flash_erase_ack();
            break;

         case state_sending:
            check_for_ack();
            break;
         }
      } // on_receive

      
      void LargeSRecordLoader::on_complete(
         char const *message,
         bool succeeded)
      {
         records.clear();
         state = state_standby;
         if(get_device_type_id)
            timer->disarm(get_device_type_id);
         if(get_ack_id)
            timer->disarm(get_ack_id);
         OsLoaderBase::on_complete(message,succeeded);
      } // on_complete

      
      void LargeSRecordLoader::send_status(char const *message)
      { on_status(message,bytes_sent,file_size_bytes); }

      
      void LargeSRecordLoader::onOneShotFired(uint4 event_id)
      {
         if(event_id == get_device_type_id && state == state_getting_device_type)
         {
            get_device_type_id = 0;
            ++retry_count;
            if(retry_count * get_device_type_timeout > 60000)
               on_error(my_strings[strid_no_device_type_received].c_str());
            else if(driver != 0)
            {
               driver->send(device_type.c_str(), (uint4)device_type.length());
               get_device_type_id = timer->arm(this, get_device_type_timeout);
            }
         }
         else if(event_id == get_ack_id && state == state_sending)
         {
            get_ack_id = 0;
            if(retry_count++ >= 3)
               on_error(my_strings[strid_no_srecord_ack].c_str());
            else
               send_next_srecord(); 
         }
         else if(event_id == get_ack_id && state == state_wait_for_flash_erase)
         {
            get_ack_id = 0;
            on_error(my_strings[strid_no_ack_after_flash_erase].c_str());
         }
         else if(event_id == reset_id && state == state_send_reset)
         {
            reset_id = 0;
            if(retry_count < 20)
            {
               ++retry_count;
               send_reset_command();
            }
            else
               start_send_device_type();
         }
      } // onOneShotFired


      void LargeSRecordLoader::on_error(char const *message)
      { on_complete(message,false); }


      namespace
      {
         uint4 const get_ack_timeout = 1500;
      };
      
      
      void LargeSRecordLoader::send_next_srecord()
      {
         if(!records.empty() && driver != 0)
         {
            StrAsc const &record = records.front();
            driver->send(record.c_str(), (uint4)record.length());
            driver->send("\r\n",2);
            state = state_sending;
            if(get_ack_id)
               timer->disarm(get_ack_id);
            get_ack_id = timer->arm(this,get_ack_timeout);
         }
      } // send_next_srecord


      void LargeSRecordLoader::check_for_device_type()
      {
         uint4 device_type_pos = receive_buffer.find(
            device_type.c_str(), (uint4)device_type.length());
         if(device_type_pos < receive_buffer.size())
         {
            timer->disarm(get_device_type_id);
            receive_buffer.pop(receive_buffer.size());
            if(needs_flash_reset)
            {
               state = state_wait_for_flash_erase;
               send_status(my_strings[strid_wait_for_flash_erase].c_str());
               get_ack_id = timer->arm(this, wait_for_flash_erase_timeout);
            }
            else
            {
               retry_count = 0;
               send_next_srecord();
            }
         }
      } // check_for_device_type


      namespace
      {
         enum srecord_events
         {
            srecord_ack = 6,
            srecord_eot = 4,
            srecord_nak = 21
         };
      };

      
      void LargeSRecordLoader::check_for_flash_erase_ack()
      {
         // look for the next valid event
         byte next_event;
         while(state == state_wait_for_flash_erase &&
               receive_buffer.pop(&next_event, 1))
         {
            if(next_event == srecord_ack)
            {
               retry_count = 0;
               send_next_srecord();
            }
         }
      } // check_for_flash_erase_ack


      void LargeSRecordLoader::check_for_ack()
      {
         // look for a valid event
         byte next_event;
         while(receive_buffer.pop(&next_event,1))
         {
            // ignore anything but valid srecord events
            if(next_event == srecord_ack ||
               next_event == srecord_eot ||
               next_event == srecord_nak)
            {
               Csi::OStrAscStream msg;

               msg.imbue(std::locale::classic());
               switch(next_event)
               {
               case srecord_ack:
                  msg << boost::format(my_strings[strid_srecord_ack].c_str()) % records_sent;
                  ++records_sent;
                  bytes_sent += (uint4)records.front().length();
                  records.pop_front();
                  retry_count = 0;
                  send_status(msg.str().c_str());
                  send_next_srecord();
                  break;
                  
               case srecord_eot:
                  records.pop_front();
                  if(records.empty())
                  {
                     Csi::OStrAscStream sig;
                     sig << file_sig << " (0x" << std::hex << std::setfill('0')
                         << std::setw(4) << file_sig << ")";
                     msg << boost::format(my_strings[strid_sent].c_str()) % os_file_name % sig.str();
                     on_complete(msg.str().c_str(),true);
                  }
                  else
                     on_error(my_strings[strid_eot_received].c_str());
                  break;
                  
               case srecord_nak:
                  if(++retry_count < 4)
                  {
                     msg << boost::format(my_strings[strid_nak_received].c_str()) % records_sent % retry_count;
                     send_status(msg.str().c_str());
                     send_next_srecord();
                  }
                  else
                  {
                     msg << my_strings[strid_too_many_retries] << records_sent;
                     on_error(msg.str().c_str());
                  }
                  break;
               }
               receive_buffer.pop(receive_buffer.size());
            }
         }
      } // check_for_ack


      namespace
      {
         ////////////////////////////////////////////////////////////
         // predicate device_has_address
         ////////////////////////////////////////////////////////////
         struct device_has_address
         {
            uint4 const address;
            device_has_address(uint4 address_):
               address(address_)
            { }

            bool operator ()(DeviceTypeMap const &device) const
            { return device.address == address; }
         };


         void hex_to_str(StrAsc &dest, char const *buff, uint4 len)
         {
            uint2 ch;
            uint4 i = 0;
            while(i < len && i + 1 < len && (ch = hex_to_byte(buff + i)))
            {
               dest.append(static_cast<char>(ch));
               i += 2;
            }
         }
      };

      
      void LargeSRecordLoader::send_reset_command()
      {
         // form the command to send
         Csi::DevConfig::Message command;
         command.set_message_type(Csi::DevConfig::Messages::control_cmd);
         command.set_tran_no(0);
         command.addUInt2(0);   // security code
         command.addByte(Csi::DevConfig::ControlCodes::action_cancel_with_reboot);
         command.addUInt2(
            calcSigNullifier(
               calcSigFor(command.getMsg(), command.length())));
         
         // we now need to encode the output
         using Csi::PakBus::LowLevelDecoder;
         StrBin output;
         byte const *src = reinterpret_cast<byte const *>(command.getMsg());
         
         output.reserve(command.length() + 10);
         output.append(&LowLevelDecoder::synch_byte, 1);
         for(uint4 i = 0; i < command.length(); ++i)
         {
            byte temp = src[i];
            if(temp == LowLevelDecoder::synch_byte || temp == LowLevelDecoder::quote_byte)
            {
               output.append(&LowLevelDecoder::quote_byte, 1);
               temp += 0x20;
            }
            output.append(&temp, 1);
         }
         state = state_send_reset;
         output.append(&LowLevelDecoder::synch_byte, 1);
         driver->send(output.getContents(), (uint4)output.length());
         reset_id = timer->arm(this, reset_timeout);
         if(retry_count == 0)
            send_status(my_strings[strid_sending_reset].c_str());
      } // send_reset_command
      

      void LargeSRecordLoader::check_for_reset_ack(byte const *buff, uint4 buff_len)
      {
         using Csi::PakBus::LowLevelDecoder;
         LowLevelDecoder::decode_outcome_type outcome = LowLevelDecoder::decode_incomplete;
         Csi::OStrAscStream msg;
         uint4 scan_offset = 0;
         while(outcome == LowLevelDecoder::decode_incomplete &&
               scan_offset < buff_len &&
               state == state_send_reset)
         {
            uint4 begins_at = 0, processed = 0;
            outcome = pakbus_decoder.decode(
               buff + scan_offset,
               buff_len - scan_offset,
               begins_at,
               processed);
            scan_offset += processed;
            if(outcome == LowLevelDecoder::decode_found_devconfig_packet)
            {
               Csi::DevConfig::Message message(
                  pakbus_decoder.get_storage(), pakbus_decoder.get_storage_len());
               if(message.get_message_type() == Csi::DevConfig::Messages::control_ack)
               {
                  byte response_code = message.readByte();
                  switch(response_code)
                  {
                  case Csi::DevConfig::ControlCodes::outcome_discarded_with_reboot:
                     timer->disarm(reset_id);
                     start_send_device_type();
                     break; 
                  }
               }
            }
         }
      } // check_for_reset_ack


      void LargeSRecordLoader::validate_device_type(
         uint4 address, char const *data, uint4 data_len)
      {
         device_types_type::iterator di = std::find_if(
            device_types.begin(), device_types.end(), device_has_address(address));
         StrAsc temp;
         while(di != device_types.end() && device_type.length() == 0)
         {
            temp.cut(0);
            hex_to_str(temp, data, data_len * 2);
            if(temp == di->device_type)
            {
               device_type = temp;
               device_sig_address = di->sig_address;
            }
            else
               di = std::find_if(++di, device_types.end(), device_has_address(address)); 
         }
      } // validate_device_type
   };
};


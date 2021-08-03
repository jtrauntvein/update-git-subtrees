/* Csi.OsLoader.CstermLoader.cpp

   Copyright (C) 2004, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 22 March 2004
   Last Change: Friday 29 April 2016
   Last Commit: $Date: 2016-05-03 16:18:03 -0600 (Tue, 03 May 2016) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.OsLoader.CstermLoader.h"
#include "Csi.Utils.h"
#include "Csi.OsException.h"
#include "Csi.PakBus.SerialDecode.h"
#include "Csi.DevConfig.Message.h"
#include "Csi.MaxMin.h"
#include "coratools.strings.h"
#include <sstream>
#include <iomanip>


namespace Csi
{
   namespace OsLoader
   {
      using namespace CstermLoaderStrings;

      
      ////////////////////////////////////////////////////////////
      // class CstermLoader definitions
      ////////////////////////////////////////////////////////////
      CstermLoader::CstermLoader(
         timer_handle &timer_,
         init_method_type init_method_):
         timer(timer_),
         file_sig(0),
         bytes_sent(0),
         state(state_standby),
         synch_id(0),
         erase_flash_id(0),
         send_id(0),
         init_method(init_method_),
         should_unquote_next(false),
         wait_after_id(0)
      { }

      
      CstermLoader::~CstermLoader()
      {
         if(synch_id)
            timer->disarm(synch_id);
         if(erase_flash_id)
            timer->disarm(erase_flash_id);
         if(send_id)
            timer->disarm(send_id);
         if(wait_after_id)
            timer->disarm(wait_after_id);
         timer.clear();
      } // destructor


      namespace
      {
         ////////////////////////////////////////////////////////////
         // validate_cr9000x
         ////////////////////////////////////////////////////////////
         void validate_cr9000x(
            byte const *buff,
            uint4 buff_len,
            uint4 boot_size)
         {
            uint4 const header_size = 10;
            uint4 const device_id_offset = 6;
            uint4 const block_size_offset = 2;
            uint4 const block_sig_offset = 0;
            uint4 const boot_sig_offset = 8;
            byte const *current_block = buff;
            uint4 block_size;
            uint4 device_id;
            uint2 given_sig;
            uint2 given_boot_sig;

            while(current_block + header_size < buff + buff_len)
            {
               // the following information resides in the header of all blocks
               memcpy(
                  &device_id,
                  current_block + device_id_offset,
                  sizeof(device_id));
               memcpy(
                  &block_size,
                  current_block + block_size_offset,
                  sizeof(block_size));
               memcpy(
                  &given_sig,
                  current_block + block_sig_offset,
                  sizeof(given_sig));
               if(is_big_endian())
               {
                  reverse_byte_order(&device_id,sizeof(device_id));
                  reverse_byte_order(&block_size,sizeof(block_size));
                  reverse_byte_order(&given_sig,sizeof(given_sig));
               }
               given_boot_sig = static_cast<uint2>(device_id & 0xFFFF);
               device_id >>= 16;

               // we need to make certain that there is enough space for the current block
               if(current_block + block_size > buff + buff_len)
                  throw MsgExcept(my_strings[strid_current_block_too_big].c_str());

               // verifying the CPU image is different from the other types because of the presence
               // of the boot sector
               if(device_id == 9032 || device_id == 150 || device_id == 48)
               {
                  // verify the boot code signature
                  uint2 calculated_boot_sig = calcSigFor(
                     current_block + header_size,
                     boot_size - 2);
                  if(given_boot_sig != calculated_boot_sig)
                     throw MsgExcept(my_strings[strid_invalid_boot_sig].c_str());

                  // we now need to verify the CPU OS signature
                  byte const *os_begin = current_block + boot_size + 26;
                  uint4 os_size = block_size - (boot_size + 16);
                  uint2 os_sig = calcSigFor(os_begin,os_size);
                  if(os_sig != given_sig)
                     throw MsgExcept(my_strings[strid_invalid_os_sig].c_str());
               }
               else if(device_id == 9058 || device_id == 9052)
               {
                  // here, we can simply verify the given sig with the calculated one
                  uint2 calculated_sig = calcSigFor(
                     current_block + header_size,
                     block_size);
                  if(calculated_sig != given_sig)
                     throw MsgExcept(my_strings[strid_invalid_filter_sig].c_str());
               }
               else
                  throw MsgExcept(my_strings[strid_unrecognised_device_type].c_str());

               // advance to the next block
               current_block += block_size + header_size;
            }
         } // validate_cr9000x


         ////////////////////////////////////////////////////////////
         // validate_other_os
         //
         // Valdiates an image from another OS type.  Assumes that the os is encoded in big endian. 
         ////////////////////////////////////////////////////////////
         void validate_other_os(
            byte const *buff,
            uint4 buff_len,
            uint4 boot_size)
         {
            // read the given information from the header
            uint4 const header_size = 10;
            uint4 const block_size_offset = 2;
            uint4 const block_sig_offset = 0;
            uint4 const boot_sig_offset = 8;
            uint4 block_size;
            uint2 given_sig;
            
            if(boot_size > 0 && buff_len < boot_size + header_size)
               throw MsgExcept(my_strings[strid_invalid_file_size].c_str());
            memcpy(
               &block_size,
               buff + block_size_offset,
               sizeof(block_size));
            memcpy(
               &given_sig,
               buff + block_sig_offset,
               sizeof(given_sig));
            if(!is_big_endian())
            {
               reverse_byte_order(&block_size,sizeof(block_size));
               reverse_byte_order(&given_sig,sizeof(given_sig));
            }
            if(block_size + header_size > buff_len)
               throw MsgExcept(my_strings[strid_current_block_too_big].c_str());

            // if there is a boot image expected, we will validate that first
            byte const *os_begin = buff + header_size;
            uint4 os_size = block_size;
            
            if(boot_size > 0)
            {
               uint2 given_boot_sig;
               uint2 calculated_boot_sig;
               
               memcpy(
                  &given_boot_sig,
                  buff + boot_sig_offset,
                  sizeof(given_boot_sig));
               if(!is_big_endian())
                  reverse_byte_order(&given_boot_sig,sizeof(given_boot_sig));
               calculated_boot_sig = calcSigFor(os_begin,boot_size - 2);
               if(given_boot_sig != calculated_boot_sig)
                  throw MsgExcept(my_strings[strid_invalid_boot_sig].c_str());
               os_begin = buff + boot_size + 26;
               os_size = block_size - (boot_size + 16);
            }

            // we can now validate the remainder of the operating system
            uint2 calculated_sig = calcSigFor(os_begin,os_size);
            if(calculated_sig != given_sig)
               throw MsgExcept(my_strings[strid_invalid_os_sig].c_str());
         } // validate_other_os

         
         ////////////////////////////////////////////////////////////
         // validate_file_structure
         //
         // Validates the contents of an OS image file structure.  Will throw an exception should
         // the validation fail.  The return value will report the number of bytes recorded in the
         // file header (this is the number of bytes that will be sent to the logger)
         ////////////////////////////////////////////////////////////
         uint4 validate_file_structure(
            byte const *buff,
            uint4 buff_len)
         {
            // the CR9000X and the CS150 both are little endian machines and their device type ID is
            // also stored in little endian
            uint4 const device_id_offset = 6;
            uint4 const os_size_offset = 2;
            uint4 device_id;
            uint4 rtn = 0;
            
            if(buff_len < 10)
               throw MsgExcept(my_strings[strid_invalid_file_size].c_str());
            memcpy(&device_id,buff + device_id_offset,sizeof(device_id));
            memcpy(&rtn,buff + os_size_offset,sizeof(rtn));
            if(is_big_endian())
               reverse_byte_order(&device_id,sizeof(device_id));
            switch(device_id >> 16)
            {
            case 9032:          // CR9032
               validate_cr9000x(buff, buff_len, 0x4000);
               if(is_big_endian())
                  reverse_byte_order(&rtn,sizeof(rtn));
               break;
               
            case 150:           // CS150
               validate_cr9000x(buff, buff_len, 0x4000);
               if(is_big_endian())
                  reverse_byte_order(&rtn,sizeof(rtn));
               break;
               
            case 48:            // CDM-VW300
               validate_cr9000x(buff, buff_len, 0x6000);
               if(is_big_endian())
                  reverse_byte_order(&rtn,sizeof(rtn));
               break;

            default:
               if(!is_big_endian())
               {
                  reverse_byte_order(&device_id,sizeof(device_id));
                  reverse_byte_order(&rtn,sizeof(rtn));
               }
               switch(device_id >> 16)
               {
               case 0x100:      // nl100
               case 0x5000:     // CR5000
                  validate_other_os(buff, buff_len, 0);
                  break;

               case 300:        // AVW200 & AVW206
               case 1000:       // CR1000
               case 3000:       // CR3000
               case 0xED:       // CSAT3A
                  validate_other_os(buff, buff_len, 0x4000);
                  break;

               default:
                  throw MsgExcept(my_strings[strid_unrecognised_device_type].c_str());
                  break;
               }
               break;
            }
            return rtn + 10;
         } // validate_file_structure
      };

      
      void CstermLoader::open_and_validate(
         char const *os_file_name_)
      {
         // check the current state and open the file for input
         if(state != state_standby)
            throw MsgExcept(my_strings[strid_invalid_start_state].c_str());
         try
         {
            input.bind(new ReadFileMapping(os_file_name_));
            input_bytes_len = static_cast<uint4>(input->file_size());
            input_bytes = static_cast<byte const *>(
               input->open_view(0,input_bytes_len));
         }
         catch(OsException &open_error)
         {
            throw OsException(
               open_error.get_osError(),
               my_strings[strid_unable_to_open_input].c_str());
         }

         // the next step is to validate the structures in the file.
         uint4 image_size = validate_file_structure(input_bytes,input_bytes_len);
         file_sig = calcSigFor(input_bytes,input_bytes_len);
         input_bytes_len = image_size;
         os_file_name = os_file_name_;
         bytes_sent = 0;
         state = state_open;
      } // open_and_validate


      namespace
      {
         byte const synch_byte = 0xBC;
         uint4 const synch_timeout = 100;
         uint4 max_synch_retries = 600;
         byte const synch_resp[] = "\x5a\xfe";
         byte const flash_erase[] = "\xc0\xde";
         uint4 const flash_erase_timeout = 60000;
         uint4 const terminal_prompt_timeout = 100;
         byte const toboot_cmd[] = "TOBOOT";
         uint4 const toboot_between = 75;
         uint4 const boot_prompt_timeout = 1000;
      };

      
      void CstermLoader::start_send(
         driver_handle driver,
         EventReceiver *client)
      {
         // check the state and set up the base class
         if(state != state_open)
            throw MsgExcept(my_strings[strid_invalid_start_state].c_str());
         OsLoaderBase::start_send(driver,client);

         // start the synchronisation process
         retry_count = 0;
         send_status(my_strings[strid_waiting_for_synch].c_str());
         switch(init_method)
         {
         case init_assume_boot_mode:
            start_synch();
            break;
            
         case init_use_toboot:
            driver->send("\r",1);
            state = state_get_terminal_prompt;
            synch_id = timer->arm(this,terminal_prompt_timeout);
            break;

         case init_use_devconfig:
            start_devconfig_reset();
            break;

         default:
            throw std::invalid_argument("Invalid initialisation method");
            break;
         }            
      } // start_send

      
      void CstermLoader::cancel_send()
      {
         if(state != state_standby)
            on_error(my_strings[strid_send_cancelled].c_str());
         else
            throw std::invalid_argument(my_strings[strid_already_cancelled].c_str());
      }


      void CstermLoader::on_receive(
         OsLoaderDriver *driver,
         void const *buff,
         uint4 buff_len)
      {
         receive_buffer.push(buff,buff_len);
         switch(state)
         {
         case state_get_terminal_prompt:
            check_for_terminal_prompt();
            break;

         case state_to_boot:
            check_for_boot_prompt();
            break;

         case state_send_devconfig:
            check_for_devconfig_reset();
            break;
            
         case state_synch1:
            check_for_synch1();
            break;

         case state_synch2:
            check_for_synch2();
            break;

         case state_erase_flash1:
            check_for_flash_erase1();
            break;

         case state_erase_flash2:
            check_for_flash_erase2();
            break;

         case state_send:
            check_for_sig();
            break;

         case state_wait_after:
            check_wait_after();
            break;
         };
      } // on_receive

      
      void CstermLoader::on_complete(
         char const *message,
         bool succeeded)
      {
         if(synch_id)
            timer->disarm(synch_id);
         if(erase_flash_id)
            timer->disarm(erase_flash_id);
         if(send_id)
            timer->disarm(send_id);
         if(state != state_standby)
         {
            state = state_standby;
            OsLoaderBase::on_complete(message,succeeded);
         }
      } // on_complete

      
      void CstermLoader::send_status(char const *message)
      { on_status(message,bytes_sent,input_bytes_len); }

      
      void CstermLoader::onOneShotFired(uint4 event_id)
      {
         if(state == state_get_terminal_prompt && event_id == synch_id)
         {
            synch_id = 0;
            if(++retry_count < max_synch_retries)
            {
               driver->send("\r",1);
               synch_id = timer->arm(this,terminal_prompt_timeout);
            }
            else
               on_error(my_strings[strid_no_term_prompt].c_str()); 
         }
         else if(state == state_to_boot && event_id == synch_id)
         {
            synch_id = 0;
            if(!send_buffer.empty())
            {
               driver->send(&send_buffer[0],1);
               send_buffer.pop(1);
               if(send_buffer.empty())
                  synch_id = timer->arm(this,boot_prompt_timeout);
               else
                  synch_id = timer->arm(this,toboot_between);
            }
            else
               on_error(my_strings[strid_no_boot_prompt].c_str());
         }
         else if(state == state_synch1 && event_id == synch_id)
         {
            synch_id = 0;
            if(++retry_count < max_synch_retries)
            {
               driver->send(&synch_byte,1);
               synch_id = timer->arm(this,synch_timeout);
            }
            else
               on_error(my_strings[strid_no_contact].c_str());
         }
         else if(state == state_synch2 && event_id == synch_id)
         {
            synch_id = 0;
            on_error(my_strings[strid_no_synch].c_str());
         }
         else if(state == state_send_devconfig && event_id == synch_id)
         {
            synch_id = 0;
            if(++retry_count < max_synch_retries)
               start_devconfig_reset();
            else
               on_error(my_strings[strid_no_devconfig_response].c_str());
         }
         else if(state == state_erase_flash1 && event_id == erase_flash_id)
         {
            // there are some CR1000 operating systems that don't drive the UART as they should.  As
            // a result, the UART may shut down while flash is being erased and the characters that
            // were are expecting won't make it out.  In this case, it's safer to proceed with the
            // first segment than it is to abort the protocol.
            erase_flash_id = 0;
            send_status(my_strings[strid_starting_send].c_str());
            state = state_erase_flash3;
            erase_flash_id = timer->arm(this,10);
            fragment_sig = 0xAAAA;
         }
         else if(state == state_erase_flash2 && event_id == erase_flash_id)
         {
            erase_flash_id = 0;
            on_error(my_strings[strid_no_response_after_reset].c_str());
         }
         else if(state == state_erase_flash3 && event_id == erase_flash_id)
         {
            erase_flash_id = 0;
            send_next();
         }
         else if(state == state_send && event_id == send_id)
         {
            std::ostringstream temp;
            temp << my_strings[strid_no_response_after_last] << " "
                 << my_strings[strid_wont_boot_warning];
            send_id = 0;
            on_error(temp.str().c_str());
         }
         else if(state == state_wait_after && event_id == wait_after_id)
         {
            if(Csi::counter(reboot_base) < 60000)
            {
               driver->send("\r",1);
               wait_after_id = timer->arm(this,500);
            }
            else
            {
               wait_after_id = 0;
               on_error(my_strings[strid_no_boot_response].c_str());
            }
         }
         else if(state == state_complete && event_id == wait_after_id)
         {
            std::ostringstream msg;
            wait_after_id = 0;
            msg.imbue(std::locale::classic());
            msg << my_strings[strid_sent1] << os_file_name
                << my_strings[strid_sent2]
                << my_strings[strid_sent3] << file_sig << " (0x"
                << std::hex << std::setw(4) << std::setfill('0') << std::uppercase
                << file_sig << ")</p>\n"
                << get_extra_admonition();
            on_complete(msg.str().c_str(),true); 
         }
      } // onOneShotFired

      
      void CstermLoader::on_error(char const *message)
      {
         state = state_error;
         on_complete(message,false);
      } // on_error


      void CstermLoader::start_synch()
      {
         state = state_synch1;
         if(synch_id != 0)
            timer->disarm(synch_id);
         driver->send(&synch_byte,1);
         synch_id = timer->arm(this,synch_timeout);
      } // start_synch


      void CstermLoader::start_devconfig_reset()
      {
         // form the command
         using namespace Csi::DevConfig;
         Message cmd;
         
         cmd.set_message_type(Messages::control_cmd);
         cmd.addUInt2(get_security_code());
         cmd.addByte(ControlCodes::action_cancel_with_reboot);
         cmd.addUInt2(
            calcSigNullifier(
               calcSigFor(cmd.getMsg(),cmd.length())));

         // send the message through the driver
         byte const *src = reinterpret_cast<byte const *>(cmd.getMsg());
         
         driver->send(&Csi::PakBus::SerialDecode::synch_byte,1);
         for(uint4 i = 0; i < cmd.length(); ++i)
         {
            byte temp = src[i];
            if(temp == Csi::PakBus::SerialDecode::synch_byte ||
               temp == Csi::PakBus::SerialDecode::quote_byte)
            {
               driver->send(&Csi::PakBus::SerialDecode::quote_byte,1);
               temp += 0x20; 
            }
            driver->send(&temp,1);
         }
         driver->send(&Csi::PakBus::SerialDecode::synch_byte,1);
         state = state_send_devconfig;
         synch_id = timer->arm(this,synch_timeout);
      } // start_devconfig_reset


      void CstermLoader::check_for_devconfig_reset()
      {
         // we want to attempt to decode the receive buffer
         using namespace Csi::PakBus::SerialDecode;
         using namespace Csi::DevConfig;
         uint4 start_offset = 0;
         decode_outcome_type outcome;
         uint4 decode_len;
         StrBin temp;
         
         receive_buffer.pop(temp,receive_buffer.size());
         while(start_offset < temp.length())
         {
            decode_len = decode_quoted_data(
               devconfig_buffer,
               should_unquote_next,
               outcome,
               temp.getContents() + start_offset,
               (uint4)temp.length() - start_offset);
            start_offset += decode_len;
            if(outcome == decode_synch_found)
            {
               if(devconfig_buffer.length() >= Message::header_len + 2)
               {
                  uint2 buff_sig = calcSigFor(
                     devconfig_buffer.getContents(),
                     devconfig_buffer.length());
                  if(buff_sig == 0)
                  {
                     if(devconfig_buffer[0] == Message::packet_type_byte)
                     {
                        Message message(
                           devconfig_buffer.getContents(), (uint4)devconfig_buffer.length(), false);
                        if(message.get_message_type() == Messages::control_ack)
                        {
                           byte outcome = message.readByte();
                           switch(outcome)
                           {
                           case ControlCodes::outcome_discarded_with_reboot:
                              timer->disarm(synch_id);
                              start_synch();
                              break;
                              
                           case ControlCodes::outcome_invalid_security_code:
                              if(on_security_code_wrong())
                                 start_devconfig_reset();
                              else
                                 on_error(my_strings[strid_devconfig_security_wrong].c_str());
                           }
                        }
                        // if the device is alredy in the boot monitor, it will simply be echoing
                        // back the packets we send.  This being the case, we can proceed as if the
                        // packet was an ack.
                        else if(message.get_message_type() == Messages::control_cmd)
                        {
                           timer->disarm(synch_id);
                           start_synch();
                        }
                     }
                  }
               }
               devconfig_buffer.cut(0);
            }
         }
      } // check_for_devconfig_reset


      void CstermLoader::check_for_terminal_prompt()
      {
         uint4 resp_pos = receive_buffer.find(">",1);
         if(resp_pos < receive_buffer.size())
         {
            send_status(my_strings[strid_entering_boot_mode].c_str());
            receive_buffer.pop(receive_buffer.size());
            send_buffer.push(toboot_cmd,sizeof(toboot_cmd) - 1);
            state = state_to_boot;
            timer->disarm(synch_id);
            driver->send(&send_buffer[0],1);
            send_buffer.pop(1);
            synch_id = timer->arm(this,toboot_between);
         }
         else
         {
            // we will also look for the echo of the carraige return to see if the logger is already
            // in boot code
            resp_pos = receive_buffer.find("\r\r\r\r",4);
            if(resp_pos < receive_buffer.size())
            {
               receive_buffer.pop(receive_buffer.size());
               timer->disarm(synch_id);
               start_synch();
            }
         }
      } // check_for_terminal_prompt


      void CstermLoader::check_for_boot_prompt()
      {
         uint4 resp_pos = receive_buffer.find("BOOT!",5);
         if(resp_pos < receive_buffer.size())
         {
            send_status(my_strings[strid_getting_synch].c_str());
            receive_buffer.pop(receive_buffer.size());
            start_synch();
         }
      } // check_for_boot_prompt
      

      void CstermLoader::check_for_synch1()
      {
         uint4 resp_pos = receive_buffer.find(&synch_byte,1);
         if(resp_pos < receive_buffer.size())
         {
            timer->disarm(synch_id);
            driver->send(&synch_byte,1);
            state = state_synch2;
            synch_id = timer->arm(this,5000);
            send_status(my_strings[strid_received_synch].c_str());
         }
      } // check_for_synch1


      void CstermLoader::check_for_synch2()
      {
         uint4 resp_pos = receive_buffer.find(synch_resp,sizeof(synch_resp) - 1);
         if(resp_pos < receive_buffer.size())
         {
            receive_buffer.pop(receive_buffer.size());
            timer->disarm(synch_id);
            bytes_sent = 10;
            driver->send(flash_erase,sizeof(flash_erase) - 1);
            driver->send(input_bytes,bytes_sent);
            state = state_erase_flash1;
            erase_flash_id = timer->arm(this,flash_erase_timeout);
            send_status(my_strings[strid_waiting_for_flash_erase].c_str());
         }
      } // check_for_synch2


      void CstermLoader::check_for_flash_erase1()
      {
         uint4 resp_pos = receive_buffer.find("\xbc",1);
         if(resp_pos < receive_buffer.size())
         {
            receive_buffer.pop(1);
            timer->disarm(erase_flash_id);
            send_status(my_strings[strid_waiting_for_reset].c_str());
            erase_flash_id = timer->arm(this,flash_erase_timeout);
            state = state_erase_flash2;
            if(!receive_buffer.empty())
               check_for_flash_erase2();
         }
      } // check_for_flash_erase1


      void CstermLoader::check_for_flash_erase2()
      {
         uint4 resp_pos = receive_buffer.find("\xbd",1);
         if(resp_pos < receive_buffer.size())
         {
            receive_buffer.pop(1);
            send_status(my_strings[strid_starting_send].c_str());
            timer->disarm(erase_flash_id);
            state = state_erase_flash3;
            erase_flash_id = timer->arm(this,10);
            fragment_sig = 0xAAAA;
         }
      } // check_for_flash_erase2


      void CstermLoader::send_next()
      {
         // read in the next file fragment
         byte fragment[1024];
         uint4 fragment_len = csimin(
            input_bytes_len - bytes_sent,
            static_cast<uint4>(1024));

         memset(fragment,'\xff',sizeof(fragment));
         if(fragment_len > 0)
         {
            memcpy(fragment,input_bytes + bytes_sent,fragment_len);
            bytes_sent += fragment_len;
            fragment_sig = calcSigFor(fragment,sizeof(fragment),fragment_sig);
            if(is_big_endian())
               reverse_byte_order(&fragment_sig,sizeof(fragment_sig));
            
            // send the fragment and set the timer
            std::ostringstream msg;
            msg << my_strings[strid_sending] << bytes_sent;
            send_status(msg.str().c_str());
            driver->send(fragment,sizeof(fragment));
            driver->send(&fragment_sig,sizeof(fragment_sig));
            state = state_send;
            send_id = timer->arm(this, 20000);
         }
         else
         {
            state = state_wait_after;
            send_status(my_strings[strid_waiting_after].c_str());
            wait_after_id = timer->arm(this,5000);
            reboot_base = Csi::counter(0);
         }
      } // send_next


      void CstermLoader::check_for_sig()
      {
         if(receive_buffer.size() >= sizeof(fragment_sig))
         {
            uint2 rcv_sig;
            receive_buffer.pop(&rcv_sig,sizeof(rcv_sig));
            if(fragment_sig == rcv_sig)
            {
               timer->disarm(send_id);
               send_next();
            }
            else
            {
               std::ostringstream temp;
               temp << my_strings[strid_invalid_sig_received] << " "
                    << my_strings[strid_wont_boot_warning];
               on_error(temp.str().c_str());
            }
         }
      } // check_for_sig


      void CstermLoader::check_wait_after()
      {
         uint4 crlf_pos = receive_buffer.find("\r\n",2);
         if(crlf_pos < receive_buffer.size())
         {
            timer->disarm(wait_after_id);
            state = state_complete;
            wait_after_id = timer->arm(this,10);
         }
      } // checkl_wait_after
   };
};


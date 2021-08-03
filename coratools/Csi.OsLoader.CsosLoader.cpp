/* Csi.OsLoader.CsosLoader.cpp

   Copyright (C) 2004, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 15 March 2004
   Last Change: Monday 12 October 2009
   Last Commit: $Date: 2009-10-12 10:19:23 -0600 (Mon, 12 Oct 2009) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.OsLoader.CsosLoader.h"
#include "MsgExcept.h"
#include "Csi.Utils.h"
#include "Csi.MaxMin.h"
#include "coratools.strings.h"
#include <sstream>
#include <iomanip>


namespace Csi
{
   namespace OsLoader
   {
      using namespace CsosLoaderStrings;
      
      
      ////////////////////////////////////////////////////////////
      // class CsosLoader definitions
      ////////////////////////////////////////////////////////////
      CsosLoader::CsosLoader(timer_handle &timer_):
         device_type_code(0),
         timer(timer_),
         view(0),
         view_len(0),
         state(state_standby),
         receive_packet(0),
         last_fragment(0),
         os_for_logger(false)
      { }


      CsosLoader::~CsosLoader()
      {
         if(timer != 0 && timer_id != 0)
            timer->disarm(timer_id);
         source.clear();
         timer.clear();
      } // destructor


      namespace
      {
         uint4 const device_type_pos1 = 0x3FE0;
         uint4 const device_type_pos2 = 0x1FFB0; 
      };

   
      void CsosLoader::open_and_validate(
         char const *os_file_name_)
      {
         // check the state of the loader
         if(state != state_standby && state != state_open)
            throw MsgExcept(my_strings[strid_invalid_loader_state].c_str());
      
         // open the file to be read
         os_file_name = os_file_name_;
         source.bind(
            new source_type(os_file_name_));
         view_len = static_cast<uint4>(source->file_size());
         view = reinterpret_cast<byte const *>(source->open_view(0,view_len));
         os_for_logger = true;
         
         // we must validate the size of the file.  It must be divisable by 32K or 16K
         if((view_len % 0x8000) != 0 &&
            (view_len % 0x4000) != 0 &&
            view_len < device_type_pos1 + 1)
            throw MsgExcept(my_strings[strid_invalid_obj_length].c_str());

         // we need to find out what device type the file is compiled for.  We do this by attempting
         // to read that type from the file contents
         uint4 type_offset = device_type_pos1;
         uint4 segment_size = 0x8000;
         uint4 sig_offset = 0;
         uint4 data_offset = 2;
         
         device_type_code = *reinterpret_cast<uint2 const *>(
            view + type_offset);
         if(device_type_code == 0xFFFF)
         {
            if(view_len >= device_type_pos2 + 2)
            {
               type_offset = device_type_pos2;
               device_type_code = *reinterpret_cast<uint2 const *>(
                  view + type_offset);
            }
            else
               device_type_code = 0;
         }
         if(!is_big_endian())
            reverse_byte_order(&device_type_code,sizeof(device_type_code));
         if(device_type_code != 0)
            device_type = reinterpret_cast<char const *>(view + type_offset + 2);
         if((view_len % 0x8000) == 0 && device_type[0] == 'S')
         {
            segment_size = 0x4000;
            sig_offset = 0x3FFE;
            data_offset = 0;
            os_for_logger = false;
         }

         // we need to validate the signature of each segment.  This signature will be stored in the
         // first two bytes of each segment.
         file_sig = 0xAAAA;
         for(uint4 i = 0; i < view_len; i += segment_size)
         {
            uint4 chunk_size = csimin(
               segment_size,
               view_len - i);
            uint2 segment_sig = calcSigFor(
               view + i + data_offset,
               chunk_size - 2);
            uint2 stored_sig = *reinterpret_cast<uint2 const *>(view + i + sig_offset);

            file_sig = calcSigFor(view + i,chunk_size,file_sig);
            if(!is_big_endian())
               reverse_byte_order(&stored_sig,sizeof(stored_sig));
            if(segment_sig != stored_sig)
            {
               std::ostringstream msg;
               msg << my_strings[strid_invalid_segment_sig] << " " << i;
               throw MsgExcept(msg.str().c_str());
            }
         }
         state = state_open;
      } // open_and_validate


      namespace
      {
         byte const synch_sequence[] = "\xBA";
         uint4 synch_timeout = 100;
         uint4 overall_synch_timeout = 15000;
         uint4 fragment_timeout = 10000;
         uint4 fragment_len = 256;
      };

   
      void CsosLoader::start_send(
         driver_handle driver,
         EventReceiver *client)
      {
         if(state == state_open)
         {
            std::ostringstream msg;
         
            OsLoaderBase::start_send(driver,client);
            send_pos = 0;
            driver->send(synch_sequence,sizeof(synch_sequence) - 1);
            retry_count = 0;
            state = state_synch1;
            timer_id = timer->arm(this,synch_timeout);
            msg << my_strings[strid_waiting_for_dev_code1] << device_type << "\n"
                << my_strings[strid_waiting_for_dev_code2];
            send_status(msg.str().c_str());
         }
         else
            throw MsgExcept(my_strings[strid_invalid_send_state].c_str());
      } // start_send


      void CsosLoader::cancel_send()
      {
         if(state != state_standby)
            on_error(my_strings[strid_send_cancelled].c_str());
         else
            throw std::invalid_argument(my_strings[strid_already_cancelled].c_str());
      } // cancel_send


      void CsosLoader::on_receive(
         OsLoaderDriver *driver,
         void const *buff,
         uint4 buff_len)
      {
         receive_packet.addBytes(buff,buff_len);
         if(receive_packet.whatsLeft() > 0)
         {
            switch(state)
            {
            case state_synch1:
            case state_synch2:
               while(receive_packet.whatsLeft() > 0 && 
                     (state == state_synch1 || state == state_synch2))
                  test_for_synch();
               if(state == state_send)
               {
                  driver->send("\xA5\x5A",2);
                  send_next_fragment();
               }
               else if(state == state_standby)
                  on_error(my_strings[strid_expected_synch_failed].c_str());
               break;
            
            case state_send:
               while(receive_packet.whatsLeft() >= 2)
                  test_for_sig();
               break;
            }
         }
      } // on_receive


      void CsosLoader::onOneShotFired(
         uint4 event_id)
      {
         if(event_id == timer_id)
         {
            timer_id = 0;
            switch(state)
            {
            case state_synch1:
               ++retry_count;
               if(retry_count * synch_timeout < overall_synch_timeout)
               {
                  driver->send(synch_sequence,sizeof(synch_sequence) - 1);
                  timer_id = timer->arm(this,synch_timeout);
               }
               else
               {
                  on_error(my_strings[strid_no_device_response].c_str());
               }
               break;
            
            case state_send:
               if(retry_count++ < 3)
               {
                  std::ostringstream msg;
                  msg << my_strings[strid_retrying_fragment1] << (send_pos / fragment_len)
                      << my_strings[strid_retrying_fragment2];
                  send_status(msg.str().c_str());
                  driver->send(last_fragment.getMsg(),last_fragment.length());
                  timer_id = timer->arm(this,fragment_timeout);
               }
               else
               {
                  std::ostringstream temp;
                  temp << my_strings[strid_no_sig_received] << " "
                       << my_strings[strid_wont_boot_warning];
                  on_error(temp.str().c_str());
               }
               break;

            case state_finish:
            {
               std::ostringstream msg;
               msg << my_strings[strid_finished1] << file_sig
                   << " (0x" << std::hex << std::setfill('0')
                   << std::setw(4) << file_sig << ")\n\n";
               if(os_for_logger)
               {
                  msg << my_strings[strid_finished2] << device_type
                      << my_strings[strid_finished3];
               }
               on_complete(msg.str().c_str(),true);
               break;
            }

            case state_error:
               on_complete(error_message.c_str(),false);
               break;
            }
         }
      } // onOneShotFired


      void CsosLoader::on_complete(
         char const *message,
         bool succeeded)
      {
         source.clear();
         view = 0;
         view_len = 0;
         device_type.cut(0);
         device_type_code = 0;
         state = state_standby;
         if(timer_id != 0)
            timer->disarm(timer_id);
         OsLoaderBase::on_complete(message,succeeded,succeeded);
      } // on_complete


      void CsosLoader::send_status(char const *message)
      { on_status(message,send_pos,view_len); }


      void CsosLoader::test_for_synch()
      {
         assert(state == state_synch1 || state == state_synch2);
         assert(receive_packet.whatsLeft() > 0);
         uint2 first = receive_packet.readByte();

         if(state == state_synch1)
         {
            if(first == 0xBA)
            {
               if(device_type_code == 0 ||
                  device_type.find("CR10X") < device_type.length())
               {
                  state = state_send;
               }
               else
                  receive_packet.clear();
            }
            else if(first == (device_type_code >> 8))
               state = state_synch2;
            else
               receive_packet.clear();
         }
         else if(state == state_synch2)
         {
            if(first == (device_type_code & 0xFF))
            {
               timer->disarm(timer_id);
               retry_count = 0;
               state = state_send;
            }
            else
               state = state_standby;
         }
      } // test_for_synch


      void CsosLoader::test_for_sig()
      {
         if(receive_packet.whatsLeft() >= 2)
         {
            uint2 receive_sig = receive_packet.readUInt2(!is_big_endian());
            if(receive_sig == fragment_sig)
            {
               retry_count = 0;
               send_pos += fragment_len;
               timer->disarm(timer_id);
               send_next_fragment();
            }
            else
            {
               std::ostringstream temp;
               temp << my_strings[strid_invalid_sig_received] << (send_pos / fragment_len);
               temp << " " << my_strings[strid_wont_boot_warning];
               on_error(temp.str().c_str());
            }
         }
      } // test_for_sig

   
      void CsosLoader::send_next_fragment()
      {
         if(send_pos < view_len)
         {
            std::ostringstream msg;
            msg << my_strings[strid_sending1] << (send_pos / fragment_len)
                << my_strings[strid_sending2] << (view_len / fragment_len);
            send_status(msg.str().c_str());
            receive_packet.clear();
            last_fragment.clear();
            last_fragment.addUInt2(
               static_cast<uint2>(send_pos / fragment_len),
               !is_big_endian());
            last_fragment.addBytes(view + send_pos,fragment_len,false);
            fragment_sig = calcSigFor(last_fragment.getMsg(),last_fragment.length());
            last_fragment.addUInt2(
               fragment_sig,
               !is_big_endian());
            driver->send(last_fragment.getMsg(),last_fragment.length());
            timer_id = timer->arm(this,fragment_timeout);
         }
         else
         {
            send_status(my_strings[strid_last_fragment_sent].c_str());
            state = state_finish;
            driver->send("\xFF\xFF",2);
            timer_id = timer->arm(this,1000);
         }
      } // send_next_fragment


      void CsosLoader::on_error(char const *error_message_)
      {
         error_message = error_message_;
         state = state_error;
         if(driver != 0)
            driver->send("\xFF\xFF",2);
         timer_id = timer->arm(this,1000);
      } // on_error
   };
};

   

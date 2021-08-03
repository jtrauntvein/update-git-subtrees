/* Csi.OsLoader.SimpleSRecord.cpp

   Copyright (C) 2007, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 08 May 2007
   Last Change: Friday 29 April 2016
   Last Commit: $Date: 2016-05-03 16:18:03 -0600 (Tue, 03 May 2016) $
   Last Changed by: $Author: jon $

*/

#pragma warning(disable: 4996)
#pragma hdrstop               // stop creation of precompiled header
#include "Csi.OsLoader.SimpleSRecord.h"
#include "Csi.OsException.h"
#include "coratools.strings.h"
#include "Csi.Utils.h"
#include "boost/format.hpp"
#include <sstream>


namespace Csi
{
   namespace OsLoader
   {
      using namespace SimpleSRecordStrings;

      
      namespace
      {
         ////////////////////////////////////////////////////////////
         // srecord_checksum
         ////////////////////////////////////////////////////////////
         uint4 srecord_checksum(
            char const *s,
            uint4 slen)
         {
            uint4 rtn = 0;
            uint4 start = (s[0] == 'S' ? 2 : 0);
            char temp[3];
            for(uint4 i = start; i + 1 < slen; i += 2)
            {
               uint4 value;
               temp[0] = s[i];
               temp[1] = s[i + 1];
               temp[2] = 0;
               value = strtoul(temp,0,16);
               rtn = (rtn + value) & 0x0FF; 
            }
            rtn = (~rtn) & 0xFF;
            return rtn;
         } // srecord_checksum

         
         ////////////////////////////////////////////////////////////
         // is_valid_srecord
         ////////////////////////////////////////////////////////////
         bool is_valid_srecord(StrAsc const &line)
         {
            bool rtn = true;
            if(line.length() > 6 && line[0] == 'S')
            {
               // calculate the checksum for all but the first two (type field) and last two bytes
               // (checksum field) of the record
               uint4 calc_checksum = srecord_checksum(line.c_str(), (uint4)line.length() - 2);
               uint4 stored_checksum = strtoul(
                  line.c_str() + line.length() - 2, 0, 16);
               if(calc_checksum != stored_checksum)
                  rtn = false;
            }
            else
               rtn = false;
            return rtn;
         } // is_valid_srecord
      };

      
      ////////////////////////////////////////////////////////////
      // class SimpleSRecord definitions
      ////////////////////////////////////////////////////////////
      SimpleSRecord::SimpleSRecord(timer_handle &timer_):
         timer(timer_),
         file_size_bytes(0),
         bytes_sent(0),
         state(state_standby),
         get_ack_id(0) 
      { }

      
      SimpleSRecord::~SimpleSRecord()
      {
         if(get_ack_id != 0)
            timer->disarm(get_ack_id);
         timer.clear();
      } // destructor

      
      void SimpleSRecord::open_and_validate(
         char const *os_file_name)
      {
         // check the state and open the input
         if(state != state_standby)
            throw MsgExcept(my_strings[strid_invalid_open_state].c_str()); 
         FILE *in = Csi::open_file(os_file_name,"rb");
         if(in == 0)
            throw OsException(my_strings[strid_unable_to_open_file].c_str());
         
         try
         {
            // we will read the file one character at a time.  When an end-of-line is encountered,
            // we will validate the line as a record and insert it into the list of records to
            // send.
            StrAsc line;
            int next_ch;

            file_size_bytes = 0;
            while((next_ch = fgetc(in)) != EOF)
            {
               char ch = static_cast<char>(next_ch);
               switch(ch)
               {
               case '\n':
                  if(is_valid_srecord(line))
                  {
                     records.push_back(line);
                     file_size_bytes += (uint4)line.length() + 2;
                     line.cut(0);
                  }
                  else
                  {
                     std::ostringstream msg;
                     msg << boost::format(my_strings[strid_invalid_srecord].c_str()) % (records.size() + 1);
                     throw MsgExcept(msg.str().c_str());
                  }
                  break;

               case '\r':
                  break;        // ignore

               default:
                  if(ch == 'S' || isxdigit(ch))
                     line += ch;
                  break;
               }
            }
            fclose(in);
            in = 0;

            // the records buffer must not be empty and must also be properly terminated
            if(records.empty())
               throw MsgExcept(my_strings[strid_no_valid_srecords].c_str());
            state = state_open;
            this->os_file_name = os_file_name;
         }
         catch(std::exception &)
         {
            if(in != 0)
               fclose(in);
            throw;
         }
      } // open_and_validate

      
      void SimpleSRecord::start_send(
         driver_handle driver,
         EventReceiver *client)
      {
         if(state != state_open)
            throw MsgExcept(my_strings[strid_invalid_start_state].c_str());
         OsLoaderBase::start_send(driver,client);
         state = state_waiting_for_first;
         first_base = counter(0);
         retry_count = records_sent = 0;
         current_record = records.begin();
         send_next_srecord();
      } // start_send

      
      void SimpleSRecord::cancel_send()
      {
         if(state > state_standby && state != state_error)
            on_error(my_strings[strid_cancelled].c_str());
      } // cancel_send

      
      void SimpleSRecord::on_receive(
         OsLoaderDriver *driver,
         void const *buff,
         uint4 buff_len)
      {
         receive_buffer.push(buff,buff_len);
         switch(state)
         {
         case state_waiting_for_first:
         case state_sending:
            check_for_ack();
            break;
         }
      } // on_receive

      
      void SimpleSRecord::on_complete(
         char const *message,
         bool succeeded)
      {
         records.clear();
         state = state_standby;
         if(get_ack_id != 0)
            timer->disarm(get_ack_id);
         OsLoaderBase::on_complete(message,succeeded);
      } // on_complete

      
      void SimpleSRecord::onOneShotFired(uint4 event_id)
      {
         if(event_id == get_ack_id)
         {
            get_ack_id = 0;
            if(state == state_waiting_for_first)
            {
               if(counter(first_base) < 60000)
                  send_next_srecord();
               else
                  on_error(my_strings[strid_no_srecord_ack].c_str());
            }
            else if(++retry_count < 5)
               send_next_srecord();
            else
               on_error(my_strings[strid_no_srecord_ack].c_str()); 
         }
      } // onOneShotFired


      void SimpleSRecord::send_status(char const *msg)
      { on_status(msg,bytes_sent,file_size_bytes); }

      
      void SimpleSRecord::on_error(char const *message)
      { on_complete(message,false); }

      
      void SimpleSRecord::send_next_srecord()
      {
         assert(current_record != records.end());
         driver->send(current_record->c_str(), (uint4)current_record->length());
         driver->send("\r\n",2);
         if(get_ack_id == 0)
            get_ack_id = timer->arm(this, 500);
         else
            timer->reset(get_ack_id);
      } // send_next_srecord


      void SimpleSRecord::check_for_ack()
      {
         byte next_event;
         while(receive_buffer.pop(&next_event,1))
         {
            // ignore anything but valid events
            enum event_types {
               srecord_ack = 0x06,
               srecord_nak = 0x15
            };
            if(next_event == srecord_ack || next_event == srecord_nak)
            {
               std::ostringstream msg;

               state = state_sending;
               switch(next_event)
               {
               case srecord_ack:
                  ++records_sent;
                  bytes_sent += (uint4)current_record->length() + 2;
                  retry_count = 0;
                  ++current_record;
                  if(current_record != records.end())
                  {
                     msg << boost::format(my_strings[strid_srecord_ack].c_str()) %
                        records_sent % records.size();
                     send_status(msg.str().c_str());
                     send_next_srecord();
                  }
                  else
                  {
                     msg << boost::format(my_strings[strid_complete].c_str()) % os_file_name;
                     on_complete(msg.str().c_str(),true);
                  }
                  break;
                  
               case srecord_nak:
                  if(++retry_count < 4)
                  {
                     msg << boost::format(my_strings[strid_retrying].c_str()) % records_sent % retry_count;
                     send_status(msg.str().c_str());
                     send_next_srecord();
                  }
                  else
                  {
                     msg << boost::format(my_strings[strid_too_many_retries].c_str()) % records_sent;
                     on_error(msg.str().c_str());
                  }
                  break;
               }
               receive_buffer.pop(receive_buffer.size());
            }
         }
      } // check_for_ack
   };
};


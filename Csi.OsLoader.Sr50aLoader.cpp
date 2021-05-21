/* Csi.OsLoader.Sr50aLoader.cpp

   Copyright (C) 2016, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Friday 18 March 2016
   Last Change: Friday 29 April 2016
   Last Commit: $Date: 2016-05-03 16:18:03 -0600 (Tue, 03 May 2016) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.OsLoader.Sr50aLoader.h"
#include "Csi.OsLoader.SRecordUtils.h"
#include "Csi.utils.h"
#include "coratools.strings.h"
#include <boost/format.hpp>
#include <iomanip>


namespace Csi
{
   namespace OsLoader
   {
      using namespace LargeSRecordLoaderStrings;


      Sr50aLoader::Sr50aLoader(timer_handle &timer_):
         timer(timer_),
         between_lines_id(0),
         waiting_for_xon(false)
      {
         if(timer == 0)
            timer.bind(new OneShot);
      } // constructor


      Sr50aLoader::~Sr50aLoader()
      {
         if(between_lines_id)
            timer->disarm(between_lines_id);
         timer.clear();
      } // destructor


      void Sr50aLoader::open_and_validate(char const *file_name)
      {
         // open the input file.
         FILE *input = Csi::open_file(file_name, "rb");
         if(input == 0)
            throw OsException(my_strings[strid_unable_to_open_file].c_str());
         os_file_name = file_name;
         file_sig = 0xaaaa;
         
         // we will now read the file one line at a time
         try
         {
            StrAsc line;
            int next_ch;
            while((next_ch = fgetc(input)) != EOF)
            {
               char ch = static_cast<char>(next_ch);
               file_sig = calcSigFor(&ch, 1, file_sig);
               if(ch == '\n')
               {
                  if(is_valid_srecord(line.c_str(), (uint4)line.length()))
                  {
                     line.append("\r\n");
                     lines.push_back(line);
                     line.cut(0);
                  }
                  else
                  {
                     message.str("");
                     message << my_strings[strid_invalid_srecord] << (lines.size() + 1);
                     throw std::invalid_argument(message.c_str());
                  }
               }
               else if(ch == ':' || std::isxdigit(ch))
                  line.append(ch);
               else if(!std::isspace(ch))
               {
                  message.str("");
                  message << my_strings[strid_invalid_srecord] << (lines.size() + 1);
                  throw std::invalid_argument(message.c_str());
               }
            }

            // if the last line didn't end with a line feed, we will need to process any remnant
            if(line.length() > 0)
            {
               if(line.length() > 3 && line[0] == ':')
               {
                  line.append("\r\n");
                  lines.push_back(line);
               }
               else
               {
                  message.str("");
                  message << my_strings[strid_invalid_srecord] << (lines.size() + 1);
                  throw std::invalid_argument(message.c_str());
               }
            }
         }
         catch(std::exception &)
         {
            if(input)
               fclose(input);
            throw;
         }
         if(input)
            fclose(input);
      } // open_and_validate


      void Sr50aLoader::start_send(driver_handle driver, EventReceiver *client)
      {
         OsLoaderBase::start_send(driver, client);
         current_line = 0;
         waiting_for_xon = false;
         send_next_line(false);
      } // start_send


      void Sr50aLoader::cancel_send()
      {
         on_complete(my_strings[strid_cancelled].c_str(), false);
      } // cancel_send


      namespace
      {
         char const xon_char = 0x11;
         char const xoff_char = 0x13;
      };


      void Sr50aLoader::on_receive(OsLoaderDriver *driver, void const *buff_, uint4 buff_len)
      {
         // we will evaluate everything that has been received to determine whether the net line
         // should be sent.
         byte const *buff(static_cast<byte const *>(buff_));
         bool needs_delay(between_lines_id != 0);
         for(uint4 i = 0; i < buff_len; ++i)
         {
            if(buff[i] == xon_char)
               waiting_for_xon = false;
            else if(buff[i] == xoff_char)
               waiting_for_xon = true;
         }

         // we can now set to timer based upon the state of xon/xoff.  If waiting for xon, we will
         // set the timer for up to 1 second.
         timer->disarm(between_lines_id);
         if(!waiting_for_xon)
            send_next_line(true);
         else
            between_lines_id = timer->arm(this, 500);
      } // on_receive


      void Sr50aLoader::onOneShotFired(uint4 id)
      {
         if(id == between_lines_id)
         {
            between_lines_id = 0;
            send_next_line(true);
         }
      } // onOneShotFired


      void Sr50aLoader::on_complete(char const *message, bool succeeded, bool display_elapsed_time)
      {
         if(between_lines_id)
            timer->disarm(between_lines_id);
         OsLoaderBase::on_complete(message, succeeded, display_elapsed_time);
      } // on_complete
      

      void Sr50aLoader::send_next_line(bool advance_first)
      {
         if(advance_first)
         {
            message.str("");
            message << boost::format(my_strings[strid_srecord_ack].c_str()) % (current_line + 1);
            on_status(message.c_str(), current_line, (uint4)lines.size());
            ++current_line;
         }
         if(current_line < lines.size())
         {
            StrAsc const &line(lines[current_line]);
            driver->send(line.c_str(), (uint4)line.length());
            between_lines_id = timer->arm(this, 100);
         }
         else
         {
            Csi::OStrAscStream sig;
            sig << file_sig << " (0x" << std::hex << std::setfill('0') << std::setw(4) << file_sig << ")";
            message.str("");
            message << boost::format(my_strings[strid_sent].c_str()) % os_file_name % sig.str();
            on_complete(message.c_str(), true);
         }
      } // send_next_line
   };
};


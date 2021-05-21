/* Csi.NmeaParser.cpp

   Copyright (C) 2021, 2021 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Monday 11 January 2021
   Last Change: Monday 12 April 2021
   Last Commit: $Date: 2021-04-12 16:49:04 -0600 (Mon, 12 Apr 2021) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.NmeaParser.h"
#include "Csi.BuffStream.h"
#include "Csi.FloatUtils.h"
#include <cstdlib>


namespace Csi
{
   namespace
   {
      /*
       *@return Returns the calculated checksum of a block of data by xor-ing bytes within the buffer.
       *
       * @param buff Specifies the start of the buffer.
       *
       * @param buff_len Specifies the number of bytes.
       */
      uint4 nmea_checksum(void const *buff, size_t buff_len)
      {
         uint4 rtn(0);
         byte const *b((byte const *)buff);
         for(size_t i = 0; i < buff_len; ++i)
            rtn = (rtn ^ b[i]) & 0xff;
         return rtn;
      }

      /**
       * @return Returns a coordinate as decimal degrees.
       *
       * @param coord Specifies the coordinate in degrees, minutes, and fractions of minutes.
       *
       * @param dir Specifies the position relative to the origin.  Values of "W" and "S" will be
       * interpreted as specifying a negative sign to the return value.
       */
      double nmea_coord(StrAsc const &coord, StrAsc const &dir)
      {
         double rtn(std::numeric_limits<double>::quiet_NaN());
         if(coord.length() && dir.length())
         {
            double sign(dir == "W" || dir == "S" ? -1.0 : 1.0);
            size_t decimal_pos(coord.find("."));
            StrAsc degrees_str, minutes_str;
            double degrees, minutes;
            coord.sub(minutes_str, decimal_pos - 2, coord.length());
            coord.sub(degrees_str, 0, decimal_pos - 2);
            degrees = atof(degrees_str.c_str());
            minutes = atof(minutes_str.c_str());
            rtn = sign * (degrees + (minutes / 60));
         }
         return rtn;
      } // nmea_coord

      /**
       * @return Returns a double value for the specified NMEA string.  Returns NaN if the string is
       * empty.
       */
      double nmea_value(StrAsc const &s)
      {
         double rtn(std::numeric_limits<double>::quiet_NaN());
         if(s.length())
            rtn = atof(s.c_str());
         return rtn;
      } // nmea_value

      /**
       * @return Returns the time stamp represented by the specified string.  The date portion of
       * the time will be inferred from the host UTC clock.
       *
       * @param value Specifies the string to convert.
       */
      LgrDate nmea_stamp(StrAsc const &value)
      {
         LgrDate rtn;
         if(value.length() >= 6)
         {
            StrAsc hr_str, mn_str, sec_str;
            uint4 hours, minutes;
            double seconds, frac_seconds;
            
            value.sub(hr_str, 0, 2);
            value.sub(mn_str, 2, 2);
            value.sub(sec_str, 4, value.length());
            hours = std::strtoul(hr_str.c_str(), 0, 10);
            minutes = std::strtoul(mn_str.c_str(), 0, 10);
            frac_seconds = std::modf(atof(sec_str.c_str()), &seconds);
            seconds = std::atof(sec_str.c_str());
            rtn = LgrDate::gmt();
            rtn.setTime(
               hours,
               minutes,
               static_cast<uint4>(seconds),
               static_cast<uint4>(frac_seconds * LgrDate::nsecPerSec));
         }
         return rtn;
      } // nmea_stamp
   };


   NmeaSatellite::NmeaSatellite(CsvRec const &sentence, size_t offset):
      id(std::numeric_limits<double>::quiet_NaN()),
      elevation(std::numeric_limits<double>::quiet_NaN()),
      azimuth(std::numeric_limits<double>::quiet_NaN()),
      snr(std::numeric_limits<double>::quiet_NaN())
   {
      id = nmea_value(sentence.at(offset));
      elevation = nmea_value(sentence.at(offset + 1));
      azimuth = nmea_value(sentence.at(offset + 2));
      snr = nmea_value(sentence.at(offset + 3));
   } // constructor
   

   NmeaParser::NmeaParser(NmeaParserClient *client_):
      client(client_),
      state(state_between_sentences),
      latitude(std::numeric_limits<double>::quiet_NaN()),
      longitude(std::numeric_limits<double>::quiet_NaN()),
      fix_quality(fix_none),
      horizontal_dilution(std::numeric_limits<double>::quiet_NaN()),
      antenna_altitude(std::numeric_limits<double>::quiet_NaN())
   {
      if(!NmeaParserClient::is_valid_instance(client))
         throw std::invalid_argument("invalid client pointer");
   } // constructor

   NmeaParser::~NmeaParser()
   {
      client = 0;
   } // destructor

   void NmeaParser::on_data(void const *data, uint4 data_len)
   {
      bool expect_more(true);
      if(data_len)
         buffer.push(data, data_len);
      while(expect_more)
         expect_more = parse_next_sentence();
   } // on_data

   void NmeaParser::process_sentence()
   {
      try
      {
         IBuffStream temp(sentence.c_str() + 1, checksum_pos - 2);
         split_sentence.read(temp);
         split_sentence.at(0).sub(talker_id, 0, 2);
         split_sentence.at(0).sub(sentence_type, 2, 3);
         position_updated = satellites_updated = false;
         if(sentence_type == "GGA")
            process_gga();
         else if(sentence_type == "GLL")
            process_gll();
         else if(sentence_type == "GNS")
            process_gns();
         else if(sentence_type == "GSV")
            process_gsv();
         if(NmeaParserClient::is_valid_instance(client))
            client->on_sentence(this, sentence);
      }
      catch(std::exception &e)
      {
         if(NmeaParserClient::is_valid_instance(client))
            client->on_failure(this, e.what());
      }
   } // process_sentence

   void NmeaParser::process_gga()
   {
      double fix_value(nmea_value(split_sentence.at(6)));
      stamp = nmea_stamp(split_sentence.at(1));
      latitude = nmea_coord(split_sentence.at(2), split_sentence.at(3));
      longitude = nmea_coord(split_sentence.at(4), split_sentence.at(5));
      horizontal_dilution = nmea_value(split_sentence.at(8));
      antenna_altitude = nmea_value(split_sentence.at(9));
      if(is_finite(fix_value) && fix_value >= 1 && fix_value <= 8)
         fix_quality = static_cast<fix_quality_type>(fix_value);
      position_updated = (is_finite(latitude) && is_finite(longitude));
   } // process_gga

   void NmeaParser::process_gll()
   {
      latitude = nmea_coord(split_sentence.at(1), split_sentence.at(2));
      longitude = nmea_coord(split_sentence.at(3), split_sentence.at(4));
      stamp = nmea_stamp(split_sentence.at(5));
      position_updated = (is_finite(latitude) && is_finite(longitude));
   } // process_gll

   void NmeaParser::process_gns()
   {
      stamp = nmea_stamp(split_sentence.at(1));
      latitude = nmea_coord(split_sentence.at(2), split_sentence.at(3));
      longitude = nmea_coord(split_sentence.at(4), split_sentence.at(5));
      horizontal_dilution = nmea_value(split_sentence.at(8));
      antenna_altitude = nmea_value(split_sentence.at(9));
      position_updated = (is_finite(latitude) && is_finite(longitude));
   } // process_gns

   void NmeaParser::process_gsv()
   {
      uint4 total_sentences(strtoul(split_sentence.at(1).c_str(), 0, 10));
      uint4 sentence_no(strtoul(split_sentence.at(2).c_str(), 0, 10));
      if(sentence_no == 1)
         satellites.clear();
      for(size_t i = 4; i < split_sentence.size(); i+= 4)
         satellites.push_back(NmeaSatellite(split_sentence, i));
      if(sentence_no == total_sentences)
         satellites_updated = true;
   } // process_gsv

   bool NmeaParser::parse_next_sentence()
   {
      bool rtn(false);
      if(state == state_between_sentences)
      {
         uint4 dollar_pos(buffer.find("$", 1));
         if(dollar_pos < buffer.size())
         {
            // we will remove all the input before the sentence.
            buffer.pop(dollar_pos);
            sentence_time = LgrDate::gmt();
            sentence.cut(0);
            state = state_in_sentence;
            rtn = (buffer.size() > 0);
         }
      }
      else if(state == state_in_sentence)
      {
         while(state == state_in_sentence && buffer.size() > 0)
         {
            byte ch;
            buffer.pop(&ch, 1);
            sentence.append((char)ch);
            if(ch == '*')
            {
               checksum_pos = sentence.length();
               state = state_in_checksum;
            }
         }
         rtn = (buffer.size() > 0);
      }
      else if(state == state_in_checksum)
      {
         while(state == state_in_checksum && buffer.size() > 0)
         {
            byte ch;
            buffer.pop(&ch, 1);
            if(ch == '\r')
            {
               StrAsc checksum_buff;
               uint4 reported_checksum;
               uint4 calc_checksum;
               
               sentence.sub(checksum_buff, checksum_pos, sentence.length());
               reported_checksum = strtoul(checksum_buff.c_str(), 0, 16);
               calc_checksum = nmea_checksum(sentence.c_str() + 1, checksum_pos - 2);
               sentence.append("\r\n");
               state = state_between_sentences;
               rtn = (buffer.size() > 0);
               if(reported_checksum == calc_checksum)
                  process_sentence();
               else
               {
                  if(NmeaParserClient::is_valid_instance(client))
                     client->on_failure(this, "invalid sentence checksum");
               }
            }
            else if(std::isxdigit(ch))
               sentence.append((char)ch);
            else
            {
               if(NmeaParserClient::is_valid_instance(client))
                  client->on_failure(this, "sentence framing error");
               state = state_between_sentences;
               rtn = (buffer.size() > 0);
            }
         }
      }
      return rtn;
   } // parse_next_sentence
};

/* Cora.Device.DeviceSettingTypes.cpp

   Copyright (C) 2000, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 20 July 2000
   Last Change: Wednesday 19 February 2020
   Last Commit: $Date: 2020-02-19 13:39:29 -0600 (Wed, 19 Feb 2020) $ 
   Commited by: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header

#include "Cora.Device.DeviceSettingTypes.h"
#include "Csi.Messaging.Message.h"
#include "Csi.CommandLine.h"
#include "Csi.Uri.h"
#include <algorithm>
#include <iostream>


namespace Cora
{
   namespace Device
   {
      bool ClockCheckSchedule::read(Csi::Messaging::Message *msg)
      {
         int8 temp;
         bool rtn;
         if(msg->readBool(schedule_on) &&
            msg->readInt8(temp) &&
            msg->readUInt4(interval) &&
            msg->readUInt4(max_deviation))
         {
            base = temp;
            rtn = true;
         }
         else
            rtn = false;
         return rtn;
      } // read


      bool ClockCheckSchedule::read(char const *s)
      {
         Csi::CommandLine cmd;
         cmd.parse_command_line(s);
         StrAsc temp;
         bool rtn = true;

         try
         {
            if(cmd.get_argument(temp,0))
            {
               if(temp == "true" || temp == "1")
                  schedule_on = true;
               else if(temp == "false" || temp == "0")
                  schedule_on = false;
               else
                  throw Csi::MsgExcept("Syntax error interpreting scheduled-on");
            }
            else
               throw Csi::MsgExcept("Expected the scheduled-on component");
            if(cmd.get_argument(temp,1))
            {
               base = Csi::LgrDate::fromStr(temp.c_str());
            }
            else
               throw Csi::MsgExcept("Expected the schedule base time component");
            if(cmd.get_argument(temp,2))
            {
               interval = strtoul(temp.c_str(),0,10);
            }
            else
               throw Csi::MsgExcept("Expected the interval component");
            if(cmd.get_argument(temp,3))
            {
               max_deviation = strtoul(temp.c_str(),0,10);
            }
            else
               throw Csi::MsgExcept("Expected the maximum deviation component");
         }
         catch(std::exception &)
         { rtn = false; }
         return rtn;
      } // read


      void ClockCheckSchedule::write(Csi::Messaging::Message *out) const
      {
         out->addBool(schedule_on);
         out->addInt8(base.get_nanoSec());
         out->addUInt4(interval);
         out->addUInt4(max_deviation);
      } // write


      void ClockCheckSchedule::format(std::ostream &out) const
      {
         out << schedule_on << ' ';
         base.format(out,"{%Y%m%d %H:%M:%S.%3} ");
         out << interval << ' ' << max_deviation;
      } // write


      bool CollectSchedule::read(Csi::Messaging::Message *in)
      {
         bool rtn;
         int8 temp;
         if(in->readBool(schedule_on) &&
            in->readInt8(temp) &&
            in->readUInt4(collect_interval) &&
            in->readUInt4(primary_collect_interval) &&
            in->readUInt4(primary_max_retry_count) &&
            in->readUInt4(secondary_collect_interval))
         {
            rtn = true;
            base = temp;
         }
         else
            rtn = false;
         return rtn;
      } // read


      bool CollectSchedule::read(char const *s)
      {
         Csi::CommandLine cmd;
         StrAsc temp;
         bool rtn = true;

         try
         {
            cmd.parse_command_line(s);
            if(cmd.get_argument(temp,0))
            {
               if(temp == "true" || temp == "1")
                  schedule_on = true;
               else if(temp == "false" || temp == "0")
                  schedule_on = false;
               else
                  throw Csi::MsgExcept("Syntax error for schedule_on");
            }
            else
               throw Csi::MsgExcept("Expecting the schedule-on component");
            if(cmd.get_argument(temp,1))
               base = Csi::LgrDate::fromStr(temp.c_str());
            if(cmd.get_argument(temp,2))
               collect_interval = strtoul(temp.c_str(),0,10);
            else
               throw Csi::MsgExcept("Expecting the collect-interval component");
            if(cmd.get_argument(temp,3))
               primary_collect_interval = strtoul(temp.c_str(),0,10);
            else
               throw Csi::MsgExcept("Expecting the primary-collect-interval component");
            if(cmd.get_argument(temp,4))
               primary_max_retry_count = strtoul(temp.c_str(),0,10);
            else
               throw Csi::MsgExcept("Expecting the primary-max-retry-count component");
            if(cmd.get_argument(temp,5))
               secondary_collect_interval = strtoul(temp.c_str(),0,10);
            else
               throw Csi::MsgExcept("Expecting the secondary-collect-interval component");
         }
         catch(std::exception &)
         { rtn = false; }
         return rtn;
      } // read

      void CollectSchedule::write(Csi::Messaging::Message *out) const
      {
         out->addBool(schedule_on);
         out->addInt8(base.get_nanoSec());
         out->addUInt4(collect_interval);
         out->addUInt4(primary_collect_interval);
         out->addUInt4(primary_max_retry_count);
         out->addUInt4(secondary_collect_interval);
      } // write


      void CollectSchedule::format(std::ostream &out) const
      {
         out << schedule_on << ' ';
         base.format(out,"{%Y%m%d %H:%M:%S.%3} ");
         out << collect_interval << ' ' << primary_collect_interval << ' '
             << primary_max_retry_count << ' ' << secondary_collect_interval;
      } // write


      bool LoggerProgramInfo::read(Csi::Messaging::Message *in)
      {
         int8 temp;
         bool rtn;
         if(in->readUInt4(resp_code) &&
            in->readStr(program_name) &&
            in->readInt8(temp) &&
            in->readStr(result_text))
         {
            rtn = true;
            when_compiled = temp;
         }
         else
            rtn = false;
         return rtn;
      } // read


      bool LoggerProgramInfo::read(char const *s)
      {
         Csi::CommandLine cmd;
         StrAsc temp;
         bool rtn = true;

         try
         {
            cmd.parse_command_line(s);
            if(cmd.get_argument(temp,0))
               resp_code = strtoul(temp.c_str(),0,10);
            else
               throw Csi::MsgExcept("Expected the resp-code component");
            if(!cmd.get_argument(program_name,1))
               throw Csi::MsgExcept("Expected the program-name component");
            if(cmd.get_argument(temp,2))
               when_compiled = Csi::LgrDate::fromStr(temp.c_str());
            else
               throw Csi::MsgExcept("Expected the when_compiled component");
            if(!cmd.get_argument(result_text,3))
               throw Csi::MsgExcept("Expected the result-text component");
         }
         catch(std::exception &)
         { rtn = false; }
         return rtn;
      } // read


      void LoggerProgramInfo::write(Csi::Messaging::Message *msg) const
      {
         msg->addUInt4(resp_code);
         msg->addStr(program_name.c_str());
         msg->addInt8(when_compiled.get_nanoSec());
         msg->addStr(result_text.c_str());
      } // write


      void LoggerProgramInfo::format(std::ostream &out) const
      {
         out << resp_code << " \"" << program_name << "\" ";
         when_compiled.format(out,"{%Y%m%d %H:%M:%S} ");
         out << '\"' << result_text << '\"';
      } // write


      bool LowLevelPollSchedule::read(Csi::Messaging::Message *msg)
      {
         return (msg->readUInt4(interval) &&
                 msg->readInt4(router_offset) &&
                 msg->readInt4(computer_offset));
      } // read


      bool LowLevelPollSchedule::read(char const *s)
      {
         Csi::CommandLine cmd;
         StrAsc temp;
         bool rtn = true;

         try
         {
            cmd.parse_command_line(s);
            if(cmd.get_argument(temp,0))
               interval = strtoul(temp.c_str(),0,10);
            else
               throw Csi::MsgExcept("Expected the interval component");
            if(cmd.get_argument(temp,1))
               router_offset = strtoul(temp.c_str(),0,10);
            else
               throw Csi::MsgExcept("Expected the router-offset component");
            if(cmd.get_argument(temp,2))
               computer_offset = strtoul(temp.c_str(),0,10);
            else
               throw Csi::MsgExcept("Expected the computer-offset component");
         }
         catch(std::exception &)
         { rtn = false; }
         return rtn;
      } // read


      void LowLevelPollSchedule::write(Csi::Messaging::Message *out) const
      {
         out->addUInt4(interval);
         out->addInt4(router_offset);
         out->addInt4(computer_offset);
      } // write


      void LowLevelPollSchedule::format(std::ostream &out) const
      {
         out << interval << ' ' << router_offset << ' ' << computer_offset;
      } // write


      bool DialStringList::read(Csi::Messaging::Message *msg)
      {
         uint4 count;
         bool rtn = true;
         uint4 delay;
         StrAsc contents;
            
         rtn = msg->readUInt4(count);
         for(uint4 i = 0; i < count && rtn; i++)
         {
            rtn = msg->readUInt4(delay) && msg->readStr(contents);
            if(rtn)
               records.push_back(Record(delay,contents));
         }
         return rtn;
      } // read


      bool DialStringList::read(char const *s)
      {
         Csi::CommandLine setting;
         uint4 count;
         StrAsc temp;
         bool rtn = true;

         try
         {
            setting.parse_command_line(s);
            if(setting.get_argument(temp,0))
            {
               count = strtoul(temp.c_str(),0,10);
               for(uint4 i = 0; i < count; i++)
               {
                  Csi::CommandLine component;
                  uint4 delay;

                  if(!setting.get_argument(temp,i + 1))
                     throw Csi::MsgExcept("Insufficient records specified");
                  component.parse_command_line(temp.c_str());
                  if(component.get_argument(temp,0))
                     delay = strtoul(temp.c_str(),0,10);
                  else
                     throw Csi::MsgExcept("Expected the delay component");
                  if(component.get_argument(temp,1))
                     records.push_back(Record(delay,temp));
                  else
                     throw Csi::MsgExcept("Expected the contents component");  
               }
            }
            else
               throw Csi::MsgExcept("Expected the number of records to follow");
         }
         catch(std::exception &)
         { rtn = false; }
         return rtn;
      } // read


      void DialStringList::write(Csi::Messaging::Message *out) const
      {
         out->addUInt4((uint4)records.size());
         for(records_type::const_iterator ri = records.begin();
             ri != records.end();
             ++ri)
         {
            out->addUInt4(ri->delay);
            out->addStr(ri->contents.c_str());
         }
      } // write


      void DialStringList::format(std::ostream &out) const
      {
         out << records.size() << std::endl;
         for(records_type::const_iterator ri = records.begin();
             ri != records.end();
             ++ri)
            out << '{' << ri->delay << " {" << ri->contents << "}}" << std::endl;
      } // write


      bool SettingInlocIds::read(Csi::Messaging::Message *msg)
      {
         uint4 count;
         bool rtn;
         if(msg->readUInt4(count))
         {
            uint2 logger_id;
            StrUni field_name;
            
            rtn = true;
            for(uint4 i = 0; rtn && i < count; ++i)
            {
               if(msg->readUInt2(logger_id) && msg->readWStr(field_name))
                  records.push_back(record_type(logger_id,field_name));
               else
                  rtn = false;
            }
         }
         else
            rtn = false;
         return rtn;
      } // read


      bool SettingInlocIds::read(char const *s)
      {
         Csi::CommandLine cmd;
         StrAsc temp;
         bool rtn = true;

         try
         {
            cmd.parse_command_line(s);
            if(cmd.get_argument(temp,0))
            {
               uint4 count = strtoul(temp.c_str(),0,10);
               for(uint4 i = 0; i < count; ++i)
               {
                  if(cmd.get_argument(temp,i + 1))
                  {
                     Csi::CommandLine record;
                     record.parse_command_line(temp.c_str());
                     if(record.get_argument(temp,0))
                     {
                        uint2 logger_id = static_cast<uint2>(strtoul(temp.c_str(),0,10));
                        if(record.get_argument(temp,1))
                        {
                           StrUni field_name(temp.c_str());
                           records.push_back(record_type(logger_id,field_name));
                        }
                        else
                           throw Csi::MsgExcept("Expected a field name");
                     }
                     else
                        throw Csi::MsgExcept("Expected an input location identifier");
                  }
                  else
                     throw Csi::MsgExcept("Expected a location record");
               }
            }
            else
               throw Csi::MsgExcept("Expected the count of location records");
         }
         catch(std::exception &)
         { rtn = false; }
         return rtn;
      } // read


      void SettingInlocIds::write(Csi::Messaging::Message *msg) const
      {
         msg->addUInt4((uint4)records.size());
         for(records_type::const_iterator i = records.begin(); i != records.end(); ++i)
         {
            msg->addUInt2(i->logger_id);
            msg->addWStr(i->field_name.c_str());
         }
      } // write


      void SettingInlocIds::format(std::ostream &out) const
      {
         out << records.size() << std::endl;
         for(records_type::const_iterator i = records.begin(); i != records.end(); ++i)
            out << '{' << i->logger_id << " {" << i->field_name << "}}" << std::endl;
      } // write


      bool TapiCountryCode::read(Csi::Messaging::Message *message)
      { return message->readUInt4(country_code) && message->readStr(country_name); }


      bool TapiCountryCode::read(char const *s)
      {
         Csi::CommandLine command_line;
         StrAsc temp;
         bool rtn = true;

         try
         {
            command_line.parse_command_line(s);
            if(command_line.get_argument(temp,0))
            {
               country_code = strtoul(temp.c_str(),0,10);
               if(command_line.get_argument(temp,1))
                  country_name = temp;
               else
                  rtn = false;
            }
            else
               rtn = false;
         }
         catch(std::exception &)
         { rtn = false; }
         return rtn;
      } // read


      void TapiCountryCode::write(Csi::Messaging::Message *message) const
      {
         message->addUInt4(country_code);
         message->addStr(country_name);
      } // write


      void TapiCountryCode::format(std::ostream &out) const
      { out << country_code << " {" << country_name << "}" << std::endl; }


      void PakbusAddressesList::format(std::ostream &out) const
      {
         for(ranges_type::const_iterator ri = ranges.begin();
             ri != ranges.end();
             ++ri)
         {
            if(ri != ranges.begin())
               out << " ";
            if(ri->first == ri->second)
               out << ri->first;
            else
               out << "{" << ri->first << " " << ri->second << "}";
         }
      } // format


      bool PakbusAddressesList::read(char const *source)
      {
         bool rtn = true;
         ranges_type ranges_;
         
         try
         {
            Csi::CommandLine p1;
            StrAsc temp;
            
            p1.parse_command_line(source);
            for(Csi::CommandLine::args_iterator si = p1.begin();
                si != p1.end();
                ++si)
            {
               Csi::CommandLine p2;
               uint4 rbeg, rend;
               
               p2.parse_command_line(si->c_str());
               if(!p2.get_argument(temp,0))
                  throw std::invalid_argument("missing begin range");
               rbeg = strtoul(temp.c_str(),0,10);
               if(rbeg < 1 || rbeg > 4094)
                  throw std::invalid_argument("invalid range begin");
               if(p2.get_argument(temp,1))
                  rend = strtoul(temp.c_str(),0,10);
               else
                  rend = rbeg;
               if(rend < 1 || rend > 4094)
                  throw std::invalid_argument("invalid range end");
               if(p2.args_size() > 2)
                  throw std::invalid_argument("invalid range format");
               ranges_.add_range(rbeg,rend);
            }
            ranges = ranges_;
         }
         catch(std::exception &)
         { rtn = false; }
         return rtn;
      } // read (string)


      bool PakbusAddressesList::read(Csi::Messaging::Message *msg)
      {
         uint4 ranges_count; 
         bool rtn = msg->readUInt4(ranges_count);
         uint2 rbeg, rend;

         ranges.clear();
         for(uint4 i = 0; rtn && i < ranges_count; ++i)
         {
            rtn = msg->readUInt2(rbeg) && msg->readUInt2(rend);
            if(rtn)
               ranges.add_range(rbeg,rend); 
         }
         return rtn;
      } // read (message)


      void PakbusAddressesList::write(Csi::Messaging::Message *msg) const
      {
         msg->addUInt4((uint4)ranges.size());
         for(ranges_type::const_iterator ri = ranges.begin();
             ri != ranges.end();
             ++ri)
         {
            msg->addUInt2(static_cast<uint2>(ri->first));
            msg->addUInt2(static_cast<uint2>(ri->second));
         }
      } // write


      void SettingFileSynchControl::format(std::ostream &out) const
      {
         for(files_type::const_iterator fi = files.begin();
             fi != files.end();
             ++fi)
         {
            out << "{{" << fi->source_pattern << "} {"
                << fi->dest_dir << "} " << fi->force << "}\n";
         }
      } // format


      bool SettingFileSynchControl::read(char const *source)
      {
         bool rtn = true;

         try
         {
            Csi::CommandLine setting_parser;
            file file_info;
            StrAsc temp;
            
            setting_parser.parse_command_line(source);
            files.clear();
            for(Csi::CommandLine::args_iterator ai = setting_parser.begin();
                ai != setting_parser.end();
                ++ai)
            {
               Csi::CommandLine file_parser;
               file_parser.parse_command_line(ai->c_str());
               if(!file_parser.get_argument(file_info.source_pattern,0))
                  throw Csi::MsgExcept("expected the file source pattern");
               if(!file_parser.get_argument(file_info.dest_dir,1))
                  throw Csi::MsgExcept("expected the destination dir");
               if(!file_parser.get_argument(temp,2))
                  throw Csi::MsgExcept("expected the force parameter");
               if(temp == "true" || temp == "1")
                  file_info.force = true;
               else if(temp == "false" || temp == "0")
                  file_info.force = false;
               else
                  throw Csi::MsgExcept("Invalid force value");
               files.push_back(file_info); 
            }
         }
         catch(std::exception &)
         { rtn = false; }
         return rtn;
      } // read


      bool SettingFileSynchControl::read(Csi::Messaging::Message *in)
      {
         uint4 count;
         bool rtn = in->readUInt4(count);
         file file_info;

         files.clear();
         for(uint4 i = 0; rtn && i < count; ++i)
         {
            rtn = in->readStr(file_info.source_pattern) &&
               in->readStr(file_info.dest_dir) &&
               in->readBool(file_info.force);
            if(rtn)
               files.push_back(file_info);
         }
         return rtn;
      } // read


      void SettingFileSynchControl::write(Csi::Messaging::Message *out) const
      {
         out->addUInt4((uint4)files.size());
         for(files_type::const_iterator fi = files.begin();
             fi != files.end();
             ++fi)
         {
            out->addStr(fi->source_pattern);
            out->addStr(fi->dest_dir);
            out->addBool(fi->force);
         }
      } // write


      void FileSynchControlEx::format(std::ostream &out) const
      {
         for(files_type::const_iterator fi = files.begin(); fi != files.end(); ++fi)
         {
            file_type const &file = *fi;
            out << "{{" << file.source_pattern
                << "} {" << file.dest_dir << "}";
            if(file.force)
               out << " --force=true";
            if(file.max_files != 0xffffffff)
               out << " --max-files=" << file.max_files;
            if(file.record_if_skipped)
               out << " --record-if-skipped=true";
            out << "}\n";
         }
      } // format


      bool FileSynchControlEx::read(char const *s)
      {
         bool rtn = true;
         try
         {
            // we need to break the individual file records
            Csi::CommandLine parser;
            parser.parse_command_line(s);

            // we can now parse each file record
            Csi::CommandLine file_parser;
            StrAsc const force_name("force");
            StrAsc const max_files_name("max-files");
            StrAsc const record_if_skipped_name("record-if-skipped");
            file_parser.add_expected_option(force_name);
            file_parser.add_expected_option(max_files_name);
            file_parser.add_expected_option(record_if_skipped_name);
            files.clear();
            for(Csi::CommandLine::args_iterator fi = parser.begin(); fi != parser.end(); ++fi)
            {
               StrAsc temp;
               file_type file;
               
               file_parser.parse_command_line(fi->c_str());
               if(!file_parser.get_argument(file.source_pattern, 0))
                  throw Csi::MsgExcept("expected the source pattern");
               if(!file_parser.get_argument(file.dest_dir, 1))
                  throw Csi::MsgExcept("expected the destination dir");
               if(file_parser.get_option_value(force_name, temp))
               {
                  if(temp == "1" || temp == "true")
                     file.force = true;
                  else if(temp == "0" || temp == "false")
                     file.force = false;
                  else
                     throw Csi::MsgExcept("invalid force value");
               }
               if(file_parser.get_option_value(max_files_name, temp))
               {
                  file.max_files = strtoul(temp.c_str(), 0, 10);
                  if(file.max_files == 0)
                     throw Csi::MsgExcept("invalid max-files"); 
               }
               if(file_parser.get_option_value(record_if_skipped_name, temp))
               {
                  if(temp == "1" || temp == "true")
                     file.record_if_skipped = true;
                  else if(temp == "0" || temp == "false")
                     file.record_if_skipped = false;
                  else
                     throw Csi::MsgExcept("invalid record-if-skipped");
               }
               files.push_back(file);
            }
         }
         catch(std::exception &)
         { rtn = false; }
         return rtn;
      } // read


      bool FileSynchControlEx::read(Csi::Messaging::Message *in)
      {
         uint4 count;
         bool rtn = in->readUInt4(count);
         file_type file;

         files.clear();
         for(uint4 i = 0; rtn && i < count; ++i)
         {
            rtn = in->readStr(file.source_pattern) &&
               in->readStr(file.dest_dir) &&
               in->readBool(file.force) &&
               in->readUInt4(file.max_files) &&
               in->readBool(file.record_if_skipped);
            if(rtn)
               files.push_back(file);
         }
         return rtn;
      } // read


      void FileSynchControlEx::write(Csi::Messaging::Message *out) const
      {
         out->addUInt4((uint4)files.size());
         for(files_type::const_iterator fi = files.begin(); fi != files.end(); ++fi)
         {
            file_type const &file = *fi;
            out->addStr(file.source_pattern);
            out->addStr(file.dest_dir);
            out->addBool(file.force);
            out->addUInt4(file.max_files);
            out->addBool(file.record_if_skipped);
         }
      } // write


      void PakbusTcpOutAddresses::format(std::ostream &out) const
      {
         for(addresses_type::const_iterator ai = addresses.begin();
             ai != addresses.end();
             ++ai)
            out << "{" << ai->first << " {" << ai->second << "}}\n";
      } // format


      bool PakbusTcpOutAddresses::read(char const *s)
      {
         bool rtn = true;
         try
         {
            Csi::CommandLine setting_parser;
            uint2 pakbus_address;
            StrAsc temp;
            addresses_type temp_addresses;
            
            setting_parser.parse_command_line(s);
            for(Csi::CommandLine::args_iterator ai = setting_parser.begin();
                ai != setting_parser.end();
                ++ai)
            {
               Csi::CommandLine address_parser;
               address_parser.parse_command_line(ai->c_str());
               if(!address_parser.get_argument(temp, 0))
                  throw Csi::MsgExcept("expected the pakbus address field");
               pakbus_address = static_cast<uint2>(strtoul(temp.c_str(), 0, 10));
               if(pakbus_address >= 4095)
                  throw Csi::MsgExcept("Invalid PakBus address");
               if(!address_parser.get_argument(temp, 1))
                  throw Csi::MsgExcept("Expected the IP address");
               temp_addresses[pakbus_address] = temp;
            }
            addresses = temp_addresses;
         }
         catch(std::exception &)
         { rtn = false; }
         return rtn;
      } // read


      bool PakbusTcpOutAddresses::read(Csi::Messaging::Message *in)
      {
         uint4 count;
         bool rtn = in->readUInt4(count);
         if(rtn)
         {
            uint2 pakbus_address;
            StrAsc ip_address;
            addresses_type temp;
            
            for(uint4 i = 0; rtn && i < count; ++i)
            {
               rtn = in->readUInt2(pakbus_address) && in->readStr(ip_address);
               if(rtn)
                  temp[pakbus_address] = ip_address;
            }
            if(rtn)
               addresses = temp;
         }
         return rtn;
      } // read


      void PakbusTcpOutAddresses::write(Csi::Messaging::Message *out) const
      {
         out->addUInt4((uint4)addresses.size());
         for(addresses_type::const_iterator ai = addresses.begin();
             ai != addresses.end();
             ++ai)
         {
            out->addUInt2(ai->first);
            out->addStr(ai->second);
         }
      } // write


      bool ComPortIdSetting::read(char const *s)
      {
         value = s;
         return true;
      } // read

      
      bool PooledTerminalServersSetting::read(char const *s)
      {
         bool rtn(true);
         resources_type temp_resources;
         Csi::CommandLine resources_parser;
         try
         {
            resources_parser.parse_command_line(s);
            for(Csi::CommandLine::args_iterator ri = resources_parser.begin();
                rtn && ri != resources_parser.end();
                ++ri)
            {
               Csi::CommandLine resource_parser;
               StrAsc address;
               
               resource_parser.parse_command_line(ri->c_str());
               if(resource_parser.get_argument(address, 0))
               {
                  size_t port_start(address.rfind(":"));
                  if(port_start < address.length())
                  {
                     uint4 tcp_port(strtoul(address.c_str() + port_start + 1, 0, 10));
                     if(tcp_port > 0 && tcp_port <= 65535)
                     {
                        bool use_rfc2217(false);
                        StrAsc temp;
                        if(resource_parser.get_argument(temp, 1))
                        {
                           if(temp == "1" || temp == "true")
                              use_rfc2217 = true;
                           else if(temp == "0" || temp == "false")
                              use_rfc2217 = false;
                           else
                              rtn = false;
                        }
                        temp_resources.push_back(value_type(address, use_rfc2217));
                     }
                     else
                        rtn = false;
                  }
                  else
                     rtn = false;
               }
               else
                  rtn = false;
            }
         }
         catch(std::exception &)
         { rtn = false; }
         if(rtn)
            resources = temp_resources;
         return rtn;
      } // read


      void PooledTerminalServersSetting::format(std::ostream &out) const
      {
         for(const_iterator ri = begin(); ri != end(); ++ri)
         {
            out << "\r\n{ " << ri->first << " " << (ri->second ? "true" : "false") << " }";
         }
      } // format


      bool PooledTerminalServersSetting::read(Csi::Messaging::Message *in)
      {
         bool rtn(true);
         resources_type temp_resources;
         uint4 count;
         StrAsc address;
         bool use_rfc2217;

         rtn = in->readUInt4(count);
         while(rtn && count > 0)
         {
            if(in->readStr(address) && in->readBool(use_rfc2217))
            {
               temp_resources.push_back(value_type(address, use_rfc2217));
               --count;
            }
            else
               rtn = false;
         }
         if(rtn)
            resources = temp_resources;
         return rtn;
      } // read (message)


      void PooledTerminalServersSetting::write(Csi::Messaging::Message *out) const
      {
         out->addUInt4((uint4)resources.size());
         for(const_iterator ri = begin(); ri != end(); ++ri)
         {
            out->addStr(ri->first);
            out->addBool(ri->second);
         }
      } // write (message)


      bool PakbusWsServerUrl::validate(StrAsc const &val)
      {
         bool rtn(true);
         try
         {
            Csi::Uri uri(val);
            StrAsc const protocol(uri.get_protocol());
            if(protocol != "ws" && protocol != "wss")
               rtn = false;
         }
         catch(std::exception &)
         { rtn = false; }
         return rtn;
      } // validate
   };
};

/* Csi.DevConfig.Tx3xx.cpp

   Copyright (C) 2011, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Monday 03 October 2011
   Last Change: Friday 29 April 2016
   Last Commit: $Date: 2016-05-03 16:18:03 -0600 (Tue, 03 May 2016) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.DevConfig.Tx3xx.h"
#include "Csi.StrAscStream.h"
#include "Csi.BuffStream.h"
#include <algorithm>
#include <iterator>


namespace Csi
{
   namespace DevConfig
   {
      namespace
      {
         ////////////////////////////////////////////////////////////
         // enum setting_id_type
         ////////////////////////////////////////////////////////////
         enum setting_id_type
         {
            setid_nesid = 1,
            setid_timed_channel = 2,
            setid_timed_bit_rate =3,
            setid_timed_tx_interval = 4,
            setid_offset_time = 5,
            setid_timed_window_len = 6,
            setid_timed_window_align = 7,
            setid_timed_data_format = 8,
            setid_timed_empty_buffer_mode = 9,
            setid_timed_preamble = 10,
            setid_timed_interleaver = 11,
            setid_random_channel = 12,
            setid_random_bit_rate = 13,
            setid_randomising_interval = 14,
            setid_randomising_percent = 15,
            setid_random_repeat_count = 16,
            setid_random_data_format = 17,
            setid_random_message_counter = 18,
            setid_gps_fix_interval = 21,
            setid_serial_no = 22,
            setid_hardware_version = 23,
            setid_firmware_version = 24,
            setid_gps_version = 25,
            setid_time = 26,
            setid_last_gps_fix = 27,
            setid_latitude = 28,
            setid_longitude = 29,
            setid_altitude = 30,
            setid_gps_status = 31,
            setid_fail_safe_status = 32,
            setid_supply_voltage = 33,
            setid_temperature = 34,
            setid_timed_buffer_bytes_count = 35,
            setid_max_timed_message_len = 36,
            setid_random_buffer_bytes_count = 37,
            setid_max_random_message_len = 38,
            setid_last_tx_status = 39,
            setid_transmit_enabled = 42,
            setid_irc = 43,
            setid_cs2_timed_bit_rate = 44,
            setid_cs2_random_bit_rate = 45,
            setid_cs2_timed_channel = 56,
            setid_cs2_random_channel = 57,
            setid_next_timed_tx = 58,
            setid_next_random_tx = 59,
            setid_sat = 60,
            setid_meteosat_timed_bit_rate = 61,
            setid_meteosat_random_bit_rate = 62,
            setid_meteosat_timed_data_format = 63,
            setid_meteosat_random_data_format = 64
         };


         ////////////////////////////////////////////////////////////
         // template ScalarSetting
         ////////////////////////////////////////////////////////////
         template<class value_type>
         class ScalarSetting: public Tx3xxHelpers::TxSettingBase
         {
         private:
            ////////////////////////////////////////////////////////////
            // value
            ////////////////////////////////////////////////////////////
            value_type value;

            ////////////////////////////////////////////////////////////
            // write_name
            ////////////////////////////////////////////////////////////
            StrAsc const write_name;
            
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            ScalarSetting(
               uint2 setting_id,
               StrAsc const &read_name,
               StrAsc const &write_name_,
               VersionNumber const &min_version = "3.0",
               VersionNumber const &max_version = "255.255",
               char const *supported_sats[] = no_supported_sats):
               TxSettingBase(setting_id, read_name, min_version, max_version, supported_sats),
               write_name(write_name_)
            { }

            ////////////////////////////////////////////////////////////
            // on_response
            ////////////////////////////////////////////////////////////
            virtual bool on_response(StrAsc const &response)
            {
               size_t value_pos = response.find(write_name.c_str()) + write_name.length();
               bool rtn(false);
               if(value_pos < response.length())
               {
                  size_t value_end_pos = response.find("\r", value_pos);
                  if(value_end_pos < response.length())
                  {
                     Csi::IBuffStream temp(response.c_str() + value_pos, value_end_pos - value_pos);
                     temp.imbue(std::locale::classic());
                     temp >> value;
                     rtn = !!temp;
                  }
               }
               return rtn;
            }

            ////////////////////////////////////////////////////////////
            // write
            ////////////////////////////////////////////////////////////
            virtual void write(Message &out)
            {
               out.addUInt2(setting_id);
               out.addUInt2(sizeof(value));
               out.addBytes(&value, sizeof(value), !Csi::is_big_endian());
            }

            ////////////////////////////////////////////////////////////
            // read
            ////////////////////////////////////////////////////////////
            virtual void read(Message &in)
            {
               in.readBytes(&value, sizeof(value), !Csi::is_big_endian());
            }

            ////////////////////////////////////////////////////////////
            // format_write
            ////////////////////////////////////////////////////////////
            virtual StrAsc format_write()
            {
               Csi::OStrAscStream temp;
               temp.imbue(std::locale::classic());
               temp << write_name << value << "\r";
               return temp.str();
            }

            ////////////////////////////////////////////////////////////
            // is_read_only
            ////////////////////////////////////////////////////////////
            virtual bool is_read_only()
            { return write_name.length() == 0; }
         };


         ////////////////////////////////////////////////////////////
         // class StringSetting
         ////////////////////////////////////////////////////////////
         class StringSetting: public Tx3xxHelpers::TxSettingBase
         {
         protected:
            ////////////////////////////////////////////////////////////
            // value
            ////////////////////////////////////////////////////////////
            StrAsc value;

            ////////////////////////////////////////////////////////////
            // write_name
            ////////////////////////////////////////////////////////////
            StrAsc const write_name;
            
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            StringSetting(
               uint2 setting_id,
               StrAsc const &name,
               StrAsc const &write_name_,
               VersionNumber const &min_version = "3.0",
               VersionNumber const &max_version = "255.255",
               char const *supported_sats[] = no_supported_sats):
               TxSettingBase(setting_id, name, min_version, max_version, supported_sats),
               write_name(write_name_)
            { }

            ////////////////////////////////////////////////////////////
            // read
            ////////////////////////////////////////////////////////////
            virtual void read(Message &in)
            {
               in.readAsciiZ(value);
            }

            ////////////////////////////////////////////////////////////
            // write
            ////////////////////////////////////////////////////////////
            virtual void write(Message &out)
            {
               out.addUInt2(setting_id);
               out.addUInt2(static_cast<uint2>(value.length() + 1));
               out.addAsciiZ(value.c_str());
            }

            ////////////////////////////////////////////////////////////
            // on_response
            ////////////////////////////////////////////////////////////
            virtual bool on_response(StrAsc const &response)
            {
               size_t value_pos(response.find(write_name.c_str()) + write_name.length());
               bool rtn = false;

               while(value_pos < response.length() && isspace(response[value_pos]))
                  ++value_pos;
               if(value_pos < response.length())
               {
                  size_t value_end_pos(response.find("\r", value_pos));
                  if(value_end_pos < response.length())
                  {
                     rtn = true;
                     response.sub(value, value_pos, value_end_pos - value_pos);
                  }
               }
               return rtn;
            }

            ////////////////////////////////////////////////////////////
            // format_write
            ////////////////////////////////////////////////////////////
            virtual StrAsc format_write()
            {
               Csi::OStrAscStream temp;
               temp << write_name << value << "\r";
               return temp.str();
            }

            ////////////////////////////////////////////////////////////
            // is_read_only
            ////////////////////////////////////////////////////////////
            virtual bool is_read_only()
            { return write_name.length() == 0; }
         };


         ////////////////////////////////////////////////////////////
         // class StringStatusSetting
         ////////////////////////////////////////////////////////////
         class StringStatusSetting: public StringSetting
         {
         private:
            ////////////////////////////////////////////////////////////
            // field_name
            ////////////////////////////////////////////////////////////
            StrAsc const field_name;

         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            StringStatusSetting(
               uint2 setting_id,
               StrAsc const &read_name,
               StrAsc const &field_name_,
               VersionNumber const &min_version = "3.0",
               VersionNumber const &max_version = "255.255"):
               StringSetting(setting_id, read_name, "", min_version, max_version),
               field_name(field_name_)
            { }

            ////////////////////////////////////////////////////////////
            // on_response
            ////////////////////////////////////////////////////////////
            virtual bool on_response(StrAsc const &response)
            {
               size_t field_beg_pos(response.find(field_name.c_str()) + field_name.length());
               bool rtn(false);

               while(field_beg_pos < response.length() && response[field_beg_pos] == ' ')
                  ++field_beg_pos;
               if(field_beg_pos < response.length())
               {
                  size_t field_end_pos(response.find("\r", field_beg_pos));
                  if(field_end_pos < response.length())
                  {
                     rtn = true;
                     response.sub(value, field_beg_pos, field_end_pos - field_beg_pos);
                  }
               } 
               return rtn;
            }
         };


         ////////////////////////////////////////////////////////////
         // class GpsSetting
         ////////////////////////////////////////////////////////////
         class GpsSetting: public StringStatusSetting
         {
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            GpsSetting(
               uint2 setting_id,
               StrAsc const &field_name,
               VersionNumber const &min_version = "3.0",
               VersionNumber const &max_version = "255.255"):
               StringStatusSetting(setting_id, "POS\r", field_name, min_version, max_version)
            { }

            ////////////////////////////////////////////////////////////
            // on_response
            ////////////////////////////////////////////////////////////
            virtual bool on_response(StrAsc const &response)
            {
               bool rtn(StringStatusSetting::on_response(response));
               if(!rtn)
               {
                  value = "No GPS Fix";
                  rtn = true;
               }
               return rtn;
            }
         };


         ////////////////////////////////////////////////////////////
         // class WholeStringSetting
         ////////////////////////////////////////////////////////////
         class WholeStringSetting: public StringSetting
         {
         private:
            ////////////////////////////////////////////////////////////
            // min_lines
            ////////////////////////////////////////////////////////////
            uint4 min_lines;
            
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            WholeStringSetting(
               uint2 setting_id,
               StrAsc const &read_name,
               VersionNumber const &min_version = "3.0",
               VersionNumber const &max_version = "255.255",
               uint4 min_lines_ = 0):
               StringSetting(setting_id, read_name, "", min_version, max_version),
               min_lines(min_lines_)
            { }

            ////////////////////////////////////////////////////////////
            // on_response
            ////////////////////////////////////////////////////////////
            virtual bool on_response(StrAsc const &response)
            {
               size_t begin_pos = 0;
               size_t end_pos = response.find("\r\n>");
               while(begin_pos < response.length() && isspace(response[begin_pos]))
                  ++begin_pos;
               response.sub(value, begin_pos, end_pos - begin_pos);
               if(min_lines > 0)
               {
                  // we need to count the total number of lines that are currently in  the string
                  size_t line_pos(0);
                  uint4 lines_count(0);
                  while(line_pos < value.length())
                  {
                     line_pos = value.find("\n", line_pos + 1);
                     if(line_pos < value.length())
                        ++lines_count;
                  }

                  // we can now pad the string with the minimum number of lines
                  while(lines_count < min_lines)
                  {
                     value.append("\r\n");
                     ++lines_count;
                  }
               }
               return true;
            }
         };


         ////////////////////////////////////////////////////////////
         // class TransmitterEnabledSetting
         ////////////////////////////////////////////////////////////
         class TransmitterEnabledSetting: public StringSetting
         {
         private:
            ////////////////////////////////////////////////////////////
            // session
            ////////////////////////////////////////////////////////////
            Tx3xxSession *session;

         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            TransmitterEnabledSetting(Tx3xxSession *session_):
               StringSetting(setid_transmit_enabled, "RST\r", "Transmitter:"),
               session(session_)
            { }

            ////////////////////////////////////////////////////////////
            // on_response
            ////////////////////////////////////////////////////////////
            virtual bool on_response(StrAsc const &response)
            {
               bool rtn(StringSetting::on_response(response));
               if(rtn)
                  session->set_transmitter_enabled(value == "Enabled");
               return rtn;
            }

            ////////////////////////////////////////////////////////////
            // read
            ////////////////////////////////////////////////////////////
            void read(Message &in)
            {
               StringSetting::read(in);
               session->set_transmitter_enabled(value == "Enabled");
            }

            ////////////////////////////////////////////////////////////
            // format_write
            ////////////////////////////////////////////////////////////
            virtual StrAsc format_write()
            { return ""; }

            ////////////////////////////////////////////////////////////
            // is_read_only
            ////////////////////////////////////////////////////////////
            virtual bool is_read_only()
            { return false; }
         };


         ////////////////////////////////////////////////////////////
         // class event_tran_complete
         ////////////////////////////////////////////////////////////
         class event_tran_complete: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // response
            ////////////////////////////////////////////////////////////
            typedef Tx3xxSession::message_handle message_handle;
            message_handle response;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               Tx3xxSession *session, message_handle &response)
            {
               event_tran_complete *event(
                  new event_tran_complete(session, response));
               event->post();
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_tran_complete(
               Tx3xxSession *session, message_handle &response_):
               Event(event_id, session),
               response(response_)
            { }
         };


         uint4 const event_tran_complete::event_id(
            Event::registerType("Csi::DevConfig::Tx3xx::event_tran_complete"));


         ////////////////////////////////////////////////////////////
         // class event_tran_failed
         ////////////////////////////////////////////////////////////
         class event_tran_failed: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // failure
            ////////////////////////////////////////////////////////////
            typedef TransactionClient::failure_type failure_type;
            failure_type failure;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(Tx3xxSession *session, failure_type failure)
            {
               event_tran_failed *event(new event_tran_failed(session, failure));
               event->post();
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_tran_failed(Tx3xxSession *session, failure_type failure_):
               Event(event_id, session),
               failure(failure_)
            { } 
         };


         uint4 const event_tran_failed::event_id(
            Csi::Event::registerType("Csi::DevConfig::Tx3xx::event_tran_failure"));


         ////////////////////////////////////////////////////////////
         // class GetVersionCommand
         ////////////////////////////////////////////////////////////
         class GetVersionCommand: public Tx3xxHelpers::Command
         {
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            GetVersionCommand(Tx3xxSession *session):
               Command(session, "VER\r", 0)
            { }

            ////////////////////////////////////////////////////////////
            // on_complete
            ////////////////////////////////////////////////////////////
            virtual void on_complete(outcome_type outcome)
            {
               if(outcome == outcome_success)
               {
                  StrAsc const version_name("Firmware Version: ");
                  size_t version_pos = response.find(version_name.c_str());
                  size_t version_end_pos = response.find("\r", version_pos);
                  if(version_pos < response.length() && version_end_pos < response.length())
                  {
                     StrAsc version;
                     response.sub(
                        version, version_pos + version_name.length(),
                        version_end_pos - version_pos - version_name.length());
                     version.cut(version.find(" "));
                     session->set_firmware_version(new VersionNumber(version.c_str()));
                     Command::on_complete(outcome_success);
                  }
                  else
                     Command::on_complete(outcome_invalid_response);
               }
               else
                  Command::on_complete(outcome);
            }

            ////////////////////////////////////////////////////////////
            // no_transaction
            ////////////////////////////////////////////////////////////
            virtual bool no_transaction() const
            { return true; }
         };


         ////////////////////////////////////////////////////////////
         // class GetSatCommand
         ////////////////////////////////////////////////////////////
         class GetSatCommand: public Tx3xxHelpers::Command
         {
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            GetSatCommand(Tx3xxSession *session):
               Command(session, "SAT\r", 0)
            { }

            ////////////////////////////////////////////////////////////
            // on_complete
            ////////////////////////////////////////////////////////////
            virtual void on_complete(outcome_type outcome)
            {
               if(outcome == outcome_success)
               {
                  StrAsc const sat_name("SAT=");
                  size_t sat_pos = response.find(sat_name.c_str());
                  size_t sat_end_pos(response.find("\r", sat_pos));
                  if(sat_pos < response.length() && sat_end_pos < response.length())
                  {
                     StrAsc sat;
                     response.sub(
                        sat,
                        sat_pos + sat_name.length(),
                        sat_end_pos - sat_pos - sat_name.length());
                     session->set_sat(sat);
                     Command::on_complete(outcome);
                  }
                  else
                     Command::on_complete(outcome_invalid_response);
               }
               else
                  Command::on_complete(outcome);
            }

            ////////////////////////////////////////////////////////////
            // no_transaction
            ////////////////////////////////////////////////////////////
            virtual bool no_transaction() const
            { return true; }
         };
      };

      
      ////////////////////////////////////////////////////////////
      // class Tx3xxSession definitions
      ////////////////////////////////////////////////////////////
      Tx3xxSession::Tx3xxSession(
         SharedPtr<SessionDriverBase> driver,
         SharedPtr<OneShot> timer):
         Session(driver, timer),
         last_tran_no(0xFF),
         current_control_action(0),
         needs_to_commit(false)
      {
         static char const *goes_only_setting[] = { "", "GOES", 0 };
         static char const *meteosat_only_setting[] = { "METEOSAT_SRD", 0 };
         settings.push_back(new StringSetting(setid_sat, "SAT\r", "SAT=", "7.0", "255.255"));
         settings.push_back(new StringSetting(setid_nesid, "RCFG\r", "NESID="));
         settings.push_back(new ScalarSetting<uint2>(setid_timed_channel, "RCFG\r", "TCH=", "3.0", "5.255"));
         settings.push_back(new ScalarSetting<uint2>(setid_cs2_timed_channel, "RCFG\r", "TCH=", "6.0", "255.255"));
         settings.push_back(new ScalarSetting<uint2>(setid_timed_bit_rate, "RCFG\r", "TBR=", "3.0", "5.255"));
         settings.push_back(
            new ScalarSetting<uint2>(
               setid_cs2_timed_bit_rate, "RCFG\r", "TBR=", "6.0", "255.255", goes_only_setting));
         settings.push_back(
            new ScalarSetting<uint2>(
               setid_meteosat_timed_bit_rate, "RCFG\r", "TBR=", "7.0", "255.255", meteosat_only_setting));
         settings.push_back(new StringSetting(setid_timed_tx_interval, "RCFG\r", "TIN="));
         settings.push_back(new StringSetting(setid_offset_time, "RCFG\r", "FTT="));
         settings.push_back(new ScalarSetting<uint2>(setid_timed_window_len, "RCFG\r", "TWL="));
         settings.push_back(new StringSetting(setid_timed_window_align, "RCFG\r", "CMSG="));
         settings.push_back(
            new StringSetting(
               setid_timed_data_format, "RCFG\r", "TDF=", "6.0", "255.255", goes_only_setting));
         settings.push_back(
            new StringSetting(
               setid_meteosat_timed_data_format, "RCFG\r", "TDF=", "7.0", "255.255", meteosat_only_setting));
         settings.push_back(new StringSetting(setid_timed_empty_buffer_mode, "RCFG\r", "EBM="));
         settings.push_back(new StringSetting(setid_timed_preamble, "RCFG\r", "TPR=", "3.0", "5.255"));
         settings.push_back(new StringSetting(setid_timed_interleaver, "RCFG\r", "TIL=", "3.0", "5.255"));
         settings.push_back(new ScalarSetting<uint2>(setid_random_channel, "RCFG\r", "RCH=", "3.0", "5.255"));
         settings.push_back(new ScalarSetting<uint2>(setid_cs2_random_channel, "RCFG\r", "RCH=", "6.0", "255.255"));
         settings.push_back(new ScalarSetting<uint2>(setid_random_bit_rate, "RCFG\r", "RBR=", "3.0", "5.255"));
         settings.push_back(
            new ScalarSetting<uint2>(
               setid_cs2_random_bit_rate, "RCFG\r", "RBR=", "6.0", "255.255", goes_only_setting));
         settings.push_back(
            new ScalarSetting<uint2>(
               setid_meteosat_random_bit_rate, "RCFG\r", "RBR=", "7.0", "255.255", meteosat_only_setting));
         settings.push_back(new ScalarSetting<uint2>(setid_randomising_interval, "RCFG\r", "RIN="));
         settings.push_back(new ScalarSetting<uint2>(setid_randomising_percent, "RCFG\r", "RPC="));
         settings.push_back(new ScalarSetting<uint2>(setid_random_repeat_count, "RCFG\r", "RRC="));
         settings.push_back(
            new StringSetting(
               setid_random_data_format, "RCFG\r", "RDF=", "6.0", "255.255", goes_only_setting));
         settings.push_back(
            new StringSetting(
               setid_meteosat_random_data_format, "RCFG\r", "RDF=", "7.0", "255.255", meteosat_only_setting));
         settings.push_back(new StringSetting(setid_random_message_counter, "RCFG\r", "RMC="));
         settings.push_back(new StringSetting(setid_gps_fix_interval, "RCFG\r", "GIN="));
         settings.push_back(new StringStatusSetting(setid_serial_no, "VER\r", "Serial Number:"));
         settings.push_back(new StringSetting(setid_irc, "RCFG\r", "IRC=")); 
         settings.push_back(new StringStatusSetting(setid_hardware_version, "VER\r", "Hardware Version:"));
         settings.push_back(new StringStatusSetting(setid_firmware_version, "VER\r", "Firmware Version:"));
         settings.push_back(new StringStatusSetting(setid_gps_version, "VER\r", "GPS NP Version:"));
         settings.push_back(new StringSetting(setid_time, "TIME\r", "TIME="));
         settings.push_back(new GpsSetting(setid_last_gps_fix, "Time of fix:"));
         settings.push_back(new GpsSetting(setid_latitude, "Lat:"));
         settings.push_back(new GpsSetting(setid_longitude, "Long:"));
         settings.push_back(new GpsSetting(setid_altitude, "Alt", "3.0", "7.0"));
         settings.push_back(new GpsSetting(setid_altitude, "Alt:", "8.0", "255.255"));
         settings.push_back(new WholeStringSetting(setid_gps_status, "GPS\r"));
         settings.push_back(new StringStatusSetting(setid_fail_safe_status, "RST\r", "Failsafe:"));
         settings.push_back(new StringStatusSetting(setid_supply_voltage, "RST\r", "Supply Voltage:"));
         settings.push_back(new StringStatusSetting(setid_next_timed_tx, "RST\r", "Next Timed TX:"));
         settings.push_back(new StringStatusSetting(setid_next_random_tx, "RST\r", "Next Random TX:")); 
         settings.push_back(new StringStatusSetting(setid_temperature, "RTEMP\r", "Temp ="));
         settings.push_back(
            new StringStatusSetting(
               setid_timed_buffer_bytes_count, "TML\r", "Timed Message Length="));
         settings.push_back(
            new StringStatusSetting(
               setid_max_timed_message_len, "MTML\r", "Maximum Timed Message Length="));
         settings.push_back(
            new StringStatusSetting(
               setid_random_buffer_bytes_count, "RML\r", "Random Message Length="));
         settings.push_back(
            new StringStatusSetting(
               setid_max_random_message_len,
               "MRML\r",
               "Maximum Random Message Length="));
         settings.push_back(
            new WholeStringSetting(
               setid_last_tx_status,
               "LTXS\r",
               "3.0",
               "255.255",
               10));
         settings.push_back(new TransmitterEnabledSetting(this));
      } // constructor


      Tx3xxSession::~Tx3xxSession()
      {
      } // destructor


      void Tx3xxSession::add_transaction(
         TransactionClient *client,
         message_handle command,
         uint4 max_retry_count,
         uint4 timeout_interval,
         byte tran_no)
      {
         if(tran_no == 0)
         {
            if(++last_tran_no == 0)
               last_tran_no = 1;
            tran_no = last_tran_no; 
         }
         if(command != 0)
         {
            transactions.push_back(transaction_type(command, client));
            if(current_command == 0)
               do_next_transaction();
         }
      } // add_transaction


      void Tx3xxSession::suspend()
      {
      } // suspend


      void Tx3xxSession::on_driver_open()
      {
         do_next_command();
      } // on_driver_open


      void Tx3xxSession::on_driver_data(
         void const *buff, uint4 buff_len)
      {
         if(current_command != 0)
            current_command->on_data(buff, buff_len);
         else if(terminal != 0)
         {
            using namespace SessionHelpers;
            if(TerminalBase::is_valid_instance(terminal))
               terminal->receive_data(buff, buff_len);
         }
      } // on_driver_data


      void Tx3xxSession::on_driver_failure()
      {
         transactions_type temp(transactions);
         transactions.clear();
         commands.clear();
         if(terminal && SessionHelpers::TerminalBase::is_valid_instance(terminal))
            terminal->on_driver_failure();
         terminal = 0;
         while(!temp.empty())
         {
            transaction_type tran(temp.front());
            temp.pop_front();
            if(TransactionClient::is_valid_instance(tran.second)) 
               tran.second->on_failure(tran.first, TransactionClient::failure_link_failed);
         }
      } // on_driver_failure


      void Tx3xxSession::send_data(void const *buff, uint4 buff_len)
      {
         driver->send(this, buff, buff_len);
      } // send_data

      
      void Tx3xxSession::on_command_complete(command_type *command, command_type::outcome_type outcome)
      {
         if(current_command == command)
         {
            // how this event gets processed depends upon the current client transaction
            if(!command->no_transaction() && !transactions.empty())
            {
               message_handle &message = transactions.front().first;
               switch(message->get_message_type())
               {
               case Messages::get_settings_cmd:
                  send_get_settings_ack();
                  break;
                  
               case Messages::set_settings_cmd:
                  send_set_settings_ack();
                  break;
                  
               case Messages::control_cmd:
                  send_control_ack();
                  break;
               }

               // we can now proceed to the next command
               current_command.clear();
               do_next_command();
            }
            else
            {
               current_command.clear();
               do_next_transaction();
            }
         }
      } // on_command_complete


      void Tx3xxSession::receive(SharedPtr<Event> &ev)
      {
         if(ev->getType() == event_tran_complete::event_id)
         {
            event_tran_complete *event(static_cast<event_tran_complete *>(ev.get_rep()));
            if(!transactions.empty())
            {
               transaction_type transaction(transactions.front());
               transactions.pop_front();
               if(TransactionClient::is_valid_instance(transaction.second))
                  transaction.second->on_complete(transaction.first, event->response);
               do_next_transaction();
            }
         }
         else if(ev->getType() == event_tran_failed::event_id)
         {
            event_tran_failed *event(static_cast<event_tran_failed *>(ev.get_rep()));
            if(!transactions.empty())
            {
               transaction_type transaction(transactions.front());
               transactions.pop_front();
               if(TransactionClient::is_valid_instance(transaction.second))
                  transaction.second->on_failure(transaction.first, event->failure);
               do_next_transaction();
            }
         }
      } // receive


      void Tx3xxSession::do_next_command()
      {
         if(current_command == 0 && !commands.empty())
         {
            if(driver->is_open(this))
            {
               current_command = commands.front();
               commands.pop_front();
               current_command->start();
            }
            else
               driver->start_open(this);
         }
      } // do_next_command


      void Tx3xxSession::do_next_transaction()
      {
         if(InstanceValidator::is_valid_instance<EventReceiver>(this))
         {
            if(firmware_version != 0)
            {
               if(*firmware_version < VersionNumber("7.0") || sat.length() > 0)
               {
                  if(!transactions.empty())
                  {
                     message_handle &message(transactions.front().first);
                     switch(message->get_message_type())
                     {
                     case Messages::get_settings_cmd:
                        on_get_settings_cmd(message);
                        break;
                        
                     case Messages::set_settings_cmd:
                        on_set_settings_cmd(message);
                        break;
                        
                     case Messages::get_setting_fragment_cmd:
                        on_get_setting_fragment_cmd(message);
                        break;
                        
                     case Messages::set_setting_fragment_cmd:
                        on_set_setting_fragment_cmd(message);
                        break;
                        
                     case Messages::control_cmd:
                        on_control_cmd(message);
                        break;
                     }
                  }
               }
               else
               {
                  commands.push_back(new GetSatCommand(this));
                  do_next_command();
               }
            }
            else
            {
               // we have to get the firmware version before we can respond to any request
               commands.push_back(new GetVersionCommand(this));
               do_next_command();
            }
         }
      } // do_next_transaction


      void Tx3xxSession::on_get_settings_cmd(message_handle &message)
      {
         try
         {
            // we need to parse the message parameters
            uint2 security_code = message->readUInt2();
            uint2 begin_id = 0;
            uint2 end_id = 0xffff;

            if(message->whatsLeft() >= 2)
            {
               begin_id = message->readUInt2();
               if((begin_id & 0x8000) != 0)
               {
                  begin_id &= 0x7fff;
                  ++begin_id;
               }
               if(message->whatsLeft() >= 2)
                  end_id = (message->readUInt2() & 0xf777);
            }
            if(begin_id > end_id)
            {
               uint2 temp = begin_id;
               begin_id = end_id;
               end_id = temp;
            }

            // we can now add the commands to collect for the specified settings
            for(settings_type::iterator si = settings.begin(); si != settings.end(); ++si)
            {
               setting_handle &setting(*si);
               if(setting->setting_id >= begin_id &&
                  setting->setting_id <= end_id &&
                  *firmware_version >= setting->min_version &&
                  *firmware_version <= setting->max_version &&
                  setting->supports_sat(sat))
               {
                  // search for a command that can answer this setting
                  command_handle command;
                  for(commands_type::iterator ci = commands.begin();
                      ci != commands.end() && command == 0;
                      ++ci)
                  {
                     command_handle &candidate(*ci);
                     if(candidate->get_name() == setting->read_command)
                        command = candidate;
                  }
                  if(command == 0)
                  {
                     command.bind(new command_type(this, setting->read_command, message->get_tran_no()));
                     commands.push_back(command);
                  }
                  command->add_setting(setting);
               }
            }
            if(commands.empty())
               commands.push_back(new command_type(this, "", message->get_tran_no()));
            do_next_command();
         }
         catch(std::exception &)
         { }
      } // on_get_settings_cmd


      void Tx3xxSession::send_get_settings_ack()
      {
         if(current_command != 0)
         {
            // start forming the response
            if(current_command->outcome == command_type::outcome_success)
            {
               for(settings_type::iterator si = current_command->settings.begin();
                   si != current_command->settings.end();
                   ++si)
               {
                  setting_handle &setting(*si);
                  if(setting->on_response(current_command->response))
                     pending_gets.push_back(setting);
               }
               if(commands.empty())
               {
                  message_handle ack(new Message);
                  ack->set_message_type(Messages::get_settings_ack);
                  ack->set_tran_no(current_command->tran_no);
                  ack->addByte(1);    // response code
                  ack->addUInt2(DeviceTypes::type_tx3xx);
                  ack->addByte(1); // major version
                  ack->addByte(1); // minor version
                  ack->addBool(false); // more settings
                  
                  // we must now read the setting values and add them to the response message
                  for(settings_type::iterator si = pending_gets.begin();
                      si != pending_gets.end();
                      ++si)
                  {
                     setting_handle &setting(*si);
                     setting->write(*ack);
                  }
                  pending_gets.clear();
                  event_tran_complete::cpost(this, ack);
               }
               
            }
            else
               event_tran_failed::cpost(this, TransactionClient::failure_timed_out);
         }
      } // send_get_settings_ack


      void Tx3xxSession::on_set_settings_cmd(message_handle &message)
      {
         try
         {
            uint2 security_code = message->readUInt2();
            set_outcomes.clear();
            while(message->whatsLeft() >= 4)
            {
               // look up the setting
               uint2 setting_id = message->readUInt2();
               uint2 setting_len = (message->readUInt2() & 0x7fff);
               setting_handle setting;
               for(settings_type::iterator si = settings.begin(); si != settings.end(); ++si)
               {
                  setting_handle &candidate = *si;
                  if(candidate->setting_id == setting_id)
                  {
                     setting = candidate;
                     break;
                  }
               }
               if(setting != 0)
               {
                  StrAsc write_command;
                  setting->read(*message);
                  write_command = setting->format_write();
                  if(write_command.length() > 0)
                  {
                     needs_to_commit = true;
                     commands.push_back(
                        new command_type(
                           this, write_command, message->get_tran_no()));
                     do_next_command();
                  }
                  else if(setting->is_read_only())
                  {
                     set_outcomes.push_back(
                        set_outcome_type(
                           setting_id, SetSettings::setting_outcome_read_only));
                  }
                  else
                  {
                     set_outcomes.push_back(
                        set_outcome_type(
                           setting_id, SetSettings::setting_outcome_changed));
                  }
               }
               else
                  set_outcomes.push_back(
                     set_outcome_type(
                        setting_id, SetSettings::setting_outcome_invalid_setting_id));
            }

            // if some of the settings (like transmit enabled) don't have particular set commands,
            // we will need to pretend as if the commands succeeded.
            if(commands.empty())
            {
               commands.push_back(new command_type(this, "", message->get_tran_no()));
               needs_to_commit = true;
               do_next_command();
            }
         }
         catch(std::exception &)
         { }
      } // on_set_settings_cmd


      void Tx3xxSession::send_set_settings_ack()
      {
         if(current_command != 0)
         {
            // if the command response doesn't specify "OK", we will need to save a commit warning.
            StrAsc response(current_command->response);
            size_t ok_pos(response.find("OK\r\n"));
            if(ok_pos >= response.length() && response != "\n\r\n>" && current_command->name.length())
            {
               Csi::OStrAscStream temp;
               StrAsc command_name(current_command->name);
               
               response.cut(response.find("\r\n"));
               command_name.cut(command_name.find("\r"));
               temp << command_name << ": " << response;
               commit_warnings.push_back(temp.str());
            }
            
            if(current_command->outcome == command_type::outcome_success)
            {
               if(commands.empty())
               {
                  message_handle ack(new Message);
                  ack->set_tran_no(current_command->tran_no);
                  ack->set_message_type(Messages::set_settings_ack);
                  ack->addByte(1); // overall outcome
                  while(!set_outcomes.empty())
                  {
                     set_outcome_type outcome(set_outcomes.front());
                     set_outcomes.pop_front();
                     ack->addUInt2(outcome.first);
                     ack->addByte(outcome.second);
                  }
                  event_tran_complete::cpost(this, ack);
               }
            }
            else
            {
               event_tran_failed::cpost(this, TransactionClient::failure_timed_out);
               current_command.clear();
               commands.clear();
            }
         }
      } // send_set_settings_ack


      void Tx3xxSession::on_get_setting_fragment_cmd(message_handle &message)
      {
      } // on_get_setting_fragment_cmd


      void Tx3xxSession::on_set_setting_fragment_cmd(message_handle &message)
      {
      } // on_set_setting_fragment_cmd
      

      void Tx3xxSession::on_control_cmd(message_handle &message)
      {
         try
         {
            uint2 security_code = message->readUInt2();
            current_control_action = message->readByte();
            if(current_control_action == ControlCodes::action_commit_changes)
            {
               if(needs_to_commit)
               {
                  if(sat == "METEOSAT_SRD")
                  {
                     commands.push_back(new command_type(this, "TBR=100\r", message->get_tran_no()));
                     commands.push_back(new command_type(this, "RBR=100\r", message->get_tran_no()));
                     commands.push_back(new command_type(this, "TDF=ASCII\r", message->get_tran_no()));
                     commands.push_back(new command_type(this, "RDF=ASCII\r", message->get_tran_no()));
                  }
                  if(*firmware_version <= Csi::VersionNumber("3.0"))
                     commands.push_back(new command_type(this, "CSMODE=SDC\r", message->get_tran_no()));
                  commands.push_back(new command_type(this, "SAVE\r", message->get_tran_no()));
                  commands.push_back(new command_type(this, "ETX\r", message->get_tran_no()));
                  do_next_command();
               }
               else
               {
                  message_handle ack(new Message);
                  ack->set_tran_no(message->get_tran_no());
                  ack->set_message_type(Messages::control_ack);
                  ack->addByte(ControlCodes::outcome_committed);
                  event_tran_complete::cpost(this, ack);
               }
            }
            else if(current_control_action == ControlCodes::action_cancel_without_reboot ||
                    current_control_action == ControlCodes::action_cancel_with_reboot)
            {
               if(needs_to_commit)
               {
                  commands.push_back(new command_type(this, "RSTR\r",  message->get_tran_no()));
                  do_next_command();
               }
               else
               {
                  message_handle ack(new Message);
                  ack->set_tran_no(message->get_tran_no());
                  ack->set_message_type(Messages::control_ack);
                  ack->addByte(ControlCodes::outcome_session_ended);
                  event_tran_complete::cpost(this, ack);
               }
            }
            else if(current_control_action == ControlCodes::action_revert_to_defaults)
            {
               commands.push_back(new command_type(this, "DEFAULT\r", message->get_tran_no()));
               commands.push_back(new command_type(this, "NESID=00000000\r", message->get_tran_no()));
               if(sat != "METEOSAT_SRD")
                  commands.push_back(new command_type(this, "TBR=300\r", message->get_tran_no()));
               else
                  commands.push_back(new command_type(this, "TBR=100\r", message->get_tran_no()));
               commands.push_back(new command_type(this, "TIN=00:01:00:00", message->get_tran_no()));
               commands.push_back(new command_type(this, "TWL=10\r", message->get_tran_no()));
               commands.push_back(new command_type(this, "CMSG=Y\r", message->get_tran_no()));
               commands.push_back(new command_type(this, "TDF=A\r", message->get_tran_no()));
               commands.push_back(new command_type(this, "EBM=N\r", message->get_tran_no()));
               commands.push_back(new command_type(this, "TPR=S\r", message->get_tran_no()));
               commands.push_back(new command_type(this, "RCH=0\r", message->get_tran_no()));
               if(sat != "METEOSAT_SRD")
                  commands.push_back(new command_type(this, "RBR=300\r", message->get_tran_no()));
               else
                  commands.push_back(new command_type(this, "RBR=100\r", message->get_tran_no()));
               commands.push_back(new command_type(this, "RIN=20\r", message->get_tran_no()));
               commands.push_back(new command_type(this, "RPC=50\r", message->get_tran_no()));
               commands.push_back(new command_type(this, "GIN=00:00:00\r", message->get_tran_no()));
               commands.push_back(new command_type(this, "RRC=2\r", message->get_tran_no()));
               if(sat.length())
               {
                  StrAsc sat_command("SAT=");
                  sat_command.append(sat);
                  sat_command.append('\r');
                  commands.push_back(new command_type(this, sat_command, message->get_tran_no()));
               }
               do_next_command();
            }
            else if(current_control_action == ControlCodes::action_refresh_timer)
            {
               message_handle ack(new Message);
               ack->set_tran_no(message->get_tran_no());
               ack->set_message_type(Messages::control_ack);
               ack->addByte(ControlCodes::outcome_session_timer_reset);
               event_tran_complete::cpost(this, ack);
            }
         }
         catch(std::exception &)
         { }
      } // on_control_cmd


      void Tx3xxSession::send_control_ack()
      {
         if(current_command != 0)
         {
            if(current_control_action == ControlCodes::action_commit_changes &&
               current_command->name == "ETX\r")
            {
               StrAsc response(current_command->response);
               size_t ok_pos(response.find("OK"));
               if(ok_pos >= response.length())
               {
                  Csi::OStrAscStream temp;
                  response.cut(response.find(">"));
                  temp << "ETX: " << response;
                  commit_warnings.push_back(temp.str());
               }
            }
            if(current_command->outcome == command_type::outcome_success && commands.empty())
            {
               message_handle ack(new Message);
               ack->set_tran_no(current_command->tran_no);
               ack->set_message_type(Messages::control_ack);
               switch(current_control_action)
               {
               case ControlCodes::action_commit_changes:
                  ack->addByte(ControlCodes::outcome_committed);
                  needs_to_commit = false;
                  break;
                  
               case ControlCodes::action_cancel_without_reboot:
               case ControlCodes::action_cancel_with_reboot:
                  ack->addByte(ControlCodes::outcome_session_ended);
                  break;
                  
               case ControlCodes::action_revert_to_defaults:
                  ack->addByte(ControlCodes::outcome_reverted_to_defaults);
                  needs_to_commit = true;
                  break;
               }
               event_tran_complete::cpost(this, ack);
            }
            else if(commands.empty())
               event_tran_failed::cpost(this, TransactionClient::failure_timed_out);
         }
      } // send_control_ack
      

      namespace Tx3xxHelpers
      {
         ////////////////////////////////////////////////////////////
         // class TxSettingBase definitions
         ////////////////////////////////////////////////////////////
         char const *TxSettingBase::no_supported_sats[] = { 0 };


         bool TxSettingBase::supports_sat(StrAsc const &sat)
         {
            bool rtn = supported_sats.empty();
            if(!rtn)
            {
               supported_sats_type::iterator si(
                  std::find(supported_sats.begin(), supported_sats.end(), sat));
               if(si != supported_sats.end())
                  rtn = true;
            }
            return rtn;
         } // supports_sat
         
         
         ////////////////////////////////////////////////////////////
         // class Command definitions
         ////////////////////////////////////////////////////////////
         Command::Command(
            Tx3xxSession *session_,
            StrAsc const &name_,
            byte tran_no_):
            session(session_),
            name(name_),
            tran_no(tran_no_),
            timer(session_->get_timer()),
            timer_id(0),
            state(state_before_start),
            synch_count(0)
         { }


         Command::~Command()
         {
            if(timer != 0)
            {
               if(timer_id != 0)
                  timer->disarm(timer_id);
               timer.clear();
            }
            settings.clear();
         }


         void Command::start()
         {
            if(state == state_before_start)
            {
               if(name.length() > 0)
               {
                  state = state_synch;
                  session->send_data("\r", 1);
                  timer_id = timer->arm(this, 100);
               }
               else
               {
                  state = state_no_op;
                  timer_id = timer->arm(this, 10);
               }
            }
            else
               throw std::invalid_argument("command is already started");
         } // start


         void Command::on_data(void const *buff_, uint4 buff_len)
         {
            char const *buff(static_cast<char const *>(buff_));
            if(state == state_synch)
            {
               size_t prompt_pos;
               response.append(buff, buff_len);
               prompt_pos = response.rfind("\r\n>");
               if(prompt_pos < response.length())
               {
                  response.cut(0);
                  timer->disarm(timer_id);
                  state = state_send_command;
                  session->send_data(name.c_str(), (uint4)name.length());
                  timer_id = timer->arm(this, 2500);
               }
            }
            else if(state == state_send_command)
            {
               // we need to locate the echo of the original name
               size_t echo_pos;
               response.append(buff, buff_len);
               echo_pos = response.find(name.c_str());
               if(echo_pos < response.length())
               {
                  response.cut(0, name.length());
                  state = state_read_response;
                  if(response.length() > 0)
                     on_data(0, 0);
               }
            }
            else if(state == state_read_response)
            {
               size_t end_pos;
               if(buff_len > 0)
                  response.append(buff, buff_len);
               end_pos = response.rfind("\r\n>");
               if(end_pos < response.length())
               {
                  state = state_complete;
                  timer->disarm(timer_id);
                  on_complete(outcome_success);
               }
            }
         } // on_data


         void Command::onOneShotFired(uint4 id)
         {
            if(timer_id == id)
            {
               timer_id = 0;
               if(state == state_synch)
               {
                  // we need to decide whether we should attempt another synch byte
                  if(synch_count < 5)
                  {
                     ++synch_count;
                     session->send_data("\r", 1);
                     timer_id = timer->arm(this, 100);
                  }
                  else
                     on_complete(outcome_synch_failed);
               }
               else if(state == state_send_command)
                  on_complete(outcome_no_echo);
               else if(state == state_read_response)
                  on_complete(outcome_invalid_response);
               else if(state == state_no_op)
                  on_complete(outcome_success);
            }
         } // onOneShotFired


         void Command::on_complete(outcome_type outcome_)
         {
            outcome = outcome_;
            session->on_command_complete(this, outcome);
         } // on_complete 
      };
   };
};


/* Csi.Alert2.cpp

   Copyright (C) 2016, 2021 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Wednesday 06 July 2016
   Last Change: Tuesday 04 May 2021
   Last Commit: $Date: 2021-05-05 12:13:03 -0600 (Wed, 05 May 2021) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Alert2.h"
#include "Csi.ByteOrder.h"
#include "Csi.Utils.h"
#include "Csi.StringLoader.h"
#include "Csi.StrAscStream.h"
#include "Csi.BuffStream.h"
#include <iostream>


namespace Csi
{
   namespace Alert2
   {
      StrUni default_report_name(uint2 sensor_id)
      {
         StrUni rtn;
         switch(sensor_id)
         {
         case standard_sensor_rain:
            rtn = L"Rain";
            break;
            
         case standard_sensor_air_temp:
            rtn = L"AirTemp";
            break;
            
         case standard_sensor_rh:
            rtn = L"RH";
            break;

         case standard_sensor_bp:
            rtn = L"BP";
            break;
            
         case standard_sensor_ws:
            rtn = L"WindSpeed";
            break;
            
         case standard_sensor_wd:
            rtn = L"WindDir";
            break;
            
         case standard_sensor_wp:
            rtn = L"PeakWindSpeed";
            break;
            
         case standard_sensor_stage:
            rtn = L"Stage";
            break;
            
         case standard_sensor_battery:
            rtn = L"BattVolts";
            break;
         }
         return rtn;
      } // default_report_name

      StrAsc default_report_convert_expression(uint2 sensor_id)
      {
         StrAsc rtn("value");
         switch(sensor_id)
         {
         case standard_sensor_air_temp:
         case standard_sensor_bp:
         case standard_sensor_battery:
            rtn = "IIF(reportType = 3 OR reportType = 4, value / 10, value)";
            break;

         case standard_sensor_stage:
            rtn = "IIF(reportType = 3, value / 100, IIF(reportType = 4, value / 1000, value))";
         }
         return rtn;
      } // default_report_convert_expression

      SensorReportValue::SensorReportValue(
         SensorReportBase *report_,
         uint2 sensor_id_,
         void const *buff_,
         size_t buff_len):
         value_time_offset(0)
      {
         byte const *buff(static_cast<byte const *>(buff_));
         FormatLen fl(buff[0]);
         fl.init_value_report(*this, report_, sensor_id_, buff + 1, buff_len - 1);
      } // constructor

      LgrDate const &SensorReportValue::get_received_time() const
      { return report->pdu->message->get_received_time(); }

      uint2 SensorReportValue::get_station_id() const
      { return report->pdu->message->get_source_address(); }
      
      LgrDate SensorReportValue::get_time_stamp() const
      {
         LgrDate pdu_time(report->pdu->get_pdu_time());
         return pdu_time - value_time_offset;
      } // get_time_stamp

      int8 SensorReportValue::get_time_offset() const
      {
         int8 rtn((report->pdu->get_time_stamp() * Csi::LgrDate::nsecPerSec) - value_time_offset);
         return rtn;
      } // get_time_offset

      bool SensorReportValue::get_has_time_stamp() const
      { return report->pdu->get_has_time_stamp(); }

      byte SensorReportValue::get_apdu_id() const
      { return report->pdu->get_apdu_id(); }

      bool SensorReportValue::get_from_test() const
      { return report->pdu->get_from_test(); }

      report_type_code SensorReportValue::get_report_type() const
      { return report->get_report_type(); }

         
      void FormatLen::init_value_report(
         SensorReportValue &value,
         SensorReportBase *report,
         uint2 sensor_id,
         void const *buff_,
         size_t buff_len)
      {
         byte const *buff(static_cast<byte const *>(buff_));
         value.report = report;
         value.sensor_id = sensor_id;
         value.value_type = value_type;
         if(value_type == sensor_value_uint)
         {
            uint4 uint_value;
            if(value_len == 1 && buff_len >= 1)
               uint_value = buff[0];
            else if(value_len == 2 && buff_len >= 2)
            {
               uint2 temp;
               memcpy(&temp, buff, sizeof(temp));
               if(!is_big_endian())
                  reverse_byte_order(&temp, sizeof(temp));
               uint_value = temp;
            }
            else if(value_len == 4 && buff_len >= 4)
            {
               uint4 temp;
               memcpy(&temp, buff, sizeof(temp));
               if(!is_big_endian())
                  reverse_byte_order(&temp, sizeof(temp));
               uint_value = temp;
            }
            else
               throw std::invalid_argument("invalid uint length");
            value.storage.uint_value = uint_value;
         }
         else if(value_type == sensor_value_int)
         {
            int4 int_value;
            if(value_len == 1 && buff_len >= 1)
               int_value = buff[0];
            else if(value_len == 2 && buff_len >= 2)
            {
               int2 temp;
               memcpy(&temp, buff, sizeof(temp));
               if(!is_big_endian())
                  reverse_byte_order(&temp, sizeof(temp));
               int_value = temp;
            }
            else if(value_len == 4 || buff_len >= 4)
            {
               int4 temp;
               memcpy(&temp, buff, sizeof(temp));
               if(!is_big_endian())
                  reverse_byte_order(&temp, sizeof(temp));
               int_value = temp;
            }
            else
               throw std::invalid_argument("invalid int length");
            value.storage.int_value = int_value;
         }
         else if(value_type == sensor_value_float)
         {
            double float_value;
            if(value_len == 4 && buff_len >= 4)
            {
               float temp;
               memcpy(&temp, buff, sizeof(temp));
               if(!is_big_endian())
                  reverse_byte_order(&temp, sizeof(temp));
               float_value = temp;
            }
            else if(value_len == 8 && buff_len >= 8)
            {
               memcpy(&float_value, buff, sizeof(float_value));
               if(!is_big_endian())
                  reverse_byte_order(&float_value, sizeof(float_value));
            }
            else
               throw std::invalid_argument("invalid float length");
            value.storage.float_value = float_value;
         }
         else if(value_type == sensor_value_transmission_offset)
         {
            if(value_len == 1 && buff_len >= 1)
               value.storage.uint_value = buff[0];
            else
               throw std::invalid_argument("invalid transmission offset length");
         }
         else if(value_type == sensor_value_day_offset)
         {
            if(value_len == 2 && buff_len >= 2)
            {
               uint2 temp;
               memcpy(&temp, buff, sizeof(temp));
               if(!is_big_endian())
                  reverse_byte_order(&temp, sizeof(temp));
               value.storage.uint_value = temp;
            }
            else
               throw std::invalid_argument("invalid day length");
         }
         else if(value_type == sensor_value_timestamp)
         {
            if(value_len == 4 && buff_len >= 4)
            {
               uint4 temp;
               memcpy(&temp, buff, sizeof(temp));
               if(!is_big_endian())
                  reverse_byte_order(&temp, sizeof(temp));
               value.storage.uint_value = temp;
            }
            else
               throw std::invalid_argument("invalid time stamp length");
         }
         else
            throw std::invalid_argument("unsupported sensor report value type");
      } // init_value_report
      

      SensorReportGeneral::SensorReportGeneral(
         MantPduBase *pdu_, void const *buff_, size_t buff_len):
         SensorReportBase(pdu_, report_general_sensor)
      {
         byte const *buff(static_cast<byte const *>(buff_));
         size_t value_pos(0);

         while(value_pos + 2 < buff_len)
         {
            byte sensor_id(buff[value_pos]);
            FormatLen fl(buff[value_pos + 1]);
            value_type value(this, sensor_id, buff + value_pos + 1, fl.value_len + 1);
            values.push_back(value);
            value_pos += fl.value_len + 2;
         }
      } // constructor


      SensorReportRainGauge::SensorReportRainGauge(
         MantPduBase *pdu_, void const *buff_, size_t buff_len):
         SensorReportBase(pdu_, report_rain_gauge)
      {
         byte const *buff(static_cast<byte const *>(buff_));
         uint2 sensor_id(buff[0]);
         size_t accum_len(buff[1] & 0x0f);
         value_type accum_value(this, sensor_id, buff + 1, accum_len + 1);
         if(accum_value.get_value_type() == sensor_value_uint || accum_value.get_value_type() == sensor_value_int)
         {
            // we will add sensor values for each tip that is reported.
            size_t offset_pos(accum_len + 2);
            size_t tips_count(buff_len - offset_pos);
            for(uint4 i = 0; accum_value.get_value_int() > 0 && i < tips_count; ++i)
            {
               value_type tip_value(this, sensor_id, sensor_value_int, static_cast<int4>(accum_value.get_value_int() - tips_count + i));
               tip_value.set_value_time_offset(buff[offset_pos + i] * Csi::LgrDate::nsecPerSec);
               values.push_back(tip_value);
            }
            if(values.empty())
               values.push_back(accum_value);
         }
         else
            values.push_back(accum_value);
      } // constructor


      SensorReportMultiEnglish::SensorReportMultiEnglish(
         MantPduBase *pdu_, void const *buff_, size_t buff_len):
         SensorReportBase(pdu_, report_multi_sensor_english)
      {
         byte const *buff(static_cast<byte const *>(buff_));
         byte flags(buff[0]);
         size_t value_pos(1);
         if((flags & 0x01) != 0) // air temperature
         {
            int2 temp;
            memcpy(&temp, buff + value_pos, sizeof(temp));
            if(!is_big_endian())
               reverse_byte_order(&temp, sizeof(temp));
            values.push_back(value_type(this, standard_sensor_air_temp, sensor_value_uint, static_cast<int4>(temp)));
            value_pos += sizeof(temp);
         }
         if((flags & 0x02) != 0) // rh
         {
            values.push_back(value_type(this, standard_sensor_rh, sensor_value_uint, static_cast<uint4>(buff[value_pos])));
            ++value_pos ;
         }
         if((flags & 0x04) != 0) // barometric pressure
         {
            uint2 temp;
            memcpy(&temp, buff + value_pos, sizeof(temp));
            if(!is_big_endian())
               reverse_byte_order(&temp, sizeof(temp));
            values.push_back(value_type(this, standard_sensor_bp, sensor_value_uint, static_cast<uint4>(temp)));
            value_pos += sizeof(temp);
         }
         if((flags & 0x08) != 0) // wind speed
         {
            values.push_back(value_type(this, standard_sensor_ws, sensor_value_uint, static_cast<uint4>(buff[value_pos])));
            ++value_pos;
         }
         if((flags & 0x10) != 0) // wind direction
         {
            uint2 temp;
            memcpy(&temp, buff + value_pos, sizeof(temp));
            if(!is_big_endian())
               reverse_byte_order(&temp, sizeof(temp));
            values.push_back(value_type(this, standard_sensor_wd, sensor_value_uint, static_cast<uint4>(temp)));
            value_pos += sizeof(temp);
         }
         if((flags & 0x20) != 0) // peak wind
         {
            values.push_back(value_type(this, standard_sensor_wp, sensor_value_uint, static_cast<uint4>(buff[value_pos])));
            ++value_pos;
         }
         if((flags & 0x40) != 0) // stage
         {
            int2 temp;
            memcpy(&temp, buff + value_pos, sizeof(temp));
            if(!is_big_endian())
               reverse_byte_order(&temp, sizeof(temp));
            values.push_back(value_type(this, standard_sensor_stage, sensor_value_int, static_cast<int4>(temp)));
            value_pos += sizeof(temp);
         }
         if((flags & 0x80) != 0) // battery voltage
         {
            values.push_back(value_type(this, standard_sensor_battery, sensor_value_uint, static_cast<uint4>(buff[value_pos])));
            ++value_pos;
         }
      } // constructor


      SensorReportMultiMetric::SensorReportMultiMetric(
         MantPduBase *pdu_, void const *buff_, size_t buff_len):
         SensorReportBase(pdu_, report_multi_sensor_metric)
      {
         byte const *buff(static_cast<byte const *>(buff_));
         byte flags(buff[0]);
         size_t value_pos(1);
         if((flags & 0x01) != 0) // air temperature
         {
            int2 temp;
            memcpy(&temp, buff + value_pos, sizeof(temp));
            if(!is_big_endian())
               reverse_byte_order(&temp, sizeof(temp));
            values.push_back(value_type(this, standard_sensor_air_temp, sensor_value_uint, static_cast<int4>(temp)));
            value_pos += sizeof(temp);
         }
         if((flags & 0x02) != 0) // rh
         {
            values.push_back(value_type(this, standard_sensor_rh, sensor_value_uint, static_cast<uint4>(buff[value_pos])));
            ++value_pos ;
         }
         if((flags & 0x04) != 0) // barometric pressure
         {
            uint2 temp;
            memcpy(&temp, buff + value_pos, sizeof(temp));
            if(!is_big_endian())
               reverse_byte_order(&temp, sizeof(temp));
            values.push_back(value_type(this, standard_sensor_bp, sensor_value_uint, static_cast<uint4>(temp)));
            value_pos += sizeof(temp);
         }
         if((flags & 0x08) != 0) // wind speed
         {
            uint2 temp;
            memcpy(&temp, buff + value_pos, sizeof(temp));
            if(!is_big_endian())
               reverse_byte_order(&temp, sizeof(temp));
            values.push_back(value_type(this, standard_sensor_ws, sensor_value_uint, static_cast<uint4>(temp)));
            value_pos += sizeof(temp);
         }
         if((flags & 0x10) != 0) // wind direction
         {
            uint2 temp;
            memcpy(&temp, buff + value_pos, sizeof(temp));
            if(!is_big_endian())
               reverse_byte_order(&temp, sizeof(temp));
            values.push_back(value_type(this, standard_sensor_wd, sensor_value_uint, static_cast<uint4>(temp)));
            value_pos += sizeof(temp);
         }
         if((flags & 0x20) != 0) // peak wind
         {
            uint2 temp;
            memcpy(&temp, buff + value_pos, sizeof(temp));
            if(!is_big_endian())
               reverse_byte_order(&temp, sizeof(temp));
            values.push_back(value_type(this, standard_sensor_wp, sensor_value_uint, static_cast<uint4>(temp)));
            value_pos += sizeof(temp);
         }
         if((flags & 0x40) != 0) // stage
         {
            int4 b1(buff[value_pos]);
            int4 b2(buff[value_pos + 1]);
            int4 b3(buff[value_pos + 2]);
            int4 temp((b1 << 16) || (b2 << 8) || b3);
            values.push_back(value_type(this, standard_sensor_stage, sensor_value_int, temp));
            value_pos += 3;
         }
         if((flags & 0x80) != 0) // battery voltage
         {
            values.push_back(value_type(this, standard_sensor_battery, sensor_value_uint, static_cast<uint4>(buff[value_pos])));
            ++value_pos;
         }
      } // constructor


      SensorReportTimeSeries::SensorReportTimeSeries(
         MantPduBase *pdu_, void const *buff_, size_t buff_len):
         SensorReportBase(pdu_, report_time_series)
      {
         // we need to peek at the sensor ID and, if the posix timestamp is provided, we need to save
         // that timestamp.
         byte const *buff(static_cast<byte const *>(buff_));
         int8 value_offset(0);
         size_t sensor_start(0);
         if(buff[0] == 0xff)
         {
            // check buffer length and F/L type and extract the time_t posix time.
            uint4 posix_time;
            if(buff_len < 6)
               throw std::invalid_argument("time series stamp is too small");
            if(buff[1] != 0xf4)
               throw std::invalid_argument("invalid F/L type for time series time stamp");
            memcpy(&posix_time, buff + 2, sizeof(posix_time));
            if(!is_big_endian())
               reverse_byte_order(&posix_time, sizeof(posix_time));
            sensor_start = 6;

            // we need to adjust the start value offset so that the posix time is applied to it.  We
            // basically need to adjust the offset so that, when applied to the PDU time stamp, it
            // will reflect the posix time.
            LgrDate pdu_time(pdu->get_pdu_time());
            LgrDate report_time(LgrDate::from_time_t(posix_time));
            value_offset = report_time.get_nanoSec() - pdu_time.get_nanoSec();
         }

         // we can now parse the sensor ID, interval, and the F/L
         uint2 sensor_id(buff[sensor_start]);
         byte interval_code(buff[sensor_start + 1]);
         byte interval_units((interval_code & 0b11000000) >> 6);
         byte interval_value(interval_code & 0b00111111);
         int8 interval;
         FormatLen sensor_fl(buff[sensor_start + 1]);
         size_t data_start(sensor_start + 3);
         switch(interval_units)
         {
         case 0:
            if(interval_value > 0 && interval_value < 60)
               interval = interval_value * LgrDate::nsecPerSec;
            else
            {
               interval = LgrDate::nsecPerSec;
               switch(interval_value)
               {
               case 60:
                  interval /= 10;
                  break;

               case 61:
                  interval /= 100;
                  break;

               case 62:
                  interval /= 1000;
                  break;

               case 63:
                  interval /= 10000;
                  break;
               }
            }
            break;
            
         case 1:
            interval = interval_value * LgrDate::nsecPerMin;
            break;

         case 2:
            interval = interval_value * LgrDate::nsecPerHour;
            break;

         case 3:
            interval = interval_value * LgrDate::nsecPerDay;
            break;
         }

         // the values are reported oldest to newest but the report time is for the newest value.
         // In order to apply an offset starting at the oldest, we will need to calculate the number
         // of records in the report.
         size_t data_bytes_count(buff_len - data_start + 1);
         size_t records_count(data_bytes_count / sensor_fl.value_len);
         for(size_t i = 0; i < records_count; ++i)
         {
            value_type value;
            size_t value_start(data_start + (i * sensor_fl.value_len));
            size_t intervals_passed(records_count - i);
            sensor_fl.init_value_report(value, this, sensor_id, buff + value_start, buff_len - value_start);
            value.set_value_time_offset(value_offset + ((records_count - i) * interval));
            values.push_back(value);
         }
      } // constructor

      
      ConcentrationReport::ConcentrationReport(
         MantPduBase *pdu_, void const *buff_, size_t buff_len):
         SensorReportBase(pdu_, report_concentration)
      {
         byte const *buff(static_cast<byte const *>(buff_));
         size_t value_pos(0);
         while(value_pos + 4 <= buff_len)
         {
            uint2 b1(buff[value_pos]);
            uint2 b2(buff[value_pos + 1]);
            uint2 b3(buff[value_pos + 2]);
            uint2 sensor_id(b1  | ((b2 & 0x1f) << 8));
            uint2 value(((b2 & 0xe0) << 3) | b3);
            value_type report_value(this, sensor_id, sensor_value_uint, static_cast<uint4>(value));
            report_value.set_value_time_offset(buff[value_pos + 3] * Csi::LgrDate::nsecPerSec);
            values.push_back(report_value);
            value_pos += 4;
         }
      } // constructor


      LgrDate MantPduBase::get_pdu_time() const
      {
         LgrDate received_time(message->get_received_time());
         LgrDate rtn(received_time);
         if(has_time_stamp)
         {
            rtn -= received_time.get_nanoSec() % (LgrDate::nsecPerHour * 12);
            rtn += time_stamp * LgrDate::nsecPerSec;
            if(rtn > received_time)
               rtn -= 12 * LgrDate::nsecPerHour;
         }
         return rtn;
      } // get_pdu_time
      
      
      SelfReportPdu::SelfReportPdu(
         IndMessageMant *message_, void const *report_buff, size_t report_buff_len):
         MantPduBase(message_)
      {
         // we need to parse the control byte.
         byte const *buff(static_cast<byte const *>(report_buff));
         uint4 control(buff[0]);
         size_t time_pos(1);
         size_t report_pos(3);
         
         version = static_cast<byte>(control & 0x03);
         if(version != 0)
            throw std::invalid_argument("unsupported self report PDU version number");
         has_time_stamp = (control & 0x04) != 0;
         from_test = (control & 0x08) != 0;
         apdu_id = static_cast<byte>((control & 0x70) >> 4);
         if((control & 0x80) != 0)
         {
            // we'll ignore the second control byte
            ++time_pos;
            ++report_pos;
         }
         if(!has_time_stamp)
            report_pos -= 2;
         if(time_pos >= report_buff_len || report_pos >= report_buff_len)
            throw std::invalid_argument("malformed self-reporting PDU");

         // we need to extract the time stamp if it is present.
         if(has_time_stamp)
         {
            uint2 temp;
            memcpy(&temp, buff + time_pos, sizeof(temp));
            if(!Csi::is_big_endian())
               Csi::reverse_byte_order(&temp, sizeof(temp));
            time_stamp = temp;
         }

         // we can now parse the self reports
         while(report_pos + 2 < report_buff_len)
         {
            // read the header for the report field.
            byte report_type(buff[report_pos]);
            uint2 report_len(buff[report_pos + 1]);
            size_t value_start(2);
            
            if((report_len & 0x80) != 0)
            {
               ++value_start;
               report_len <<= 8;
               report_len += buff[report_pos + 2];
            }
            if(value_start + report_len > report_buff_len)
               throw std::invalid_argument("malformed MANT PDU");

            // we need to interpret the report type
            byte const *report_start(buff + report_pos + value_start);
            switch(report_type)
            {
            case report_general_sensor:
               reports.push_back(new SensorReportGeneral(this, report_start, report_len));
               break;
               
            case report_rain_gauge:
               reports.push_back(new SensorReportRainGauge(this, report_start, report_len));
               break;
               
            case report_multi_sensor_english:
               reports.push_back(new SensorReportMultiEnglish(this, report_start, report_len));
               break;
               
            case report_multi_sensor_metric:
               reports.push_back(new SensorReportMultiMetric(this, report_start, report_len));
               break;

            case report_time_series:
               reports.push_back(new SensorReportTimeSeries(this, report_start, report_len));
               break;
            }

            // position the index at the start of the next report.
            report_pos += value_start + report_len;
         }
      } // constructor


      ConcentrationPdu::ConcentrationPdu(
         IndMessageMant *message_, void const *buff_, size_t buff_len):
         MantPduBase(message_)
      {
         // we need to parse the control byte
         byte const *buff(static_cast<byte const *>(buff_));
         byte control(buff[0]);
         size_t time_pos(1);
         size_t report_pos(3);
         version = static_cast<byte>(control & 0x03);
         if(version != 0)
            throw std::invalid_argument("unsupported concentration version number");
         has_time_stamp = (control & 0x04) != 0;
         from_test = (control & 0x08) != 0;
         apdu_id = (control & 0x70) >> 4;
         if((control & 0x80) != 0)
         {
            // we'll ignore the second control byte
            ++time_pos;
            ++report_pos;
         }
         if(!has_time_stamp)
            report_pos -= 2;
         if(time_pos >= buff_len || report_pos >= buff_len)
            throw std::invalid_argument("malformed concentration PDU");

         // we need to extract the time stamp if present.
         if(has_time_stamp)
         {
            uint2 temp;
            memcpy(&temp, buff + time_pos, sizeof(temp));
            if(!is_big_endian())
               reverse_byte_order(&temp, sizeof(temp));
            time_stamp = temp;
         }
         reports.push_back(new ConcentrationReport(this, buff + report_pos, buff_len - report_pos));
      } // constructor
      

      IndMessageMant::IndMessageMant(CsvRec const &content):
         IndMessageBase(ind_message_mant),
         protocol(protocol_broadcast),
         add_path_service(false),
         has_destination_address(false),
         service_port(port_self_reporting),
         has_mant_pdu_id(false),
         mant_pdu_id(0),
         source_address(0)
      {
         // define the index of values within the MANT IND structure
         size_t const received_year_pos(1);
         size_t const received_month_pos(2);
         size_t const received_day_pos(3);
         size_t const received_hour_pos(4);
         size_t const received_minute_pos(5);
         size_t const received_seconds_pos(6);
         size_t const protocol_pos(8);
         size_t const add_path_service_pos(10);
         size_t const destination_address_present_pos(11);
         size_t const service_port_pos(12);
         size_t const ack_pos(14);
         size_t const added_header_pos(15);
         size_t const hop_limit_pos(16);
         size_t const payload_len_pos(17);
         size_t const source_address_pos(18);
         size_t dest_address_pos(19);
         size_t mant_pdu_id_pos(19);
         size_t path_count_pos(19);
         size_t path_address_pos(20);
         size_t payload_pos(19);
         
         // we need to parse the received time stamp.
         OStrAscStream temp;

         temp.imbue(StringLoader::make_locale(0));
         temp << content.at(received_year_pos) << "-"
              << content.at(received_month_pos) << "-"
              << content.at(received_day_pos) << "T"
              << content.at(received_hour_pos) << ":"
              << content.at(received_minute_pos) << ":"
              << content.at(received_seconds_pos);
         received_time = LgrDate::fromStr(temp.c_str());

         // we need to parse the protocol field.
         if(content.at(protocol_pos) == "0")
            protocol = protocol_broadcast;
         else if(content.at(protocol_pos) == "1")
            protocol = protocol_end_to_end;
         else
            throw std::invalid_argument("invalid protocol code");

         // we need to parse the add path service flag
         if(content.at(add_path_service_pos) == "1")
         {
            add_path_service = true;
            ++payload_pos;
         }

         // we need to process the destination address in header flag
         if(content.at(destination_address_present_pos) == "1")
         {
            has_destination_address = true;
            ++mant_pdu_id_pos;
            ++path_count_pos;
            ++path_address_pos;
            ++payload_pos;
         }

         // we need to parse the service port.
         if(content.at(service_port_pos) == "0")
            service_port = port_self_reporting;
         else if(content.at(service_port_pos) == "1")
            service_port = port_concentration;
         else
            throw std::invalid_argument("unsupported service port");

         // we need to process the ack bit
         if(content.at(ack_pos) == "1")
         {
            has_mant_pdu_id = true;
            ++path_count_pos;
            ++path_address_pos;
            ++payload_pos;
         }

         // since the standard does not specify what, if anything we should expect for an added
         // header, we will have to reject any message with an added header.
         if(content.at(added_header_pos) == "1")
            throw std::invalid_argument("cannot support the added header");

         // we need to parse the hop limit
         hop_limit = strtoul(content.at(hop_limit_pos).c_str(), 0, 10);

         // we need to parse the payload length.
         uint4 payload_len = strtoul(content.at(payload_len_pos).c_str(), 0, 10);

         // we need to parse the source address.
         uint4 address_spec(strtoul(content.at(source_address_pos).c_str(), 0, 10));
         if(address_spec > 0xffff)
            throw std::invalid_argument("invalid source address");
         source_address = static_cast<uint2>(address_spec);

         // we need to parse the destination address
         if(has_destination_address)
         {
            address_spec = strtoul(content.at(dest_address_pos).c_str(), 0, 10);
            if(address_spec > 0xffff)
               throw std::invalid_argument("invalid destination address");
            destination_address = static_cast<uint2>(address_spec);
         }

         // we need to parse the mant pdu field
         if(has_mant_pdu_id)
            mant_pdu_id = strtoul(content.at(mant_pdu_id_pos).c_str(), 0, 10);

         // we need to parse the repeater addresses
         if(add_path_service)
         {
            uint4 repeaters_count(strtoul(content.at(path_count_pos).c_str(), 0, 10));
            payload_pos += repeaters_count;
            for(uint4 i = 0; i < repeaters_count; ++i)
            {
               address_spec = strtoul(content.at(path_address_pos + i).c_str(), 0, 10);
               if(address_spec > 0xffff)
                  throw std::invalid_argument("invalid repeater address");
               source_addresses.push_back(static_cast<uint2>(address_spec));
            }
         }

         // we will only parse the payload if theport indicates self reporting data.
         if(service_port == port_self_reporting || service_port == port_concentration)
         {
            // we need to parse the payload as a sequence of hex digits.
            StrBin payload;
            payload.reserve(payload_len);
            for(uint4 i = 0; i < payload_len; ++i)
            {
               uint4 value(strtoul(content.at(payload_pos + i).c_str(), 0, 16));
               if(value > 0xff)
                  throw std::invalid_argument("invalid payload value");
               payload.append(static_cast<byte>(value));
            }

            // with the payload extracted, we can now parse the PDU.
            if(service_port == port_self_reporting)
               pdu.bind(new SelfReportPdu(this, payload.getContents(), payload.length()));
            else if(service_port == port_concentration)
               pdu.bind(new ConcentrationPdu(this, payload.getContents(), payload.length()));
         }
      } // constructor


      IndMessageMant::sensor_identifiers_type IndMessageMant::get_sensor_identifiers() const
      {
         sensor_identifiers_type rtn;
         if(pdu != 0)
         {
            for(MantPduBase::const_iterator ri = pdu->begin(); ri != pdu->end(); ++ri)
            {
               MantPduBase::value_type const  &report(*ri);
               for(SensorReportBase::const_iterator vi = report->begin(); vi != report->end(); ++vi)
               {
                  SensorReportBase::value_type const &value(*vi);
                  rtn.push_back(value.get_sensor_id());
               }
            }
         }
         std::sort(rtn.begin(), rtn.end());
         rtn.erase(std::unique(rtn.begin(), rtn.end()), rtn.end());
         return rtn;
      } // get_sensor_identifiers


      size_t IndMessageMant::get_values_count() const
      {
         size_t rtn(0);
         if(pdu != 0)
         {
            for(MantPduBase::const_iterator ri = pdu->begin(); ri != pdu->end(); ++ri)
            {
               MantPduBase::value_type const &report(*ri);
               rtn += report->size();
            }
         }
         return rtn;
      } // get_values_count


      IndStream::IndStream(IndStreamClient *client_):
         client(client_)
      { }


      void IndStream::on_data(void const *buff, size_t buff_len)
      {
         uint4 eol_pos;
         rx_buff.push(buff, (uint4)buff_len);
         eol_pos = rx_buff.find("\n", 1);
         while(eol_pos < rx_buff.size() && IndStreamClient::is_valid_instance(client))
         {
            try
            {
               // we need to move the data from the receive buffer to the message buffer.
               message_buff.cut(0);
               rx_buff.pop(message_buff, eol_pos + 1);
               client->on_message_content(this, message_buff);
               
               // we need to parse the data as comma separated.
               IBuffStream input(message_buff.getContents(), message_buff.length());
               input.imbue(StringLoader::make_locale(0));
               parser.clear();
               parser.read(input);
               if(parser.size() > 0)
               {
                  // there is not an IND that currently exists that conforms to the standard (which
                  // makes one question the standard).  Since we have to parsestuff that does not
                  // conform and the non-conforming is a subset of the standard, we will cut down
                  // any standard messages to the non-conforming versions.
                  if(parser.front() == "ALERT2A")
                  {
                     // The concomforming "N" messages omit the time quality flag so we need to cut
                     // it as well as the IND header.
                     parser.erase(parser.begin(), parser.begin() + 5);
                     if(parser.front() == "N")
                        parser.erase(parser.begin() + 1, parser.begin() + 2);
                  }

                  // we can now look at the message to determine its format.
                  if(parser.front() == "N")
                  {
                     LightPolySharedPtr<IndMessageBase, IndMessageMant> message(new IndMessageMant(parser));
                     client->on_message(this, message.get_handle());
                  }
                  else if(parser.front() == "P")
                  {
                     LightPolySharedPtr<IndMessageBase, IndMessageAirlink> message(new IndMessageAirlink(parser));
                     client->on_message(this, message.get_handle());
                  }
                  else if(parser.front() == "C" || parser.front() == "A")
                  {
                     LightPolySharedPtr<IndMessageBase, IndMessageConcentration> message(new IndMessageConcentration(parser));
                     client->on_message(this, message.get_handle());
                  }
                  else if(parser.front() == "S")
                  {
                     LightPolySharedPtr<IndMessageBase, IndMessageStatus> message(new IndMessageStatus(parser));
                     client->on_message(this, message.get_handle());
                  }
               }
            }
            catch(std::exception &e)
            {
               if(IndStreamClient::is_valid_instance(client))
                  client->on_error(this, e.what(), message_buff);
            }

            // search for the next end of line.
            eol_pos = rx_buff.find("\n", 1);
         }
      } // on_data


      void IndStream::clear()
      {
         rx_buff.pop(rx_buff.size());
      } // clear
   };
};


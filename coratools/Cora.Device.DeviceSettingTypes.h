/* Cora.Device.DeviceSettingTypes.h

   Defines device specific setting types (specifically structures)
   
   Copyright (C) 2000, 2021 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 20 July 2000
   Last Change: Friday 19 February 2021
   Last Commit: $Date: 2021-02-19 09:13:53 -0600 (Fri, 19 Feb 2021) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Cora_Device_DeviceSettingTypes_h
#define Cora_Device_DeviceSettingTypes_h

#include "Cora.Setting.h"
#include "Cora.CommonSettingTypes.h"
#include "Cora.Device.Defs.h"
#include "Csi.LgrDate.h"
#include "StrUni.h"
#include "Csi.RangeList.h"
#include <list>


namespace Cora
{
   namespace Device
   {
      ////////////////////////////////////////////////////////////
      // class ClockSetSchedule
      //////////////////////////////////////////////////////////// 
      class ClockCheckSchedule: public Setting
      {
      public:
         ClockCheckSchedule(uint4 setting_id):
            Setting(setting_id)
         { }

         virtual bool read(Csi::Messaging::Message *msg);
         virtual bool read(char const *s);
         virtual void write(Csi::Messaging::Message *out) const;
         virtual void format(std::ostream &out) const;

         virtual json_value_handle write_json() const
         {
            Csi::Json::ObjectHandle rtn(new Csi::Json::Object);
            rtn->set_property_bool("schedule_on", schedule_on);
            rtn->set_property_date("base", base);
            rtn->set_property_number("interval", interval);
            rtn->set_property_number("max_deviation", max_deviation);
            return rtn.get_handle();
         }

         virtual bool read_json(json_value_handle &val_)
         {
            bool rtn(true);
            try
            {
               Csi::Json::ObjectHandle val(val_);
               schedule_on = val->get_property_bool("schedule_on");
               base = Csi::LgrDate::fromStr(val->get_property_str("base").c_str());
               interval = (uint4)val->get_property_number("interval");
               max_deviation = (uint4)val->get_property_number("max_deviation");
            }
            catch(std::exception &)
            { rtn = false; }
            return rtn;
         }
         
      protected:
         bool schedule_on;
         Csi::LgrDate base;
         uint4 interval;
         uint4 max_deviation;

      public:
         ////////////////////////////////////////////////////////////
         // get_schedule_on
         ////////////////////////////////////////////////////////////
         bool get_schedule_on() const
         { return schedule_on; }

         ////////////////////////////////////////////////////////////
         // get_base
         ////////////////////////////////////////////////////////////
         Csi::LgrDate const &get_base() const
         { return base; }

         ////////////////////////////////////////////////////////////
         // get_interval
         ////////////////////////////////////////////////////////////
         uint4 get_interval() const
         { return interval; }
         
         ////////////////////////////////////////////////////////////
         // get_max_deviation
         ////////////////////////////////////////////////////////////
         uint4 get_max_deviation() const
         { return max_deviation; }

         ////////////////////////////////////////////////////////////
         // set
         ////////////////////////////////////////////////////////////
         void set(
            bool schedule_on_,
            Csi::LgrDate const &base_,
            uint4 interval_,
            uint4 max_deviation_)
         {
            schedule_on = schedule_on_;
            base = base_;
            interval = interval_;
            max_deviation = max_deviation_;
         }

         ////////////////////////////////////////////////////////////
         // set_schedule_on
         ////////////////////////////////////////////////////////////
         void set_schedule_on(bool schedule_on_)
         { schedule_on = schedule_on_; }

         ////////////////////////////////////////////////////////////
         // set_base
         ////////////////////////////////////////////////////////////
         void set_base(Csi::LgrDate const &base_)
         { base = base_; }

         ////////////////////////////////////////////////////////////
         // set_interval
         ////////////////////////////////////////////////////////////
         void set_interval(uint4 interval_)
         { interval = interval_; }

         ////////////////////////////////////////////////////////////
         // set_max_deviation
         ////////////////////////////////////////////////////////////
         void set_max_deviation(uint4 max_deviation_)
         { max_deviation = max_deviation_; }
      };


      ////////////////////////////////////////////////////////////
      // class CollectSchedule
      //////////////////////////////////////////////////////////// 
      class CollectSchedule: public Setting
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         CollectSchedule(uint4 setting_id):
            Setting(setting_id)
         { }
            
         ////////////////////////////////////////////////////////////
         // read
         ////////////////////////////////////////////////////////////
         virtual bool read(Csi::Messaging::Message *in);

         ////////////////////////////////////////////////////////////
         // read(string)
         ////////////////////////////////////////////////////////////
         virtual bool read(char const *s);

         ////////////////////////////////////////////////////////////
         // write(message)
         ////////////////////////////////////////////////////////////
         virtual void write(Csi::Messaging::Message *out) const;

         ////////////////////////////////////////////////////////////
         // format (stream)
         ////////////////////////////////////////////////////////////
         virtual void format(std::ostream &out) const;

         virtual json_value_handle write_json() const
         {
            Csi::Json::ObjectHandle rtn(new Csi::Json::Object);
            rtn->set_property_bool("schedule_on", schedule_on);
            rtn->set_property_date("base", base);
            rtn->set_property_number("collect_interval", collect_interval);
            rtn->set_property_number("primary_collect_interval", primary_collect_interval);
            rtn->set_property_number("primary_max_retry_count", primary_max_retry_count);
            rtn->set_property_number("secondary_collect_interval", secondary_collect_interval);
            return rtn.get_handle();
         }

         bool read_json(json_value_handle &val_)
         {
            bool rtn(true);
            try
            {
               Csi::Json::ObjectHandle val(val_);
               schedule_on = val->get_property_bool("schedule_on");
               base = Csi::LgrDate::fromStr(val->get_property_str("base").c_str());
               collect_interval = (uint4)val->get_property_number("collect_interval");
               primary_collect_interval = (uint4)val->get_property_number("primary_collect_interval");
               primary_max_retry_count = (uint4)val->get_property_number("primary_max_retry_count");
               secondary_collect_interval = (uint4)val->get_property_number("secondary_collect_interval");
            }
            catch(std::exception &)
            { rtn = false; }
            return rtn;
         }
         
      private:
         ////////////////////////////////////////////////////////////
         // schedule_on
         ////////////////////////////////////////////////////////////
         bool schedule_on;

         ////////////////////////////////////////////////////////////
         // base
         ////////////////////////////////////////////////////////////
         Csi::LgrDate base;

         ////////////////////////////////////////////////////////////
         // collect_interval
         ////////////////////////////////////////////////////////////
         uint4 collect_interval;

         ////////////////////////////////////////////////////////////
         // primary_collect_interval
         ////////////////////////////////////////////////////////////
         uint4 primary_collect_interval;

         ////////////////////////////////////////////////////////////
         // primary_max_retry_count
         ////////////////////////////////////////////////////////////
         uint4 primary_max_retry_count;

         ////////////////////////////////////////////////////////////
         // secondary_collect_interval
         ////////////////////////////////////////////////////////////
         uint4 secondary_collect_interval;

      public:
         ////////////////////////////////////////////////////////////
         // get_schedule_on
         ////////////////////////////////////////////////////////////
         bool get_schedule_on() const
         { return schedule_on; }

         ////////////////////////////////////////////////////////////
         // set_scheduled_on
         ////////////////////////////////////////////////////////////
         void set_scheduled_on(bool schedule_on_)
         { schedule_on = schedule_on_; } 

         ////////////////////////////////////////////////////////////
         // get_base
         ////////////////////////////////////////////////////////////
         Csi::LgrDate const &get_base() const
         { return base; }

         ////////////////////////////////////////////////////////////
         // set_base
         ////////////////////////////////////////////////////////////
         void set_base(Csi::LgrDate const &base_)
         { base = base_; }

         ////////////////////////////////////////////////////////////
         // get_collect_intevral
         ////////////////////////////////////////////////////////////
         uint4 get_collect_interval() const
         { return collect_interval; }

         ////////////////////////////////////////////////////////////
         // set_collect_interval
         ////////////////////////////////////////////////////////////
         void set_collect_interval(uint4 collect_interval_)
         { collect_interval = collect_interval_; }

         ////////////////////////////////////////////////////////////
         // get_primary_collect_interval
         ////////////////////////////////////////////////////////////
         uint4 get_primary_collect_interval() const
         { return primary_collect_interval; }

         ////////////////////////////////////////////////////////////
         // set_primary_collect_interval
         ////////////////////////////////////////////////////////////
         void set_primary_collect_interval(uint4 interval)
         { primary_collect_interval = interval; } 

         ////////////////////////////////////////////////////////////
         // get_primary_max_retry_count
         ////////////////////////////////////////////////////////////
         uint4 get_primary_max_retry_count() const
         { return primary_max_retry_count; }

         ////////////////////////////////////////////////////////////
         // set_primary_max_retry_count
         ////////////////////////////////////////////////////////////
         void set_primary_max_retry_count(uint4 count)
         { primary_max_retry_count = count; }

         ////////////////////////////////////////////////////////////
         // get_secondary_collect_interval
         ////////////////////////////////////////////////////////////
         uint4 get_secondary_collect_interval() const
         { return secondary_collect_interval; }

         ////////////////////////////////////////////////////////////
         // set_secondary_collect_interval
         ////////////////////////////////////////////////////////////
         void set_secondary_collect_interval(uint4 interval)
         { secondary_collect_interval = interval; }

         ////////////////////////////////////////////////////////////
         // set
         ////////////////////////////////////////////////////////////
         void set(
            bool schedule_on_,
            Csi::LgrDate const &base_,
            uint4 collect_interval_,
            uint4 primary_collect_interval_,
            uint4 primary_max_retry_count_,
            uint4 secondary_collect_interval_)
         {
            schedule_on = schedule_on_;
            base = base_;
            collect_interval = collect_interval_;
            primary_collect_interval = primary_collect_interval_;
            primary_max_retry_count = primary_max_retry_count_;
            secondary_collect_interval = secondary_collect_interval_;
         }
      };


      ////////////////////////////////////////////////////////////
      // class TablesToExclude
      ////////////////////////////////////////////////////////////
      class TablesToExclude: public SettingNameSet
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         //////////////////////////////////////////////////////////// 
         TablesToExclude():
            SettingNameSet(Settings::tables_to_exclude)
         { }
      };


      ////////////////////////////////////////////////////////////
      // class LoggerProgramInfo
      //////////////////////////////////////////////////////////// 
      class LoggerProgramInfo: public Setting
      {
      public:
         LoggerProgramInfo(uint4 identifier):
            Setting(identifier)
         { }
         
         virtual bool read(Csi::Messaging::Message *in);
         virtual bool read(char const *s);
         virtual void write(Csi::Messaging::Message *msg) const;
         virtual void format(std::ostream &out) const;

         virtual json_value_handle write_json() const
         {
            Csi::Json::ObjectHandle rtn(new Csi::Json::Object);
            rtn->set_property_number("resp_code", resp_code);
            rtn->set_property_str("program_name", program_name);
            rtn->set_property_date("when_compiled", when_compiled);
            rtn->set_property_str("result_text", result_text);
            return rtn.get_handle();
         }

         virtual bool read_json(json_value_handle &val_)
         {
            bool rtn(true);
            try
            {
               Csi::Json::ObjectHandle val(val_);
               resp_code = (uint4)val->get_property_number("resp_code");
               program_name = val->get_property_str("program_name");
               when_compiled = Csi::LgrDate::fromStr(val->get_property_str("when_compiled").c_str());
               result_text = val->get_property_str("result_text");
            }
            catch(std::exception &)
            { rtn = false ; }
            return rtn;
         }
         
      private:
         uint4 resp_code;
         StrAsc program_name;
         Csi::LgrDate when_compiled;
         StrAsc result_text;

      public:
         uint4 get_resp_code() const { return resp_code; }
         StrAsc const &get_program_name() const { return program_name; }
         Csi::LgrDate const &get_when_compiled() const { return when_compiled; }
         StrAsc const &get_result_text() const { return result_text; }
         
      };


      ////////////////////////////////////////////////////////////
      // class LowLevelPollSchedule
      //////////////////////////////////////////////////////////// 
      class LowLevelPollSchedule: public Setting
      {
      public:
         LowLevelPollSchedule(uint4 identifier):
            Setting(identifier)
         { }
         
         virtual bool read(Csi::Messaging::Message *msg);
         virtual bool read(char const *s);
         virtual void write(Csi::Messaging::Message *out) const;
         virtual void format(std::ostream &out) const;

         virtual json_value_handle write_json() const
         {
            Csi::Json::ObjectHandle rtn(new Csi::Json::Object);
            rtn->set_property_number("interval", interval);
            rtn->set_property_number("router_offset", router_offset);
            rtn->set_property_number("computer_offset", computer_offset);
            return rtn.get_handle();
         }

         virtual bool read_json(json_value_handle &val_)
         {
            bool rtn(true);
            try
            {
               Csi::Json::ObjectHandle val(val_);
               interval = (uint4)val->get_property_number("interval");
               router_offset = (int4)val->get_property_number("router_offset");
               computer_offset = (int4)val->get_property_number("computer_offset");
            }
            catch(std::exception &)
            { rtn = false; }
            return rtn;
         }
                                   
      private:
         uint4 interval;
         int4 router_offset;
         int4 computer_offset;

      public:
         uint4 get_interval() const { return interval; }
         int4 get_router_offset() const { return router_offset; }
         int4 get_computer_offset() const { return computer_offset; }
         void set(
            uint4 interval_,
            int4 router_offset_,
            int4 computer_offset_)
         {
            interval = interval_;
            router_offset = router_offset_;
            computer_offset = computer_offset_;
         }
      };


      ////////////////////////////////////////////////////////////
      // class DialStringList
      //////////////////////////////////////////////////////////// 
      class DialStringList: public Setting
      {
      private:
         struct Record
         {
            Record():
               delay(0)
            { }

            Record(uint4 delay_, StrAsc const &contents_):
               delay(delay_),
               contents(contents_)
            { }

            Record(Record const &other):
               delay(other.delay),
               contents(other.contents)
            { }
                  
            Record &operator =(Record const &other)
            { delay = other.delay; contents = other.contents; return *this; }
               
            uint4 delay;
            StrAsc contents;
         };
         typedef std::list<Record> records_type;
         records_type records;

      public:
         DialStringList(uint4 identifier):
            Setting(identifier)
         { }
         
         virtual bool read(Csi::Messaging::Message *msg);
         virtual bool read(char const *s);
         virtual void write(Csi::Messaging::Message *out) const;
         virtual void format(std::ostream &out) const;

         virtual json_value_handle write_json() const
         {
            Csi::Json::ArrayHandle rtn(new Csi::Json::Array);
            for(records_type::const_iterator ri = records.begin(); ri != records.end(); ++ri)
            {
               Csi::Json::ObjectHandle record_json(new Csi::Json::Object);
               Record const &record(*ri);
               record_json->set_property_number("delay", record.delay);
               record_json->set_property_str("contents", record.contents);
               rtn->push_back(record_json.get_handle());
            }
            return rtn.get_handle();
         }

         virtual bool read_json(json_value_handle &val_)
         {
            bool rtn(true);
            try
            {
               Csi::Json::ArrayHandle val(val_);
               records.clear();
               for(Csi::Json::Array::iterator ri = val->begin(); ri != val->end(); ++ri)
               {
                  Csi::Json::ObjectHandle record_json(*ri);
                  Record record;
                  record.delay = (uint4)record_json->get_property_number("delay");
                  record.contents = record_json->get_property_str("contents");
                  records.push_back(record);
               }
            }
            catch(std::exception &)
            { rtn = false; }
            return rtn;
         }
      };

      
      ////////////////////////////////////////////////////////////
      // class SettingInlocIds
      //
      // Describes the input location identifiers setting for device collect areas
      ////////////////////////////////////////////////////////////
      class SettingInlocIds: public Setting
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         //////////////////////////////////////////////////////////// 
         SettingInlocIds(uint4 identifier):
            Setting(identifier)
         { }

         ////////////////////////////////////////////////////////////
         // destructor
         //////////////////////////////////////////////////////////// 
         virtual ~SettingInlocIds() { }

         ////////////////////////////////////////////////////////////
         // read
         //////////////////////////////////////////////////////////// 
         virtual bool read(Csi::Messaging::Message *msg);

         ////////////////////////////////////////////////////////////
         // read
         //////////////////////////////////////////////////////////// 
         virtual bool read(char const *s);

         ////////////////////////////////////////////////////////////
         // write
         //////////////////////////////////////////////////////////// 
         virtual void write(Csi::Messaging::Message *msg) const;

         ////////////////////////////////////////////////////////////
         // write
         //////////////////////////////////////////////////////////// 
         virtual void format(std::ostream &out) const;

      public:
         ////////////////////////////////////////////////////////////
         // class record_type
         //
         // Defines the structure that is stored for each label in the setting
         ////////////////////////////////////////////////////////////
         class record_type
         {
         public:
            uint2 logger_id;
            StrUni field_name;

         public:
            record_type() { }
            
            record_type(record_type const &other):
               logger_id(other.logger_id),
               field_name(other.field_name)
            { }
            
            record_type(uint2 logger_id_, StrUni const &field_name_):
               logger_id(logger_id_),
               field_name(field_name_)
            { }
         };

         ////////////////////////////////////////////////////////////
         // records container access methods
         ////////////////////////////////////////////////////////////
         typedef std::list<record_type> records_type;
         typedef records_type::const_iterator iterator;
         typedef records_type::const_reverse_iterator reverse_iterator;
         iterator begin() const { return records.begin(); }
         iterator end() const { return records.end(); }
         reverse_iterator rbegin() const { return records.rbegin(); }
         reverse_iterator rend() const { return records.rend(); }
         records_type::size_type size() const { return records.size(); }
         bool empty() const { return records.empty(); }

         virtual json_value_handle write_json() const
         {
            Csi::Json::ArrayHandle rtn(new Csi::Json::Array);
            for(iterator ri = begin(); ri != end(); ++ri)
            {
               record_type const &record(*ri);
               Csi::Json::ObjectHandle record_json(new Csi::Json::Object);
               record_json->set_property_number("logger_id", record.logger_id);
               record_json->set_property_str("field_name", record.field_name.to_utf8());
               rtn->push_back(record_json.get_handle());
            }
            return rtn.get_handle();
         }

         virtual bool read_json(json_value_handle &val_)
         {
            bool rtn(true);
            try
            {
               Csi::Json::ArrayHandle val(val_);
               records.clear();
               for(Csi::Json::Array::iterator ri = val->begin(); ri != val->end(); ++ri)
               {
                  Csi::Json::ObjectHandle record_json(*ri);
                  records.push_back(
                     record_type(
                        (uint2)record_json->get_property_number("logger_id"),
                        record_json->get_property_str("field_name")));
               }
            }
            catch(std::exception &)
            { rtn = false; }
            return rtn;
         }
         
      private: 
         ////////////////////////////////////////////////////////////
         // records
         //
         // The list of records read from the datalogger setting
         ////////////////////////////////////////////////////////////
         records_type records;
      };


      ////////////////////////////////////////////////////////////
      // class SettingGenericRtsCtsUse
      //
      // Defines the default class used to represent the GenericRtsCtsUse setting for generic
      // modems.
      ////////////////////////////////////////////////////////////
      class SettingGenericRtsCtsUse: public SettingEnumeration
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         //////////////////////////////////////////////////////////// 
         SettingGenericRtsCtsUse():
            SettingEnumeration(Settings::generic_rts_cts_use)
         {
            supported_values[1] = "flow-control";
            supported_values[2] = "raise-rts";
            supported_values[3] = "lower-rts";
         }
      };


      ////////////////////////////////////////////////////////////
      // class TapiCountryCode
      ////////////////////////////////////////////////////////////
      class TapiCountryCode: public Setting
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         TapiCountryCode(uint4 identifier):
            Setting(identifier),
            country_code(1),
            country_name("United States of America")
         { }

         ////////////////////////////////////////////////////////////
         // read (message)
         ////////////////////////////////////////////////////////////
         virtual bool read(Csi::Messaging::Message *message);

         ////////////////////////////////////////////////////////////
         // read (string)
         ////////////////////////////////////////////////////////////
         virtual bool read(char const *s);

         ////////////////////////////////////////////////////////////
         // write
         ////////////////////////////////////////////////////////////
         virtual void write(Csi::Messaging::Message *message) const;

         ////////////////////////////////////////////////////////////
         // format
         ////////////////////////////////////////////////////////////
         virtual void format(std::ostream &out) const;

         virtual json_value_handle write_json() const
         {
            Csi::Json::ObjectHandle rtn(new Csi::Json::Object);
            rtn->set_property_number("country_code", country_code);
            rtn->set_property_str("country_name", country_name);
            return rtn.get_handle();
         }

         virtual bool read_json(json_value_handle &val_)
         {
            bool rtn(true);
            try
            {
               Csi::Json::ObjectHandle val(val_);
               country_code = (uint4)val->get_property_number("country_code");
               country_name = val->get_property_str("country_name");
            }
            catch(std::exception &)
            { rtn = false; }
            return rtn;
         }
               

      private:
         ////////////////////////////////////////////////////////////
         // country_code
         ////////////////////////////////////////////////////////////
         uint4 country_code;

         ////////////////////////////////////////////////////////////
         // country_name
         ////////////////////////////////////////////////////////////
         StrAsc country_name;
      };


      ////////////////////////////////////////////////////////////
      // class SettingTableDefsPolicy
      ////////////////////////////////////////////////////////////
      class SettingTableDefsPolicy: public SettingEnumeration
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         SettingTableDefsPolicy(uint4 setting_id = Settings::table_defs_policy):
            SettingEnumeration(setting_id)
         {
            supported_values[1] = "manual";
            supported_values[2] = "automatic";
         }
      };


      ////////////////////////////////////////////////////////////
      // class PakbusAddressesList
      ////////////////////////////////////////////////////////////
      class PakbusAddressesList: public Setting
      {
      protected:
         ////////////////////////////////////////////////////////////
         // ranges
         //
         // The range of addresses assigned to this setting. 
         ////////////////////////////////////////////////////////////
         typedef Csi::RangeList ranges_type;
         ranges_type ranges;
         
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         PakbusAddressesList(uint4 setting_id):
            Setting(setting_id)
         { }

         ////////////////////////////////////////////////////////////
         // format
         ////////////////////////////////////////////////////////////
         virtual void format(std::ostream &out) const;

         ////////////////////////////////////////////////////////////
         // read
         ////////////////////////////////////////////////////////////
         virtual bool read(char const *source);
         
         ////////////////////////////////////////////////////////////
         // read
         ////////////////////////////////////////////////////////////
         virtual bool read(Csi::Messaging::Message *message);

         ////////////////////////////////////////////////////////////
         // write
         ////////////////////////////////////////////////////////////
         virtual void write(Csi::Messaging::Message *message) const;

         virtual json_value_handle write_json() const
         {
            Csi::Json::ArrayHandle rtn(new Csi::Json::Array);
            for(ranges_type::const_iterator ri = ranges.begin(); ri != ranges.end(); ++ri)
            {
               Csi::Json::ArrayHandle range(new Csi::Json::Array);
               range->push_back(new Csi::Json::Number(ri->first));
               range->push_back(new Csi::Json::Number(ri->second));
               rtn->push_back(range.get_handle());
            }
            return rtn.get_handle();
         }

         virtual bool read_json(json_value_handle &val_)
         {
            bool rtn(true);
            try
            {
               Csi::Json::ArrayHandle val(val_);
               ranges.clear();
               for(Csi::Json::Array::iterator ri = val->begin(); ri != val->end(); ++ri)
               {
                  Csi::Json::ArrayHandle range(*ri);
                  Csi::Json::NumberHandle r1(range->at(0));
                  Csi::Json::NumberHandle r2(range->at(1));
                  ranges.add_range(r1->get_value_uint2(), r2->get_value_uint2());
               }
            }
            catch(std::exception &)
            { rtn = false; }
            return rtn;
         }
      };

      
      ////////////////////////////////////////////////////////////
      // class AllowedPakbusNeighbours
      ////////////////////////////////////////////////////////////
      class AllowedPakbusNeighbours: public PakbusAddressesList
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         AllowedPakbusNeighbours():
            PakbusAddressesList(Settings::allowed_pakbus_neighbours)
         { ranges.add_range(1,4094); }
      };


      ////////////////////////////////////////////////////////////
      // class SettingFileSynchControl
      ////////////////////////////////////////////////////////////
      class SettingFileSynchControl: public Setting
      {
      public:
         ////////////////////////////////////////////////////////////
         // struct file
         ////////////////////////////////////////////////////////////
         struct file
         {
            StrAsc source_pattern;
            StrAsc dest_dir;
            bool force;
         };

         ////////////////////////////////////////////////////////////
         // files
         ////////////////////////////////////////////////////////////
         typedef std::list<file> files_type;
         files_type files;

      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         SettingFileSynchControl():
            Setting(Settings::file_synch_control)
         { }

         ////////////////////////////////////////////////////////////
         // format
         ////////////////////////////////////////////////////////////
         virtual void format(std::ostream &out) const;

         ////////////////////////////////////////////////////////////
         // read
         ////////////////////////////////////////////////////////////
         virtual bool read(char const *source);

         ////////////////////////////////////////////////////////////
         // read
         ////////////////////////////////////////////////////////////
         virtual bool read(Csi::Messaging::Message *message);

         ////////////////////////////////////////////////////////////
         // write
         ////////////////////////////////////////////////////////////
         virtual void write(Csi::Messaging::Message *message) const;

         virtual json_value_handle write_json() const
         {
            Csi::Json::ArrayHandle rtn(new Csi::Json::Array);
            for(files_type::const_iterator fi = files.begin(); fi != files.end(); ++fi)
            {
               Csi::Json::ObjectHandle file_json(new Csi::Json::Object);
               file_json->set_property_str("source_pattern", fi->source_pattern);
               file_json->set_property_str("dest_dir", fi->dest_dir);
               file_json->set_property_bool("force", fi->force);
               rtn->push_back(file_json.get_handle());
            }
            return rtn.get_handle();
         }

         virtual bool read_json(json_value_handle &val_)
         {
            bool rtn(true);
            try
            {
               Csi::Json::ArrayHandle val(val_);
               files.clear();
               for(Csi::Json::Array::iterator fi = val->begin(); fi != val->end(); ++fi)
               {
                  Csi::Json::ObjectHandle file_json(*fi);
                  files.push_back({
                        file_json->get_property_str("source_pattern"),
                           file_json->get_property_str("dest_dir"),
                           file_json->get_property_bool("force")});
               }
            }
            catch(std::exception &)
            { rtn = false; }
            return rtn;
         }
      };


      ////////////////////////////////////////////////////////////
      // class FileSynchMode
      ////////////////////////////////////////////////////////////
      class FileSynchMode: public SettingEnumeration
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         FileSynchMode(uint4 setting_id = Settings::file_synch_mode):
            SettingEnumeration(setting_id)
         {
            supported_values[1] = "disabled";
            supported_values[2] = "bound-to-data";
            supported_values[3] = "independent";
         }
      };


      ////////////////////////////////////////////////////////////
      //  class FileSynchControlEx
      ////////////////////////////////////////////////////////////
      class FileSynchControlEx: public Setting
      {
      public:
         ////////////////////////////////////////////////////////////
         // struct file_type
         ////////////////////////////////////////////////////////////
         struct file_type
         {
            StrAsc source_pattern;
            StrAsc dest_dir;
            bool force;
            uint4 max_files;
            bool record_if_skipped;

            file_type():
               force(false),
               max_files(0xffffffff),
               record_if_skipped(false)
            { }

            file_type(file_type const &other):
               source_pattern(other.source_pattern),
               dest_dir(other.dest_dir),
               force(other.force),
               max_files(other.max_files),
               record_if_skipped(other.record_if_skipped)
            { }

            file_type &operator =(file_type const &other)
            {
               source_pattern = other.source_pattern;
               dest_dir = other.dest_dir;
               force = other.force;
               max_files = other.max_files;
               record_if_skipped = other.record_if_skipped;
               return *this;
            }
         };

         ////////////////////////////////////////////////////////////
         // files
         ////////////////////////////////////////////////////////////
         typedef std::list<file_type> files_type;
         files_type files;

      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         FileSynchControlEx(uint4 setting_id = Settings::file_synch_control_ex):
            Setting(setting_id)
         { }

         ////////////////////////////////////////////////////////////
         // format
         ////////////////////////////////////////////////////////////
         virtual void format(std::ostream &out) const;

         ////////////////////////////////////////////////////////////
         // read
         ////////////////////////////////////////////////////////////
         virtual bool read(char const *s);

         ////////////////////////////////////////////////////////////
         // read (message)
         ////////////////////////////////////////////////////////////
         virtual bool read(Csi::Messaging::Message *message);

         ////////////////////////////////////////////////////////////
         // write (message)
         ////////////////////////////////////////////////////////////
         virtual void write(Csi::Messaging::Message *message) const;

         virtual json_value_handle write_json() const
         {
            Csi::Json::ArrayHandle rtn(new Csi::Json::Array);
            for(files_type::const_iterator fi = files.begin(); fi != files.end(); ++fi)
            {
               Csi::Json::ObjectHandle file_json(new Csi::Json::Object);
               file_json->set_property_str("source_pattern", fi->source_pattern);
               file_json->set_property_str("dest_dir", fi->dest_dir);
               file_json->set_property_bool("force", fi->force);
               file_json->set_property_number("max_files", fi->max_files);
               file_json->set_property_bool("record_if_skipped", fi->record_if_skipped);
               rtn->push_back(file_json.get_handle());
            }
            return rtn.get_handle();
         }

         virtual bool read_json(json_value_handle &val_)
         {
            bool rtn(true);
            try
            {
               Csi::Json::ArrayHandle val(val_);
               files.clear();
               for(Csi::Json::Array::iterator fi = val->begin(); fi != val->end(); ++fi)
               {
                  file_type file;
                  Csi::Json::ObjectHandle file_json(*fi);
                  file.source_pattern = file_json->get_property_str("source_pattern");
                  file.dest_dir = file_json->get_property_str("dest_dir");
                  file.force = file_json->get_property_bool("force");
                  file.max_files = (uint4)file_json->get_property_number("max_files");
                  file.record_if_skipped = file_json->get_property_bool("record_if_skipped");
                  files.push_back(file);
               }
            }
            catch(std::exception &)
            { rtn = false; }
            return rtn;
         }
      };


      ////////////////////////////////////////////////////////////
      // class PakbusTcpOutAddresses
      ////////////////////////////////////////////////////////////
      class PakbusTcpOutAddresses: public Setting
      {
      public:
         ////////////////////////////////////////////////////////////
         // addresses
         ////////////////////////////////////////////////////////////
         typedef std::map<uint2, StrAsc> addresses_type;
         typedef addresses_type::value_type value_type;
         addresses_type addresses;

         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         PakbusTcpOutAddresses():
            Setting(Settings::pakbus_tcp_out_addresses)
         { }

         ////////////////////////////////////////////////////////////
         // format
         ////////////////////////////////////////////////////////////
         virtual void format(std::ostream &out) const;

         ////////////////////////////////////////////////////////////
         // read
         ////////////////////////////////////////////////////////////
         virtual bool read(char const *s);

         ////////////////////////////////////////////////////////////
         // read (message)
         ////////////////////////////////////////////////////////////
         virtual bool read(Csi::Messaging::Message *message);

         ////////////////////////////////////////////////////////////
         // write (message)
         ////////////////////////////////////////////////////////////
         virtual void write(Csi::Messaging::Message *message) const;

         virtual json_value_handle write_json() const
         {
            Csi::Json::ArrayHandle rtn(new Csi::Json::Array);
            for(addresses_type::const_iterator ai = addresses.begin(); ai != addresses.end(); ++ai)
            {
               Csi::Json::ObjectHandle address_json(new Csi::Json::Object);
               address_json->set_property_number("pakbus_address", ai->first);
               address_json->set_property_str("ip_address", ai->second);
               rtn->push_back(address_json.get_handle());
            }
            return rtn.get_handle();
         }

         virtual bool read_json(json_value_handle &val_)
         {
            bool rtn(true);
            try
            {
               Csi::Json::ArrayHandle val(val_);
               addresses.clear();
               for(Csi::Json::Array::iterator ai = val->begin(); ai != val->end(); ++ai)
               {
                  Csi::Json::ObjectHandle address_json(*ai);
                  addresses[(uint2)address_json->get_property_number("pakbus_address")] = address_json->get_property_str("ip_address");
               }
            }
            catch(std::exception &)
            { rtn = false; }
            return rtn;
         }
      };


      ////////////////////////////////////////////////////////////
      // class ComPortIdSetting
      ////////////////////////////////////////////////////////////
      class ComPortIdSetting: public SettingStrAsc
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         ComPortIdSetting(uint4 setting_id = Settings::com_port_id):
            SettingStrAsc(setting_id)
         { }
         
         ////////////////////////////////////////////////////////////
         // read (string)
         ////////////////////////////////////////////////////////////
         virtual bool read(char const *s);
      };


      ////////////////////////////////////////////////////////////
      // class PooledTerminalServersSetting
      //
      // Defines an object that represents an instance of the
      // pooledTerminalServers setting.
      ////////////////////////////////////////////////////////////
      class PooledTerminalServersSetting: public Setting
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         PooledTerminalServersSetting():
            Setting(Settings::pooled_terminal_servers)
         { }

         ////////////////////////////////////////////////////////////
         // read
         ////////////////////////////////////////////////////////////
         virtual bool read(char const *s);

         ////////////////////////////////////////////////////////////
         // format
         ////////////////////////////////////////////////////////////
         virtual void format(std::ostream &out) const;

         ////////////////////////////////////////////////////////////
         // read
         ////////////////////////////////////////////////////////////
         virtual bool read(Csi::Messaging::Message *in);

         ////////////////////////////////////////////////////////////
         // write
         ////////////////////////////////////////////////////////////
         virtual void write(Csi::Messaging::Message *out) const;

         // @group: declarations to act as a container of resources

         ////////////////////////////////////////////////////////////
         // begin
         ////////////////////////////////////////////////////////////
         typedef std::pair<StrAsc, bool> value_type;
         typedef std::list<value_type> resources_type;
         typedef resources_type::iterator iterator;
         typedef resources_type::const_iterator const_iterator;
         iterator begin()
         { return resources.begin(); }
         const_iterator begin() const
         { return resources.begin(); }

         ////////////////////////////////////////////////////////////
         // end
         ////////////////////////////////////////////////////////////
         iterator end()
         { return resources.end(); }
         const_iterator end() const
         { return resources.end(); }

         ////////////////////////////////////////////////////////////
         // empty
         ////////////////////////////////////////////////////////////
         bool empty() const
         { return resources.empty(); }

         ////////////////////////////////////////////////////////////
         // size
         ////////////////////////////////////////////////////////////
         typedef resources_type::size_type size_type;
         size_type size() const
         { return resources.size(); }

         ////////////////////////////////////////////////////////////
         // clear
         ////////////////////////////////////////////////////////////
         void clear()
         { resources.clear(); }

         ////////////////////////////////////////////////////////////
         // push_back
         ////////////////////////////////////////////////////////////
         void push_back(value_type const &value)
         { resources.push_back(value); }
         void push_back(StrAsc const &address, bool use_rfc2217 = false)
         { resources.push_back(value_type(address, use_rfc2217)); }
         
         ////////////////////////////////////////////////////////////
         // push_front
         ////////////////////////////////////////////////////////////
         void push_front(value_type const &value)
         { resources.push_front(value); }
         void push_front(StrAsc const &address, bool use_rfc2217 = false)
         { resources.push_front(value_type(address, use_rfc2217)); }

         ////////////////////////////////////////////////////////////
         // pop_front
         ////////////////////////////////////////////////////////////
         void pop_front()
         {
            if(!resources.empty())
               resources.pop_front();
         }

         ////////////////////////////////////////////////////////////
         // pop_back
         ////////////////////////////////////////////////////////////
         void pop_back()
         {
            if(!resources.empty())
               resources.pop_back();
         }
         
         // @endgroup:

         virtual json_value_handle write_json() const
         {
            Csi::Json::ArrayHandle rtn(new Csi::Json::Array);
            for(const_iterator ri = begin(); ri != end(); ++ri)
            {
               Csi::Json::ObjectHandle resource(new Csi::Json::Object);
               resource->set_property_str("address", ri->first);
               resource->set_property_bool("use_rfc2217", ri->second);
               rtn->push_back(resource.get_handle());
            }
            return rtn.get_handle();
         }

         virtual bool read_json(json_value_handle &val_)
         {
            bool rtn(true);
            try
            {
               Csi::Json::ArrayHandle val(val_);
               resources.clear();
               for(Csi::Json::Array::iterator ri = val->begin(); ri != val->end(); ++ri)
               {
                  Csi::Json::ObjectHandle resource(*ri);
                  resources.push_back(
                     value_type(
                        resource->get_property_str("address"),
                        resource->get_property_bool("use_rfc2217")));
               }
            }
            catch(std::exception &)
            { rtn = false; }
            return rtn;
         }
         
      private:
         ////////////////////////////////////////////////////////////
         // resources
         ////////////////////////////////////////////////////////////
         resources_type resources;
      };


      ////////////////////////////////////////////////////////////
      // class SettingTableFileStationNameSelector
      ////////////////////////////////////////////////////////////
      class SettingTableFileStationNameSelector: public SettingEnumeration
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         SettingTableFileStationNameSelector(uint4 id = Settings::table_file_station_name_selector):
            SettingEnumeration(id)
         {
            supported_values[1] = "use-network-map";
            supported_values[2] = "use-datalogger-reported";
         }
      };


      /**
       * Defines a string based setting that represents a URL.
       */
      class PakbusWsServerUrl: public SettingStrAsc
      {
      public:
         /**
          * Constructor
          */
         PakbusWsServerUrl():
            SettingStrAsc(Settings::pakbus_ws_server_url, "ws://localhost:8080")
         { }

      protected:
         /**
          * @return Overloads the base class version to validate the content.
          */
         virtual bool validate(StrAsc const &val);
      };


      /**
       * Defines the export type setting for aloha receivers.
       */
      class AlohaExportType: public SettingEnumeration
      {
      public:
         /**
          * Constructor
          *
          * @param setting_id Specifies the device setting identifier.
          */
         AlohaExportType(uint4 setting_id = Settings::aloha_export_type):
            SettingEnumeration(setting_id)
         {
            supported_values[1] = "disabled";
            supported_values[2] = "serial-port";
            supported_values[3] = "tcp-client";
            supported_values[4] = "tcp-server";
         }
      };


      /**
       * Defines the export status type setting for ALOHA receivers
       */
      class AlohaExportStatusType: public SettingEnumeration
      {
      public:
         /**
          * Constructor
          */
         AlohaExportStatusType():
            SettingEnumeration(Settings::aloha_export_status)
         {
            supported_values[1] = "disabled";
            supported_values[2] = "active";
            supported_values[3] = "failed";
         }
      };
   };
};

#endif

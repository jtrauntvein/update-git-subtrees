/* Cora.DataSources.Request.cpp

   Copyright (C) 2008, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 04 August 2008
   Last Change: Friday 29 April 2016
   Last Commit: $Date: 2016-05-03 16:18:03 -0600 (Tue, 03 May 2016) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.DataSources.Request.h"
#include "Cora.DataSources.SinkBase.h"
#include "Cora.DataSources.SourceBase.h"
#include "Cora.Broker.RecordDesc.h" 
#include <algorithm>
#include <functional>


namespace Cora
{
   namespace DataSources
   {
      ////////////////////////////////////////////////////////////
      // class Request definitions
      ////////////////////////////////////////////////////////////
      Request::Request(SinkBase *sink_, StrUni const &uri_):
         sink(sink_),
         source(0),
         uri(uri_),
         state(state_inactive),
         start_option(start_at_newest),
         order_option(order_real_time),
         file_mark_no(0),
         record_no(0),
         backfill_interval(0),
         start_record_offset(0),
         cache_size_controller(1024),
         frozen(false),
         use_table_index(false),
         expect_more_data(true),
         cacheable(false),
         begin_index(0),
         end_index(0),
         report_offset(0),
         app_data(0)
      { }


      Request::Request(Request const &request):
         sink(request.sink),
         source(request.source),
         state(request.state),
         uri(request.uri),
         start_option(request.start_option),
         order_option(request.order_option),
         file_mark_no(request.file_mark_no),
         record_no(request.record_no),
         start_time(request.start_time),
         end_time(request.end_time),
         backfill_interval(request.backfill_interval),
         start_record_offset(request.start_record_offset),
         cache_size_controller(request.cache_size_controller),
         frozen(request.frozen),
         use_table_index(request.use_table_index),
         expect_more_data(true),
         cacheable(request.cacheable),
         js_name(request.js_name),
         begin_index(0),
         end_index(0),
         report_offset(request.report_offset),
         app_data(0)
      { }


      Request &Request::operator =(Request const &other)
      {
         sink = other.sink;
         source = other.source;
         state = other.state;
         uri = other.uri;
         start_option = other.start_option;
         order_option = other.order_option;
         file_mark_no = other.file_mark_no;
         record_no = other.record_no;
         start_time = other.start_time;
         end_time = other.end_time;
         backfill_interval = other.backfill_interval;
         start_record_offset = other.start_record_offset;
         cache_size_controller = other.cache_size_controller;
         frozen = other.frozen;
         use_table_index = other.use_table_index;
         expect_more_data = other.expect_more_data;
         cacheable = other.cacheable;
         js_name = other.js_name;
         begin_index = other.begin_index;
         end_index = other.end_index;
         report_offset = report_offset;
         app_data = 0;
         return *this;
      } // copy operator


      bool Request::operator ==(Request const &other) const
      {
         bool rtn =
            sink == other.sink &&
            start_option == other.start_option &&
            uri == other.uri &&
            js_name == other.js_name &&
            begin_index == other.begin_index &&
            end_index == other.end_index;

         if(rtn)
         {
            switch(start_option)
            {
            case start_at_record:
               rtn =
                  file_mark_no == other.file_mark_no &&
                  record_no == other.record_no;
               break;
               
            case start_at_time:
               rtn = start_time == other.start_time;
               break;
               
            case start_at_newest:
            case start_after_newest:
               break;
               
            case start_relative_to_newest:
               rtn = backfill_interval == other.backfill_interval;
               break;
               
            case start_at_offset_from_newest:
               rtn = start_record_offset == other.start_record_offset;
               break;

            case start_date_query:
               rtn = start_time == other.start_time && end_time == other.end_time;
               break;
            }
         }
         return rtn;
      } // equality operator


      void Request::set_state(SourceBase *source_, state_type state_)
      {
         source = source_;
         state = state_;
         if(SinkBase::is_valid_instance(sink) && source != 0)
            sink->on_request_state_change(source->get_manager(), this);
      } // set_state


      void Request::set_start_at_record(
         uint4 file_mark_no_,
         uint4 record_no_,
         Csi::LgrDate const &start_time_)
      {
         if(state == state_inactive || state == state_remove_pending)
         {
            if(!frozen)
            {
               start_option = start_at_record;
               file_mark_no = file_mark_no_;
               record_no = record_no_;
               start_time = start_time_;
            }
         }
         else
            throw std::invalid_argument("request already started");
      } // set_start_at_record


      void Request::set_start_at_time(Csi::LgrDate const &start_time_)
      {
         if(state == state_inactive || state == state_remove_pending)
         {
            if(!frozen)
            {
               start_option = start_at_time;
               start_time = start_time_;
            }
         }
         else
            throw std::invalid_argument("request already started"); 
      } // set_start_at_time


      void Request::set_start_at_newest()
      {
         if(state == state_inactive || state == state_remove_pending)
         {
            if(!frozen)
               start_option = start_at_newest;
         }
         else
            throw std::invalid_argument("request already started");
      } // set_start_at_newest


      void Request::set_start_after_newest()
      {
         if(state == state_inactive || state == state_remove_pending)
         {
            if(!frozen)
               start_option = start_after_newest;
         }
         else
            throw std::invalid_argument("request already started");
      } // set_start_after_newest


      void Request::set_start_relative_to_newest(int8 backfill_interval_)
      {
         if(state == state_inactive || state == state_remove_pending)
         {
            if(!frozen)
            {
               start_option = start_relative_to_newest;
               backfill_interval = backfill_interval_;
            }
         }
         else
            throw std::invalid_argument("request already started");
      } // set_start_relative_to_newest


      void Request::set_start_at_offset_from_newest(uint4 start_record_offset_)
      {
         if(state == state_inactive || state == state_remove_pending)
         {
            if(!frozen)
            {
               start_option = start_at_offset_from_newest;
               start_record_offset = start_record_offset_;
            }
         }
         else
            throw std::invalid_argument("request already started");
      } // set_start_at_offset_from_newest


      void Request::set_query_times(Csi::LgrDate const &begin, Csi::LgrDate const &end)
      {
         if(state == state_inactive || state == state_remove_pending)
         {
            if(!frozen)
            {
               start_time = begin;
               end_time = end;
               start_option = start_date_query;
            }
         }
         else
            throw std::invalid_argument("request already started");
      } // set_query_times


      bool Request::is_valid_order_option(order_option_type option)
      {
         return option >= order_collected && option <= order_real_time;
      } // is_valid_order_option

      
      void Request::set_order_option(order_option_type order_option_)
      {
         if(state == state_inactive || state == state_remove_pending)
         {
            if(!frozen)
               order_option = order_option_;
         }
         else
            throw std::invalid_argument("request already started");
      } // set_order_option


      namespace
      {
         ////////////////////////////////////////////////////////////
         // class ValueName
         ////////////////////////////////////////////////////////////
         class ValueName
         {
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            ValueName(StrUni const &s)
            { parse(s); }

            ////////////////////////////////////////////////////////////
            // parse
            ////////////////////////////////////////////////////////////
            void parse(StrUni const &s);

            ////////////////////////////////////////////////////////////
            // get_column_name
            ////////////////////////////////////////////////////////////
            StrUni const &get_column_name() const
            { return column_name; }

            ////////////////////////////////////////////////////////////
            // get_subscripts
            ////////////////////////////////////////////////////////////
            typedef std::vector<uint4> subscripts_type;
            subscripts_type const &get_subscripts() const
            { return subscripts; }
            subscripts_type &get_subscripts()
            { return subscripts; }

            // @group: subscripts container methods

            ////////////////////////////////////////////////////////////
            // begin
            ////////////////////////////////////////////////////////////
            typedef subscripts_type::iterator iterator;
            typedef subscripts_type::const_iterator const_iterator;
            iterator begin()
            { return subscripts.begin(); }
            const_iterator begin() const
            { return subscripts.end(); }

            ////////////////////////////////////////////////////////////
            // end
            ////////////////////////////////////////////////////////////
            iterator end()
            { return subscripts.end(); }
            const_iterator end() const
            { return subscripts.end(); }

            ////////////////////////////////////////////////////////////
            // empty
            ////////////////////////////////////////////////////////////
            bool empty() const
            { return subscripts.empty(); }

            ////////////////////////////////////////////////////////////
            // size
            ////////////////////////////////////////////////////////////
            typedef subscripts_type::size_type size_type;
            size_type size() const
            { return subscripts.size(); }
            
            // @endgroup
            
         private:
            ////////////////////////////////////////////////////////////
            // column_name
            ////////////////////////////////////////////////////////////
            StrUni column_name;

            ////////////////////////////////////////////////////////////
            // subscripts
            ////////////////////////////////////////////////////////////
            subscripts_type subscripts;
         };


         void ValueName::parse(StrUni const &s)
         {
            enum state_type
            {
               state_in_name,
               state_before_subscript,
               state_in_subscript,
               state_after_subscript,
               state_complete
            } state = state_in_name;
            size_t i = 0;
            StrUni temp;
            size_t subscripts_begin = s.length();
            bool advance_i = false;
            
            column_name.cut(0);
            subscripts.clear();
            while(i < s.length() && state != state_complete)
            {
               advance_i = true;
               switch(state)
               {
               case state_in_name:
                  if(s[i] == L'(')
                  {
                     state = state_before_subscript;
                     subscripts_begin = i;
                  }
                  break;

               case state_before_subscript:
                  switch(s[i])
                  {
                  case L'0':
                  case L'1':
                  case L'2':
                  case L'3':
                  case L'4':
                  case L'5':
                  case L'6':
                  case L'7':
                  case L'8':
                  case L'9':
                     state = state_in_subscript;
                     advance_i = false;
                     temp.cut(0);
                     break;

                  default:
                     if(!isspace(s[i]))
                        throw std::invalid_argument("invalid value name syntax");
                     break;
                  }
                  break;

               case state_in_subscript:
                  switch(s[i])
                  {
                  case L'0':
                  case L'1':
                  case L'2':
                  case L'3':
                  case L'4':
                  case L'5':
                  case L'6':
                  case L'7':
                  case L'8':
                  case L'9':
                     temp.append(s[i]);
                     break;

                  case L',':
                     if(temp.length())
                        subscripts.push_back(wcstoul(temp.c_str(), 0, 10));
                     else
                        throw std::invalid_argument("invalid column name syntax");
                     state = state_before_subscript;
                     break;

                  case L')':
                     if(temp.length())
                        subscripts.push_back(wcstoul(temp.c_str(), 0, 10));
                     else
                        throw std::invalid_argument("invalid column name syntax");
                     state = state_complete;
                     break;

                  default:
                     if(isspace(s[i]) && temp.length())
                     {
                        subscripts.push_back(wcstoul(temp.c_str(), 0, 10));
                        state = state_after_subscript;
                     }
                     else
                        throw std::invalid_argument("invalid column name syntax");
                     break;
                  }
                  break;

               case state_after_subscript:
                  if(s[i] == L',')
                     state = state_before_subscript;
                  else if(!isspace(s[i]))
                     throw std::invalid_argument("invalid column name syntax");
                  break;
               }

               if(advance_i)
                  ++i;
            }
            if(subscripts_begin < s.length())
               s.sub(column_name, 0, subscripts_begin);
            else
               column_name = s;
         } // parse

         
         ////////////////////////////////////////////////////////////
         // predicate value_has_name
         ////////////////////////////////////////////////////////////
         struct value_has_name
         {
            ValueName const &value_name;
            value_has_name(ValueName const &value_name_):
               value_name(value_name_)
            { }

            typedef Cora::Broker::Record::value_type argument_type;
            bool operator ()(argument_type const &value) const
            {
               Cora::Broker::Value::desc_handle const &desc = value->get_description();
               bool rtn = desc->name == value_name.get_column_name().c_str();
               if(rtn && !value_name.empty() && !desc->empty())
               {
                  Broker::ValueDesc::array_address_type desc_address(desc->array_address);
                  if(value->get_combined_with_adjacent_values())
                     desc_address.pop_back();
                  if(value_name.get_subscripts() != desc_address)
                     rtn = false;
               }
               else if(rtn && !value_name.empty() && desc->empty())
                  rtn = false;
               return rtn;
            }
         };
      };


      void Request::copy_start_options(Request const &other)
      {
         start_option = other.start_option;
         order_option = other.order_option;
         file_mark_no = other.file_mark_no;
         record_no = other.record_no;
         start_time = other.start_time;
         end_time = other.end_time;
         backfill_interval = other.backfill_interval;
         start_record_offset = other.start_record_offset;
         cache_size_controller = other.cache_size_controller; 
         js_name = other.js_name;
         report_offset = other.report_offset;
      } // copy_start_options

      
      void Request::set_value_indices(
         Cora::Broker::Record const &record,
         StrUni const &column_name)
      {
         if(column_name.length() > 0)
         {
            using namespace Cora::Broker;
            ValueName value_name(column_name.c_str());
            Record::const_iterator vi = std::find_if(
               record.begin(), record.end(), value_has_name(value_name));
            begin_index = (uint4)std::distance(record.begin(), vi);
            end_index = (uint4)record.size();
            if(vi != record.end())
            {
               // Our search above either located the first value with a column name or the value
               // with the column name and array address.  If the request did not specify an
               // array address and the value has subscripts, we need to find the last value that
               // has the column name. 
               Record::value_type const &value = *vi;
               Value::desc_handle const &desc = value->get_description();
               end_index = begin_index + 1;
               if(value_name.empty() && !desc->empty() && vi != record.end())
               {
                  // we will need to scan forward to find the last value that has the specified
                  // name
                  vi = std::find_if(
                     vi, record.end(), std::not1(value_has_name(value_name)));
                  end_index = (uint4)std::distance(record.begin(), vi);
               }
            }
         }
         else
         {
            begin_index = 0;
            end_index = (uint4)record.size();
         }
      } // set_value_indices


      StrUni Request::get_source_name() const
      {
         StrUni rtn;
         size_t colon_pos = uri.find(L":");
         size_t start_pos(0);
         if(uri.first() == L'\"')
         {
            start_pos = 1;
            --colon_pos ;
         }
         if(colon_pos < uri.length())
            uri.sub(rtn, start_pos, colon_pos);
         return rtn;
      } // get_source_name


      bool Request::is_compatible(Request const &other) const
      {
         bool rtn = false;
         if(start_option == other.start_option)
         {
            switch(start_option)
            {
            case start_at_record:
               if(order_option == other.order_option &&
                  file_mark_no == other.file_mark_no &&
                  record_no == other.record_no)
               {
                  if(start_time == 0 ||
                     other.start_time == 0 ||
                     start_time == other.start_time)
                  {
                     rtn = true;
                  }
               }
               break;
               
            case start_at_time:
               if(order_option == other.order_option && start_time == other.start_time)
                  rtn = true;
               break;
               
            case start_at_newest:
            case start_after_newest:
               if(order_option == other.order_option)
                  rtn = true;
               break;
                  
            case start_relative_to_newest:
               if(order_option == other.order_option && backfill_interval == other.backfill_interval)
                  rtn = true;
               break;
               
            case start_at_offset_from_newest:
               if(order_option == other.order_option && start_record_offset == other.start_record_offset)
                  rtn = true;
               break;
               
            case start_date_query:
               if(start_time == other.start_time && end_time == other.end_time)
                  rtn = true;
               break;
            }
         }
         return rtn;
      } // is_compatible


      void Request::set_use_table_index(bool use_table_index_)
      {
         if(state == state_inactive || state == state_remove_pending)
            use_table_index = use_table_index_;
         else
            throw std::invalid_argument("Request already started");
      } // set_use_table_index
   };
};


/* Cora.DataSources.LgrNetSource.cpp

   Copyright (C) 2008, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 06 August 2008
   Last Change: Thursday 31 December 2020
   Last Commit: $Date: 2020-12-31 12:54:32 -0600 (Thu, 31 Dec 2020) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.DataSources.LgrNetSource.h"
#include "Cora.DataSources.Manager.h"
#include "Cora.Broker.ValueName.h"
#include "Cora.Device.VariableSetter.h"
#include "Cora.Device.FileSender.h"
#include "Cora.Device.FileReceiver.h"
#include "Cora.Device.FileLister.h"
#include "Cora.Device.ClockSetter.h"
#include "Cora.Device.FileController.h"
#include "Cora.Device.TerminalEmulator.h"
#include "Csi.SocketConnection.h" 
#include "Csi.LoginManager.h"


namespace Cora
{
   namespace DataSources
   {
      namespace LgrNetSourceHelpers
      {
         ////////////////////////////////////////////////////////////
         // class LnWart
         ////////////////////////////////////////////////////////////
         class LnWart: public WartBase
         {
         public:
            ////////////////////////////////////////////////////////////
            // station_name
            ////////////////////////////////////////////////////////////
            StrUni const station_name;

            ////////////////////////////////////////////////////////////
            // table_name
            ////////////////////////////////////////////////////////////
            StrUni const table_name;

            ////////////////////////////////////////////////////////////
            // column_name
            ////////////////////////////////////////////////////////////
            StrUni const column_name;

            ////////////////////////////////////////////////////////////
            // wants_column_name
            ////////////////////////////////////////////////////////////
            bool wants_column_name;
            
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            LnWart(
               StrUni const &station_name_,
               StrUni const &table_name_,
               StrUni const &column_name_):
               station_name(station_name_),
               column_name(column_name_),
               table_name(table_name_),
               wants_column_name(column_name_.length() > 0)
            { }
         };
         
         ////////////////////////////////////////////////////////////
         // class Uri
         ////////////////////////////////////////////////////////////
         class Uri
         {
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            Uri(StrUni const &uri)
            { parse(uri); }

            ////////////////////////////////////////////////////////////
            // parse
            ////////////////////////////////////////////////////////////
            void parse(StrUni const &uri);

            ////////////////////////////////////////////////////////////
            // get_source_name
            ////////////////////////////////////////////////////////////
            StrUni const &get_source_name() const
            { return source_name; }

            ////////////////////////////////////////////////////////////
            // get_broker_name
            ////////////////////////////////////////////////////////////
            StrUni const &get_broker_name() const
            { return broker_name; }

            ////////////////////////////////////////////////////////////
            // get_table_name
            ////////////////////////////////////////////////////////////
            StrUni const &get_table_name() const
            { return table_name; }

            ////////////////////////////////////////////////////////////
            // get_column_name
            ////////////////////////////////////////////////////////////
            StrUni const &get_column_name() const
            { return column_name; }
            
         private:
            ////////////////////////////////////////////////////////////
            // source_name
            ////////////////////////////////////////////////////////////
            StrUni source_name;

            ////////////////////////////////////////////////////////////
            // broker_name
            ////////////////////////////////////////////////////////////
            StrUni broker_name;

            ////////////////////////////////////////////////////////////
            // table_name
            ////////////////////////////////////////////////////////////
            StrUni table_name;

            ////////////////////////////////////////////////////////////
            // column_name
            ////////////////////////////////////////////////////////////
            StrUni column_name; 
         };


         void Uri::parse(StrUni const &uri)
         {
            size_t source_end_pos = uri.rfind(L":");
            size_t start_pos = 0;

            if(uri.first() == '\"')
               start_pos = 1;
            source_name.cut(0);
            broker_name.cut(0);
            table_name.cut(0);
            column_name.cut(0);
            uri.sub(source_name, start_pos, source_end_pos - start_pos);
            if(source_name.last() == '\"')
               source_name.cut(source_name.length() - 1);
            if(source_end_pos < uri.length())
            {
               // we need take the "remnant" of the URI which is everything left after the source
               // name is cut out and trailing quotes are eliminated
               StrUni remnant;
               uri.sub(remnant, source_end_pos + 1, uri.length());
               if(remnant.last() == '\"')
                  remnant.cut(remnant.length() - 1);

               // because lgrnet station names can contain periods, we need to perform right to left
               // parsing on the URI.  We will do this breaking the remnant down into a collection
               // of tokens
               typedef std::list<StrUni> tokens_type;
               tokens_type tokens;
               StrUni token;
               
               while(remnant.length() > 0)
               {
                  size_t token_end_pos = remnant.rfind(L".");
                  if(token_end_pos < remnant.length())
                  {
                     remnant.sub(token, token_end_pos + 1, remnant.length());
                     tokens.push_front(token);
                     remnant.cut(token_end_pos);
                  }
                  else
                  {
                     tokens.push_front(remnant);
                     remnant.cut(0);
                  }
               }

               // before assigning meanings to the tokens, we need to combine any tokens that may
               // have been "commented"
               tokens_type::iterator ti = tokens.begin();
               while(ti != tokens.end())
               {
                  if(ti->last() == '\\')
                  {
                     tokens_type::iterator ti_next = ti;
                     if(++ti_next != tokens.end())
                     {
                        ti->cut(ti->length() - 1);
                        ti->append('.');
                        ti->append(*ti_next);
                        tokens.erase(ti_next);
                     }
                     else
                        ++ti;
                  }
                  else
                     ++ti;
               }

               // The URI for LgrNet should have, at most, three levels: station name, table name,
               // and column name.  We will form the broker name by appending all front tokens to
               // the broker name separated by periods.
               if(!tokens.empty())
               {
                  broker_name = tokens.front();
                  tokens.pop_front();
               }
               if(!tokens.empty())
               {
                  table_name = tokens.front();
                  tokens.pop_front();
               }
               for(tokens_type::iterator ti = tokens.begin(); ti != tokens.end(); ++ti)
               {
                  if(ti != tokens.begin())
                     column_name.append('.');
                  column_name.append(*ti);
               }
            }
         } // parse


         ////////////////////////////////////////////////////////////
         // class MyAdvisorClient definitions
         ////////////////////////////////////////////////////////////
         MyAdvisorClient::MyAdvisorClient(
            LgrNetSource *source_,
            request_handle &request,
            bool more_to_follow):
            source(source_),
            end_file_mark(0xFFFFFFFF),
            end_record_no(0xFFFFFFFF),
            started(false),
            wants_specific_columns(true)
         { add_request(request, more_to_follow); }


         MyAdvisorClient::~MyAdvisorClient()
         {
            advisor.clear();
            index_getter.clear();
         } // destructor


         bool MyAdvisorClient::add_request(request_handle &request, bool more_to_follow)
         {
            bool rtn(false);
            if(!started)
            {
               // We need to determine whether the specified request is compatible with any
               // that have been added before. 
               Csi::PolySharedPtr<WartBase, LnWart> wart(request->get_wart());
               if(requests.empty())
                  rtn = true;
               else if(advisor == 0)
               {
                  // we are waiting for the advise ready from the server so we can determine whether
                  // this request is compatible with those already added.
                  request_handle &first(requests.front());
                  Csi::PolySharedPtr<WartBase, LnWart> first_wart(first->get_wart());
                  if(wart->station_name == first_wart->station_name &&
                     wart->table_name == first_wart->table_name &&
                     first->is_compatible(*request))
                     rtn = true;
               }

               // we need to ensure that the advisor has been created.
               if(rtn)
               {
                  if(wants_specific_columns && !wart->wants_column_name)
                     wants_specific_columns = false;
                  requests.push_back(request);
                  request->set_state(source, Request::state_pending);
                  if(request->get_order_option() == Request::order_real_time)
                     request->set_cacheable(true);
                  if(advisor == 0 && !more_to_follow)
                     start();
               }
            }  
            return rtn;
         } // add_request


         uint4 MyAdvisorClient::remove_request(request_handle &request)
         {
            requests_type::iterator ri(std::find(requests.begin(), requests.end(), request));
            if(ri != requests.end())
               requests.erase(ri);
            if(requests.empty())
               advisor.clear();
            return (uint4)requests.size();
         } // remove_request


         void MyAdvisorClient::start()
         {
            if(!requests.empty() && advisor == 0)
            {
               // we need to optionally start the index getter
               request_handle &request(requests.front());
               Csi::PolySharedPtr<WartBase, LnWart> wart(request->get_wart());
               if(request->get_use_table_index() && request->get_start_option() != Request::start_at_newest)
               {
                  index_getter.bind(new index_getter_type);
                  index_getter->set_open_broker_active_name(wart->station_name);
                  index_getter->set_table_name(wart->table_name);
                  index_getter->start(this, source->get_loggernet_component());
               }
               
               // start the advisor
               advisor.bind(new advisor_type);
               advisor->set_open_broker_active_name(wart->station_name);
               advisor->set_table_name(wart->table_name);
               advisor->set_start_option(static_cast<advisor_type::start_option_type>(request->get_start_option()));
               advisor->set_order_option(static_cast<advisor_type::order_option_type>(request->get_order_option()));
               advisor->set_start_record_no(request->get_record_no());
               advisor->set_start_file_mark_no(request->get_file_mark_no());
               advisor->set_start_date(request->get_start_time().get_nanoSec());
               advisor->set_start_interval(request->get_backfill_interval());
               advisor->set_start_record_offset(request->get_start_record_offset());
               advisor->set_value_factory(source->get_manager()->get_value_factory());
               advisor->set_cache_size_controller(request->get_cache_size_controller());
               for(requests_type::iterator ri = requests.begin();
                   ri != requests.end() && wants_specific_columns;
                   ++ri)
               {
                  request_handle &spec_request(*ri);
                  wart = spec_request->get_wart();
                  advisor->add_column(wart->column_name);
               }
               advisor->start(this, source->get_loggernet_component());
            }
         } // start

         
         void MyAdvisorClient::on_advise_ready(
            advisor_type *advisor)
         {
            using Cora::Broker::Record;
            LgrNetSource *source(this->source);
            requests_type temp(requests);
            SinkBase::record_handle &record(advisor->get_record());
            started = true;
            for(requests_type::iterator ri = temp.begin(); ri != temp.end(); ++ri)
            {
               request_handle &request(*ri);
               SinkBase *sink(request->get_sink());
               Csi::PolySharedPtr<WartBase, LnWart> wart(request->get_wart());
               
               if(SinkBase::is_valid_instance(sink))
               {
                  // we can now report to the sink that the data flow is ready
                  bool report_failure(true);
                  StrAsc column_name_mb(wart->column_name.to_utf8());;
                  try
                  {
                     request->set_value_indices(*record, column_name_mb);
                     if(request->get_begin_index() != request->get_end_index())
                     {
                        report_failure = false;
                        request->set_state(source, Request::state_started);
                        sink->on_sink_ready(source->get_manager(), request, record);
                     }
                     else
                        report_failure = true;
                  }
                  catch(std::exception &)
                  { report_failure = true; }

                  // we need to report any failures to the sink
                  if(report_failure)
                  {
                     uint4 after_remove_count;
                     source->set_request_retry();
                     request->set_state(source, Request::state_error);
                     after_remove_count = remove_request(request);
                     sink->on_sink_failure(
                        source->get_manager(), request, SinkBase::sink_failure_invalid_column_name);
                     if(after_remove_count == 0)
                        source->on_advisor_failure(this);
                  }
                  
               }
               else
                  source->remove_request(request);
            }
         } // on_advise_ready


         void MyAdvisorClient::on_advise_failure(
            advisor_type *advisor, failure_type failure)
         {
            requests_type requests(this->requests);
            LgrNetSource *source(this->source);
            while(!requests.empty())
            {
               request_handle request(requests.front());
               SinkBase *sink(request->get_sink());
               SinkBase::sink_failure_type sink_failure(SinkBase::sink_failure_unknown);
               Csi::PolySharedPtr<WartBase, LnWart> wart(request->get_wart());
               
               requests.pop_front();
               switch(failure)
               {
               case failure_connection_failed:
                  sink_failure = SinkBase::sink_failure_connection_failed;
                  break;
                  
               case failure_invalid_logon:
                  sink_failure = SinkBase::sink_failure_invalid_logon;
                  break;
                  
               case failure_invalid_station_name:
                  sink_failure = SinkBase::sink_failure_invalid_station_name;
                  break;
                  
               case failure_invalid_table_name:
                  sink_failure = SinkBase::sink_failure_invalid_table_name;
                  break;
                  
               case failure_server_security:
                  sink_failure = SinkBase::sink_failure_server_security;
                  break;
                  
               case failure_invalid_start_option:
                  sink_failure = SinkBase::sink_failure_invalid_start_option;
                  break;
                  
               case failure_invalid_order_option:
                  sink_failure = SinkBase::sink_failure_invalid_order_option;
                  break;
                  
               case failure_table_deleted:
                  sink_failure = SinkBase::sink_failure_table_deleted;
                  break;
                  
               case failure_station_shut_down:
                  sink_failure = SinkBase::sink_failure_station_shut_down;
                  break;
                  
               case failure_unsupported:
                  sink_failure = SinkBase::sink_failure_unsupported;
                  break;
                  
               case failure_invalid_column_name:
                  wart->wants_column_name = false;
                  sink_failure = SinkBase::sink_failure_invalid_column_name;
                  break;
                  
               case failure_invalid_array_address:
                  wart->wants_column_name = false;
                  sink_failure = SinkBase::sink_failure_invalid_array_address;
                  break;
               }
               if(SinkBase::is_valid_instance(sink))
               {
                  source->set_request_retry();
                  request->set_state(source, Request::state_error);
                  sink->on_sink_failure(source->get_manager(), request, sink_failure);
               }
            }
            source->on_advisor_failure(this);
         } // on_advise_failure


         void MyAdvisorClient::on_advise_record(
            advisor_type *advisor_)
         {
            typedef advisor_type::unread_records_type records_type;
            Csi::SharedPtr<advisor_type> advisor(this->advisor);
            records_type &records(advisor->get_unread_records());
            if(!records.empty())
            {
               records_type::value_type &last(records.back());
               bool expect_more(true);
               if(last->get_file_mark_no() >= end_file_mark && last->get_record_no() >= end_record_no)
                  expect_more = false;
               if(!requests.empty() && expect_more)
               {
                  request_handle &request(requests.front());
                  if(request->get_start_option() == Request::start_at_newest)
                     expect_more = false; 
               }
               for(requests_type::iterator ri = requests.begin();
                   !expect_more && ri != requests.end();
                   ++ri)
               {
                  request_handle &request(*ri);
                  request->set_expect_more_data(false);
               }
               SinkBase::report_sink_records(
                  source->get_manager(), requests, records);
               advisor->get_next_block();
            }
         } // on_advise_record


         void MyAdvisorClient::on_complete(
            index_getter_type *index,
            index_getter_type::client_type::outcome_type outcome,
            index_records_type const &records)
         {
            if(outcome == index_getter_type::client_type::outcome_success)
            {
               if(!records.empty())
               {
                  index_records_type::value_type const &record(records.back());
                  end_file_mark = record.file_mark_no;
                  end_record_no = record.end_record_no;
               }
               else
               {
                  for(requests_type::iterator ri = requests.begin(); ri != requests.end(); ++ri)
                  {
                     request_handle &request(*ri);
                     request->set_expect_more_data(false);
                  }
               }
            }
            else
            {
               for(requests_type::iterator ri = requests.begin(); ri != requests.end(); ++ri)
               {
                  request_handle &request(*ri);
                  request->set_expect_more_data(false);
               }
            }
            index_getter.clear();
         } // on_complete


         ////////////////////////////////////////////////////////////
         // class MyDataManagerClient definitions
         ////////////////////////////////////////////////////////////
         MyDataManagerClient::MyDataManagerClient(
            LgrNetSource *source_,
            manager_handle &manager_,
            request_handle &request):
            source(source_),
            manager(manager_),
            end_file_mark(0xFFFFFFFF),
            end_record_no(0xFFFFFFFF),
            started(false)
         { add_request(request); }


         MyDataManagerClient::~MyDataManagerClient()
         {
            while(manager != 0 && !requests.empty())
            {
               request_handle request(requests.front());
               LnWart *wart = static_cast<LnWart *>(request->get_wart().get_rep());
               manager->remove_data_request(
                  wart->station_name, wart->table_name, start_info, this, wart->column_name);
               requests.pop_front();
            }
            manager.clear();
         } // destructor


         bool MyDataManagerClient::add_request(request_handle &request)
         {
            bool rtn = false;
            if(!started)
            {
               LnWart *wart = static_cast<LnWart *>(request->get_wart().get_rep());
               if(requests.empty())
               {
                  rtn = true;
                  start_info.order_option = static_cast<Cora::Broker::DataAdvisor::order_option_type>(request->get_order_option());
                  start_info.start_option = static_cast<Cora::Broker::DataAdvisor::start_option_type>(request->get_start_option());
                  start_info.file_mark_no = request->get_file_mark_no();
                  start_info.record_no = request->get_record_no();
                  start_info.start_date = request->get_start_time().get_nanoSec();
                  start_info.start_interval = request->get_backfill_interval();
                  start_info.cache_size_controller = request->get_cache_size_controller();
                  start_info.start_record_offset = request->get_start_record_offset();
               }
               else
               {
                  // since a request has already been added added, we need to determine whether the new
                  // request is compatible with the existing ones
                  request_handle &first = requests.front();
                  Csi::PolySharedPtr<WartBase, LnWart> first_wart(first->get_wart());
                  if(wart->station_name == first_wart->station_name &&
                     wart->table_name == first_wart->table_name &&
                     first->is_compatible(*request))
                     rtn = true;
               } 
               if(rtn)
               {
                  try
                  {
                     if(request->get_use_table_index() && request->get_start_option() != Request::start_at_newest)
                     {
                        table_index.bind(new table_index_type);
                        table_index->set_open_broker_active_name(wart->station_name);
                        table_index->set_table_name(wart->table_name);
                        table_index->start(this, source->get_loggernet_component());
                     }
                     manager->add_data_request(
                        wart->station_name, wart->table_name, start_info, this, wart->column_name);
                     requests.push_back(request);
                     request->set_state(source, Request::state_pending);
                     request->set_cacheable(true); 
                  }
                  catch(std::exception &)
                  {
                     request->set_state(source, Request::state_error);
                  }
               }
            }
            return rtn;
         } // add_request


         uint4 MyDataManagerClient::remove_request(request_handle &request)
         {
            requests_type::iterator ri = std::find(requests.begin(), requests.end(), request);
            if(ri != requests.end())
            {
               if(manager != 0)
               {
                  LnWart *wart = static_cast<LnWart *>(request->get_wart().get_rep());
                  if(requests.size() == 1)
                  {
                     manager->remove_data_request(
                        wart->station_name, wart->table_name, start_info, this, wart->column_name);
                  }
               }
               requests.erase(ri);
            }
            return (uint4)requests.size();
         } // remove_request

         
         void MyDataManagerClient::on_advise_ready(
            advisor_type *advisor,
            StrUni const &broker_name_,
            StrUni const &table_name,
            record_type &record)
         {
            using Cora::Broker::Record;
            LgrNetSource *source = this->source;
            requests_type temp(requests);
            started = true;
            for(requests_type::iterator ri = temp.begin(); ri != temp.end(); ++ri)
            {
               request_handle request = *ri;
               SinkBase *sink = request->get_sink();
               LnWart *wart = static_cast<LnWart *>(request->get_wart().get_rep());
               if(SinkBase::is_valid_instance(sink))
               {
                  bool report_failure = true;
                  // we can now report to the sink that the data flow is ready
                  StrAsc column_name_mb(wart->column_name.to_utf8());
                  try
                  {
                     request->set_value_indices(*record, column_name_mb);
                     if(request->get_begin_index() != request->get_end_index())
                     {
                        report_failure = false;
                        request->set_state(source, Request::state_started);
                        sink->on_sink_ready(
                           source->get_manager(), request, record);
                     }
                     else
                        report_failure = true;
                  }
                  catch(std::exception &)
                  { report_failure = true; }
                  
                  if(report_failure)
                  {
                     uint4 after_remove_count;
                     
                     source->set_request_retry();
                     request->set_state(source, Request::state_error);
                     after_remove_count = remove_request(request);
                     sink->on_sink_failure(
                        source->get_manager(), request, SinkBase::sink_failure_invalid_column_name);
                     if(after_remove_count == 0)
                        source->on_client_advise_failure(this);
                  }
               }
               else
                  source->remove_request(request);
            }
         } // on_advise_ready


         void MyDataManagerClient::on_advise_record(
            advisor_type *advisor,
            StrUni const &broker_name,
            StrUni const &table_name,
            records_type &records)
         {
            if(!records.empty())
            {
               records_type::value_type &last(records.back());
               bool expect_more(true);

               if(last->get_file_mark_no() >= end_file_mark && last->get_record_no() >= end_record_no)
                  expect_more = false;
               if(!requests.empty() && expect_more)
               {
                  request_handle &request(requests.front());
                  if(request->get_start_option() == Request::start_at_newest)
                     expect_more = false;
               }
               for(requests_type::iterator ri = requests.begin();
                   !expect_more && ri != requests.end();
                   ++ri)
               {
                  request_handle &request(*ri);
                  request->set_expect_more_data(false);
               }
               SinkBase::report_sink_records(
                  source->get_manager(), requests, records);
            }
         } // on_advise_record


         void MyDataManagerClient::on_advise_failure(
            advisor_type *advisor,
            StrUni const &broker_name,
            StrUni const &table_name,
            failure_type failure)
         {
            requests_type requests(this->requests);
            LgrNetSource *source(this->source);
            while(!requests.empty())
            {
               request_handle request(requests.front()); 
               SinkBase *sink = request->get_sink();
               SinkBase::sink_failure_type sink_failure = SinkBase::sink_failure_unknown;

               requests.pop_front();
               switch(failure)
               {
               case failure_connection_failed:
                  sink_failure = SinkBase::sink_failure_connection_failed;
                  break;
                  
               case failure_invalid_logon:
                  sink_failure = SinkBase::sink_failure_invalid_logon;
                  break;
                  
               case failure_invalid_station_name:
                  sink_failure = SinkBase::sink_failure_invalid_station_name;
                  break;
                  
               case failure_invalid_table_name:
                  sink_failure = SinkBase::sink_failure_invalid_table_name;
                  break;
                  
               case failure_server_security:
                  sink_failure = SinkBase::sink_failure_server_security;
                  break;
                  
               case failure_invalid_start_option:
                  sink_failure = SinkBase::sink_failure_invalid_start_option;
                  break;
                  
               case failure_invalid_order_option:
                  sink_failure = SinkBase::sink_failure_invalid_order_option;
                  break;
                  
               case failure_table_deleted:
                  sink_failure = SinkBase::sink_failure_table_deleted;
                  break;
                  
               case failure_station_shut_down:
                  sink_failure = SinkBase::sink_failure_station_shut_down;
                  break;
                  
               case failure_unsupported:
                  sink_failure = SinkBase::sink_failure_unsupported;
                  break;
                  
               case failure_invalid_column_name:
                  sink_failure = SinkBase::sink_failure_invalid_column_name;
                  break;
                  
               case failure_invalid_array_address:
                  sink_failure = SinkBase::sink_failure_invalid_array_address;
                  break;
               }
               if(SinkBase::is_valid_instance(sink))
               {
                  source->set_request_retry();
                  request->set_state(source, Request::state_error);
                  sink->on_sink_failure(
                     source->get_manager(), request, sink_failure);
               }
            }
            source->on_client_advise_failure(this);
         } // on_advise_failure


         void MyDataManagerClient::on_complete(
            table_index_type *index,
            table_index_type::client_type::outcome_type outcome,
            index_records_type const &records)
         {
            if(outcome == table_index_type::client_type::outcome_success)
            {
               if(!records.empty())
               {
                  index_records_type::value_type const &record(records.back());
                  end_file_mark = record.file_mark_no;
                  end_record_no = record.end_record_no;
               }
               else
               {
                  for(requests_type::iterator ri = requests.begin(); ri != requests.end(); ++ri)
                  {
                     request_handle &request(*ri);
                     request->set_expect_more_data(false);
                  }
               }
            }
            else
            {
               for(requests_type::iterator ri = requests.begin(); ri != requests.end(); ++ri)
               {
                  request_handle &request(*ri);
                  request->set_expect_more_data(false);
               }
            }
            table_index.clear();
         } // on_complete
         

         ////////////////////////////////////////////////////////////
         // class Query definitions
         ////////////////////////////////////////////////////////////
         Query::Query(LgrNetSource *source_, request_handle &request):
            source(source_)
         {
            Uri uri(request->get_uri());
            requests.push_back(request);
         } // constructor


         Query::~Query()
         {
            query.clear();
            requests.clear();
         } // destructor


         void Query::start(ClientBase *component)
         {
            if(query == 0 && !requests.empty())
            {
               request_handle &request = requests.front();
               LnWart *wart = static_cast<LnWart *>(request->get_wart().get_rep());
               query.bind(new query_type);
               query->set_open_broker_active_name(wart->station_name);
               query->set_table_name(wart->table_name);
               query->set_query_date(request->get_start_time(), request->get_end_time());
               query->start(this, component);
            }
         } // start

         
         void Query::on_started(
            query_type *query, record_handle &record)
         {
            requests_type temp(requests);
            while(!temp.empty())
            {
               request_handle request(temp.front());
               bool report_error = false;
               SinkBase *sink = request->get_sink();
               if(SinkBase::is_valid_instance(sink))
               {
                  try
                  {
                     Uri uri(request->get_uri());
                     temp.pop_front();
                     request->set_value_indices(*record, uri.get_column_name());
                     if(request->get_begin_index() != request->get_end_index())
                     {
                        request->set_state(source, Request::state_started);
                        sink->on_sink_ready(source->get_manager(), request, record);
                        if(!Query::is_valid_instance(this))
                           return;
                     }
                     else
                        report_error = true;
                  }
                  catch(std::exception &)
                  { report_error = true; }
               }
               else
                  report_error = true;
               if(report_error)
               {
                  source->set_request_retry();
                  request->set_state(source, Request::state_error);
                  if(SinkBase::is_valid_instance(sink))
                     sink->on_sink_failure(
                        source->get_manager(), request, SinkBase::sink_failure_invalid_column_name);
                  remove_request(request);
               }
            }
         } // on_started


         void Query::on_complete(
            query_type *query, outcome_type outcome)
         {
            requests_type temp(requests);
            this->query.clear();
            if(outcome != outcome_success)
            {
               SinkBase::sink_failure_type failure = SinkBase::sink_failure_unknown;
               switch(outcome)
               {
               case outcome_connection_failed:
                  failure = SinkBase::sink_failure_connection_failed;
                  break;
                  
               case outcome_invalid_logon:
                  failure = SinkBase::sink_failure_invalid_logon;
                  break;
                  
               case outcome_invalid_station_name:
                  failure = SinkBase::sink_failure_invalid_station_name;
                  break;
                  
               case outcome_invalid_table_name:
                  failure = SinkBase::sink_failure_invalid_table_name;
                  break;
                  
               case outcome_server_security_failed:
                  failure = SinkBase::sink_failure_server_security;
                  break;
                           
               case outcome_unsupported:
                  failure = SinkBase::sink_failure_unsupported;
                  break;
               }
               while(!temp.empty())
               {
                  request_handle request(temp.front());
                  SinkBase *sink = request->get_sink();
                  
                  temp.pop_front();
                  request->set_state(source, Request::state_error);
                  if(SinkBase::is_valid_instance(sink))
                     sink->on_sink_failure(source->get_manager(), request, failure);
               }
            }
         } // on_complete


         void Query::on_records(
            query_type *query, records_type const &records, bool more_expected)
         {
            // if no more records are expected, we need to mark all of the requests as satisfied
            if(!more_expected)
            {
               for(requests_type::iterator ri = requests.begin(); ri != requests.end(); ++ri)
               {
                  request_handle &request = *ri;
                  request->set_state(source, Request::state_satisfied);
                  request->set_expect_more_data(false);
               }
            }
            SinkBase::report_sink_records(source->get_manager(), requests, records);
         } // on_records


         bool Query::add_request(request_handle &request)
         {
            bool rtn = false;
            if(query == 0)
            {
               LnWart *wart = static_cast<LnWart *>(request->get_wart().get_rep());
               if(!requests.empty())
               {
                  request_handle &first(requests.front());
                  LnWart *first_wart = static_cast<LnWart *>(first->get_wart().get_rep());
                  if(first_wart->station_name == wart->station_name &&
                     first_wart->table_name == wart->table_name &&
                     first->is_compatible(*request))
                  {
                     rtn = true;
                     requests.push_back(request);
                  }
               }
            }
            return rtn;
         } // add_request


         bool Query::remove_request(request_handle &request)
         {
            bool rtn = false;
            requests_type::iterator ri = std::find(
               requests.begin(), requests.end(), request);
            if(ri != requests.end())
            {
               requests.erase(ri);
               rtn = true;
            }
            return rtn;
         } // remove_request


         ////////////////////////////////////////////////////////////
         // class VariableSetter
         ////////////////////////////////////////////////////////////
         class VariableSetter:
            public Cora::Device::VariableSetterClient,
            public Csi::EventReceiver
         {
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            VariableSetter(
               SinkBase *sink_,
               StrUni const &uri_,
               ValueSetter const &value_,
               LgrNetSource *source_);

            ////////////////////////////////////////////////////////////
            // destructor
            ////////////////////////////////////////////////////////////
            virtual ~VariableSetter();

            ////////////////////////////////////////////////////////////
            // on_complete
            ////////////////////////////////////////////////////////////
            typedef Cora::Device::VariableSetter setter_type;
            virtual void on_complete(
               setter_type *setter, setter_type::client_type::outcome_type outcome);

            ////////////////////////////////////////////////////////////
            // receive
            ////////////////////////////////////////////////////////////
            virtual void receive(Csi::SharedPtr<Csi::Event> &ev);
            
         private:
            ////////////////////////////////////////////////////////////
            // sink
            ////////////////////////////////////////////////////////////
            SinkBase *sink;

            ////////////////////////////////////////////////////////////
            // uri
            ////////////////////////////////////////////////////////////
            StrUni const uri;

            ////////////////////////////////////////////////////////////
            // source
            ////////////////////////////////////////////////////////////
            LgrNetSource *source;

            ////////////////////////////////////////////////////////////
            // setter
            ////////////////////////////////////////////////////////////
            Csi::SharedPtr<setter_type> setter;

            ////////////////////////////////////////////////////////////
            // error_event
            ////////////////////////////////////////////////////////////
            static uint4 const error_event;
         };


         uint4 const VariableSetter::error_event(
            Csi::Event::registerType("Cora::DataSources::LgrNetSource::VariableSetter::error"));


         VariableSetter::VariableSetter(
            SinkBase *sink_,
            StrUni const &uri_,
            ValueSetter const &value,
            LgrNetSource *source_):
            sink(sink_),
            uri(uri_),
            source(source_)
         {
            try
            {
               // create and initialise the setter
               Uri uri(uri_);
               Cora::Broker::ValueName value_name(uri.get_column_name().c_str());
               
               setter.bind(new setter_type);
               setter->set_device_name(uri.get_broker_name());
               setter->set_table_name(uri.get_table_name());
               if(Cora::LgrNet::DataManager::get_inlocs_table_name() == setter->get_table_name())
               {
                  StrUni table_name;
                  source->get_actual_inloc_table_name(uri_, table_name);
                  setter->set_table_name(table_name);
               }
               setter->set_column_name(value_name.get_column_name());
               setter->set_index(value_name.get_subscripts());
               switch(value.value_type)
               {
               case ValueSetter::value_type_bool:
                  setter->set_value_bool(value.value_variant.v_bool);
                  break;
                  
               case ValueSetter::value_type_float:
                  setter->set_value_float(value.value_variant.v_float);
                  break;
                  
               case ValueSetter::value_type_uint4:
                  setter->set_value_uint4(value.value_variant.v_uint4);
                  break;
                  
               case ValueSetter::value_type_int4:
                  setter->set_value_int4(value.value_variant.v_int4);
                  break;
                  
               case ValueSetter::value_type_uint2:
                  setter->set_value_uint2(value.value_variant.v_uint2);
                  break;
                  
               case ValueSetter::value_type_int2:
                  setter->set_value_int2(value.value_variant.v_int2);
                  break;
                  
               case ValueSetter::value_type_uint1:
                  setter->set_value_uint1(value.value_variant.v_uint1);
                  break;
                  
               case ValueSetter::value_type_int1:
                  setter->set_value_int1(value.value_variant.v_int1);
                  break;
                  
               case ValueSetter::value_type_string:
                  setter->set_value_string(value.value_string.c_str());
                  break;
                  
               default:
                  throw std::invalid_argument("unsupported value type");
                  break;
               }

               // at this point, we are now ready to start the setter
               setter->set_locale(value.locale);
               setter->start(this, source->get_loggernet_component());
            }
            catch(std::exception &)
            {
               Csi::Event *ev(Csi::Event::create(error_event, this));
               ev->post();
            }
         } // constructor


         VariableSetter::~VariableSetter()
         { setter.clear(); }


         void VariableSetter::on_complete(
            setter_type *setter, outcome_type outcome)
         {
            LgrNetSource *source(this->source);
            if(SinkBase::is_valid_instance(sink))
            {
               SinkBase::set_outcome_type rtn;
               switch(outcome)
               {
               default:
               case outcome_unknown:
                  rtn = SinkBase::set_outcome_unknown;
                  break;
                  
               case outcome_succeeded:
                  rtn = SinkBase::set_outcome_succeeded;
                  break;
                  
               case outcome_connection_failed:
                  rtn = SinkBase::set_outcome_connection_failed;
                  break;
                  
               case outcome_invalid_logon:
                  rtn = SinkBase::set_outcome_invalid_logon;
                  break;
                  
               case outcome_server_security_blocked:
                  rtn = SinkBase::set_outcome_server_security_blocked;
                  break;
                  
               case outcome_column_read_only:
                  rtn = SinkBase::set_outcome_column_read_only;
                  break;
                  
               case outcome_invalid_table_name:
                  rtn = SinkBase::set_outcome_invalid_table_name;
                  break;
                  
               case outcome_invalid_column_name:
                  rtn = SinkBase::set_outcome_invalid_column_name;
                  break;
                  
               case outcome_invalid_subscript:
                  rtn = SinkBase::set_outcome_invalid_subscript;
                  break;
                  
               case outcome_invalid_data_type:
                  rtn = SinkBase::set_outcome_invalid_data_type;
                  break;
                  
               case outcome_communication_failed:
                  rtn = SinkBase::set_outcome_communication_failed;
                  break;
                  
               case outcome_communication_disabled:
                  rtn = SinkBase::set_outcome_communication_disabled;
                  break;
                  
               case outcome_logger_security_blocked:
                  rtn = SinkBase::set_outcome_logger_security_blocked;
                  break;
                  
               case outcome_unmatched_logger_table_definition:
                  rtn = SinkBase::set_outcome_unmatched_logger_table_definition;
                  break;
                  
               case outcome_invalid_device_name:
                  rtn = SinkBase::set_outcome_invalid_device_name;
                  break;
               }
               sink->on_set_complete(source->get_manager(), uri, rtn);
            }
            if(Csi::InstanceValidator::is_valid_instance<OneShotClient>(source))
               source->end_set_value(this);
         } // on_complete


         void VariableSetter::receive(Csi::SharedPtr<Csi::Event> &ev)
         {
            if(ev->getType() == error_event)
            {
               outcome_type outcome(outcome_connection_failed);
               if(source->get_last_connect_error() == ManagerClient::disconnect_invalid_logon)
                  outcome = outcome_invalid_logon;
               on_complete(setter.get_rep(), outcome);
            }
         } // receive


         /**
          * Defines an object that will send a file for the LgrNet data source.
          */
         class FileSender: public Cora::Device::FileSenderClient, Csi::EventReceiver
         {
         public:
            /**
             * Constructor
             */
            FileSender(
               SinkBase *sink_,
               StrUni const &uri_,
               StrUni const &dest_file_name_,
               StrUni const &file_name_,
               LgrNetSource *source_);

            /**
             * Destructor
             */
            virtual ~FileSender();

            /**
             * Handles the completion event notification for the  device file sender.
             */
            typedef Cora::Device::FileSender sender_type;
            virtual void on_complete(sender_type *sender, outcome_type outcome);

            /**
             * Handles an event notification/
             */
            virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

         private:
            /**
             * Specifies the sink that requested this service.
             */
            SinkBase *sink;

            /**
             * Specifies the station uri.
             */
            StrUni const uri;

            /**
             * Specifies the destination file name.
             */
            StrUni const dest_file_name;

            /**
             * Specifies the source file name.
             */
            StrUni const source_file_name;

            /**
             * Specifies the source that owns this sender.
             */
            LgrNetSource *source;

            /**
             * Specifies the sender component.
             */
            Csi::SharedPtr<sender_type> sender;

            /**
             * Identifies an error event.
             */
            static uint4 const error_event;
         };


         uint4 const FileSender::error_event(
            Csi::Event::registerType("Cora::DataSources::LgrNetSource::FileSender::error_event"));
         

         FileSender::FileSender(
            SinkBase *sink_,
            StrUni const &uri_,
            StrUni const &dest_file_name_,
            StrUni const &file_name,
            LgrNetSource *source_):
            sink(sink_),
            source(source_),
            uri(uri_),
            dest_file_name(dest_file_name_)
         {
            try
            {
               // we need to initialise the sender.  We will start by parsing the source uri.
               Uri station_uri(uri_);
               sender.bind(new sender_type);
               sender->set_device_name(station_uri.get_broker_name());
               sender->set_logger_file_name(dest_file_name.to_utf8());
               sender->set_send_source(new Cora::Device::FileSenderHelpers::file_send_source(file_name.to_utf8()));
               sender->start(this, source->get_loggernet_component());
            }
            catch(std::exception &)
            {
               Csi::Event *ev(Csi::Event::create(error_event, this));
               ev->post();
            }
         } // constructor


         FileSender::~FileSender()
         { sender.clear(); }


         void FileSender::on_complete(
            sender_type *component, outcome_type outcome)
         {
            LgrNetSource *source(this->source);
            if(SinkBase::is_valid_instance(sink))
            {
               SinkBase::send_file_outcome_type rtn;
               switch(outcome)
               {
               default:
                  rtn = SinkBase::send_file_unknown;
                  break;

               case outcome_success:
                  rtn = SinkBase::send_file_success;
                  break;

               case outcome_communication_disabled:
                  rtn = SinkBase::send_file_communication_disabled;
                  break;

               case outcome_missing_file_name:
               case outcome_invalid_file_name:
                  rtn = SinkBase::send_file_invalid_file_name;
                  break;

               case outcome_logger_resource_error:
                  rtn = SinkBase::send_file_logger_resource_error;
                  break;

               case outcome_communication_failed:
                  rtn = SinkBase::send_file_communication_failed;
                  break;

               case outcome_logger_permission_denied:
                  rtn = SinkBase::send_file_logger_security_blocked;
                  break;

               case outcome_invalid_logon:
                  rtn = SinkBase::send_file_invalid_logon;
                  break;

               case outcome_server_connection_failed:
                  rtn = SinkBase::send_file_connection_failed;
                  break;

               case outcome_invalid_device_name:
                  rtn = SinkBase::send_file_invalid_uri;
                  break;

               case outcome_server_permission_denied:
                  rtn = SinkBase::send_file_server_security_blocked;
                  break;

               case outcome_logger_root_dir_full:
                  rtn = SinkBase::send_file_logger_root_full;
                  break;
               }
               sink->on_send_file_complete(source->get_manager(), uri, dest_file_name, rtn);
            }
            if(Csi::InstanceValidator::is_valid_instance<OneShotClient>(source))
               source->end_send_file(this);
         } // on_complete


         void FileSender::receive(Csi::SharedPtr<Csi::Event> &ev)
         {
            if(ev->getType() == error_event)
            {
               outcome_type outcome(outcome_server_connection_failed);
               if(source->get_last_connect_error() == ManagerClient::disconnect_invalid_logon)
                  outcome = outcome_invalid_logon;
               on_complete(sender.get_rep(), outcome);
            }
         } // receive


         class NewestFileGetter:
            public Cora::Device::FileListerClient,
            public Cora::Device::FileReceiverClient,
            public Csi::EventReceiver
         {
         public:
            /**
             * Constructor.
             */
            NewestFileGetter(
               LgrNetSource *source_,
               SinkBase *sink_,
               StrUni const &uri_,
               StrUni const &pattern_);

            /**
             * Destructor
             */
            virtual ~NewestFileGetter();

            /**
             * Overloads the on_complete method for the files lister.
             */
            typedef Cora::Device::FileLister lister_type;
            virtual void on_complete(
               lister_type *sender,
               lister_type::client_type::outcome_type outcome, 
               file_list_type const &files);

            /**
             * Overloads the completion event handler for the file receiver.
             */
            typedef Cora::Device::FileReceiver receiver_type;
            virtual void on_complete(
               receiver_type *sender,
               receiver_type::client_type::outcome_type outcome);

            /**
             * Overloads the receive method.
             */
            virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

         private:
            /**
             * Starts retrieval of the specified file.
             */
            void start_get_file(StrUni const &pattern);
            
         private:
            /**
             * Specifies the source for this component.
             */
            LgrNetSource *source;

            /**
             * Specifies the sink.
             */
            SinkBase *sink;

            /**
             * Specifies the uri for the station.
             */
            StrUni uri;

            /**
             * Specifies the pattern.
             */
            StrUni pattern;

            /**
             * Specifies the station name.
             */
            StrUni station_name;

            /**
             * Specifies the selected file.
             */
            StrUni file_name;

            /**
             * Specifies the component that gets the file list.
             */
            Csi::SharedPtr<lister_type> lister;

            /**
             * Specifies the component used for retrieving the file.
             */
            Csi::SharedPtr<receiver_type> receiver;

            /**
             * Specifies the event ID used to identify an event posted when components can't be
             * started.
             */
            static uint4 const error_event;

            /**
             * Defines the sink type for the receiver.
             */
            class newest_file_sink: public Cora::Device::FileReceiverHelpers::receive_sink_type
            {
            private:
               NewestFileGetter *getter;
               
            public:
               newest_file_sink(NewestFileGetter *getter_):
                  getter(getter_)
               { }

               virtual void receive_next_fragment(void const *buff, uint4 buff_len)
               {
                  if(SinkBase::is_valid_instance(getter->sink))
                  {
                     bool accepted(
                        getter->sink->on_get_newest_file_status(
                           getter->source->get_manager(),
                           SinkBase::get_newest_status_in_progress,
                           getter->uri,
                           getter->pattern,
                           getter->file_name,
                           buff,
                           buff_len));
                     if(!accepted)
                     {
                        getter->receiver.clear();
                        getter->source->end_get_newest_file(getter);
                     }
                  }
               }
            };
            friend class newest_file_sink;
         };


         uint4 const NewestFileGetter::error_event(
            Csi::Event::registerType("Cora::DataSources::LgrNetSource::newest_file_getter::error"));
         

         NewestFileGetter::NewestFileGetter(
            LgrNetSource *source_,
            SinkBase *sink_,
            StrUni const &uri_,
            StrUni const &pattern_):
            source(source_),
            sink(sink_),
            uri(uri_),
            pattern(pattern_)
         {
            try
            {
               Uri station_uri(uri_);
               size_t wild_card_pos(pattern.find(L"*"));
               station_name = station_uri.get_broker_name();
               if(wild_card_pos >= pattern.length())
                  wild_card_pos = pattern.find(L"?");
               if(wild_card_pos >= pattern.length())
                  start_get_file(pattern);
               else
               {
                  lister.bind(new lister_type);
                  lister->set_device_name(station_name);
                  lister->set_pattern(pattern.to_utf8());
                  lister->start(this, source->get_loggernet_component());
               }
            }
            catch(std::exception &)
            {
               Csi::Event *ev(Csi::Event::create(error_event, this));
               ev->post();
            }
         } // constructor

         
         NewestFileGetter::~NewestFileGetter()
         {
            lister.clear();
            receiver.clear();
         } // destructor


         void NewestFileGetter::on_complete(
            lister_type *sender,
            lister_type::client_type::outcome_type outcome,
            file_list_type const &files)
         {
            if(outcome == lister_type::client_type::outcome_success)
            {
               // we need to pick out the newest matching file.
               Csi::LgrDate selected_time;
               Csi::LgrDate file_time;
               StrAsc last_update;
               StrAsc selected_name;
               StrAsc expr(pattern.to_utf8());
               for(file_list_type::const_iterator fi = files.begin(); fi != files.end(); ++fi)
               {
                  file_list_type::value_type const &file(*fi);
                  if(Csi::matches_wildcard_expr(file.get_name(), expr))
                  {
                     try
                     {
                        file.get_attr_last_update(last_update);
                        file_time = Csi::LgrDate::fromStr(last_update.c_str());
                     }
                     catch(std::exception &)
                     { file_time = 0; }
                     if(selected_name.length() == 0 || selected_time < file_time)
                     {
                        selected_name = file.get_name();
                        selected_time = file_time;
                     }
                  }
               }

               // we can now start to get the newest file (if any)
               if(selected_name.length() > 0)
                  start_get_file(selected_name);
               else
               {
                  if(SinkBase::is_valid_instance(sink))
                  {
                     sink->on_get_newest_file_status(
                        source->get_manager(),
                        SinkBase::get_newest_status_no_file,
                        uri,
                        pattern);
                     sink = 0;
                  }
                  source->end_get_newest_file(this);
               }
            }
            else
            {
               // we need to map the outcome code to a sink status code.
               SinkBase::get_newest_file_status_type status;
               switch(outcome)
               {
               case lister_type::client_type::outcome_invalid_logon:
                  status = SinkBase::get_newest_status_invalid_logon;
                  break;
                  
               case lister_type::client_type::outcome_session_failure:
                  status = SinkBase::get_newest_status_connection_failed;
                  break;
                  
               case lister_type::client_type::outcome_invalid_device_name:
                  status = SinkBase::get_newest_status_invalid_uri;
                  break;
                  
               case lister_type::client_type::outcome_blocked_by_server:
                  status = SinkBase::get_newest_status_server_permission_denied;
                  break;
                  
               case lister_type::client_type::outcome_unsupported:
                  status = SinkBase::get_newest_status_unsupported;
                  break;
                  
               case lister_type::client_type::outcome_blocked_by_logger:
                  status = SinkBase::get_newest_status_logger_permission_denied;
                  break;
                  
               case lister_type::client_type::outcome_comm_disabled:
                  status = SinkBase::get_newest_status_communication_disabled;
                  break;
                  
               case lister_type::client_type::outcome_comm_failed:
                  status = SinkBase::get_newest_status_communication_failed;
                  break;
                  
               default:
                  status = SinkBase::get_newest_status_unknown_failure;
                  break;
               }
               if(SinkBase::is_valid_instance(sink))
               {
                  sink->on_get_newest_file_status(
                     source->get_manager(), status, uri, pattern);
                  sink = 0;
               }
               source->end_get_newest_file(this);
            }
         } // on_complete


         void NewestFileGetter::on_complete(
            receiver_type *sender,
            receiver_type::client_type::outcome_type outcome)
         {
            SinkBase::get_newest_file_status_type status;
            switch(outcome)
            {
            case receiver_type::client_type::outcome_success:
               status = SinkBase::get_newest_status_complete;
               break;
               
            case receiver_type::client_type::outcome_invalid_server_logon:
               status = SinkBase::get_newest_status_invalid_logon;
               break;
               
            case receiver_type::client_type::outcome_invalid_device_name:
               status = SinkBase::get_newest_status_invalid_uri;
               break;
               
            case receiver_type::client_type::outcome_server_connection_failed:
               status = SinkBase::get_newest_status_connection_failed;
               break;
               
            case receiver_type::client_type::outcome_server_permission_denied:
               status = SinkBase::get_newest_status_server_permission_denied;
               break;
               
            case receiver_type::client_type::outcome_communication_failed:
               status = SinkBase::get_newest_status_communication_failed;
               break;
               
            case receiver_type::client_type::outcome_communication_disabled:
               status = SinkBase::get_newest_status_communication_disabled;
               break;
               
            case receiver_type::client_type::outcome_logger_permission_denied:
               status = SinkBase::get_newest_status_logger_permission_denied;
               break;
               
            case receiver_type::client_type::outcome_invalid_file_name:
               status = SinkBase::get_newest_status_no_file;
               break;
               
            case receiver_type::client_type::outcome_unsupported:
               status = SinkBase::get_newest_status_unsupported;
               break;

            default:
               status = SinkBase::get_newest_status_unknown_failure;
               break;
            }
            if(SinkBase::is_valid_instance(sink))
            {
               sink->on_get_newest_file_status(
                  source->get_manager(), status, uri, pattern, file_name);
               sink = 0;
            }
            source->end_get_newest_file(this);
         } // on_complete


         void NewestFileGetter::start_get_file(StrUni const &file_name_)
         {
            file_name = file_name_;
            try
            {
               receiver.bind(new receiver_type);
               receiver->set_device_name(station_name);
               receiver->set_logger_file_name(file_name.to_utf8());
               receiver->start(this, new newest_file_sink(this), source->get_loggernet_component());
            }
            catch(std::exception &)
            {
               Csi::Event::create(error_event, this)->post();
            }
         } // start_get_file


         void NewestFileGetter::receive(Csi::SharedPtr<Csi::Event> &ev)
         {
            if(ev->getType() == error_event)
            {
               if(SinkBase::is_valid_instance(sink))
               {
                  SinkBase::get_newest_file_status_type status(SinkBase::get_newest_status_connection_failed);
                  if(source->get_last_connect_error() == ManagerClient::disconnect_invalid_logon)
                     status = SinkBase::get_newest_status_invalid_logon;
                  sink->on_get_newest_file_status(source->get_manager(), status, uri, pattern);
               }
               source->end_get_newest_file(this);
            }
         } // receive


         class ClockChecker: public Cora::Device::ClockSetterClient, Csi::EventReceiver
         {
         private:
            /**
             * Specifies the source that owns this component.
             */
            LgrNetSource *source;

            /**
             * Specifies the sink for this component.
             */
            SinkBase *sink;

            /**
             * Specifies the station URI.
             */
            StrUni const uri;

            /**
             * Specifies the component used for the clock.
             */
            typedef Device::ClockSetter setter_type;
            Csi::SharedPtr<setter_type> setter;

            /**
             * Specifies the identifier posted when we can't start.
             */
            static uint4 const error_id;

            /**
             * Specifis the identifier posted when the client has requested the server time.
             */
            static uint4 const server_time_id;

         public:
            /**
             * Constructor
             */
            ClockChecker(
               LgrNetSource *source_,
               SinkBase *sink_,
               StrUni const &uri_,
               bool should_set,
               bool send_server_time,
               Csi::LgrDate const &server_time);

            /**
             * Destructor
             */
            virtual ~ClockChecker()
            { setter.clear(); }

            /**
             * Overloads the client notification.
             */
            virtual void on_complete(
               setter_type *setter,
               outcome_type outcome,
               Csi::LgrDate const &logger_time,
               int8 nanoseconds_difference);

            /**
             * Overloads the event handler.
             */
            virtual void receive(Csi::SharedPtr<Csi::Event> &ev);
         };


         uint4 const ClockChecker::error_id(
            Csi::Event::registerType("Cora::DataSources::LgrNetSource::ClockChecker::error_id"));
         uint4 const ClockChecker::server_time_id(
            Csi::Event::registerType("Cora::DataSources::LgrNetSource::ClockChecker::server_time"));

         
         ClockChecker::ClockChecker(
            LgrNetSource *source_,
            SinkBase *sink_,
            StrUni const &uri_,
            bool should_set,
            bool send_server_time,
            Csi::LgrDate const &server_time):
            source(source_),
            uri(uri_),
            sink(sink_)
         {
            try
            {
               Uri station_uri(uri_);
               if(should_set || station_uri.get_broker_name().length() > 0)
               {
                  setter.bind(new setter_type);
                  setter->set_device_name(station_uri.get_broker_name());
                  if(should_set)
                  {
                     setter->set_should_set_clock(should_set);
                     if(send_server_time)
                        setter->set_server_time(server_time);
                  }
                  setter->start(this, source->get_loggernet_component());
               }
               else
                  Csi::Event::create(server_time_id, this)->post();
            }
            catch(std::exception &)
            {
               Csi::Event::create(error_id, this)->post();
            }
         } // constructor


         void ClockChecker::on_complete(
            setter_type *setter,
            outcome_type outcome_,
            Csi::LgrDate const &logger_time,
            int8 nanoseconds_difference)
         {
            SinkBase::clock_outcome_type outcome(SinkBase::clock_failure_unknown);
            switch(outcome_)
            {
            case outcome_success_clock_checked:
               outcome = SinkBase::clock_success_checked;
               break;
               
            case outcome_success_clock_set:
               outcome = SinkBase::clock_success_set;
               break;
               
            case outcome_session_failed:
               outcome = SinkBase::clock_failure_connection;
               break;
               
            case outcome_invalid_logon:
               outcome = SinkBase::clock_failure_invalid_logon;
               break;
               
            case outcome_server_security_blocked:
               outcome = SinkBase::clock_failure_server_permission;
               break;
               
            case outcome_communication_failed:
               outcome = SinkBase::clock_failure_communication;
               break;
               
            case outcome_communication_disabled:
               outcome = SinkBase::clock_failure_communication_disabled;
               break;
               
            case outcome_logger_security_blocked:
               outcome = SinkBase::clock_failure_logger_permission;
               break;
               
            case outcome_invalid_device_name:
               outcome = SinkBase::clock_failure_invalid_uri;
               break;
               
            case outcome_unsupported:
               outcome = SinkBase::clock_failure_unsupported;
               break;
               
            case outcome_device_busy:
               outcome = SinkBase::clock_failure_busy;
               break;
               
            }
            if(SinkBase::is_valid_instance(sink))
               sink->on_clock_complete(source->get_manager(), uri, outcome, logger_time);
            source->end_clock_check(this);
         } // on_complete


         void ClockChecker::receive(Csi::SharedPtr<Csi::Event> &ev)
         {
            if(ev->getType() == error_id)
            {
               if(SinkBase::is_valid_instance(sink))
                  sink->on_clock_complete(source->get_manager(), uri, SinkBase::clock_failure_connection);
               source->end_clock_check(this);
            }
            else if(ev->getType() == server_time_id)
            {
               if(SinkBase::is_valid_instance(sink))
                  sink->on_clock_complete(source->get_manager(), uri, SinkBase::clock_success_checked, source->get_loggernet_time());
               source->end_clock_check(this);
            }
         } // receive


         /**
          * Defines the class that implements file control for the LgrNet source.
          */
         class FileControl: public Cora::Device::FileControllerClient, Csi::EventReceiver
         {
         private:
            /**
             * Specifies the source that owns this operation.
             */
            LgrNetSource *source;

            /**
             * Specifies the sink that started this operation.
             */
            SinkBase *sink;

            /**
             * Specifies the data source uri for the station.
             */
            StrUni const &uri;

            /**
             * Specifies the control command code.
             */
            uint4 command;

            /**
             * Specifies the first control parameter.
             */
            StrAsc p1;

            /**
             * Specifies the second control parameter.
             */
            StrAsc p2;

            /**
             * Specifies the coratools component used for the file control command.
             */
            typedef Cora::Device::FileController controller_type;
            Csi::SharedPtr<controller_type> controller;

            /**
             * Identifies the event that will get posted when an error is detected in the
             * constructor.
             */
            static uint4 const error_id;
            
         public:
            /**
             * Constructor
             */
            FileControl(
               LgrNetSource *source_,
               SinkBase *sink_,
               StrUni const &uri_,
               uint4 command_,
               StrAsc const &p1_,
               StrAsc const &p2_);

            /**
             * Destructor
             */
            virtual ~FileControl();

            /**
             * Overloads the completion notification.
             */
            virtual void on_complete(
               controller_type *sender, outcome_type outcome);

            /**
             * Overloads the event handler.
             */
            virtual void receive(Csi::SharedPtr<Csi::Event> &ev);
         };


         uint4 const FileControl::error_id(
            Csi::Event::registerType("Cora::DataSources::LgrNetSource::FileControl::error_id"));

         
         FileControl::FileControl(
            LgrNetSource *source_,
            SinkBase *sink_,
            StrUni const &uri_,
            uint4 command_,
            StrAsc const &p1_,
            StrAsc const &p2_):
            source(source_),
            sink(sink_),
            uri(uri_),
            command(command_),
            p1(p1_),
            p2(p2_)
         {
            try
            {
               Uri station_uri(uri_);
               controller.bind(new controller_type);
               controller->set_device_name(station_uri.get_broker_name());
               controller->set_file_command(controller_type::file_command_type(command));
               controller->set_file_argument(p1);
               controller->set_file_argument2(p2);
               controller->start(this, source->get_loggernet_component());
            }
            catch(std::exception &)
            {
               Csi::Event::create(error_id, this)->post();
            }
         } // constructor


         FileControl::~FileControl()
         { controller.clear(); }


         void FileControl::on_complete(
            controller_type *sender, outcome_type outcome_)
         {
            SinkBase::file_control_outcome_type outcome(SinkBase::filecontrol_failure_unknown);
            switch(outcome_)
            {
            case outcome_success:
               outcome = SinkBase::filecontrol_success;
               break;
               
            case outcome_invalid_logon:
               outcome = SinkBase::filecontrol_failure_invalid_logon;
               break;
               
            case outcome_server_session_failed:
               outcome = SinkBase::filecontrol_failure_connection;
               break;
               
            case outcome_invalid_device_name:
               outcome = SinkBase::filecontrol_failure_invalid_uri;
               break;

            case outcome_unsupported:
               outcome = SinkBase::filecontrol_failure_unsupported;
               break;
               
            case outcome_server_security_blocked:
               outcome = SinkBase::filecontrol_failure_server_permission;
               break;
               
            case outcome_logger_communication_failed:
               outcome = SinkBase::filecontrol_failure_communication;
               break;
               
            case outcome_communication_disabled:
               outcome = SinkBase::filecontrol_failure_communication_disabled;
               break;
               
            case outcome_logger_security_blocked:
               outcome = SinkBase::filecontrol_failure_logger_permission;
               break;
               
            case outcome_insufficient_logger_resources:
               outcome = SinkBase::filecontrol_failure_logger_resources;
               break;
               
            case outcome_invalid_file_name:
               outcome = SinkBase::filecontrol_failure_invalid_file_name;
               break;
               
            case outcome_unsupported_command:
               outcome = SinkBase::filecontrol_failure_unsupported_command;
               break;
               
            case outcome_logger_locked:
               outcome = SinkBase::filecontrol_failure_logger_locked;
               break;
               
            case outcome_logger_root_dir_full:
               outcome = SinkBase::filecontrol_failure_logger_root_dir_full;
               break;
               
            case outcome_logger_file_busy:
               outcome = SinkBase::filecontrol_failure_file_busy;
               break;
               
            case outcome_logger_drive_busy:
               outcome = SinkBase::filecontrol_failure_drive_busy;
               break;
            }
            if(SinkBase::is_valid_instance(sink))
            {
               sink->on_file_control_complete(
                  source->get_manager(),
                  uri,
                  command,
                  p1,
                  p2,
                  outcome);
            }
            source->end_file_control(this);
         } // on_complete


         void FileControl::receive(Csi::SharedPtr<Csi::Event> &ev)
         {
            if(ev->getType() == error_id)
            {
               if(SinkBase::is_valid_instance(sink))
               {
                  sink->on_file_control_complete(
                     source->get_manager(),
                     uri,
                     command,
                     p1,
                     p2,
                     SinkBase::filecontrol_failure_connection);
               }
               source->end_file_control(this);
            }
         } // receive


         /**
          * Defines a class that implements terminal service with a station.
          */
         class Terminal: public Cora::Device::TerminalEmulatorClient, public Csi::EventReceiver
         {
         private:
            friend class Cora::DataSources::LgrNetSource;

            /**
             * Identifies an error event.
             */
            static uint4 const error_id;

            /**
             * Specifies the source that owns this object.
             */
            LgrNetSource *source;
            
            /**
             * Specifies the terminal sink.
             */
            TerminalSinkBase *sink;

            /**
             * Specifies the application's sink token.
             */
            int8 sink_token;

            /**
             * Specifies the station URI.
             */
            StrUni const uri;
            
            /**
             * Specifies the terminal emulator component.
             */
            typedef Cora::Device::TerminalEmulator terminal_type;
            Csi::SharedPtr<terminal_type> terminal;

            /**
             * Set to true if the terminal has been started.
             */
            bool terminal_started;

            /**
             * Specifies the data that needs to be sent to the terminal after it has been started.
             */
            StrBin terminal_buffer;

         public:
            /**
             * Constructor
             */
            Terminal(
               LgrNetSource *source_,
               TerminalSinkBase *sink_,
               int8 sink_token_,
               StrUni const &uri_);

            /**
             * Destructor
             */
            virtual ~Terminal()
            { terminal.clear(); }

            /**
             * Overloads the base class version to handle notification that the terminal has been
             * started.
             */
            virtual void on_started(terminal_type *sender)
            {
               terminal_started = true;
               if(terminal_buffer.length() > 0)
                  terminal->send_bytes(terminal_buffer);
               terminal_buffer.cut(0);
            }

            /**
             * Overloads the base class to handle a failure notification.
             */
            virtual void on_failure(terminal_type *sender, failure_type failure);

            /**
             * Overloads the base class version to handle received data.
             */
            virtual void on_received(terminal_type *sender, bool more, StrBin const &content);

            /**
             * Overloads the base class version to handle a start failure.
             */
            virtual void receive(Csi::SharedPtr<Csi::Event> &ev);
         };


         uint4 const Terminal::error_id(
            Csi::Event::registerType("Cora::DataSources::LgrNetSource::Terminal::error_id"));


         Terminal::Terminal(
            LgrNetSource *source_, TerminalSinkBase *sink_, int8 sink_token_, StrUni const &uri_):
            source(source_),
            sink(sink_),
            sink_token(sink_token_),
            uri(uri_),
            terminal_started(false)
         {
            try
            {
               Uri station_uri(uri);
               terminal.bind(new terminal_type);
               terminal->set_device_name(station_uri.get_broker_name());
               terminal->start(this, source->get_loggernet_component());
            }
            catch(std::exception &)
            {
               Csi::Event::create(error_id, this)->post();
            }
         } // constructor


         void Terminal::on_failure(
            terminal_type *sender, failure_type failure_)
         {
            if(TerminalSinkBase::is_valid_instance(sink))
            {
               TerminalSinkBase::terminal_failure_type failure(TerminalSinkBase::terminal_failure_unknown);
               switch(failure_)
               {
               case failure_connection_failed:
                  failure = TerminalSinkBase::terminal_failure_connection;
                  break;
                  
               case failure_invalid_logon:
                  failure = TerminalSinkBase::terminal_failure_invalid_logon;
                  break;
                  
               case failure_server_security_blocked:
                  failure = TerminalSinkBase::terminal_failure_server_security_blocked;
                  break;
                  
               case failure_device_name_invalid:
                  failure = TerminalSinkBase::terminal_failure_invalid_device_name;
                  break;
                  
               case failure_comm_disabled:
                  failure = TerminalSinkBase::terminal_failure_communication_disabled;
                  break;
                  
               case failure_already_servicing_tran:
               case failure_send_failed:
                  failure = TerminalSinkBase::terminal_failure_communication;
                  break;
               }
               sink->on_terminal_failed(source->get_manager(), sink_token, failure);
            }
            source->stop_terminal(sink, sink_token);
         } // on_failure


         void Terminal::on_received(
            terminal_type *sender, bool more, StrBin const &content)
         {
            if(TerminalSinkBase::is_valid_instance(sink))
               sink->on_terminal_content(source->get_manager(), sink_token, content);
            if(!more || !TerminalSinkBase::is_valid_instance(sink))
               source->stop_terminal(sink, sink_token);
         } // on_received


         void Terminal::receive(Csi::SharedPtr<Csi::Event> &ev)
         {
            if(ev->getType() == error_id && TerminalSinkBase::is_valid_instance(sink))
            {
               sink->on_terminal_failed(
                  source->get_manager(), sink_token, TerminalSinkBase::terminal_failure_invalid_device_name);
               source->stop_terminal(sink, sink_token);
            }
         } // receive


         class ListFiles:
            public Cora::Device::FileListerClient,
            public Csi::EventReceiver
         {
         private:
            /**
             * Specifies the source that owns this operation.
             */
            LgrNetSource *source;
            
            /**
             * Specifies the station uri
             */
            StrUni station_uri;

            /**
             * Specifies the data source sink.
             */
            SinkBase *sink;

            /**
             * Specifies the transaction
             */
            int8 transaction;

            /**
             * Specifies the filter.
             */
            StrAsc filter;

            /**
             * Specifies the component responsible for listing files.
             */
            typedef Cora::Device::FileLister lister_type;
            Csi::SharedPtr<lister_type> lister;

            /**
             * Specifies the event that gets posted if the lister cannot be started.
             */
            static uint4 const error_id;

         public:
            /**
             * Constructor
             */
            ListFiles(
               LgrNetSource *source_,
               SinkBase *sink_,
               StrUni const &station_uri_,
               int8 transaction_,
               StrAsc const &filter_);

            /**
             * Destructor
             */
            virtual ~ListFiles()
            { lister.clear(); }

            /**
             * Overloads the base class version to handle the outcome.
             */
            virtual void on_complete(
               lister_type *sender,
               outcome_type outcome,
               file_list_type const &files);

            /**
             * Overloads the base class version to handle an error event.
             */
            virtual void receive(Csi::SharedPtr<Csi::Event> &event);
         };


         uint4 const ListFiles::error_id(
            Csi::Event::registerType("Cora::DataSources::ListFiles::error"));
         

         ListFiles::ListFiles(
            LgrNetSource *source_,
            SinkBase *sink_,
            StrUni const &station_uri_,
            int8 transaction_,
            StrAsc const &filter_):
            source(source_),
            sink(sink_),
            station_uri(station_uri_),
            transaction(transaction_),
            filter(filter_)
         {
            try
            {
               Uri source_uri(station_uri);
               lister.bind(new lister_type);
               lister->set_device_name(source_uri.get_broker_name());
               lister->set_pattern(filter);
               lister->start(this, source->get_loggernet_component());
            }
            catch(std::exception &)
            {
               Csi::Event::create(error_id, this)->post();
            }
         } // constructor


         void ListFiles::on_complete(
            lister_type *sender, outcome_type outcome, file_list_type const &files)
         {
            if(SinkBase::is_valid_instance(sink))
            {
               SinkBase::list_files_outcome_type sink_outcome(SinkBase::list_files_outcome_failure_unknown);
               switch(outcome)
               {
               case outcome_success:
                  sink_outcome = SinkBase::list_files_outcome_success;
                  break;
                  
               case outcome_invalid_logon:
                  sink_outcome = SinkBase::list_files_outcome_failure_logon;
                  break;
                  
               case outcome_session_failure:
                  sink_outcome = SinkBase::list_files_outcome_failure_session;
                  break;
                  
               case outcome_invalid_device_name:
                  sink_outcome = SinkBase::list_files_outcome_failure_invalid_station_uri;
                  break;
                  
               case outcome_blocked_by_server:
                  sink_outcome = SinkBase::list_files_outcome_failure_server_security;
                  break;
                  
               case outcome_unsupported:
                  sink_outcome = SinkBase::list_files_outcome_failure_unsupported;
                  break;
                  
               case outcome_blocked_by_logger:
                  sink_outcome = SinkBase::list_files_outcome_failure_logger_security;
                  break;
                  
               case outcome_comm_disabled:
                  sink_outcome = SinkBase::list_files_outcome_failure_comms_disabled;
                  break;
                  
               case outcome_comm_failed:
                  sink_outcome = SinkBase::list_files_outcome_failure_comms;
                  break;
               }
               sink->on_list_files_complete(
                  source->get_manager(), sink_outcome, station_uri, transaction, filter, files);
            }
            source->end_list_files(this);
         } // on_complete


         void ListFiles::receive(Csi::SharedPtr<Csi::Event> &event)
         {
            if(event->getType() == error_id && SinkBase::is_valid_instance(sink))
            {
               SinkBase::list_files_outcome_type outcome(SinkBase::list_files_outcome_failure_session);
               SinkBase::files_type files;
               sink->on_list_files_complete(
                  source->get_manager(), outcome, station_uri, transaction, filter, files);
               source->end_list_files(this);
            }
         } // receive
      };


      ////////////////////////////////////////////////////////////
      // class LgrNetSource definitions
      ////////////////////////////////////////////////////////////
      uint4 const LgrNetSource::std_retry_interval = 10000;

      
      LgrNetSource::LgrNetSource(
         StrUni const &name, bool password_encrypted_):
         SourceBase(name),
         server_port(6789),
         server_address("localhost"),
         remember(true),
         connect_active(false),
         was_connected(false),
         retry_id(0),
         retry_requests_id(0),
         password_encrypted(password_encrypted_)
      { }


      LgrNetSource::~LgrNetSource()
      {
         if(source_symbol != 0)
            source_symbol->set_source(0);
         source_symbol.clear();
         time_estimator.clear();
      } // destructor


      void LgrNetSource::connect()
      {
         was_connected = true;
         if(!connect_active)
         {
            if(retry_id && timer != 0)
               timer->disarm(retry_id);
            try
            {
               manager->report_source_connecting(this);
               if(time_estimator == 0)
               {
                  time_estimator.bind(new time_estimator_type);
                  time_estimator->set_logon_name(logon_name);
                  time_estimator->set_logon_password(logon_password);
                  time_estimator->set_access_token(access_token);
                  if(source_symbol != 0 && source_symbol->get_loggernet_component() != 0)
                     time_estimator->start(this, source_symbol->get_loggernet_component());
                  else
                  {
                     Csi::SharedPtr<Csi::Messaging::Router> router(
                        new Csi::Messaging::Router(
                           new Csi::SocketConnection(
                              server_address.c_str(), server_port)));
                     time_estimator->start(this, router);
                  }
               }
            }
            catch(std::exception &)
            {
               time_estimator.clear();
               retry_id = timer->arm(this, std_retry_interval);
               manager->report_source_disconnect(this, ManagerClient::disconnect_connection_failed);
            }
         }
      } // connect


      void LgrNetSource::disconnect()
      {
         bool report_disconnect = is_connected();
         time_estimator.clear();
         data_manager.clear();
         connect_active = was_connected = false;
         if(report_disconnect)
            manager->report_source_disconnect(this, ManagerClient::disconnect_by_application);
      } // stop


      bool LgrNetSource::is_connected() const
      { return connect_active; }


      namespace
      {
         StrUni const server_address_name(L"address");
         StrUni const server_port_name(L"port");
         StrUni const logon_name_name(L"logon");
         StrUni const logon_password_name(L"password");
         StrUni const remember_name(L"remember");
         StrUni const password_encrypted_name(L"encrypted");
         StrUni const access_token_name(L"access_token");
      };
      

      void LgrNetSource::get_properties(Csi::Xml::Element &prop_xml)
      {
         prop_xml.set_attr_str(server_address, server_address_name);
         prop_xml.set_attr_uint2(server_port, server_port_name);
         prop_xml.set_attr_wstr(logon_name, logon_name_name);
         if(password_encrypted)
         {
            StrAsc encrypted_password;
            Csi::LoginManager::encrypt_password(encrypted_password, logon_password);
            prop_xml.set_attr_str(encrypted_password, logon_password_name);
         }
         else
            prop_xml.set_attr_wstr(logon_password, logon_password_name);
         prop_xml.set_attr_bool(password_encrypted, password_encrypted_name);
         prop_xml.set_attr_str(access_token, access_token_name);
         prop_xml.set_attr_bool(remember, remember_name);
      } // get_properties


      void LgrNetSource::set_properties(Csi::Xml::Element &prop_xml)
      {
         bool orig_was_connected = was_connected;
         disconnect();
         if(prop_xml.has_attribute(password_encrypted_name))
            password_encrypted = prop_xml.get_attr_bool(password_encrypted_name);
         else
            password_encrypted = false;
         server_address = prop_xml.get_attr_str(server_address_name);
         server_port = prop_xml.get_attr_uint2(server_port_name);
         logon_name = prop_xml.get_attr_wstr(logon_name_name);
         if(prop_xml.has_attribute(access_token_name))
            access_token = prop_xml.get_attr_str(access_token_name);
         if(password_encrypted)
         {
            StrAsc decrypted_password;
            Csi::LoginManager::decrypt_password(
               decrypted_password, prop_xml.get_attr_str(logon_password_name));
            logon_password = decrypted_password.c_str();
         }
         else
            logon_password = prop_xml.get_attr_wstr(logon_password_name);
         if(prop_xml.has_attribute(remember_name))
            remember = prop_xml.get_attr_bool(remember_name);
         if(orig_was_connected)
            connect();
      } // set_properties


      void LgrNetSource::set_server_address(
         StrAsc const &server_address_, uint2 server_port_)
      {
         bool reconnect(was_connected);
         disconnect();
         server_address = server_address_;
         server_port = server_port_;
         if(reconnect)
            connect();
      } // set_server_address


      void LgrNetSource::set_logon_name(StrUni const &logon_name_)
      {
         bool reconnect(was_connected);
         disconnect();
         logon_name = logon_name_;
         if(reconnect)
            connect();
      } // set_logon_name


      void LgrNetSource::set_logon_password(StrUni const &logon_password_)
      {
         bool reconnect(was_connected);
         disconnect();
         logon_password = logon_password_;
         if(reconnect)
            connect();
      } // set_logon_password


      void LgrNetSource::add_request(
         request_handle &request, bool more_to_follow)
      {
         if(SinkBase::is_valid_instance(request->get_sink()))
         {
            using namespace LgrNetSourceHelpers;
            Csi::PolySharedPtr<WartBase, LnWart> wart;
            if(request->get_wart() == 0)
            {
               Uri uri(request->get_uri());
               wart.bind(
                  new LnWart(
                     uri.get_broker_name().c_str(),
                     uri.get_table_name().c_str(),
                     uri.get_column_name().c_str()));
               request->set_wart(wart.get_handle());
            }
            else
               wart = request->get_wart();
            if(connect_active)
            {
               bool needs_start = false;
               if(data_manager == 0)
               {
                  data_manager.bind(new data_manager_type);
                  needs_start = true;
               }
               if(request->get_start_option() != Request::start_date_query)
               {
                  // we will pass all requests for classic logger inlocs tables to the data manager.
                  // All other requests will go directly to data advisors.
                  if(wart->table_name == data_manager_type::get_inlocs_table_name())
                  {
                     clients_type::iterator ci = clients.begin();
                     bool client_added = false;
                     while(!client_added && ci != clients.end())
                     {
                        client_handle &client = *ci;
                        client_added = client->add_request(request);
                        ++ci;
                     }
                     if(!client_added)
                        clients.push_back(new client_type(this, data_manager, request));
                     if(needs_start && !more_to_follow)
                        data_manager->start(time_estimator.get_rep());
                  }
                  else
                  {
                     advisors_type::iterator ai(advisors.begin());
                     bool advisor_added(false);
                     while(!advisor_added && ai != advisors.end())
                     {
                        advisor_handle &advisor(*ai);
                        advisor_added = advisor->add_request(request, more_to_follow);
                        ++ai;
                     }
                     if(!advisor_added)
                        advisors.push_back(new advisor_type(this, request, more_to_follow)); 
                  }
               }
               else
               {
                  bool query_added = false;
                  queries_type::iterator qi = queries.begin();
                  while(qi != queries.end() && !query_added)
                  {
                     query_handle &query = *qi;
                     query_added = query->add_request(request);
                     if(!more_to_follow)
                        query->start(time_estimator.get_rep());
                     ++qi;
                  }
                  if(!query_added)
                  {
                     query_handle query(new LgrNetSourceHelpers::Query(this, request));
                     queries.push_back(query);
                     if(!more_to_follow)
                        query->start(time_estimator.get_rep());
                  }
               }
            }
         }
      } // add_request


      void LgrNetSource::remove_request(request_handle &request)
      {
         // remove any advisors
         advisors_type::iterator ai(advisors.begin());
         while(ai != advisors.end())
         {
            advisors_type::value_type &advisor(*ai);
            if(advisor->remove_request(request) == 0)
            {
               advisors_type::iterator dai(ai++);
               advisors.erase(dai);
            }
            else
               ++ai;
         }
         
         // remove any data manager clients
         clients_type::iterator ci = clients.begin();
         while(ci != clients.end())
         {
            clients_type::value_type &client = *ci;
            if(client->remove_request(request) == 0)
            {
               clients_type::iterator dci = ci++;
               clients.erase(dci);
            }
            else
               ++ci;
         }
         if(clients.empty())
            data_manager.clear();

         // remove any associated queries
         for(queries_type::iterator qi = queries.begin(); qi != queries.end(); ++qi)
         {
            query_handle &query = *qi;
            if(query->remove_request(request))
            {
               queries.erase(qi);
               break;
            }
         }
      } // remove_request


      void LgrNetSource::remove_all_requests()
      {
         queries.clear();
         clients.clear();
         data_manager.clear();
      } // remove_all_requests


      void LgrNetSource::set_request_retry()
      {
         if(retry_requests_id == 0)
            retry_requests_id = timer->arm(this, 15000);
      } // set_request_retry


      Csi::LgrDate LgrNetSource::get_source_time()
      {
         Csi::LgrDate rtn = 0;
         if(is_connected())
         {
            rtn = time_estimator->get_server_time();
         }
         return rtn;
      }


      void LgrNetSource::activate_requests()
      {
         if(!clients.empty() && connect_active)
         {
            try
            {
               data_manager->start(time_estimator.get_rep());
            }
            catch(ClientBase::exc_invalid_state &)
            { trace("data manager already started"); }
            catch(std::exception &e)
            { trace(e.what()); };
         }
         for(advisors_type::iterator ai = advisors.begin(); ai != advisors.end(); ++ai)
         {
            try
            {
               advisor_handle &advisor(*ai);
               advisor->start();
            }
            catch(std::exception &e)
            { trace(e.what()); };
         }
         for(queries_type::iterator qi = queries.begin(); qi != queries.end(); ++qi)
         {
            try
            {
               query_handle &query = *qi;
               query->start(time_estimator.get_rep());
            }
            catch(std::exception &e)
            { trace(e.what()); };
         }
      } // activate_requests


      void LgrNetSource::stop()
      {
         SourceBase::stop();
         if(retry_requests_id != 0)
         {
            timer->disarm(retry_requests_id);
            retry_requests_id = 0;
         }
      } // stop


      void LgrNetSource::set_manager(Manager *manager)
      {
         SourceBase::set_manager(manager);
         if(manager)
            timer = manager->get_timer();
      } // set_manager


      void LgrNetSource::get_statistic_uri(
         std::ostream &out, StrUni const &station_uri, StrUni const &statistic_name)
      {
         using namespace LgrNetSourceHelpers;
         try
         {
            Uri uri(station_uri);
            StrUni broker_name(uri.get_broker_name());
            broker_name.replace(L".", L"\\.");
            out << name << ":__statistics__." << broker_name << "_std." << statistic_name;
         }
         catch(std::exception &)
         { }
      } // get_statistic_uri


      void LgrNetSource::get_statistic_station(
         std::ostream &out, StrUni const &statistic_uri)
      {
         using namespace LgrNetSourceHelpers;
         try
         {
            Uri uri(statistic_uri);
            if(uri.get_broker_name() == L"__statistics__")
            {
               StrUni station_name(uri.get_table_name());
               size_t end_pos = station_name.rfind(L"_std");
               if(end_pos < station_name.length())
               {
                  station_name.cut(end_pos);
                  out << name << ":" << station_name;
               }
            }
         }
         catch(std::exception &)
         { }
      } // get_statistic_station


      void LgrNetSource::get_actual_inloc_table_name(
         StrUni const &uri_, 
         StrUni &table_name)
      {
         LgrNetSourceHelpers::Uri uri(uri_);
         if(data_manager.get_rep())
         {
            bool needs_started = false;
            if(data_manager == 0)
            {
               data_manager.bind(new data_manager_type);
               needs_started = true;
            }
            
            data_manager->get_actual_inloc_table_name(uri.get_broker_name().c_str(), table_name);
            
            if(needs_started)
               data_manager->start(time_estimator.get_rep());
         }
      }


      void LgrNetSource::on_client_advise_failure(client_type *client)
      {
         clients_type::iterator ci(
            std::find_if(clients.begin(), clients.end(), Csi::HasSharedPtr<client_type>(client)));
         if(ci != clients.end())
            clients.erase(ci);
      } // on_client_advise_failure


      void LgrNetSource::on_advisor_failure(advisor_type *advisor)
      {
         advisors_type::iterator ai(
            std::find_if(advisors.begin(), advisors.end(), Csi::HasSharedPtr<advisor_type>(advisor)));
         if(ai != advisors.end())
            advisors.erase(ai);
      } // on_advisor_failure


      bool LgrNetSource::is_classic_inlocs(StrUni const &uri_)
      {
         LgrNetSourceHelpers::Uri uri(uri_);
         bool rtn = false;
         StrUni inlocs_table_name(
            Cora::LgrNet::DataManager::get_inlocs_table_name());
         if(uri.get_table_name() == inlocs_table_name)
            rtn = true;
         return rtn;
      } // is_classic_inlocs


      void LgrNetSource::get_table_range(
         Csi::EventReceiver *client, StrUni const &uri_)
      {
         if(is_connected())
         {
            try
            {
               LgrNetSourceHelpers::Uri uri(uri_);
               range_getter_handle getter(new range_getter_type);
               getter->set_open_broker_active_name(uri.get_broker_name());
               getter->set_table_name(uri.get_table_name());
               table_ranges.push_back(
                  table_range_op(getter, table_range_client(client, uri_)));
               getter->start(this, time_estimator.get_rep());
            }
            catch(std::exception &)
            {
               GetTableRangeCompleteEvent::cpost(
                  client, uri_, GetTableRangeCompleteEvent::outcome_no_table);
            }
         }
         else
            GetTableRangeCompleteEvent::cpost(
               client, uri_, GetTableRangeCompleteEvent::outcome_not_connected);
      } // get_table_range


      void LgrNetSource::on_complete(
         range_getter_type *getter,
         range_getter_type::client_type::outcome_type outcome,
         index_records_type const &index_records)
      {
         for(table_ranges_type::iterator gi = table_ranges.begin(); gi != table_ranges.end(); ++gi)
         {
            table_range_op &op(*gi);
            if(op.first == getter)
            {
               GetTableRangeCompleteEvent::outcome_type reported_outcome(
                  GetTableRangeCompleteEvent::outcome_unknown);
               Csi::LgrDate begin_date;
               Csi::LgrDate end_date;
               if(outcome == range_getter_type::client_type::outcome_success)
               {
                  if(!index_records.empty())
                  {
                     reported_outcome = GetTableRangeCompleteEvent::outcome_success;
                     for(index_records_type::const_iterator ri = index_records.begin();
                         ri != index_records.end();
                         ++ri)
                     {
                        if(ri == index_records.begin() || begin_date > ri->begin_stamp)
                           begin_date = ri->begin_stamp;
                        if(ri == index_records.begin() || end_date < ri->end_stamp)
                           end_date = ri->end_stamp;
                     }
                  }
                  else
                     reported_outcome = GetTableRangeCompleteEvent::outcome_no_records;
               }
               else
               {
                  switch(outcome)
                  {
                  case outcome_invalid_station_name:
                     reported_outcome = GetTableRangeCompleteEvent::outcome_no_broker;
                     break;

                  case outcome_invalid_table_name:
                     reported_outcome = GetTableRangeCompleteEvent::outcome_no_table;
                     break;
                  }
               }
               GetTableRangeCompleteEvent::cpost(
                  op.second.first, op.second.second, reported_outcome, begin_date, end_date);
               table_ranges.erase(gi);
               break;
            }
         }
      } // on_complete


      bool LgrNetSource::start_set_value(
         SinkBase *sink, StrUni const &uri, ValueSetter const &value)
      {
         setters.push_back(new setter_type(sink, uri, value, this));
         return true;
      } // start_set_value


      void LgrNetSource::end_set_value(setter_type *setter)
      {
         setters_type::iterator si(
            std::find_if(setters.begin(), setters.end(), Csi::HasSharedPtr<setter_type>(setter)));
         if(si != setters.end())
            setters.erase(si);
      } // end_set_value


      bool LgrNetSource::start_send_file(
         SinkBase *sink,
         StrUni const &uri,
         StrUni const &dest_file_name,
         StrUni const &file_name)
      {
         file_senders.push_back(new file_sender_type(sink, uri, dest_file_name, file_name, this));
         return true;
      } // start_send_file


      void LgrNetSource::end_send_file(file_sender_type *sender)
      {
         file_senders_type::iterator fi(
            std::find_if(file_senders.begin(), file_senders.end(), Csi::HasSharedPtr<file_sender_type>(sender)));
         if(fi != file_senders.end())
            file_senders.erase(fi);
      } // end_send_file


      bool LgrNetSource::start_get_newest_file(
         SinkBase *sink,
         StrUni const &uri,
         StrUni const &pattern)
      {
         newest_getters.push_back(new newest_getter_type(this, sink,  uri, pattern));
         return true;
      } // start_get_newest_file


      void LgrNetSource::end_get_newest_file(newest_getter_type *getter)
      {
         newest_getters_type::iterator gi(
            std::find_if(
               newest_getters.begin(), newest_getters.end(), Csi::HasSharedPtr<newest_getter_type>(getter)));
         if(gi != newest_getters.end())
            newest_getters.erase(gi);
      } // end_get_newest_file


      bool LgrNetSource::start_clock_check(
         SinkBase *sink,
         StrUni const &uri,
         bool should_set,
         bool send_server_time,
         Csi::LgrDate const &server_time)
      {
         clock_checkers.push_back(
            new clock_checker_type(
               this, sink, uri, should_set, send_server_time, server_time));
         return true;
      } // start_clock_check


      void LgrNetSource::end_clock_check(clock_checker_type *checker)
      {
         clock_checkers_type::iterator ci(
            std::find_if(
               clock_checkers.begin(), clock_checkers.end(), Csi::HasSharedPtr<clock_checker_type>(checker)));
         if(ci != clock_checkers.end())
            clock_checkers.erase(ci);
      } // end_clock_set


      bool LgrNetSource::start_file_control(
         SinkBase *sink,
         StrUni const &uri,
         uint4 command,
         StrAsc const &p1,
         StrAsc const &p2)
      {
         file_controls.push_back(
            new file_control_type(
               this, sink, uri, command, p1, p2));
         return true;
      } // start_file_control


      void LgrNetSource::end_file_control(file_control_type *control)
      {
         file_controls_type::iterator fi(
            std::find_if(
               file_controls.begin(),
               file_controls.end(),
               Csi::HasSharedPtr<file_control_type>(control)));
         if(fi != file_controls.end())
            file_controls.erase(fi);
      } // end_file_control


      bool LgrNetSource::start_list_files(
         SinkBase *sink, StrUni const &station_uri, int8 transaction, StrAsc const &filter)
      {
         listers.push_back(
            new lister_type(
               this, sink, station_uri, transaction, filter));
         return true;
      } // start_list_files


      void LgrNetSource::end_list_files(lister_type *lister)
      {
         listers_type::iterator li(
            std::find_if(
               listers.begin(), listers.end(), Csi::HasSharedPtr<lister_type>(lister)));
         if(li != listers.end())
            listers.erase(li);
      } // end_list_files
      
      void LgrNetSource::breakdown_uri(symbols_type &symbols, StrUni const &uri_)
      {
         try
         {
            LgrNetSourceHelpers::Uri uri(uri_);
            symbols.clear();
            if(uri.get_source_name().length() > 0)
            {
               symbols.push_back(symbol_type(uri.get_source_name(), SymbolBase::type_lgrnet_source));
               if(uri.get_broker_name().length() > 0)
               {
                  if(uri.get_broker_name() == L"__statistics__")
                     symbols.push_back(symbol_type(uri.get_broker_name(), SymbolBase::type_statistics_broker));
                  else
                     symbols.push_back(symbol_type(uri.get_broker_name(), SymbolBase::type_station));
                  if(uri.get_table_name().length() != 0)
                  {
                     symbols.push_back(symbol_type(uri.get_table_name(), SymbolBase::type_table));
                     if(uri.get_column_name().length() > 0)
                        symbols.push_back(symbol_type(uri.get_column_name(), SymbolBase::type_scalar));
                  }
               }
            }
         }
         catch(std::exception &)
         { symbols.clear(); }
      } // breakdown_uri


      namespace
      {
         ////////////////////////////////////////////////////////////
         // functor do_add_requests
         ////////////////////////////////////////////////////////////
         struct do_add_requests
         {
            SourceBase *source;
            do_add_requests(SourceBase *source_):
               source(source_)
            { }

            void operator ()(Manager::request_handle &request)
            {
               if(request->get_source() == source)
               {
                  SinkBase *sink = request->get_sink();
		            SinkBase::sink_failure_type failure = SinkBase::sink_failure_unknown;
                  try
                  {
                     source->add_request(request);
		               request->set_state(source, Request::state_pending);
                  }
                  catch(Cora::LgrNet::InvalidBrokerException &)
                  { failure = SinkBase::sink_failure_invalid_station_name; }
                  catch(Cora::LgrNet::InvalidTableException &)
                  { failure = SinkBase::sink_failure_invalid_table_name; }
                  catch(std::exception &)
                  { }
                  if(request->get_state() != Request::state_pending ||
                     failure != SinkBase::sink_failure_unknown)
                  {
                     request->set_state(source, Request::state_error);
                     if(SinkBase::is_valid_instance(sink))
                        sink->on_sink_failure(source->get_manager(), request, failure);
                  }
               }
            }
         };
      };
      

      void LgrNetSource::on_started(
         time_estimator_type *estimator)
      {
         // report that the connection is now live
         connect_active = true;
         manager->report_source_connect(this);

         // we need to process any requests that are pending from the data manager
         if(is_started())
         {
            if(data_manager == 0)
            {
               data_manager.bind(new data_manager_type);
               data_manager->start(estimator);
            }
            start();
         }
      } // on_started


      void LgrNetSource::on_failure(
         time_estimator_type *estimator,
         time_estimator_type::client_type::failure_type failure)
      {
         // we need to report failures for any pending requests
         using namespace LgrNetSourceHelpers;
         clients_type local_clients(clients);
         queries_type local_queries(queries);

         clients.clear();
         queries.clear();
         while(!local_clients.empty())
         {
            clients_type::value_type client(local_clients.front());
            local_clients.pop_front();
            client->on_advise_failure(0, L"", L"", MyDataManagerClient::failure_connection_failed);
         }
         while(!local_queries.empty())
         {
            queries_type::value_type query(local_queries.front());
            local_queries.pop_front();
            query->on_complete(0, Query::outcome_connection_failed);
         }
         
         // we also need to report the failure to the manager
         ManagerClient::disconnect_reason_type reason = ManagerClient::disconnect_connection_failed;
         Csi::OStrAscStream reason_str;
         switch(failure)
         {
         case failure_invalid_logon:
         case failure_server_security_blocked:
            reason = ManagerClient::disconnect_invalid_logon;
            break;

         case failure_access_token_expired:
            access_token.cut(0);
            break;
         }
         data_manager.clear();
         time_estimator.clear();
         if(failure != failure_access_token_expired || refresh_token.length() == 0)
         {
            connect_active = false;
            if(timer != 0)
               retry_id = timer->arm(this, std_retry_interval);
            last_connect_error = reason;
            time_estimator_type::format_failure(reason_str, failure);
            log_event(reason_str.str());
            manager->report_source_disconnect(this, reason);
         }
         else
         {
            access_token_getter.bind(new access_token_getter_type);
            access_token_getter->set_refresh_token(refresh_token);
            if(source_symbol != 0 && source_symbol->get_loggernet_component() != 0)
               access_token_getter->start(this, source_symbol->get_loggernet_component());
            else
            {
               access_token_getter_type::router_handle router(
                  new Csi::Messaging::Router(
                     new Csi::SocketConnection(server_address.c_str(), server_port)));
               access_token_getter->start(this, router);
            }
         }
      } // on_failure

      void LgrNetSource::on_complete(
         access_token_getter_type *sender,
         access_token_getter_type::client_type::outcome_type outcome,
         StrAsc const &access_token_,
         StrAsc const &refresh_token_)
      {
         connect_active = false;
         if(outcome == AccessTokenGetterClient::outcome_success)
         {
            access_token = access_token_;
            refresh_token = refresh_token_;
            connect();
         }
         else
         {
            Csi::OStrAscStream reason_str;
            
            connect_active = false;
            data_manager.clear();
            time_estimator.clear();
            access_token_getter_type::describe_outcome(reason_str, outcome);
            last_connect_error = ManagerClient::disconnect_connection_failed;
            log_event(reason_str.str());
            manager->report_source_disconnect(this, ManagerClient::disconnect_connection_failed);
            if(timer != 0)
               retry_id = timer->arm(this, std_retry_interval);
         }
         access_token_getter.clear();
      } // on_complete

      void LgrNetSource::onOneShotFired(uint4 id)
      {
         if(id == retry_id)
         {
            retry_id = 0;
            if(was_connected && !connect_active)
               connect();
         }
         else if(id == retry_requests_id)
         {
            // we will use this timed event to attempt to restart any requests for this source that
            // are in an error state.
            retry_requests_id = 0;
            for(Manager::requests_iterator ri = manager->requests_begin();
                ri != manager->requests_end(); ++ri)
            {
               request_handle &request = *ri;
               if(request->get_source() == static_cast<SourceBase *>(this) &&
                  request->get_state() == Request::state_error)
                  add_request(request, true);
            }
            activate_requests();
         }
      } // onOneShotFired


      LgrNetSource::symbol_handle LgrNetSource::get_source_symbol()
      {
         if(source_symbol == 0)
            source_symbol.bind(new LgrNetSourceSymbol(this, get_name()));
         return source_symbol.get_handle();
      } // get_source_symbol


      bool LgrNetSource::start_terminal(
         TerminalSinkBase *sink, StrUni const &station_uri, int8 sink_token)
      {
         bool rtn(true);
         terminal_handle match;
         for(terminals_type::iterator ti = terminals.begin(); match == 0 && ti != terminals.end(); ++ti)
         {
            terminal_handle &candidate(*ti);
            if(candidate->sink == sink && candidate->sink_token == sink_token)
               match = candidate;
         }
         if(match == 0)
         {
            match.bind(new terminal_type(this, sink, sink_token, station_uri));
            terminals.push_back(match);
         }
         else
            rtn = false;
         return rtn;
      } // start_terminal
      
      
      void LgrNetSource::send_terminal(
         TerminalSinkBase *sink, int8 sink_token, void const *buff, size_t buff_len)
      {
         // we need to search for the terminal.
         for(terminals_type::iterator ti = terminals.begin(); ti != terminals.end(); ++ti)
         {
            terminal_handle &terminal(*ti);
            if(terminal->sink == sink && terminal->sink_token == sink_token)
            {
               terminal->terminal_buffer.append(buff, buff_len);
               if(terminal->terminal_started)
               {
                  terminal->terminal->send_bytes(terminal->terminal_buffer);
                  terminal->terminal_buffer.cut(0);
               }
               break;
            }
         }
      } // send_terminal
      
      
      void LgrNetSource::stop_terminal(
         TerminalSinkBase *sink, int8 sink_token)
      {
         for(terminals_type::iterator ti = terminals.begin(); ti != terminals.end(); ++ti)
         {
            terminal_handle terminal(*ti);
            if(terminal->sink == sink && terminal->sink_token == sink_token)
            {
               terminals.erase(ti);
               break;
            }
         }
      } // stop_terminal
   };
};


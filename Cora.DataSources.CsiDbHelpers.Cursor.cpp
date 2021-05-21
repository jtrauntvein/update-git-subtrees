/* Cora.DataSources.CsiDbHelpers.Cursor.cpp

   Copyright (C) 2009, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 02 March 2009
   Last Change: Thursday 07 February 2019
   Last Commit: $Date: 2020-07-14 15:41:32 -0600 (Tue, 14 Jul 2020) $
   Last Changed by: $Author: jbritt $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.DataSources.CsiDbHelpers.Cursor.h"
#include "Cora.DataSources.CsiDbHelpers.Wart.h"
#include "Cora.DataSources.CsiDbSource.h"
#include "Cora.DataSources.SinkBase.h"


namespace Cora
{
   namespace DataSources
   {
      namespace CsiDbHelpers
      {
         namespace
         {
            class RequestFailureEvent: public Csi::Event
            {
            public:
               static uint4 const event_id;

               typedef Cursor::request_handle request_handle;
               request_handle request;

               SinkBase::sink_failure_type failure;

               static void cpost(
                  Cursor *cursor,
                  request_handle &request,
                  SinkBase::sink_failure_type failure)
               {
                  RequestFailureEvent *event = new RequestFailureEvent(cursor, request, failure);
                  event->post();
               }

            private:
               RequestFailureEvent(
                  Cursor *cursor, request_handle &request_, SinkBase::sink_failure_type failure_):
                  Event(event_id, cursor),
                  request(request_),
                  failure(failure_)
               { }
            };


            uint4 const RequestFailureEvent::event_id(
               Csi::Event::registerType("Cora::DataSources::CsiDbHelpers::Cursor::RequestEventFailure"));
         };

         
         Cursor::Cursor(CsiDbSource *source_, request_handle &request):
            source(source_),
            state(state_not_started),
            satisfied_after_first(false),
            reported_ready(false),
            last_record_no(0),
            has_received_data(false)
         {
            requests.push_back(request);
         } // constructor


         Cursor::~Cursor()
         {
            if(pending_query != 0)
            {
               pending_query->client = 0;
               pending_query.clear();
            }
         } // destructor


         bool Cursor::add_request(request_handle &request)
         {
            bool rtn = false;
            Wart *request_wart = static_cast<Wart *>(request->get_wart().get_rep());
            if(!requests.empty() && state == state_not_started)
            {
               request_handle &first = requests.front();
               Wart *first_wart = static_cast<Wart *>(first->get_wart().get_rep());
               if(request_wart->table_name == first_wart->table_name)
                  rtn = first->is_compatible(*request);
            }
            else 
               rtn = (state == state_not_started);
            if(rtn)
               requests.push_back(request);
            return rtn;
         } // add_request


         bool Cursor::remove_request(request_handle &request)
         {
            bool rtn = false;
            requests_type::iterator ri = std::find(requests.begin(), requests.end(), request);
            if(ri != requests.end())
            {
               requests.erase(ri);
               rtn = requests.empty();
            }
            return rtn;
         } // remove_request

#if __getNumRecords
         void Cursor::get_num_records(request_handle &request)
         {
            int rtn(0);
            Wart* wart = static_cast<Wart*>(request->get_wart().get_rep());

            switch (request->get_start_option())
            {
            default:
               break;


            case Request::start_at_time:
            case Request::start_relative_to_newest:
            case Request::start_date_query:
               pending_query.bind(
                  new GetNumRecordsTimeRangeCommand(
                     this,
                     source->get_connection(),
                     wart->table_name,
                     request->get_start_time(),
                     request->get_end_time()));
               state = state_polling;
               break;

            case Request::start_at_newest:
            case Request::start_after_newest:
               rtn = 1;
               break;

            case Request::start_at_offset_from_newest:
               rtn = request->get_start_record_offset();
               state = state_polling;
               break;
            }

            if (pending_query != 0)
               source->add_thread_command(pending_query.get_handle());
         }
#endif

         void Cursor::start()
         {
            if(state == state_not_started)
            {
               // Our first query will depend upon the starting conditions specified by the requests.
               // We will assume that all of the requests share compatible start conditions since this
               // condition is looked for in add_request().
               generate_column_names();
               if(!requests.empty() && pending_query == 0)
               {
                  request_handle &request = requests.front();
                  Wart *wart = static_cast<Wart *>(request->get_wart().get_rep());
                  
                  switch(request->get_start_option())
                  {
                  case Request::start_at_record:
                     pending_query.bind(
                        new PollNewDataCommand(
                           this,
                           source->get_connection(),
                           header,
                           wart->table_name,
                           column_names,
                           request->get_start_time(),
                           request->get_record_no()));
                     break;
                     
                  default:
                     break;

                  case Request::start_at_time:
                     pending_query.bind(
                        new GetDataFromTimeCommand(
                           this,
                           source->get_connection(),
                           wart->table_name,
                           column_names,
                           request->get_start_time()));
                     state = state_polling;
                     break;

                  case Request::start_relative_to_newest:
                     pending_query.bind(
                        new GetDataLastCommand(
                           this,
                           source->get_connection(),
                           wart->table_name,
                           column_names,
                           1));
                     state = state_relative_query;
                     break;
                     
                  case Request::start_at_newest:
                  case Request::start_after_newest:
                     pending_query.bind(
                        new GetDataLastCommand(
                           this,
                           source->get_connection(),
                           wart->table_name,
                           column_names,
                           1));
                     state = state_polling;
                     break;
                     
                  case Request::start_at_offset_from_newest:
                     pending_query.bind(
                        new GetDataLastCommand(
                           this,
                           source->get_connection(),
                           wart->table_name,
                           column_names,
                           request->get_start_record_offset()));
                     state = state_polling;
                     break;
                     
                  case Request::start_date_query:
                     pending_query.bind(
                        new GetDataTimeRangeCommand(
                           this,
                           source->get_connection(),
                           wart->table_name,
                           column_names,
                           request->get_start_time(),
                           request->get_end_time()));
                     state = state_polling;
                     satisfied_after_first = true;
                     break;
                  }
                  if(pending_query != 0)
                     source->add_thread_command(pending_query.get_handle());
                  else
                  {
                     for(requests_type::iterator ri = requests.begin(); ri != requests.end(); ++ri)
                     {
                        request_handle &request(*ri);
                        RequestFailureEvent::cpost(this, request, SinkBase::sink_failure_invalid_start_option);
                     }
                  }
               }
            }
         } // start


         void Cursor::poll()
         {
            if(state == state_ready_to_poll && !requests.empty())
            {
               if(has_received_data)
               {
                  request_handle &request = requests.front();
                  Wart *wart = static_cast<Wart *>(request->get_wart().get_rep());
                  
                  pending_query.bind(
                     new PollNewDataCommand(
                        this,
                        source->get_connection(),
                        header,
                        wart->table_name,
                        column_names,
                        last_stamp,
                        last_record_no));
                  pending_query->return_records(records_cache);
                  pending_query->set_table_metadata(cached_meta);
                  state = state_polling;
                  source->add_thread_command(pending_query.get_handle());
               }
               else
               {
                  state = state_not_started;
                  start();
               }
            }
         } // poll


         void Cursor::receive(Csi::SharedPtr<Csi::Event> &ev)
         {
            // much of the processing that will take place here will need to refer to the request
            // and associated wart.  We will get these now to avoid duplicate code
            request_handle request;
            Wart *wart = 0;
            if(!requests.empty())
            {
               request = requests.front();
               wart = static_cast<Wart *>(request->get_wart().get_rep());
            }

            // process incoming records
            if(ev->getType() == EventQueryHeader::event_id)
            {
               EventQueryHeader *event = static_cast<EventQueryHeader *>(ev.get_rep());
               if(state != state_relative_query && !reported_ready)
               {
                  reported_ready = true;
                  cached_meta = event->command->get_table_metadata();
                  report_ready(event->record);
               }
            }
#ifdef __getNumRecords
            else if (ev->getType() == EventQueryNumRecords::event_id)
            {
               EventQueryNumRecords *event = static_cast<EventQueryNumRecords *>(ev.get_rep());



            }
#endif
            else if(ev->getType() == EventQueryRecords::event_id)
            {
               EventQueryRecords *event = static_cast<EventQueryRecords *>(ev.get_rep());
               if(state == state_relative_query)
               {
                  // if these records are associated with a relative query, the record should be the
                  // newest record in the table.  We will need to relaunch a new query based upon a
                  // time stamp relative to that of the newest record.
                  QueryCommandBase::value_type record(event->records.front());
                  Wart *wart = static_cast<Wart *>(request->get_wart().get_rep());
                  last_stamp = record->get_stamp() + request->get_backfill_interval();
               }
               else
               {
                  // we can process the records that came in here
                  pending_query->continue_query();
                  if(!reported_ready && !event->records.empty())
                  {
                     reported_ready = true;
                     report_ready(event->records.front());
                     if(!is_valid_instance(this))
                        return;
                  }
                  if(!event->records.empty())
                  {
                     report_records(event->records);
                     if(!is_valid_instance(this))
                        return;
                     pending_query->return_records(event->records);
                  }
               }
            }
            else if(ev->getType() == CommandCompleteEvent::event_id)
            {
               CommandCompleteEvent *event = static_cast<CommandCompleteEvent *>(ev.get_rep());
               if(event->command->error_code == 0)
               {
                  if(state == state_relative_query)
                  {
                     // if this event signals the end of the start relative query, we will need to
                     // launch a new query that starts at the time given before.
                     pending_query.bind(
                        new GetDataFromTimeCommand(
                           this,
                           source->get_connection(),
                           wart->table_name,
                           column_names,
                           last_stamp));
                     state = state_polling;
                     source->add_thread_command(pending_query.get_handle());
                  }
                  else
                  {
                     // if the type of query is to make a single query from a begin to end, we will
                     // need to change the state of this cursor and associated requests.
                     if(satisfied_after_first)
                     {
                        requests_type temp(requests);
                        state = state_satisfied;
                        for(requests_type::iterator ri = temp.begin(); ri != temp.end(); ++ri)
                        {
                           request_handle &req(*ri);
                           req->set_state(source, Request::state_satisfied);
                        }
                     }
                     else
                        state = state_ready_to_poll;

                     // the command is complete so we will need to set the request expect_more flag
                     // to indicate that no more data will be forthcoming for the current query. 
                     for(requests_type::iterator ri = requests.begin(); ri != requests.end(); ++ri)
                     {
                        request_handle &request(*ri);
                        request->set_expect_more_data(false);
                     }
                     header = pending_query->get_header();
                     pending_query->get_cache(records_cache);
                     pending_query.clear();
                  }
               }
               else
               {
                  source->log_event(StrAsc("query failed\",\"") + event->command->last_error);
                  on_error(event->command->error_code);
               }
            }
            else if(ev->getType() == RequestFailureEvent::event_id)
            {
               RequestFailureEvent *event = static_cast<RequestFailureEvent *>(ev.get_rep());
               SinkBase *sink = event->request->get_sink();
               event->request->set_state(source, Request::state_error);
               if(SinkBase::is_valid_instance(sink))
                  sink->on_sink_failure(source->get_manager(), event->request, event->failure);
               if(Cursor::is_valid_instance(this))
                  source->remove_request(event->request);
            }
         } // receive


         void Cursor::on_error(int error_code)
         {
            SinkBase::sink_failure_type outcome = SinkBase::sink_failure_unknown;
            requests_type temp(requests);
            switch(error_code)
            {
            case Dll::dberr_not_connected:
               outcome = SinkBase::sink_failure_connection_failed;
               break;
               
            case Dll::dberr_query_fail:
               outcome = SinkBase::sink_failure_invalid_table_name;
               break;
            }
            for(requests_type::iterator ri = temp.begin(); ri != temp.end(); ++ri)
            {
               request_handle &request(*ri);
               RequestFailureEvent::cpost(this, request, outcome);
            }
            if(error_code == Dll::dberr_not_connected)
               event_connect_failure::cpost(source);
         } // on_error


         void Cursor::report_ready(QueryCommandBase::value_type &record)
         {
            requests_type temp(requests);
            for(requests_type::iterator ri = temp.begin(); ri != temp.end(); ++ri)
            {
               request_handle &request = *ri;
               SinkBase *sink = request->get_sink();
               Wart *wart = static_cast<Wart *>(request->get_wart().get_rep());
               if(SinkBase::is_valid_instance(sink))
               {
                  bool report_failure = false;
                  try
                  {
                     request->set_value_indices(*record, wart->column_name);
                     if(request->get_begin_index() != request->get_end_index())
                     {
                        request->set_state(source, Request::state_started);
                        sink->on_sink_ready(source->get_manager(), request, record);
                     }
                     else
                        report_failure = true;
                  }
                  catch(std::exception &)
                  { report_failure = true; }
                  if(report_failure)
                     RequestFailureEvent::cpost(this, request, SinkBase::sink_failure_invalid_column_name);
               }
               else
               {
                  RequestFailureEvent::cpost(this, request, SinkBase::sink_failure_unknown);
               }
            }
         } // report_ready


         void Cursor::report_records(QueryCommandBase::cache_type &records)
         {
            if(!records.empty())
            {
               QueryCommandBase::value_type &record = records.back();
               has_received_data = true;
               last_stamp = record->get_stamp();
               last_record_no = record->get_record_no();
               SinkBase::report_sink_records(source->get_manager(), requests, records);
            }
         } // report_records


         void Cursor::generate_column_names()
         {
            // we will first form a list of column names
            bool use_all = false;
            std::list<StrAsc> names;
            for(requests_type::iterator ri = requests.begin(); ri != requests.end(); ++ri)
            {
               requests_type::value_type &request = *ri;
               Wart *wart = static_cast<Wart *>(request->get_wart().get_rep());
               if(wart->column_name.length() == 0)
               {
                  use_all = true;
                  break;
               }
               else
               {
                  std::list<StrAsc>::iterator ni = std::find(names.begin(), names.end(), wart->column_name);
                  if(ni == names.end())
                     names.push_back(wart->column_name);
               }
            }

            // we will now format that list as a string
            column_names.cut(0);
            if(use_all)
               column_names = "*";
            while(!use_all && !names.empty())
            {
               column_names.append(names.front());
               names.pop_front();
               if(!names.empty())
                  column_names.append(", ");
            }
         } // generate_column_names
      };
   };
};


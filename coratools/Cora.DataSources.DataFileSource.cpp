/* Cora.DataSources.DataFileSource.cpp

   Copyright (C) 2008, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 20 August 2008
   Last Change: Monday 11 July 2016
   Last Commit: $Date: 2016-07-11 09:29:07 -0600 (Mon, 11 Jul 2016) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.DataSources.DataFileSource.h"
#include "Cora.DataSources.Manager.h"
#include "Csi.FileSystemObject.h"
#include <algorithm>


namespace Cora
{
   namespace DataSources
   {
      namespace
      {
         ////////////////////////////////////////////////////////////
         // class read_complete_event
         ////////////////////////////////////////////////////////////
         class read_complete_event: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // read
            ////////////////////////////////////////////////////////////
            typedef DataFileSourceHelpers::MyThread::read_handle read_handle;
            read_handle read;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(DataFileSource *source, read_handle &read)
            {
               read_complete_event *event = new read_complete_event(source, read);
               event->post();
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            read_complete_event(DataFileSource *source, read_handle &read_):
               Event(event_id, source),
               read(read_)
            { }
         };


         uint4 const read_complete_event::event_id = Csi::Event::registerType(
            "Cora::DataSources::DataFileSource::read_complete_event");


         ////////////////////////////////////////////////////////////
         // class read_failed_event
         ////////////////////////////////////////////////////////////
         class read_failed_event: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // failure
            ////////////////////////////////////////////////////////////
            StrAsc const failure;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(DataFileSource *source, StrAsc const &failure)
            {
               read_failed_event *event = new read_failed_event(source, failure);
               event->post();
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            read_failed_event(DataFileSource *source, StrAsc const &failure_):
               Event(event_id, source),
               failure(failure_)
            { }
         };


         uint4 const read_failed_event::event_id = Csi::Event::registerType(
            "Cora::DataSources::DataFileSource::read_failed_event");


         ////////////////////////////////////////////////////////////
         // class activate_event
         ////////////////////////////////////////////////////////////
         class activate_event: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(DataFileSource *source)
            {
               activate_event *event = new activate_event(source);
               event->post();
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            activate_event(DataFileSource *source):
               Event(event_id, source)
            { }
         };


         uint4 const activate_event::event_id =
            Csi::Event::registerType("Cora::DataSources::DataFileSource::activate_event");


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
            // get_table_name
            ////////////////////////////////////////////////////////////
            StrUni const &get_table_name() const
            { return table_name; }

            ////////////////////////////////////////////////////////////
            // get_column_name
            ////////////////////////////////////////////////////////////
            StrUni const get_column_name() const
            { return column_name; }

         private:
            ////////////////////////////////////////////////////////////
            // source_name
            ////////////////////////////////////////////////////////////
            StrUni source_name;

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
            if(uri.first() == L'\"')
               start_pos = 1;
            source_name.cut(0);
            table_name.cut(0);
            column_name.cut(0);
            uri.sub(source_name, start_pos, source_end_pos - start_pos);
            if(source_name.last() == '\"')
               source_name.cut(source_name.length() - 1);
            if(source_end_pos < uri.length())
            {
               StrUni remnant;
               typedef std::list<StrUni> tokens_type;
               tokens_type tokens;
               StrUni token;
               
               uri.sub(remnant, source_end_pos + 1, uri.length());
               if(remnant.last() == L'\"')
                  remnant.cut(remnant.length() - 1);

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

               // we need to combine any tokens that may have been "commented"
               tokens_type::iterator ti = tokens.begin();
               while(ti != tokens.end())
               {
                  if(ti->last() == L'\\')
                  {
                     tokens_type::iterator ti_next = ti;
                     if(++ti_next != tokens.end())
                     {
                        ti->cut(ti->length() - 1);
                        ti->append(L'.');
                        ti->append(*ti_next);
                        tokens.erase(ti_next);
                     }
                     ++ti;
                  }
                  else
                     ++ti;
               }

               // the data file uri should have, at most, two levels: table name, and column name
               if(!tokens.empty())
               {
                  table_name = tokens.front();
                  tokens.pop_front();
               }
               while(!tokens.empty())
               {
                  column_name += tokens.front();
                  tokens.pop_front();
               }
            }
         } // parse


         uint4 const std_retry_interval(10000);
      };

      
      ////////////////////////////////////////////////////////////
      // class DataFileSource definitions
      ////////////////////////////////////////////////////////////
      DataFileSource::DataFileSource(StrUni const &source_name):
         SourceBase(source_name),
         backfill_bytes(0xFFFFFFFF),
         was_connected(false),
         retry_id(0),
         poll_id(0),
         poll_schedule_interval(5 * Csi::LgrDate::msecPerMin),
         poll_schedule_enabled(true)
      {
      } // constructor


      DataFileSource::~DataFileSource()
      {
         if(source_symbol != 0)
            source_symbol->set_source(0);
         source_symbol.clear();
         if(retry_id != 0)
            timer->disarm(retry_id);
         if(poll_id != 0)
            scheduler->cancel(poll_id);
         thread.clear();
         was_started = false;
         retry_id = poll_id = 0;
         reader.clear();  
         timer.clear();
         scheduler.clear();
      } // destructor


      void DataFileSource::connect()
      {
         was_connected = true;
         try
         {
            manager->report_source_connecting(this);
            get_reader(false);
            thread.bind(new thread_type(this, reader));
            manager->report_source_connect(this);
            if(is_started())
               start();
         }
         catch(std::exception &)
         {
            retry_id = timer->arm(this, std_retry_interval);
            manager->report_source_disconnect(this, ManagerClient::disconnect_connection_failed);
         }
      } // connect


      void DataFileSource::disconnect()
      {
         bool report_disconnect = is_connected();
         if(retry_id != 0)
            timer->disarm(retry_id);
         thread.clear();
         was_connected = false;
         retry_id = 0;
         reader.clear();
         if(report_disconnect)
            manager->report_source_disconnect(this, ManagerClient::disconnect_by_application);
      } // disconnect


      bool DataFileSource::is_connected() const
      {
         bool rtn = was_connected;
         if(rtn && reader == 0)
            rtn = false; 
         return rtn;
      } // is_connected


      void DataFileSource::start()
      {
         SourceBase::start();
         if(poll_schedule_interval < 1000)
            poll_schedule_interval = 1000;
         poll_id = scheduler->start(this, poll_schedule_base, poll_schedule_interval);
      } // start


      void DataFileSource::stop()
      {
         if(poll_id != 0)
         {
            scheduler->cancel(poll_id);
            poll_id = 0;
         }
         SourceBase::stop();
      } // stop


      namespace
      {
         StrUni const file_name_name(L"file-name");
         StrUni const labels_file_name_name(L"labels-file");
         StrUni const poll_schedule_base_name(L"poll-base");
         StrUni const poll_schedule_interval_name(L"poll-int");
         StrUni const backfill_bytes_name(L"backfill-bytes");
      };
      

      void DataFileSource::get_properties(Csi::Xml::Element &prop_xml)
      {
         prop_xml.set_attr_str(file_name, file_name_name);
         prop_xml.set_attr_str(labels_file_name, labels_file_name_name);
         prop_xml.set_attr_lgrdate(poll_schedule_base, poll_schedule_base_name);
         prop_xml.set_attr_uint4(poll_schedule_interval, poll_schedule_interval_name);
         prop_xml.set_attr_int8(backfill_bytes, backfill_bytes_name);
      } // get_properties


      void DataFileSource::set_properties(Csi::Xml::Element &prop_xml)
      {
         stop();
         file_name = prop_xml.get_attr_str(file_name_name);
         labels_file_name = prop_xml.get_attr_str(labels_file_name_name);
         poll_schedule_base = prop_xml.get_attr_lgrdate(poll_schedule_base_name);
         poll_schedule_interval = prop_xml.get_attr_uint4(poll_schedule_interval_name);
         if(poll_schedule_interval <= 0) //Make sure we have a valid interval else set to default
            poll_schedule_interval = 5 * Csi::LgrDate::msecPerMin;
         if(prop_xml.has_attribute(backfill_bytes_name))
            backfill_bytes = prop_xml.get_attr_int8(backfill_bytes_name);
         if(manager != 0)
            manager->report_source_disconnect(this, ManagerClient::disconnect_properties_changed);
      } // set_properties


      void DataFileSource::set_manager(Manager *manager)
      {
         SourceBase::set_manager(manager);
         if(manager)
         {
            timer = manager->get_timer();
            scheduler.bind(new Scheduler(timer));
         }
      } // set_manager


      namespace
      {
         ////////////////////////////////////////////////////////////
         // functor reader_has_name
         ////////////////////////////////////////////////////////////
         struct reader_has_name
         {
            StrUni const table_name;
            reader_has_name(StrUni const &table_name_):
               table_name(table_name_.c_str())
            { }

            bool operator ()(Cora::Broker::DataFileReader::value_type &desc)
            { return desc.second->table_name == table_name; }
         };
      };
      

      void DataFileSource::add_request(request_handle &request, bool more_to_follow)
      {
         // we will do nothing if this source is not started
         if(reader == 0 || !SinkBase::is_valid_instance(request->get_sink()))
            return;
         
         // we need to to determine the the array that is referred by this URI
         Uri const uri(request->get_uri());
         reader_type::iterator ri = std::find_if(
            reader->begin(), reader->end(), reader_has_name(uri.get_table_name()));
         if(ri != reader->end())
         {
            // we now need to determine whether any of the current read states can service this request.
            uint4 array_id = ri->first;
            reader_type::record_handle record;
            bool request_added = false;
            read_states_type::iterator rsi = read_states.begin();
            read_state_handle read_state;
            
            while(!request_added && rsi != read_states.end())
            {
               read_state_handle &state = *rsi;
               request_added = state->add_request(request, array_id);
               if(request_added)
               {
                  record = state->results.front();
                  read_state = state;
               }
               else
                  ++rsi;
            }
            if(!request_added)
            {
               read_state = new DataFileSourceHelpers::ReadState(request, array_id);
               read_states.push_back(read_state);
               record = reader->make_record(array_id);
               read_state->results.push_back(record);
            }

            // we now need to report to the sink that the request is ready
            request->set_value_indices(*record, uri.get_column_name());
            request->get_sink()->on_sink_ready(get_manager(), request, record);
            if(is_started() && thread != 0 && !more_to_follow)
            {
               thread->add_read(read_state);
            }
         }
      } // add_request


      void DataFileSource::activate_requests()
      {
         if(is_started())
            activate_event::cpost(this);
      } // activate_requests
      

      void DataFileSource::remove_request(request_handle &request)
      {
         for(read_states_type::iterator si = read_states.begin();
             si != read_states.end();
             ++si)
         {
            read_states_type::value_type &read_state = *si;
            read_state_type::requests_type::iterator ri = std::find(
               read_state->requests.begin(), read_state->requests.end(), request);
            if(ri != read_state->requests.end())
            {
               read_state->requests.erase(ri);
               if(read_state->requests.empty())
               {
                  read_states.erase(si);
               }
               break;
            }
         }
      } // remove_request


      void DataFileSource::remove_all_requests()
      {
         read_states.clear();
      } // remove_all_requests


      DataFileSource::symbol_handle DataFileSource::get_source_symbol()
      {
         if(source_symbol == 0)
            source_symbol.bind(new DataFileSymbol(this));
         return source_symbol.get_handle();
      } // get_source_symbol


      void DataFileSource::onOneShotFired(uint4 id)
      {
         if(id == retry_id)
         {
            retry_id = 0;
            connect();
         }
      } // onOneShotFired

      
      void DataFileSource::onScheduledEvent(uint4 id)
      {
         if(id == poll_id && poll_schedule_enabled)
         {
            // if the file has changed, we can start polling it.
            Csi::FileSystemObject file_info(file_name.c_str());
            for(read_states_type::iterator si = read_states.begin(); si != read_states.end(); ++si)
            {
               bool do_poll = false;
               read_states_type::value_type &state = *si;

               if(!state->previously_polled || file_info.get_last_write_date() != state->last_change_date)
               {
                  do_poll = true;
                  state->last_change_date = file_info.get_last_write_date();
               }
               if(do_poll && !state->poll_pending && !state->is_satisfied())
               {
                  thread->add_read(state);
               }
            }
         }
      } // onScheduledEvent


      DataFileSource::reader_handle &DataFileSource::get_reader(bool do_reload)
      {
         if(reader == 0 && was_connected)
         {
            reader = Cora::Broker::make_data_file_reader(
               file_name, manager->get_value_factory(), labels_file_name);
            if(source_symbol != 0 && do_reload)
               source_symbol->reload_symbols();
         }
         return reader;
      } // get_reader


      void DataFileSource::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         if(ev->getType() == read_complete_event::event_id)
         {
            read_complete_event *event = static_cast<read_complete_event *>(ev.get_rep());
            read_states_type::iterator si = std::find(
               read_states.begin(), read_states.end(), event->read);
            if(si != read_states.end())
            {
               // if there is no data to be reported, the requests should be considered to be
               // satisifed
               read_states_type::value_type read = *si;
               assert(read->poll_pending == true);
               read->previously_polled = true;
               if(!read->has_more_data)
               {
                  for(read_state_type::requests_type::iterator ri = read->requests.begin();
                      ri != read->requests.end();
                      ++ri)
                  {
                     request_handle &request(*ri);
                     request->set_expect_more_data(false);
                     if(request->get_start_option() == Request::start_date_query)
                        request->set_state(this, Request::state_satisfied);
                  }
               }

               // report the records
               if(!read->results.empty())
                  SinkBase::report_sink_records(
                     get_manager(), read->requests, read->results);

               read->poll_pending = false;

               if(read->has_more_data)
                  thread->add_read(read);
            }
         }
         else if(ev->getType() == read_failed_event::event_id)
         {
            retry_id = timer->arm(this, std_retry_interval);
            while(!read_states.empty())
            {
               read_state_handle state(read_states.front());
               read_states.pop_front();
               while(!state->requests.empty())
               {
                  request_handle request(state->requests.front());
                  SinkBase *sink = request->get_sink();
                  
                  state->requests.pop_front();
                  request->set_state(this, Request::state_error);
                  if(SinkBase::is_valid_instance(sink))
                     sink->on_sink_failure(get_manager(), request, SinkBase::sink_failure_connection_failed);
               }
            }
            reader.clear();
            thread.clear();
            manager->report_source_disconnect(this, ManagerClient::disconnect_connection_failed);
         }
         else if(ev->getType() == activate_event::event_id)
         {
            if(is_started() && thread != 0)
            {
               for(read_states_type::iterator si = read_states.begin();
                   si != read_states.end();
                   ++si)
               {
                  read_states_type::value_type &state = *si;
                  if(!state->poll_pending && !state->previously_polled)
                  {
                     thread->add_read(state);
                  }
               }
            } 
         }
      } // receive


      void DataFileSource::breakdown_uri(symbols_type &symbols, StrUni const &uri_)
      {
         try
         {
            Uri uri(uri_);
            symbols.clear();
            if(uri.get_source_name().length() > 0)
            {
               symbols.push_back(symbol_type(uri.get_source_name(), SymbolBase::type_file_source));
               if(uri.get_table_name().length() > 0)
               {
                  symbols.push_back(symbol_type(uri.get_table_name(), SymbolBase::type_table));
                  if(uri.get_column_name().length() > 0)
                     symbols.push_back(symbol_type(uri.get_column_name(), SymbolBase::type_scalar));
               }
            }
         }
         catch(std::exception &)
         { symbols.clear(); }
      } // breakdown_uri


      namespace DataFileSourceHelpers
      {
         ////////////////////////////////////////////////////////////
         // class ReadState definitions
         ////////////////////////////////////////////////////////////
         ReadState::ReadState(request_handle &request, uint4 array_id_):
            poll_pending(false),
            current_offset(-1),
            array_id(array_id_),
            previously_polled(false)
         {
            if(request->get_start_option() == Request::start_at_record)
               last_record_stamp = request->get_start_time();
            requests.push_back(request);
         } // constructor


         ReadState::~ReadState()
         { }


         bool ReadState::add_request(request_handle &request, uint4 array_id)
         {
            bool rtn = false;
            if(array_id == this->array_id && !requests.empty() &&
               poll_pending == false && previously_polled == false &&
               requests.front()->is_compatible(*request))
            {
               requests.push_back(request);
               rtn = true;
            }
            return rtn;
         } // add_request

         
         namespace
         {
            struct index_has_array
            {
               uint4 const array_id;
               index_has_array(uint4 id):
                  array_id(id)
               { }

               bool operator() (Cora::Broker::DataFileIndexEntry const &entry) const
               { return entry.array_id == array_id; }
            };


            struct index_has_record_no
            {
               uint4 const record_no;
               index_has_record_no(uint4 record_no_):
                  record_no(record_no_)
               { }

               bool operator() (Cora::Broker::DataFileIndexEntry const &entry) const
               { return entry.record_no >= record_no; }
            };
         };
         

         void ReadState::poll(
            Cora::Broker::DataFileReader &reader,
            Cora::Broker::DataFileReader::index_type &index,
            bool &must_stop)
         {
            // make a copy of the old results container.  we will re-use any records that may be
            // there
            using Cora::Broker::DataFileReader;
            results_type cache(results); 
            results.clear();
            has_more_data = false;
            if(requests.empty())
               return;

            // if the file is mixed array and there is more than one array, we might need to select
            // only the entries associated with the array id for this cursor.
            typedef DataFileReader::index_type index_type;
            index_type array_index;
            index_type *keys(&index);

            if(reader.size() > 1)
            {
               keys = &array_index;
               index_has_array predicate(array_id);
               for(index_type::iterator ii = index.begin(); ii != index.end(); ++ii)
               {
                  if(predicate(*ii))
                     array_index.push_back(*ii);
               }
            }
            
            // we now need to determine the starting position in the index.  This will be determined
            // based upon whether we have been previously started and the request start conditions
            // and order options.
            requests_type::value_type request(requests.front());
            uint4 const max_records(request->get_cache_size_controller());
            DataFileReader::index_type::iterator ri(keys->begin());
            uint4 start_offset(request->get_start_record_offset());
            
            if(ri == keys->end())
               return;
            if(!previously_polled)
            {
               DataFileReader::index_type::value_type key;
               switch(request->get_start_option())
               {
               case Request::start_at_newest:
               case Request::start_after_newest:
                  ri = keys->end() - 1;
                  break;

               case Request::start_at_time:
               case Request::start_date_query:
                  key.time_stamp = request->get_start_time();
                  key.record_no = 0;
                  key.array_id = array_id;
                  ri = std::lower_bound(keys->begin(), keys->end(), key);
                  break;

               case Request::start_at_record:
                  ri = std::find_if(
                     keys->begin(), keys->end(), index_has_record_no(request->get_record_no()));
                  if(ri == keys->end())
                     ri = keys->begin();
                  break;

               case Request::start_relative_to_newest:
                  key = keys->back();
                  key.time_stamp = key.time_stamp + request->get_backfill_interval() - 1;
                  key.record_no = 0;
                  key.array_id = array_id;
                  ri = std::lower_bound(keys->begin(), keys->end(), key);
                  if(ri == keys->end())
                     ri = keys->begin();
                  break;

               case Request::start_at_offset_from_newest:
                  if(start_offset > (uint4)keys->size())
                     start_offset = (uint4)keys->size();
                  ri = keys->end() - start_offset;
                  break;
               }
               if(ri != keys->end())
                  previously_polled = true;
            }
            else
            {
               // we have been previously polled but we still need to find the beginning offset.
               // This will depend upon the order option and previous results.
               if(request->get_order_option() == Request::order_real_time)
                  ri = keys->end() - 1;
               else
               {
                  // we will search for the index entry for the last record reported and, if found,
                  // we will go one beyond that.
                  ri = std::lower_bound(keys->begin(), keys->end(), last_record_entry);
                  if(ri != keys->end() && *ri == last_record_entry)
                     ++ri;
               }
            }

            // now that we have found the starting location, we need to read the records starting at
            // the iterator and going up to the end.
            DataFileReader::index_type::const_iterator ci;
            has_more_data = true;
            for(ci = ri; has_more_data && !must_stop && ci != keys->end(); ++ci)
            {
               results_type::value_type record;
               if(!cache.empty())
               {
                  record = cache.front();
                  cache.pop_front();
               }
               else
                  record = reader.make_record(array_id);
               last_record_entry = *ci;
               if(request->get_start_option() == Request::start_date_query &&
                  last_record_entry.time_stamp >= request->get_end_time())
               {
                  has_more_data = false;
                  break;
               }
               reader.seek_data(last_record_entry.data_offset, false);
               if(reader.read_next_record(record, 0, 0, array_id) == DataFileReader::read_outcome_success)
               {
                  results.push_back(record);
                  if(request->get_cache_size_controller() > 0 && results.size() > request->get_cache_size_controller())
                     break;
               }
               else
                  has_more_data = false;
            }
            if(has_more_data && ci == keys->end())
               has_more_data = false;
         } // poll


         bool ReadState::is_satisfied() const
         {
            bool rtn = false;
            if(!requests.empty())
            {
               request_handle const &request = requests.front();
               rtn = (request->get_state() == Request::state_satisfied);
            }
            return rtn;
         } // is_satisfied

         
         ////////////////////////////////////////////////////////////
         // class MyThread definitions
         ////////////////////////////////////////////////////////////
         MyThread::MyThread(DataFileSource *source_, reader_handle &reader_):
            reader(reader_),
            source(source_),
            must_stop(false)
         { start(); }


         MyThread::~MyThread()
         { wait_for_end(); }


         void MyThread::start()
         {
            must_stop = false;
            Thread::start();
         } // start


         void MyThread::wait_for_end()
         {
            must_stop = true;
            trigger.set();
            Thread::wait_for_end();
         } // wait_for_end


         void MyThread::add_read(read_handle &read)
         {
            mutex.lock();
            read->poll_pending = true;
            reads.push_back(read);
            mutex.unlock();
            trigger.set();
         } // add_read


         struct index_older_than
         {
            int8 offset;
            index_older_than(int8 offset_):
               offset(offset_)
            { }

            bool operator ()(Broker::DataFileIndexEntry const &entry) const
            { return entry.data_offset < offset; }
         };
         

         void MyThread::execute()
         {
            try
            {
               // before processing any read cursors, we will need to first generate an index for
               // the portion of the file specified by the source.  To begin with, we need to
               // calculate the starting data offset based on the source backfill bytes.
               int8 backfill(source->get_backfill_bytes());
               int8 data_len(reader->get_data_len());
               int8 start_pos(0);
               int8 last_pos(0);
               uint4 next_record_no(0);

               if(backfill > 0 && backfill < data_len)
                  start_pos = data_len - backfill;

               // we can now generate the records index
               typedef Cora::Broker::DataFileReader::index_type index_type;
               index_type index;
               reader->seek_data(start_pos, true);
               reader->generate_index(index, must_stop, &next_record_no);
               if(!index.empty())
                  last_pos = index.back().data_offset;
               std::sort(index.begin(), index.end());
               
               // we can now process the read cursors.
               bool reader_is_hibernating = false;
               bool all_data_overwritten = false;
               while(!must_stop)
               {
                  // determine if there is a cursor that needs service.
                  trigger.reset();
                  mutex.lock();
                  read_handle current;
                  if(!reads.empty())
                  {
                     current = reads.front();
                     reads.pop_front();
                  }
                  mutex.unlock();
                  if(current != 0)
                  {
                     // if the reader is hibernating, we need to revive it.
                     if(reader_is_hibernating)
                     {
                        // wake up the reader
                        reader_is_hibernating = false;
                        if(!reader->wake_up(all_data_overwritten))
                           throw Csi::MsgExcept("Failed to revive the reader");

                        // if the data file has been overwritten, we will need to regenerate our
                        // index.  Otherwise, we will need to generate an index for any newly added records.
                        if(all_data_overwritten || index.empty())
                        {
                           next_record_no = 0;
                           data_len = reader->get_data_len();
                           start_pos = 0;
                           if(backfill > 0 && backfill < data_len)
                              start_pos = data_len - backfill;
                           reader->seek_data(start_pos);
                           reader->generate_index(index, must_stop, &next_record_no);
                           if(!index.empty())
                              last_pos = index.back().data_offset;
                           std::sort(index.begin(), index.end());
                        }
                        else
                        {
                           // we will seek to the last position and attempt to index from that
                           // position.  Since this could be a mixed array file and the last record
                           // may be incomplete, we will allow the last record to be read again.
                           index_type new_index;
                           reader->seek_data(last_pos, false);
                           --next_record_no;
                           reader->generate_index(new_index, must_stop, &next_record_no);
                           if(!new_index.empty())
                           {
                              last_pos = new_index.back().data_offset;
                              new_index.erase(new_index.begin());
                           }
                           std::sort(new_index.begin(), new_index.end());

                           // we need to remove any index records in the file that lie before our
                           // start position.
                           index_type::iterator ri(
                              std::remove_if(index.begin(), index.end(), index_older_than(start_pos)));
                           if(ri != index.end())
                              index.erase(ri, index.end());

                           // the new index can now be appended to the old.  Before doing so,
                           // however, we need to decide whether the merged index needs to be
                           // sorted.
                           bool sort_after(false);
                           if(!new_index.empty() && new_index.front() <= index.back())
                              sort_after = true;
                           index.insert(index.end(), new_index.begin(), new_index.end());
                           if(sort_after)
                              std::sort(index.begin(), index.end());
                        }
                     }
                     
                     // send an event back to the source that the read is complete
                     reader->resynch();
                     current->poll(*reader, index, must_stop);
                     read_complete_event::cpost(source, current);
                     current.clear();
                  }
                  else
                  {
                     reader->hibernate();
                     reader_is_hibernating = true;
                     all_data_overwritten = false;
                     trigger.wait(1000);
                  }
               }
            }
            catch(std::exception &e)
            {
               read_failed_event::cpost(source, e.what());
            }
         } // execute
      };
   };
};


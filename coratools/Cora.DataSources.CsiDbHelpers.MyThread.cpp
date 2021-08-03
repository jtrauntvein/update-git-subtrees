/* Cora.DataSources.CsiDbHelpers.MyThread.cpp

   Copyright (C) 2009, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Saturday 07 February 2009
   Last Change: Thursday 07 February 2019
   Last Commit: $Date: 2020-07-14 15:41:32 -0600 (Tue, 14 Jul 2020) $
   Last Changed by: $Author: jbritt $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.DataSources.CsiDbHelpers.MyThread.h"
#include "Cora.DataSources.Manager.h"
#include <iterator>
#include <locale.h>
#include <limits>
#ifdef max
#undef max
#undef min
#endif


namespace Cora
{
   namespace DataSources
   {
      namespace CsiDbHelpers
      {
         MyThread::MyThread():
            should_quit(false),
            add_signal(0, false, false)
         {
            start();
         } // constructor


         MyThread::~MyThread()
         {
            wait_for_end();
         } // destructor


         void MyThread::wait_for_end()
         {
            should_quit = true;
            add_signal.set();
            Thread::wait_for_end();
         } // wait_for_end


         void MyThread::add_command(command_handle command)
         {
            commands_protector.lock();
            commands.push_back(command);
            commands_protector.unlock();
            add_signal.set();
         } // add_command
      

         void MyThread::execute()
         {
            // before we load the DLL, we want to ensure that any changes it makes to the global locale
            // are confined to this thread
            _configthreadlocale(_ENABLE_PER_THREAD_LOCALE);

            // we can now load the DLL.
            Csi::SharedPtr<Dll> dll;
            command_handle current;
            while(!should_quit)
            {
               // check to see if there is a command to execute
               current.clear();
               commands_protector.lock();
               if(!commands.empty())
               {
                  current = commands.front();
                  commands.pop_front();
               }
               commands_protector.unlock();

               // now execute the command
               if(current != 0)
               {
                  try
                  {
                     if(dll == 0)
                        dll.bind(new Dll);
                     current->execute(dll.get_rep());
                  }
                  catch(std::exception &e)
                  {
                     trace("db command failed: %s", e.what());
                     current->error_code = Dll::dberr_unknown;
                     if(get_last_error_function != 0)
                        current->last_error = get_last_error_function();
                     current->release();
                  }

                  try
                  {
                     if(Csi::EventReceiver::is_valid_instance(current->client))
                        CsiDbHelpers::CommandCompleteEvent::cpost(current);
                  }
                  catch(std::exception &e)
                  {
                     trace("CsiDbHelpers::CommandCompleteEvent::cpost: %s", e.what());
                  }
               }
               else
                  add_signal.wait(10000);
            }
            dll.clear();
         } // execute


         uint4 const CommandCompleteEvent::event_id =
            Csi::Event::registerType("Cora::DataSources::CsiDbHelpers::MyThread::CommandComplete");
         

         uint4 const ConnectCommand::command_id(
            Csi::Event::registerType("Cora::DataSourcea::CsiDbHelpers::MyThread::ConnectCommand"));

            
         ConnectCommand::ConnectCommand(
            Csi::EventReceiver *client, DbProperties const &properties_):
            CommandBase(command_id, client),
            properties(properties_)
         { }


         ConnectCommand::~ConnectCommand()
         { connection.clear(); }

         
         void ConnectCommand::execute(Dll *dll)
         {
            try
            {
               connection = dll->connect(properties);
            }
            catch(CsiDbException &e)
            {
               error_code = e.get_error_code();
               if(get_last_error_function)
                  last_error = get_last_error_function();
            } 
         }


         uint4 const DisconnectCommand::command_id(
            Csi::Event::registerType("Cora::DataSources::CsiDbHelpers::MyThread::DisconnectCommand"));

         
         uint4 const ListTablesCommand::command_id(
            Csi::Event::registerType("Cora::DataSources::CsiDbHelpers::MyThread::ListTablesCommanbd"));

            
         ListTablesCommand::ListTablesCommand(
            Csi::EventReceiver *client, Dll::connection_handle &connection_):
            CommandBase(command_id, client),
            connection(connection_)
         { }


         ListTablesCommand::~ListTablesCommand()
         { connection.clear(); }


         void ListTablesCommand::execute(Dll *dll)
         {
            try
            { connection->list_tables(table_names); }
            catch(CsiDbException &e)
            {
               error_code = e.get_error_code();
               if(get_last_error_function)
                  last_error = get_last_error_function();
            } 
         } // execute


         uint4 const ListColumnsCommand::command_id(
            Csi::Event::registerType("Cora::DataSources::CsiDbHelpers::MyThread::ListColumnsCommand"));


         void ListColumnsCommand::execute(Dll *dll)
         {
            try
            { connection->list_columns(column_names, table_name); }
            catch(CsiDbException &e)
            {
               error_code = e.get_error_code();
               if(get_last_error_function)
                  last_error = get_last_error_function();
            }
         } // execute

#ifdef __getNumRecords
         uint4 const NumRecordsCommand::command_id(
            Csi::Event::registerType("Cora::DataSources::CsiDbHelpers::MyThread::NumRecordsCommand"));

         void NumRecordsCommand::execute(Dll *dll)
         {
            try
            {
               num_records = 0;
               get_num_records_in_table();
            }
            catch(CsiDbException &e)
            {
               error_code = e.get_error_code();
               if(get_last_error_function)
                  last_error = get_last_error_function();
            }
         } // execute


         void NumRecordsCommand::get_num_records_in_table()
         {
            uint4 num_records_handle(0);
            num_records = 0;
            try
            {
               // we need to launch the query
               int rcd(0);
               if (get_num_records_function == nullptr)
                  return;
               num_records_handle = get_num_records_function(
                  connection->get_connect_handle(), &rcd, table_name.c_str());
               if(rcd != Dll::dberr_success)
                  throw CsiDbException(rcd);

               // we now need to retrieve the results
               uint4 buff_size(0);
               void const *buff(0);
               Csi::OStrAscStream temp;

               rcd = Connection::query_busy;
               while(rcd == Connection::query_busy)
               {
                  buff = get_query_results_function(num_records_handle, 4, &rcd, &buff_size);
                  if(rcd == Connection::query_busy ||
                     rcd == Connection::query_finished ||
                     rcd == Connection::query_finished_no_records)
                  {
                     int const *int_buff(reinterpret_cast<int const *>(buff));
                     if (*int_buff != 0)
                        num_records = *int_buff;
                  }
               }
               close_query_results_function(num_records_handle);
            }
            catch(CsiDbException &)
            { 
               num_records = 0;
            }
            if(num_records_handle != 0)
               close_query_results_function(num_records_handle);

         } // get_num_records_in_table

#endif

         void QueryCommandBase::execute(Dll *dll)
         {
            uint4 query_handle = 0;
            try
            {
               // we need to check to ensure that the connection is valid
               if(is_connected_function != 0 &&
                  !is_connected_function(connection->get_connect_handle()))
                  throw CsiDbException(Dll::dberr_not_connected);

               // we can now concern ourselves with getting the next data block from the query
               int rcd = Connection::query_busy;
               uint4 tob1_buff_size = 0;
               void const *tob1_buff;
               cache_type local_cache;
               uint4 records_sent = 0;
               
               if(!Csi::EventReceiver::is_valid_instance(client))
                  return;
               if(table_metadata == 0 && connection->get_properties().get_should_poll_meta())
                  get_column_meta();
               query_handle = start_query();
               while(rcd == Connection::query_busy)
               {
                  // if records have been previously posted, we need to wait until the dispatch
                  // thread has acknowledged that event by setting the condition.
                  bool ready = (records_sent == 0);
                  while(!ready && Csi::EventReceiver::is_valid_instance(client))
                     ready = condition.wait(1000);
                  if(ready)
                  {
                     // we need to copy any records that have been "returned" to the local cache.
                     cache_protector.lock();
                     if(!cache.empty())
                        local_cache.insert(local_cache.end(), cache.begin(), cache.end());
                     cache.clear();
                     cache_protector.unlock();
                     
                     // we can now get the next set of query results
                     tob1_buff = CsiDbHelpers::get_query_results_function(
                        query_handle, 1024, &rcd, &tob1_buff_size);
                     if(rcd == Connection::query_busy ||
                        rcd == Connection::query_finished ||
                        rcd == Connection::query_finished_no_records)
                     {
                        char const *buff = reinterpret_cast<char const *>(tob1_buff);
                        uint4 current_pos = 0;
                        uint4 record_len = 0;
                        EventQueryRecords *event = EventQueryRecords::create(this);
                        
                        if(header == 0)
                        {
                           // we'll use this opportunity to report the new header to the application
                           value_type record;
                           header.bind(new header_type(tob1_buff, tob1_buff_size));
                           if(table_metadata != 0)
                              apply_column_meta();
                           record = header->make_record();
                           local_cache.push_back(record);
                           EventQueryHeader::cpost(this, record);
                        }
                        current_pos = header->get_header_end();
                        record_len = header->get_record_len();
                        while(current_pos + record_len <= tob1_buff_size)
                        {
                           // generate a record
                           value_type record;
                           if(!local_cache.empty())
                           {
                              record = local_cache.front();
                              local_cache.pop_front();
                           }
                           else
                           {
                              // there may be a chance that records have been returned while this
                              // thread was working.  We'll check the queue here to see.
                              cache_protector.lock();
                              if(!cache.empty())
                              {
                                 local_cache.insert(local_cache.end(), cache.begin(), cache.end());
                                 cache.clear();
                              }
                              cache_protector.unlock();
                              if(!local_cache.empty())
                              {
                                 record = local_cache.front();
                                 local_cache.pop_front();
                              }
                              else
                                 record = header->make_record();
                           }

                           // read the record and add it to the results if it matches
                           header->read_record(record, buff + current_pos, tob1_buff_size);
                           if(include_record(record))
                              event->records.push_back(record);
                           else
                              local_cache.push_front(record);
                           current_pos += record_len;
                        }
                        if(Csi::EventReceiver::is_valid_instance(client) && !event->records.empty())
                        {
                           records_sent += (uint4)event->records.size();
                           event->post();
                        }
                        else
                        {
                           delete event;
                           rcd = Connection::query_finished;
                        }
                     }
                  }
                  else
                     rcd = Connection::query_finished;
               }
            }
            catch(CsiDbException &e)
            {
               error_code = e.get_error_code();
               if(get_last_error_function)
                  last_error = get_last_error_function();
            }
            if(query_handle != 0)
               CsiDbHelpers::close_query_results_function(query_handle);
         } // execute


         void QueryCommandBase::return_records(cache_type &records)
         {
            cache_protector.lock();
            cache.insert(cache.end(), records.begin(), records.end());
            cache_protector.unlock(); 
         } // return_records


         void QueryCommandBase::get_cache(cache_type &records)
         {
            cache_protector.lock();
            std::copy(cache.begin(), cache.end(), std::back_inserter(records));
            cache_protector.unlock();
         } // get_cache


         void QueryCommandBase::get_column_meta()
         {
            uint4 meta_handle(0);
            try
            {
               // we need to launch the query
               int rcd(0);
               meta_handle = get_meta_data_function(
                  connection->get_connect_handle(), &rcd, table_name.c_str());
               if(rcd != Dll::dberr_success)
                  throw CsiDbException(rcd);
               
               // we now need to retrieve the results
               uint4 tob1_buff_size(0);
               void const *tob1_buff(0);
               Csi::SharedPtr<Cora::Broker::Tob1Header> meta_header;
               Csi::SharedPtr<Cora::Broker::Record> meta_record;
               size_t db_column_name_pos(0xffffffff);
               size_t process_pos(0xffffffff);
               size_t units_pos(0xffffffff);
               Csi::OStrAscStream temp;

               table_metadata.bind(new TableMetadata);
               rcd = Connection::query_busy;
               while(rcd == Connection::query_busy)
               {
                  tob1_buff = get_query_results_function(meta_handle, 1024, &rcd, &tob1_buff_size);
                  if(rcd == Connection::query_busy ||
                     rcd == Connection::query_finished ||
                     rcd == Connection::query_finished_no_records)
                  {
                     char const *buff(reinterpret_cast<char const *>(tob1_buff));
                     uint4 current_pos(0);
                     uint4 record_len(0);
                     
                     if(meta_header == 0)
                     {
                        meta_header.bind(new Cora::Broker::Tob1Header(tob1_buff, tob1_buff_size));
                        meta_record = meta_header->make_record();
                        for(Broker::Record::iterator vi = meta_record->begin(); vi != meta_record->end(); ++vi)
                        {
                           Broker::Record::value_type &value(*vi);
                           if(value->get_name() == L"dbColumnName")
                              db_column_name_pos = std::distance(meta_record->begin(), vi);
                           else if(value->get_name() == L"units")
                              units_pos = std::distance(meta_record->begin(), vi);
                           else if(value->get_name() == L"process")
                              process_pos = std::distance(meta_record->begin(), vi);
                        }
                     }
                     current_pos = meta_header->get_header_end();
                     record_len = meta_header->get_record_len();
                     while(current_pos + record_len <= tob1_buff_size)
                     {
                        ColumnMetadata column;
                        meta_header->read_record(meta_record, buff + current_pos, tob1_buff_size);
                        temp.str("");
                        meta_record->at(db_column_name_pos)->format(temp);
                        column.db_column_name = temp.str();
                        temp.str("");
                        meta_record->at(units_pos)->format(temp);
                        column.units = temp.str();
                        temp.str("");
                        meta_record->at(process_pos)->format(temp);
                        table_metadata->push_back(column);
                        current_pos += record_len;
                     }
                  }
               }
               close_query_results_function(meta_handle);

               // we need to get the loggernet station and column name for the table as well.  If
               // using an older DLL that dos not support the function, we will skip it.
               if(get_station_table_function != 0)
               {
                  uint4 tob1_buff_size(0);
                  void const *tob1_buff(0);
                  
                  meta_handle = get_station_table_function(connection->get_connect_handle(), &rcd, table_name.c_str());
                  if(rcd != Dll::dberr_success)
                     throw CsiDbException(rcd);
                  tob1_buff = get_query_results_function(meta_handle, 1, &rcd, &tob1_buff_size);
                  if(rcd == Connection::query_finished ||
                     rcd == Connection::query_busy ||
                     rcd == Connection::query_finished_no_records)
                  {
                     char const *buff(reinterpret_cast<char const *>(tob1_buff));
                     uint4 current_pos, record_len;
                     
                     meta_header.bind(new Cora::Broker::Tob1Header(tob1_buff, tob1_buff_size));
                     meta_record = meta_header->make_record();
                     current_pos = meta_header->get_header_end();
                     record_len = meta_header->get_record_len();
                     if(current_pos + record_len <= tob1_buff_size)
                     {
                        meta_header->read_record(meta_record, buff + current_pos, tob1_buff_size);
                        for(Broker::Record::iterator vi = meta_record->begin(); vi != meta_record->end(); ++vi)
                        {
                           Broker::Record::value_type &value(*vi);
                           temp.str("");
                           value->format(temp);
                           if(value->get_name() == L"lnStationName")
                              table_metadata->station_name = temp.str();
                           else if(value->get_name() == L"lnTableName")
                              table_metadata->table_name = temp.str();
                        }
                     }
                  }
               }
            }
            catch(CsiDbException &)
            { }
            if(meta_handle != 0)
               close_query_results_function(meta_handle);
         } // get_column_meta


         void QueryCommandBase::apply_column_meta()
         {
            if(table_metadata != 0)
            {
               header->set_station_name(table_metadata->station_name);
               header->set_table_name(table_metadata->table_name);
               for(header_type::iterator fi = header->begin(); fi != header->end(); ++fi)
               {
                  for(TableMetadata::const_iterator ci = table_metadata->begin(); ci != table_metadata->end(); ++ci)
                  {
                     if(fi->get_field_name() == ci->db_column_name)
                     {
                        fi->set_units(ci->units);
                        fi->set_process(ci->process);
                        break;
                     }
                  }
               }
            }
         } // apply_column_meta

         ////////////////////////////////////////////////////////////
         // class EventQueryHeader definitions
         ////////////////////////////////////////////////////////////
         uint4 const EventQueryHeader::event_id(
            Csi::Event::registerType("Cora::DataSources::CsiDbHelpers::EventQueryHeader"));
         

         ////////////////////////////////////////////////////////////
         // class EventQueryRecords definitions
         ////////////////////////////////////////////////////////////
         uint4 const EventQueryRecords::event_id(
            Csi::Event::registerType("Cora::DataSources::CsiDbHelpers::EventQueryRecords"));
         

         ////////////////////////////////////////////////////////////
         // class GetDataFromTimeCommand definitions
         ////////////////////////////////////////////////////////////
         uint4 const GetDataFromTimeCommand::command_id(
            Csi::Event::registerType("Cora::DataSources::CsiDbHelpers::GetDataFromTime"));


         uint4 GetDataFromTimeCommand::start_query()
         {
            int rcd;
            uint4 rtn = 0;
            Csi::OStrAscStream temp;

            start_time = connection->get_properties().round_timestamp(start_time);
            temp << table_name << ": Time query starting at " << start_time;
            trace(temp.str().c_str());
            rtn = CsiDbHelpers::get_data_from_time_function(
               connection->get_connect_handle(),
               &rcd,
               table_name.c_str(),
               columns.c_str(),
               start_time.get_nanoSec(),
               100100); //std::numeric_limits<int>::max());
            if(rcd != Dll::dberr_success)
               throw CsiDbException(rcd);
            return rtn;
         } // start_query


         ////////////////////////////////////////////////////////////
         // class GetDataLastCommand definitions
         ////////////////////////////////////////////////////////////
         uint4 const GetDataLastCommand::command_id(
            Csi::Event::registerType("Cora::DataSources::CsiDbHelpers::GetDataLastCommand"));


         uint4 GetDataLastCommand::start_query()
         {
            int rcd(-1);
            uint4 rtn = CsiDbHelpers::get_data_last_function(
               connection->get_connect_handle(),
               &rcd,
               table_name.c_str(),
               columns.c_str(),
               count);
            if(rcd != Dll::dberr_success)
               throw CsiDbException(rcd);
            return rtn;
         } // start_query

#ifdef __getNumRecords
         ////////////////////////////////////////////////////////////
         // class GetNumRecordsTimeRangeCommand definitions
         ////////////////////////////////////////////////////////////
         uint4 const GetNumRecordsTimeRangeCommand::command_id(
            Csi::Event::registerType("Cora::DataSources::CsiDbHelpers::GetNumRecordsTimeRangeCommand"));


         uint4 GetNumRecordsTimeRangeCommand::start_query()
         {
            int rcd;
            Csi::OStrAscStream msg;
            uint4 rtn = 0;

            begin_time = connection->get_properties().round_timestamp(begin_time);
            end_time = connection->get_properties().round_timestamp(end_time);
            msg << "Querying Num Records " << table_name << " from " << begin_time << " to " << end_time;
            trace(msg.str().c_str());
            rtn = CsiDbHelpers::get_num_records_between_time_function(
               connection->get_connect_handle(),
               &rcd,
               table_name.c_str(),
               begin_time.get_nanoSec(),
               end_time.get_nanoSec());
            if(rcd != Dll::dberr_success)
               throw CsiDbException(rcd);
            return rtn;
         } // start_query
#endif


         ////////////////////////////////////////////////////////////
         // class GetDataTimeRangeCommand definitions
         ////////////////////////////////////////////////////////////
         uint4 const GetDataTimeRangeCommand::command_id(
            Csi::Event::registerType("Cora::DataSources::CsiDbHelpers::GetDataTimeRangeCommand"));


         uint4 GetDataTimeRangeCommand::start_query()
         {
            int rcd;
            Csi::OStrAscStream msg;
            uint4 rtn = 0;
            
            begin_time = connection->get_properties().round_timestamp(begin_time);
            end_time = connection->get_properties().round_timestamp(end_time);
            msg << "Querying " << table_name << " from " << begin_time << " to " << end_time;
            trace(msg.str().c_str());
            rtn = CsiDbHelpers::get_data_time_range_function(
               connection->get_connect_handle(),
               &rcd,
               table_name.c_str(),
               columns.c_str(),
               begin_time.get_nanoSec(),
               end_time.get_nanoSec());
            if(rcd != Dll::dberr_success)
               throw CsiDbException(rcd);
            return rtn;
         } // start_query


         ////////////////////////////////////////////////////////////
         // class   PollNewDataCommand definitions
         ////////////////////////////////////////////////////////////
         uint4 const PollNewDataCommand::command_id(
            Csi::Event::registerType("Cora::DataSources::CsiDbHelpers::PollNewDataCommand"));


         uint4 PollNewDataCommand::start_query()
         {
            // we can now launch the query
            int rcd = 0;
            uint4 rtn = get_data_time_record_function(
               connection->get_connect_handle(),
               &rcd,
               table_name.c_str(),
               columns.c_str(),
               begin_time.get_nanoSec(),
               begin_record_no,
               100100, /*std::numeric_limits<int>::max()*/
               false);
            if(rcd != Dll::dberr_success)
               throw CsiDbException(rcd);
            return rtn;
         } // start_query


         ////////////////////////////////////////////////////////////
         // class ListDataSourcesCommand definitions
         ////////////////////////////////////////////////////////////
         uint4 const ListDataSourcesCommand::command_id(
            Csi::Event::registerType("Cora::DataSources::CsiDbHelpers::ListDataSourcesCommand"));


         void ListDataSourcesCommand::execute(Dll *dll)
         {
            try
            {
               dll->list_data_sources(sources, source_type);
               condition.set();
            }
            catch(std::exception &)
            {
               if(get_last_error_function)
                  last_error = get_last_error_function();
               condition.set();
               throw;
            }
         } // execute


         ////////////////////////////////////////////////////////////
         // class ListDatabasesCommand definitions
         ////////////////////////////////////////////////////////////
         uint4 const ListDatabasesCommand::command_id(
            Csi::Event::registerType("Cora::DataSources::CsiDbHelpers::ListDatabasesCommand"));


         void ListDatabasesCommand::execute(Dll *dll)
         {
            try
            {
               StrAsc const connect(properties.make_connect_str());
               dll->list_databases(connect.c_str(), databases);
               condition.set();
            }
            catch(std::exception &)
            {
               if(get_last_error_function)
                  last_error = get_last_error_function();
               condition.set();
               throw;
            }
         } // execute


         ////////////////////////////////////////////////////////////
         // class GetTableRangeCommand definitions
         ////////////////////////////////////////////////////////////
         uint4 const GetTableRangeCommand::command_id(
            Csi::Event::registerType("Cora::DataSources::CsiDbHelpers::GetTableRangeCommand"));


         void GetTableRangeCommand::execute(Dll *dll)
         {
            typedef GetTableRangeCompleteEvent event_type;
            uint4 query = 0;
            try
            {
               // we will make calls to the get_data_first() and get_data_last() functions in the
               // DLL.  We need to first ensure that these functions are valid.  We also need to
               // ensure that the connection is valid.
               if(get_data_first_function == 0 || get_data_last_function == 0)
                  throw CsiDbException(Dll::dberr_not_connected);
               if(is_connected_function != 0 && !is_connected_function(connection->get_connect_handle()))
                  throw CsiDbException(Dll::dberr_not_connected);

               // we can now launch the first query
               int rcd = 0;
               query = get_data_first_function(
                  connection->get_connect_handle(), &rcd, table_name.c_str(), "", 1);
               if(rcd != Dll::dberr_success)
                  throw CsiDbException(rcd);

               // we can now process the results that were returned.
               void const *tob1_buff;
               uint4 tob1_buff_len = 0;
               tob1_buff = get_query_results_function(query, 1, &rcd, &tob1_buff_len);
               if(rcd != Connection::query_finished && rcd != Connection::query_finished_no_records)
                  throw CsiDbException(Dll::dberr_unknown); 

               if(rcd == Connection::query_finished)
               {
                  // we need to create an object to parse the header that was returned
                  typedef Broker::Tob1Header header_type;
                  header_type header(tob1_buff, tob1_buff_len);
                  header_type::record_handle record(header.make_record());
                  char const *buff = reinterpret_cast<char const *>(tob1_buff);
                  Csi::LgrDate oldest_stamp;
                  uint4 header_len(header.get_header_end());
                  
                  if(header_len < tob1_buff_len)
                  {
                     header.read_record(record, buff + header_len, tob1_buff_len - header_len);
                     oldest_stamp = record->get_stamp();
                     close_query_results_function(query);
                     query = 0;
                     
                     // we now need to query for the newest record
                     query = get_data_last_function(
                        connection->get_connect_handle(), &rcd, table_name.c_str(), "", 1);
                     if(rcd != Dll::dberr_success)
                        throw CsiDbException(rcd);
                     tob1_buff = get_query_results_function(query, 1, &rcd, &tob1_buff_len);
                     if(rcd != Connection::query_finished)
                        throw CsiDbException(Dll::dberr_unknown);
                     buff = reinterpret_cast<char const *>(tob1_buff);
                     header.read_record(record, buff + header_len, tob1_buff_len - header_len);
                     close_query_results_function(query);
                     query = 0;
                     
                     // we can now report the results to the final client
                     Csi::LgrDate newest_stamp(record->get_stamp());
                     event_type::cpost(
                        final_client, uri, event_type::outcome_success, oldest_stamp, newest_stamp);
                  }
                  else
                     event_type::cpost(
                        final_client, uri, event_type::outcome_no_records);
               }
               else
               {
                  event_type::cpost(
                     final_client, uri, event_type::outcome_no_records);
               }
            }
            catch(CsiDbException &e)
            {
               event_type::outcome_type outcome(event_type::outcome_unknown);
               if(get_last_error_function != 0)
                  last_error = get_last_error_function();
               switch(e.get_error_code())
               {
               case Dll::dberr_not_connected:
                  outcome = event_type::outcome_not_connected;
                  break;

               case Dll::dberr_query_fail:
                  outcome = event_type::outcome_no_table;
                  break;
               }
               event_type::cpost(final_client, uri, outcome);
            }
            catch(std::exception &)
            { event_type::cpost(final_client, uri, event_type::outcome_unknown); }
            if(query != 0)
               close_query_results_function(query);
         } // execute
      }; 
   };
};



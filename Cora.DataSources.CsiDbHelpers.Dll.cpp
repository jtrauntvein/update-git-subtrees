/* Cora.DataSources.CsiDbHelpers.Dll.cpp

   Copyright (C) 2009, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 26 January 2009
   Last Change: Tuesday 05 March 2019
   Last Commit: $Date: 2020-07-14 15:41:32 -0600 (Tue, 14 Jul 2020) $
   Last Changed by: $Author: jbritt $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.DataSources.CsiDbHelpers.Dll.h"
#include "Cora.Broker.Tob1Header.h"
#include "Csi.OsException.h"
#include "Csi.CsvRec.h"
#include "Csi.BuffStream.h"
#include "Csi.Utils.h"
#include "CsiTypes.h"
#include <algorithm>
#include <limits>
#include <fstream>
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
         connect_function_type *connect_function;
         disconnect_function_type *disconnect_function;
         get_system_data_sources_function_type *get_system_data_sources_function;
         get_system_data_sources_type_function_type *get_system_data_sources_type_function;
         get_tables_function_type *get_tables_function;
         get_columns_function_type *get_columns_function;
         get_meta_data_function_type *get_meta_data_function;
#ifdef __getNumRecords
         get_num_records_function_type *get_num_records_function;
         get_num_records_between_time_function_type *get_num_records_between_time_function;
#endif
         get_data_first_function_type *get_data_first_function;
         get_data_last_function_type *get_data_last_function;
         get_data_time_range_function_type *get_data_time_range_function;
         get_data_from_time_function_type *get_data_from_time_function;
         get_data_time_record_function_type *get_data_time_record_function;
         get_query_results_function_type *get_query_results_function;
         close_query_results_function_type *close_query_results_function;
         get_initial_catalog_function_type *get_initial_catalog_function;
         is_connected_function_type *is_connected_function;
         get_last_error_function_type *get_last_error_function;
         get_last_conn_error_function_type *get_last_conn_error_function;
         get_station_table_function_type *get_station_table_function;
         backup_database_function_type *backup_database_function;
         restore_database_function_type *restore_database_function;
         
      
         ////////////////////////////////////////////////////////////
         // class Dll definitions
         ////////////////////////////////////////////////////////////
         Dll *Dll::the_loader = 0;

      
         Dll::Dll():
            module_handle(0)
         {
            if(the_loader == 0)
            {
               // format the path to the DLL
               StrAsc dll_path;
               Csi::get_app_dir(dll_path);
               if(dll_path.last() != '\\')
                  dll_path.append('\\');
               dll_path.append("csi_dbc.dll");
               module_handle = ::LoadLibraryA(dll_path.c_str());
               if(module_handle == 0)
                  throw Csi::OsException("load library failed");

               // now that the library is loaded, we need to resolve the DLL entry points.
               using namespace CsiDbHelpers;
               try
               {
                  connect_function = reinterpret_cast<connect_function_type *>(
                     ::GetProcAddress(module_handle, "Connect"));
                  if(connect_function == 0)
                     throw Csi::OsException("failed to locate Connect() entry point");
                  disconnect_function = reinterpret_cast<disconnect_function_type *>(
                     ::GetProcAddress(module_handle, "Disconnect"));
                  if(disconnect_function == 0)
                     throw Csi::OsException("failed to locate Disconnect() entry point");
                  get_system_data_sources_function = reinterpret_cast<get_system_data_sources_function_type *>(
                     ::GetProcAddress(module_handle, "GetSystemDataSources"));
                  get_system_data_sources_type_function = reinterpret_cast<get_system_data_sources_type_function_type *>(
                     ::GetProcAddress(module_handle, "GetSystemDataSourcesByType"));
                  if(get_system_data_sources_function == 0)
                     throw Csi::OsException("failed to locate GetSystemDataSources() point");
                  get_tables_function = reinterpret_cast<get_tables_function_type *>(
                     ::GetProcAddress(module_handle, "GetTables"));
                  if(get_tables_function == 0)
                     throw Csi::OsException("failed to locate the GetTables() entry point");
                  get_columns_function = reinterpret_cast<get_columns_function_type *>(
                     ::GetProcAddress(module_handle, "GetColumns"));
                  if(get_columns_function == 0)
                     throw Csi::OsException("failed to locate the GetColumns entry point");
                  get_data_first_function = reinterpret_cast<get_data_first_function_type *>(
                     ::GetProcAddress(module_handle, "GetDataFirst"));
                  get_data_last_function = reinterpret_cast<get_data_last_function_type *>(
                     ::GetProcAddress(module_handle, "GetDataLast"));
                  get_meta_data_function = reinterpret_cast<get_meta_data_function_type *>(
                     ::GetProcAddress(module_handle, "GetMetaData")); 
#ifdef __getNumRecords
                  get_num_records_function = reinterpret_cast<get_num_records_function_type *>(
                     ::GetProcAddress(module_handle, "GetNumRecordsInTable")); 
                  get_num_records_between_time_function = reinterpret_cast<get_num_records_between_time_function_type *>(
                     ::GetProcAddress(module_handle, "GetNumRecordsBetweenTime")); 
#endif
                  get_data_time_range_function = reinterpret_cast<get_data_time_range_function_type *>(
                     ::GetProcAddress(module_handle, "GetDataBetweenTime"));
                  get_data_from_time_function = reinterpret_cast<get_data_from_time_function_type *>(
                     ::GetProcAddress(module_handle, "GetDataFromTime"));
                  get_query_results_function = reinterpret_cast<get_query_results_function_type *>(
                     ::GetProcAddress(module_handle, "GetQueryResults"));
                  get_data_time_record_function = reinterpret_cast<get_data_time_record_function_type *>(
                     ::GetProcAddress(module_handle, "GetDataTimeRecordNum"));
                  if(get_query_results_function == 0)
                     throw Csi::OsException("failed to locate the GetQueryResults entry point");
                  close_query_results_function = reinterpret_cast<close_query_results_function_type *>(
                     ::GetProcAddress(module_handle, "CloseQueryResults"));
                  if(close_query_results_function == 0)
                     throw Csi::OsException("failed to locate the CloseQueryResults entry point");
                  get_initial_catalog_function = reinterpret_cast<get_initial_catalog_function_type *>(
                     ::GetProcAddress(module_handle, "GetInitialCatalog"));
                  if(get_initial_catalog_function == 0)
                     throw Csi::OsException("failed to locate the GetInitialCatalog entry point");
                  is_connected_function = reinterpret_cast<is_connected_function_type *>(
                     ::GetProcAddress(module_handle, "IsStillConnected"));
                  get_last_conn_error_function = reinterpret_cast<get_last_conn_error_function_type *>(
                     ::GetProcAddress(module_handle, "GetLastErrorMsgForConn"));
                  get_last_error_function = reinterpret_cast<get_last_error_function_type *>(
                     ::GetProcAddress(module_handle, "GetLastErrorMsg"));
                  get_station_table_function = reinterpret_cast<get_station_table_function_type *>(
                     ::GetProcAddress(module_handle, "GetStationTableFromDBTablename"));
                  backup_database_function = reinterpret_cast<backup_database_function_type *>(
                     ::GetProcAddress(module_handle, "BackupDatabase"));
                  restore_database_function = reinterpret_cast<restore_database_function_type *>(
                     ::GetProcAddress(module_handle, "RestoreDatabase"));
                  the_loader = this;
               }
               catch(std::exception &)
               {
                  ::FreeLibrary(module_handle);
                  connect_function = 0;
                  disconnect_function = 0;
                  get_system_data_sources_function = 0;
                  get_tables_function = 0;
                  get_columns_function = 0;
                  get_meta_data_function = 0;
#ifdef __getNumRecords
                  get_num_records_function = 0;
                  get_num_records_between_time_function = 0;
#endif
                  get_data_time_range_function = 0;
                  get_data_from_time_function = 0;
                  get_query_results_function = 0;
                  get_data_first_function = 0;
                  get_data_last_function = 0;
                  close_query_results_function = 0;
                  get_initial_catalog_function = 0;
                  get_last_conn_error_function = 0;
                  get_last_error_function = 0;
                  get_station_table_function = 0;
                  module_handle = 0;
                  the_loader = 0;
                  throw;
               }
            }
            else
               throw Csi::MsgExcept("Dll should be a singleton");
         } // constructor


         Dll::~Dll()
         {
            // we need to make sure that all of the connections are cancelled
            while(!connections.empty())
            {
               connections.front()->disconnect();
               connections.pop_front();
            }
            if(module_handle != 0)
            {
               using namespace CsiDbHelpers;
               connect_function = 0;
               disconnect_function = 0;
               get_system_data_sources_function = 0;
               get_system_data_sources_type_function = 0;
               get_tables_function = 0;
               get_columns_function = 0;
               get_meta_data_function = 0;
#ifdef __getNumRecords
               get_num_records_function = 0;
               get_num_records_between_time_function = 0;
#endif
               get_data_time_range_function = 0;
               get_data_from_time_function = 0;
               get_query_results_function = 0;
               close_query_results_function = 0;
               get_initial_catalog_function = 0;
               get_data_first_function = 0;
               get_data_last_function = 0;
               get_last_conn_error_function = 0;
               get_last_error_function = 0;
               get_station_table_function = 0;
               backup_database_function = 0;
               restore_database_function = 0;
               ::FreeLibrary(module_handle);
               module_handle = 0;
            }
            the_loader = 0;
         } // destructor


         Dll::connection_handle Dll::connect(DbProperties const &properties)
         {
            connection_handle rtn(new Connection(properties));
            connections.push_back(rtn);
            return rtn;
         } // connect


         void Dll::disconnect(Connection *connection)
         {
            connections_type::iterator ci = std::find_if(
               connections.begin(), connections.end(), Csi::HasSharedPtr<Connection>(connection));
            if(ci != connections.end())
            {
               connection_handle conn(*ci);
               connections.erase(ci);
               conn->disconnect();
            }
         } // disconnect


         void Dll::list_data_sources(source_names_type &source_names, int source_type)
         {
            // call the function to get the list of data sources
            int rcd = 0;
            uint4 tob_buffer_len = 0;
            void const *tob_buffer;
            if(source_type <= 0 || get_system_data_sources_type_function == 0)
               tob_buffer = get_system_data_sources_function(
                  &rcd, &tob_buffer_len);
            else
               tob_buffer = get_system_data_sources_type_function(
                  &rcd, &tob_buffer_len, source_type);
            if(rcd != 0)
               throw CsiDbException(rcd);

            // we now need to parse the TOB1 structure that was returned
            Broker::Tob1Header header(tob_buffer, tob_buffer_len);
            if(header.size() >= 2)
            {
               Broker::Tob1Field const &source_name_field = *header.begin();
               Broker::Tob1Field const &source_type_field = *(header.begin() + 1);
               char const *buff = reinterpret_cast<char const *>(tob_buffer);
               uint4 current_pos = header.get_header_end();
               StrAsc source_name;
               uint4 field_len = source_name_field.get_field_size();
               int source_type;
               
               source_names.clear();
               while(current_pos + header.get_record_len() <= tob_buffer_len)
               {
                  // read the source name
                  source_name.setContents(buff + current_pos, field_len);
                  source_name.cut(source_name.find('\0'));

                  // read the source type
                  memcpy(&source_type, buff + current_pos + field_len, sizeof(source_type));
                  source_names.push_back(source_name_type(source_name, source_type));
                  current_pos += header.get_record_len();
               }
            }
            else
               throw std::invalid_argument("not enough fields returned");
         } // list_data_sources


         void Dll::list_databases(char const *connect_str, databases_type &databases)
         {
            // in order to get the "initial catalog", we need to create a connection to the database
            // server identified in the connect string.  This connection will only last the lifetime
            // of this method
            uint4 connect_handle = 0;
            uint4 query_handle = 0;
            int rcd = connect_function(&connect_handle, connect_str);
            if(rcd != 0)
               throw CsiDbException(rcd);

            try
            {
               // we can now use this connection to query the server for the set of databases
               uint4 query_handle = get_initial_catalog_function(connect_handle, &rcd);
               if(rcd != 0)
                  throw CsiDbException(rcd);
               
               // we now need to retrieve the results
               rcd = Connection::query_busy;
               while(rcd == Connection::query_busy)
               {
                  uint4 tob1_buff_size = 0;
                  void const *tob1_buff = get_query_results_function(
                     query_handle, std::numeric_limits<int4>::max(), &rcd, &tob1_buff_size);
                  if(rcd == Connection::query_busy || rcd == Connection::query_finished)
                  {
                     Broker::Tob1Header header(tob1_buff, tob1_buff_size);
                     Broker::Tob1Field const &field = header.front();
                     char const *buff = reinterpret_cast<char const *>(tob1_buff);
                     uint4 current_pos = header.get_header_end();
                     uint4 record_len = header.get_record_len();
                     StrAsc database_name;
                     
                     while(current_pos + record_len <= tob1_buff_size)
                     {
                        database_name.setContents(buff + current_pos, record_len);
                        database_name.cut(database_name.find('\0'));
                        databases.push_back(database_name);
                        current_pos += record_len;
                     }
                  } 
               }
               close_query_results_function(query_handle);
            }
            catch(std::exception &)
            {
               if(query_handle != 0)
                  close_query_results_function(query_handle);
               if(connect_handle != 0)
                  disconnect_function(connect_handle);
               throw;
            }
            
            // we need to release the connection
            disconnect_function(connect_handle);
         } // list_databases


         ////////////////////////////////////////////////////////////
         // class CsiDbException definitions
         ////////////////////////////////////////////////////////////
         char const *CsiDbException::what() const throw ()
         {
            switch(error_code)
            {
            case Dll::dberr_connected:
               return "database is connected";
            
            case Dll::dberr_not_connected:
               return "database is not connected";
            
            case Dll::dberr_query_fail:
               return "database query failed";
            
            case Dll::dberr_missing_handle:
               return "invalid database connection handle";
            
            default:
               return "unrecognised error";
            }
         } // what

      
         ////////////////////////////////////////////////////////////
         // class Connection definitions
         ////////////////////////////////////////////////////////////
         Connection::Connection(DbProperties const &properties_):
            properties(properties_),
            connect_handle(0)
         {
            if(Dll::the_loader != 0)
            {
               StrAsc const source_name(properties.make_connect_str());
               int rcd = CsiDbHelpers::connect_function(&connect_handle, source_name.c_str());
               if(rcd != 0)
                  throw CsiDbException(rcd);
            }
            else
               throw std::invalid_argument("CSIDB.DLL not loaded");
         } // constructor


         void Connection::disconnect()
         {
            if(connect_handle != 0)
            {
               if(Dll::the_loader != 0 && connect_handle != 0)
                  CsiDbHelpers::disconnect_function(connect_handle);
               connect_handle = 0;
            }
         } // disconnect


         void Connection::list_tables(table_names_type &table_names)
         {
            // we need to start the query to get the list of table names from the source
            uint4 query_handle;
            int rcd;
         
            if(Dll::the_loader == 0)
               throw std::invalid_argument("CSIDB.DLL not loaded");
            query_handle = CsiDbHelpers::get_tables_function(connect_handle, &rcd);
            if(rcd != 0)
               throw CsiDbException(rcd);

            // we now can use the get query results function to retrieve the data
            rcd = query_busy;
            while(rcd == query_busy)
            {
               uint4 tob1_buff_size = 0; 
               void const *tob1_buff = CsiDbHelpers::get_query_results_function(
                  query_handle, std::numeric_limits<int4>::max(), &rcd, &tob1_buff_size);
               if(rcd == query_busy || rcd == query_finished)
               {
                  Broker::Tob1Header header(tob1_buff, tob1_buff_size);
                  Broker::Tob1Field const &field = header.front();
                  char const *buff = reinterpret_cast<char const *>(tob1_buff);
                  uint4 current_pos = header.get_header_end();
                  uint4 record_len = field.get_field_size();
                  StrAsc table_name;
               
                  while(current_pos + record_len <= tob1_buff_size)
                  {
                     table_name.setContents(buff + current_pos, record_len);
                     table_name.cut(table_name.find('\0'));
                     table_names.push_back(table_name);
                     current_pos += record_len;
                  }
               }
            }
            CsiDbHelpers::close_query_results_function(query_handle);
         } // list_tables


         void Connection::list_columns(column_names_type &column_names, StrAsc const &table_name)
         {
            // we need to start the query to get the list of columns names from the source for the
            // specified table
            uint4 query_handle;
            int rcd;
         
            if(Dll::the_loader == 0)
               throw std::invalid_argument("CSIDB.DLL not loaded");
            query_handle = CsiDbHelpers::get_columns_function(connect_handle, &rcd, table_name.c_str());
            if(rcd != 0)
               throw CsiDbException(rcd);

            // we now can use the get query results function to retrieve the data
            rcd = query_busy;
            while(rcd == query_busy)
            {
               uint4 tob1_buff_size = 0; 
               void const *tob1_buff = CsiDbHelpers::get_query_results_function(
                  query_handle, std::numeric_limits<int4>::max(), &rcd, &tob1_buff_size);
               if(rcd == query_busy || rcd == query_finished)
               {
                  Broker::Tob1Header header(tob1_buff, tob1_buff_size);
                  Broker::Tob1Field const &field = header.front();
                  char const *buff = reinterpret_cast<char const *>(tob1_buff);
                  uint4 current_pos = header.get_header_end();
                  uint4 record_len = header.get_record_len();
                  StrAsc column_name;
               
                  while(current_pos + record_len <= tob1_buff_size)
                  {
                     column_name.setContents(buff + current_pos, record_len);
                     column_name.cut(column_name.find('\0'));
                     if(column_name != "TMSTAMP" && column_name != "RECNUM" && column_name != "SUBSECOND")
                        column_names.push_back(column_name);
                     current_pos += record_len;
                  }
               }
            }
            CsiDbHelpers::close_query_results_function(query_handle);
         } // list_columns
      };
   };
};

/* Cora.DataSources.CsiDbHelpersDll.h

   Copyright (C) 2009, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 26 January 2009
   Last Change: Tuesday 05 March 2019
   Last Commit: $Date: 2020-07-14 15:41:32 -0600 (Tue, 14 Jul 2020) $
   Last Changed by: $Author: jbritt $

*/

#pragma once
#ifndef Cora_DataSources_CsiDbHelpers_Dll_h
#define Cora_DataSources_CsiDbHelpers_Dll_h

#include "Cora.DataSources.CsiDbHelpers.DbProperties.h"
#include "Csi.OsException.h"
#include "Csi.SharedPtr.h"
#include <list>


namespace Cora
{
   namespace DataSources
   {
      namespace CsiDbHelpers
      {
         // @group: class forward declarations
         class Connection;
         // @endgroup
      

         /**
          * Defines an object that can dynamically load the CSI database access DLL and expose its
          * functions as methods.
          */
         class Dll
         {
         public:
            /**
             * Constructor
             */
            Dll();

            /**
             * Destructor
             */
            virtual ~Dll();

            /**
             * Initiates a connection to the database.
             *
             * @param properties Specifies the properties for the connection.
             *
             * @return Returns a handle to an object that represents and maintains the connection.
             */
            typedef Csi::SharedPtr<Connection> connection_handle;
            connection_handle connect(DbProperties const &properties);

            /**
             * Releases any connection to the database that was added through a call to connect().
             */
            void disconnect(Connection *connection);

            /**
             * Generates a list of all data sources on the host machine.
             *
             * @param source_names Specifies a container in which the data source names will be
             * returned.
             */
            typedef std::pair<StrAsc, int> source_name_type;
            typedef std::list<source_name_type> source_names_type;
            void list_data_sources(source_names_type &source_names, int source_type = -1);

            /**
             * Generates a list of all databases associated with the specified connection string.
             *
             * @param connect_str Specifies the string that dictates the database connection.
             *
             * @param databases Specifies a container of strings in which the databases will be
             * listed.
             */
            typedef std::list<StrAsc> databases_type;
            void list_databases(char const *connect_str, databases_type &databases);

            /**
             * Defines the possible DLL error codes.
             */
            enum db_error_codes
            {
               dberr_dll_load_failed = -1,
               dberr_success = 0,
               dberr_connected = 1,
               dberr_not_connected = 2,
               dberr_query_fail = 3,
               dberr_missing_handle = 4,
               dberr_unknown = 5
            };

            /**
             * References to the singleton instance of this class.
             */
            static Dll *the_loader;

         private:
            /**
             * Specifies the module handle for the CSI_DBC.dll.
             */
            HMODULE module_handle;

            /**
             * Specifies the list of current connections.
             */
            typedef std::list<connection_handle> connections_type;
            connections_type connections;
         };


         /**
          * Defines a class for an exception that will get thrown by this layer.
          */
         class CsiDbException: public std::exception
         {
         public:
            /**
             * Constructor
             *
             * @param error_code_ Specifies the error code.
             */
            CsiDbException(int error_code_):
               error_code(error_code_)
            { }

            /**
             * @return Overloads the base class method to format this exception.
             */
            virtual char const *what() const throw ();

            /**
             * @return Returns the error code for this exception.
             */
            int get_error_code() const
            { return error_code; }

         private:
            /**
             * Specifies the error code for this exception.
             */
            int error_code;
         };
         

         /**
          * Defines a class that encapsulates the functionality of a database connection maintained
          * through the CSI_DBC.dll.  The application can create objects of this class by calling
          * Dll::connect().
          */
         class Connection
         {
         public:
            /**
             * Destructor
             */
            virtual ~Connection()
            { disconnect(); }

            /**
             * Generates a list of tables stored in the database.
             *
             * @param table_names Specifies the container where the table names will be written.
             */
            typedef std::list<StrAsc> table_names_type;
            void list_tables(table_names_type &table_names);

            /**
             * Generates a list of column names associated with a given table name.
             *
             * @param column_names Specifies the list to which the column names will be written.
             *
             * @param table_name Specifies the table for which column names will be queried.
             */
            typedef std::list<StrAsc> column_names_type;
            void list_columns(column_names_type &column_names, StrAsc const &table_name);

            /**
             * Defines the status codes that reflect the outcomes of query attempts.
             */
            enum query_status_codes
            {
               query_executed = 0,
               query_no_id = 1,
               query_busy = 2,
               query_finished = 3,
               query_finished_no_records = 4
            };

            /**
             * @return Returns the CSI_DBC connection handle.
             */
            uint4 get_connect_handle() const
            { return connect_handle; }

            /**
             * @return Returns the database connection properties.
             */
            DbProperties const get_properties() const
            { return properties; }
            
         private:
            /**
             * Constructor
             *
             * Meant to be called only by the DLL.
             */
            Connection(DbProperties const &properties_);

            /**
             * Releases the connection.
             */
            void disconnect();

            /**
             * Specifies the DLL connection handle.
             */
            uint4 connect_handle;

            /**
             * Specifies the database connection properties.
             */
            DbProperties const properties;
            
            friend class Dll;
         };


         // @group: The following function pointers are initialised when the
         // DLL is loaded in the constructor.

         /**
          * Should be called by the application to initiate a connection to a database using either
          * an ODBC data source name or a path to an embedded SQL server database.  The list of
          * available data source names can be obtained by calling the
          * get_system_data_sources_function().
          *
          * @return Returns a code that indicates success or an error.
          *
          * @param connect_handle Specifies a pointer to a connection handle that will be written by
          * this function on success.
          *
          * @param system_dsn Specifies the database connection string.
          */
         typedef int (__stdcall connect_function_type)(
            uint4 *connect_handle,
            char const *system_dsn);
         extern connect_function_type *connect_function;

         /**
          * Called to release a connection that was created when create_function() was called.
          *
          * @return Returns a code that indicates success or failure.
          *
          * @param connect_handle Specifies the connection handle.
          */
         typedef int (__stdcall disconnect_function_type)(
            uint4 connect_handle);
         extern disconnect_function_type *disconnect_function;

         /**
          * @return Returns a TOB1 data structure that lists all of the available data source names
          * and paths.
          *
          * @param error_code Specifies a reference to an error code that will be written to report
          * the outcome of this function.
          *
          * @param tob1_data_len Specfies a pointer that will be written to reflect the length of
          * the return buffer.
          */
         typedef void const * (__stdcall get_system_data_sources_function_type)(
            int *error_code,
            uint4 *tob1_data_size);
         extern get_system_data_sources_function_type *get_system_data_sources_function;

         /**
          * Retrieves a list of available system data sources that match the specified source type.
          *
          * @return Returns a pointer to a TOB1 data structure that describes the data sources
          *
          * @param error_code Specifies a pointer to which the success or failure outcome will be
          * written.
          *
          * @param tob1_data_len Specifies a pointer to which the length of the returned TOB1 data
          * structure will be written.
          *
          * @param source_type Specifies the type of data sources that should be returned.
          */
         typedef void const * (__stdcall get_system_data_sources_type_function_type)(
            int *error_code,
            uint4 *tob1_data_len,
            uint4 source_type);
         extern get_system_data_sources_type_function_type *get_system_data_sources_type_function;

         /**
          * Initiates a query that will return the names of all of the tables that are available for
          * the specified connection.
          *
          * @return On success, returns a handle to the query object.
          *
          * @param connect_handle Specifies the database connection.
          *
          * @param error_code Specifies a pointer to which the success or failure outcome will be
          * written.
          */
         typedef uint4 (__stdcall get_tables_function_type)(
            uint4 connect_handle,
            int *error_code);
         extern get_tables_function_type *get_tables_function;

         /**
          * Initiates a query to return the list of columns for a specified table with the given
          * connection handle.
          *
          * @return On success, returns a handle to the query object.
          *
          * @param connect_handle Specifies a handle to the database connection.
          *
          * @param error_code Specifies a pointer to which the success or failure outcome will be
          * written.
          *
          * @param table_name Specifies the table for which the columns will be returned.
          */
         typedef uint4 (__stdcall get_columns_function_type)(
            uint4 connect_handle,
            int *error_code,
            char const *table_name);
         extern get_columns_function_type *get_columns_function;

#ifdef __getNumRecords
         /**
         * Initiates a query to return the number of records for the specified table name.
         *
         * @return On success, returns a handle to a query object.
         *
         * @param connect_handle Specifies a handle to a database connection.
         *
         * @param error_code Specifies a pointer to which the success or failure outcome will be
         * written.
         *
         * @param table_name Specifies the table for which meta-data should be returned.
         */
         typedef uint4 (__stdcall get_num_records_function_type)(
            uint4 connect_handle,
            int *error_code,
            char const *table_name);
         extern get_num_records_function_type *get_num_records_function;


         typedef uint4 (__stdcall get_num_records_between_time_function_type)(
            uint4 connect_handle,
            int *error_code,
            char const *table_name,
            int8 begin_time,
            int8 end_time);
         extern get_num_records_between_time_function_type *get_num_records_between_time_function;
#endif


         /**
          * Initiates a query to return the meta-data for the specified table name.
          *
          * @return On success, returns a handle to a query object.
          *
          * @param connect_handle Specifies a handle to a database connection.
          *
          * @param error_code Specifies a pointer to which the success or failure outcome will be
          * written.
          *
          * @param table_name Specifies the table for which meta-data should be returned.
          */
         typedef uint4 (__stdcall get_meta_data_function_type)(
            uint4 connect_handle,
            int *error_code,
            char const *table_name);
         extern get_meta_data_function_type *get_meta_data_function;


         /**
          * Initiates a query to return the station and table name for the specified database table
          * name.
          *
          * @return On success, returns a handle to a query object.
          *
          * @param connect_handle Specifies the database connection.
          *
          * @param error_code Specifies the error code.
          *
          * @param table_name Specifies the database table name.
          */
         typedef uint4 (__stdcall get_station_table_function_type)(
            uint4 connect_handle,
            int *error_code,
            char const *table_name);
         extern get_station_table_function_type *get_station_table_function;

         /**
          * Initiates a query to return the oldest x records from a database table.
          *
          * @return On success, returns a handle to the query object.
          *
          * @param connect_handle Specifies a handle to a database connection.
          *
          * @param error_code Specifies a pointer to which the success or failure outcome will be
          * written.
          *
          * @param table_name Specifies the name of the table to query.
          *
          * @param columns Specifies a comma-separated list of table columns to query.  If all
          * columns are to be returned, this should be specified with a value of "*".
          *
          * @param record_count Specifies the number of records to return.
          */
         typedef uint4 (__stdcall get_data_first_function_type)(
            uint4 connect_handle,
            int *error_code,
            char const *table_name,
            char const *columns,
            int record_count);
         extern get_data_first_function_type *get_data_first_function;

         /**
          * Initiates a query to return the newest x records from a database table.
          *
          * @return On success, returns a handle to the query object.
          *
          * @param connect_handle Specifies a handle to a database connection.
          *
          * @param error_code Specifies a pointer to which the success or failure outcome will be
          * written.
          *
          * @param table_name Specifies the name of the table to query.
          *
          * @param columns Specifies a comma-separated list of table columns to query.  If all
          * columns are to be returned, this should be specified with a value of "*".
          *
          * @param record_count Specifies the number of records to return.
          */
         typedef uint4 (__stdcall get_data_last_function_type)(
            uint4 connect_handle,
            int *error_code,
            char const *table_name,
            char const *columns,
            int record_count);
         extern get_data_last_function_type *get_data_last_function;

         /**
          * Initiates a query to return records within a given time range.
          *
          * @return On success, returns a handle to the query object.
          *
          * @param connect_handle Specifies a handle to a database connection.
          *
          * @param error_code Specifies a pointer to which the success or failure outcome will be
          * written.
          *
          * @param table_name Specifies the name of the table to query.
          *
          * @param columns Specifies a comma-separated list of table columns to query.  If all
          * columns are to be returned, this should be specified with a value of "*".
          *
          * @param begin_time Specifies the start time as nano-seconds elapsed since midnight, 1
          * January 1990.
          *
          * @param end_time Specifis the end time as nano-seconds elapsed since midnight, 1 January
          * 1990.
          */
         typedef uint4 (__stdcall get_data_time_range_function_type)(
            uint4 connect_handle,
            int *error_code,
            char const *table_name,
            char const *column_list,
            int8 begin_time,
            int8 end_time);
         extern get_data_time_range_function_type *get_data_time_range_function;

         /**
          * Initiates a query to return records from a given start time to the newest.
          *
          * @return On success, returns a handle to the query object.
          *
          * @param connect_handle Specifies a handle to a database connection.
          *
          * @param error_code Specifies a pointer to which the success or failure outcome will be
          * written.
          *
          * @param table_name Specifies the name of the table to query.
          *
          * @param columns Specifies a comma-separated list of table columns to query.  If all
          * columns are to be returned, this should be specified with a value of "*".
          *
          * @param begin_time Specifies the start time as nano-seconds elapsed since midnight, 1
          * January 1990.
          *
          * @param record_count Specifies the maximum number of records to return.
          */
         typedef uint4 (__stdcall get_data_from_time_function_type)(
            uint4 connect_handle,
            int *error_code,
            char const *table_name,
            char const *column_list,
            int8 begin_time,
            int record_count);
         extern get_data_from_time_function_type *get_data_from_time_function;

         /**
          * Initiates a query to return records starting at the specified record number and time
          * stamp to the newest.
          *
          * @return On success, returns a handle to the query object.
          *
          * @param connect_handle Specifies a handle to a database connection.
          *
          * @param error_code Specifies a pointer to which the success or failure outcome will be
          * written.
          *
          * @param table_name Specifies the name of the table to query.
          *
          * @param columns Specifies a comma-separated list of table columns to query.  If all
          * columns are to be returned, this should be specified with a value of "*".
          *
          * @param begin_time Specifies the start time as nano-seconds elapsed since midnight, 1
          * January 1990.
          *
          * @param begin_record_no Specifies the record number of the starting record number.
          *
          * @param record_count Specifies the maximum number of records to return.
          *
          * @param include_first_record Set to true if the first record should be included.
          */
         typedef uint4 (__stdcall get_data_time_record_function_type)(
            uint4 connect_handle,
            int *error_code,
            char const *table_name,
            char const *columns,
            int8 begin_time,
            uint4 begin_record_no,
            int record_count,
            bool include_first_record);
         extern get_data_time_record_function_type *get_data_time_record_function;

         /**
          * Called to return the data from a query that was started with the above functions.
          *
          * @return Returns the TOB1 formatted data.
          *
          * @param query_handle Specifies the query handle returned by one of the above functions.
          *
          * @param record_count Specifies the maximum number of records to return.
          *
          * @param query_status Specifies a pointer to which the status of the query will be
          * written.
          *
          * @param tob1_data_size Specifies the size of the TOB1 structure.
          */
         typedef void const * (__stdcall get_query_results_function_type)(
            uint4 query_handle,
            int record_count,
            int *query_status,
            uint4 *tob1_data_size);
         extern get_query_results_function_type *get_query_results_function;

         /**
          * Called to close query results that were opened by a call to one of the get_data... calls
          * defined above.
          *
          * @return Returns the success or failure code.
          *
          * @param query_handle Specifies the query handle that was returned above.
          */
         typedef int (__stdcall close_query_results_function_type)(
            uint4 query_handle);
         extern close_query_results_function_type *close_query_results_function;

         /**
          * Called to obtain the initial catalog.
          */
         typedef int (__stdcall get_initial_catalog_function_type)(
            uint4 connect_handle,
            int *error_code);
         extern get_initial_catalog_function_type *get_initial_catalog_function;

         /**
          * Called to determine whether the specified connection handle is still valid.
          *
          * @return Returns success or failure.
          *
          * @param connect_handle Specifies the handle to the connection to test.
          */
         typedef int (__stdcall is_connected_function_type)(
            uint4 connect_handle);
         extern is_connected_function_type *is_connected_function;

         /**
          * @return Returns a nul terminated string that describes the last DLL error.
          */
         typedef char * (__stdcall get_last_error_function_type)();
         extern get_last_error_function_type *get_last_error_function;

         /**
          * @return Returns a nul terminated string that describes the last error encountered for
          * the specified connection.
          *
          * @param connect_handle Specifies the connection handle.
          */
         typedef char * (__stdcall get_last_conn_error_function_type)(
            uint4 connect_handle);
         extern get_last_conn_error_function_type *get_last_conn_error_function;

         /**
          * @return Returns an error code indicating success or failure.
          *
          * @param connect_handle Specifies the database connection handle.
          *
          * @param file_path Specifies the nul-terminated path for the bacjup file to be generated.
          */
         typedef int (__stdcall backup_database_function_type) (
            uint4 connect_handle, char const *file_path);
         extern backup_database_function_type *backup_database_function;

         /**
          * @return Returns an error code indicating success or failure/.
          *
          * @param connect_handle Specifies the database connection handle.
          *
          * @param file_path Specifies the path of the snapshot to restore.
          */
         typedef int (__stdcall restore_database_function_type) (
            uint4 connect_handle, char const *file_path);
         extern restore_database_function_type *restore_database_function;
            
         // @endgroup
      };
   };
};


#endif

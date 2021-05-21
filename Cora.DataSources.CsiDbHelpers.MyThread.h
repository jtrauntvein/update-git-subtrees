/* Cora.DataSources.CsiDbHelpers.MyThread.h

   Copyright (C) 2009, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Saturday 07 February 2009
   Last Change: Friday 06 December 2019
   Last Commit: $Date: 2020-07-14 15:41:32 -0600 (Tue, 14 Jul 2020) $
   Last Changed by: $Author: jbritt $

*/

#pragma once
#ifndef Cora_DataSources_CsiDbHelpers_MyThread_h
#define Cora_DataSources_CsiDbHelpers_MyThread_h

#include "Cora.DataSources.CsiDbHelpers.Dll.h"
#include "Cora.DataSources.CsiDbHelpers.DbProperties.h"
#include "Cora.Broker.Tob1Header.h"
#include "Csi.Thread.h"
#include "Csi.Events.h"
#include "Csi.CriticalSection.h"
#include "Csi.Condition.h"
#include <deque>


namespace Cora
{
   namespace DataSources
   {
      namespace CsiDbHelpers
      {
         ////////////////////////////////////////////////////////////
         // class CommandBase
         ////////////////////////////////////////////////////////////
         class CommandBase
         {
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            CommandBase(
               uint4 command_type_,
               Csi::EventReceiver *client_):
               command_type(command_type_),
               client(client_),
               error_code(0)
            { }

            ////////////////////////////////////////////////////////////
            // destructor
            ////////////////////////////////////////////////////////////
            virtual ~CommandBase()
            { }

            ////////////////////////////////////////////////////////////
            // execute
            ////////////////////////////////////////////////////////////
            virtual void execute(Dll *dll_interface) = 0;

            ////////////////////////////////////////////////////////////
            // get_error_code
            ////////////////////////////////////////////////////////////
            uint4 get_error_code() const
            { return error_code; }

            ////////////////////////////////////////////////////////////
            // get_last_error
            ////////////////////////////////////////////////////////////
            StrAsc const &get_last_error() const
            { return last_error; }

            ////////////////////////////////////////////////////////////
            // release
            //
            // Can be overloaded to release any resource that was claimed by
            // the command that would not be released through the normal
            // processing of events. 
            ////////////////////////////////////////////////////////////
            virtual void release()
            { }

            ////////////////////////////////////////////////////////////
            // client
            ////////////////////////////////////////////////////////////
            Csi::EventReceiver *client;

            ////////////////////////////////////////////////////////////
            // command_type
            ////////////////////////////////////////////////////////////
            uint4 const command_type;

            ////////////////////////////////////////////////////////////
            // error_code
            ////////////////////////////////////////////////////////////
            int error_code;

            ////////////////////////////////////////////////////////////
            // last_error
            ////////////////////////////////////////////////////////////
            StrAsc last_error;
         };


         ////////////////////////////////////////////////////////////
         // class CommandCompleteEvent
         ////////////////////////////////////////////////////////////
         class CommandCompleteEvent: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // command
            ////////////////////////////////////////////////////////////
            typedef Csi::SharedPtr<CommandBase> command_handle;
            command_handle command;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(command_handle &command)
            {
               CommandCompleteEvent *event = new CommandCompleteEvent(command);
               event->post();
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            CommandCompleteEvent(command_handle &command_):
               Event(event_id, command_->client),
               command(command_)
            { }
         };
         
      
         ////////////////////////////////////////////////////////////
         // class MyThread
         //
         // Defines an object that provides access to functions provided by the
         // CsiDb DLL in a separate thread.  This is accomplished by adding
         // command objects to the thread queue which will be operated on
         // sequentially by the thread.  Once those command objects are complete,
         // and event will be posted to the requester to indicate this
         // completion.  
         ////////////////////////////////////////////////////////////
         class MyThread: public Csi::Thread
         {
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            MyThread();

            ////////////////////////////////////////////////////////////
            // destructor
            ////////////////////////////////////////////////////////////
            virtual ~MyThread();

            ////////////////////////////////////////////////////////////
            // wait_for_end
            ////////////////////////////////////////////////////////////
            virtual void wait_for_end();

            ////////////////////////////////////////////////////////////
            // add_command
            ////////////////////////////////////////////////////////////
            typedef Csi::SharedPtr<CsiDbHelpers::CommandBase> command_handle; 
            void add_command(command_handle command);
         
         protected:
            ////////////////////////////////////////////////////////////
            // execute
            ////////////////////////////////////////////////////////////
            virtual void execute(); 

         private:
            ////////////////////////////////////////////////////////////
            // should_quit
            ////////////////////////////////////////////////////////////
            bool should_quit;

            ////////////////////////////////////////////////////////////
            // commands
            ////////////////////////////////////////////////////////////
            typedef std::list<command_handle> commands_type;
            Csi::CriticalSection commands_protector;
            commands_type commands;

            ////////////////////////////////////////////////////////////
            // add_signal
            ////////////////////////////////////////////////////////////
            Csi::Condition add_signal;
         };

      
         ////////////////////////////////////////////////////////////
         // class ConnectCommand
         ////////////////////////////////////////////////////////////
         class ConnectCommand: public CommandBase
         {
         public:
            ////////////////////////////////////////////////////////////
            // command_id
            ////////////////////////////////////////////////////////////
            static uint4 const command_id;
            
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            ConnectCommand(
               Csi::EventReceiver *client, DbProperties const &properties_);

            ////////////////////////////////////////////////////////////
            // destructor
            ////////////////////////////////////////////////////////////
            virtual ~ConnectCommand();

            ////////////////////////////////////////////////////////////
            // execute
            ////////////////////////////////////////////////////////////
            virtual void execute(Dll *dll);

            ////////////////////////////////////////////////////////////
            // get_connection
            ////////////////////////////////////////////////////////////
            Dll::connection_handle &get_connection()
            { return connection; }

         private:
            ////////////////////////////////////////////////////////////
            // connection
            ////////////////////////////////////////////////////////////
            Dll::connection_handle connection;

            ////////////////////////////////////////////////////////////
            // properties
            ////////////////////////////////////////////////////////////
            DbProperties properties;
         };


         ////////////////////////////////////////////////////////////
         // class DisconnectCommand
         ////////////////////////////////////////////////////////////
         class DisconnectCommand: public CommandBase
         {
         private:
            ////////////////////////////////////////////////////////////
            // connection
            ////////////////////////////////////////////////////////////
            Dll::connection_handle connection;
            
         public:
            ////////////////////////////////////////////////////////////
            // command_id
            ////////////////////////////////////////////////////////////
            static uint4 const command_id;
            
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            DisconnectCommand(
               Csi::EventReceiver *client, Dll::connection_handle &connection_):
               CommandBase(command_id, client),
               connection(connection_)
            { }

            ////////////////////////////////////////////////////////////
            // execute
            ////////////////////////////////////////////////////////////
            virtual void execute(Dll *dll)
            {
               try
               {
                  dll->disconnect(connection.get_rep());
                  connection.clear();
               }
               catch(CsiDbException &)
               { }
            }
         };

         
         ////////////////////////////////////////////////////////////
         // class ListTablesCommand
         //
         // Defines a command that lists the tables associated with a specified
         // connection handle.
         ////////////////////////////////////////////////////////////
         class ListTablesCommand: public CommandBase
         {
         public:
            ////////////////////////////////////////////////////////////
            // command_id
            ////////////////////////////////////////////////////////////
            static uint4 const command_id;
            
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            ListTablesCommand(
               Csi::EventReceiver *client, Dll::connection_handle &connection_);

            ////////////////////////////////////////////////////////////
            // destructor
            ////////////////////////////////////////////////////////////
            virtual ~ListTablesCommand();

            ////////////////////////////////////////////////////////////
            // execute
            ////////////////////////////////////////////////////////////
            virtual void execute(Dll *dll);

            ////////////////////////////////////////////////////////////
            // begin
            ////////////////////////////////////////////////////////////
            typedef Connection::table_names_type table_names_type;
            typedef table_names_type::const_iterator const_iterator;
            const_iterator begin() const
            { return table_names.begin(); }

            ////////////////////////////////////////////////////////////
            // end
            ////////////////////////////////////////////////////////////
            const_iterator end() const
            { return table_names.end(); }

            ////////////////////////////////////////////////////////////
            // empty
            ////////////////////////////////////////////////////////////
            bool empty() const
            { return table_names.empty(); }

            ////////////////////////////////////////////////////////////
            // size
            ////////////////////////////////////////////////////////////
            typedef table_names_type::size_type size_type;
            size_type size() const
            { return table_names.size(); }
            
         private:
            ////////////////////////////////////////////////////////////
            // connection
            ////////////////////////////////////////////////////////////
            Dll::connection_handle connection;

            ////////////////////////////////////////////////////////////
            // table_names
            ////////////////////////////////////////////////////////////
            table_names_type table_names;
         };


         ////////////////////////////////////////////////////////////
         // class ListColumnsCommand
         ////////////////////////////////////////////////////////////
         class ListColumnsCommand: public CommandBase
         {
         public:
            ////////////////////////////////////////////////////////////
            // command_id
            ////////////////////////////////////////////////////////////
            static uint4 const command_id;

            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            ListColumnsCommand(
               Csi::EventReceiver *client,
               Dll::connection_handle &connection_,
               StrAsc const &table_name_):
               CommandBase(command_id, client),
               connection(connection_),
               table_name(table_name_)
            { }

            ////////////////////////////////////////////////////////////
            // destructor
            ////////////////////////////////////////////////////////////
            virtual ~ListColumnsCommand()
            { }

            ////////////////////////////////////////////////////////////
            // execute
            ////////////////////////////////////////////////////////////
            virtual void execute(Dll *dll);

            ////////////////////////////////////////////////////////////
            // begin
            ////////////////////////////////////////////////////////////
            typedef Connection::column_names_type column_names_type;
            typedef column_names_type::const_iterator const_iterator;
            const_iterator begin() const
            { return column_names.begin(); }

            ////////////////////////////////////////////////////////////
            // end
            ////////////////////////////////////////////////////////////
            const_iterator end() const
            { return column_names.end(); }

            ////////////////////////////////////////////////////////////
            // empty
            ////////////////////////////////////////////////////////////
            bool empty() const
            { return column_names.empty(); }

            ////////////////////////////////////////////////////////////
            // size
            ////////////////////////////////////////////////////////////
            typedef column_names_type::size_type size_type;
            size_type size() const
            { return column_names.size(); }

         private:
            ////////////////////////////////////////////////////////////
            // connection
            ////////////////////////////////////////////////////////////
            Dll::connection_handle connection;

            ////////////////////////////////////////////////////////////
            // column_names
            ////////////////////////////////////////////////////////////
            column_names_type column_names;

            ////////////////////////////////////////////////////////////
            // table_name
            ////////////////////////////////////////////////////////////
            StrAsc const table_name;
         };

#ifdef __getNumRecords
         ////////////////////////////////////////////////////////////
         // class NumRecordsCommand
         ////////////////////////////////////////////////////////////
         class NumRecordsCommand: public CommandBase
         {
         public:
            ////////////////////////////////////////////////////////////
            // command_id
            ////////////////////////////////////////////////////////////
            static uint4 const command_id;

            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            NumRecordsCommand(
               Csi::EventReceiver *client,
               Dll::connection_handle &connection_,
               StrAsc const &table_name_):
               CommandBase(command_id, client),
               connection(connection_),
               table_name(table_name_),
               num_records(0)
            { }

            ////////////////////////////////////////////////////////////
            // destructor
            ////////////////////////////////////////////////////////////
            virtual ~NumRecordsCommand()
            { }

            ////////////////////////////////////////////////////////////
            // execute
            ////////////////////////////////////////////////////////////
            virtual void execute(Dll *dll);

            ////////////////////////////////////////////////////////////
            // num records
            ////////////////////////////////////////////////////////////
            int get_num_records()
            { return num_records; }


         private:
            ////////////////////////////////////////////////////////////
            // connection
            ////////////////////////////////////////////////////////////
            Dll::connection_handle connection;

            ////////////////////////////////////////////////////////////
            // table_name
            ////////////////////////////////////////////////////////////
            StrAsc const table_name;

            ////////////////////////////////////////////////////////////
            // num_records
            ////////////////////////////////////////////////////////////
            void get_num_records_in_table();

            int num_records;
         };
#endif
         /**
          * Defines an object that organises the meta-data for a column.
          */
         class ColumnMetadata
         {
         public:
            /**
             * Specifies the lndb column name.
             */
            StrAsc db_column_name;
            
            /**
             * Specifies the units.
             */
            StrAsc units;

            /**
             * Specifies the process string.
             */
            StrAsc process;
         };
         

         /**
          * Defines an object that origanises the meta-data that can be requested from the database.
          */
         class TableMetadata
         {
         public:
            /**
             * Specifies the name of the loggernet station that produced this data.
             */
            StrAsc station_name;

            /**
             * Specifies the name of the loggernet table that produced that data.
             */
            StrAsc table_name;
            
            /**
             * Specifies the list of meta-data for columns.
             */
            typedef std::deque<ColumnMetadata> columns_type;
            columns_type columns;
            
         public:
            /**
             * Constructor
             */
            TableMetadata()
            { }

            /**
             * Destructor
             */
            ~TableMetadata()
            {
               columns.clear();
            }

            /**
             * @return Returns the first column iterator.
             */
            typedef columns_type::iterator iterator;
            typedef columns_type::const_iterator const_iterator;
            iterator begin()
            { return columns.begin(); }
            const_iterator begin() const
            { return columns.begin(); }

            /**
             * @return Returns the iterator beyond the last column.
             */
            iterator end()
            { return columns.end(); }
            const_iterator end() const
            { return columns.end(); }

            /**
             * @return Returns true if there are no columns
             */
            bool empty() const
            { return columns.empty(); }

            /**
             * @return Returns the number of columns
             */
            columns_type::size_type size() const
            { return columns.size(); }

            /**
             * @param column Specifies the metadata for a column to be added.
             */
            void push_back(ColumnMetadata const &column)
            { columns.push_back(column); }
         };
         

         ////////////////////////////////////////////////////////////
         // class QueryCommandBase
         //
         // Defines many of the fields that are common between all database
         // query operations and implements much of the mechanics of managing
         // the data being returned from those queries.  
         ////////////////////////////////////////////////////////////
         class QueryCommandBase: public CommandBase
         {
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            QueryCommandBase(
               Csi::EventReceiver *client,
               uint4 command_id,
               Dll::connection_handle &connection_,
               StrAsc const &table_name_,
               StrAsc const &columns_):
               CommandBase(command_id, client),
               columns(columns_),
               table_name(table_name_),
               connection(connection_),
                  condition(0, false, false)
            { }

            ////////////////////////////////////////////////////////////
            // destructor
            ////////////////////////////////////////////////////////////
            virtual ~QueryCommandBase()
            { }

            ////////////////////////////////////////////////////////////
            // execute
            ////////////////////////////////////////////////////////////
            virtual void execute(Dll *dll_interface);

            ////////////////////////////////////////////////////////////
            // get_header
            ////////////////////////////////////////////////////////////
            typedef Broker::Tob1Header header_type;
            typedef Csi::SharedPtr<header_type> header_handle;
            header_handle &get_header()
            { return header; }

            ////////////////////////////////////////////////////////////
            // continue_query
            ////////////////////////////////////////////////////////////
            void continue_query()
            { condition.set(); }

            ////////////////////////////////////////////////////////////
            // return_records
            ////////////////////////////////////////////////////////////
            typedef Csi::SharedPtr<Cora::Broker::Record> value_type;
            typedef std::deque<value_type> cache_type;
            void return_records(cache_type &records);

            ////////////////////////////////////////////////////////////
            // get_cache
            //
            // Copies any records in the cache to the specified container.
            ////////////////////////////////////////////////////////////
            void get_cache(cache_type &records);

            /**
             * @return Returns the table metadata that was collected for this query.
             */
            typedef Csi::SharedPtr<TableMetadata> table_meta_handle;
            table_meta_handle &get_table_metadata()
            { return table_metadata; }

            /**
             * @param value Specifies the table meta-data to be used for this query.
             */
            void set_table_metadata(table_meta_handle &value)
            { table_metadata = value; }
            
         protected:
            ////////////////////////////////////////////////////////////
            // start_query
            //
            // Must be overloaded to implement the details for this query.  The
            // return value on success should be a query handle that can be
            // used to retrieve the query data. 
            ////////////////////////////////////////////////////////////
            virtual uint4 start_query() = 0;

            ////////////////////////////////////////////////////////////
            // include_record
            ////////////////////////////////////////////////////////////
            virtual bool include_record(value_type &record)
            { return true; }

            /**
             * Retrieves metadata for the table.
             */
            void get_column_meta();

            /**
             * Applies the metadata for the table to the current record description.
             */
            void apply_column_meta();
            
         protected:
            ////////////////////////////////////////////////////////////
            // connection
            ////////////////////////////////////////////////////////////
            Dll::connection_handle connection;

            ////////////////////////////////////////////////////////////
            // table_name
            ////////////////////////////////////////////////////////////
            StrAsc const table_name;
            
            ////////////////////////////////////////////////////////////
            // columns
            ////////////////////////////////////////////////////////////
            StrAsc const columns;

            ////////////////////////////////////////////////////////////
            // header
            ////////////////////////////////////////////////////////////
            header_handle header;

            ////////////////////////////////////////////////////////////
            // condition
            ////////////////////////////////////////////////////////////
            Csi::Condition condition;

            ////////////////////////////////////////////////////////////
            // cache
            ////////////////////////////////////////////////////////////
            Csi::CriticalSection cache_protector;
            cache_type cache;

            /**
             * Specifies the table metadata that was used.
             */
            table_meta_handle table_metadata;
         };


         ////////////////////////////////////////////////////////////
         // class EventQueryHeader
         ////////////////////////////////////////////////////////////
         class EventQueryHeader: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // record
            ////////////////////////////////////////////////////////////
            typedef Csi::SharedPtr<Cora::Broker::Record> record_handle;
            record_handle record;

            ////////////////////////////////////////////////////////////
            // command
            ////////////////////////////////////////////////////////////
            QueryCommandBase *command;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(QueryCommandBase *command, record_handle &record)
            {
               EventQueryHeader *ev(new EventQueryHeader(command, record));
               ev->post();
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            EventQueryHeader(QueryCommandBase *command_, record_handle &record_):
               Event(event_id, command_->client),
               command(command_),
               record(record_)
            { }
         };

         
         ////////////////////////////////////////////////////////////
         // class EventQueryRecords
         ////////////////////////////////////////////////////////////
         class EventQueryRecords: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // records
            ////////////////////////////////////////////////////////////
            typedef QueryCommandBase::cache_type records_type;
            records_type records;

            ////////////////////////////////////////////////////////////
            // command
            ////////////////////////////////////////////////////////////
            QueryCommandBase *command;

            ////////////////////////////////////////////////////////////
            // create
            ////////////////////////////////////////////////////////////
            static EventQueryRecords *create(QueryCommandBase *command)
            {
               return new EventQueryRecords(command);
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            EventQueryRecords(QueryCommandBase *command_):
               Event(event_id, command_->client),
               command(command_)
            { }
         };
#ifdef __getNumRecords

         ////////////////////////////////////////////////////////////
         // class EventQueryNumRecords
         ////////////////////////////////////////////////////////////
         class EventQueryNumRecords: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // numRecords
            ////////////////////////////////////////////////////////////
            int numRecords;

            ////////////////////////////////////////////////////////////
            // command
            ////////////////////////////////////////////////////////////
            QueryCommandBase *command;

            ////////////////////////////////////////////////////////////
            // create
            ////////////////////////////////////////////////////////////
            static EventQueryNumRecords *create(QueryCommandBase *command)
            {
               return new EventQueryNumRecords(command);
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            EventQueryNumRecords(QueryCommandBase *command_):
               Event(event_id, command_->client),
               command(command_)
            { }
         };

         ////////////////////////////////////////////////////////////
         // class GetNumRecordsTimeRangeCommand
         ////////////////////////////////////////////////////////////
         class GetNumRecordsTimeRangeCommand: public QueryCommandBase
         {
         public:
            ////////////////////////////////////////////////////////////
            // command_id
            ////////////////////////////////////////////////////////////
            static uint4 const command_id;

            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            GetNumRecordsTimeRangeCommand(
               Csi::EventReceiver *client,
               Dll::connection_handle &connection,
               StrAsc const &table_name,
               Csi::LgrDate const &begin_time_,
               Csi::LgrDate const &end_time_):
               QueryCommandBase(client, command_id, connection, table_name, columns),
               begin_time(begin_time_),
               end_time(end_time_)
            { }

         protected:
            ////////////////////////////////////////////////////////////
            // start_query
            ////////////////////////////////////////////////////////////
            virtual uint4 start_query();

         private:
            ////////////////////////////////////////////////////////////
            // begin_time
            ////////////////////////////////////////////////////////////
            Csi::LgrDate begin_time;

            ////////////////////////////////////////////////////////////
            // end_time
            ////////////////////////////////////////////////////////////
            Csi::LgrDate end_time;
         };
#endif
         ////////////////////////////////////////////////////////////
         // class GetDataFromTimeCommand
         ////////////////////////////////////////////////////////////
         class GetDataFromTimeCommand: public QueryCommandBase
         {
         public:
            ////////////////////////////////////////////////////////////
            // command_id
            ////////////////////////////////////////////////////////////
            static uint4 const command_id;

            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            GetDataFromTimeCommand(
               Csi::EventReceiver *client,
               Dll::connection_handle &connection,
               StrAsc const &table_name,
               StrAsc const &columns,
               Csi::LgrDate const &start_time_):
               QueryCommandBase(client, command_id, connection, table_name, columns),
               start_time(start_time_)
            { }

         protected:
            ////////////////////////////////////////////////////////////
            // start_query
            ////////////////////////////////////////////////////////////
            virtual uint4 start_query();

         private:
            ////////////////////////////////////////////////////////////
            // start_time
            ////////////////////////////////////////////////////////////
            Csi::LgrDate start_time;

         };


         ////////////////////////////////////////////////////////////
         // class GetDataLastCommand
         ////////////////////////////////////////////////////////////
         class GetDataLastCommand: public QueryCommandBase
         {
         public:
            ////////////////////////////////////////////////////////////
            // command_id
            ////////////////////////////////////////////////////////////
            static uint4 const command_id;

            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            GetDataLastCommand(
               Csi::EventReceiver *client,
               Dll::connection_handle &connection,
               StrAsc const &table_name,
               StrAsc const &columns,
               uint4 count_):
               QueryCommandBase(client, command_id, connection, table_name, columns),
               count(count_)
            { }

         protected:
            ////////////////////////////////////////////////////////////
            // start_query
            ////////////////////////////////////////////////////////////
            virtual uint4 start_query();

         private:
            ////////////////////////////////////////////////////////////
            // count
            ////////////////////////////////////////////////////////////
            uint4 count;
         };

         ////////////////////////////////////////////////////////////
         // class GetDataTimeRangeCommand
         ////////////////////////////////////////////////////////////
         class GetDataTimeRangeCommand: public QueryCommandBase
         {
         public:
            ////////////////////////////////////////////////////////////
            // command_id
            ////////////////////////////////////////////////////////////
            static uint4 const command_id;

            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            GetDataTimeRangeCommand(
               Csi::EventReceiver *client,
               Dll::connection_handle &connection,
               StrAsc const &table_name,
               StrAsc const &columns,
               Csi::LgrDate const &begin_time_,
               Csi::LgrDate const &end_time_):
               QueryCommandBase(client, command_id, connection, table_name, columns),
               begin_time(begin_time_),
               end_time(end_time_)
            { }

         protected:
            ////////////////////////////////////////////////////////////
            // start_query
            ////////////////////////////////////////////////////////////
            virtual uint4 start_query();

         private:
            ////////////////////////////////////////////////////////////
            // begin_time
            ////////////////////////////////////////////////////////////
            Csi::LgrDate begin_time;

            ////////////////////////////////////////////////////////////
            // end_time
            ////////////////////////////////////////////////////////////
            Csi::LgrDate end_time;
         };


         ////////////////////////////////////////////////////////////
         // class PollNewDataCommand
         //
         // Defines a command object that will poll for new data after another
         // query has already succeeded.  This class will allow reuse of a
         // header object from a previous command.
         ////////////////////////////////////////////////////////////
         class PollNewDataCommand: public QueryCommandBase
         {
         public:
            ////////////////////////////////////////////////////////////
            // command_id
            ////////////////////////////////////////////////////////////
            static uint4 const command_id;

            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            PollNewDataCommand(
               Csi::EventReceiver *client,
               Dll::connection_handle &connection,
               header_handle &previous_header,
               StrAsc const &table_name,
               StrAsc const &columns,
               Csi::LgrDate const &begin_time_,
               uint4 begin_record_no_):
               QueryCommandBase(client, command_id, connection, table_name, columns),
               begin_time(begin_time_),
               begin_record_no(begin_record_no_)
            { header = previous_header; }

         protected:
            ////////////////////////////////////////////////////////////
            // start_query
            //////////////////////////////////////////////////////////// 
            virtual uint4 start_query();

            ////////////////////////////////////////////////////////////
            // include_record
            ////////////////////////////////////////////////////////////
            virtual bool include_record(value_type &record)
            {
               bool rtn = true;
               if(record->get_stamp() == begin_time && record->get_record_no() == begin_record_no)
                  rtn = false;
               return rtn;
            }
            
         private:
            ////////////////////////////////////////////////////////////
            // begin_time
            ////////////////////////////////////////////////////////////
            Csi::LgrDate begin_time;

            ////////////////////////////////////////////////////////////
            // begin_record_no
            ////////////////////////////////////////////////////////////
            uint4 begin_record_no; 
         };


         ////////////////////////////////////////////////////////////
         // class ListDataSourcesCommand
         ////////////////////////////////////////////////////////////
         class ListDataSourcesCommand: public CommandBase
         {
         public:
            ////////////////////////////////////////////////////////////
            // command_id
            ////////////////////////////////////////////////////////////
            static uint4 const command_id;

            ////////////////////////////////////////////////////////////
            // condition
            ////////////////////////////////////////////////////////////
            Csi::Condition condition;

            ////////////////////////////////////////////////////////////
            // source_type
            ////////////////////////////////////////////////////////////
            int source_type;
            
            ////////////////////////////////////////////////////////////
            // sources
            ////////////////////////////////////////////////////////////
            Dll::source_names_type sources;
            
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            ListDataSourcesCommand(int source_type_ = -1):
               CommandBase(command_id, 0),
               source_type(source_type_)
            { }

            ////////////////////////////////////////////////////////////
            // execute
            ////////////////////////////////////////////////////////////
            virtual void execute(Dll *dll);

            ////////////////////////////////////////////////////////////
            // release
            ////////////////////////////////////////////////////////////
            virtual void release()
            { condition.set(); }
         };


         ////////////////////////////////////////////////////////////
         // class ListDatabasesCommand
         ////////////////////////////////////////////////////////////
         class ListDatabasesCommand: public CommandBase
         {
         public:
            ////////////////////////////////////////////////////////////
            // command_id
            ////////////////////////////////////////////////////////////
            static uint4 const command_id;

            ////////////////////////////////////////////////////////////
            // condition
            ////////////////////////////////////////////////////////////
            Csi::Condition condition;

            ////////////////////////////////////////////////////////////
            // databases
            ////////////////////////////////////////////////////////////
            Dll::databases_type databases;

            ////////////////////////////////////////////////////////////
            // properties
            ////////////////////////////////////////////////////////////
            DbProperties properties;

            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            ListDatabasesCommand(DbProperties const &properties_):
               CommandBase(command_id, 0),
               properties(properties_)
            { }

            ////////////////////////////////////////////////////////////
            // execute
            ////////////////////////////////////////////////////////////
            virtual void execute(Dll *dll);

            ////////////////////////////////////////////////////////////
            // release
            ////////////////////////////////////////////////////////////
            virtual void release()
            { condition.set(); }
         };


         ////////////////////////////////////////////////////////////
         // class GetTableRangeCommand
         ////////////////////////////////////////////////////////////
         class GetTableRangeCommand: public CommandBase
         {
         public:
            ////////////////////////////////////////////////////////////
            // command_id
            ////////////////////////////////////////////////////////////
            static uint4 const command_id;

            ////////////////////////////////////////////////////////////
            // uri
            ////////////////////////////////////////////////////////////
            StrAsc const uri;
            
            ////////////////////////////////////////////////////////////
            // table_name
            ////////////////////////////////////////////////////////////
            StrAsc const table_name;

            ////////////////////////////////////////////////////////////
            // connection
            ////////////////////////////////////////////////////////////
            Dll::connection_handle connection;

            ////////////////////////////////////////////////////////////
            // final_client
            //
            // Specifies the client that will receive the event that announces
            // the results for this command.  Note that this is different from
            // the client that would receive notification when any command is
            // complete. 
            ////////////////////////////////////////////////////////////
            Csi::EventReceiver *final_client;

            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            GetTableRangeCommand(
               Csi::EventReceiver *final_client_,
               StrAsc const &uri_,
               StrAsc const &table_name_,
               Dll::connection_handle &connection_):
               CommandBase(command_id, 0),
               uri(uri_),
               table_name(table_name_),
               connection(connection_),
               final_client(final_client_)
            { }

            ////////////////////////////////////////////////////////////
            // execute
            ////////////////////////////////////////////////////////////
            virtual void execute(Dll *dll);
         };
      };
   };
};


#endif

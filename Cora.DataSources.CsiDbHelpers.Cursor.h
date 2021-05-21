/* Cora.DataSources.CsiDbHelpers.Cursor.h

   Copyright (C) 2009, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 02 March 2009
   Last Change: Thursday 07 February 2019
   Last Commit: $Date: 2020-07-14 15:41:32 -0600 (Tue, 14 Jul 2020) $
   Last Changed by: $Author: jbritt $

*/

#pragma once
#ifndef Cora_DataSources_CsiDbHelpers_Cursor_h
#define Cora_DataSources_CsiDbHelpers_Cursor_h
#include "Cora.DataSources.Request.h"
#include "Cora.DataSources.CsiDbHelpers.MyThread.h"
#include "Csi.Events.h"
#include "Csi.PolySharedPtr.h"
#include <list>


namespace Cora
{
   namespace DataSources
   {
      class CsiDbSource;

      
      namespace CsiDbHelpers
      {
         /**
          * Defines an object that represents the state of one or more data requests for the
          * database source.
          */
         class Cursor: public Csi::EventReceiver
         {
         public:
            /**
             * Constructor
             *
             * @param source_ Specifies the source that owns this cursor.
             *
             * @param request Specifies the first request to start this cursor.
             */
            typedef Csi::SharedPtr<Request> request_handle;
            Cursor(CsiDbSource *source_, request_handle &request);

            /**
             * Destructor
             */
            virtual ~Cursor();

            /**
             * @param request Specifies the request to add to this cursor.
             *
             * @return Returns true if the request is compatible with the first request that was
             * added.
             */
            bool add_request(request_handle &request);

            /**
             * @param request Specifies a request to be removed.
             *
             * @return Returns true if the request was removed and there are no more requests for
             * this cursor.
             */
            bool remove_request(request_handle &request);

#ifdef __getNumRecords
            /**
            * @param request Specifies a request to be removed.
            *
            * @return Returns true if the request waas removed and there are no more requests for
            * this cursor.
            */
            void get_num_records(request_handle &request);
#endif
            /**
             * Called by the data source to start the first query for this cursor.
             */
            void start();

            /**
             * Called to poll the database for new records.
             */
            void poll();

            /**
             * Overloads the base class version to handle notification of database command
             * completion.
             */
            virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

         private:
            /**
             * Called to handle database errors.
             */
            void on_error(int error_code);

            /**
             * Called to report to all request sinks that the cursor is ready.
             */
            void report_ready(QueryCommandBase::value_type &record);

            /**
             * Called to report data records to all request sinks.
             */
            void report_records(QueryCommandBase::cache_type &records);

            /**
             * Called to generate the list of column names.
             */
            void generate_column_names();
            
         private:
            /**
             * Specifies the source that owns this cursor.
             */
            CsiDbSource *source;

            /**
             * Specifies the set of requests serviced by this cursor.
             */
            typedef std::list<request_handle> requests_type;
            requests_type requests;

            /**
             * Specifies the query that is currently pending.
             */
            typedef CsiDbHelpers::CommandBase command_base_type;
            typedef CsiDbHelpers::QueryCommandBase query_command_type;
            Csi::PolySharedPtr<command_base_type, query_command_type> pending_query;

            /**
             * Specifies the TOB1 header used for queries.
             */
            query_command_type::header_handle header;

            /**
             * Specifies the state of this cursor.
             */
            enum state_type
            {
               state_not_started,
               state_relative_query,
               state_ready_to_poll,
               state_polling,
               state_satisfied,
               state_error
            } state;

            /**
             * Specifies the last time stamp that was received.
             */
            Csi::LgrDate last_stamp;

            /**
             * Specifies the last record number that was received.
             */
            uint4 last_record_no;

            /**
             * Set to true if we have already reported the ready event to the request sinks.
             */
            bool reported_ready;

            /**
             * Set to true if the request will be satisfied after the first query.
             */
            bool satisfied_after_first;

            /**
             * Specifies the collection of column names being requested.
             */
            StrAsc column_names;

            /**
             * Set to true if data has been received.
             */
            bool has_received_data;

            /**
             * Specifies the collection of records that are ready to be recycled.
             */
            typedef QueryCommandBase::cache_type cache_type;
            cache_type records_cache;

            /**
             * Specifies the cached table meta data retrieved from the first query.
             */
            Csi::SharedPtr<TableMetadata> cached_meta;
         };
      };
   };
};


#endif

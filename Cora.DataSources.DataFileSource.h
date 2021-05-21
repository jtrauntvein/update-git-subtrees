/* Cora.DataSources.DataFileSource.h

   Copyright (C) 2008, 2014 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 20 August 2008
   Last Change: Friday 07 November 2014
   Last Commit: $Date: 2014-11-07 16:55:18 -0600 (Fri, 07 Nov 2014) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Cora_DataSources_DataFileSource_h
#define Cora_DataSources_DataFileSource_h

#include "Cora.DataSources.SourceBase.h"
#include "Cora.DataSources.DataFileSymbol.h"
#include "Cora.DataSources.SinkBase.h"
#include "Cora.Broker.DataFileReader.h"
#include "Scheduler.h"
#include "Csi.PolySharedPtr.h"
#include "Csi.Thread.h"
#include "Csi.CriticalSection.h"


namespace Cora
{
   namespace DataSources
   {
      namespace DataFileSourceHelpers
      {
         /**
          * Defines an object that will act as a cursor for a set of data
          * requests with similar parameters.
          */
         class ReadState
         {
         public:
            /**
             * Construct from an initial request and array identifier.
             *
             * @param request  Specifies the first request for this cursor.
             *
             * @param Specifies the array id for this cursor.
             */
            typedef SourceBase::request_handle request_handle;
            ReadState(request_handle &request, uint4 array_id_);

            /**
             * Destructor
             */
            ~ReadState();

            /**
             * Attempts to add the specified request to this cursor.
             *
             * @param request Specifies the request to be added.
             *
             * @param array_id Specifies the array ID associated with this
             * request.
             *
             * @return Returns true if the request is compatible with this
             * cursor and was added, false otherwise.
             */
            bool add_request(request_handle &request, uint4 array_id);

            /**
             * Called to test to see if the data file has new records that can
             * be reported.
             *
             * @param reader Specifies the data file reader.
             *
             * @param index Specifies the index of records in the reader.
             *
             * @param must_stop Specifies a boolean reference that will be set
             * to true if the cursor must stop searching for new records.
             */
            void poll(
               Cora::Broker::DataFileReader &reader,
               Cora::Broker::DataFileReader::index_type &index,
               bool &must_stop);

            /**
             * @return Returns true if all of the requests associated with this
             * cursor are satisifed.
             */
            bool is_satisfied() const;

            /**
             * Specifies the set of requests serviced by this cursor.
             */
            typedef std::list<request_handle> requests_type;
            requests_type requests;

            /**
             * Specifies the current data offset in the file reader.
             */
            int8 current_offset;

            /**
             * Set to true of this cursor has been polled for data previously.
             */
            bool previously_polled;

            /**
             * Specifies the last time that the data file was changed.
             */
            Csi::LgrDate last_change_date;

            /**
             * Specifies the set of records returned from the last poll.
             */
            typedef SinkBase::records_type results_type;
            results_type results;

            /**
             * Set to true of this cursor needs to be polled.
             */
            bool poll_pending;

            /**
             * Set to true at the end of a poll to indicate whether there is
             * the potential for more data to be returned.
             */
            bool has_more_data;

            /**
             * Specifies the last record time stamp that was returned.
             */
            Csi::LgrDate last_record_stamp;

            /**
             * Specifies the array ID for the requests in this cursor.
             */
            uint4 const array_id;

            /**
             * Specifies the index for the last record that was reported though
             * this cursor.
             */
            Broker::DataFileIndexEntry last_record_entry;
         };

         
         /**
          * Specifies a thread object that will be used for processing the
          * source data file.
          */
         class MyThread: public Csi::Thread
         {
         public:
            /**
             * Constructor.
             *
             * @param source_ Specifies the source that owns this thread.
             *
             * @param reader_ Specifies the file reader owned by the source.
             */
            typedef Cora::Broker::DataFileReader reader_type;
            typedef Csi::SharedPtr<reader_type> reader_handle; 
            MyThread(DataFileSource *source_, reader_handle &reader_);

            /**
             * Destructor
             */
            virtual ~MyThread();

            /**
             * Overloads the base class start() method.
             */
            virtual void start();

            /**
             * Overloads the wait_for_end() method to set the conditions for
             * stopping this thread.
             */
            virtual void wait_for_end();

            /**
             * Adds a cursor to be processed by this thread.
             */
            typedef Csi::SharedPtr<ReadState> read_handle;
            void add_read(read_handle &read);

            /**
             * Overloads the execute() method to perform the thread processing.
             */
            virtual void execute();
            
         private:
            /**
             * Specifies the source that owns this thread.
             */
            DataFileSource *source;
            
            /**
             * Used to provide synchronisation for the reads queue.
             */
            Csi::CriticalSection mutex;

            /**
             * Specifies the list of read cursors that need to be serviced.
             */
            std::list<read_handle> reads;

            /**
             * Specifies the data file reader owned by the source.
             */
            reader_handle reader;

            /**
             * Set to true if this thread must be shut down.
             */
            bool must_stop;

            /**
             * Used to signal this thread from the main thread that there are
             * cursors waiting to be serviced.
             */
            Csi::Condition trigger;
         };
      };


      /**
       * Defines a data source that can read a data file formatted as TOA5,
       * TOACI1, TOB1/2/3, CSIXML, or mixed array with FSL.
       */
      class DataFileSource:
         public SourceBase,
         public SchedulerClient,
         public OneShotClient,
         public Csi::EventReceiver
      {
      public:
         /**
          * Constructor
          *
          * @param name Specifies the name for this source.
          */
         DataFileSource(StrUni const &name);

         /**
          * Destructor
          */
         virtual ~DataFileSource();

         /**
          * @return Overloads the base class to indicate that this a file
          * source.
          */
         virtual SymbolBase::symbol_type_code get_type()
         { return SymbolBase::type_file_source; }
         
         /**
          * Overloads the base class version to start the source thread, open
          * the reader, and index the file content.
          */
         virtual void connect();

         /**
          * Overloads the base class version to release the reader and stop the
          * reader thread.
          */
         virtual void disconnect();

         /**
          * Set to true if the reader thread has been started.
          */
         virtual bool is_connected() const;

         /**
          * Overloads the base class version to start all pending requests.
          */
         virtual void start();

         /**
          * Overloads the base class version to release all current requests.
          */
         virtual void stop();

         /**
          * Fills in the specified XML structure with the settings for this
          * source.
          *
          * @param prop_xml  Specifies the XML structure that will be filled
          * in.
          */
         virtual void get_properties(Csi::Xml::Element &prop_xml);

         /**
          * Overloads the base class version to initialise source properties
          * from the specified XML structure.
          *
          * @param prop_xml  Specifies the XML structure that contains the
          * properties for this source.
          */
         virtual void set_properties(Csi::Xml::Element &prop_xml);

         /**
          * Sets the data source manager.
          */
         virtual void set_manager(Manager *manager);

         /**
          * Adds a request to this source.
          *
          * @param request  Specifies the application data request.
          *
          * @param more_to_follow Set to true if there are more requests that
          * are expected to follow.
          */
         virtual void add_request(request_handle &request, bool more_to_follow = false);

         /**
          * Activites all recently added requests.
          */
         virtual void activate_requests();

         /**
          * Removes a request from this source.
          */
         virtual void remove_request(request_handle &request);

         /**
          * Removes all requests associated with this source.
          */
         virtual void remove_all_requests();

         /**
          * @return Returns the symbol object associated with this source.
          */
         virtual symbol_handle get_source_symbol();

         /**
          * Enables or disables the poll schedule for this source.
          *
          * @param enabled Set to true if the poll scheduled should be
          * enabled.
          */
         virtual void enable_poll_schedule(bool enabled)
         { poll_schedule_enabled = enabled; }

         /**
          * Overloads the base class version to handle timed events.
          */
         virtual void onOneShotFired(uint4 id);
         
         /**
          * Overloads the base class version to handle scheduled events.
          */
         virtual void onScheduledEvent(uint4 id);

         /**
          * @return Returns (and possible allocates) the reader object.
          *
          * @param do_reload  Set to true if the reader is to be reloaded.
          */
         typedef Cora::Broker::DataFileReader reader_type;
         typedef Csi::SharedPtr<reader_type> reader_handle;
         reader_handle &get_reader(bool do_reload);

         /**
          * Overloads the base class to handle received events.
          */
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

         /**
          * Overloads the base class to break the specified URL into a
          * collection of symbols.
          *
          * @param symbols Specifies the collection of symbols to be returned.
          *
          * @param uri Specifies the URI to be parsed.
          */
         virtual void breakdown_uri(symbols_type &symbols, StrUni const &uri);

         /**
          * @return Returns the maximum number of bytes that should be
          * processed in the file.
          */
         int8 get_backfill_bytes() const
         { return backfill_bytes; }
         
      private:
         // @group: properties of this source

         /**
          * Specifies the name and path of the data file.
          */
         StrAsc file_name;

         /**
          * Specifies the name and path for the optional labels file (required
          * for mixed array files).
          */
         StrAsc labels_file_name;

         /**
          * Specifies the base date/time for the polling schedule.
          */
         Csi::LgrDate poll_schedule_base;

         /**
          * Specifies the source polling interval in milliseconds.
          */
         uint4 poll_schedule_interval;

         /**
          * Specifies the maximum number of bytes in the input file that should
          * be processed.
          */
         int8 backfill_bytes;

         // @endgroup:

         /**
          * Specifies the object that drives schedules for this source.
          */
         Csi::SharedPtr<Scheduler> scheduler;

         /**
          * Specifies the value of the schedule identifier for polling data.
          */
         uint4 poll_id;

         /**
          * Specifies whether scheduled polling should take place.
          */
         bool poll_schedule_enabled;

         /**
          * Specifies the data file reader object.
          */
         Csi::SharedPtr<reader_type> reader;

         /**
          * Specifies the symbol for this data source.
          */
         Csi::PolySharedPtr<SymbolBase, DataFileSymbol> source_symbol;

         /**
          * Specifies the thread used to process data and requests.
          */
         typedef DataFileSourceHelpers::MyThread thread_type;
         Csi::SharedPtr<thread_type> thread;

         /**
          * Specifies the event timer.
          */
         Csi::SharedPtr<OneShot> timer;
         
         /**
          * Specifies the event timer that will be set to retry a failed
          * connection.
          */
         uint4 retry_id;

         /**
          * Set to true if we are connected to the data file.
          */
         bool was_connected;

         /**
          * Specifies the set of cursors for this source.
          */
         typedef DataFileSourceHelpers::ReadState read_state_type;
         typedef Csi::SharedPtr<read_state_type> read_state_handle;
         typedef std::list<read_state_handle> read_states_type;
         read_states_type read_states;
      };
   };
};


#endif

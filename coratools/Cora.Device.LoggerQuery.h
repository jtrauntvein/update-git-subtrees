/* Cora.Device.LoggerQuery.h

   Copyright (C) 2001, 2017 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 12 December 2001
   Last Change: Wednesday 10 May 2017
   Last Commit: $Date: 2017-05-11 10:32:26 -0600 (Thu, 11 May 2017) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Cora_Device_LoggerQuery_h
#define Cora_Device_LoggerQuery_h

#include "Cora.Device.DeviceBase.h"
#include "Csi.InstanceValidator.h"
#include "Csi.Events.h"
#include "LgrDate.h"


namespace Cora
{
   namespace Device
   {
      //@group class forward declarations
      class LoggerQuery;
      //@endgroup


      ////////////////////////////////////////////////////////////
      // class LoggerQueryClient
      //
      // Defines a base class for an object that can serve as a query
      // client. This class defines the virtual methods that serve as event
      // notifications from the LoggerQuery component.
      ////////////////////////////////////////////////////////////
      class LoggerQueryClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_query_status
         //
         // Called to provide status information regarding the progress of the query.
         ////////////////////////////////////////////////////////////
         enum status_code_type
         {
            status_temporary_table_created = 1,
            status_some_data_collected = 2,
            status_all_data_collected = 3,
            status_query_interrupted = 4,
            status_file_mark_created = 5
         };
         virtual void on_query_status(
            LoggerQuery *query,
            status_code_type status_code,
            StrUni const &temp_table_name,
            uint4 record_count,
            uint4 begin_record_no,
            uint4 end_record_no)
         { }


         ////////////////////////////////////////////////////////////
         // on_failure
         ////////////////////////////////////////////////////////////
         enum failure_type
         {
            failure_unknown = 0,
            failure_invalid_logon = 1,
            failure_server_session_failed = 2,
            failure_invalid_device_name = 3,
            failure_unsupported = 4,
            failure_server_security_blocked = 5,
            failure_logger_security_blocked = 6,
            failure_communication_failure = 7,
            failure_communication_disabled = 8,
            failure_invalid_table_name = 9,
            failure_invalid_table_definition = 10,
            failure_insufficient_resources = 11,
         };
         virtual void on_failure(
            LoggerQuery *query,
            failure_type failure) = 0;
      };


      ////////////////////////////////////////////////////////////
      // class LoggerQuery
      //
      // Defines a component that encapsulates the "Device Logger Query
      // Transaction".  This component provides properties that allow the
      // application that uses it to specify the query parameters and to
      // receive event notifications regarding the progress of the query. This
      // component will also serve to keep the resources (specifically the
      // temporary table allocated for the query) allocated in the server until
      // the component is deleted or stopped (finish() is called).
      //
      // In order to use this component, an application must provide an object
      // that is derived from class LoggerQueryClient. This object will receive
      // the event notifications from the query component. The application
      // should instantiate an object of type LoggerQuery and set its
      // properties according to its needs. It can then call one of the
      // versions of start(). At this point, the query with the server will
      // begin. While the query is taking place with the station, the component
      // will call the client's on_query_status() at times to inform the client
      // of the progress of the query. If an error occurs at any time, the
      // client's on_failure() method will be called and the component returned
      // to a standby state.
      //
      // This component does not provide an interface to access the data
      // queried. In order to do so, the application can use
      // Cora::Broker::DataAdvisor component. The temporary table allocated by
      // the server will be kept as long as the component is active. If a
      // failure occurrs or if the component is stopped, the server resources
      // will be released.
      ////////////////////////////////////////////////////////////
      class LoggerQuery:
         public DeviceBase,
         public Csi::EventReceiver
      {
      public:
         ////////////////////////////////////////////////////////////
         // enum query_mode_type
         ////////////////////////////////////////////////////////////
         enum query_mode_type
         {
            query_date_range = 1,
            query_recent_records = 2,
            query_record_number_range = 3,
            query_all_since_last_poll = 4,
            query_all = 5,
            query_start_at_record = 6,
            query_backfill = 7
         };

      private:
         //@group properties
         ////////////////////////////////////////////////////////////
         // table_name
         ////////////////////////////////////////////////////////////
         StrUni table_name;

         ////////////////////////////////////////////////////////////
         // query_mode
         ////////////////////////////////////////////////////////////
         query_mode_type query_mode;

         ////////////////////////////////////////////////////////////
         // begin_date
         //
         // specifies the start of the date range when query_mode is set to
         // query_date_range.
         ////////////////////////////////////////////////////////////
         LgrDate begin_date;

         ////////////////////////////////////////////////////////////
         // end_date
         //
         // Specifies the end of the date range when query_mode is set to
         // query_date_range. Note that this range is a half-open interval
         // meaning that the query will look for dates up to but not including
         // the end date.
         ////////////////////////////////////////////////////////////
         LgrDate end_date;

         ////////////////////////////////////////////////////////////
         // number_of_records
         //
         // Specifies the number of records that will be sent when query_mode
         // is set to query_recent_records.
         ////////////////////////////////////////////////////////////
         uint4 number_of_records;

         ////////////////////////////////////////////////////////////
         // begin_record_no
         //
         // Specifies the start record number when query_mode is set to
         // query_record_number_range.
         ////////////////////////////////////////////////////////////
         uint4 begin_record_no;

         ////////////////////////////////////////////////////////////
         // end_record_no
         //
         // Specifies the ending record number when the query mode is set to
         // query_record_number_range. Note that the interval is half-open
         // meaning that the query will include records up to but not including
         // the end_record_no.
         ////////////////////////////////////////////////////////////
         uint4 end_record_no;

         ////////////////////////////////////////////////////////////
         // use_same_table
         //
         // Specifies whether the queried records should be stored in the same
         // table that is used to store records normally collected from the
         // specified table.  
         ////////////////////////////////////////////////////////////
         bool use_same_table;

         /**
          * Specifies the backfill interval.
          */
         int8 backfill_interval;
         
         //@endgroup

         ////////////////////////////////////////////////////////////
         // temp_table_name
         //
         // This value is not a settable property but is provided so that it
         // can be read using get_temp_table_name while the component is
         // active.
         ////////////////////////////////////////////////////////////
         StrUni temp_table_name;

      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         LoggerQuery();

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~LoggerQuery();

         //@group property set methods
         ////////////////////////////////////////////////////////////
         // set_query_mode
         ////////////////////////////////////////////////////////////
         void set_query_mode(query_mode_type query_mode_);

         ////////////////////////////////////////////////////////////
         // set_table_name
         ////////////////////////////////////////////////////////////
         void set_table_name(StrUni const &table_name_);

         ////////////////////////////////////////////////////////////
         // set_begin_date
         ////////////////////////////////////////////////////////////
         void set_begin_date(LgrDate const &begin_date_);

         ////////////////////////////////////////////////////////////
         // set_end_date
         ////////////////////////////////////////////////////////////
         void set_end_date(LgrDate const &end_date_);

         ////////////////////////////////////////////////////////////
         // set_number_of_records
         ////////////////////////////////////////////////////////////
         void set_number_of_records(uint4 number_of_records_);

         ////////////////////////////////////////////////////////////
         // set_begin_record_no
         ////////////////////////////////////////////////////////////
         void set_begin_record_no(uint4 begin_record_no_);

         ////////////////////////////////////////////////////////////
         // set_end_record_no
         ////////////////////////////////////////////////////////////
         void set_end_record_no(uint4 end_record_no_);

         /**
          * Sets the backfill interval parameter and sets up the query mode to use backfill.
          */
         void set_backfill_interval(int8 value);

         ////////////////////////////////////////////////////////////
         // get_use_same_table
         ////////////////////////////////////////////////////////////
         bool get_use_same_table() const
         { return use_same_table; }

         ////////////////////////////////////////////////////////////
         // set_use_same_table
         ////////////////////////////////////////////////////////////
         void set_use_same_table(bool use_same_table_);

         //@endgroup

         //@group property read methods
         ////////////////////////////////////////////////////////////
         // get_query_mode
         ////////////////////////////////////////////////////////////
         query_mode_type get_query_mode() const { return query_mode; }

         ////////////////////////////////////////////////////////////
         // get_table_name
         ////////////////////////////////////////////////////////////
         StrUni const &get_table_name() const { return table_name; }

         ////////////////////////////////////////////////////////////
         // get_begin_date
         ////////////////////////////////////////////////////////////
         LgrDate const &get_begin_date() const { return begin_date; }

         ////////////////////////////////////////////////////////////
         // get_end_date
         ////////////////////////////////////////////////////////////
         LgrDate const &get_end_date() const { return end_date; }

         ////////////////////////////////////////////////////////////
         // get_number_of_records
         ////////////////////////////////////////////////////////////
         uint4 get_number_of_records() const { return number_of_records; }

         ////////////////////////////////////////////////////////////
         // get_begin_record_no
         ////////////////////////////////////////////////////////////
         uint4 get_begin_record_no() const { return begin_record_no; }

         ////////////////////////////////////////////////////////////
         // get_end_record_no
         ////////////////////////////////////////////////////////////
         uint4 get_end_record_no() const { return end_record_no; }

         /**
          * @return Returns the backfill interval.
          */
         int8 get_backfill_interval() const
         { return backfill_interval; }

         ////////////////////////////////////////////////////////////
         // get_temp_table_name
         ////////////////////////////////////////////////////////////
         StrUni const &get_temp_table_name() const { return temp_table_name; }

         ////////////////////////////////////////////////////////////
         // start
         //
         // This version of start is intended to work with a newly created router object. 
         ////////////////////////////////////////////////////////////
         typedef LoggerQueryClient client_type;
         void start(
            client_type *client_,
            router_handle &router);

         ////////////////////////////////////////////////////////////
         // start
         //
         // This version of start() will initialise itself using the logon properties, router, and
         // default net session of the specified component.
         ////////////////////////////////////////////////////////////
         void start(
            client_type *client_,
            ClientBase *other_component);

         ////////////////////////////////////////////////////////////
         // finish
         ////////////////////////////////////////////////////////////
         void finish();

         ////////////////////////////////////////////////////////////
         // format_failure
         ////////////////////////////////////////////////////////////
         static void format_failure(std::ostream &out, client_type::failure_type failure);

      protected:
         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

         ////////////////////////////////////////////////////////////
         // onNetMessage
         ////////////////////////////////////////////////////////////
         virtual void onNetMessage(
            Csi::Messaging::Router *rtr,
            Csi::Messaging::Message *msg);

         ////////////////////////////////////////////////////////////
         // on_devicebase_ready
         ////////////////////////////////////////////////////////////
         virtual void on_devicebase_ready();

         ////////////////////////////////////////////////////////////
         // on_devicebase_failure
         ////////////////////////////////////////////////////////////
         virtual void on_devicebase_failure(devicebase_failure_type failure);

         ////////////////////////////////////////////////////////////
         // on_devicebase_session_failure
         ////////////////////////////////////////////////////////////
         virtual void on_devicebase_session_failure();

      private:
         ////////////////////////////////////////////////////////////
         // on_status_not
         ////////////////////////////////////////////////////////////
         void on_status_not(Csi::Messaging::Message *message);

         ////////////////////////////////////////////////////////////
         // on_stopped_not
         ////////////////////////////////////////////////////////////
         void on_stopped_not(Csi::Messaging::Message *message);
         
      private:
         ////////////////////////////////////////////////////////////
         // client
         ////////////////////////////////////////////////////////////
         client_type *client;

         ////////////////////////////////////////////////////////////
         // state
         ////////////////////////////////////////////////////////////
         enum state_type
         {
            state_standby,
            state_delegate,
            state_before_active,
            state_active
         } state;

         ////////////////////////////////////////////////////////////
         // query_transaction
         ////////////////////////////////////////////////////////////
         uint4 query_transaction;
      };
   };
};


#endif

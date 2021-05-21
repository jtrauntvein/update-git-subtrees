/* Cora.Device.ManualQuery.h

   Copyright (C) 2000, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 02 August 2000
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Committed by: $Author: jon $
   
*/


#ifndef Cora_Device_ManualQuery_h
#define Cora_Device_ManualQuery_h


#include "Cora.Device.DeviceBase.h"
#include "Cora.Device.DeviceSettingTypes.h"


namespace Cora
{
   namespace Device
   {
      //@group class forward declarations
      class ManualQuery;
      //@endgroup

      ////////////////////////////////////////////////////////////
      // class ManualQueryClient
      //
      // Defines the interface that is expected from a client to a ManualQuery object.
      ////////////////////////////////////////////////////////////
      class ManualQueryClient: public Csi::InstanceValidator
      {
      public:
         ////////// on_complete
         // Called when the manual query transaction has been finished. The resp_code parameter will
         // indicate the outcome of the transaction. The file_mark_no parameter contains the file
         // mark that was allocated in the table for the query results and should only be used when
         // the resp_code parameter indicates success.
         enum resp_code_type
         {
            resp_unknown = 0,   // covers any unexpected error
            resp_success = 1,   // the query succeeded
            resp_invalid_logon = 2, // invalid logon properties
            resp_invalid_device_name = 3, // invalid device name specified
            resp_server_security_blocked = 4, // server security stopped the transaction(s)
            resp_logger_security_blocked = 5, // datalogger security blocked the transaction
            resp_invalid_table_name = 6, // an invalid table name was specified
            resp_another_in_progress = 7, // another transaction is in progress
            resp_logger_communication_failure = 8, // communications with the datalogger  failed
                                                   //  the transaction was active
            resp_logger_communications_disabled = 9,// datalogger communications are disabled
            resp_table_enabled = 10, // The specified table is enabled for scheduled collection
            resp_unsupported = 11,    // this transaction is not supported on the server or on the
                                      // specified device
            resp_session_failed = 12, // the host messaging session failed
         };
         virtual void on_complete(ManualQuery *query,
                                  resp_code_type resp_code,
                                  uint4 file_mark_no) = 0;
      };


      ////////////////////////////////////////////////////////////
      // class ManualQuery
      //
      // Defines a concrete class that encapsulates the Device Manual Query (datalogger directed
      // query) transaction.
      //
      // This class works only on cora server version 1.1.
      //
      // When first created, an object of this class will be in a standby state where properties
      // (device name, query mode, query parameters, etc.) can be set. After start() is called, a
      // device manual query will be started and the results will be reported to the client via
      // on_complete().
      //
      // Once on_complete() has been called, the object will return to a standby state.
      ////////////////////////////////////////////////////////////
      class ManualQuery:
         public DeviceBase,
         public Csi::EvReceiver
      {
      public:
         ////////// constructor
         ManualQuery();

         ////////// destructor
         virtual ~ManualQuery();

         //@group properties setting methods
         ////////// set_table_name
         void set_table_name(StrUni const &table_name_);

         ////////// set_date_range
         // Sets the query mode to collect records within a time stamp range and sets the begin and
         // ending stamps parameters for that mode.
         void set_date_range(int8 const &begin_stamp_, int8 const &end_stamp_);

         ////////// set_offset
         // Sets the query mode to collect records at an offset from the newest and sets the offset
         // parameter to that mode.
         void set_offset(uint4 offset);

         ////////// set_record_no_range
         // Sets the query mode to collect records by a range of record numbers and sets up the
         // parameters for the half-open interval.
         void set_record_no_range(uint4 begin_record_no, uint4 end_record_no);
         //@endgroup

         ////////// get_table_name
         StrUni const &get_table_name() const { return table_name; }

         ////////// start
         typedef ManualQueryClient client_type;
         void start(
            client_type *client_,
            router_handle &router);
         void start(
            client_type *client_,
            ClientBase *other_component);

         ////////// finish
         void finish();

      protected:
         //@group DeviceBase overloaded methods
         ////////// on_devicebase_ready
         virtual void on_devicebase_ready();

         ////////// on_devicebase_failure
         virtual void on_devicebase_failure(devicebase_failure_type failure);

         ////////// on_devicebase_session_failure
         virtual void on_devicebase_session_failure();

         ////////// onNetMessage
         virtual void onNetMessage(Csi::Messaging::Router *rtr,
                                   Csi::Messaging::Message *msg);
         //@endgroup

         ////////// receive
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);
         
      private:
         ////////// start_query
         // Starts the actual query transaction
         void start_query();
         
      private:
         ////////// client
         ManualQueryClient *client;

         ////////// state
         enum state_type
         {
            state_standby,
            state_delegate,
            state_active
         } state;

         //@group properties storage
         ////////// table_name
         StrUni table_name;

         ////////// query_mode
         enum query_mode_type
         {
            mode_unspecified = 0,
            mode_by_time_stamp = 1,
            mode_most_recent_x = 2,
            mode_by_record_no = 3,
         } query_mode;
         
         ////////// begin_date
         int8 begin_date;

         ////////// end_date
         int8 end_date;

         ////////// offset
         uint4 offset;

         ////////// begin_record_no
         uint4 begin_record_no;

         ////////// end_record_no
         uint4 end_record_no;
         //@endgroup
      };
   };
};

#endif

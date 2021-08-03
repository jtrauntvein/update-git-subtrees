/* Cora.Device.LowLevelLogAdvisor.h

   Copyright (C) 2002, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 05 July 2002
   Last Change: Thursday 21 July 2016
   Last Commit: $Date: 2016-07-21 15:51:16 -0600 (Thu, 21 Jul 2016) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Cora_Device_LowLevelLogAdvisor_h
#define Cora_Device_LowLevelLogAdvisor_h

#include "Cora.Device.DeviceBase.h"
#include "Csi.Events.h"
#include "LgrDate.h"
#include <list>


namespace Cora
{
   namespace Device
   {
      //@group class forward declarations
      class LowLevelLogAdvisor;
      //@endgroup


      ////////////////////////////////////////////////////////////
      // class LowLevelLogAdvisorClient
      ////////////////////////////////////////////////////////////
      class LowLevelLogAdvisorClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_started
         ////////////////////////////////////////////////////////////
         virtual void on_started(
            LowLevelLogAdvisor *advisor)
         { }

         ////////////////////////////////////////////////////////////
         // on_failure
         ////////////////////////////////////////////////////////////
         enum failure_type
         {
            failure_unknown = 0,
            failure_session_failed = 1,
            failure_invalid_logon = 2,
            failure_server_security_blocked = 3,
            failure_unsupported_transaction = 4,
            failure_server_cancelled = 5,
            failure_invalid_device_name = 6,
         };
         virtual void on_failure(
            LowLevelLogAdvisor *advisor,
            failure_type failure) = 0;

         ////////////////////////////////////////////////////////////
         // on_log_message
         ////////////////////////////////////////////////////////////
         virtual void on_log_message(
            LowLevelLogAdvisor *advisor,
            LgrDate const &time_stamp,
            bool is_input, 
            void const *record_data,
            uint4 record_data_len) = 0;
      };


      ////////////////////////////////////////////////////////////
      // class LowLevelLogAdvisor
      //
      // Defines a component that provides access to the device low level log transaction.  An
      // application can use this component by providing a client object derived from
      // LowLevelLogAdvisorClient and creating an instance of LowLevelLogAdvisor.  The application
      // must then set the appropriate properties including device_name and call one of the versions
      // of start().  Once the server transaction is started, the client object's on_log_message()
      // method will be invoked periodically when log data is available to be read.  The application
      // can then invoke either next_record() or next_batch() to get the next set.
      //
      // The two methods, next_record() and next_batch() are provided so that the application can
      // choose the appropriate type of processing for its requirements.  next_batch() can take
      // advantage of the server transaction to make the transfer of data more efficient.
      // next_record() can, however, be conceptually easier for a simple application.
      //
      // The component will continue until the client calls the finish() method or the server shuts
      // down the transaction for some reason. If the transaction had to be shut down, the
      // client's on_failure() method will be invoked. 
      ////////////////////////////////////////////////////////////
      class LowLevelLogAdvisor:
         public DeviceBase,
         public Csi::EventReceiver
      {
      private:
         ////////////////////////////////////////////////////////////
         // records_per_batch
         ////////////////////////////////////////////////////////////
         uint4 records_per_batch;

         ////////////////////////////////////////////////////////////
         // back_uo_count
         ////////////////////////////////////////////////////////////
         uint4 back_up_count;
         
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         LowLevelLogAdvisor();

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~LowLevelLogAdvisor();

         ////////////////////////////////////////////////////////////
         // get_records_per_batch
         ////////////////////////////////////////////////////////////
         uint4 get_records_per_batch() const { return records_per_batch; }

         ////////////////////////////////////////////////////////////
         // get_back_up_count
         ////////////////////////////////////////////////////////////
         uint4 get_back_up_count() const { return back_up_count; }

         ////////////////////////////////////////////////////////////
         // set_records_per_batch
         ////////////////////////////////////////////////////////////
         void set_records_per_batch(uint4 records_per_batch_);

         ////////////////////////////////////////////////////////////
         // set_back_up_count
         ////////////////////////////////////////////////////////////
         void set_back_up_count(uint4 back_up_count_);

         ////////////////////////////////////////////////////////////
         // start
         //
         // Used to start this component using the router and connection already owned by anothger
         // "started" component.  Using this version of start() prevents having to repeat the logon
         // protocol. 
         ////////////////////////////////////////////////////////////
         typedef LowLevelLogAdvisorClient client_type;
         void start(
            ClientBase *other_component,
            client_type *client_);

         ////////////////////////////////////////////////////////////
         // start
         //
         // Used to start this component using a newly initialised router.  This router should not
         // have any connections when passed here.
         ////////////////////////////////////////////////////////////
         void start(
            router_handle &router,
            client_type *client_);

         ////////////////////////////////////////////////////////////
         // finish
         //
         // Used to tereminate the component activities as well as to release the server connections
         // and transactions.
         ////////////////////////////////////////////////////////////
         virtual void finish();

         ////////////////////////////////////////////////////////////
         // next_record
         //
         // Should be called by the client when it is ready to receive notification regarding the
         // next low level log record.  If the unread_records cache is empty, the component will
         // send a continue notification to the server to get more log records.  When a log record
         // is available, the client object's on_log_message() method will be called.
         ////////////////////////////////////////////////////////////
         void next_record();

         ////////////////////////////////////////////////////////////
         // next_batch
         //
         // Should be called by the client when it has processed all of the log messages in the
         // unread_records queue.  This will cause the component to remove those records and to send
         // a continue message to the server to get the next set of messages.  When one or more
         // messages are available, the component will invoke the client object's on_log_message()
         // method. 
         ////////////////////////////////////////////////////////////
         void next_batch();

         ////////////////////////////////////////////////////////////
         // class log_record_type
         ////////////////////////////////////////////////////////////
         class log_record_type
         {
         public:
            ////////////////////////////////////////////////////////////
            // stamp
            ////////////////////////////////////////////////////////////
            LgrDate stamp;

            ////////////////////////////////////////////////////////////
            // is_input
            ////////////////////////////////////////////////////////////
            bool is_input;

            /**
             * Specifies the data that was read.
             */
            StrBin data;

            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            log_record_type():
               is_input(false)
            { }

            ////////////////////////////////////////////////////////////
            // copy constructor
            ////////////////////////////////////////////////////////////
            log_record_type(log_record_type const &other):
               stamp(other.stamp),
               is_input(other.is_input),
               data(other.data)
            { }
         };

         ////////////////////////////////////////////////////////////
         // unread_begin
         ////////////////////////////////////////////////////////////
         typedef Csi::SharedPtr<log_record_type> log_record_handle;
         typedef std::list<log_record_handle> unread_records_type;
         typedef unread_records_type::iterator iterator;
         iterator unread_begin() { return unread_records.begin(); }

         ////////////////////////////////////////////////////////////
         // unread_end
         ////////////////////////////////////////////////////////////
         iterator unread_end() { return unread_records.end(); }

         ////////////////////////////////////////////////////////////
         // unread_size
         ////////////////////////////////////////////////////////////
         uint4 unread_size() const { return (uint4)unread_records.size(); }

         ////////////////////////////////////////////////////////////
         // unread_front
         ////////////////////////////////////////////////////////////
         log_record_handle const &unread_front() const
         { return unread_records.front(); }

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
         // on_log_advise_not
         ////////////////////////////////////////////////////////////
         void on_log_advise_not(Csi::Messaging::Message *message);

         ////////////////////////////////////////////////////////////
         // start_get_next_batch
         ////////////////////////////////////////////////////////////
         void start_get_next_batch();
         
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
            state_wait_for_server,
            state_wait_for_client
         } state;

         ////////////////////////////////////////////////////////////
         // unread_records
         //
         // The list of records that have not been read by the client yet.
         ////////////////////////////////////////////////////////////
         unread_records_type unread_records;

         ////////////////////////////////////////////////////////////
         // cached_records
         //
         // The list of records that have been read an can be used to read the next time we get the
         // data advise from the server.
         ////////////////////////////////////////////////////////////
         unread_records_type cached_records;

         ////////////////////////////////////////////////////////////
         // advise_tran
         ////////////////////////////////////////////////////////////
         uint4 advise_tran;
      };
   };
};


#endif

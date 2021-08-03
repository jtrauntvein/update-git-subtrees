/* Cora.LgrNet.LogQuery.h

   Copyright (C) 2001, 2020 Campbell Scientific, Inc.

   Written by: Andrew Mortenson
   Date Begun: Thursday 13 February 2020
   Last Change: Thursday 16 April 2020
   Last Commit: $Date: 2020-08-26 14:26:36 -0600 (Wed, 26 Aug 2020) $ 
   Committed by: $author: $

*/

#ifndef Cora_LgrNet_LogQuery_h
#define Cora_LgrNet_LogQuery_h
#include "Cora.ClientBase.h"
#include "Csi.Events.h"
#include <deque>


namespace Cora
{
   namespace LgrNet
   {
      // forward declarations
      class LogQuery;


      namespace LogQueryHelpers
      {
         /**
          * Defines the structure of a log record that has been received from the server.
          */
         struct LogRecord
         {
            /**
             * Specifies the time stamp of the log record.
             */
            Csi::LgrDate stamp;

            /**
             * Specifies the content of the log record formatted as comma-separated text.
             */
            StrAsc data;

            /**
             * Default Constructor
             */
            LogRecord()
            { }

            /**
             * Copy constructor
             */
            LogRecord(LogRecord const &other):
               stamp(other.stamp),
               data(other.data)
            { }

            /**
             * Copy operator
             */
            LogRecord &operator =(LogRecord const &other)
            {
               stamp = other.stamp;
               data = other.data;
               return *this;
            }
         };
      };

      
      class LogQueryClient: public Csi::InstanceValidator
      {
      public:
         /**
          * Called by the component when the LoggerNet transaction has been completed.
          *
          * @param sender Specifies the component calling this method.
          *
          * @param outcome Specifies the reason why the transaction has ended.
          */
         enum outcome_type
         {
            outcome_failure_unknown = 0,
            outcome_success = 1,
            outcome_failure_logon = 2,
            outcome_failure_session = 3,
            outcome_failure_unsupported = 4,
            outcome_failure_security = 5,
            outcome_failure_expression = 6,
            outcome_success_client_request = 7
         };
         virtual void on_complete(LogQuery *sender, outcome_type outcome) = 0;

         /**
          * Called to report one or more log records that have been received from the server.
          *
          * @return Must return true if the transaction is to continue.  Return false if the
          * transaction is to be aborted.
          * 
          * @param sender Specifies the component calling this method.
          *
          * @param records Specifies the records that have been received.  The component will recycle
          * these record objects when this method has returned so, if the client needs to maintain
          * them beyond that scope, it must make copies of the record objects or their content.
          */
         typedef LogQueryHelpers::LogRecord record_type;
         typedef Csi::SharedPtr<record_type> record_handle;
         typedef std::deque<record_handle> records_type;
         virtual bool on_records(LogQuery *sender, records_type const &records) = 0;
      };


      class LogQuery: public ClientBase, public Csi::EventReceiver
      {
      private:
         /**
          * Specifies the client to this component
          */
         LogQueryClient *client;
         
         /**
          * Specifies the log that will be queried.
          */
      public:
         enum log_id_type
         {
            log_transaction = 1,
            log_comms_status = 2,
            log_object_state = 3
         };
      private:
         log_id_type log_id;
         
         /**
          * Specifies the query expression
          */
         StrAsc expression;
         
         /**
          * Specifies the backfill time in milliseconds
          */
         uint4 backfill_interval;
         
         /**
          * Specifies the transaction number for the transaction
          */
         uint4 server_tran;
         
         /**
          * Specifies the date and time for which the query should begin.  If specified, no records
          * should be reported that are older than this value.
          */
         Csi::LgrDate begin;
         
         /**
          * Specifies the date and time on which the query should end.  If specified, no records
          * should be reported that are newer than this value.
          */
         Csi::LgrDate end;

         /**
          * Specifies the state of this transaction
          */
         enum state_type
         {
            state_standby,
            state_delegate,
            state_active
         } state;

         /**
          * Specifies the last set of log messages that were received from the server.
          */
         typedef LogQueryClient::record_type record_type;
         typedef LogQueryClient::record_handle record_handle;
         typedef LogQueryClient::records_type records_type;
         records_type records;

         /**
          * Specifies the cache of records to be used for future client notifications.
          */
         records_type cache;

         /**
          * Specifies the last sequence number received from the server.
          */
         int8 last_sequence;

      public:

         /**
          * Constructor
          */
         LogQuery():
            state(state_standby),
            begin(0),
            end(Int8_Max),
            backfill_interval(0),
            client(0),
            log_id(log_transaction),
            last_sequence(0)
         {}

         /**
          * Destructor
          */
         virtual ~LogQuery()
         {}

         /**
          * @param value Specifies the type of log that should be queried.
          */
         void set_log_id(log_id_type value)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            log_id = value;
         }
         
         /**
          * @return Returns the type of log being queried.
          */
         log_id_type get_log_id() const
         { return log_id; }
         
         /**
          * @param value Specifies the filter expression 
          */
         void set_expression(StrAsc const &value)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            expression = value;
         }
         
         /**
          * @return Returns the filter expression.
          */
         StrAsc const &get_expresssion() const
         { return expression; }

         /**
          * @param value Specifies the backfill interval that will be applied to the query as
          * positive milliseconds.
          */
         void set_backfill_interval(uint4 value)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            backfill_interval = value;
         }
         
         /**
          * @return Returns the backfill interval in milliseconds.
          */
         uint4 get_backfill_interval() const
         { return backfill_interval; }

         /**
          * @param value Specifies the begin date/time value for the query.  If not specified, all
          * messages from the beginning of the log to the end will be returned.
          */
         void set_begin(Csi::LgrDate const &value)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            begin = value;
         }

         /**
          * @return Returns the begin date/time for the query.
          */
         Csi::LgrDate const &get_begin() const
         { return begin; }
         
         /**
          * @param value Specifies the end date/time value for the query.  If not specified all
          * messages from the beginning will be returned.
          */
         void set_end(Csi::LgrDate const &value)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            end = value;
         }
         
         /**
          * @return Returns the date/time where the query will end.
          */
         Csi::LgrDate const &get_end() const
         { return end; }
         

         /**
         * Called to start this component using a newly created router
         *
         * @param client_ Specifies the client for this component.
         *
         * @param router Specfies a newly created (unattached) messaging router.
         *
         * @param other_component Specifies a component that is already connected to loggernet and
         * from which this component can borrow the connection.
         */
         typedef LogQueryClient client_type;
         void start(client_type *client_, router_handle router)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("invalid client pointer");
            client = client_;
            state = state_delegate;
            ClientBase::start(router);
         }
         void start(client_type *client_, ClientBase *other_component)
         {
            if(state != state_standby)
               throw exc_invalid_state();
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("invalid client pointer");
            client = client_;
            state = state_delegate;
            ClientBase::start(other_component);
         }

         /**
         * Cancels the server transaction.
         */
         virtual void finish()
         {
            records.clear();
            cache.clear();
            state = state_standby;
            client = 0;
            ClientBase::finish();
         }

         /**
          * Overrides the base class version to handle asynch events.
          */
         virtual void receive(event_handle &ev) override;

         /**
          * Writes a description of the specified outcome to the given stream.
          *
          * @param out Specifies the stream to which the failure will be formatted.
          *
          * @param failure Specifies the failure to format.
          */
         static void format_outcome(std::ostream &out, client_type::outcome_type outcome);

         /**
          * Overrides the base class version to handle the notification that the cloned session is
          * ready to proceed.
          */
         virtual void on_corabase_ready() override;

         /**
          * Overrides the base class version to handle a failure notification.
          */
         virtual void on_corabase_failure(corabase_failure_type failure) override;

         /**
          * Overrides the base class version to handle an incoming message.
          */
         virtual void onNetMessage(Csi::Messaging::Router *router, Csi::Messaging::Message *message);

      private:
         /**
          * Handles an incoming log query notification.
          */
         void on_log_query_not(Csi::Messaging::Message *msg);

         /**
          * Handles an incoming stopped notification.
          */
         void on_log_query_stopped_not(Csi::Messaging::Message *msg);

         /**
          * Sends the next acknowledgement to the server.
          *
          * @param cont Specifies if the transaction should continue.
          */
         void send_continue_cmd(bool cont = true);

      };
   };
};


#endif // !Cora_LgrNet_LogQuery_h



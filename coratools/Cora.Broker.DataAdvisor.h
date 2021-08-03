/* Cora.Broker.DataAdvisor.h

   Copyright (C) 2000, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 17 April 2000
   Last Change: Friday 29 April 2016
   Last Commit: $Date: 2016-05-03 16:18:03 -0600 (Tue, 03 May 2016) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Cora_Broker_DataAdvisor_h
#define Cora_Broker_DataAdvisor_h

#include "Csi.SharedPtr.h"
#include "StrUni.h"
#include "Cora.Broker.Record.h"
#include "CsiEvents.h"
#include "Cora.Broker.BrokerBase.h"
#include "Csi.InstanceValidator.h"
#include "Cora.Broker.TableDataIndexGetter.h"
#include <list>
#include <deque>


namespace Cora
{
   namespace Broker
   {
      //@group class forward declarations
      class DataAdvisor;
      //@endgroup

      ////////////////////////////////////////////////////////////
      // class DataAdvisorClient
      //
      // Defines the methods that a data advise transaction client object
      // needs to implement.
      ////////////////////////////////////////////////////////////
      class DataAdvisorClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_advise_ready
         //
         // Called when the advise transaction has been successfully begun
         ////////////////////////////////////////////////////////////
         virtual void on_advise_ready(
            DataAdvisor *tran)
         { }

         ////////////////////////////////////////////////////////////
         // on_advise_failure
         //
         // Called when a failure has occurred at some point in the advise
         // transaction
         ////////////////////////////////////////////////////////////
         enum failure_type
         {
            failure_unknown = 0,
            failure_connection_failed = 1,
            failure_invalid_logon = 2,
            failure_invalid_station_name = 3,
            failure_invalid_table_name = 4,
            failure_server_security = 5,
            failure_invalid_start_option = 6,
            failure_invalid_order_option = 7,
            failure_table_deleted = 8,
            failure_station_shut_down = 9,
            failure_unsupported = 10,
            failure_invalid_column_name = 11,
            failure_invalid_array_address = 12,
         };
         virtual void on_advise_failure(
            DataAdvisor *tran,
            failure_type failure = failure_unknown) = 0;

         ////////////////////////////////////////////////////////////
         // on_advise_record
         // 
         // Called when a new data record has been received
         ////////////////////////////////////////////////////////////
         virtual void on_advise_record(
            DataAdvisor *tran)
         { } 
      };


      ////////////////////////////////////////////////////////////
      // class DataAdvisor
      //
      // Defines a class that wraps around a data advise transaction and
      // the records that it produces.
      //
      // In order to use this class, an application must provide a client
      // object that can receive notifications of significant events
      // regarding the progress of the data advise transaction. The
      // application is also responsible for setting the appropriate
      // properties before the transaction is started (via the start
      // method). The application is also responsible for providing a
      // created router object.
      //
      // Typically, this class will be used in the following sequence:
      //   - The application creates an instance of this class
      //
      //   - The application sets the appropriate properties using the
      //     set_XXX() methods.
      // 
      //   - The application invokes the start() method
      //
      //   - This object will invoke the client's on_advise_ready() method
      //     when the advise operation has been started.
      //
      //   - When a record arrives from the server, the client's
      //     on_advise_record() method will be invoked. The application can
      //     then iterate through record values using the begin() and end()
      //     methods. It can also access record attributes such as time
      //     stamp, file mark number, and record number.
      //
      //   - When the application is finished with the current record, it
      //     can invoke get_next_record() which will cause this object to
      //     wait for the next record that becomes available from the server
      //     at which point, this object will again invoke the client's
      //     on_advise_record() method.
      //
      // This process will continue until this object is destroyed or an
      // error occurs. The application will be notified when an error
      // occurs when this object invokes the client's on_advise_failure()
      // method.
      ////////////////////////////////////////////////////////////
      class DataAdvisor:
         public BrokerBase,
         public Csi::EvReceiver,
         private TableDataIndexGetterClient
      {
      public:
         enum start_option_type
         {
            start_at_record_id = 1,
            start_at_time_stamp = 2,
            start_at_newest = 3,
            start_after_newest = 4,
            start_relative_to_newest = 5,
            start_at_offset_from_newest = 6,
         };
      
         enum order_option_type
         {
            order_collected = 1,
            order_logged_with_holes = 2,
            order_logged_without_holes = 3,
            order_real_time = 4
         };

         typedef Csi::SharedPtr<Cora::Broker::Record> record_handle;

      private:
         //@group properties
         //@group start up options
         ////////////////////////////////////////////////////////////
         // table_name
         ////////////////////////////////////////////////////////////
         StrUni table_name;

         ////////////////////////////////////////////////////////////
         // start_option
         ////////////////////////////////////////////////////////////
         start_option_type start_option;

         ////////// order_option
         ////////////////////////////////////////////////////////////
         // order_option
         ////////////////////////////////////////////////////////////
         order_option_type order_option;

         ////////// start_record_no
         ////////////////////////////////////////////////////////////
         // start_record_no
         //
         // Specifies the starting record number when a starting option of
         // start_at_record_id is used
         ////////////////////////////////////////////////////////////
         uint4 start_record_no;

         ////////////////////////////////////////////////////////////
         // start_file_mark_no
         //
         // Specifies the starting file mark number when a starting option
         // of start_at_record_id is used
         ////////////////////////////////////////////////////////////
         uint4 start_file_mark_no;

         ////////////////////////////////////////////////////////////
         // start_date
         //
         // Specifies the starting date when a starting option of
         // start_at_date is used
         ////////////////////////////////////////////////////////////
         int8 start_date;

         ////////////////////////////////////////////////////////////
         // start_record_offset
         //
         // Controls the number of records behind the newest record in the
         // table that the advise operation will start on when the value of
         // start_option is set as start_at_offset_from_newest.
         ////////////////////////////////////////////////////////////
         uint4 start_record_offset;

         ////////////////////////////////////////////////////////////
         // start_interval
         //
         // Specifies the starting interval when a starting option of
         // start_relative_to_newest is used
         ////////////////////////////////////////////////////////////
         int8 start_interval;

         ////////////////////////////////////////////////////////////
         // value_factory
         //
         // Specifies what type of value objects will be created for the
         // record.
         ////////////////////////////////////////////////////////////
         Csi::SharedPtr<ValueFactory> value_factory;
         //@endgroup

         ////////////////////////////////////////////////////////////
         // cache_size_controller
         //
         // Specifies the most records that will be requested from the
         // server in any single notification and therefore controls how
         // many records this component is allowed to buffer.
         //
         // The default value for this property is 1 records which should
         // give the same behaviour that existed before the record cache
         // was introduced.
         ////////////////////////////////////////////////////////////
         uint4 cache_size_controller;

         ////////////////////////////////////////////////////////////
         // selectors
         //
         // Specifies a list of strings that the client uses to select
         // specific columns and/or values from the server cache table.
         // This property is modified by clear_columns() and
         // add_column(). 
         ////////////////////////////////////////////////////////////
         typedef std::list<StrUni> selectors_type;
         selectors_type selectors;
         //@endgroup

      public:
         ////////////////////////////////////////////////////////////
         // constructor
         //
         // Sets the start up options to default values but does not
         // perform any action until the start() method is invoked. This
         // will give the client an opportunity to adjust desired
         // properties before the transaction is started.
         ////////////////////////////////////////////////////////////
         DataAdvisor();

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~DataAdvisor();

         //@group starting option properties set methods
         ////////////////////////////////////////////////////////////
         // set_table_name
         ////////////////////////////////////////////////////////////
         void set_table_name(StrUni const &table_name_);

         ////////////////////////////////////////////////////////////
         // set_start_option
         ////////////////////////////////////////////////////////////
         void set_start_option(start_option_type start_option_);

         ////////////////////////////////////////////////////////////
         // set_order_option
         ////////////////////////////////////////////////////////////
         void set_order_option(order_option_type order_option_);

         ////////////////////////////////////////////////////////////
         // set_start_record_no
         ////////////////////////////////////////////////////////////
         void set_start_record_no(uint4 start_record_no_);

         ////////////////////////////////////////////////////////////
         // set_start_file_mark_no
         ////////////////////////////////////////////////////////////
         void set_start_file_mark_no(uint4 start_file_mark_no_);

         ////////////////////////////////////////////////////////////
         // set_start_date
         ////////////////////////////////////////////////////////////
         void set_start_date(int8 start_date_);

         ////////////////////////////////////////////////////////////
         // set_start_interval
         ////////////////////////////////////////////////////////////
         void set_start_interval(int8 start_interval_);

         ////////////////////////////////////////////////////////////
         // set_value_factory
         ////////////////////////////////////////////////////////////
         void set_value_factory(Csi::SharedPtr<ValueFactory> &value_factory_);

         ////////////////////////////////////////////////////////////
         // set_cache_size_controller
         ////////////////////////////////////////////////////////////
         void set_cache_size_controller(uint4 cache_size_controller_);

         ////////////////////////////////////////////////////////////
         // set_start_record_offset
         ////////////////////////////////////////////////////////////
         void set_start_record_offset(uint4 start_record_offset_);

         ////////////////////////////////////////////////////////////
         // clear_columns
         //
         // Clears the list of column selectors so that, when start() is
         // called, the entire table will be selected. 
         ////////////////////////////////////////////////////////////
         void clear_columns();

         ////////////////////////////////////////////////////////////
         // add_column
         //
         // Adds the specified column name to the list of selectors.  If
         // this is called, the component will ask the server only for
         // those values that have been requested through calls to this
         // method.  If that list is empty, the response record will
         // consist of all of the values in the server cache table.
         //
         // The string specified should conform to the following syntax:
         //
         //  selector := column-name ["(" subscript { "," subscript } ")"].
         //  column-name := string.
         //  subscript := integer.
         //
         // If the table column being specified is an array and the
         // application does not specify the subscript list for that
         // selector, the server will select the entire column.  If the
         // application specifies the subscript list for a column, the
         // number of subscripts specified should match the number of array
         // dimensions defined for that column in the server's table
         // definitions.  
         ////////////////////////////////////////////////////////////
         void add_column(StrUni const &selector);
         //@endgroup

         //@group starting option properties access methods
         ////////////////////////////////////////////////////////////
         // get_table_name
         ////////////////////////////////////////////////////////////
         StrUni const &get_table_name() const { return table_name; }

         ////////////////////////////////////////////////////////////
         // get_start_option
         ////////////////////////////////////////////////////////////
         start_option_type get_start_option() { return start_option; }

         ////////////////////////////////////////////////////////////
         // get_order_option
         ////////////////////////////////////////////////////////////
         order_option_type get_order_option() const { return order_option; }

         ////////////////////////////////////////////////////////////
         // get_start_record_no
         ////////////////////////////////////////////////////////////
         uint4 get_start_record_no() const { return start_record_no; }

         ////////////////////////////////////////////////////////////
         // get_start_file_mark_no
         ////////////////////////////////////////////////////////////
         uint4 get_start_file_mark_no() const { return start_file_mark_no; } 

         ////////////////////////////////////////////////////////////
         // get_start_date
         ////////////////////////////////////////////////////////////
         int8 get_start_date() const { return start_date; }

         ////////////////////////////////////////////////////////////
         // set_start_interval
         ////////////////////////////////////////////////////////////
         int8 get_start_interval() const { return start_interval; }

         ////////////////////////////////////////////////////////////
         // get_value_factory
         ////////////////////////////////////////////////////////////
         Csi::SharedPtr<ValueFactory> &get_value_factory() { return value_factory; }

         ////////////////////////////////////////////////////////////
         // get_cache_size_controller
         ////////////////////////////////////////////////////////////
         uint4 get_cache_size_controller() const { return cache_size_controller; }

         ////////////////////////////////////////////////////////////
         // get_start_record_offset
         ////////////////////////////////////////////////////////////
         uint4 get_start_record_offset() const { return start_record_offset; }
         //@endgroup

         ////////////////////////////////////////////////////////////
         // start
         //
         // Responsible for starting the transaction. Returns false if a
         // required parameter has not been specified.
         //
         // The caller is responsible for providing an allocated router and
         // can optionally take responsibility for logon, etc. If the
         // default_net_session parameter is set to zero, this class will
         // assume that the router is newly allocated and engage in the
         // logon procedure before starting the data advise, otherwise,
         // this class will clone the net session and, bypassing the logon,
         // will attach to the specified data broker and start the data
         // advise transaction.
         ////////////////////////////////////////////////////////////
         typedef DataAdvisorClient client_type;
         void start(
            client_type *client,
            router_handle &router);
         void start(
            client_type *client,
            ClientBase *other_client);        

         ////////////////////////////////////////////////////////////
         // finish
         //
         // Places the transaction in an inactive state and closes any
         // connections (and associated server resources)
         ////////////////////////////////////////////////////////////
         void finish();

         ////////////////////////////////////////////////////////////
         // format_failure
         ////////////////////////////////////////////////////////////
         static void format_failure(
            std::ostream &out, client_type::failure_type failure);

         ////////////////////////////////////////////////////////////
         // get_record
         //
         // Returns the record object
         ////////////////////////////////////////////////////////////
         record_handle &get_record();

         ////////////////////////////////////////////////////////////
         // format_record_for_ldep
         //
         // Formats the current record (if any) to the specified stream
         // using the new LDEP syntax (comma separated values)
         ////////////////////////////////////////////////////////////
         void format_record_ldep(std::ostream &out);

         ////////////////////////////////////////////////////////////
         // format_record_ldep_xml
         ////////////////////////////////////////////////////////////
         void format_record_ldep_xml(std::ostream &out);

         ////////////////////////////////////////////////////////////
         // get_next_record
         //
         // Sets up the conditions to receive the next record.
         ////////////////////////////////////////////////////////////
         void get_next_record();

         //@group block access methods

         // These methods and declarations can be used by the client to
         // access whole blocks of records rather than dealing with records
         // singly.  The advantage of this is to provide greater efficiency
         // in the transfer of data from the component to the client.  Care
         // should be taken by the application not to mix the use of these
         // mechanisms with get_next_record().

         ////////////////////////////////////////////////////////////
         // unread_begin
         //
         // Returns the iterator to the first element in the unread records
         // queue.  This is defined in both const and non-const versions
         ////////////////////////////////////////////////////////////
         typedef std::deque<record_handle> unread_records_type;
         typedef unread_records_type::iterator iterator;
         typedef unread_records_type::const_iterator const_iterator;
         typedef unread_records_type::reverse_iterator reverse_iterator;
         iterator unread_begin() { return unread_records.begin(); }
         const_iterator unread_begin() const { return unread_records.begin(); }
         reverse_iterator unread_rbegin() { return unread_records.rbegin(); }

         ////////////////////////////////////////////////////////////
         // unread_end
         //
         // Returns the iterator at the end of the unread records queue.
         // This is defined in both const and non-const versions
         ////////////////////////////////////////////////////////////
         iterator unread_end() { return unread_records.end(); }
         const_iterator unread_end() const { return unread_records.end(); }
         reverse_iterator unread_rend() { return unread_records.rend(); }

         ////////////////////////////////////////////////////////////
         // get_unread_records
         ////////////////////////////////////////////////////////////
         unread_records_type &get_unread_records(){return unread_records;}

         ////////////////////////////////////////////////////////////
         // unread_size
         //
         // Returns the number of records in the unread records queue.
         ////////////////////////////////////////////////////////////
         size_t unread_size() const
         { return unread_records.size(); }

         ////////////////////////////////////////////////////////////
         // unread_empty
         //
         // Returns whether the unread records queue is empty.
         ////////////////////////////////////////////////////////////
         bool unread_empty() const { return unread_records.empty(); }

         ////////////////////////////////////////////////////////////
         // get_next_block
         //
         // Prepares the component to get the next block by moving all of
         // the records in the unread queue to the recycled queue and then
         // invoking get_next_record() to prepare the server transaction to
         // send more records.
         ////////////////////////////////////////////////////////////
         void get_next_block();
         //@endgroup

      protected:
         //@group methods overloaded from class BrokerBase
         ////////////////////////////////////////////////////////////
         // onNetMessage
         ////////////////////////////////////////////////////////////
         virtual void onNetMessage(
            Csi::Messaging::Router *rtr,
            Csi::Messaging::Message *msg);

         ////////////////////////////////////////////////////////////
         // on_brokerbase_ready
         ////////////////////////////////////////////////////////////
         virtual void on_brokerbase_ready();

         ////////////////////////////////////////////////////////////
         // on_brokerbase_failure
         ////////////////////////////////////////////////////////////
         virtual void on_brokerbase_failure(brokerbase_failure_type failure);

         ////////////////////////////////////////////////////////////
         // on_brokerbase_session_failure
         ////////////////////////////////////////////////////////////
         virtual void on_brokerbase_session_failure();
         //@endgroup

         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

      private:
         ////////////////////////////////////////////////////////////
         // on_data_advise_start_ack
         ////////////////////////////////////////////////////////////
         void on_data_advise_start_ack(Csi::Messaging::Message *msg);

         ////////////////////////////////////////////////////////////
         // on_data_advise_start_ack_ex
         ////////////////////////////////////////////////////////////
         void on_data_advise_start_ack_ex(Csi::Messaging::Message *message);

         ////////////////////////////////////////////////////////////
         // on_data_advise_ack
         ////////////////////////////////////////////////////////////
         void on_data_advise_not(Csi::Messaging::Message *msg);

         ////////////////////////////////////////////////////////////
         // send_continue_command
         //
         // Forms and sends the data advise continue command.
         ////////////////////////////////////////////////////////////
         void send_continue_command();

         ////////////////////////////////////////////////////////////
         // on_complete
         //
         // Overloads the class TableDataIndexGetterClient's version of the
         // method to handle the acknowledgement.
         ////////////////////////////////////////////////////////////
         virtual void on_complete(
            TableDataIndexGetter *index_getter,
            outcome_type outcome,
            index_records_type const &index_records);

         ////////////////////////////////////////////////////////////
         // start_advise_transaction
         ////////////////////////////////////////////////////////////
         void start_advise_transaction(start_option_type real_start_option);

      private:
         ////////////////////////////////////////////////////////////
         // client
         ////////////////////////////////////////////////////////////
         DataAdvisorClient *client;

         ////////////////////////////////////////////////////////////
         // advise_tran
         ////////////////////////////////////////////////////////////
         uint4 advise_tran;

         ////////////////////////////////////////////////////////////
         // state
         ////////////////////////////////////////////////////////////
         enum state_type
         {
            state_standby,
            state_delegate,
            state_before_active,
            state_active,
         } state;

         ////////////////////////////////////////////////////////////
         // unread_records
         //
         // The set of cached records that have not yet been processed by
         // the client
         ////////////////////////////////////////////////////////////
         unread_records_type unread_records;

         ////////////////////////////////////////////////////////////
         // recycled_records
         //
         // The set of record objects that have already been used and can
         // be recycled. This list will be checked before a new record
         // object is allocated. If there is a record in the list, that
         // element will be popped off and used rather than allocating a
         // new record.
         ////////////////////////////////////////////////////////////
         typedef std::deque<record_handle> recycled_records_type;
         recycled_records_type recycled_records;

         ////////////////////////////////////////////////////////////
         // report_new_records
         //
         // Set to true if new records should be reported to the client
         // when a data advise notification is received.
         ////////////////////////////////////////////////////////////
         bool report_new_records;

         ////////////////////////////////////////////////////////////
         // index_getter
         //
         // Reference to the index getter object that is used to support
         // start options like start at record number offset. This
         // reference will only be valid when needed.
         ////////////////////////////////////////////////////////////
         Csi::SharedPtr<TableDataIndexGetter> index_getter;

         ////////////////////////////////////////////////////////////
         // current_record
         //
         // Keeps track of the current record and also serves as a
         // prototype for creating new records.  This object is allocated
         // when the acknowledgement is received and can be acccessed by
         // the client using get_record().
         ////////////////////////////////////////////////////////////
         record_handle current_record;

         ////////////////////////////////////////////////////////////
         // actual_start_file_mark
         ////////////////////////////////////////////////////////////
         uint4 actual_start_file_mark;

         ////////////////////////////////////////////////////////////
         // actual_start_record_no
         ////////////////////////////////////////////////////////////
         uint4 actual_start_record_no;
      };
   };
};

#endif

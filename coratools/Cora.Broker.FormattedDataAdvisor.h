/* Cora.Broker.FormattedDataAdvisor.h

   Copyright (C) 2002, 2013 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 02 January 2002
   Last Change: Friday 18 January 2013
   Last Commit: $Date: 2013-01-18 13:34:50 -0600 (Fri, 18 Jan 2013) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Cora_Broker_FormattedDataAdvisor_h
#define Cora_Broker_FormattedDataAdvisor_h

#include "Cora.Broker.BrokerBase.h"
#include "Csi.Events.h"
#include "Csi.InstanceValidator.h"
#include <list>


namespace Cora
{
   namespace Broker
   {
      //@group class forward declarations
      class FormattedDataAdvisor;
      class DataAdvisor;
      //@endgroup


      ////////////////////////////////////////////////////////////
      // class FormattedDataAdvisorClient
      ////////////////////////////////////////////////////////////
      class FormattedDataAdvisorClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_started
         //
         // Called when the advise operation has been started.  The data_header
         // holds the contents of the data file header and the data_footer hold
         // the contents of the data file footer.  It is up to the application
         // to remember and/or use them.
         ////////////////////////////////////////////////////////////
         virtual void on_started(
            FormattedDataAdvisor *advisor,
            StrAsc const &data_header,
            StrAsc const &data_footer)
         { }

         ////////////////////////////////////////////////////////////
         // on_failure
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
            failure_invalid_format_option = 8,
            failure_table_deleted = 9,
            failure_station_shut_down = 10,\
            failure_unsupported = 11,
         };
         virtual void on_failure(
            FormattedDataAdvisor *advisor,
            failure_type failure) = 0;

         ////////////////////////////////////////////////////////////
         // on_records
         //
         // Called when one or more records have been received from the
         // server. The records can be retrieved from the advisor by using its
         // begin() and end() methods to iterate through the set of records.
         //
         // The advisor will not get more records until the application calls
         // its continue_advise() method.
         ////////////////////////////////////////////////////////////
         virtual void on_records(
            FormattedDataAdvisor *advisor)
         { }
      };


      ////////////////////////////////////////////////////////////
      // class FormattedDataRecord
      //
      // Represents a data record that is used by class FormattedDataAdvisor.
      ////////////////////////////////////////////////////////////
      class FormattedDataRecord
      {
      private:
         ////////////////////////////////////////////////////////////
         // station_name
         ////////////////////////////////////////////////////////////
         StrUni station_name;

         ////////////////////////////////////////////////////////////
         // table_name
         ////////////////////////////////////////////////////////////
         StrUni table_name;

         ////////////////////////////////////////////////////////////
         // file_mark_no
         ////////////////////////////////////////////////////////////
         uint4 file_mark_no;

         ////////////////////////////////////////////////////////////
         // record_no
         ////////////////////////////////////////////////////////////
         uint4 record_no;

         ////////////////////////////////////////////////////////////
         // stamp
         ////////////////////////////////////////////////////////////
         int8 stamp;

         ////////////////////////////////////////////////////////////
         // formatted_data
         ////////////////////////////////////////////////////////////
         StrBin formatted_data;
         
      public:
         ////////////////////////////////////////////////////////////
         // default constructor
         ////////////////////////////////////////////////////////////
         FormattedDataRecord();

         ////////////////////////////////////////////////////////////
         // copy constructor
         ////////////////////////////////////////////////////////////
         FormattedDataRecord(FormattedDataRecord const &other);

         ////////////////////////////////////////////////////////////
         // copy operator
         ////////////////////////////////////////////////////////////
         FormattedDataRecord &operator =(FormattedDataRecord const &other);

         ////////////////////////////////////////////////////////////
         // read
         //
         // Initialises this object from the parameters and the contents of the message. 
         ////////////////////////////////////////////////////////////
         bool read(
            StrUni const &station_name_,
            StrUni const &table_name_,
            Csi::Messaging::Message *in);

         ////////////////////////////////////////////////////////////
         // get_station_name
         ////////////////////////////////////////////////////////////
         StrUni const &get_station_name() const { return station_name; }

         ////////////////////////////////////////////////////////////
         // get_table_name
         ////////////////////////////////////////////////////////////
         StrUni const &get_table_name() const { return table_name; }

         ////////////////////////////////////////////////////////////
         // get_file_mark_no
         ////////////////////////////////////////////////////////////
         uint4 get_file_mark_no() const { return file_mark_no; }

         ////////////////////////////////////////////////////////////
         // get_record_no
         ////////////////////////////////////////////////////////////
         uint4 get_record_no() const { return record_no; }

         ////////////////////////////////////////////////////////////
         // get_stamp
         ////////////////////////////////////////////////////////////
         int8 get_stamp() const { return stamp; }

         ////////////////////////////////////////////////////////////
         // get_formatted_data
         ////////////////////////////////////////////////////////////
         StrBin const &get_formatted_data() const { return formatted_data; }
      };

      
      ////////////////////////////////////////////////////////////
      // class FormattedDataAdvisor
      //
      // Defines a component that takes advantage of the server's formatted
      // data advise transaction.
      //
      // In order to use this component, an application must provide an object
      // that is derived from class FormattedDataAdvisorClient.  This object
      // will receive the event notifications from the advisor component.  The
      // following sequence of events is typical usage for this component:
      //
      //   - The application creates an instance of this class.
      //
      //   - The application sets the appropriate properties in that instance
      //   by calling the set_xxx() methods defined in this class as well as
      //   its base classes.
      //
      //   - The application invokes the appropriate start() method with a
      //   valid reference to a FormattedDataAdvisorClient derived object.
      //
      //   - When the component recieves the appropriate notifications from the
      //   server that the transaction has been started, the client's
      //   on_started() method will be invoked. Alternatively, the client's
      //   on_failure() method can be invoked if the transaction could not be
      //   started.
      //
      //   - When the component receives records from the server, it will call
      //   the client's on_records() method.  The application can then use the
      //   component's begin(), and end() methods to iterate through the list
      //   of available records.  No further records will be received by the
      //   component until the application invokes its continue_advise()
      //   method. After that, the application should not attempt to access the
      //   record list until the next time on_records() is called.
      //
      // The component will continue to operate in this fashion until an error
      // occurs or the component is stopped (calling finish()) or deleted.  The
      // application will be notified of any errors that couild occur to stop
      // the component through the client's on_failure() method.
      ////////////////////////////////////////////////////////////
      class FormattedDataAdvisor:
         public BrokerBase,
         public Csi::EventReceiver
      {
      public:
         ////////////////////////////////////////////////////////////
         // enum start_option_type
         ////////////////////////////////////////////////////////////
         enum start_option_type
         {
            start_at_record_id = 1,
            start_at_time_stamp = 2,
            start_at_newest = 3,
            start_after_newest = 4,
            start_relative_to_newest = 5,
            start_at_offset_from_newest = 6,
         };
      

         ////////////////////////////////////////////////////////////
         // enum order_option_type
         ////////////////////////////////////////////////////////////
         enum order_option_type
         {
            order_collected = 1,
            order_logged_with_holes = 2,
            order_logged_without_holes = 3,
            order_real_time = 4
         };


         ////////////////////////////////////////////////////////////
         // enum format_option_type
         ////////////////////////////////////////////////////////////
         enum format_option_type
         {
            format_toaci1 = 1,
            format_toa5 = 2,
            format_tob1 = 3,
            format_ldxp = 4,
            format_custom_csv = 5,
            format_csixml = 6,
            format_noh = 7
         };


      private:
         //@group properties
         ////////////////////////////////////////////////////////////
         // table_name
         ////////////////////////////////////////////////////////////
         StrUni table_name;

         ////////////////////////////////////////////////////////////
         // start_option
         ////////////////////////////////////////////////////////////
         start_option_type start_option;

         ////////////////////////////////////////////////////////////
         // order_option
         ////////////////////////////////////////////////////////////
         order_option_type order_option;

         ////////////////////////////////////////////////////////////
         // format_option
         ////////////////////////////////////////////////////////////
         format_option_type format_option;

         ////////////////////////////////////////////////////////////
         // format_option_flags
         ////////////////////////////////////////////////////////////
         uint4 format_option_flags;

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
         // Specifies the starting file mark number when a starting option of
         // start_at_record_id is used
         ////////////////////////////////////////////////////////////
         uint4 start_file_mark_no;

         ////////////////////////////////////////////////////////////
         // start_date
         //
         // Specifies the starting date when a starting option of start_at_date
         // is used
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
         // cache_size_controller
         //
         // Specifies the most records that will be requested from the server
         // in any single notification and therefore controls how many records
         // this component is allowed to buffer.
         //
         // The default value for this property is 1 records which should give
         // the same behaviour that existed before the record cache was
         // introduced.
         ////////////////////////////////////////////////////////////
         uint4 cache_size_controller;

         ////////////////////////////////////////////////////////////
         // reported_station_name
         //
         // Specifies the default station name that the server should use when
         // the data and/or header is formatted.
         ////////////////////////////////////////////////////////////
         StrUni reported_station_name;

         ////////////////////////////////////////////////////////////
         // reported_table_name
         //
         // Specifies the default table name that the server should use when
         // the data and/or header is formatted.
         ////////////////////////////////////////////////////////////
         StrUni reported_table_name;
         //@endgroup

      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         FormattedDataAdvisor();

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~FormattedDataAdvisor();

         //@group properties set methods
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
         // set_format_option
         //
         // Sets the format option and the default option flags for that
         // option.  
         ////////////////////////////////////////////////////////////
         void set_format_option(format_option_type format_option_);

         ////////////////////////////////////////////////////////////
         // set_format_option_flags
         //
         // Sets the flags for the format option.
         ////////////////////////////////////////////////////////////
         void set_format_option_flags(uint4 format_option_flags_);

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
         // set_cache_size_controller
         ////////////////////////////////////////////////////////////
         void set_cache_size_controller(uint4 cache_size_controller_);

         ////////////////////////////////////////////////////////////
         // set_start_record_offset
         ////////////////////////////////////////////////////////////
         void set_start_record_offset(uint4 start_record_offset_);

         ////////////////////////////////////////////////////////////
         // set_reported_station_name
         ////////////////////////////////////////////////////////////
         void set_reported_station_name(StrUni const &reported_station_name_);

         ////////////////////////////////////////////////////////////
         // set_reported_table_name
         ////////////////////////////////////////////////////////////
         void set_reported_table_name(StrUni const &reported_table_name_);

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


         //@group properties access methods
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
         // get_format_option
         ////////////////////////////////////////////////////////////
         format_option_type get_format_option() const { return format_option; }

         ////////////////////////////////////////////////////////////
         // get_format_option_flags
         ////////////////////////////////////////////////////////////
         uint4 get_format_option_flags() const
         { return format_option_flags; }

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
         // get_cache_size_controller
         ////////////////////////////////////////////////////////////
         uint4 get_cache_size_controller() const { return cache_size_controller; }

         ////////////////////////////////////////////////////////////
         // get_start_record_offset
         ////////////////////////////////////////////////////////////
         uint4 get_start_record_offset() const { return start_record_offset; }

         ////////////////////////////////////////////////////////////
         // get_reported_station_name
         ////////////////////////////////////////////////////////////
         StrUni const &get_reported_station_name() const { return reported_station_name; }

         ////////////////////////////////////////////////////////////
         // get_reported_table_name
         ////////////////////////////////////////////////////////////
         StrUni const &get_reported_table_name() const { return reported_table_name; }
         //@endgroup


         ////////////////////////////////////////////////////////////
         // start
         //
         // Should be called by the application to initiate the transaction
         // with the server.  There are two different versions defined. The
         // first version should be used by the application when it has a newly
         // created router object. The second should be used when the
         // application needs this component to share the router and other
         // information with another component that is in an active state.
         ////////////////////////////////////////////////////////////
         typedef FormattedDataAdvisorClient client_type;
         void start(
            client_type *client_,
            router_handle &router);
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

         //@group current record set access methods
         
         // These methods and typedefs are used to allow the application access
         // to the set of records that are cached by this component while it is
         // in the state where records are available to be read.  These methods
         // should not be invoked outside of this state. If the application
         // attempts to do so, an exc_invalid_state exception will be thrown.
         typedef FormattedDataRecord value_type;
         typedef std::list<value_type> records_type;
         typedef records_type::const_iterator const_iterator;
         typedef records_type::size_type size_type;
         
         ////////////////////////////////////////////////////////////
         // begin
         ////////////////////////////////////////////////////////////
         const_iterator begin() const
         { return records.begin(); }

         ////////////////////////////////////////////////////////////
         // end
         ////////////////////////////////////////////////////////////
         const_iterator end() const
         { return records.end(); }

         ////////////////////////////////////////////////////////////
         // size
         ////////////////////////////////////////////////////////////
         size_type size() const
         { return records.size(); }

         ////////////////////////////////////////////////////////////
         // empty
         ////////////////////////////////////////////////////////////
         bool empty() const
         { return records.empty(); }

         ////////////////////////////////////////////////////////////
         // front
         ////////////////////////////////////////////////////////////
         value_type const &front() const
         { return records.front(); }

         ////////////////////////////////////////////////////////////
         // back
         ////////////////////////////////////////////////////////////
         value_type const &back() const
         { return records.back(); }
         //@endgroup

         ////////////////////////////////////////////////////////////
         // continue_advise
         ////////////////////////////////////////////////////////////
         void continue_advise();


      protected:
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
         
         ////////////////////////////////////////////////////////////
         // onNetMessage
         ////////////////////////////////////////////////////////////
         virtual void onNetMessage(
            Csi::Messaging::Router *rtr,
            Csi::Messaging::Message *msg);

         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);


      private:
         ////////////////////////////////////////////////////////////
         // on_start_ack
         ////////////////////////////////////////////////////////////
         void on_start_ack(Csi::Messaging::Message *msg);

         ////////////////////////////////////////////////////////////
         // on_data_not
         ////////////////////////////////////////////////////////////
         void on_data_not(Csi::Messaging::Message *msg);
         
         ////////////////////////////////////////////////////////////
         // on_stopped_not
         ////////////////////////////////////////////////////////////
         void on_stopped_not(Csi::Messaging::Message *msg);


      private:
         ////////////////////////////////////////////////////////////
         // client
         ////////////////////////////////////////////////////////////
         FormattedDataAdvisorClient *client;

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
            state_wait_for_records,
            state_wait_for_continue,
         } state;

         ////////////////////////////////////////////////////////////
         // records
         ////////////////////////////////////////////////////////////
         records_type records;

         ////////////////////////////////////////////////////////////
         // selectors
         ////////////////////////////////////////////////////////////
         typedef std::vector<StrUni> selectors_type;
         selectors_type selectors;
      }; 
   };
};


#endif

/* Cora.DataSources.LgrNetSource.h

   Copyright (C) 2008, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 06 August 2008
   Last Change: Thursday 31 December 2020
   Last Commit: $Date: 2020-12-31 12:54:32 -0600 (Thu, 31 Dec 2020) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Cora_DataSources_LgrNetSource_h
#define Cora_DataSources_LgrNetSource_h

#include "Cora.DataSources.SourceBase.h"
#include "Cora.DataSources.LgrNetSymbols.h"
#include "Cora.DataSources.SinkBase.h"
#include "Cora.LgrNet.DataManager.h"
#include "Cora.LgrNet.ServerTimeEstimator.h"
#include "Cora.LgrNet.AccessTokenGetter.h"
#include "Cora.Broker.DataQuery.h"
#include "Cora.Broker.TableDataIndexGetter.h"
#include "OneShot.h"


namespace Cora
{
   namespace DataSources
   {
      class LgrNetSource;
      namespace LgrNetSourceHelpers
      {
         ////////////////////////////////////////////////////////////
         // class MyAdvisorClient
         ////////////////////////////////////////////////////////////
         class MyAdvisorClient:
            public Cora::Broker::DataAdvisorClient,
            public Cora::Broker::TableDataIndexGetterClient
         {
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            typedef Cora::LgrNet::DataManager manager_type;
            typedef Csi::SharedPtr<Request> request_handle;
            typedef Csi::SharedPtr<manager_type> manager_handle;
            MyAdvisorClient(
               LgrNetSource *source_,
               request_handle &request_,
               bool more_to_follow);

            ////////////////////////////////////////////////////////////
            // destructor
            ////////////////////////////////////////////////////////////
            virtual ~MyAdvisorClient();

            ////////////////////////////////////////////////////////////
            // add_request
            ////////////////////////////////////////////////////////////
            bool add_request(request_handle &request, bool more_to_follow);

            ////////////////////////////////////////////////////////////
            // remove_request
            ////////////////////////////////////////////////////////////
            uint4 remove_request(request_handle &request);

            ////////////////////////////////////////////////////////////
            // start
            ////////////////////////////////////////////////////////////
            void start();
            
            ////////////////////////////////////////////////////////////
            // on_advise_ready
            ////////////////////////////////////////////////////////////
            typedef Cora::Broker::DataAdvisor advisor_type;
            virtual void on_advise_ready(
               advisor_type *advisor);

            ////////////////////////////////////////////////////////////
            // on_advise_failure
            ////////////////////////////////////////////////////////////
            virtual void on_advise_failure(
               advisor_type *advisor, failure_type failure);

            ////////////////////////////////////////////////////////////
            // on_advise_record
            ////////////////////////////////////////////////////////////
            virtual void on_advise_record(
               advisor_type *advisor);

            ////////////////////////////////////////////////////////////
            // on_complete
            ////////////////////////////////////////////////////////////
            typedef Cora::Broker::TableDataIndexGetter index_getter_type;
            virtual void on_complete(
               index_getter_type *getter,
               index_getter_type::client_type::outcome_type outcome,
               index_records_type const &records);

         private:
            ////////////////////////////////////////////////////////////
            // source
            ////////////////////////////////////////////////////////////
            LgrNetSource *source;
            
            ////////////////////////////////////////////////////////////
            // requests
            ////////////////////////////////////////////////////////////
            typedef std::list<request_handle> requests_type;
            requests_type requests;

            ////////////////////////////////////////////////////////////
            // advisor
            ////////////////////////////////////////////////////////////
            Csi::SharedPtr<advisor_type> advisor;

            ////////////////////////////////////////////////////////////
            // index_getter
            ////////////////////////////////////////////////////////////
            Csi::SharedPtr<index_getter_type> index_getter;

            ////////////////////////////////////////////////////////////
            // end_file_mark
            ////////////////////////////////////////////////////////////
            uint4 end_file_mark;

            ////////////////////////////////////////////////////////////
            // end_record_no
            ////////////////////////////////////////////////////////////
            uint4 end_record_no;

            ////////////////////////////////////////////////////////////
            // started
            ////////////////////////////////////////////////////////////
            bool started;

            ////////////////////////////////////////////////////////////
            // wants_specific_columns
            ////////////////////////////////////////////////////////////
            bool wants_specific_columns;
         };

         
         ////////////////////////////////////////////////////////////
         // class MyDataManagerClient
         ////////////////////////////////////////////////////////////
         class MyDataManagerClient:
            public Cora::LgrNet::DataManagerAdviseClient,
            public Cora::Broker::TableDataIndexGetterClient
         {
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            typedef Cora::LgrNet::DataManager manager_type;
            typedef Csi::SharedPtr<Request> request_handle;
            typedef Csi::SharedPtr<manager_type> manager_handle;
            MyDataManagerClient(
               LgrNetSource *source_,
               Csi::SharedPtr<manager_type> &manager_,
               request_handle &request_);

            ////////////////////////////////////////////////////////////
            // destructor
            ////////////////////////////////////////////////////////////
            virtual ~MyDataManagerClient();

            ////////////////////////////////////////////////////////////
            // add_request
            ////////////////////////////////////////////////////////////
            bool add_request(request_handle &request);

            ////////////////////////////////////////////////////////////
            // remove_request
            //
            // Removes the specified request and returns the number of requests that are still being
            // served. 
            ////////////////////////////////////////////////////////////
            uint4 remove_request(request_handle &request);

            ////////////////////////////////////////////////////////////
            // on_advise_ready
            ////////////////////////////////////////////////////////////
            typedef Cora::LgrNet::TableDataAdvisor advisor_type;
            virtual void on_advise_ready(
               advisor_type *advisor,
               StrUni const &broker_name,
               StrUni const &table_name,
               record_type &record);

            ////////////////////////////////////////////////////////////
            // on_advise_record
            ////////////////////////////////////////////////////////////
            virtual void on_advise_record(
               advisor_type *advisor,
               StrUni const &broker_name,
               StrUni const &table_name,
               records_type &records);

            ////////////////////////////////////////////////////////////
            // on_advise_failure
            ////////////////////////////////////////////////////////////
            virtual void on_advise_failure(
               advisor_type *advisor,
               StrUni const &broker_name,
               StrUni const &table_name,
               failure_type failure);

            ////////////////////////////////////////////////////////////
            // on_complete
            ////////////////////////////////////////////////////////////
            typedef Cora::Broker::TableDataIndexGetter table_index_type;
            virtual void on_complete(
               table_index_type *getter,
               table_index_type::client_type::outcome_type outcome,
               index_records_type const &records);

         private:
            ////////////////////////////////////////////////////////////
            // source
            ////////////////////////////////////////////////////////////
            LgrNetSource *source;

            ////////////////////////////////////////////////////////////
            // manager
            ////////////////////////////////////////////////////////////
            Csi::SharedPtr<manager_type> manager;

            ////////////////////////////////////////////////////////////
            // requests
            ////////////////////////////////////////////////////////////
            typedef SinkBase::requests_type requests_type;
            requests_type requests;

            ////////////////////////////////////////////////////////////
            // start_info
            ////////////////////////////////////////////////////////////
            manager_type::StartInfo start_info;

            ////////////////////////////////////////////////////////////
            // table_index
            ////////////////////////////////////////////////////////////
            Csi::SharedPtr<table_index_type> table_index;

            ////////////////////////////////////////////////////////////
            // end_file_mark
            ////////////////////////////////////////////////////////////
            uint4 end_file_mark;

            ////////////////////////////////////////////////////////////
            // end_record_no
            ////////////////////////////////////////////////////////////
            uint4 end_record_no;

            ////////////////////////////////////////////////////////////
            // started
            ////////////////////////////////////////////////////////////
            bool started;
         };


         ////////////////////////////////////////////////////////////
         // class Query
         ////////////////////////////////////////////////////////////
         class Query: public Cora::Broker::DataQueryClient
         {
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            typedef Csi::SharedPtr<Request> request_handle;
            Query(LgrNetSource *source, request_handle &request);

            ////////////////////////////////////////////////////////////
            // destructor
            ////////////////////////////////////////////////////////////
            virtual ~Query();

            ////////////////////////////////////////////////////////////
            // start
            ////////////////////////////////////////////////////////////
            void start(ClientBase *component);
            
            ////////////////////////////////////////////////////////////
            // on_started
            ////////////////////////////////////////////////////////////
            typedef Cora::Broker::DataQuery query_type;
            virtual void on_started(
               query_type *query, record_handle &record);

            ////////////////////////////////////////////////////////////
            // on_complete
            ////////////////////////////////////////////////////////////
            virtual void on_complete(
               query_type *query, outcome_type outcome);

            ////////////////////////////////////////////////////////////
            // on_records
            ////////////////////////////////////////////////////////////
            virtual void on_records(
               query_type *query, records_type const &records, bool more_expected);

            ////////////////////////////////////////////////////////////
            // add_request
            //
            // Determines whether the specified request can be added to this
            // query and, if so, will add it.  The return value will indicate
            // whether the request was added.
            ////////////////////////////////////////////////////////////
            bool add_request(request_handle &request);

            ////////////////////////////////////////////////////////////
            // remove_request
            //
            // Determines whether the specified request is associated with this
            // query and removes it if so.  If this is the last request for
            // this query, the query object, if active, will be deleted and the
            // return value will be true to indicate that the source should
            // remove this object from the list it manages.
            ////////////////////////////////////////////////////////////
            bool remove_request(request_handle &request);

            ////////////////////////////////////////////////////////////
            // start
            ////////////////////////////////////////////////////////////
            void start();
            
         private:
            ////////////////////////////////////////////////////////////
            // source
            ////////////////////////////////////////////////////////////
            LgrNetSource *source;
            
            ////////////////////////////////////////////////////////////
            // requests
            ////////////////////////////////////////////////////////////
            typedef std::list<request_handle> requests_type;
            requests_type requests;

            ////////////////////////////////////////////////////////////
            // query
            ////////////////////////////////////////////////////////////
            Csi::SharedPtr<query_type> query;

            ////////////////////////////////////////////////////////////
            // wants_specific_columns
            ////////////////////////////////////////////////////////////
            bool wants_specific_columns;
         };


         class VariableSetter;
         class FileSender;
         class NewestFileGetter;
         class ClockChecker;
         class FileControl;
         class Terminal;
         class ListFiles;
      };

      
      ////////////////////////////////////////////////////////////
      // class LgrNetSource
      //
      // Defines a data source that communicates with the LoggerNet server to
      // obtain its data.
      ////////////////////////////////////////////////////////////
      class LgrNetSource:
         public SourceBase,
         public Cora::LgrNet::ServerTimeEstimatorClient,
         public OneShotClient,
         public Cora::Broker::TableDataIndexGetterClient,
         public Cora::LgrNet::AccessTokenGetterClient
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         LgrNetSource(StrUni const &name, bool password_encrypted_ = false);

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~LgrNetSource();

         ////////////////////////////////////////////////////////////
         // connect
         ////////////////////////////////////////////////////////////
         virtual void connect();

         ////////////////////////////////////////////////////////////
         // disconnect
         ////////////////////////////////////////////////////////////
         virtual void disconnect();

         ////////////////////////////////////////////////////////////
         // is_connected
         ////////////////////////////////////////////////////////////
         virtual bool is_connected() const;

         /**
          * @return Return the timestamp for the source
          */
         virtual Csi::LgrDate get_source_time();

         ////////////////////////////////////////////////////////////
         // get_properties
         ////////////////////////////////////////////////////////////
         virtual void get_properties(Csi::Xml::Element &prop_xml);

         ////////////////////////////////////////////////////////////
         // set_properties
         ////////////////////////////////////////////////////////////
         virtual void set_properties(Csi::Xml::Element &prop_xml);

         ////////////////////////////////////////////////////////////
         // add_request
         ////////////////////////////////////////////////////////////
         virtual void add_request(
            request_handle &request, bool more_to_follow = false);

         ////////////////////////////////////////////////////////////
         // set_request_retry
         ////////////////////////////////////////////////////////////
         void set_request_retry();

         ////////////////////////////////////////////////////////////
         // remove_request
         ////////////////////////////////////////////////////////////
         virtual void remove_request(request_handle &request);

         ////////////////////////////////////////////////////////////
         // remove_all_requests
         ////////////////////////////////////////////////////////////
         virtual void remove_all_requests();
         
         ////////////////////////////////////////////////////////////
         // activate_requests
         ////////////////////////////////////////////////////////////
         virtual void activate_requests();

         ////////////////////////////////////////////////////////////
         // stop
         ////////////////////////////////////////////////////////////
         virtual void stop(); 

         ////////////////////////////////////////////////////////////
         // set_manager
         ////////////////////////////////////////////////////////////
         virtual void set_manager(Manager *manager);

         ////////////////////////////////////////////////////////////
         // get_statistic_uri
         ////////////////////////////////////////////////////////////
         virtual void get_statistic_uri(
            std::ostream &out,
            StrUni const &station_uri,
            StrUni const &statistic_name);

         ////////////////////////////////////////////////////////////
         // get_statistic_station
         ////////////////////////////////////////////////////////////
         virtual void get_statistic_station(
            std::ostream &out, StrUni const &statistic_uri);

         ////////////////////////////////////////////////////////////
         // start_set_value
         ////////////////////////////////////////////////////////////
         virtual bool start_set_value(
            SinkBase *sink, StrUni const &uri, ValueSetter const &value);

         ////////////////////////////////////////////////////////////
         // end_set_value
         ////////////////////////////////////////////////////////////
         typedef LgrNetSourceHelpers::VariableSetter setter_type;
         void end_set_value(setter_type *setter);

         /**
          * Overloads the base class version to start the send file operation.
          */
         virtual bool start_send_file(
            SinkBase *sink,
            StrUni const &uri,
            StrUni const &dest_file_name,
            StrUni const &file_name);

         /**
          * Removes the specified file sender from this source.
          */
         typedef LgrNetSourceHelpers::FileSender file_sender_type;
         void end_send_file(file_sender_type *sender);

         /**
          * Overloads the base class version to start the operation to poll for the newest file.
          */
         virtual bool start_get_newest_file(
            SinkBase *sink,
            StrUni const &uri,
            StrUni const &pattern);

         /**
          * Removes the specified getter.
          */
         typedef LgrNetSourceHelpers::NewestFileGetter newest_getter_type;
         void end_get_newest_file(newest_getter_type *getter);

         /**
          * Overloads the base class' version to start a clock check transaction.
          */
         virtual bool start_clock_check(
            SinkBase *sink,
            StrUni const &uri,
            bool should_set,
            bool send_server_time,
            Csi::LgrDate const &server_time);

         /**
          * Removes the specified clock checker.
          */
         typedef LgrNetSourceHelpers::ClockChecker clock_checker_type;
         void end_clock_check(clock_checker_type *checker);

         /**
          * Overloads the base class to enable the file control operation for this source.
          */
         virtual bool start_file_control(
            SinkBase *sink,
            StrUni const &uri,
            uint4 command,
            StrAsc const &p1,
            StrAsc const &p2);

         /**
          * Responsible for releasing resources associated with the specified file control
          * operation.
          */
         typedef LgrNetSourceHelpers::FileControl file_control_type;
         void end_file_control(file_control_type *controller);

         /**
          * Overloads the base class version to start a transaction to get a list of files from a
          * station.
          */
         virtual bool start_list_files(
            SinkBase *sink,
            StrUni const &station_uri,
            int8 transaction,
            StrAsc const &filter = "");

         /**
          * Responsible for releasing resources associated with the specified list files operation.
          */
         typedef LgrNetSourceHelpers::ListFiles list_files_type;
         void end_list_files(list_files_type *lister);

         ////////////////////////////////////////////////////////////
         // breakdown_uri
         ////////////////////////////////////////////////////////////
         virtual void breakdown_uri(symbols_type &symbols, StrUni const &uri);

         ////////////////////////////////////////////////////////////
         // on_started
         ////////////////////////////////////////////////////////////
         typedef Cora::LgrNet::ServerTimeEstimator time_estimator_type;
         virtual void on_started(
            time_estimator_type *estimator);

         ////////////////////////////////////////////////////////////
         // on_failure
         ////////////////////////////////////////////////////////////
         virtual void on_failure(
            time_estimator_type *estimator,
            time_estimator_type::client_type::failure_type failure);

         /**
          * Overrides the base class version to handle the completion event for the access token
          * getter.
          */
         typedef Cora::LgrNet::AccessTokenGetter access_token_getter_type;
         virtual void on_complete(
            access_token_getter_type *sender,
            access_token_getter_type::client_type::outcome_type outcome,
            StrAsc const &access_token_,
            StrAsc const &refresh_token_) override;

         ////////////////////////////////////////////////////////////
         // onOneShotFired
         ////////////////////////////////////////////////////////////
         virtual void onOneShotFired(uint4 id);

         ////////////////////////////////////////////////////////////
         // get_server_address
         ////////////////////////////////////////////////////////////
         StrAsc const &get_server_address() const
         { return server_address; }

         ////////////////////////////////////////////////////////////
         // set_server_address
         ////////////////////////////////////////////////////////////
         void set_server_address(StrAsc const &server_address_, uint2 server_port_);

         ////////////////////////////////////////////////////////////
         // get_server_port
         ////////////////////////////////////////////////////////////
         uint2 get_server_port() const
         { return server_port; }

         ////////////////////////////////////////////////////////////
         // get_logon_name
         ////////////////////////////////////////////////////////////
         StrUni const &get_logon_name() const
         { return logon_name; }

         ////////////////////////////////////////////////////////////
         // set_logon_name
         ////////////////////////////////////////////////////////////
         void set_logon_name(StrUni const &logon_name_);

         ////////////////////////////////////////////////////////////
         // get_logon_password
         ////////////////////////////////////////////////////////////
         StrUni const &get_logon_password() const
         { return logon_password; }

         ////////////////////////////////////////////////////////////
         // set_logon_password
         ////////////////////////////////////////////////////////////
         void set_logon_password(StrUni const &logon_pasword_);

         ////////////////////////////////////////////////////////////
         // get_password_encrypted
         ////////////////////////////////////////////////////////////
         bool get_password_encrypted() const
         { return password_encrypted; }

         ////////////////////////////////////////////////////////////
         // set_password_encrypted
         ////////////////////////////////////////////////////////////
         void set_password_encrypted(bool encrypted)
         { password_encrypted = encrypted; }

         /**
          * @param value Specifies the access token.
          */
         void set_access_token(StrAsc const &value)
         { access_token = value; }

         /**
          * @return Returns the access token.
          */
         StrAsc const &get_access_token() const
         { return access_token; }

         /**
          * @return Returns the refresh token.
          */
         StrAsc const &get_refresh_token() const
         { return refresh_token; }

         /**
          * @param value Specifies the refresh token.
          */
         void set_refresh_token(StrAsc const &value)
         { refresh_token = value; }

         ////////////////////////////////////////////////////////////
         // get_loggernet_component
         ////////////////////////////////////////////////////////////
         Cora::ClientBase *get_loggernet_component()
         {
            Cora::ClientBase *rtn = 0;
            if(connect_active)
               rtn = time_estimator.get_rep();
            return rtn;
         }

         ////////////////////////////////////////////////////////////
         // get_timer
         ////////////////////////////////////////////////////////////
         Csi::SharedPtr<OneShot> &get_timer()
         { return timer; }

         ////////////////////////////////////////////////////////////
         // get_source_symbol
         ////////////////////////////////////////////////////////////
         virtual symbol_handle get_source_symbol();
         
         ///////////////////////////////////////////////////////////
         // get_type
         ///////////////////////////////////////////////////////////
         virtual SymbolBase::symbol_type_code get_type()
         { return SymbolBase::type_lgrnet_source; }

         ///////////////////////////////////////////////////////////
         // get_loggernet_time
         ///////////////////////////////////////////////////////////
         Csi::LgrDate get_loggernet_time()
         {
            if(time_estimator != 0)
               return time_estimator->get_server_time();
            else
               return 0;
         }

         ///////////////////////////////////////////////////////////
         // get_actual_inloc_table_name
         ///////////////////////////////////////////////////////////
         void get_actual_inloc_table_name(
            StrUni const &uri, 
            StrUni &table_name);

         ////////////////////////////////////////////////////////////
         // std_retry_interval
         //
         // Specifies the standard interval at while source of this type will
         // attempt to reconnect to the loggernet server. 
         ////////////////////////////////////////////////////////////
         static uint4 const std_retry_interval;

         ////////////////////////////////////////////////////////////
         // on_client_advise_failure
         ////////////////////////////////////////////////////////////
         typedef LgrNetSourceHelpers::MyDataManagerClient client_type;
         void on_client_advise_failure(client_type *client);

         ////////////////////////////////////////////////////////////
         // on_advisor_failure
         ////////////////////////////////////////////////////////////
         typedef LgrNetSourceHelpers::MyAdvisorClient advisor_type;
         void on_advisor_failure(advisor_type *advisor);
         
         ////////////////////////////////////////////////////////////
         // get_data_manager
         ////////////////////////////////////////////////////////////
         typedef Cora::LgrNet::DataManager data_manager_type;
         typedef Csi::SharedPtr<data_manager_type> manager_handle;
         manager_handle &get_data_manager()
         { return data_manager; }

         ////////////////////////////////////////////////////////////
         // is_classic_inlocs
         ////////////////////////////////////////////////////////////
         virtual bool is_classic_inlocs(StrUni const &uri);

         ////////////////////////////////////////////////////////////
         // get_last_connect_error
         ////////////////////////////////////////////////////////////
         uint4 get_last_connect_error() const
         { return last_connect_error; }

         ////////////////////////////////////////////////////////////
         // get_table_range
         ////////////////////////////////////////////////////////////
         virtual void get_table_range(
            Csi::EventReceiver *client, StrUni const &uri);
         
         ////////////////////////////////////////////////////////////
         // on_complete (table index getter)
         ////////////////////////////////////////////////////////////
         typedef Cora::Broker::TableDataIndexGetter range_getter_type;
         virtual void on_complete(
            range_getter_type *getter,
            range_getter_type::client_type::outcome_type outcome,
            index_records_type const &index_records);

         /**
          * Overloads the base class version to attempt to start terminal services.
          */
         virtual bool start_terminal(
            TerminalSinkBase *sink, StrUni const &station_uri, int8 sink_token);

         /**
          * Overloads the base class to transmit the specified data to the service identified by the
          * sink and application token.
          */
         virtual void send_terminal(
            TerminalSinkBase *sink, int8 sink_token, void const *buff, size_t buff_len);

         /**
          * Overloads the base class version to close the terminal service.
          */
         virtual void stop_terminal(
            TerminalSinkBase *sink, int8 sink_token);

      private:
         ////////////////////////////////////////////////////////////
         // time_estimator
         ////////////////////////////////////////////////////////////
         Csi::SharedPtr<time_estimator_type> time_estimator;

         /**
          * Specifies the token used to get a new access token when the current access token has
          * expired.
          */
         Csi::SharedPtr<access_token_getter_type> access_token_getter;

         ////////////////////////////////////////////////////////////
         // server_address
         //
         // Specifies the TCP address of the LoggerNet server.
         ////////////////////////////////////////////////////////////
         StrAsc server_address;

         ////////////////////////////////////////////////////////////
         // server_port
         //
         // Specifies the TCP port for the LoggerNet server
         ////////////////////////////////////////////////////////////
         uint2 server_port;

         ////////////////////////////////////////////////////////////
         // logon_name
         //
         // Specifies the account name used to log into the LoggerNet server. 
         ////////////////////////////////////////////////////////////
         StrUni logon_name;

         ////////////////////////////////////////////////////////////
         // logon_password
         //
         // Specifies the password used to log into the LoggerNet server. 
         ////////////////////////////////////////////////////////////
         StrUni logon_password;

         ////////////////////////////////////////////////////////////
         // remember
         //
         // Set to true if the application should persist the logon information
         // for the LoggerNet server.
         ////////////////////////////////////////////////////////////
         bool remember;

         ////////////////////////////////////////////////////////////
         // password_encrypted
         ////////////////////////////////////////////////////////////
         bool password_encrypted;

         /**
          * Specifies the access token.
          */
         StrAsc access_token;

         /**
          * Specifies the refresh token.
          */
         StrAsc refresh_token;

         ////////////////////////////////////////////////////////////
         // connect_active
         //
         // Set to true when the connection to the LoggerNet server is active. 
         ////////////////////////////////////////////////////////////
         bool connect_active;

         ////////////////////////////////////////////////////////////
         // was_connected
         //
         // Set to true if a connection to the LoggerNet server was started.
         ////////////////////////////////////////////////////////////
         bool was_connected;

         ////////////////////////////////////////////////////////////
         // clients
         ////////////////////////////////////////////////////////////
         typedef Csi::SharedPtr<client_type> client_handle;
         typedef std::list<client_handle> clients_type;
         clients_type clients;

         ////////////////////////////////////////////////////////////
         // advisors
         ////////////////////////////////////////////////////////////
         typedef Csi::SharedPtr<advisor_type> advisor_handle;
         typedef std::list<advisor_handle> advisors_type;
         advisors_type advisors;
         
         ////////////////////////////////////////////////////////////
         // data_manager
         ////////////////////////////////////////////////////////////
         manager_handle data_manager;

         ////////////////////////////////////////////////////////////
         // queries
         ////////////////////////////////////////////////////////////
         typedef Csi::SharedPtr<LgrNetSourceHelpers::Query> query_handle;
         typedef std::list<query_handle> queries_type;
         queries_type queries;

         ////////////////////////////////////////////////////////////
         // timer
         ////////////////////////////////////////////////////////////
         Csi::SharedPtr<OneShot> timer;

         ////////////////////////////////////////////////////////////
         // retry_id
         ////////////////////////////////////////////////////////////
         uint4 retry_id;

         ////////////////////////////////////////////////////////////
         // source_symbol
         ////////////////////////////////////////////////////////////
         Csi::PolySharedPtr<SymbolBase, LgrNetSourceSymbol> source_symbol;

         ////////////////////////////////////////////////////////////
         // retry_requests_id
         //
         // Identifies a timer used to retry failed requests.  
         ////////////////////////////////////////////////////////////
         uint4 retry_requests_id;

         ////////////////////////////////////////////////////////////
         // setters
         ////////////////////////////////////////////////////////////
         typedef Csi::SharedPtr<setter_type> setter_handle;
         typedef std::list<setter_handle> setters_type;
         setters_type setters;

         /**
          * Specifies the set of file senders that are pending.
          */
         typedef Csi::SharedPtr<file_sender_type> file_sender_handle;
         typedef std::deque<file_sender_handle> file_senders_type;
         file_senders_type file_senders;

         /**
          * Specifies the set of newest file getters that are pending.
          */
         typedef Csi::SharedPtr<newest_getter_type> newest_getter_handle;
         typedef std::deque<newest_getter_handle> newest_getters_type;
         newest_getters_type newest_getters;

         /**
          * Specifies the list of pending clock checkers.
          */
         typedef Csi::SharedPtr<clock_checker_type> clock_checker_handle;
         typedef std::deque<clock_checker_handle> clock_checkers_type;
         clock_checkers_type clock_checkers;

         /**
          * Specifies the list of file controllers that are active.
          */
         typedef Csi::SharedPtr<file_control_type> file_control_handle;
         typedef std::deque<file_control_handle> file_controls_type;
         file_controls_type file_controls;

         /**
          * Specifies the collection of terminal services that are being supported presently by this
          * source.
          */
         typedef LgrNetSourceHelpers::Terminal terminal_type;
         typedef Csi::SharedPtr<terminal_type> terminal_handle;
         typedef std::deque<terminal_handle> terminals_type;
         terminals_type terminals;

         /**
          * Lists the list files operations that are currently pending.
          */
         typedef LgrNetSourceHelpers::ListFiles lister_type;
         typedef Csi::SharedPtr<lister_type> lister_handle;
         typedef std::deque<lister_handle> listers_type;
         listers_type listers;

         ////////////////////////////////////////////////////////////
         // last_connect_error
         ////////////////////////////////////////////////////////////
         uint4 last_connect_error;

         ////////////////////////////////////////////////////////////
         // table_ranges
         //
         // Represents the set of table range operations that are currently pending. 
         ////////////////////////////////////////////////////////////
         typedef std::pair<Csi::EventReceiver *, StrUni> table_range_client;
         typedef Csi::SharedPtr<Cora::Broker::TableDataIndexGetter> range_getter_handle;
         typedef std::pair<range_getter_handle, table_range_client> table_range_op;
         typedef std::list<table_range_op> table_ranges_type;
         table_ranges_type table_ranges;
      };
   };
};


#endif

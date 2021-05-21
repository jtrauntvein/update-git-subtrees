/* Cora.DataSources.Manager.h

   Copyright (C) 2008, 2014 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 01 August 2008
   Last Change: Friday 27 June 2014
   Last Commit: $Date: 2014-06-27 16:43:57 -0600 (Fri, 27 Jun 2014) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Cora_DataSources_Manager_h
#define Cora_DataSources_Manager_h

#include "Cora.DataSources.SourceBase.h"
#include "Cora.DataSources.Request.h"
#include "Cora.DataSources.SymbolBase.h"
#include "Cora.DataSources.SinkBase.h"
#include "Csi.Events.h"
#include "OneShot.h"
#include <list>


namespace Cora
{
   namespace DataSources
   {
      ////////////////////////////////////////////////////////////
      // class ManagerClient
      //
      // Defines the interface that receives event from a Manager object.  These
      // events deal with the management of data sources such as data sources
      // being added, removed, or the status of these sources changing.
      ////////////////////////////////////////////////////////////
      class Manager;
      class ManagerClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_source_added
         //
         // Called when a data source has been added to the manager.
         ////////////////////////////////////////////////////////////
         typedef Csi::SharedPtr<SourceBase> source_handle;
         virtual void on_source_added(
            Manager *manager,
            source_handle &source)
         { }
         
         ////////////////////////////////////////////////////////////
         // on_source_removed
         //
         // Called when a data source has been removed from the manager.
         ////////////////////////////////////////////////////////////
         virtual void on_source_removed(
            Manager *manager,
            source_handle &source)
         { }
         
         ////////////////////////////////////////////////////////////
         // on_source_connecting
         //
         // Called when a source is entering a "connecting" state.  
         ////////////////////////////////////////////////////////////
         virtual void on_source_connecting(
            Manager *manager,
            source_handle &source)
         { }
         
         ////////////////////////////////////////////////////////////
         // on_source_connect
         //
         // Called when a source has  been successfully "connected" 
         ////////////////////////////////////////////////////////////
         virtual void on_source_connect(
            Manager *manager,
            source_handle &source)
         { }
         
         ////////////////////////////////////////////////////////////
         // on_source_disconnect
         //
         // Called when the connection for a source has failed. 
         ////////////////////////////////////////////////////////////
         enum disconnect_reason_type
         {
            disconnect_connection_failed = 1,
            disconnect_properties_changed = 2,
            disconnect_by_application = 3,
            disconnect_invalid_logon = 4
         };
         virtual void on_source_disconnect(
            Manager *manager,
            source_handle &source,
            disconnect_reason_type reason)
         { }

         ////////////////////////////////////////////////////////////
         // on_source_log
         //
         // Called when the source has an event that should be logged. 
         ////////////////////////////////////////////////////////////
         virtual void on_source_log(
            Manager *manager,
            source_handle &source,
            StrAsc const &message)
         { }
      };


      ////////////////////////////////////////////////////////////
      // class GetTableRangeCompleteEvent
      ////////////////////////////////////////////////////////////
      class GetTableRangeCompleteEvent: public Csi::Event
      {
      public:
         ////////////////////////////////////////////////////////////
         // event_id
         ////////////////////////////////////////////////////////////
         static uint4 const event_id;

         ////////////////////////////////////////////////////////////
         // uri
         ////////////////////////////////////////////////////////////
         StrUni const uri;

         ////////////////////////////////////////////////////////////
         // outcome
         ////////////////////////////////////////////////////////////
         enum outcome_type
         {
            outcome_unknown = 0,
            outcome_success = 1,
            outcome_not_implemented = 2,
            outcome_no_table = 3,
            outcome_not_connected = 4,
            outcome_no_records = 5,
            outcome_no_broker = 6,
            outcome_no_source = 7
         };
         outcome_type const outcome;

         ////////////////////////////////////////////////////////////
         // begin_date
         ////////////////////////////////////////////////////////////
         Csi::LgrDate const begin_date;

         ////////////////////////////////////////////////////////////
         // end_date
         ////////////////////////////////////////////////////////////
         Csi::LgrDate const end_date;

         ////////////////////////////////////////////////////////////
         // cpost
         ////////////////////////////////////////////////////////////
         static void cpost(
            Csi::EventReceiver *client,
            StrUni const &uri,
            outcome_type outcome,
            Csi::LgrDate const &begin_date = 0,
            Csi::LgrDate const &end_date = 0)
         {
            if(Csi::EventReceiver::is_valid_instance(client))
            {
               GetTableRangeCompleteEvent *event(
                  new GetTableRangeCompleteEvent(
                     client, uri, outcome, begin_date, end_date));
               event->post();
            }
         }
         
      private:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         GetTableRangeCompleteEvent(
            Csi::EventReceiver *client,
            StrUni const &uri_,
            outcome_type outcome_,
            Csi::LgrDate const &begin_date_,
            Csi::LgrDate const &end_date_):
            Csi::Event(event_id, client),
            uri(uri_),
            outcome(outcome_),
            begin_date(begin_date_),
            end_date(end_date_)
         { }
      };


      ////////////////////////////////////////////////////////////
      // class ManagerSupervisor
      //
      // Defines an interface that can be implemented by the application to
      // "supervise" the flow of data and requests to and from the manager and
      // its sources.  The application can use this class by creating a derived
      // class that overloads the virtual methods and "plugging" in the
      // supervisor by calling the manager's set_supervisor() method.
      ////////////////////////////////////////////////////////////
      class ManagerSupervisor
      {
      public:
         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~ManagerSupervisor()
         { }

         ////////////////////////////////////////////////////////////
         // on_add_request
         //
         // Called when the manager's add_request() method has been called.
         // This gives the supervisor the opportunity to alter that request's
         // starting conditions as needed.
         ////////////////////////////////////////////////////////////
         typedef Csi::SharedPtr<Request> request_handle;
         virtual void on_add_request(
            Manager *manager,
            request_handle &request)
         { }

         ////////////////////////////////////////////////////////////
         // on_sink_data
         //
         // Called when a source is getting ready to deliver a set of records
         // to a sink.  This will give the supervisor the opportunity to weed
         // out records that the sink should not receive.  If the records
         // container is cleared by the supervisor, no notification will be
         // sent to the sink.
         ////////////////////////////////////////////////////////////
         typedef SinkBase::records_type records_type;
         typedef SinkBase::requests_type requests_type;
         virtual void on_sink_data(
            Manager *manager,
            requests_type &requests,
            records_type &records)
         { }
      };


      ////////////////////////////////////////////////////////////
      // class ValueSetter
      //
      // Defines an object that holds the value to which a variable should be
      // set.  This class can deal with multiple types of values.
      ////////////////////////////////////////////////////////////
      class ValueSetter
      {
      public:
         ////////////////////////////////////////////////////////////
         // value_type
         ////////////////////////////////////////////////////////////
         enum value_type_type
         {
            value_type_bool,
            value_type_float,
            value_type_uint4,
            value_type_int4,
            value_type_uint2,
            value_type_int2,
            value_type_uint1,
            value_type_int1,
            value_type_string 
         } value_type;

         ////////////////////////////////////////////////////////////
         // value_variant
         ////////////////////////////////////////////////////////////
         union value_variant_type
         {
            bool v_bool;
            float v_float;
            uint4 v_uint4;
            int4 v_int4;
            uint2 v_uint2;
            int2 v_int2;
            byte v_uint1;
            char v_int1;
         } value_variant;

         ////////////////////////////////////////////////////////////
         // value_string
         ////////////////////////////////////////////////////////////
         StrAsc const value_string;

         ////////////////////////////////////////////////////////////
         // locale
         ////////////////////////////////////////////////////////////
         std::locale locale;
         
         ////////////////////////////////////////////////////////////
         // constructor (from string)
         ////////////////////////////////////////////////////////////
         ValueSetter(StrAsc const &s, std::locale const &locale_ = std::locale::classic()):
            value_string(s),
            value_type(value_type_string),
            locale(locale_)
         { }

         ////////////////////////////////////////////////////////////
         // constructor (from double)
         ////////////////////////////////////////////////////////////
         explicit ValueSetter(double value, std::locale const &locale_ = std::locale::classic()):
            value_type(value_type_float),
            locale(locale_)
         { value_variant.v_float = static_cast<float>(value); }

         ////////////////////////////////////////////////////////////
         // constructor (from float)
         ////////////////////////////////////////////////////////////
         explicit ValueSetter(float value, std::locale const &locale_ = std::locale::classic()):
            value_type(value_type_float),
            locale(locale_)
         { value_variant.v_float = value; }

         ////////////////////////////////////////////////////////////
         // constructor (from uint4)
         ////////////////////////////////////////////////////////////
         explicit ValueSetter(uint4 value, std::locale const &locale_ = std::locale::classic()):
            value_type(value_type_uint4),
            locale(locale_)
         { value_variant.v_uint4 = value; }

         ////////////////////////////////////////////////////////////
         // constructor (from int4)
         ////////////////////////////////////////////////////////////
         explicit ValueSetter(int4 value, std::locale const &locale_ = std::locale::classic()):
            value_type(value_type_int4),
            locale(locale_)
         { value_variant.v_int4 = value; }

         ////////////////////////////////////////////////////////////
         // constructor (from uint2)
         ////////////////////////////////////////////////////////////
         explicit ValueSetter(uint2 value, std::locale const &locale_ = std::locale::classic()):
            value_type(value_type_uint2),
            locale(locale_)
         { value_variant.v_uint2 = value; }

         ////////////////////////////////////////////////////////////
         // constructor (from int2
         ////////////////////////////////////////////////////////////
         explicit ValueSetter(int2 value, std::locale const &locale_ = std::locale::classic()):
            value_type(value_type_int2),
            locale(locale_)
         { value_variant.v_int2 = value; }

         ////////////////////////////////////////////////////////////
         // constructor (from uint1)
         ////////////////////////////////////////////////////////////
         explicit ValueSetter(byte value, std::locale const locale_ = std::locale::classic()):
            value_type(value_type_uint1),
            locale(locale_)
         { value_variant.v_uint1 = value; }

         ////////////////////////////////////////////////////////////
         // constructor (from int1)
         ////////////////////////////////////////////////////////////
         explicit ValueSetter(char value, std::locale const &locale_ = std::locale::classic()):
            value_type(value_type_int1),
            locale(locale_)
         { value_variant.v_int1 = value; }

         ////////////////////////////////////////////////////////////
         // constructor (from bool)
         ////////////////////////////////////////////////////////////
         explicit ValueSetter(bool value, std::locale const &locale_ = std::locale::classic()):
            value_type(value_type_bool),
            locale(locale_)
         { value_variant.v_bool = value; }

         ////////////////////////////////////////////////////////////
         // copy constructor
         ////////////////////////////////////////////////////////////
         ValueSetter(ValueSetter const &other):
            value_type(other.value_type),
            value_variant(other.value_variant),
            value_string(other.value_string),
            locale(other.locale)
         { }
      };
      
      
      ////////////////////////////////////////////////////////////
      // class Manager
      //
      // Defines a component that manages access to data from multiple polymorphic
      // data source objects.  Typically, this class is a singleton within an
      // application but is not constrained to be so.  An application can monitor
      // events associated with the manager by registering a client object
      // (derived from class ManagerClient).
      ////////////////////////////////////////////////////////////
      class Manager: public Csi::EventReceiver
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         Manager();
         
         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~Manager();

         ////////////////////////////////////////////////////////////
         // add_client
         ////////////////////////////////////////////////////////////
         typedef ManagerClient client_type;
         void add_client(client_type *client);

         ////////////////////////////////////////////////////////////
         // remove_client
         ////////////////////////////////////////////////////////////
         void remove_client(client_type *client);

         ////////////////////////////////////////////////////////////
         // set_supervisor
         //
         // Can be called by the application to set the supervisor for this
         // manager.  This call will only succeed if there are no pending
         // requests for this manager.  If there are pending requests, an
         // exception will be thrown.
         ////////////////////////////////////////////////////////////
         typedef Csi::SharedPtr<ManagerSupervisor> supervisor_handle;
         void set_supervisor(supervisor_handle supervisor_);

         ////////////////////////////////////////////////////////////
         // get_supervisor
         ////////////////////////////////////////////////////////////
         supervisor_handle &get_supervisor()
         { return supervisor; }

         // @group: this manager will act as a data sources container

         ////////////////////////////////////////////////////////////
         // begin
         ////////////////////////////////////////////////////////////
         typedef Csi::SharedPtr<SourceBase> source_handle;
         typedef source_handle value_type;
         typedef std::list<source_handle> sources_type;
         typedef sources_type::iterator iterator;
         typedef sources_type::const_iterator const_iterator;
         iterator begin()
         { return sources.begin(); }
         const_iterator begin() const
         { return sources.begin(); }

         ////////////////////////////////////////////////////////////
         // end
         ////////////////////////////////////////////////////////////
         iterator end()
         { return sources.end(); }
         const_iterator end() const
         { return sources.end(); }

         ////////////////////////////////////////////////////////////
         // empty
         ////////////////////////////////////////////////////////////
         bool empty() const
         { return sources.empty(); }

         ////////////////////////////////////////////////////////////
         // size
         ////////////////////////////////////////////////////////////
         typedef sources_type::size_type size_type;
         size_type size() const
         { return sources.size(); }

         ////////////////////////////////////////////////////////////
         // find
         ////////////////////////////////////////////////////////////
         iterator find(SourceBase *source);
         const_iterator find(SourceBase *source) const;
         iterator find(StrUni const &source_name);
         const_iterator find(StrUni const &source_name) const;

         ////////////////////////////////////////////////////////////
         // find_source
         //
         // Searches for the source with the specified name.  If the specified
         // source cannot be found, a null reference will be returned. 
         ////////////////////////////////////////////////////////////
         source_handle find_source(StrUni const &source);

         ////////////////////////////////////////////////////////////
         // parse_source_name
         //
         // Given a URI, this method will parse the source name.
         ////////////////////////////////////////////////////////////
         static StrUni parse_source_name(StrUni const &uri);

         ////////////////////////////////////////////////////////////
         // find_source_uri
         //
         // Searches for the source with the specified uri.  If the source
         // cannot be found, a null reference will be returned. 
         ////////////////////////////////////////////////////////////
         source_handle find_source_uri(StrUni const &uri)
         { return find_source(parse_source_name(uri)); }

         ////////////////////////////////////////////////////////////
         // add_source
         ////////////////////////////////////////////////////////////
         void add_source(source_handle source, bool start_now = false);
         void push_back(source_handle source)
         { add_source(source); }

         ////////////////////////////////////////////////////////////
         // remove_source
         ////////////////////////////////////////////////////////////
         void remove_source(SourceBase *source);

         ///////////////////////////////////////////////////////////
         // remove_all_sources
         ///////////////////////////////////////////////////////////
         void remove_all_sources();

         // @endgroup:

         ////////////////////////////////////////////////////////////
         // start_sources
         //
         // Iterates the list of sources and attempts to "start" each one.  
         ////////////////////////////////////////////////////////////
         void start_sources();

         ////////////////////////////////////////////////////////////
         // stop_sources
         //
         // Iterates the list of sources and "stops" each one. 
         ////////////////////////////////////////////////////////////
         void stop_sources();

         ////////////////////////////////////////////////////////////
         // report_source_connecting
         ////////////////////////////////////////////////////////////
         void report_source_connecting(SourceBase *source);
         
         ////////////////////////////////////////////////////////////
         // report_source_connect
         ////////////////////////////////////////////////////////////
         void report_source_connect(SourceBase *source);

         ////////////////////////////////////////////////////////////
         // report_source_disconnect
         ////////////////////////////////////////////////////////////
         void report_source_disconnect(SourceBase *source, ManagerClient::disconnect_reason_type reason);

         ////////////////////////////////////////////////////////////
         // report_source_log
         ////////////////////////////////////////////////////////////
         void report_source_log(SourceBase *source, StrAsc const &message);
         
         // @group: this class will also act as a container for request objects

         ////////////////////////////////////////////////////////////
         // requests_begin
         ////////////////////////////////////////////////////////////
         typedef Csi::SharedPtr<Request> request_handle;
         typedef std::list<request_handle> requests_type;
         typedef requests_type::iterator requests_iterator;
         typedef requests_type::const_iterator requests_const_iterator;
         requests_iterator requests_begin()
         { return requests.begin(); }
         requests_const_iterator requests_begin() const
         { return requests.begin(); }

         ////////////////////////////////////////////////////////////
         // requests_end
         ////////////////////////////////////////////////////////////
         requests_iterator requests_end()
         { return requests.end(); }
         requests_const_iterator requests_end() const
         { return requests.end(); }

         ////////////////////////////////////////////////////////////
         // requests_empty
         ////////////////////////////////////////////////////////////
         bool requests_empty() const
         { return requests.empty(); }

         ////////////////////////////////////////////////////////////
         // requests_size
         ////////////////////////////////////////////////////////////
         size_type requests_size() const
         { return requests.size(); }

         ////////////////////////////////////////////////////////////
         // add_request
         //
         // Adds a request to those that the manager will attempt to honour when
         // started.
         //
         // The more_to_follow parameter, if set to true, will instruct the
         // data source to make room for the request but to not activate that
         // request since there may be more compatible requests that will
         // follow.  If the application sets this flag, it should call the
         // activate_requests() method following.
         ////////////////////////////////////////////////////////////
         void add_request(request_handle request, bool more_to_follow = false);

         ////////////////////////////////////////////////////////////
         // activate_requests
         //
         // Can be called to "activate" any requests that were added with the
         // more_to_follow flag set to false. 
         ////////////////////////////////////////////////////////////
         void activate_requests();

         ////////////////////////////////////////////////////////////
         // remove_request
         //
         // Removes the specified request.
         ////////////////////////////////////////////////////////////
         void remove_request(request_handle &request);
         
         ////////////////////////////////////////////////////////////
         // remove_requests
         //
         // Removes any requests associated with the specified sink.
         ////////////////////////////////////////////////////////////
         void remove_requests(SinkBase *sink);

         ////////////////////////////////////////////////////////////
         // remove_all_requests
         //
         // Removes all requests for all data sources. 
         ////////////////////////////////////////////////////////////
         void remove_all_requests();
         
         // @endgroup

         ////////////////////////////////////////////////////////////
         // get_timer
         ////////////////////////////////////////////////////////////
         Csi::SharedPtr<OneShot> &get_timer()
         { return timer; }

         ////////////////////////////////////////////////////////////
         // get_value_factory
         ////////////////////////////////////////////////////////////
         Csi::SharedPtr<Cora::Broker::ValueFactory> &get_value_factory()
         { return value_factory; }

         ////////////////////////////////////////////////////////////
         // set_value_factory
         ////////////////////////////////////////////////////////////
         void set_value_factory(Csi::SharedPtr<Cora::Broker::ValueFactory> &value_factory_)
         { value_factory = value_factory_; }

         ////////////////////////////////////////////////////////////
         // get_statistic_uri
         //
         // Given a URI to a station object and the name of a statistic
         // associated with that station, this method will return the URI to
         // that statistic.
         ////////////////////////////////////////////////////////////
         StrUni get_statistic_uri(StrUni const &station_uri, StrUni const &statistic_name);

         ////////////////////////////////////////////////////////////
         // get_statistic_station
         //
         // Given a URI to a statistic, as produced by get_statistic_uri(),
         // this method will return a URI to the associated station object. 
         ////////////////////////////////////////////////////////////
         StrUni get_statistic_station(StrUni const &statistic_uri);

         ////////////////////////////////////////////////////////////
         // start_set_value
         //
         // Initiates the process of changing the variable referred to by the
         // provided uri.  This method replaces the older make_value_setter()
         // in that it will work with any data source type and not just
         // Loggernet data sources.  When the operation is complete, the
         // specified sink's on_set_value_complete() will get called.
         //
         // The return value will be true if the source specified by the uri
         // exists and if it supports a set value attempt. 
         ////////////////////////////////////////////////////////////
         bool start_set_value(
            SinkBase *sink,
            StrUni const &uri,
            ValueSetter const &value);

         ////////////////////////////////////////////////////////////
         // breakdown_uri
         //
         // This method will break down the given URI into a collection of
         // pairs of symbol names and types.
         ////////////////////////////////////////////////////////////
         typedef SourceBase::symbols_type symbols_type;
         void breakdown_uri(
            symbols_type &symbols, StrUni const &uri, bool quote_embedded_periods = false);

         ////////////////////////////////////////////////////////////
         // make_table_uri
         //
         // Extracts the portion of the given URI to produce a URI for the
         // associated table.  An empty string will be returned if the
         // extraction will not work.
         ////////////////////////////////////////////////////////////
         StrUni make_table_uri(StrUni const &uri);

         ////////////////////////////////////////////////////////////
         // get_table_range
         ////////////////////////////////////////////////////////////
         void get_table_range(
            Csi::EventReceiver *client, StrUni const &uri)
         {
            source_handle source(find_source_uri(uri));
            if(source != 0)
               source->get_table_range(client, uri);
            else
            {
               GetTableRangeCompleteEvent::cpost(
                  client, uri, GetTableRangeCompleteEvent::outcome_no_source);
            }
         }

         ////////////////////////////////////////////////////////////
         // source_is_connected
         //
         // Evaluates whether the source for the specified URI is in a
         // connected state (ready to handle requests).  A value of true will
         // be returned if the source is found and is reported to be
         // connected.  Otherwise, the return value will be false. 
         ////////////////////////////////////////////////////////////
         bool source_is_connected(StrUni const &uri);
         
         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

         ////////////////////////////////////////////////////////////
         // is_classic_inlocs
         //
         // Evaluates whether the specified UIR references a classic inlocs
         // table for a LgrNet source.
         ////////////////////////////////////////////////////////////
         bool is_classic_inlocs(StrUni const &uri); 

      private:
         ////////////////////////////////////////////////////////////
         // clients
         ////////////////////////////////////////////////////////////
         typedef std::list<client_type *> clients_type;
         clients_type clients;

         ////////////////////////////////////////////////////////////
         // sources
         ////////////////////////////////////////////////////////////
         sources_type sources;

         ////////////////////////////////////////////////////////////
         // requests
         ////////////////////////////////////////////////////////////
         requests_type requests;

         ////////////////////////////////////////////////////////////
         // timer
         ////////////////////////////////////////////////////////////
         Csi::SharedPtr<OneShot> timer;

         ////////////////////////////////////////////////////////////
         // value_factory
         ////////////////////////////////////////////////////////////
         Csi::SharedPtr<Cora::Broker::ValueFactory> value_factory;

         ////////////////////////////////////////////////////////////
         // supervisor
         ////////////////////////////////////////////////////////////
         supervisor_handle supervisor;
      };
   };
};

#endif

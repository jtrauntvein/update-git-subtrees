/* Cora.LgrNet.BrokerBrowser.h

   Copyright (C) 2000, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 01 June 2000
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Cora_LgrNet_BrokerBrowser_h
#define Cora_LgrNet_BrokerBrowser_h

#include "Cora.Broker.TableBrowser.h"


namespace Cora
{
   namespace LgrNet
   {
      //@group class forward declarations
      class BrokerBrowser;
      //@endgroup

      ////////// class TableBrowserEx
      // Defines an extension of the Cora::Broker::TableBrowser class that includes information
      // derived from the data brokers enumerate transaction.
      class TableBrowserEx: public Cora::Broker::TableBrowser
      {
      private:
         //@group extended properties
         // Identifies attributes associated with data brokers that is delivered in the enumerate
         // data brokers transaction. 

         ////////// identifier
         uint4 identifier;

         ////////// name
         StrUni name;

         ////////// type
         uint4 type;
         //@endgroup

      public:
         ////////// constructor
         TableBrowserEx(uint4 identifier_,
                        StrUni const &name_,
                        uint4 type_);

         //@group extended properties access methods
         uint4 get_identifier() const { return identifier; }
         StrUni const &get_name() const { return name; }
         uint4 get_type() const { return type; }
         //@endgroup
      };

      ////////// class BrokerBrowserClient
      // Defines methods that provide event notification to a client of a broker browser transaction
      // object
      class BrokerBrowserClient: public Csi::InstanceValidator
      {
      public:
         ////////// on_all_started
         // This method will be called after the data broker enumeration transaction has been started,
         // the individual table definitions advise transaction have been started, the initial table
         // advise notifications have been received for each of these, and the table definitions have
         // been retrieved for each table. This is considered to be a steady state although further
         // changes can and will take place after this event is fired.
         virtual void on_all_started(BrokerBrowser *tran) { }

         ////////// on_broker_added
         // This method will be invoked when a new data broker has been added to the server.
         typedef Csi::SharedPtr<TableBrowserEx> table_browser_handle;
         virtual void on_broker_added(BrokerBrowser *tran,
                                      table_browser_handle &broker)
         { }

         ////////// on_broker_deleted
         // This method will be invoked when a data broker has been deleted from the server.
         virtual void on_broker_deleted(BrokerBrowser *tran,
                                        table_browser_handle &broker)
         { }

         ////////// on_table_added
         // This method will be invoked after a table has been added to a data broker and the table
         // definitions have been received.
         typedef Cora::Broker::TableDesc TableDesc;
         typedef Csi::SharedPtr<TableDesc> table_desc_handle;
         virtual void on_table_added(BrokerBrowser *tran,
                                     table_browser_handle &broker,
                                     table_desc_handle &table)
         { }

         ////////// on_table_deleted
         // This method will be invoked after a table has been deleted.
         virtual void on_table_deleted(BrokerBrowser *tran,
                                       table_browser_handle &broker,
                                       table_desc_handle &table)
         { }

         ////////// on_failure
         // Called when a failure has occurred that prevents the broker browser from performing its
         // work.
         enum failure_type
         {
            failure_unknown = 0,
            failure_connection_failed = 1,
            failure_invalid_logon = 2,
            failure_server_security = 3,
            failure_table_browser = 4, // a table browser object reported failure
         };
         virtual void on_failure(BrokerBrowser *tran, failure_type failure)
         { }
      };

      ////////// class BrokerBrowser
      // Defines an object that provides meta data regarding all brokers, tables, columns, and column
      // elements associated with a cora server. This class can establish its own account with the cora
      // server or it will clone an existing session with the server (specified by the start method).
      //
      // Use of this class consists of constructing an object, setting up properties, and then invoking
      // start(). When all of the initial information has been gathered from the server, the object
      // will invoke the client's on_all_started() method. It will also invoke on_broker_added() and
      // on_table_added() as brokers and tables are added both before and after on_all_started() has
      // been invoked. Once the client has invoked finish() or a failure occurs (the client will be
      // notified by a call to on_failure(), the transaction object will return to an inactive state in
      // which properties can be set and start() can be invoked anew.
      class BrokerBrowser:
         public ClientBase,
         public Cora::Broker::TableBrowserClient,
         public Csi::EvReceiver
      {
      public:
         ////////// enum broker_mask_type
         // Defines the masks that control which classes of brokers will be reported.
         enum broker_mask_type
         {
            broker_mask_active = 0x01,
            broker_mask_backup = 0x02,
            broker_mask_client_defined = 0x04,
            broker_mask_statistics = 0x08,
         };

      private:
         //@group properties declarations
         ////////// broker_mask
         // Controls which brokers (and associated tables, etc) will be exposed by this transaction.
         byte broker_mask;

         ////////////////////////////////////////////////////////////
         // send_temporaries
         //
         // Controls whether temporary tables should be reported. The default value is false.
         ////////////////////////////////////////////////////////////
         bool send_temporaries;
         //@endgroup
      
      public:
         ////////// constructor
         BrokerBrowser();

         ////////// destructor
         virtual ~BrokerBrowser();

         //@group properties set methods
         // These methods will succeed only while the transaction is in a standby (before start)
         // state. 

         ////////// add_broker_type
         // Enables brokers of the type specified by mask to be reported
         void add_broker_type(broker_mask_type mask);

         ////////// remove_broker_type
         // Disables brokers of the type specified by mask from being reported.
         void remove_broker_type(broker_mask_type mask);

         ////////// set_broker_mask
         // Directly sets the broker mask. Values supplied should be the results of and'ing and or'ing
         // broker_mask_type values together.
         void set_broker_mask(byte broker_mask_);

         ////////////////////////////////////////////////////////////
         // set_send_temporaries
         ////////////////////////////////////////////////////////////
         void set_send_temporaries(bool send_temporaries_);

         ////////////////////////////////////////////////////////////
         // get_send_temporaries
         ////////////////////////////////////////////////////////////
         bool get_send_temporaries() const { return send_temporaries; }
         //@endgroup

         ////////// start
         // Called to start the transaction. The client and the router must be valid. The value of
         // default_net_session must either be zero (the default) or must be a legitimate opened
         // session. This method must also be called while the transaction objet is in a standby
         // state. If any of these conditions is not met, a std::invalid_argument or exc_invalid_state
         // exception will be thrown..
         void start(
            BrokerBrowserClient *client_,
            router_handle &router);
         void start(
            BrokerBrowserClient *client_,
            ClientBase *other_component);

         ////////// finish
         // Called to end the transaction and place it back into a standby state. If the transaction is
         // already in a standby state, this method will have no effect
         virtual void finish();

         //@group broker container access methods
         typedef Csi::SharedPtr<TableBrowserEx> broker_handle;
         typedef std::list<broker_handle> brokers_type;
         typedef brokers_type::const_iterator const_iterator;
         typedef brokers_type::const_reverse_iterator const_reverse_iterator;
         typedef brokers_type::size_type size_type;
         
         ////////// begin
         const_iterator begin() const { return brokers.begin(); }

         ////////// end
         const_iterator end() const { return brokers.end(); }

         ////////// rbegin
         const_reverse_iterator rbegin() const { return brokers.rbegin(); }

         ////////// rend
         const_reverse_iterator rend() const { return brokers.rend(); }

         ////////// size
         size_type size() const { return brokers.size(); }

         ////////// front
         broker_handle const &front() const { return brokers.front(); }

         ////////// back
         broker_handle const &back() const { return brokers.back(); }

         ////////// empty
         bool empty() const { return brokers.empty(); }
         //@endgroup

         ////////// find_broker_by_id
         // Searches for the specified data broker object (table browser) that has a numeric id that
         // matches that specified. If the broker is found, the dest parameter will be assigned to
         // the match and a true value will be returned. If a match cannot be found, a false value
         // will be returned and the dest parameter will be bound to a null pointer.
         bool find_broker_by_id(broker_handle &dest, uint4 broker_id);

         ////////////////////////////////////////////////////////////
         // find_active_broker_by_name
         ////////////////////////////////////////////////////////////
         bool find_active_broker_by_name(broker_handle &broker, StrUni const &broker_name);

      protected:
         //@group Methods overloaded from class TableBrowserClient
         ////////// on_all_started
         virtual void on_all_started(Cora::Broker::TableBrowser *broker);

         ////////// on_table_added
         virtual void on_table_added(Cora::Broker::TableBrowser *broker,
                                     Csi::SharedPtr<Cora::Broker::TableDesc> &table);

         ////////// on_table_deleted
         virtual void on_table_deleted(Cora::Broker::TableBrowser *broker,
                                       Csi::SharedPtr<Cora::Broker::TableDesc> &table);

         ////////// on_failure
         virtual void on_failure(Cora::Broker::TableBrowser *broker,
                                 Cora::Broker::TableBrowserClient::failure_type failure);
         //@endgroup

         //@group Methods overloaded from class ClientBase
         ////////// onNetMessage
         virtual void onNetMessage(
            Csi::Messaging::Router *rtr,
            Csi::Messaging::Message *msg);

         ////////// on_corabase_ready
         virtual void on_corabase_ready();

         ////////// on_corabase_failure
         virtual void on_corabase_failure(corabase_failure_type failure);

         ////////// on_corabase_session_failure
         virtual void on_corabase_session_failure();
         //@endgroup

         ////////// receive
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

      private:
         ////////// find_broker
         // Searches for a broker by pointer. Returns true if the specified pointer was found
         bool find_broker(Cora::Broker::TableBrowser *key, broker_handle &broker);

         ////////// on_data_brokers_enum_not
         void on_data_brokers_enum_not(Csi::Messaging::Message *msg);

         ////////// on_broker_added
         // Called when a new broker has been detected
         void on_broker_added(uint4 identifier, 
                              uint4 type,
                              StrUni const &name,
                              bool should_expand);

         ////////// on_broker_changed
         // Called when the type of a broker has been changed
         void on_broker_changed(uint4 identifier,
                                uint4 type,
                                StrUni const &name,
                                bool should_expand);

         ////////// on_broker_deleted
         void on_broker_deleted(uint4 identifier);

      private:
         ////////// state
         // Identifies the internal state of this class.
         enum state_type
         {
            state_standby,      // ready to recieve properties and start()
            state_attaching,    // waiting for ClientBase to logon
            state_first_set,    // gathering initial network information
            state_steady,       // in a steady state (all initial information gathered)
         } state;

         ////////// brokers
         // List of all data brokers currently known
         brokers_type brokers;

         ////////// first_set_wait_count
         // The number of brokers that we are still waiting for in the first_set state
         uint4 first_set_wait_count;

         ////////// client
         BrokerBrowserClient *client;
      };
   };
};

#endif

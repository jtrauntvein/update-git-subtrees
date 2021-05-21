/* Cora.Broker.TableDataIndexGetter.h

   Copyright (C) 2001, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 18 June 2001
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Cora_Broker_TableDataIndexGetter_h
#define Cora_Broker_TableDataIndexGetter_h

#include "Cora.Broker.BrokerBase.h"
#include "CsiEvents.h"
#include "Csi.InstanceValidator.h"
#include <list>


namespace Cora
{
   namespace Broker
   {
      //@group class forward declarations
      class TableDataIndexGetter;
      //@endgroup


      ////////////////////////////////////////////////////////////
      // class TableDataIndexGetterClient
      ////////////////////////////////////////////////////////////
      class TableDataIndexGetterClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // struct index_record_type
         //
         // Defines the values that will be kept in the index record list. 
         ////////////////////////////////////////////////////////////
         struct index_record_type
         {
            uint4 file_mark_no;
            uint4 begin_record_no;
            uint4 end_record_no;
            int8 begin_stamp;
            int8 end_stamp;
         };
         typedef std::list<index_record_type> index_records_type;

         ////////////////////////////////////////////////////////////
         // on_complete
         //
         // Called when the server transaction has been completed. If the outcome is successful, the
         // index_records parameter will contain the list of index records. Otherwise, it will be an
         // empty list. 
         ////////////////////////////////////////////////////////////
         enum outcome_type
         {
            outcome_unknown = 0,
            outcome_success = 1,
            outcome_invalid_logon = 2,
            outcome_unsupported = 3,
            outcome_server_security_blocked = 4,
            outcome_invalid_station_name = 5,
            outcome_invalid_table_name = 6,
            outcome_connection_failed = 7,
         };
         virtual void on_complete(
            TableDataIndexGetter *getter,
            outcome_type outcome,
            index_records_type const &index_records) = 0;
      };


      ////////////////////////////////////////////////////////////
      // class TableDataIndexGetter
      //
      // Defines a component that allows the client to get an index for each file mark in a
      // specified cache table.
      //
      // An application can use this component by creating an object that is derived from class
      // TableDataIndexGetterClient (the client) and an object of this class. After setting the
      // appropriate settings (set_station_name(), set_table_name(), etc.), the client should invoke
      // the start() method. When the index has been gathered, the client's on_complete() method
      // will be complete. 
      ////////////////////////////////////////////////////////////
      class TableDataIndexGetter:
         public BrokerBase,
         public Csi::EvReceiver
      {
      private:
         ////////////////////////////////////////////////////////////
         // table_name
         ////////////////////////////////////////////////////////////
         StrUni table_name;

      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         TableDataIndexGetter();

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~TableDataIndexGetter();

         ////////////////////////////////////////////////////////////
         // set_table_name
         ////////////////////////////////////////////////////////////
         void set_table_name(StrUni const &table_name);

         ////////////////////////////////////////////////////////////
         // get_table_name
         ////////////////////////////////////////////////////////////
         StrUni const &get_table_name() const { return table_name; }

         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         typedef TableDataIndexGetterClient client_type;
         void start(
            client_type *client_,
            router_handle &router);
         void start(
            client_type *client,
            ClientBase *other_component);

         ////////////////////////////////////////////////////////////
         // finish
         ////////////////////////////////////////////////////////////
         void finish();

      protected:
         //@group methods inherited from class BrokerBase
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
         //@endgroup

         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

      private:
         ////////////////////////////////////////////////////////////
         // state
         ////////////////////////////////////////////////////////////
         enum state_type
         {
            state_standby,
            state_delegate,
            state_active,
         } state;

         ////////////////////////////////////////////////////////////
         // client
         ////////////////////////////////////////////////////////////
         TableDataIndexGetterClient *client;
      };
   };
};


#endif

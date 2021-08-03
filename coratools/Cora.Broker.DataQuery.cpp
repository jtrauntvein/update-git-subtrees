/* Cora.Broker.DataQuery.cpp

   Copyright (C) 2009, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 19 February 2009
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Broker.DataQuery.h"
#include "coratools.strings.h"


namespace Cora
{
   namespace Broker
   {
      namespace
      {
         ////////////////////////////////////////////////////////////
         // class event_started
         ////////////////////////////////////////////////////////////
         class event_started: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // record
            ////////////////////////////////////////////////////////////
            typedef DataQuery::client_type::record_handle record_handle;
            record_handle record;
            
            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(DataQuery *query, record_handle &record)
            {
               event_started *event = new event_started(query, record);
               event->post();
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_started(DataQuery *query, record_handle &record_):
               Event(event_id, query),
               record(record_)
            { }
         };


         uint4 const event_started::event_id(
            Csi::Event::registerType("Cora::Broker::DataQuery::event_started"));


         ////////////////////////////////////////////////////////////
         // class event_complete
         ////////////////////////////////////////////////////////////
         class event_complete: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // client
            ////////////////////////////////////////////////////////////
            typedef DataQuery::client_type client_type;
            client_type *client;
            
            ////////////////////////////////////////////////////////////
            // outcome
            ////////////////////////////////////////////////////////////
            typedef client_type::outcome_type outcome_type;
            outcome_type outcome;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(DataQuery *query, client_type *client, outcome_type outcome)
            {
               event_complete *event = new event_complete(query, client, outcome);
               event->post();
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_complete(DataQuery *query, client_type *client_, outcome_type outcome_):
               Event(event_id, query),
               client(client_),
               outcome(outcome_)
            { }
         };


         uint4 const event_complete::event_id(
            Csi::Event::registerType("Cora::Broker::DataQuery::event_complete"));


         ////////////////////////////////////////////////////////////
         // class event_records
         ////////////////////////////////////////////////////////////
         class event_records: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // more_records_expected
            ////////////////////////////////////////////////////////////
            bool const more_records_expected;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(DataQuery *query, bool more_records_expected)
            {
               event_records *event = new event_records(query, more_records_expected);
               event->post();
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_records(DataQuery *query, bool more_records_expected_):
               Event(event_id, query),
               more_records_expected(more_records_expected_)
            { }
         };


         uint4 const event_records::event_id(
            Csi::Event::registerType("Cora::Broker::DataQuery::event_records"));
      };

      
      ////////////////////////////////////////////////////////////
      // class DataQuery definitions
      ////////////////////////////////////////////////////////////
      DataQuery::DataQuery():
         query_option(query_not_set),
         start_file_mark(0),
         start_record_no(0),
         end_file_mark(0),
         end_record_no(0),
         client(0),
         state(state_standby)
      { }


      DataQuery::~DataQuery()
      { finish(); }


      void DataQuery::set_query_record_no(
         uint4 start_file_mark_, uint4 start_record_no_, uint4 end_file_mark_, uint4 end_record_no_)
      {
         if(state == state_standby)
         {
            start_file_mark = start_file_mark_;
            start_record_no = start_record_no_;
            end_file_mark = end_file_mark_;
            end_record_no = end_record_no_;
            query_option = query_record_no;
         }
         else
            throw exc_invalid_state();
      } // set_query_record_no


      void DataQuery::set_query_date(
         Csi::LgrDate const &begin_date_, Csi::LgrDate const &end_date_)
      {
         if(state == state_standby)
         {
            start_time = begin_date_;
            end_time = end_date_;
            query_option = query_date;
         }
         else
            throw exc_invalid_state();
      } // set_query_date


      void DataQuery::set_table_name(StrUni const &table_name_)
      {
         if(state != state_standby)
            throw exc_invalid_state();
         table_name = table_name_;
      } // set_table_name


      void DataQuery::set_value_factory(Csi::SharedPtr<ValueFactory> value_factory_)
      {
         if(state != state_standby)
            throw exc_invalid_state();
         value_factory = value_factory;
      } // set_value_factory
      

      void DataQuery::start(
         client_type *client_, ClientBase *other_client)
      {
         if(state != state_standby)
            throw exc_invalid_state();
         if(!client_type::is_valid_instance(client_))
            throw std::invalid_argument("invalid client pointer");
         if(query_option == query_not_set)
            throw std::invalid_argument("invalid query option");
         if(value_factory == 0)
            value_factory.bind(new ValueFactory);
         client = client_;
         BrokerBase::start(other_client);
      } // start


      void DataQuery::start(
         client_type *client_, router_handle router)
      {
         if(state != state_standby)
            throw exc_invalid_state();
         if(!client_type::is_valid_instance(client_))
            throw std::invalid_argument("invalid client pointer");
         if(query_option == query_not_set)
            throw std::invalid_argument("invalid query option");
         client = client_;
         if(value_factory == 0)
            value_factory.bind(new ValueFactory);
         BrokerBase::start(router);
      } // start


      void DataQuery::finish()
      {
         state = state_standby;
         client = 0;
         records.clear();
         record_desc.clear();
         BrokerBase::finish();
      } // finish


      void DataQuery::format_outcome(
         std::ostream &out, client_type::outcome_type outcome)
      {
         using namespace DataQueryStrings;
         switch(outcome)
         {
         case client_type::outcome_success:
            out << my_strings[strid_success];
            break;
            
         case client_type::outcome_connection_failed:
            BrokerBase::format_failure(out, brokerbase_failure_session);
            break;
            
         case client_type::outcome_invalid_logon:
            BrokerBase::format_failure(out, brokerbase_failure_logon);
            break;
            
         case client_type::outcome_invalid_station_name:
            BrokerBase::format_failure(out, brokerbase_failure_invalid_id);
            break;
            
         case client_type::outcome_invalid_table_name:
            out << my_strings[strid_invalid_table_name];
            break;
            
         case client_type::outcome_server_security_failed:
            BrokerBase::format_failure(out, brokerbase_failure_security);
            break;
            
         case client_type::outcome_unsupported:
            BrokerBase::format_failure(out, brokerbase_failure_unsupported);
            break;
            
         case client_type::outcome_invalid_range:
            out << my_strings[strid_invalid_range];
            break;
            
         default:
            BrokerBase::format_failure(out, brokerbase_failure_unknown);
            break;
         }
      } // format_outcome


      void DataQuery::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         if(ev->getType() == event_started::event_id)
         {
            event_started *event = static_cast<event_started *>(ev.get_rep());
            if(client_type::is_valid_instance(client))
               client->on_started(this, event->record);
            else
               finish();
         }
         else if(ev->getType() == event_records::event_id)
         {
            event_records *event = static_cast<event_records *>(ev.get_rep());
            if(client_type::is_valid_instance(client))
            {
               if(event->more_records_expected)
               {
                  Csi::Messaging::Message cont_cmd(broker_session, Messages::data_query_cont_cmd);
                  cont_cmd.addUInt4(last_tran_no);
                  cont_cmd.addBool(false);
                  router->sendMessage(&cont_cmd);
               }
               client->on_records(this, records, event->more_records_expected);
            }
            else
               finish();
         } 
         else if(ev->getType() == event_complete::event_id)
         {
            event_complete *event = static_cast<event_complete *>(ev.get_rep());
            client_type *client = this->client;
            finish();
            if(event->client == client && client_type::is_valid_instance(client))
               client->on_complete(this, event->outcome);
         }
      } // receive


      void DataQuery::on_brokerbase_ready()
      {
         Csi::Messaging::Message cmd(broker_session, Messages::data_query_cmd);
         cmd.addUInt4(++last_tran_no);
         cmd.addWStr(table_name);
         cmd.addUInt4(1024);    // max records to send
         cmd.addBool(true);     // do server translation
         cmd.addUInt4(query_option);
         cmd.addUInt4(0);       // send the whole record
         cmd.addUInt4(start_file_mark);
         cmd.addUInt4(start_record_no);
         cmd.addUInt4(end_file_mark);
         cmd.addUInt4(end_record_no);
         cmd.addInt8(start_time.get_nanoSec());
         cmd.addInt8(end_time.get_nanoSec());
         state = state_active;
         router->sendMessage(&cmd);
      } // on_brokerbase_ready


      void DataQuery::on_brokerbase_failure(brokerbase_failure_type failure)
      {
         client_type::outcome_type outcome = client_type::outcome_unknown;
         switch(failure)
         {
         case brokerbase_failure_logon:
            outcome = client_type::outcome_invalid_logon;
            break;
            
         case brokerbase_failure_session:
            outcome = client_type::outcome_connection_failed;
            break;
            
         case brokerbase_failure_invalid_id:
            outcome = client_type::outcome_invalid_station_name;
            break;
            
         case brokerbase_failure_unsupported:
            outcome = client_type::outcome_unsupported;
            break;
            
         case brokerbase_failure_security:
            outcome = client_type::outcome_server_security_failed;
            break;
         }
         event_complete::cpost(this, client, outcome);
      } // on_brokerbase_failure


      void DataQuery::onNetMessage(
         Csi::Messaging::Router *router, Csi::Messaging::Message *message)
      {
         if(state == state_active)
         {
            switch(message->getMsgType())
            {
            case Messages::data_query_ack:
               on_start_ack(message);
               break;
               
            case Messages::data_query_return_recs:
               on_return_records(message);
               break;
               
            default:
               BrokerBase::onNetMessage(router, message);
               break;
            }
         }
         else
            BrokerBase::onNetMessage(router, message);
      } // onNetMessage;


      void DataQuery::on_start_ack(Csi::Messaging::Message *message)
      {
         uint4 tran_no;
         uint4 rcd;
         message->readUInt4(tran_no);
         message->readUInt4(rcd);
         if(rcd == 1)
         {
            try
            {
               // read the record description
               record_desc.bind(new RecordDesc(get_broker_name(), table_name));
               record_desc->read(*message);
               records.clear();
               records.push_back(new Record(record_desc, *value_factory));
               event_started::cpost(this, records.front());
            }
            catch(std::exception &)
            { event_complete::cpost(this, client, client_type::outcome_unknown); } 
         }
         else
         {
            client_type::outcome_type outcome = client_type::outcome_unknown;
            switch(rcd)
            {
            case 2:
               outcome = client_type::outcome_invalid_table_name;
               break;
               
            case 4:
               outcome = client_type::outcome_invalid_range;
               break;
            }
            event_complete::cpost(this, client, outcome);
         }
      } // on_start_ack


      void DataQuery::on_return_records(Csi::Messaging::Message *message)
      {
         uint4 tran_no;
         uint4 rcd;
         uint4 count;
         client_type::records_type cache(records);
         
         message->readUInt4(tran_no);
         message->readUInt4(rcd);
         message->readUInt4(count);
         records.clear();
         try
         {
            if(rcd == 1 || rcd == 2)
            {
               for(uint4 i = 0; i < count; ++i)
               {
                  // get a new record from the cache or create it
                  client_type::record_handle record;
                  if(!cache.empty())
                  {
                     record = cache.front();
                     cache.pop_front();
                  }
                  else
                     record.bind(new Record(record_desc, *value_factory));
                  record->read(*message);
                  if(query_option != query_date || record->get_stamp() < end_time)
                     records.push_back(record);
                  else
                     cache.push_back(record);
               }
               event_records::cpost(this, rcd == 1);
            }
         }
         catch(std::exception &)
         { rcd = 0; }

         if(rcd != 1)
         {
            client_type::outcome_type outcome = client_type::outcome_unknown;
            switch(rcd)
            {
            case 2:
               outcome = client_type::outcome_success;
               break;
               
            case 3:
               outcome = client_type::outcome_invalid_table_name;
               break;
            }
            event_complete::cpost(this, client, outcome);
         }
      } // on_return_records
   };
};


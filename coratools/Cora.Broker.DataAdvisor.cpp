/* Cora.Broker.DataAdvisor.cpp

   Copyright (C) 2000, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 17 April 2000
   Last Change: Friday 29 April 2016
   Last Commit: $Date: 2016-05-03 16:18:03 -0600 (Tue, 03 May 2016) $ 
   Committed by: $author$
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Broker.DataAdvisor.h"
#include "Cora.Defs.h"
#include "Cora.Broker.RecordDesc.h"
#include "Cora.Broker.ValueFactory.h"
#include "Cora.Broker.Value.h"
#include "Cora.Broker.ValueName.h"
#include "Csi.Xml.Element.h"
#include "Csi.ArrayDimensions.h"
#include "coratools.strings.h"
#include <assert.h>
#include <algorithm>
#include <iostream>
#include <iterator>


namespace Cora
{
   namespace Broker
   {
      namespace DataAdvisorHelpers
      {
         ////////////////////////////////////////////////////////////
         // class event_base
         ////////////////////////////////////////////////////////////
         class event_base: public Csi::Event
         {
         protected:
            DataAdvisor *advisor;
            DataAdvisorClient *client;
            friend class Cora::Broker::DataAdvisor;

         public:
            event_base(uint4 event_id,
                       DataAdvisor *advisor_,
                       DataAdvisorClient *client_):
               Event(event_id,advisor_),
               advisor(advisor_),
               client(client_)
            { }

            virtual void notify() = 0;
         };


         ////////////////////////////////////////////////////////////
         // class event_ready
         ////////////////////////////////////////////////////////////
         class event_ready: public event_base
         {
         public:
            static uint4 const event_id;

            static void create_and_post(DataAdvisor *tran, DataAdvisorClient *client);

            virtual void notify()
            { client->on_advise_ready(advisor); }

         private:
            event_ready(DataAdvisor *tran_, DataAdvisorClient *client_):
               event_base(event_id,tran_,client_)
            { }
         };


         uint4 const event_ready::event_id =
         Csi::Event::registerType("Cora::Broker::DataAdvisorHelpers::event_ready");


         void event_ready::create_and_post(DataAdvisor *tran, DataAdvisorClient *client)
         {
            try { (new event_ready(tran,client))->post(); }
            catch(Csi::Event::BadPost &) { }
         } // create_and_post


         ////////////////////////////////////////////////////////////
         // class event_failure
         ////////////////////////////////////////////////////////////
         class event_failure: public event_base
         {
         public:
            static uint4 const event_id;
            DataAdvisorClient::failure_type failure;
         
         public:
            static void create_and_post(DataAdvisor *tran,
                                        DataAdvisorClient *client,
                                        DataAdvisorClient::failure_type failure);

            virtual void notify()
            { client->on_advise_failure(advisor,failure); }

         private:
            event_failure(DataAdvisor *tran_,
                          DataAdvisorClient *client_,
                          DataAdvisorClient::failure_type failure_):
               event_base(event_id,tran_,client_),
               failure(failure_)
            { }
         };


         uint4 const event_failure::event_id =
         Csi::Event::registerType("Cora::Broker::DataAdvisor::event_failure");


         void event_failure::create_and_post(DataAdvisor *tran,
                                             DataAdvisorClient *client,
                                             DataAdvisorClient::failure_type failure)
         {
            try { (new event_failure(tran,client,failure))->post(); }
            catch(Csi::Event::BadPost &) { }
         } // create_and_post


         ////////////////////////////////////////////////////////////
         // class event_advise_record
         ////////////////////////////////////////////////////////////
         class event_advise_record: public event_base
         {
         public:
            static uint4 const event_id;

         public:
            static void create_and_post(DataAdvisor *tran, DataAdvisorClient *client);

            virtual void notify()
            { client->on_advise_record(advisor); }

         private:
            event_advise_record(DataAdvisor *tran_, DataAdvisorClient *client_):
               event_base(event_id,tran_,client_)
            { }
         };


         uint4 const event_advise_record::event_id =
         Csi::Event::registerType("Cora::Broker::DataAdvisor::event_advise_record");


         void event_advise_record::create_and_post(DataAdvisor *tran, DataAdvisorClient *client)
         {
            try { (new event_advise_record(tran,client))->post(); }
            catch(Csi::Event::BadPost &) {}
         } // create_and_post
      };


      ////////////////////////////////////////////////////////////
      // class DataAdvisor definitions
      ////////////////////////////////////////////////////////////
      DataAdvisor::DataAdvisor():
         client(0),
         advise_tran(0),
         state(state_standby),
         start_option(start_at_newest),
         order_option(order_real_time),
         start_record_no(0),
         start_file_mark_no(0),
         start_date(0),
         start_record_offset(0),
         cache_size_controller(1),
         actual_start_file_mark(0),
         actual_start_record_no(0)
      { value_factory.bind(new ValueFactory); }


      DataAdvisor::~DataAdvisor()
      { finish(); }


      void DataAdvisor::set_table_name(StrUni const &table_name_)
      {
         if(state == state_standby)
            table_name = table_name_;
         else
            throw exc_invalid_state();
      } // set_table_name

   
      void DataAdvisor::set_start_option(start_option_type start_option_)
      {
         if(state == state_standby)
            start_option = start_option_;
         else
            throw exc_invalid_state();
      } // set_start_option

   
      void DataAdvisor::set_order_option(order_option_type order_option_)
      {
         if(state == state_standby)
            order_option = order_option_;
         else
            throw exc_invalid_state();
      } // set_order_option

   
      void DataAdvisor::set_start_record_no(uint4 start_record_no_)
      {
         if(state == state_standby)
            start_record_no = actual_start_record_no = start_record_no_;
         else
            throw exc_invalid_state();
      } // set_start_record_no

   
      void DataAdvisor::set_start_file_mark_no(uint4 start_file_mark_no_)
      {
         if(state == state_standby)
            start_file_mark_no = actual_start_file_mark = start_file_mark_no_;
         else
            throw exc_invalid_state();
      } // set_start_file_mark_no

   
      void DataAdvisor::set_start_date(int8 start_date_)
      {
         if(state == state_standby)
            start_date = start_date_;
         else
            throw exc_invalid_state();
      } // set_start_date


      void DataAdvisor::set_start_interval(int8 start_interval_)
      {
         if(state == state_standby)
            start_interval = start_interval_;
         else
            throw exc_invalid_state();
      } // set_start_interval

   
      void DataAdvisor::set_value_factory(Csi::SharedPtr<ValueFactory> &value_factory_)
      {
         if(state == state_standby)
            value_factory = value_factory_;
         else
            throw exc_invalid_state();
      } // set_value_factory


      void DataAdvisor::set_cache_size_controller(uint4 cache_size_controller_)
      {
         if(state == state_standby)
            cache_size_controller = cache_size_controller_;
         else
            throw exc_invalid_state();
      } // set_cache_size_controller


      void DataAdvisor::set_start_record_offset(uint4 start_record_offset_)
      {
         if(state == state_standby)
            start_record_offset = start_record_offset_;
         else
            throw exc_invalid_state();
      } // set_start_record_offset

      
      void DataAdvisor::clear_columns()
      {
         if(state == state_standby)
            selectors.clear();
         else
            throw exc_invalid_state();
      } // clear_columns


      void DataAdvisor::add_column(StrUni const &selector)
      {
         if(state == state_standby)
         {
            selectors_type::iterator si(
               std::find(selectors.begin(), selectors.end(), selector));
            if(si == selectors.end())
               selectors.push_back(selector);
         }
         else
            throw exc_invalid_state();
      } // add_column

   
      void DataAdvisor::start(
         DataAdvisorClient *client_,
         router_handle &router_)
      {
         if(state == state_standby)
         {
            if(DataAdvisorClient::is_valid_instance(client_))
            {
               current_record.clear();
               unread_records.clear();
               recycled_records.clear();
               report_new_records = false;
               state = state_delegate;
               client = client_;
               BrokerBase::start(router_);
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start


      void DataAdvisor::start(
         DataAdvisorClient *client_,
         ClientBase *other_client)
      {
         if(state == state_standby)
         {
            if(DataAdvisorClient::is_valid_instance(client_))
            {
               current_record.clear();
               unread_records.clear();
               recycled_records.clear();
               report_new_records = false;
               state = state_delegate;
               client = client_;
               BrokerBase::start(other_client);
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start


      void DataAdvisor::finish()
      {
         state = state_standby;
         client = 0;
         advise_tran = 0;
         unread_records.clear();
         recycled_records.clear();
         report_new_records = false;
         index_getter.clear();
         current_record.clear();
         BrokerBase::finish();
      } // finish


      void DataAdvisor::format_failure(
         std::ostream &out, client_type::failure_type failure)
      {
         using namespace DataAdvisorStrings;
         switch(failure)
         {
         default:
         case client_type::failure_unknown:
            ClientBase::describe_failure(out, corabase_failure_unknown);
            break;
                     
         case client_type::failure_connection_failed:
            ClientBase::describe_failure(out, corabase_failure_session);
            break;
            
         case client_type::failure_invalid_logon:
            ClientBase::describe_failure(out, corabase_failure_logon);
            break;
            
         case client_type::failure_invalid_station_name:
            BrokerBase::format_failure(out, brokerbase_failure_invalid_id); 
            break;
            
         case client_type::failure_invalid_table_name:
            out << my_strings[strid_invalid_table_name];
            break;
            
         case client_type::failure_server_security:
            ClientBase::describe_failure(out, corabase_failure_security);
            break;
            
         case client_type::failure_invalid_start_option:
            out << my_strings[strid_invalid_start_option];
            break;
            
         case client_type::failure_invalid_order_option:
            out << my_strings[strid_invalid_order_option];
            break;
            
         case client_type::failure_table_deleted:
            out << my_strings[strid_table_deleted];
            break;
            
         case client_type::failure_station_shut_down:
            out << my_strings[strid_station_shut_down];
            break;
            
         case client_type::failure_unsupported:
            ClientBase::describe_failure(out, corabase_failure_unsupported);
            break;
            
         case client_type::failure_invalid_column_name:
            out << my_strings[strid_invalid_column_name];
            break;
            
         case client_type::failure_invalid_array_address:
            out << my_strings[strid_invalid_array_address];
            break;
         }
      } // format_failure


      DataAdvisor::record_handle &DataAdvisor::get_record()
      {
         if(current_record == 0)
            throw exc_invalid_state();
         if(!unread_records.empty())
            current_record = unread_records.front();
         return current_record;
      } // get_record


      void DataAdvisor::format_record_ldep(std::ostream &out)
      {
         if(current_record.get_rep())
         {
            out << '\"' << current_record->get_broker_name() << "\",\""
                << current_record->get_table_name()
                << "\",\"";
            current_record->get_stamp().format(out,"%Y-%m-%d %H:%M:%S%x\",\"");
            out << current_record->get_record_no();
            for(Record::iterator fi = current_record->begin();
                fi != current_record->end();
                fi++)
            {
               // format the field
               out << "\",\"";
               (*fi)->format_name(out);
               out << "\",\"";
               (*fi)->format_ldep_type(out);
               out << "\",\"";
               (*fi)->format(out);
               
               // add a carraige return if this is the last field
               Record::iterator next_fi = fi;
               if(++next_fi == current_record->end())
                  out << "\"\r\n";
            }
         }
         else
            throw exc_invalid_state();
      } // format_ldep_record


      namespace
      {
         StrUni const record_name(L"record");
         StrUni const station_name(L"station");
         StrUni const table_name_name(L"table");
         StrUni const record_no_name(L"record-no");
         StrUni const time_name(L"time");
         char const *stamp_format = "%Y-%m-%dT%H:%M:%S%x"; 
         StrUni const field_name(L"field");
         StrUni const field_name_name(L"name");
         StrUni const field_type_name(L"type");
         StrUni const field_process_name(L"process");
         StrUni const field_units_name(L"units");
         struct do_format_field
         {
            Csi::Xml::Element &record_xml;
            do_format_field(Csi::Xml::Element &record_xml_):
               record_xml(record_xml_)
            { }
            void operator ()(Record::value_type value)
            {
               Csi::Xml::Element::value_type field_xml(record_xml.add_element(field_name));
               Csi::OStrAscStream temp;
               
               value->format_name_ex(temp,true,"(","",",",")");
               field_xml->set_attr_str(temp.str(),field_name_name);
               field_xml->set_attr_str(xml_type_string(value->get_type()),field_type_name);
               field_xml->set_attr_wstr(value->get_units_string(),field_units_name);
               field_xml->set_attr_wstr(value->get_process_string(),field_process_name);
               temp.str("");
               value->format(temp,false,stamp_format);
               field_xml->set_cdata_str(temp.str());
            }
         };
      };

      
      void DataAdvisor::format_record_ldep_xml(std::ostream &out)
      {
         using Csi::Xml::Element;
         Element record_xml(record_name);
         Csi::OStrAscStream temp; 
         
         record_xml.set_attr_wstr(get_broker_name(), station_name);
         record_xml.set_attr_wstr(get_table_name(), table_name_name);
         record_xml.set_attr_uint4(current_record->get_record_no(), record_no_name);
         current_record->get_stamp().format(temp, stamp_format);
         record_xml.set_attr_str(temp.str(), time_name);
         std::for_each(
            current_record->begin(),
            current_record->end(),
            do_format_field(record_xml));
         record_xml.output(out,true);
      } // format_ldep_xml

      
      void DataAdvisor::get_next_record()
      {
         if(!unread_records.empty())
         {
            // In order to prevent unneccesary future record allocations, we can place the current
            // record object in the recycle queue so that it can be re-used.
            recycled_records.push_back(unread_records.front());
            while(recycled_records.size() > cache_size_controller)
               recycled_records.pop_front();
            unread_records.pop_front();
            report_new_records = true;
            if(!unread_records.empty())
            {
               report_new_records = false;
               DataAdvisorHelpers::event_advise_record::create_and_post(this,client);
            }
            else
               send_continue_command();
         }
         else
            throw exc_invalid_state();
      } // get_next_record


      void DataAdvisor::get_next_block()
      {
         if(!unread_records.empty())
         {
            std::copy(
               unread_records.begin(),
               unread_records.end(),
               std::back_inserter(recycled_records));
            unread_records.clear();
            while(recycled_records.size() > cache_size_controller)
               recycled_records.pop_front();
            report_new_records = true;
            send_continue_command();
         }
         else
            throw exc_invalid_state();
      } // get_next_block


      void DataAdvisor::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(state == state_active || state == state_before_active)
         {
            switch(msg->getMsgType())
            {
            case Cora::Broker::Messages::data_advise_start_ack:
               on_data_advise_start_ack(msg);
               break;

            case Cora::Broker::Messages::data_advise_start_ack_ex:
               on_data_advise_start_ack_ex(msg);
               break;
               
            case Cora::Broker::Messages::data_advise_not:
               on_data_advise_not(msg);
               break;

            default:
               BrokerBase::onNetMessage(rtr,msg);
               break;
            }
         }
         else
            BrokerBase::onNetMessage(rtr,msg);
      } // onNetMessage


      void DataAdvisor::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace DataAdvisorHelpers;
         event_base *event = dynamic_cast<event_base *>(ev.get_rep());

         assert(event != 0);
         if(event->client == client &&
            DataAdvisorClient::is_valid_instance(event->client))
         {
            if(event->getType() == event_failure::event_id)
               finish();
            event->notify();
         }
         else
            finish();
      } // receive


      void DataAdvisor::on_brokerbase_ready()
      {
         // if the start_at_record_offset option is selected, we need to get the table data index
         // first before we can start the advise transaction, otherwise, we can start the advise
         // transaction immediately.
         if(start_option == start_at_offset_from_newest)
         {
            index_getter.bind(new TableDataIndexGetter);
            if(get_broker_name().length() > 0)
               index_getter->set_open_broker_active_name(get_broker_name());
            else
               index_getter->set_open_broker_id(get_open_broker_id());
            index_getter->set_table_name(table_name);
            index_getter->start(this,this);
         }
         else
            start_advise_transaction(start_option);
         state = state_before_active;
      } // on_net_open_active_data_broker_ses_ack


      void DataAdvisor::on_brokerbase_failure(brokerbase_failure_type failure)
      {
         using namespace DataAdvisorHelpers;
         DataAdvisorClient::failure_type client_failure;
         switch(failure)
         {
         default:
            client_failure = DataAdvisorClient::failure_unknown;
            break;
            
         case brokerbase_failure_logon:
            client_failure = DataAdvisorClient::failure_invalid_logon;
            break;
            
         case brokerbase_failure_session:
            client_failure = DataAdvisorClient::failure_connection_failed;
            break;
            
         case brokerbase_failure_invalid_id:
            client_failure = DataAdvisorClient::failure_invalid_station_name;
            break;
            
         case brokerbase_failure_unsupported:
            client_failure = DataAdvisorClient::failure_unsupported;
            break;
            
         case brokerbase_failure_security:
            client_failure = DataAdvisorClient::failure_server_security;
            break;
         }
         event_failure::create_and_post(this,client,client_failure);
      } // on_brokerbase_failure


      void DataAdvisor::on_brokerbase_session_failure()
      {
         using namespace DataAdvisorHelpers;
         event_failure::create_and_post(
            this,
            client,
            DataAdvisorClient::failure_connection_failed);
      } // on_brokerbase_session_failure


      void DataAdvisor::on_data_advise_start_ack(Csi::Messaging::Message *msg)
      {
         using namespace DataAdvisorHelpers;
         uint4 tran_no;
         uint4 resp_code;
         
         msg->readUInt4(tran_no);
         msg->readUInt4(resp_code);
         if(resp_code == 1)
         {
            Cora::Broker::Record::desc_handle description(
               new Cora::Broker::RecordDesc(get_broker_name(),table_name));
            if(description->read(*msg))
            {
               current_record.bind(new Cora::Broker::Record(description,*value_factory));
               event_ready::create_and_post(this,client);
               state = state_active;
               report_new_records = true;
            }
            else
               event_failure::create_and_post(
                  this,client,DataAdvisorClient::failure_unknown);
         }
         else
         {
            DataAdvisorClient::failure_type failure;
            switch(resp_code)
            {
            case 2:
               failure = DataAdvisorClient::failure_invalid_table_name;
               break;
               
            case 3:
               failure = client_type::failure_invalid_column_name;
               break;
               
            case 4:
               if(interface_version < Csi::VersionNumber("1.3.4.68"))
                  failure = client_type::failure_invalid_column_name;
               else
                  failure = client_type::failure_invalid_order_option;
               break;
               
            case 5:
               failure = client_type::failure_invalid_start_option;
               break;

            case 7:
               failure = client_type::failure_invalid_array_address;
               break;
               
            default:
               failure = client_type::failure_unknown;
               break;
            }
            event_failure::create_and_post(this,client,failure);
         }
      } // on_data_advise_start_ack


      void DataAdvisor::on_data_advise_start_ack_ex(Csi::Messaging::Message *message)
      {
         using namespace DataAdvisorHelpers;
         uint4 tran_no;
         uint4 resp_code;
         
         message->readUInt4(tran_no);
         message->readUInt4(resp_code);
         if(resp_code == 1)
         {
            Record::desc_handle description(
               new RecordDesc(get_broker_name(), table_name));
            uint4 num_columns(0);
            StrUni column_name;
            uint4 column_data_type(0);
            uint4 column_modifying_command(0);
            StrUni column_units;
            StrUni column_process;
            StrUni column_description;
            Csi::ArrayDimensions column_dimensions;
            uint4 piece_size(0);
            uint4 piece_offset(0);
            uint4 num_dims(0);
            uint4 dim(0);
            
            message->readUInt4(num_columns);
            for(uint4 i = 0; i < num_columns; ++i)
            {
               // read the parameters for this piece
               message->readWStr(column_name);
               message->readUInt4(column_data_type);
               message->readUInt4(column_modifying_command);
               message->readWStr(column_units);
               message->readWStr(column_process);
               message->readWStr(column_description);
               message->readUInt4(num_dims);
               column_dimensions.clear();
               for(uint4 j = 0; j < num_dims; ++j)
               {
                  message->readUInt4(dim);
                  column_dimensions.add_dimension(dim);
               }
               message->readUInt4(piece_size);
               message->readUInt4(piece_offset);

               // we now need to add the values associated with this piece
               for(uint4 j = 0; j < piece_size; ++j)
               {
                  RecordDesc::value_type value(new ValueDesc);
                  value->name = column_name;
                  value->data_type = static_cast<CsiDbTypeCode>(column_data_type);
                  value->modifying_cmd = column_modifying_command;
                  value->units = column_units;
                  value->process = column_process;
                  value->description = column_description;
                  if(!column_dimensions.empty())
                  {
                     value->array_address.resize(column_dimensions.size());
                     column_dimensions.to_index(value->array_address.begin(), piece_offset + j);
                  }
                  description->values.push_back(value);
               }
            }
            
            // we are now ready to proceed
            current_record.bind(new Record(description, *value_factory));
            event_ready::create_and_post(this, client);
            state = state_active;
            report_new_records = true;
         }
         else
         {
            DataAdvisorClient::failure_type failure;
            switch(resp_code)
            {
            case 2:
               failure = DataAdvisorClient::failure_invalid_table_name;
               break;
               
            case 3:
               failure = client_type::failure_invalid_column_name;
               break;
               
            case 4:
               if(interface_version < Csi::VersionNumber("1.3.4.68"))
                  failure = client_type::failure_invalid_column_name;
               else
                  failure = client_type::failure_invalid_order_option;
               break;
               
            case 5:
               failure = client_type::failure_invalid_start_option;
               break;

            case 7:
               failure = client_type::failure_invalid_array_address;
               break;
               
            default:
               failure = client_type::failure_unknown;
               break;
            }
            event_failure::create_and_post(this, client, failure);
         }
      } // on_data_advise_start_ack_ex


      void DataAdvisor::on_data_advise_not(Csi::Messaging::Message *msg)
      {
         if(state == state_active)
         {
            using namespace DataAdvisorHelpers;
            uint4 tran_no;
            uint4 server_state;

            msg->readUInt4(tran_no);
            msg->readUInt4(server_state);
            if(server_state == 1)
            {
               // read all of the records in the notification and place them in the unread queue.
               uint4 num_records;
               bool can_use_current_record = report_new_records;

               msg->readUInt4(num_records);
               for(uint4 i = 0; i < num_records; ++i)
               {
                  // if a recycled record exists, we can use it. 
                  record_handle new_record;
                  if(!recycled_records.empty())
                  {
                     new_record = recycled_records.front();
                     recycled_records.pop_front();
                  }
                  // if the client is using the current record and the recycle queue is empty, we will
                  // have to create a new record.
                  else
                     new_record.bind(new Record(*current_record));

                  // we can now read the new record and place it in the unread queue
                  new_record->read(*msg);
                  unread_records.push_back(new_record);
               }

               // if the client is in a state to receive new records, we can notify it that at least
               // one new record is available
               if(report_new_records && !unread_records.empty())
               {
                  current_record = unread_records.front();
                  report_new_records = false;
                  event_advise_record::create_and_post(this,client);
               }
               else if(unread_records.empty())
                  send_continue_command();
            }
            else
            {
               DataAdvisorClient::failure_type failure;
               switch(server_state)
               {
               case 3: failure = DataAdvisorClient::failure_table_deleted; break;
               case 4: failure = DataAdvisorClient::failure_station_shut_down; break;
               default: failure = DataAdvisorClient::failure_unknown; break;
               }
               event_failure::create_and_post(this,client,failure);
            }
         }
      } // on_data_advise_not


      void DataAdvisor::send_continue_command()
      {
         Csi::Messaging::Message continue_command(
            broker_session,
            Messages::data_advise_cont_cmd);
         continue_command.addUInt4(advise_tran);
         continue_command.addBool(false); // don't abort the transaction
         router->sendMessage(&continue_command);
      } // send_continue_command


      void DataAdvisor::on_complete(
         TableDataIndexGetter *index_getter,
         outcome_type outcome,
         index_records_type const &index_records)
      {
         // make sure that the index getter is clear (we don't need it any more) 
         this->index_getter.clear();

         if(outcome == outcome_success)
         {
            // we will iterate through the index records in reverse order (newest to oldest) until
            // the size of the blocks iterated through is greater than or equal to
            // start_record_offset.
            index_records_type::const_reverse_iterator ri = index_records.rbegin();
            uint4 previous_blocks_size = 0;
            
            while(ri != index_records.rend() && previous_blocks_size < start_record_offset)
            {
               uint4 block_size = ri->end_record_no - ri->begin_record_no + 1;
               if(block_size + previous_blocks_size >= start_record_offset)
                  break;
               else
               {
                  previous_blocks_size += block_size;
                  ++ri;
               }
            }

            // The iterator will be valid iff we found an appropriate block to start in, otherwise,
            // we will start at the beginning of the table
            if(ri != index_records.rend())
            {
               uint4 remainder = start_record_offset - previous_blocks_size;
               
               actual_start_file_mark = ri->file_mark_no;
               actual_start_record_no = ri->end_record_no - remainder + 1;
            }
            else
               actual_start_record_no = actual_start_file_mark = 0;
            start_advise_transaction(start_at_record_id);
         }
         else
         {
            using namespace DataAdvisorHelpers;
            DataAdvisorClient::failure_type client_failure;
            switch(outcome)
            {
            case outcome_invalid_logon:
               client_failure = DataAdvisorClient::failure_invalid_logon;
               break;
               
            case outcome_unsupported:
               client_failure = DataAdvisorClient::failure_unsupported;
               break;
               
            case outcome_server_security_blocked:
               client_failure = DataAdvisorClient::failure_server_security;
               break;
               
            case outcome_invalid_station_name:
               client_failure = DataAdvisorClient::failure_invalid_station_name;
               break;
               
            case outcome_invalid_table_name:
               client_failure = DataAdvisorClient::failure_invalid_table_name;
               break;
               
            case outcome_connection_failed:
               client_failure = DataAdvisorClient::failure_connection_failed;
               break;
               
            default:
               client_failure = DataAdvisorClient::failure_unknown;
               break;
            }
            event_failure::create_and_post(this,client,client_failure);
         }
      } // on_complete


      void DataAdvisor::start_advise_transaction(start_option_type real_start_option)
      {
         try
         {
            // send the command to start the data advise
            Csi::Messaging::Message start_cmd(
               broker_session,
               Messages::data_advise_start_cmd);
            
            start_cmd.addUInt4(advise_tran = ++last_tran_no);
            start_cmd.addWStr(table_name);
            start_cmd.addUInt4(static_cast<uint4>(selectors.size()));
            for(selectors_type::iterator si = selectors.begin();
                si != selectors.end();
                ++si)
            {
               ValueName value(si->c_str());
               start_cmd.addWStr(value.get_column_name());
               start_cmd.addUInt4(static_cast<uint4>(value.size()));
               for(ValueName::iterator vi = value.begin();
                   vi != value.end();
                   ++vi)
                  start_cmd.addUInt4(*vi);
            }
            if(order_option == order_real_time || cache_size_controller <= 1)
               start_cmd.addUInt4(1);    // send only one record at a time
            else
               start_cmd.addUInt4(cache_size_controller);
            start_cmd.addUInt4(order_option);
            start_cmd.addBool(true);
            start_cmd.addUInt4(real_start_option);
            start_cmd.addUInt4(actual_start_file_mark);
            start_cmd.addUInt4(actual_start_record_no);
            if(real_start_option == start_relative_to_newest)
               start_cmd.addInt8(start_interval);
            else
               start_cmd.addInt8(start_date);
            if(interface_version >= Csi::VersionNumber("1.4.14"))
            {
               start_cmd.addBool(true); // send the description
               start_cmd.addBool(true); // send the extended ack message (if supported)
            }
            router->sendMessage(&start_cmd);
         }
         catch(ValueName::exc_invalid_syntax &)
         {
            using namespace DataAdvisorHelpers;
            event_failure::create_and_post(
               this,
               client,
               client_type::failure_invalid_column_name);
         }
      } // start_advise_transaction
   };
};

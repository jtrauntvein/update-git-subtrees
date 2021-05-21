/* Cora.Broker.FormattedDataAdvisor.cpp

   Copyright (C) 2002, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 02 January 2002
   Last Change: Friday 29 April 2016
   Last Commit: $Date: 2016-05-03 16:18:03 -0600 (Tue, 03 May 2016) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Broker.FormattedDataAdvisor.h"
#include "Cora.Broker.ValueName.h"
#include "Cora.Broker.CustomCsvOptions.h"
#include "Cora.Device.Defs.h"
#include "coratools.strings.h"


namespace Cora
{
   namespace Broker
   {
      namespace FormattedDataAdvisorHelpers
      {
         ////////////////////////////////////////////////////////////
         // event_base
         ////////////////////////////////////////////////////////////
         class event_base: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // advisor
            ////////////////////////////////////////////////////////////
            typedef FormattedDataAdvisor advisor_type;
            advisor_type *advisor;

            ////////////////////////////////////////////////////////////
            // client
            ////////////////////////////////////////////////////////////
            typedef FormattedDataAdvisorClient client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify() = 0;

         protected:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_base(
               uint4 event_id,
               advisor_type *advisor_,
               client_type *client_):
               Event(event_id,advisor_),
               advisor(advisor_),
               client(client_)
            { }
         };


         ////////////////////////////////////////////////////////////
         // class event_started
         ////////////////////////////////////////////////////////////
         class event_started: public event_base
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // data_header
            ////////////////////////////////////////////////////////////
            StrAsc data_header;

            ////////////////////////////////////////////////////////////
            // data_footer
            ////////////////////////////////////////////////////////////
            StrAsc data_footer;

            ////////////////////////////////////////////////////////////
            // create_and_post
            ////////////////////////////////////////////////////////////
            static void create_and_post(
               advisor_type *advisor,
               client_type *client,
               StrAsc const &data_header,
               StrAsc const &data_footer)
            {
               try{(new event_started(advisor,client,data_header,data_footer))->post(); }
               catch(Event::BadPost &) { }
            }

            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            { client->on_started(advisor,data_header,data_footer); } 
            
         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_started(
               advisor_type *advisor,
               client_type *client,
               StrAsc const &data_header_,
               StrAsc const &data_footer_):
               event_base(event_id,advisor,client),
               data_header(data_header_),
               data_footer(data_footer_)
            { }
         };


         uint4 const event_started::event_id =
         Csi::Event::registerType("Cora::Broker::FormattedDataAdvisor::event_started");


         ////////////////////////////////////////////////////////////
         // class event_failure
         ////////////////////////////////////////////////////////////
         class event_failure: public event_base
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // failure
            ////////////////////////////////////////////////////////////
            typedef client_type::failure_type failure_type;
            failure_type failure;

            ////////////////////////////////////////////////////////////
            // create_and_post
            ////////////////////////////////////////////////////////////
            static void create_and_post(
               advisor_type *advisor,
               client_type *client,
               failure_type failure)
            {
               try{(new event_failure(advisor,client,failure))->post(); }
               catch(Event::BadPost &) { }
            }

            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            { client->on_failure(advisor,failure); }
            
         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_failure(
               advisor_type *advisor,
               client_type *client,
               failure_type failure_):
               event_base(event_id,advisor,client),
               failure(failure_)
            { }
         };


         uint4 const event_failure::event_id =
         Csi::Event::registerType("Cora::Broker::FormattedDataAdvisor::event_failure");


         ////////////////////////////////////////////////////////////
         // class event_records
         ////////////////////////////////////////////////////////////
         class event_records: public event_base
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // create_and_post
            ////////////////////////////////////////////////////////////
            static void create_and_post(
               advisor_type *advisor,
               client_type *client)
            {
               try{(new event_records(advisor,client))->post(); }
               catch(Event::BadPost &) { }
            }

            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            { client->on_records(advisor); }
            
         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_records(
               advisor_type *advisor,
               client_type *client):
               event_base(event_id,advisor,client)
            { }
         };


         uint4 const event_records::event_id =
         Csi::Event::registerType("Cora::Broker::FormattedDataAdvisor::event_records"); 
      };


      ////////////////////////////////////////////////////////////
      // class FormattedDataRecord definitions
      ////////////////////////////////////////////////////////////
      FormattedDataRecord::FormattedDataRecord():
         file_mark_no(0),
         record_no(0),
         stamp(0)
      { }

      
      FormattedDataRecord::FormattedDataRecord(FormattedDataRecord const &other):
         station_name(other.station_name),
         table_name(other.table_name),
         file_mark_no(other.file_mark_no),
         record_no(other.record_no),
         stamp(other.stamp),
         formatted_data(other.formatted_data)
      { }


      FormattedDataRecord &FormattedDataRecord::operator =(FormattedDataRecord const &other)
      {
         station_name = other.station_name;
         table_name = other.table_name;
         file_mark_no = other.file_mark_no;
         record_no = other.record_no;
         stamp = other.stamp;
         formatted_data = other.formatted_data;
         return *this;
      } // copy operator


      bool FormattedDataRecord::read(
         StrUni const &station_name_,
         StrUni const &table_name_,
         Csi::Messaging::Message *in)
      {
         bool rtn =
            in->readUInt4(record_no) &&
            in->readUInt4(file_mark_no) &&
            in->readInt8(stamp) &&
            in->readBStr(formatted_data);
         if(rtn)
         {
            station_name = station_name_;
            table_name = table_name_;
         }
         return rtn;
      } // read



      ////////////////////////////////////////////////////////////
      // class FormattedDataAdvisor definitions
      ////////////////////////////////////////////////////////////
      FormattedDataAdvisor::FormattedDataAdvisor():
         start_option(start_at_newest),
         order_option(order_real_time),
         format_option(format_toa5),
         format_option_flags(
            Cora::Device::CollectArea::table_file_include_time |
            Cora::Device::CollectArea::table_file_include_record_no),
         start_record_no(0),
         start_file_mark_no(0),
         start_date(0),
         start_record_offset(1),
         start_interval(0),
         cache_size_controller(1),
         state(state_standby)
      { }
      

      FormattedDataAdvisor::~FormattedDataAdvisor()
      { finish(); }

      
      void FormattedDataAdvisor::set_table_name(StrUni const &table_name_)
      {
         if(state == state_standby)
            table_name = table_name_;
         else
            throw exc_invalid_state();
      } // set_table_name


      void FormattedDataAdvisor::set_start_option(start_option_type start_option_)
      {
         if(state == state_standby)
            start_option = start_option_;
         else
            throw exc_invalid_state();
      } // set_start_option

      
      void FormattedDataAdvisor::set_order_option(order_option_type order_option_)
      {
         if(state == state_standby)
            order_option = order_option_;
         else
            throw exc_invalid_state();
      } // set_order_option

      
      void FormattedDataAdvisor::set_format_option(
         format_option_type format_option_)
      {
         if(state == state_standby)
         {
            format_option = format_option_;
            if(format_option == format_custom_csv)
               format_option_flags = CustomCsvOptions().get_options();
            else
            {
               using namespace Cora::Device::CollectArea;
               format_option_flags =
                  (table_file_include_record_no |
                   table_file_include_time);
            }
         }
         else
            throw exc_invalid_state();
      } // set_format_option


      void FormattedDataAdvisor::set_format_option_flags(uint4 format_option_flags_)
      {
         if(state != state_standby)
            throw exc_invalid_state();
         format_option_flags = format_option_flags_;
      } // set_format_option_flags

      
      void FormattedDataAdvisor::set_start_record_no(uint4 start_record_no_)
      {
         if(state == state_standby)
            start_record_no = start_record_no_;
         else
            throw exc_invalid_state();
      } // set_start_record_no

      
      void FormattedDataAdvisor::set_start_file_mark_no(uint4 start_file_mark_no_)
      {
         if(state == state_standby)
            start_file_mark_no = start_file_mark_no_;
         else
            throw exc_invalid_state();
      } // set_start_file_mark_no

      
      void FormattedDataAdvisor::set_start_date(int8 start_date_)
      {
         if(state == state_standby)
            start_date = start_date_;
         else
            throw exc_invalid_state();
      } // set_start_date

      
      void FormattedDataAdvisor::set_start_interval(int8 start_interval_)
      {
         if(state == state_standby)
            start_interval = start_interval_;
         else
            throw exc_invalid_state();
      } // set_start_interval

      
      void FormattedDataAdvisor::set_cache_size_controller(uint4 cache_size_controller_)
      {
         if(state == state_standby)
            cache_size_controller = cache_size_controller_;
         else
            throw exc_invalid_state();
      } // set_cache_size_controller

      
      void FormattedDataAdvisor::set_start_record_offset(uint4 start_record_offset_)
      {
         if(state == state_standby)
            start_record_offset = start_record_offset_;
         else
            throw exc_invalid_state();
      } // set_start_record_offset


      void FormattedDataAdvisor::set_reported_station_name(StrUni const &reported_station_name_)
      {
         if(state == state_standby)
            reported_station_name = reported_station_name_;
         else
            throw exc_invalid_state();
      } // set_reported_station_name


      void FormattedDataAdvisor::set_reported_table_name(StrUni const &reported_table_name_)
      {
         if(state == state_standby)
            reported_table_name = reported_table_name_;
         else
            throw exc_invalid_state();
      } // set_reported_table_name


      void FormattedDataAdvisor::clear_columns()
      {
         if(state == state_standby)
            selectors.clear();
         else
            throw exc_invalid_state();
      } // clear_columns


      void FormattedDataAdvisor::add_column(StrUni const &selector)
      {
         if(state == state_standby)
            selectors.push_back(selector);
         else
            throw exc_invalid_state();
      } // add_column
      
      
      void FormattedDataAdvisor::start(
         client_type *client_,
         router_handle &router)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
            {
               client = client_;
               state = state_delegate;
               BrokerBase::start(router);
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start

      
      void FormattedDataAdvisor::start(
         client_type *client_,
         ClientBase *other_component)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
            {
               client = client_;
               state = state_delegate;
               BrokerBase::start(other_component);
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start

      
      void FormattedDataAdvisor::finish()
      {
         state = state_standby;
         client = 0;
         advise_tran = 0;
         records.clear();
         BrokerBase::finish();
      } // finish


      void FormattedDataAdvisor::format_failure(std::ostream &out, client_type::failure_type failure)
      {
         using namespace FormattedDataAdvisorStrings;
         switch(failure)
         {
         case client_type::failure_connection_failed:
            BrokerBase::format_failure(out, brokerbase_failure_session);
            break;
            
         case client_type::failure_invalid_logon:
            BrokerBase::format_failure(out, brokerbase_failure_logon);
            break;
            
         case client_type::failure_invalid_station_name:
            BrokerBase::format_failure(out, brokerbase_failure_invalid_id);
            break;
            
         case client_type::failure_invalid_table_name:
         case client_type::failure_server_security:
            BrokerBase::format_failure(out, brokerbase_failure_security);
            break;
            
         case client_type::failure_invalid_start_option:
            out << my_strings[strid_invalid_start_option];
            break;
            
         case client_type::failure_invalid_order_option:
            out << my_strings[strid_invalid_order_option];
            break;
            
         case client_type::failure_invalid_format_option:
            out << my_strings[strid_invalid_format_option];
            break;
            
         case client_type::failure_table_deleted:
            out << my_strings[strid_table_deleted];
            break;
            
         case client_type::failure_station_shut_down:
            out << my_strings[strid_station_shut_down];
            break;
            
         case client_type::failure_unsupported:
            BrokerBase::format_failure(out, brokerbase_failure_unsupported);
            break;
            
         default:
            BrokerBase::format_failure(out, brokerbase_failure_unknown);
            break;
         }
      } // format_failure

      
      void FormattedDataAdvisor::continue_advise()
      {
         if(state == state_wait_for_continue)
         {
            Csi::Messaging::Message command(
               broker_session,
               Messages::formatted_data_advise_cont_cmd);
            
            records.clear();
            state = state_wait_for_records;
            command.addUInt4(advise_tran);
            command.addBool(false);
            router->sendMessage(&command);
         }
         else
            throw exc_invalid_state();
      } // continue_advise

      
      void FormattedDataAdvisor::on_brokerbase_ready()
      {
         using namespace FormattedDataAdvisorHelpers;
         if(interface_version >= Csi::VersionNumber("1.3.1.32"))
         {
            Csi::Messaging::Message start_command(
               broker_session,
               Messages::formatted_data_advise_start_cmd);

            start_command.addUInt4(advise_tran = ++last_tran_no);
            start_command.addWStr(table_name);
            start_command.addUInt4(static_cast<uint4>(selectors.size()));
            for(selectors_type::iterator si = selectors.begin();
                si != selectors.end();
                ++si)
            {
               ValueName value(si->c_str());
               start_command.addWStr(value.get_column_name());
               start_command.addUInt4(static_cast<uint4>(value.size()));
               for(ValueName::iterator vi = value.begin();
                   vi != value.end();
                   ++vi)
                  start_command.addUInt4(*vi);
            }
            start_command.addUInt4(cache_size_controller);
            start_command.addUInt4(order_option);
            start_command.addUInt4(start_option);
            start_command.addUInt4(start_file_mark_no);
            start_command.addUInt4(start_record_no);
            if(start_option == start_at_time_stamp)
               start_command.addInt8(start_date);
            else
               start_command.addInt8(start_interval);
            start_command.addUInt4(format_option);
            start_command.addUInt4(start_record_offset);
            start_command.addWStr(reported_station_name);
            start_command.addWStr(reported_table_name);
            start_command.addUInt4(format_option_flags);
            state = state_before_active;
            router->sendMessage(&start_command);
         }
         else
            event_failure::create_and_post(this,client,client_type::failure_unsupported);
      } // on_brokerbase_ready

      
      void FormattedDataAdvisor::on_brokerbase_failure(brokerbase_failure_type failure)
      {
         using namespace FormattedDataAdvisorHelpers;
         client_type::failure_type client_failure;

         switch(failure)
         {
         default:
            client_failure = client_type::failure_unknown;
            break;
            
         case brokerbase_failure_logon:
            client_failure = client_type::failure_invalid_logon;
            break;
            
         case brokerbase_failure_session:
            client_failure = client_type::failure_connection_failed;
            break;
            
         case brokerbase_failure_invalid_id:
            client_failure = client_type::failure_invalid_station_name;
            break;
            
         case brokerbase_failure_unsupported:
            client_failure = client_type::failure_unsupported;
            break;
            
         case brokerbase_failure_security:
            client_failure = client_type::failure_server_security;
            break;
         }
         event_failure::create_and_post(this,client,client_failure);
      } // on_brokerbase_failure

      
      void FormattedDataAdvisor::on_brokerbase_session_failure()
      {
         using namespace FormattedDataAdvisorHelpers;
         event_failure::create_and_post(
            this,
            client,
            client_type::failure_connection_failed);
      } // on_brokerbase_session_failure

      
      void FormattedDataAdvisor::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(state != state_delegate)
         {
            switch(msg->getMsgType())
            {
            case Messages::formatted_data_advise_start_ack:
               on_start_ack(msg);
               break;
                        
            case Messages::formatted_data_advise_not:
               on_data_not(msg);
               break;
               
            case Messages::formatted_data_advise_stopped_not:
               on_stopped_not(msg);
               break;
               
            default:
               BrokerBase::onNetMessage(rtr,msg);
               break;
            }
         }
         else
            BrokerBase::onNetMessage(rtr,msg);
      } // onNetMessage

      
      void FormattedDataAdvisor::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace FormattedDataAdvisorHelpers;
         event_base *event = static_cast<event_base *>(ev.get_rep());
         client_type *old_client = client;
         if(event->getType() == event_failure::event_id)
            finish();
         if(event->client == old_client && client_type::is_valid_instance(old_client))
            event->notify();
         else
            finish();
      } // receive

      
      void FormattedDataAdvisor::on_start_ack(Csi::Messaging::Message *msg)
      {
         // read the message
         uint4 tran_no;
         uint4 resp_code;
         StrAsc data_header;
         StrAsc data_footer;
         
         msg->readUInt4(tran_no);
         msg->readUInt4(resp_code);
         if(resp_code == 1)
         {
            msg->readStr(data_header);
            if(msg->whatsLeft())
               msg->readStr(data_footer);
         }
         
         // process the response
         using namespace FormattedDataAdvisorHelpers;
         if(resp_code == 1)
         {
            state = state_wait_for_records;
            event_started::create_and_post(this,client,data_header,data_footer);
         }
         else
         {
            client_type::failure_type failure;
            switch(resp_code)
            {
            default:
               failure = client_type::failure_unknown;
               break;

            case 2:
               failure = client_type::failure_invalid_table_name;
               break;

            case 4:
               failure = client_type::failure_invalid_order_option;
               break;

            case 5:
               failure = client_type::failure_invalid_start_option;
               break;

            case 8:
               failure = client_type::failure_invalid_format_option;
               break;
            }
            event_failure::create_and_post(this,client,failure);
         }
      } // on_start_ack

      
      void FormattedDataAdvisor::on_data_not(Csi::Messaging::Message *msg)
      {
         using namespace FormattedDataAdvisorHelpers;
         uint4 tran_no;
         uint4 num_records;

         msg->readUInt4(tran_no);
         msg->readUInt4(num_records);
         records.clear();
         for(uint4 i = 0; i < num_records; ++i)
         {
            FormattedDataRecord record;
            record.read(get_broker_name(),table_name,msg);
            records.push_back(record);
         }
         state = state_wait_for_continue;
         event_records::create_and_post(this,client);
      } // on_data_not

      
      void FormattedDataAdvisor::on_stopped_not(Csi::Messaging::Message *msg)
      {
         using namespace FormattedDataAdvisorHelpers;
         uint4 tran_no;
         uint4 reason;
         client_type::failure_type failure;
         
         msg->readUInt4(tran_no);
         msg->readUInt4(reason);
         switch(reason)
         {
         case 2:
            failure = client_type::failure_station_shut_down;
            break;

         case 3:
            failure = client_type::failure_table_deleted;
            break;

         default:
            failure = client_type::failure_unknown;
            break;
         }
         event_failure::create_and_post(this,client,failure);
      } // on_stopped_not
   };
};


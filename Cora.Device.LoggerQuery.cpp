/* Cora.Device.LoggerQuery.cpp

   Copyright (C) 2001, 2017 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 12 December 2001
   Last Change: Wednesday 10 May 2017
   Last Commit: $Date: 2017-05-11 10:32:26 -0600 (Thu, 11 May 2017) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.LoggerQuery.h"
#include "coratools.strings.h"


namespace Cora
{
   namespace Device
   {
      namespace LoggerQueryHelpers
      {
         ////////////////////////////////////////////////////////////
         // class event_base
         ////////////////////////////////////////////////////////////
         class event_base: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // client
            ////////////////////////////////////////////////////////////
            typedef LoggerQueryClient client_type;
            client_type *client;
            
            ////////////////////////////////////////////////////////////
            // query
            ////////////////////////////////////////////////////////////
            typedef LoggerQuery query_type;
            query_type *query;

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
               client_type *client_,
               query_type *query_):
               Event(event_id,query_),
               query(query_),
               client(client_)
            { }
         };


         ////////////////////////////////////////////////////////////
         // class event_status
         ////////////////////////////////////////////////////////////
         class event_status: public event_base
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;
            
            ////////////////////////////////////////////////////////////
            // status_code
            ////////////////////////////////////////////////////////////
            typedef client_type::status_code_type status_code_type;
            status_code_type status_code;

            ////////////////////////////////////////////////////////////
            // temp_table_name
            ////////////////////////////////////////////////////////////
            StrUni temp_table_name;

            ////////////////////////////////////////////////////////////
            // record_count
            ////////////////////////////////////////////////////////////
            uint4 record_count;

            ////////////////////////////////////////////////////////////
            // begin_record_no
            ////////////////////////////////////////////////////////////
            uint4 begin_record_no;
            
            ////////////////////////////////////////////////////////////
            // end_record_no
            ////////////////////////////////////////////////////////////
            uint4 end_record_no;

            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            {
               client->on_query_status(
                  query,status_code,temp_table_name,record_count,begin_record_no,end_record_no);
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_status(
               client_type *client,
               query_type *query,
               status_code_type status_code_,
               StrUni const &temp_table_name_,
               uint4 record_count_,
               uint4 begin_record_no_,
               uint4 end_record_no_):
               event_base(event_id,client,query),
               status_code(status_code_),
               temp_table_name(temp_table_name_),
               record_count(record_count_),
               begin_record_no(begin_record_no_),
               end_record_no(end_record_no_)
            { }
            
         public:
            ////////////////////////////////////////////////////////////
            // create_and_post
            ////////////////////////////////////////////////////////////
            static void create_and_post(
               client_type *client,
               query_type *query,
               status_code_type status_code,
               StrUni const &temp_table_name,
               uint4 record_count,
               uint4 begin_record_no,
               uint4 end_record_no)
            {
               try
               {
                  event_status *ev = new event_status(
                     client,
                     query,
                     status_code,
                     temp_table_name,
                     record_count,
                     begin_record_no,
                     end_record_no);
                  ev->post();
               }
               catch(Csi::Event::BadPost &)
               { }
            }
         };


         uint4 const event_status::event_id =
         Csi::Event::registerType("Cora::Device::LoggerQuery::event_status");


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
            // notify
            ////////////////////////////////////////////////////////////
            virtual void notify()
            { client->on_failure(query,failure); }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_failure(
               client_type *client,
               query_type *query,
               failure_type failure_):
               event_base(event_id,client,query),
               failure(failure_)
            { }
            
         public:
            ////////////////////////////////////////////////////////////
            // create_and_post
            ////////////////////////////////////////////////////////////
            static void create_and_post(
               client_type *client,
               query_type *query,
               failure_type failure)
            {
               try { (new event_failure(client,query,failure))->post(); }
               catch(Csi::Event::BadPost &) { }
            }
         };


         uint4 const event_failure::event_id =
         Csi::Event::registerType("Cora::Device::LoggerQuery::event_failure"); 
      };

      
      ////////////////////////////////////////////////////////////
      // class LoggerQuery definitions
      ////////////////////////////////////////////////////////////
      LoggerQuery::LoggerQuery():
         query_mode(query_recent_records),
         number_of_records(1),
         begin_record_no(0),
         end_record_no(UInt4_Max),
         client(0),
         state(state_standby),
         query_transaction(0),
         use_same_table(false)
      { }

      
      LoggerQuery::~LoggerQuery()
      { finish(); }

      
      void LoggerQuery::set_query_mode(query_mode_type query_mode_)
      {
         if(state == state_standby)
            query_mode = query_mode_;
         else
            throw exc_invalid_state();
      } // set_query_mode

      
      void LoggerQuery::set_table_name(StrUni const &table_name_)
      {
         if(state == state_standby)
            table_name = table_name_;
         else
            throw exc_invalid_state();
      } // set_table_name

      
      void LoggerQuery::set_begin_date(LgrDate const &begin_date_)
      {
         if(state == state_standby)
            begin_date = begin_date_;
         else
            throw exc_invalid_state();
      } // set_begin_date

      
      void LoggerQuery::set_end_date(LgrDate const &end_date_)
      {
         if(state == state_standby)
            end_date = end_date_;
         else
            throw exc_invalid_state();
      } // set_end_date

      
      void LoggerQuery::set_number_of_records(uint4 number_of_records_)
      {
         if(state == state_standby)
            number_of_records = number_of_records_;
         else
            throw exc_invalid_state();
      } // set_number_of_records

      
      void LoggerQuery::set_begin_record_no(uint4 begin_record_no_)
      {
         if(state == state_standby)
            begin_record_no = begin_record_no_;
         else
            throw exc_invalid_state();
      } // set_begin_record_no

      
      void LoggerQuery::set_end_record_no(uint4 end_record_no_)
      {
         if(state == state_standby)
            end_record_no = end_record_no_;
         else
            throw exc_invalid_state();
      } // set_end_record_no


      void LoggerQuery::set_use_same_table(bool use_same_table_)
      {
         if(state != state_standby)
            throw exc_invalid_state();
         use_same_table = use_same_table_;
      } // set_use_same_table


      void LoggerQuery::set_backfill_interval(int8 value)
      {
         if(state == state_standby)
         {
            backfill_interval = value;
            query_mode = query_backfill;
         }
         else
            throw exc_invalid_state();
      } // set_backfill_interval

      
      void LoggerQuery::start(
         client_type *client_,
         router_handle &router)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
            {
               client = client_;
               state = state_delegate;
               DeviceBase::start(router);
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start

      
      void LoggerQuery::start(
         client_type *client_,
         ClientBase *other_component)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
            {
               client = client_;
               state = state_delegate;
               DeviceBase::start(other_component);
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start

      
      void LoggerQuery::finish()
      {
         state = state_standby;
         client = 0;
         query_transaction = 0;
         DeviceBase::finish();
      } // finish


      void LoggerQuery::format_failure(std::ostream &out, client_type::failure_type failure)
      {
         using namespace Cora::Device::LoggerQueryStrings;
         switch(failure)
         {
         default:
         case client_type::failure_unknown:
            format_devicebase_failure(out, devicebase_failure_unknown);
            break;
            
         case client_type::failure_invalid_logon:
            format_devicebase_failure(out, devicebase_failure_logon);
            break;
            
         case client_type::failure_server_session_failed:
            format_devicebase_failure(out, devicebase_failure_session);
            break;
            
         case client_type::failure_invalid_device_name:
            format_devicebase_failure(out,  devicebase_failure_invalid_device_name);
            break;
            
         case client_type::failure_unsupported:
            format_devicebase_failure(out, devicebase_failure_unsupported);
            break;

         case client_type::failure_server_security_blocked:
            format_devicebase_failure(out, devicebase_failure_security);
            break;

         case client_type::failure_logger_security_blocked:
            out << my_strings[strid_logger_security_blocked];
            break;
            
         case client_type::failure_communication_failure:
            out << my_strings[strid_communication_failure];
            break;
            
         case client_type::failure_communication_disabled:
            out << my_strings[strid_communication_disabled];
            break;
            
         case client_type::failure_invalid_table_name:
            out << my_strings[strid_invalid_table_name];
            break;
            
         case client_type::failure_invalid_table_definition:
            out << my_strings[strid_invalid_table_definition];
            break;
            
         case client_type::failure_insufficient_resources:
            out << my_strings[strid_insufficient_resources];
            break;
         }
      } // format_failure

      
      void LoggerQuery::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace LoggerQueryHelpers;
         event_base *event = static_cast<event_base *>(ev.get_rep());
         if(event->getType() == event_failure::event_id)
            finish();
         if(client_type::is_valid_instance(event->client))
            event->notify();
      } // receive

      
      void LoggerQuery::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(state == state_before_active || state == state_active)
         {
            switch(msg->getMsgType())
            {
            case Messages::logger_query_status_not:
               on_status_not(msg);
               break;
               
            case Messages::logger_query_stopped_not:
               on_stopped_not(msg);
               break;
               
            default:
               DeviceBase::onNetMessage(rtr,msg);
               break;
            }
         }
         else
            DeviceBase::onNetMessage(rtr,msg);
      } // onNetMessage

      
      void LoggerQuery::on_devicebase_ready()
      {
         Csi::Messaging::Message start_command(
            device_session,
            Messages::logger_query_start_cmd);

         start_command.addUInt4(query_transaction = ++last_tran_no);
         start_command.addWStr(table_name);
         if(query_mode == query_start_at_record && interface_version < Csi::VersionNumber("1.3.8.7"))
         {
            query_mode = query_record_number_range;
            end_record_no = 0xffffffff;
         }
         if(query_mode == query_backfill && interface_version < Csi::VersionNumber("1.12.6"))
         {
            Csi::LgrDate now(Csi::LgrDate::system());
            query_mode = query_date_range;
            begin_date = now - backfill_interval * Csi::LgrDate::nsecPerMSec;
            end_date = now;
         }
         start_command.addUInt4(query_mode);
         start_command.addInt8(begin_date.get_nanoSec());
         start_command.addInt8(end_date.get_nanoSec());
         start_command.addUInt4(number_of_records);
         start_command.addUInt4(begin_record_no);
         start_command.addUInt4(end_record_no);
         if(interface_version >= Csi::VersionNumber("1.3.9.11"))
         {
            start_command.addBool(use_same_table);
            if(interface_version >= Csi::VersionNumber("1.12.6"))
               start_command.addInt8(backfill_interval);
         }
         else
            use_same_table = false;
         router->sendMessage(&start_command);
         state = state_before_active; 
      } // on_devicebase_ready

      
      void LoggerQuery::on_devicebase_failure(devicebase_failure_type failure)
      {
         using namespace LoggerQueryHelpers;
         client_type::failure_type client_failure;
         switch(failure)
         {
         case devicebase_failure_logon:
            client_failure = client_type::failure_invalid_logon;
            break;
            
         case devicebase_failure_session:
            client_failure = client_type::failure_server_session_failed;
            break;
            
         case devicebase_failure_invalid_device_name:
            client_failure = client_type::failure_invalid_device_name;
            break;
            
         case devicebase_failure_unsupported:
            client_failure = client_type::failure_unsupported;
            break;
            
         case devicebase_failure_security:
            client_failure = client_type::failure_server_security_blocked;
            break;
            
         default:
            client_failure = client_type::failure_unknown;
            break;
         }
         event_failure::create_and_post(client,this,client_failure);
      } // on_devicebase_failure

      
      void LoggerQuery::on_devicebase_session_failure()
      {
         using namespace LoggerQueryHelpers;
         event_failure::create_and_post(
            client,this,client_type::failure_server_session_failed);
      } // on_devicebase_session_failure


      void LoggerQuery::on_status_not(Csi::Messaging::Message *message)
      {
         using namespace LoggerQueryHelpers;
         uint4 tran_no;
         uint4 status_code;
         uint4 record_count;
         uint4 begin_record_no = 0;
         uint4 end_record_no = 0;
         
         message->readUInt4(tran_no);
         message->readUInt4(status_code);
         message->readWStr(temp_table_name);
         message->readUInt4(record_count);
         if(record_count > 0)
         {
            message->readUInt4(begin_record_no);
            message->readUInt4(end_record_no);
         }
         event_status::create_and_post(
            client,
            this,
            static_cast<client_type::status_code_type>(status_code),
            temp_table_name,
            record_count,
            begin_record_no,
            end_record_no); 
      } // on_status_not


      void LoggerQuery::on_stopped_not(Csi::Messaging::Message *message)
      {
         using namespace LoggerQueryHelpers; 
         uint4 tran_no;
         uint4 reason;
         client_type::failure_type failure;

         message->readUInt4(tran_no);
         message->readUInt4(reason);
         switch(reason)
         {
         case 3:
            failure = client_type::failure_invalid_table_name;
            break;

         case 4:
            failure = client_type::failure_invalid_table_definition;
            break;

         case 5:
            failure = client_type::failure_insufficient_resources;
            break;

         case 6:
            failure = client_type::failure_communication_disabled;
            break;

         case 7:
            failure = client_type::failure_communication_failure;
            break;
            
         default:
            failure = client_type::failure_unknown;
            break;
         }
         event_failure::create_and_post(client,this,failure);
      } // on_stopped_not
   };
};

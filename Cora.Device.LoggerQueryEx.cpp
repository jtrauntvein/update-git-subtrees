/* Cora.Device.LoggerQueryEx.cpp

   Copyright (C) 2019, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Monday 16 September 2019
   Last Change: Tuesday 03 December 2019
   Last Commit: $Date: 2019-12-03 17:00:46 -0600 (Tue, 03 Dec 2019) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.LoggerQueryEx.h"
#include "coratools.strings.h"


namespace Cora
{
   namespace Device
   {
      namespace
      {
         /**
          * Defines the event used to report that the transaction is started.
          */
         class event_started: public Csi::Event
         {
         public:
            static uint4 const event_id;
            LoggerQueryExClient *client;
            typedef LoggerQueryExClient::start_outcome_type outcome_type;
            outcome_type outcome;

            static void cpost(LoggerQueryEx *sender, outcome_type outcome)
            {
               (new event_started(sender, outcome))->post();
            }

         private:
            event_started(LoggerQueryEx *sender, outcome_type outcome_):
               Event(event_id, sender),
               outcome(outcome_)
            { }
         };


         uint4 const event_started::event_id(
            Csi::Event::registerType("Cora::Device::LoggerQueryEx::event_started"));


         /**
          * Defines the event that will be posted to report new records.
          */
         class event_records: public Csi::Event
         {
         public:
            static uint4 const event_id;

            static void cpost(LoggerQueryEx *sender)
            { (new event_records(sender))->post(); }

         private:
            event_records(LoggerQueryEx *sender):
               Event(event_id, sender)
            { }
         };


         uint4 const event_records::event_id(
            Csi::Event::registerType("Cora::Device::LoggerQueryEx::event_records"));


         /**
          * Defines the event that is posted when the transaction has been stopped.
          */
         class event_stopped: public Csi::Event
         {
         public:
            static uint4 const event_id;
            typedef LoggerQueryExClient::stopped_outcome_type outcome_type;
            outcome_type outcome;

            static void cpost(LoggerQueryEx *sender, outcome_type outcome)
            { (new event_stopped(sender, outcome))->post(); }

         private:
            event_stopped(LoggerQueryEx *sender, outcome_type outcome_):
               Event(event_id, sender),
               outcome(outcome_)
            { }
         };


         uint4 const event_stopped::event_id(
            Csi::Event::registerType("Cora::Device::LoggerQueryEx::event_stopped"));
      };


      void LoggerQueryEx::format_start_outcome(
         std::ostream &out, client_type::start_outcome_type outcome)
      {
         switch(outcome)
         {
         case client_type::start_outcome_success:
            out << common_strings[common_success];
            break;
            
         case client_type::start_outcome_failure_logon:
            format_devicebase_failure(out, devicebase_failure_logon);
            break;
            
         case client_type::start_outcome_failure_invalid_device_name:
            format_devicebase_failure(out, devicebase_failure_invalid_device_name);
            break;
            
         case client_type::start_outcome_failure_server_security_blocked:
            format_devicebase_failure(out, devicebase_failure_security);
            break;
            
         case client_type::start_outcome_failure_session:
            format_devicebase_failure(out, devicebase_failure_session);
            break;
            
         case client_type::start_outcome_failure_invalid_table_name:
            out << LoggerQueryStrings::my_strings[LoggerQueryStrings::strid_invalid_table_name];
            break;
            
         case client_type::start_outcome_failure_comms_disabled:
            out << common_strings[common_comm_disabled];
            break;
            
         case client_type::start_outcome_failure_unsupported:
            format_devicebase_failure(out, devicebase_failure_unsupported);
            break;
            
         case client_type::start_outcome_failure_unsupported_file_format:
         case client_type::start_outcome_failure_unsupported_query_mode:
         case client_type::start_outcome_failure_unknown:
         default:
            format_devicebase_failure(out, devicebase_failure_unknown);
            break;
         }
      } // format_start_outcome


      void LoggerQueryEx::format_stopped_outcome(
         std::ostream &out, client_type::stopped_outcome_type outcome)
      {
         using namespace LoggerQueryExStrings;
         switch(outcome)
         {
         case client_type::stopped_outcome_success:
            out << common_strings[common_success];
            break;
            
         case client_type::stopped_outcome_success_client_request:
            out << common_strings[common_aborted];
            break;
            
         case client_type::stopped_outcome_failure_table_defs:
            out << common_strings[common_table_defs_invalid];
            break;
            
         case client_type::stopped_outcome_failure_comms:
            out << common_strings[common_comm_failed];
            break;
            
         case client_type::stopped_outcome_failure_comms_disabled:
            out << common_strings[common_comm_disabled];
            break;
            
         case client_type::stopped_outcome_failure_blocked_by_logger:
            out << common_strings[common_logger_security_blocked];
            break;
            
         case client_type::stopped_outcome_failure_session:
            format_devicebase_failure(out, devicebase_failure_session);
            break;

         case client_type::stopped_outcome_failure_ack_timed_out:
            out << my_strings[strid_stopped_outcome_failure_ack_timed_out];
            break;
            
         case client_type::stopped_outcome_failure_unknown:
         default:
            format_devicebase_failure(out, devicebase_failure_unknown);
            break;
         }
      } // format_stopped_outcome

      
      void LoggerQueryEx::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         if(ev->getType() == event_started::event_id)
         {
            client_type *dest(client);
            event_started *event(static_cast<event_started *>(ev.get_rep()));
            if(!client_type::is_valid_instance(client) || event->outcome != client_type::start_outcome_success)
               finish();
            if(client_type::is_valid_instance(dest))
               dest->on_started(this, event->outcome, formatted_header, formatted_footer);
         }
         else if(ev->getType() == event_stopped::event_id)
         {
            event_stopped *event(static_cast<event_stopped *>(ev.get_rep()));
            client_type *dest(client);
            finish();
            if(client_type::is_valid_instance(dest))
               dest->on_stopped(this, event->outcome);
         }
         else if(ev->getType() == event_records::event_id)
         {
            event_records *event(static_cast<event_records *>(ev.get_rep()));
            if(client_type::is_valid_instance(client))
            {
               if(!records.empty())
               {
                  Csi::Messaging::Message ack_command(device_session, Messages::logger_query_ex_records_ack_cmd);
                  ack_command.addUInt4(tran_no);
                  ack_command.addUInt4(records.back()->record_no);
                  client->on_records(this, records);
                  router->sendMessage(&ack_command);
               }
               records_cache.insert(records_cache.end(), records.begin(), records.end());
               records.clear();
            }
            else
               finish();
         }
      } // receive

      
      void LoggerQueryEx::on_devicebase_ready()
      {
         Csi::Messaging::Message command(device_session, Messages::logger_query_ex_start_cmd);
         tran_no = ++last_tran_no;
         command.addUInt4(tran_no);
         command.addWStr(table_name);
         command.addUInt4(query_mode);
         command.addInt8(begin_date.get_nanoSec());
         command.addInt8(end_date.get_nanoSec());
         command.addUInt4(max_records);
         command.addUInt4(begin_record_no);
         command.addUInt4(end_record_no);
         command.addInt8(backfill_interval);
         command.addUInt4(record_format);
         command.addUInt4(record_format_options);
         command.addWStr(reported_station_name);
         state = state_starting;
         router->sendMessage(&command);
      } // on_devicebase_ready


      void LoggerQueryEx::on_devicebase_failure(devicebase_failure_type failure)
      {
         if(state == state_starting)
         {
            client_type::start_outcome_type outcome;
            switch(failure)
            {
            case devicebase_failure_logon:
               outcome = client_type::start_outcome_failure_logon;
               break;
               
            case devicebase_failure_session:
               outcome = client_type::start_outcome_failure_session;
               break;
               
            case devicebase_failure_invalid_device_name:
               outcome = client_type::start_outcome_failure_invalid_device_name;
               break;
               
            case devicebase_failure_unsupported:
               outcome = client_type::start_outcome_failure_unsupported;
               break;
               
            case devicebase_failure_security:
               outcome = client_type::start_outcome_failure_server_security_blocked;
               break;
               
            default:
               outcome = client_type::start_outcome_failure_unknown;
               break;
            }
            event_started::cpost(this, outcome);
         }
         else if(state == state_started)
         {
            client_type::stopped_outcome_type outcome;
            switch(failure)
            {
            case devicebase_failure_session:
               outcome = client_type::stopped_outcome_failure_session;
               break;
               
            default:
               outcome = client_type::stopped_outcome_failure_unknown;
               break;
            }
            event_stopped::cpost(this, outcome);
         }
      } // on_devicbase_failure
         
         
      void LoggerQueryEx::onNetMessage(Csi::Messaging::Router *router, Csi::Messaging::Message *message)
      {
         if(state >= state_starting)
         {
            switch(message->getMsgType())
            {
            case Messages::logger_query_ex_start_ack:
               on_start_ack(message);
               break;
               
            case Messages::logger_query_ex_records_not:
               on_records_not(message);
               break;
               
            case Messages::logger_query_ex_stopped_not:
               on_stopped_not(message);
               break;
               
            default:
               DeviceBase::onNetMessage(router, message);
               break;
            }
         }
         else
            DeviceBase::onNetMessage(router, message);
      } // onNetMessage


      void LoggerQueryEx::on_start_ack(Csi::Messaging::Message *message)
      {
         uint4 reported_tran;
         uint4 rcd;
         if(message->readUInt4(reported_tran) && message->readUInt4(rcd) && reported_tran == tran_no)
         {
            client_type::start_outcome_type outcome;
            switch(rcd)
            {
            case 1:
               outcome = client_type::start_outcome_success;
               message->readBStr(formatted_header);
               message->readBStr(formatted_footer);
               state = state_started;
               break;

            case 2:
               outcome = client_type::start_outcome_failure_invalid_table_name;
               break;

            case 3:
               outcome = client_type::start_outcome_failure_comms_disabled;
               break;

            case 4:
               outcome = client_type::start_outcome_failure_server_security_blocked;
               break;

            case 5:
               outcome = client_type::start_outcome_failure_unsupported_query_mode;
               break;

            case 6:
               outcome = client_type::start_outcome_failure_unsupported_file_format;
               break;

            default:
               outcome = client_type::start_outcome_failure_unknown;
               break;
            }
            event_started::cpost(this, outcome);
         }
         else
            event_started::cpost(this, client_type::start_outcome_failure_unknown);
      } // on_start_ack


      void LoggerQueryEx::on_records_not(Csi::Messaging::Message *message)
      {
         uint4 reported_tran, records_count;
         if(message->readUInt4(reported_tran) && message->readUInt4(records_count) && reported_tran == tran_no)
         {
            for(uint4 i = 0; i < records_count; ++i)
            {
               record_handle record;
               int8 nsec;
               if(!records_cache.empty())
               {
                  record = records_cache.back();
                  records_cache.pop_back();
               }
               else
                  record.bind(new record_type);
               message->readUInt4(record->record_no);
               message->readInt8(nsec);
               record->timestamp = nsec;
               message->readBStr(record->record);
               records.push_back(record);
            }
            event_records::cpost(this);
         }
         else
            event_stopped::cpost(this, client_type::stopped_outcome_failure_unknown);
      } // on_records_not


      void LoggerQueryEx::on_stopped_not(Csi::Messaging::Message *message)
      {
         uint4 reported_tran, rcd;
         if(message->readUInt4(reported_tran) && reported_tran == tran_no && message->readUInt4(rcd))
         {
            client_type::stopped_outcome_type outcome;
            switch(rcd)
            {
            case 1:
               outcome = client_type::stopped_outcome_success;
               break;

            case 2:
               outcome = client_type::stopped_outcome_success_client_request;
               break;

            case 3:
               outcome = client_type::stopped_outcome_failure_table_defs;
               break;

            case 4:
               outcome = client_type::stopped_outcome_failure_comms;
               break;

            case 5:
               outcome = client_type::stopped_outcome_failure_comms_disabled;
               break;

            case 6:
               outcome = client_type::stopped_outcome_failure_blocked_by_logger;
               break;

            case 7:
               outcome = client_type::stopped_outcome_failure_ack_timed_out;
               break;
               
            default:
               outcome = client_type::stopped_outcome_failure_unknown;
               break;
            }
            event_stopped::cpost(this, outcome);
         }
         else
            event_stopped::cpost(this, client_type::stopped_outcome_failure_unknown);
      } // on_stopped_not
   };
};

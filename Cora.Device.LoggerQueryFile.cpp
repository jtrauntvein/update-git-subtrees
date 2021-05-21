/* Cora.Device.LoggerQueryFile.cpp

   Copyright (C) 2008, 2013 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 18 December 2008
   Last Change: Thursday 25 July 2013
   Last Commit: $Date: 2019-10-21 15:27:04 -0600 (Mon, 21 Oct 2019) $
   Last Changed by: $Author: amortenson $

*/

#pragma warning(disable: 4996)
#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.LoggerQueryFile.h"
#include "Csi.Utils.h"
#include "coratools.strings.h"


namespace Cora
{
   namespace Device
   {
      namespace
      {
         ////////////////////////////////////////////////////////////
         // class event_status
         ////////////////////////////////////////////////////////////
         class event_status: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // client
            ////////////////////////////////////////////////////////////
            typedef LoggerQueryFile query_type;
            typedef query_type::client_type client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // status_code
            ////////////////////////////////////////////////////////////
            typedef client_type::status_code_type status_code_type;
            status_code_type const status_code;

            ////////////////////////////////////////////////////////////
            // record_count
            ////////////////////////////////////////////////////////////
            uint4 const record_count;

            ////////////////////////////////////////////////////////////
            // begin_record_no
            ////////////////////////////////////////////////////////////
            uint4 const begin_record_no;

            ////////////////////////////////////////////////////////////
            // end_record_no
            ////////////////////////////////////////////////////////////
            uint4 const end_record_no;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               LoggerQueryFile *query,
               client_type *client,
               status_code_type status_code,
               uint4 record_count,
               uint4 begin_record_no,
               uint4 end_record_no)
            {
               event_status *event = new event_status(
                  query, client, status_code, record_count, begin_record_no, end_record_no);
               event->post();
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_status(
               LoggerQueryFile *query,
               client_type *client_,
               status_code_type status_code_,
               uint4 record_count_,
               uint4 begin_record_no_,
               uint4 end_record_no_):
               Event(event_id, query),
               client(client_),
               status_code(status_code_),
               record_count(record_count_),
               begin_record_no(begin_record_no_),
               end_record_no(end_record_no_)
            { }
         };


         uint4 const event_status::event_id = Csi::Event::registerType(
            "Cora::Device::LoggerQueryFile::event_status");


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
            typedef LoggerQueryFile query_type;
            typedef query_type::client_type client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // outcome
            ////////////////////////////////////////////////////////////
            client_type::outcome_type outcome;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(query_type *query, client_type *client, client_type::outcome_type outcome)
            {
               event_complete *event = new event_complete(query, client, outcome);
               event->post();
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_complete(
               query_type *query, client_type *client_, client_type::outcome_type outcome_):
               Event(event_id, query),
               client(client_),
               outcome(outcome_)
            { }
         };


         uint4 const event_complete::event_id =
            Csi::Event::registerType("Cora::Device::LoggerQueryFile::event_complete");
      };

      
      ////////////////////////////////////////////////////////////
      // class LoggerQueryFile definitions
      ////////////////////////////////////////////////////////////
      LoggerQueryFile::LoggerQueryFile():
         output(0),
         state(state_standby)
      {
         query.bind(new query_type);
         advisor.bind(new advisor_type);
      } // constructor


      LoggerQueryFile::~LoggerQueryFile()
      { finish(); }


      void LoggerQueryFile::set_output_file_name(StrAsc const &name)
      {
         if(state != state_standby)
            throw ClientBase::exc_invalid_state();
         output_file_name = name;
      } // set_output_file_name


      void LoggerQueryFile::set_append_data(bool append_data_)
      {
         if(state != state_standby)
            throw ClientBase::exc_invalid_state();
         append_data = append_data_;
      } // set_append_data

      
      void LoggerQueryFile::start(client_type *client_, router_handle router)
      {
         if(state != state_standby)
            throw ClientBase::exc_invalid_state();
         if(!client_type::is_valid_instance(client_))
            throw std::invalid_argument("invalid client pointer");
         client = client_;
         state = state_query_started;
         records_expected = records_collected = 0;
         advisor_started = false;
         query->start(this, router);
      } // start


      void LoggerQueryFile::start(client_type *client_, ClientBase *other_component)
      {
         if(state != state_standby)
            throw ClientBase::exc_invalid_state();
         if(!client_type::is_valid_instance(client_))
            throw std::invalid_argument("invalid client pointer");
         client = client_;
         state = state_query_started;
         records_expected = records_collected = 0;
         advisor_started = false;
         query->start(this, other_component);         
      } // start


      void LoggerQueryFile::finish()
      {
         query->finish();
         advisor->finish();
         client = 0;
         state = state_standby;
         if(output != 0)
         {
            fclose(output);
            output = 0;
         }
      } // finish


      void LoggerQueryFile::format_outcome(std::ostream &out, client_type::outcome_type outcome)
      {
         using namespace LoggerQueryFileStrings;
         switch(outcome)
         {
         case client_type::outcome_success:
            out << my_strings[strid_outcome_success];
            break;
            
         case client_type::outcome_server_session_failed:
            query_type::format_failure(out, query_type::client_type::failure_server_session_failed);
            break;
            
         case client_type::outcome_invalid_device_name:
            query_type::format_failure(out, query_type::client_type::failure_invalid_device_name);
            break;
            
         case client_type::outcome_unsupported:
            query_type::format_failure(out, query_type::client_type::failure_unsupported);
            break;
            
         case client_type::outcome_server_security_blocked:
            query_type::format_failure(out, query_type::client_type::failure_server_security_blocked);
            break;
            
         case client_type::outcome_logger_security_blocked:
            query_type::format_failure(out, query_type::client_type::failure_logger_security_blocked);
            break;
            
         case client_type::outcome_communication_failure:
            query_type::format_failure(out, query_type::client_type::failure_communication_failure);
            break;
            
         case client_type::outcome_communication_disabled:
            query_type::format_failure(out, query_type::client_type::failure_communication_disabled);
            break;
            
         case client_type::outcome_invalid_table_name:
            query_type::format_failure(out, query_type::client_type::failure_invalid_table_name);
            break;
            
         case client_type::outcome_invalid_table_definitions:
            query_type::format_failure(out, query_type::client_type::failure_invalid_table_definition);
            break;
            
         case client_type::outcome_insufficient_resources:
            query_type::format_failure(out, query_type::client_type::failure_insufficient_resources);
            break;
            
         case client_type::outcome_file_open_failed:
            out << my_strings[strid_outcome_file_open_failed];
            break;
            
         case client_type::outcome_query_interrupted:
            out << my_strings[strid_outcome_query_interrupted];
            break;
            
         case client_type::outcome_invalid_logon:
            query_type::format_failure(out, query_type::client_type::failure_invalid_logon);
            break;
            
         case client_type::outcome_invalid_format_option:
            advisor_type::format_failure(out, advisor_type::client_type::failure_invalid_format_option);
            break;
            
         case client_type::outcome_cannot_append:
            out << my_strings[strid_outcome_cannot_append];
            break;
            
         case client_type::outcome_file_io_failed:
            out << my_strings[strid_outcome_file_io_failed];
            break;
            
         default:
            query_type::format_failure(out, query_type::client_type::failure_unknown);
            break;
         }
      } // format_outcome


      void LoggerQueryFile::format_status(std::ostream &out, client_type::status_code_type status)
      {
         using namespace LoggerQueryFileStrings;
         switch(status)
         {
            case client_type::status_temporary_table_created:
               out << my_strings[strid_status_temporary_table_created];
               break;
            case client_type::status_some_data_collected:
               out << my_strings[strid_status_some_data_collected];
               break;
            case client_type::status_all_data_collected:
               out << my_strings[strid_status_all_data_collected];
               break;
            case client_type::status_file_mark_created:
               out << my_strings[strid_status_file_mark_created];
               break;
         }
      } // format_status


      void LoggerQueryFile::on_query_status(
         query_type *query,
         query_type::client_type::status_code_type status_code,
         StrUni const &temp_table_name,
         uint4 record_count,
         uint4 begin_record_no,
         uint4 end_record_no)
      {
         try
         {
            records_expected = record_count;
            switch(status_code)
            {
            case status_temporary_table_created:
            case status_file_mark_created:
            case status_some_data_collected:
               if(!advisor_started)
               {
                  advisor->set_open_broker_active_name(query->get_device_name());
                  advisor->set_table_name(temp_table_name);
                  advisor->set_order_option(advisor_type::order_collected);
                  advisor->set_start_option(advisor_type::start_at_record_id);
                  advisor->set_start_record_no(0);
                  advisor->set_start_file_mark_no(0);
                  advisor->set_cache_size_controller(1024);
                  advisor->set_reported_table_name(query->get_table_name());
                  advisor->start(this, query);
                  advisor_started = true;
               }
               event_status::cpost(
                  this,
                  client,
                  static_cast<client_type::status_code_type>(status_code),
                  record_count,
                  begin_record_no,
                  end_record_no);
               break;
               
            case status_all_data_collected:
               state = state_query_complete;
               if(records_expected == records_collected)
                  post_complete(client_type::outcome_success);
               else
                  event_status::cpost(
                     this,
                     client,
                     client_type::status_all_data_collected, 
                     record_count,
                     begin_record_no,
                     end_record_no);
               break;
               
            case status_query_interrupted:
               post_complete(client_type::outcome_query_interrupted);
               break;
            }
         }
         catch(std::exception &)
         { post_complete(client_type::outcome_unknown); }
      } // on_query_status


      void LoggerQueryFile::on_failure(
         query_type *query, query_type::client_type::failure_type failure)
      {
         client_type::outcome_type outcome;
         switch(failure)
         {
         case LoggerQueryClient::failure_invalid_logon:
            outcome = client_type::outcome_invalid_logon;
            break;
            
         case LoggerQueryClient::failure_server_session_failed:
            outcome = client_type::outcome_server_session_failed;
            break;
            
         case LoggerQueryClient::failure_invalid_device_name:
            outcome = client_type::outcome_invalid_device_name;
            break;
            
         case LoggerQueryClient::failure_unsupported:
            outcome = client_type::outcome_unsupported;
            break;
            
         case LoggerQueryClient::failure_server_security_blocked:
            outcome = client_type::outcome_server_security_blocked;
            break;
            
         case LoggerQueryClient::failure_logger_security_blocked:
            outcome = client_type::outcome_logger_security_blocked;
            break;
            
         case LoggerQueryClient::failure_communication_failure:
            outcome = client_type::outcome_communication_failure;
            break;
            
         case LoggerQueryClient::failure_communication_disabled:
            outcome = client_type::outcome_communication_disabled;
            break;
            
         case LoggerQueryClient::failure_invalid_table_name:
            outcome = client_type::outcome_invalid_table_name;
            break;
            
         case LoggerQueryClient::failure_invalid_table_definition:
            outcome = client_type::outcome_invalid_table_definitions;
            break;
            
         case LoggerQueryClient::failure_insufficient_resources:
            outcome = client_type::outcome_insufficient_resources;
            break;
            
         default:
            outcome = client_type::outcome_unknown;
            break;
         }
         post_complete(outcome);
      } // on_failure

      
      void LoggerQueryFile::on_started(
         advisor_type *advisor,
         StrAsc const &data_header,
         StrAsc const &data_footer)
      {
         header = data_header;
         footer = data_footer;
      } // on_started
      
      
      void LoggerQueryFile::on_failure(
         advisor_type *advisor,
         advisor_type::client_type::failure_type failure)
      {
         client_type::outcome_type outcome;
         switch(failure)
         {
         case advisor_type::client_type::failure_connection_failed:
            outcome = client_type::outcome_server_session_failed;
            break;
            
         case advisor_type::client_type::failure_invalid_logon:
            outcome = client_type::outcome_invalid_logon;
            break;
            
         case advisor_type::client_type::failure_invalid_station_name:
            outcome = client_type::outcome_invalid_device_name;
            break;
            
         case advisor_type::client_type::failure_invalid_table_name:
            outcome = client_type::outcome_invalid_table_name;
            break;
            
         case advisor_type::client_type::failure_server_security:
            outcome = client_type::outcome_server_security_blocked;
            break;
            
         case advisor_type::client_type::failure_invalid_format_option:
            outcome = client_type::outcome_invalid_format_option;
            break;
            
         case advisor_type::client_type::failure_station_shut_down:
            outcome = client_type::outcome_query_interrupted;
            break;
            
         case advisor_type::client_type::failure_unsupported:
            outcome = client_type::outcome_unsupported;
            break;

         default:
            outcome = client_type::outcome_unknown;
            break;
         }
         post_complete(outcome);
      } // on_failure

      
      void LoggerQueryFile::on_records(
         advisor_type *advisor)
      {
         // we need to check to see if the file is opened.  If not, now is the time to do so
         client_type::outcome_type outcome = client_type::outcome_success;
         try
         {
            if(output == 0)
            {
               // if the file needs to be appended, we need to validate our header against the files
               bool should_append = false;
               int8 append_offset = 0;
               if(append_data && Csi::file_exists(output_file_name.c_str()))
               {
                  StrAsc reason;
                  int rcd = Csi::data_file_can_append(
                     output_file_name.c_str(),
                     advisor->get_format_option(),
                     header.c_str(),
                     (uint4)header.length(),
                     &append_offset,
                     &reason);
                  if(!client_type::is_valid_instance(client))
                     throw client_type::outcome_query_interrupted;
                  if(rcd != Csi::can_append_success && !client->on_append_warnings(this, rcd, reason))
                     throw client_type::outcome_cannot_append;
                  should_append = true;
               }

               // now we need to try to open the file
               output = Csi::open_file(output_file_name.c_str(), should_append ? "ab" : "wb");
               if(output != 0)
               {
                  if(!should_append)
                     fwrite(header.c_str(), header.length(), 1, output);
                  else
                     Csi::file_seek(output, append_offset, SEEK_SET);
               }
               else
                  throw client_type::outcome_file_open_failed;
            }


            // we can now try to output the records
            for(advisor_type::const_iterator ri = advisor->begin(); ri != advisor->end(); ++ri)
            {
               StrBin const &data = ri->get_formatted_data();
               Csi::efwrite(data.getContents(), data.length(), 1, output);
               ++records_collected;
            }
            advisor->continue_advise();
         }
         catch(std::exception &)
         { outcome = client_type::outcome_file_io_failed; }
         catch(client_type::outcome_type outcome_)
         { outcome = outcome_; }
         if(outcome != client_type::outcome_success ||
            (state == state_query_complete && records_collected == records_expected))
            post_complete(outcome);
      } // on_records


      void LoggerQueryFile::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         if(state != state_standby)
         {
            if(ev->getType() == event_status::event_id)
            {
               event_status *event = static_cast<event_status *>(ev.get_rep());
               if(client == event->client && client_type::is_valid_instance(client))
               {
                  client->on_query_status(
                     this,
                     event->status_code,
                     event->record_count,
                     event->begin_record_no,
                     event->end_record_no);
               }
               else if(!client_type::is_valid_instance(client))
                  finish();
            }
            else if(ev->getType() == event_complete::event_id)
            {
               event_complete *event = static_cast<event_complete *>(ev.get_rep());
               if(event->client == client)
               {
                  finish();
                  if(client_type::is_valid_instance(event->client))
                     event->client->on_complete(this, event->outcome);
               }
            }
         }
      } // receive

      
      void LoggerQueryFile::post_complete(client_type::outcome_type outcome)
      {
         if(output != 0)
         {
            if(footer.length() > 0)
               fwrite(footer.c_str(), footer.length(), 1, output);
            fclose(output);
            output = 0;
         }
         event_complete::cpost(this, client, outcome);
      } // post_complete
   };
};


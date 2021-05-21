/* Cora.LgrNet.LogQuery.cpp

   Copyright (C) 2020, 2020 Campbell Scientific, Inc.

   Written by: Andrew Mortenson
   Date Begun: Thursday 13 February 2020
   Last Change: Saturday 08 August 2020
   Last Commit: $Date: 2020-09-17 07:14:44 -0600 (Thu, 17 Sep 2020) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.LgrNet.LogQuery.h"
#include "Cora.LgrNet.Defs.h"


namespace Cora
{
   namespace LgrNet
   {
      namespace
      {
         /**
          * Defines the event that will be posted when log records have been received.
          */
         class event_log_records: public Csi::Event
         {
         public:
            static uint4 const event_id;

            static void cpost(LogQuery *sender)
            { (new event_log_records(sender))->post(); }

         private:
            event_log_records(LogQuery *sender):
               Event(event_id, sender)
            {}
         };
         uint4 const event_log_records::event_id(
            Csi::Event::registerType("Cora::LgrNet::LogQuery::event_log_records"));


         /**
          * Defines the event that will be posted when the query is complete.
          */
         class event_complete: public Csi::Event
         {
         public:
            static uint4 const event_id;
            typedef LogQueryClient::outcome_type outcome_type;
            outcome_type const outcome;

            static void cpost(LogQuery *sender, outcome_type outcome)
            { (new event_complete(sender, outcome))->post(); }

         private:
            event_complete(LogQuery *sender, outcome_type outcome_):
               Event(event_id, sender),
               outcome(outcome_)
            { }
         };
         uint4 const event_complete::event_id(
            Csi::Event::registerType("Cora::LgrNet::LogQuery::event_complete"));
      };


      void LogQuery::format_outcome(std::ostream &out, client_type::outcome_type outcome)
      {
         switch(outcome)
         {
         case client_type::outcome_success:
            out << "success";
            break;
            
         case client_type::outcome_failure_logon:
            describe_failure(out, corabase_failure_logon);
            break;
            
         case client_type::outcome_failure_session:
            describe_failure(out, corabase_failure_session);
            break;
            
         case client_type::outcome_failure_unsupported:
            describe_failure(out, corabase_failure_unsupported);
            break;
            
         case client_type::outcome_failure_security:
            describe_failure(out, corabase_failure_security);
            break;

         case client_type::outcome_failure_expression:
            out << "invalid filter expression";
            break;
         case client_type::outcome_success_client_request:
            out << "aborted because of user action";
            break;
         default:
            describe_failure(out, corabase_failure_unknown);
            break;
         }
      } // format_failure
      

      void LogQuery::on_corabase_ready()
      {
         Csi::Messaging::Message start_cmd(net_session, Messages::log_query_start_cmd);
         server_tran = ++last_tran_no;
         start_cmd.addUInt4(server_tran);
         start_cmd.addUInt4(log_id);
         start_cmd.addStr(expression);
         start_cmd.addUInt4(backfill_interval);
         start_cmd.addInt8(begin.get_nanoSec());
         start_cmd.addInt8(end.get_nanoSec());
         state = state_active;
         router->sendMessage(&start_cmd);
      } // on_corabase_ready
      

      void LogQuery::on_corabase_failure(corabase_failure_type failure)
      {
         client_type::outcome_type report;
         switch(failure)
         {
         case corabase_failure_logon:
            report = client_type::outcome_failure_logon;
            break;
            
         case corabase_failure_session:
            report = client_type::outcome_failure_session;
            break;
            
         case corabase_failure_unsupported:
            report = client_type::outcome_failure_unsupported;
            break;
            
         case corabase_failure_security:
            report = client_type::outcome_failure_security;
            break;
            
         default:
            report = client_type::outcome_failure_unknown;
            break;
         }
         event_complete::cpost(this, report);
      } // on_corabase_failure
      

      void LogQuery::onNetMessage(Csi::Messaging::Router *rtr, Csi::Messaging::Message *msg)
      {
         if(state == state_active)
         {
            switch(msg->getMsgType())
            {
            case Messages::log_query_not:
               on_log_query_not(msg);
               break;
               
            case Messages::log_query_stopped_not:
               on_log_query_stopped_not(msg);
               break;
               
            default:
               ClientBase::onNetMessage(rtr, msg);
               break;
            }
         }
         ClientBase::onNetMessage(rtr, msg);
      } // onNetMessage
      

      void LogQuery::receive(event_handle &ev)
      {
         client_type *report(client);
         if(ev->getType() == event_log_records::event_id)
         {
            event_log_records *event(static_cast<event_log_records *>(ev.get_rep()));
            if(client_type::is_valid_instance(report))
            {
               if(!records.empty())
               {
                  if(report->on_records(this, records))
                  {
                     cache.insert(cache.end(), records.begin(), records.end());
                     records.clear();
                     send_continue_cmd();
                  }
                  else
                     send_continue_cmd(false);
               }
            }
            else
               finish();
         }
         else if(ev->getType() == event_complete::event_id)
         {
            event_complete *event(static_cast<event_complete *>(ev.get_rep()));
            finish();
            if(client_type::is_valid_instance(report))
               report->on_complete(this, event->outcome);
         }
      } // receive
      

      void LogQuery::send_continue_cmd(bool cont)
      {
         Csi::Messaging::Message command(net_session, Messages::log_query_cont);
         command.addUInt4(server_tran);
         command.addBool(cont);
         command.addInt8(last_sequence);
         router->sendMessage(&command);
      } // send_continue_cmd

      
      void LogQuery::on_log_query_not(Csi::Messaging::Message *message)
      {
         uint4 tran_no, records_count;
         bool rcd(message->readUInt4(tran_no) && message->readUInt4(records_count));
         for(uint4 i = 0; tran_no == server_tran && rcd && i < records_count; ++i)
         {
            int8 nsec;
            record_handle record;
            bool time_ok(false), record_ok(false);

            if(!cache.empty())
            {
               record = cache.front();
               cache.pop_front();
            }
            else
               record.bind(new record_type);
            time_ok = message->readInt8(nsec);
            if(time_ok)
               record_ok = message->readStr(record->data);
            rcd = time_ok && record_ok;
            if(rcd)
            {
               record->stamp = nsec;
               records.push_back(record);
            }
            else
               trace("unexpected problem reading message");
         }
         if(rcd)
         {
            message->readInt8(last_sequence);
            event_log_records::cpost(this);
         }
         else
            event_complete::cpost(this, client_type::outcome_failure_unknown);
      } // on_log_query_not
      

      void LogQuery::on_log_query_stopped_not(Csi::Messaging::Message *message)
      {
         uint4 tran_no;
         uint4 reason;
         client_type::outcome_type outcome(client_type::outcome_failure_unknown);
         if(message->readUInt4(tran_no) && message->readUInt4(reason) && server_tran == tran_no)
         {
            switch(reason)
            {
            case 1:
               outcome = client_type::outcome_success;
               break;
            case 2:
               outcome = client_type::outcome_success_client_request;
               break;

            case 3:
               outcome = client_type::outcome_failure_expression;
               break;
            }
         }
         event_complete::cpost(this, outcome);
      } // on_log_query_stopped_not
   };
};



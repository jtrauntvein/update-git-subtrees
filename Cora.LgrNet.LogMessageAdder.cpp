/* Cora.LgrNet.LogMessageAdder.cpp

   Copyright (C) 2000, 2021 Campbell Scientific, Inc.

   Written by: tmecham
   Last Modified by: $Author: jon $
   Date Begun: Thursday 2 August 2001
   Last Change: Monday 01 March 2021
   Last Commit: $Date: 2021-03-01 14:37:09 -0600 (Mon, 01 Mar 2021) $ 

*/


#pragma warning(disable:4786) // disable overlong symbols warning
#pragma hdrstop               // stop creation of precompiled header
#include "Cora.LgrNet.LogMessageAdder.h"
#include "Cora.LgrNet.Defs.h"
#include <assert.h>


namespace Cora
{
   namespace LgrNet
   {
      namespace LogMessageAdderHelpers
      {
         class event_complete: public Csi::Event
         {
         public:
            static uint4 const event_id;

            typedef LogMessageAdderClient::outcome_type outcome_type;
            static void create_and_post(
               LogMessageAdder *adder,
               LogMessageAdderClient *client,
               outcome_type outcome);

            void notify()
            { client->on_complete(adder,outcome); }
            
         private:
            LogMessageAdder *adder;
            LogMessageAdderClient *client;
            outcome_type outcome;

            event_complete(
               LogMessageAdder *adder_,
               LogMessageAdderClient *client_,
               outcome_type outcome_):
               Event(event_id,adder_),
               adder(adder_),
               client(client_),
               outcome(outcome_)
            { }

            friend class Cora::LgrNet::LogMessageAdder;
         };
         uint4 const event_complete::event_id(
            Csi::Event::registerType("Cora::LgrNet::LogMessageAdder::event_complete"));


         void event_complete::create_and_post(
            LogMessageAdder *adder,
            LogMessageAdderClient *client,
            outcome_type outcome)
         {
            try { (new event_complete(adder,client,outcome))->post(); }
            catch(Event::BadPost &) { }
         } // create_and_post
      };

      void LogMessageAdder::describe_outcome(
         std::ostream& out, 
         LogMessageAdderClient::outcome_type outcome)
      {
         switch (outcome)
         {
         case LogMessageAdderClient::outcome_success:
            out << "success";
            break;
            
         case LogMessageAdderClient::outcome_session_failed:
            describe_failure(out, corabase_failure_session);
            break;
            
         case LogMessageAdderClient::outcome_invalid_logon:
            describe_failure(out, corabase_failure_logon);
            break;
            
         case LogMessageAdderClient::outcome_server_security_blocked:
            describe_failure(out, corabase_failure_security);
            break;
            
         case LogMessageAdderClient::outcome_unsupported_transaction:
            describe_failure(out, corabase_failure_unsupported);
            break;
            
         case LogMessageAdderClient::outcome_invalid_log_identifier:
            out << "invalid log identifier";
            break;

         case client_type::outcome_invalid_tran_log_id:
            out << "invalid transaction log message type";
            break;
            
         default:
         case LogMessageAdderClient::outcome_unknown:
            describe_failure(out, corabase_failure_unknown);
            break;              
         }
      }

      void LogMessageAdder::on_corabase_ready()
      {
         Csi::Messaging::Message cmd(net_session, Messages::add_log_message_cmd);
         cmd.addUInt4(++last_tran_no);
         cmd.addUInt4(log_identifier);
         cmd.addInt8(log_date.get_nanoSec());
         cmd.addStr(log_message);
         if(log_identifier == transaction_log)
         {
            cmd.addUInt4(tran_log_id);
            cmd.addStr(tran_log_desc);
         }
         router->sendMessage(&cmd);
         state = state_active; 
      } // on_corabase_ready
      
      void LogMessageAdder::on_corabase_failure(corabase_failure_type failure)
      {
         using namespace LogMessageAdderHelpers;
         client_type::outcome_type outcome;
         switch(failure)
         {
         case corabase_failure_logon:
            outcome = client_type::outcome_invalid_logon;
            break;
            
         case corabase_failure_session:
            outcome = client_type::outcome_session_failed;
            break;
            
         case corabase_failure_unsupported:
            outcome = client_type::outcome_unsupported_transaction;
            break;
            
         case corabase_failure_security:
            outcome = client_type::outcome_server_security_blocked;
            break;
            
         default:
            outcome = client_type::outcome_unknown;
            break;
         }
         event_complete::create_and_post(this,client,outcome);
      } // on_corabase_failure

      void LogMessageAdder::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(state == state_active)
         {
            if(msg->getMsgType() == Messages::add_log_message_ack)
            {
               using namespace LogMessageAdderHelpers;
               uint4 tran_no;
               uint4 resp_code;
               client_type::outcome_type outcome;
               
               msg->readUInt4(tran_no);
               msg->readUInt4(resp_code);
               switch(resp_code)
               {
               case 1:
                  outcome = client_type::outcome_success;
                  break;
                  
               case 2:
                  outcome = client_type::outcome_invalid_log_identifier;
                  break;

               case 3:
                  outcome = client_type::outcome_invalid_tran_log_id;
                  break;

               default:
                  outcome = client_type::outcome_unknown;
                  break;
               }
               event_complete::create_and_post(this,client,outcome); 
            }
            else
               ClientBase::onNetMessage(rtr,msg);
         }
         else
            ClientBase::onNetMessage(rtr,msg);
      } // onNetMessage
      
      void LogMessageAdder::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace LogMessageAdderHelpers;

         if(ev->getType() == event_complete::event_id)
         {
            event_complete *event = static_cast<event_complete *>(ev.get_rep());
            
            finish();
            if(LogMessageAdderClient::is_valid_instance(event->client))
               event->notify(); 
         }
      } // receive 
   };
};

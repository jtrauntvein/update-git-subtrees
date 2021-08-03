/* Cora.Broker.TableDefsGetter.cpp

   Copyright (C) 2004, 2021 Campbell Scientific, Inc.

   Written by: Tyler Mecham
   Date Begun: Thursday 22 January 2004
   Last Change: Monday 12 April 2021
   Last Commit: $Date: 2021-04-12 09:23:10 -0600 (Mon, 12 Apr 2021) $ 
   Committed by: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Broker.TableDefsGetter.h"


namespace Cora
{
   namespace Broker
   {
      namespace 
      {
         class event_complete: public Csi::Event
         {
         public:
            static uint4 const event_id;

            typedef TableDefsGetter::client_type client_type;
            client_type *client;
            typedef client_type::outcome_type outcome_type;
            outcome_type outcome;
            typedef TableDefsGetterClient::table_defs_type table_defs_type;
            table_defs_type table_defs;

            typedef TableDefsGetter receiver_type;
            static event_complete *create(
               receiver_type *receiver,
               client_type *client,
               outcome_type outcome,
               table_defs_type &table_defs)
            { return new event_complete(receiver,client,outcome,table_defs); }

         private:
            event_complete(
               receiver_type *receiver,
               client_type *client_,
               outcome_type outcome_,
               table_defs_type &table_defs_):
               Event(event_id,receiver),
               client(client_),
               outcome(outcome_),
               table_defs(table_defs_)
            { }
         };
         uint4 const event_complete::event_id =
            Csi::Event::registerType("Cora::Broker::TableDefsGetter::event_complete");
      };


      void TableDefsGetter::describe_outcome(std::ostream &out, client_type::outcome_type outcome)
      {
         switch(outcome)
         {
         case client_type::outcome_success:
            out << "success";
            break;

         case client_type::outcome_invalid_table:
            out << "invalid table name";
            break;
            
         case client_type::outcome_invalid_logon:
            format_failure(out, brokerbase_failure_logon);
            break;
            
         case client_type::outcome_connection_failed:
            format_failure(out, brokerbase_failure_session);
            break;
            
         case client_type::outcome_invalid_station_name:
            format_failure(out, brokerbase_failure_invalid_id);
            break;
            
         case client_type::outcome_unsupported:
            format_failure(out, brokerbase_failure_unsupported);
            break;
            
         case client_type::outcome_server_security_blocked:
            format_failure(out, brokerbase_failure_security);
            break;
            
         default:
            format_failure(out, brokerbase_failure_unknown);
            break;
         }
      } // dscribe_outcome
      
      void TableDefsGetter::on_brokerbase_ready()
      {
         state = state_active;
         if(get_interface_version() < Csi::VersionNumber("1.3.1"))
         {
            Csi::Messaging::Message cmd(broker_session,Cora::Broker::Messages::table_def_get_cmd);
            cmd.addUInt4(++last_tran_no);
            cmd.addWStr(table_name.c_str());
            router->sendMessage(&cmd);
         }
         else
         {
            Csi::Messaging::Message command(
               broker_session,
               Cora::Broker::Messages::extended_table_def_get_cmd);
            command.addUInt4(++last_tran_no);
            command.addWStr(table_name.c_str());
            router->sendMessage(&command);
         }
      } // on_brokerbase_ready

      void TableDefsGetter::on_brokerbase_failure(brokerbase_failure_type failure)
      {
         client_type::outcome_type outcome;
         switch(failure)
         {
         default:
            outcome = client_type::outcome_unknown;
            break;

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
            outcome = client_type::outcome_server_security_blocked;
            break;
            
         }
         event_complete::table_defs_type table_defs;
         event_complete::create(this,client,outcome,table_defs)->post();
      }

      void TableDefsGetter::on_brokerbase_session_failure()
      {
         event_complete::table_defs_type table_defs;
         event_complete::create(this,client,client_type::outcome_connection_failed,table_defs)->post();
      }

      void TableDefsGetter::onNetMessage(Csi::Messaging::Router *rtr, Csi::Messaging::Message *msg)
      {
         if(state == state_active)
         {
            if(msg->getMsgType() ==  Cora::Broker::Messages::table_def_get_ack ||
               msg->getMsgType() == Cora::Broker::Messages::extended_table_def_get_ack)
            {
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
                  outcome = client_type::outcome_invalid_table;
                  break;

               default:
                  outcome = client_type::outcome_unknown;
                  break;
               }

               event_complete::table_defs_type table_defs(new Cora::Broker::TableDesc);
               if(resp_code == client_type::outcome_success)
               {
                  if(!table_defs->read(msg))
                  {
                     table_defs.clear();
                     outcome = client_type::outcome_unknown;
                  }
               }
               event_complete *event = event_complete::create(this,client,outcome,table_defs);
               event->post();
            }
            else
               BrokerBase::onNetMessage(rtr,msg);
         }
         else
            BrokerBase::onNetMessage(rtr,msg);
      }

      void TableDefsGetter::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         if(ev->getType() == event_complete::event_id)
         {
            event_complete *event = static_cast<event_complete *>(ev.get_rep());
            if(event->client == client && client_type::is_valid_instance(client))
            {
               finish();
               event->client->on_complete(this,event->outcome,event->table_defs);
            }
            else
               finish();
         }
      }
   };
};
